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
//#include "circuits/cbor_parser/scan.h"
//
//#include <stddef.h>
//#include <stdint.h>
//#include <stdlib.h>
//
//#include <vector>
//
//#include "algebra/fp.h"
//#include "circuits/logic/evaluation_backend.h"
//#include "circuits/logic/logic.h"
//#include "gtest/gtest.h"
//
//namespace proofs {
//namespace {
//using Field = Fp<1>;
//const Field F("18446744073709551557");
//
//using EvalBackend = EvaluationBackend<Field>;
//using Logic = Logic<Field, EvalBackend>;
//
//static void one_add(size_t n, const uint64_t A[/*n*/], const uint64_t ds[/*n*/],
//                    const bool S[/*n*/]) {
//  const EvalBackend ebk(F);
//  const Logic L(&ebk, F);
//  Scan<Logic> SC(L);
//
//  // reference implementation
//  std::vector<Logic::EltW> BB(n);
//  std::vector<Logic::BitW> SS(n);
//  std::vector<Logic::EltW> AA(n);
//  std::vector<Logic::EltW> ddss(n);
//
//  for (size_t i = 0; i < n; ++i) {
//    AA[i] = L.konst(A[i]);
//    ddss[i] = L.konst(ds[i]);
//    SS[i] = L.bit(S[i]);
//  }
//
//  auto s = L.konst(0);
//  for (size_t i = 0; i < n; ++i) {
//    if (S[i]) {
//      s = AA[i];
//    } else {
//      s = L.add(&s, ddss[i]);
//    }
//    BB[i] = s;
//  }
//
//  std::vector<Logic::EltW> B(n);
//  SC.add(n, B.data(), SS.data(), AA.data(), ddss.data());
//
//  for (size_t i = 0; i < n; ++i) {
//    EXPECT_EQ(B[i].elt(), BB[i].elt());
//  }
//}
//
//TEST(Scan, Add) {
//  constexpr size_t n = 15;
//  uint64_t A[n], ds[n];
//  bool S[n];
//
//  for (uint64_t ls = 0; ls < (1 << n); ++ls) {
//    for (size_t k = 0; k < n; ++k) {
//      S[k] = (ls >> k) & 0x1u;
//      A[k] = random();
//      ds[k] = random();
//    }
//
//    for (size_t n1 = 0; n1 < n; ++n1) {
//      one_add(n1, A, ds, S);
//    }
//  }
//}
//
//static void one_add(size_t n, const uint64_t ds[/*n*/]) {
//  const EvalBackend ebk(F);
//  const Logic L(&ebk, F);
//  Scan<Logic> SC(L);
//
//  // reference implementation
//  std::vector<Logic::EltW> BB(n);
//  std::vector<Logic::EltW> ddss(n);
//
//  for (size_t i = 0; i < n; ++i) {
//    ddss[i] = L.konst(ds[i]);
//  }
//
//  auto s = L.konst(0);
//  for (size_t i = 0; i < n; ++i) {
//    s = L.add(&s, ddss[i]);
//    BB[i] = s;
//  }
//
//  std::vector<Logic::EltW> B(n);
//  SC.add(n, B.data(), ddss.data());
//
//  for (size_t i = 0; i < n; ++i) {
//    EXPECT_EQ(B[i].elt(), BB[i].elt());
//  }
//}
//
//TEST(Scan, UnsegmentedAdd) {
//  constexpr size_t n = 100;
//  uint64_t ds[n];
//
//  for (size_t k = 0; k < n; ++k) {
//    ds[k] = random();
//  }
//
//  for (size_t n1 = 0; n1 < n; ++n1) {
//    one_add(n1, ds);
//  }
//}
//}  // namespace
//}  // namespace proofs
