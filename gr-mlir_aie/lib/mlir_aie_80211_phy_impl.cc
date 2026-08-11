/* -*- c++ -*- */
/*
 * Copyright 2026 gr-mlir_aie author.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "mlir_aie_80211_phy_impl.h"
#include <gnuradio/io_signature.h>
#include <pmt/pmt.h>

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstring>
#include <iostream>

namespace gr {
namespace mlir_aie {

mlir_aie_80211_phy::sptr mlir_aie_80211_phy::make(const char* path_xclbin,
                                                   const char* path_insts_bin,
                                                   const char* kernel_name,
                                                   int VECTOR_SIZE,
                                                   double nominal_frequency)
{
    return gnuradio::make_block_sptr<mlir_aie_80211_phy_impl>(
        path_xclbin, path_insts_bin, kernel_name, VECTOR_SIZE, nominal_frequency);
}

mlir_aie_80211_phy_impl::mlir_aie_80211_phy_impl(const char* path_xclbin,
                                                 const char* path_insts_bin,
                                                 const char* kernel_name,
                                                 int VECTOR_SIZE,
                                                 double nominal_frequency)
    : gr::block("mlir_aie_80211_phy",
                 gr::io_signature::make(1, 1, sizeof(phy_input_type)),
                 gr::io_signature::make(1, 1, sizeof(phy_output_type))),
      _nominal_frequency(nominal_frequency)
{
    _path_xclbin = path_xclbin;
    _path_insts_bin = path_insts_bin;
    _VECTOR_SIZE = VECTOR_SIZE;
    _TILE_SIZE = _VECTOR_SIZE / _N_TILES;
    _kernel_name = kernel_name;
    _trace_size = 0;
    _opcode_run = 3;

    set_tag_propagation_policy(TPP_DONT);

    _instr_v = test_utils::load_instr_binary(path_insts_bin);
    std::cout << "Sequence instr count: " << _instr_v.size() << "\n";

    test_utils::init_xrt_load_kernel(_device, _kernel, 1, path_xclbin, _kernel_name);

    std::cout << "kernel load ok";
    _bo_instr = xrt::bo(_device,
                        _instr_v.size() * sizeof(int),
                        XCL_BO_FLAGS_CACHEABLE,
                        _kernel.group_id(1));
    _bo_inA = xrt::bo(_device,
                      _VECTOR_SIZE * sizeof(phy_input_type),
                      XRT_BO_FLAGS_HOST_ONLY,
                      _kernel.group_id(3));
    _bo_out = xrt::bo(_device,
                      _VECTOR_SIZE * sizeof(phy_output_type) + _trace_size,
                      XRT_BO_FLAGS_HOST_ONLY,
                      _kernel.group_id(3));
    _bo_out_meta = xrt::bo(_device,
                           _N_TILES * sizeof(tile_metadata),
                           XRT_BO_FLAGS_HOST_ONLY,
                           _kernel.group_id(3));

    std::cout << "Writing data into buffer objects.\n";

    bufInstr = _bo_instr.map<void*>();
    std::memcpy(bufInstr, _instr_v.data(), _instr_v.size() * sizeof(int));

    _bufInA = _bo_inA.map<phy_input_type*>();
    _bufOut = _bo_out.map<phy_output_type*>();
    _bufOutMeta = _bo_out_meta.map<tile_metadata*>();
    std::memset(_bufOut, 42, _VECTOR_SIZE * sizeof(phy_output_type) + _trace_size);
    std::memset(_bufOutMeta, 0, _N_TILES * sizeof(tile_metadata));

    _bo_out.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    _bo_out_meta.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    _bo_instr.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    _run = xrt::run(_kernel);

    _run.set_arg(0, _opcode_run);
    _run.set_arg(1, _bo_instr);
    _run.set_arg(2, _instr_v.size());
    _run.set_arg(3, _bo_inA);
    _run.set_arg(4, _bo_out);
    _run.set_arg(5, _bo_out_meta);
}

mlir_aie_80211_phy_impl::~mlir_aie_80211_phy_impl() {}

void mlir_aie_80211_phy_impl::set_nominal_frequency(double nominal_frequency)
{
    _nominal_frequency.store(nominal_frequency);
}

double mlir_aie_80211_phy_impl::nominal_frequency() const
{
    return _nominal_frequency.load();
}

void mlir_aie_80211_phy_impl::forecast(int noutput_items,
                                       gr_vector_int& ninput_items_required)
{
    ninput_items_required[0] = noutput_items;
}

int mlir_aie_80211_phy_impl::general_work(int noutput_items,
                                          gr_vector_int& ninput_items,
                                          gr_vector_const_void_star& input_items,
                                          gr_vector_void_star& output_items)
{
    auto in = static_cast<const phy_input_type*>(input_items[0]);
    auto out = static_cast<phy_output_type*>(output_items[0]);

    const int n_chunks = std::min(ninput_items[0], noutput_items) / _VECTOR_SIZE;
    if (n_chunks == 0) {
        return 0;
    }

    const auto frame_bytes_key = pmt::intern("frame bytes");
    const auto encoding_key = pmt::intern("encoding");
    const auto snr_key = pmt::intern("snr");
    const auto nominal_frequency_key = pmt::intern("nominal frequency");
    const auto frequency_offset_key = pmt::intern("frequency offset");
    const auto beta_key = pmt::intern("beta");
    const auto csi_key = pmt::intern("csi");
    const auto tag_srcid = pmt::intern("frame_equalizer");
    constexpr double q16_15_scale = 1.0 / (std::int64_t{ 1 } << 15);
    const uint64_t output_abs_start = nitems_written(0);
    int total_produced = 0;

    for (int i = 0; i < n_chunks; ++i) {
        const phy_input_type* in_ptr = in + (i * _VECTOR_SIZE);

        std::memcpy(_bufInA, in_ptr, _VECTOR_SIZE * sizeof(phy_input_type));
        _bo_inA.sync(XCL_BO_SYNC_BO_TO_DEVICE);

        _run.start();
        _run.wait();

        _bo_out.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
        _bo_out_meta.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

        for (int tile_idx = 0; tile_idx < _N_TILES; ++tile_idx) {
            const tile_metadata& tile_meta = _bufOutMeta[tile_idx];

            int tile_len = tile_meta.output_length;
            const int tag_count =
                std::clamp(tile_meta.tag_count, 0, _MAX_TAGS_PER_TILE);
            const int tile_start = tile_idx * _TILE_SIZE;

            if (tile_len < 0) {
                tile_len = 0;
            } else if (tile_len > _TILE_SIZE) {
                tile_len = _TILE_SIZE;
            }

            std::memcpy(out + total_produced,
                        _bufOut + tile_start,
                        tile_len * sizeof(phy_output_type));

            const uint64_t tile_abs_start = output_abs_start + total_produced;
            for (int tag_idx = 0; tag_idx < tag_count; ++tag_idx) {
                const tag_metadata& tag = tile_meta.tags[tag_idx];

                if (0 <= tag.offset && tag.offset < tile_len) {
                    const uint64_t tag_offset = tile_abs_start + tag.offset;
                    std::vector<std::complex<float>> csi(_CSI_TAG_SIZE);
                    for (int csi_idx = 0; csi_idx < _CSI_TAG_SIZE; ++csi_idx) {
                        csi[csi_idx] = {
                            static_cast<float>(tag.csi[csi_idx].real * q16_15_scale),
                            static_cast<float>(tag.csi[csi_idx].imag * q16_15_scale)
                        };
                    }

                    const double snr =
                        10.0 * std::log10(static_cast<double>(tag.snr_linear) / 2.0);
                    add_item_tag(0,
                                 tag_offset,
                                 frame_bytes_key,
                                 pmt::from_uint64(tag.frame_bytes),
                                 tag_srcid);
                    add_item_tag(0,
                                 tag_offset,
                                 encoding_key,
                                 pmt::from_uint64(tag.encoding),
                                 tag_srcid);
                    add_item_tag(
                        0,
                        tag_offset,
                        snr_key,
                        pmt::from_double(snr),
                        tag_srcid);
                    add_item_tag(0,
                                  tag_offset,
                                  nominal_frequency_key,
                                  pmt::from_double(nominal_frequency()),
                                  tag_srcid);
                    add_item_tag(0,
                                 tag_offset,
                                 frequency_offset_key,
                                 pmt::from_double(tag.frequency_offset * q16_15_scale),
                                 tag_srcid);
                    add_item_tag(
                        0,
                        tag_offset,
                        beta_key,
                        pmt::from_double(tag.beta * q16_15_scale),
                        tag_srcid);
                    add_item_tag(0,
                                 tag_offset,
                                 csi_key,
                                 pmt::init_c32vector(csi.size(), csi),
                                 tag_srcid);
                }
            }

            total_produced += tile_len;
        }
    }

    const int processed_items = n_chunks * _VECTOR_SIZE;
    consume_each(processed_items);

    return total_produced;
}

} /* namespace mlir_aie */
} /* namespace gr */
