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

using input_type = unsigned char;
using output_type = unsigned char;
mlir_aie_cpp_uint8::sptr mlir_aie_cpp_uint8::make(const char* path_xclbin,
                                                  const char* path_insts_bin,
                                                  int VECTOR_SIZE)
{
    return gnuradio::make_block_sptr<mlir_aie_cpp_uint8_impl>(
        path_xclbin, path_insts_bin, VECTOR_SIZE);
}


/*
 * The private constructor
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
    
    consume_each(_VECTOR_SIZE);
    return _VECTOR_SIZE;
}

} /* namespace mlir_aie */
} /* namespace gr */
