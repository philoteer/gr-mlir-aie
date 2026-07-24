/* -*- c++ -*- */
/*
 * Copyright 2026 gr-mlir_aie author.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "complex64_to_cint16_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace mlir_aie {

#pragma message("set the following appropriately and remove this warning")
using input_type = float;
#pragma message("set the following appropriately and remove this warning")
using output_type = float;
complex64_to_cint16::sptr complex64_to_cint16::make()
{
    return gnuradio::make_block_sptr<complex64_to_cint16_impl>();
}


/*
 * The private constructor
 */
complex64_to_cint16_impl::complex64_to_cint16_impl()
    : gr::sync_block("complex64_to_cint16",
                     gr::io_signature::make(
                         1 /* min inputs */, 1 /* max inputs */, sizeof(input_type)),
                     gr::io_signature::make(
                         1 /* min outputs */, 1 /*max outputs */, sizeof(output_type)))
{
}

/*
 * Our virtual destructor.
 */
complex64_to_cint16_impl::~complex64_to_cint16_impl() {}

int complex64_to_cint16_impl::work(int noutput_items,
                                   gr_vector_const_void_star& input_items,
                                   gr_vector_void_star& output_items)
{
    auto in = static_cast<const input_type*>(input_items[0]);
    auto out = static_cast<output_type*>(output_items[0]);

#pragma message("Implement the signal processing in your block and remove this warning")
    // Do <+signal processing+>

    // Tell runtime system how many output items we produced.
    return noutput_items;
}

} /* namespace mlir_aie */
} /* namespace gr */
