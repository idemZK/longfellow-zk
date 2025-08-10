//// Copyright 2025 Google LLC.
////
//// Licensed under the Apache License, Version 2.0 (the "License");
//// you may not use this file except in compliance with the License.
//// You may obtain a copy of the License at
////
////     http://www.apache.org/licenses/LICENSE-2.0
////
//// Unless required by applicable law or agreed to in writing, software
//// distributed under the License is distributed on an "AS IS" BASIS,
//// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//// See the License for the specific language governing permissions and
//// limitations under the License.
//
//
//#include "algebra/rfft.h"
//
//#include <stddef.h>
//
//#include <cstdint>
//#include <vector>
//
//#include "algebra/fft.h"
//#include "algebra/fp2.h"
//#include "algebra/fp_p256.h"
//#include "gtest/gtest.h"
//
//namespace proofs {
//namespace {
//TEST(RFFTTest, Simple) {
//  using BaseField = Fp256<>;
//  using BaseElt = BaseField::Elt;
//  using ExtField = Fp2<BaseField>;
//  using ExtElt = ExtField::Elt;
//
//  const BaseField F0;        // base field
//  const ExtField F_ext(F0);  // p^2 field extension
//
//  ExtElt omega = F_ext.of_string(
//      "112649224146410281873500457609690258373018840430489408729223714171582664"
//      "680802",
//      "317040948518153410669569855215889129699039744181079354462206130544166376"
//      "41043");
//  uint64_t omega_order = 1ull << 31;
//
//  ExtElt one = F_ext.mulf(omega, F_ext.conjf(omega));
//  EXPECT_EQ(one, F_ext.one());
//
//  for (size_t iter = 0; iter < 2; ++iter) {
//    // Everything must work for both omega and conj(omega).
//    // (The test would fail, e.g., if RFFT hardcodes that
//    // omega^(n/4) = I or -I somewhere.)
//    F_ext.conj(omega);
//
//    for (size_t n = 1; n < 1024; n *= 2) {
//      std::vector<BaseElt> AR0(n);
//      std::vector<BaseElt> AR1(n);
//      std::vector<ExtElt> AC(n);
//
//      // Arbitrary coefficients in base field.  Keep three copies.
//      for (size_t i = 0; i < n; ++i) {
//        AR0[i] = F0.of_scalar(i * i * i + (i & 0xF) + (i ^ (i << 2)));
//        AR1[i] = AR0[i];
//        AC[i] = ExtElt{AR0[i]};
//      }
//
//      // compare RFFT against FFT
//      FFT<ExtField>::fftb(&AC[0], n, omega, omega_order, F_ext);
//      RFFT<ExtField>::r2hc(&AR0[0], n, omega, omega_order, F_ext);
//
//      for (size_t i = 0; i < n; ++i) {
//        if (i + i <= n) {
//          EXPECT_EQ(AR0[i], AC[i].re);
//        } else {
//          EXPECT_EQ(AR0[i], AC[i].im);
//        }
//      }
//
//      // invert and compare against AR1
//      RFFT<ExtField>::hc2r(&AR0[0], n, omega, omega_order, F_ext);
//      BaseElt scale = F0.of_scalar(n);
//      for (size_t i = 0; i < n; ++i) {
//        EXPECT_EQ(AR0[i], F0.mulf(scale, AR1[i]));
//      }
//    }
//  }
//}
//
//}  // namespace
//}  // namespace proofs
