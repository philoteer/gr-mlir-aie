// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include <stdint.h>

#include <aie_api/aie.hpp>

namespace {

constexpr unsigned kFftSize = 64;
constexpr unsigned kVectorSize = 8;
constexpr unsigned kTwiddleShift = 15;
constexpr unsigned kStageShift = 15;

alignas(aie::vector_decl_align) static const cint16 kStage0Tw2[] = {
    {32767, 0}};
alignas(aie::vector_decl_align) static const cint16 kStage0Tw1[] = {
    {32767, 0}};
alignas(aie::vector_decl_align) static const cint16 kStage0Tw3[] = {
    {32767, 0}};

alignas(aie::vector_decl_align) static const cint16 kStage1Tw2[] = {
    {32767, 0}, {23170, -23170}, {0, -32768}, {-23170, -23170}};
alignas(aie::vector_decl_align) static const cint16 kStage1Tw1[] = {
    {32767, 0}, {30273, -12539}, {23170, -23170}, {12539, -30273}};
alignas(aie::vector_decl_align) static const cint16 kStage1Tw3[] = {
    {32767, 0}, {12539, -30273}, {-23170, -23170}, {-30273, 12539}};

alignas(aie::vector_decl_align) static const cint16 kStage2Tw2[] = {
    {32767, 0},       {32138, -6392},   {30273, -12539}, {27245, -18204},
    {23170, -23170},  {18204, -27245},  {12539, -30273}, {6392, -32138},
    {0, -32768},      {-6392, -32138},  {-12539, -30273}, {-18204, -27245},
    {-23170, -23170}, {-27245, -18204}, {-30273, -12539}, {-32138, -6392}};
alignas(aie::vector_decl_align) static const cint16 kStage2Tw1[] = {
    {32767, 0},      {32610, -3211},  {32138, -6392},   {31357, -9512},
    {30273, -12539}, {28898, -15446}, {27245, -18204},  {25330, -20787},
    {23170, -23170}, {20787, -25330}, {18204, -27245},  {15446, -28898},
    {12539, -30273}, {9512, -31357},  {6392, -32138},   {3211, -32610}};
alignas(aie::vector_decl_align) static const cint16 kStage2Tw3[] = {
    {32767, 0},       {31357, -9512},   {27245, -18204},  {20787, -25330},
    {12539, -30273},  {3211, -32610},   {-6392, -32138},  {-15446, -28898},
    {-23170, -23170}, {-28898, -15446}, {-32138, -6392},  {-32610, 3211},
    {-30273, 12539},  {-25330, 20787},  {-18204, 27245},  {-9512, 31357}};

alignas(aie::vector_decl_align) static cint32 scratch[kFftSize];
alignas(aie::vector_decl_align) static cint32 bins[kFftSize];

} // namespace

extern "C" {

void fft64_cint16(const cint16 *__restrict input,
                  cint32 *__restrict output,
                  int32_t count) {
  aie::set_rounding(aie::rounding_mode::positive_inf);
  aie::set_saturation(aie::saturation_mode::saturate);

  for (int32_t frame = 0; frame + kFftSize <= count; frame += kFftSize) {
    aie::fft_dit_r4_stage<16>(input + frame, kStage0Tw2, kStage0Tw1,
                              kStage0Tw3, kFftSize, kTwiddleShift,
                              kStageShift, false, bins);
    aie::fft_dit_r4_stage<4>(bins, kStage1Tw2, kStage1Tw1, kStage1Tw3,
                             kFftSize, kTwiddleShift, kStageShift, false,
                             scratch);
    aie::fft_dit_r4_stage<1>(scratch, kStage2Tw2, kStage2Tw1, kStage2Tw3,
                             kFftSize, kTwiddleShift, kStageShift, false,
                             bins);

    // fftshift: place negative frequencies before non-negative frequencies.
    for (unsigned i = 0; i < kFftSize / 2; i += kVectorSize) {
      aie::store_unaligned_v(output + frame + i,
                   aie::load_v<kVectorSize>(bins + i + kFftSize / 2));
      aie::store_unaligned_v(output + frame + i + kFftSize / 2,
                   aie::load_v<kVectorSize>(bins + i));
    }
  }
}

} // extern "C"
