/* -*- c++ -*- */
/*
 * Copyright 2026 gr-mlir_aie author.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_MLIR_AIE_MLIR_AIE_CPP_80211_MAGSQ_AND_DIV_IMPL_H
#define INCLUDED_MLIR_AIE_MLIR_AIE_CPP_80211_MAGSQ_AND_DIV_IMPL_H

#include <gnuradio/mlir_aie/mlir_aie_cpp_80211_magsq_and_div.h>

namespace gr {
namespace mlir_aie {

class mlir_aie_cpp_80211_magsq_and_div_impl : public mlir_aie_cpp_80211_magsq_and_div
{
private:
    // Nothing to declare in this block.

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
