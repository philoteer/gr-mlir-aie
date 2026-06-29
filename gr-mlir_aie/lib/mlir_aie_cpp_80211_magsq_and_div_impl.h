/* -*- c++ -*- */
/*
 * Copyright 2026 gr-mlir_aie author.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_MLIR_AIE_MLIR_AIE_CPP_80211_MAGSQ_AND_DIV_IMPL_H
#define INCLUDED_MLIR_AIE_MLIR_AIE_CPP_80211_MAGSQ_AND_DIV_IMPL_H

#include <gnuradio/mlir_aie/mlir_aie_cpp_80211_magsq_and_div.h>

#include "runtime_lib/test_lib/test_utils.h"
#include "xrt/xrt_bo.h"
#include "xrt/xrt_device.h"
#include "xrt/xrt_kernel.h"

#include <cstdint>
#include <cstring>
#include <vector>

namespace gr {
namespace mlir_aie {

using magsq_complex_input_type = gr_complex;
using magsq_mag_input_type = float;
using magsq_output_type = float;

using magsq_aie_complex_type = std::uint32_t;
using magsq_aie_bfloat_type = std::uint16_t;

class mlir_aie_cpp_80211_magsq_and_div_impl : public mlir_aie_cpp_80211_magsq_and_div
{
private:
    const char* _path_xclbin;
    const char* _path_insts_bin;
    int _VECTOR_SIZE;
    const char* _kernel_name;
    int _trace_size;
    unsigned int _opcode_run;
    xrt::kernel _kernel;
    xrt::bo _bo_instr, _bo_ac_in, _bo_mag_in, _bo_out;
    std::vector<std::uint32_t> _instr_v;
    xrt::device _device;
    xrt::run _run;

    magsq_aie_complex_type* _bufAcIn;
    magsq_aie_bfloat_type* _bufMagIn;
    magsq_aie_bfloat_type* _bufOut;
    void* _bufInstr;

    static std::uint16_t float_to_bfloat16(float f)
    {
        std::uint32_t x;
        std::memcpy(&x, &f, sizeof(x));
        return static_cast<std::uint16_t>(x >> 16);
    }

    static float bfloat16_to_float(std::uint16_t bf)
    {
        std::uint32_t x = static_cast<std::uint32_t>(bf) << 16;
        float f;
        std::memcpy(&f, &x, sizeof(f));
        return f;
    }

    static std::uint32_t complex_to_cbfloat(gr_complex c)
    {
        return (static_cast<std::uint32_t>(float_to_bfloat16(c.imag())) << 16) |
               float_to_bfloat16(c.real());
    }

public:
    mlir_aie_cpp_80211_magsq_and_div_impl(const char* path_xclbin,
                                          const char* path_insts_bin,
                                          const char* kernel_name,
                                          int VECTOR_SIZE);
    ~mlir_aie_cpp_80211_magsq_and_div_impl();

    // Where all the action really happens
    void forecast(int noutput_items, gr_vector_int& ninput_items_required);

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items);
};

} // namespace mlir_aie
} // namespace gr

#endif /* INCLUDED_MLIR_AIE_MLIR_AIE_CPP_80211_MAGSQ_AND_DIV_IMPL_H */
