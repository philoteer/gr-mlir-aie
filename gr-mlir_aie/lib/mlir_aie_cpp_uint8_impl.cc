/* -*- c++ -*- */
/*
 * Copyright 2026 gr-mlir_aie author.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "mlir_aie_cpp_uint8_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace mlir_aie {

mlir_aie_cpp_uint8::sptr mlir_aie_cpp_uint8::make(const char* path_xclbin,
                                                  const char* path_insts_bin,
                                                  int VECTOR_SIZE)
{
    return gnuradio::make_block_sptr<mlir_aie_cpp_uint8_impl>(
        path_xclbin, path_insts_bin, VECTOR_SIZE);
}


/*
 * The private constructor
 * Based on the official AMD mlir-aie passthru kernel test.cpp example
 */
mlir_aie_cpp_uint8_impl::mlir_aie_cpp_uint8_impl(const char* path_xclbin,
                                                 const char* path_insts_bin,
                                                 int VECTOR_SIZE)
    : gr::block("mlir_aie_cpp_uint8",
                gr::io_signature::make(
                    1 /* min inputs */, 1 /* max inputs */, sizeof(input_type)),
                gr::io_signature::make(
                    1 /* min outputs */, 1 /*max outputs */, sizeof(output_type)))
{
    _path_xclbin = path_xclbin;
    _path_insts_bin = path_insts_bin;
    _VECTOR_SIZE = VECTOR_SIZE;
    _kernel_name =  "MLIR_AIE:MLIRAIE"; //TODO FIX (make this a parameter?)
    _trace_size = 0;
    _opcode_run = 3;
    xrt::device device;

    // Load instruction sequence
    std::vector<uint32_t> _instr_v = test_utils::load_instr_binary(path_insts_bin);
    std::cout << "Sequence instr count: " << _instr_v.size() << "\n";
    
    // Start the XRT context and load the kernel
    test_utils::init_xrt_load_kernel(device, _kernel, 0,
                                   path_xclbin,
                                   _kernel_name);

    std::cout << "#1";
    // set up the buffer objects
    _bo_instr = xrt::bo(device, _instr_v.size() * sizeof(int),
                          XCL_BO_FLAGS_CACHEABLE, _kernel.group_id(1));
    _bo_inA = xrt::bo(device,  _VECTOR_SIZE * sizeof(input_type),
                        XRT_BO_FLAGS_HOST_ONLY, _kernel.group_id(3));
    _bo_out =
      xrt::bo(device,  _VECTOR_SIZE * sizeof(output_type) + _trace_size,
              XRT_BO_FLAGS_HOST_ONLY, _kernel.group_id(3));
              
    std::cout << "Writing data into buffer objects.\n";

    // Copy instruction stream to xrt buffer object
    void *bufInstr = _bo_instr.map<void *>();
    memcpy(bufInstr, _instr_v.data(), _instr_v.size() * sizeof(int));
    
    // Initialize buffers
    _bufInA = _bo_inA.map<input_type *>();     
    _bufOut = _bo_out.map<output_type *>();
    
}

/*
 * Our virtual destructor.
 */
mlir_aie_cpp_uint8_impl::~mlir_aie_cpp_uint8_impl() {}

void mlir_aie_cpp_uint8_impl::forecast(int noutput_items,
                                       gr_vector_int& ninput_items_required)
{
    ninput_items_required[0] = noutput_items;
}

int mlir_aie_cpp_uint8_impl::general_work(int noutput_items,
                                          gr_vector_int& ninput_items,
                                          gr_vector_const_void_star& input_items,
                                          gr_vector_void_star& output_items)
{
    auto in = static_cast<const input_type*>(input_items[0]);
    auto out = static_cast<output_type*>(output_items[0]);

    if(noutput_items < _VECTOR_SIZE)
    {
        return 0;
    }

    // ## AIE code goes below
    for (int i = 0; i < _VECTOR_SIZE; i++) //TODO maybe try memcpy
        _bufInA[i] = in[i];

    memset(_bufOut, 0, _VECTOR_SIZE * sizeof(output_type) + _trace_size); //TODO does this matter? maybe I can remove this line.

    // sync host to device memories
    _bo_instr.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    _bo_inA.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    _bo_out.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    
    // Execute the kernel and wait to finish
    std::cout << "Running Kernel.\n";
    auto run = _kernel(_opcode_run, _bo_instr, _instr_v.size(), _bo_inA, _bo_out);
    run.wait();

    // Sync device to host memories
    _bo_out.sync(XCL_BO_SYNC_BO_FROM_DEVICE);

    for (int i = 0; i < _VECTOR_SIZE; i++) //TODO maybe try memcpy
        out[i] = _bufOut[i];

    // ## Back to GNURadio
    consume_each(_VECTOR_SIZE);
    return _VECTOR_SIZE;
}

} /* namespace mlir_aie */
} /* namespace gr */
