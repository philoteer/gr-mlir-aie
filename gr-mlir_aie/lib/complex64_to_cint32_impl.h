/* -*- c++ -*- */
/*
 * Copyright 2026 gr-mlir_aie author.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_MLIR_AIE_COMPLEX64_TO_CINT32_IMPL_H
#define INCLUDED_MLIR_AIE_COMPLEX64_TO_CINT32_IMPL_H

#include <gnuradio/mlir_aie/complex64_to_cint32.h>

namespace gr {
namespace mlir_aie {

class complex64_to_cint32_impl : public complex64_to_cint32
{
private:
    // Nothing to declare in this block.

public:
    complex64_to_cint32_impl();
    ~complex64_to_cint32_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace mlir_aie
} // namespace gr

#endif /* INCLUDED_MLIR_AIE_COMPLEX64_TO_CINT32_IMPL_H */
