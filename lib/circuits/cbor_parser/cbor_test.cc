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
//#include "circuits/cbor_parser/cbor.h"
//
//#include <stddef.h>
//
//#include <cstdint>
//#include <vector>
//
//#include "algebra/fp.h"
//#include "circuits/cbor_parser/cbor_constants.h"
//#include "circuits/cbor_parser/cbor_pluck.h"
//#include "circuits/compiler/circuit_dump.h"
//#include "circuits/compiler/compiler.h"
//#include "circuits/logic/compiler_backend.h"
//#include "circuits/logic/evaluation_backend.h"
//#include "circuits/logic/logic.h"
//#include "util/log.h"
//#include "gtest/gtest.h"
//
//namespace proofs {
//namespace {
//
//using Field = Fp<1>;
//const Field F("18446744073709551557");
//
//using CompilerBackend = CompilerBackend<Field>;
//using LogicCircuit = Logic<Field, CompilerBackend>;
//
//using EvalBackend = EvaluationBackend<Field>;
//using Logic = Logic<Field, EvalBackend>;
//
//TEST(CBOR, DecodeOneV8) {
//  const EvalBackend ebk(F);
//  const Logic L(&ebk, F);
//  using Cbor = Cbor<Logic>;
//  const Cbor CBOR(L);
//  for (size_t type = 0; type < 8; ++type) {
//    for (size_t count = 0; count < 32; ++count) {
//      size_t v = (type << 5) | count;
//      auto ds = CBOR.decode_one_v8(L.vbit<8>(v));
//
//      bool atomp = (type == 0) || (type == 1);
//      bool stringp = (type == 2) || (type == 3);
//      bool arrayp = (type == 4);
//      bool mapp = (type == 5);
//      bool itemsp = arrayp || mapp;
//      bool tagp = (type == 6);
//      bool specialp = (type == 7);
//      bool simple_specialp = specialp && (20 <= count && count < 24);
//      bool count0_23 = (count < 24);
//      bool count24 = (count == 24);
//
//      bool length_plus_next_v8 = false;
//      bool count_is_next_v8 = false;
//      bool invalid = false;
//      size_t length = ~0;  // bogus
//      size_t count_as_field_element = count;
//      if (atomp || tagp) {
//        if (count0_23) {
//          length = 1;
//        } else if (count24) {
//          length = 2;
//        } else {
//          invalid = true;
//        }
//      } else if (itemsp) {
//        if (count0_23) {
//          length = 1;
//        } else if (count24) {
//          length = 2;
//          count_is_next_v8 = true;
//        } else {
//          invalid = true;
//        }
//      } else if (stringp) {
//        if (count0_23) {
//          length = 1 + count;
//        } else if (count24) {
//          length = 2;
//          length_plus_next_v8 = true;
//        } else {
//          invalid = true;
//        }
//      } else if (simple_specialp) {
//        length = 1;
//      } else {
//        invalid = true;
//      }
//
//      EXPECT_EQ(L.eval(ds.itemsp), L.eval(L.bit(itemsp)));
//      EXPECT_EQ(L.eval(ds.arrayp), L.eval(L.bit(arrayp)));
//      EXPECT_EQ(L.eval(ds.mapp), L.eval(L.bit(mapp)));
//      EXPECT_EQ(L.eval(ds.stringp), L.eval(L.bit(stringp)));
//      EXPECT_EQ(L.eval(ds.tagp), L.eval(L.bit(tagp)));
//      EXPECT_EQ(L.eval(ds.specialp), L.eval(L.bit(specialp)));
//      EXPECT_EQ(L.eval(ds.simple_specialp), L.eval(L.bit(simple_specialp)));
//      EXPECT_EQ(L.eval(ds.invalid), L.eval(L.bit(invalid)));
//
//      EXPECT_EQ(L.eval(ds.count0_23), L.eval(L.bit(count0_23)));
//      EXPECT_EQ(L.eval(ds.count24), L.eval(L.bit(count24)));
//      EXPECT_EQ(L.eval(ds.length_plus_next_v8),
//                L.eval(L.bit(length_plus_next_v8)));
//      EXPECT_EQ(L.eval(ds.count_is_next_v8), L.eval(L.bit(count_is_next_v8)));
//      if (!invalid) {
//        // the length is don't care unless valid
//        EXPECT_EQ(ds.length, L.konst(length));
//      }
//
//      EXPECT_EQ(ds.count_as_field_element, L.konst(count_as_field_element));
//      EXPECT_EQ(ds.as_field_element, L.konst(v));
//
//      // This module is expected to set these bits to 0.
//      // A later module fixes them up.
//      EXPECT_EQ(L.eval(ds.header), L.eval(L.bit(0)));
//    }
//  }
//}
//
//// encoder of input bytes
//static inline uint8_t X(uint8_t type, uint8_t count) {
//  return (type << 5) | count;
//}
//
//const struct {
//  uint8_t v, len;
//} testcase[] = {
//    // a small atom, constant 23
//    {X(0, 23), 1},
//
//    // a larger atom, constant 33
//    {X(0, 24), 2},
//    {33},
//
//    // another large atom
//    {X(0, 24), 2},
//    {34},
//
//    // a short string
//    {X(2, 3), 4},
//    {'f'},
//    {'o'},
//    {'o'},
//
//    // a long string
//    {X(2, 24), 5},  // header + next byte + string
//    {/*length of the string*/ 3},
//    {0xff},
//    {25},
//    {31},
//
//    // another small atom
//    {X(0, 22), 1},
//
//    // a long string
//    {X(2, 24), 6},  // header + next byte + string
//    {/*length of the string*/ 4},
//    {'q'},
//    {'u'},
//    {'u'},
//    {'x'},
//};
//constexpr size_t ntestcase = sizeof(testcase) / sizeof(testcase[0]);
//
//TEST(CBOR, VerifyDecode) {
//  const EvalBackend ebk(F);
//  const Logic L(&ebk, F);
//  using Cbor = Cbor<Logic>;
//  const Cbor CBOR(L);
//
//  constexpr size_t n = ntestcase;
//  std::vector<Cbor::v8> in(n);
//  std::vector<Cbor::position_witness> pw(n);
//  Cbor::global_witness gw;
//
//  size_t slen = 1;
//  auto prod = L.elt(1);
//  for (size_t i = 0; i < n; ++i) {
//    in[i] = L.vbit<8>(testcase[i].v);
//
//    size_t slenm1 = slen - 1;
//    size_t slen_next;
//    if (slenm1 == 0) {
//      slen_next = testcase[i].len;
//    } else {
//      if (i > 0) {
//        prod = L.mulf(prod, L.elt(slenm1));
//      }
//      slen_next = slenm1;
//    }
//    pw[i].encoded_sel_header =
//        L.konst(cbor_plucker_point<Field, CborConstants::kNCounters>()(
//            (slenm1 == 0), 0, F));
//    slen = slen_next;
//  }
//
//  std::vector<Cbor::decode> ds(n);
//  gw.invprod_decode = L.konst(L.invertf(prod));
//  CBOR.decode_and_assert_decode(n, ds.data(), in.data(), pw.data(), gw);
//}
//
//TEST(CBOR, VerifyParseSize) {
//  set_log_level(INFO);
//
//  size_t sizes[] = {247, 503, 1079, 1591, 2231, 2551};
//
//  for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
//    size_t n = sizes[i];
//    QuadCircuit<Field> Q(F);
//    const CompilerBackend cbk(&Q);
//    const LogicCircuit LC(&cbk, F);
//    using CborC = Cbor<LogicCircuit>;
//    const CborC CBORC(LC);
//
//    std::vector<CborC::v8> inC(n);
//    std::vector<CborC::position_witness> pwC(n);
//    CborC::global_witness gwC;
//
//    for (size_t j = 0; j < n; ++j) {
//      inC[j] = LC.vinput<8>();
//      pwC[j].encoded_sel_header = Q.input();
//    }
//    gwC.invprod_decode = Q.input();
//    gwC.cc0 = Q.input();
//    gwC.invprod_parse = Q.input();
//
//    std::vector<CborC::decode> dsC(n);
//    std::vector<CborC::parse_output> psC(n);
//    CBORC.decode_and_assert_decode_and_parse(n, dsC.data(), psC.data(),
//                                             inC.data(), pwC.data(), gwC);
//
//    // Fake parser output, otherwise the compiler eliminates important wires.
//    constexpr size_t kNCounters = CborC::kNCounters;
//    size_t nout = 0;
//    for (size_t j = 0; j < n; ++j) {
//      for (size_t l = 0; l < kNCounters; ++l) {
//        Q.output(psC[j].c[l], nout++);
//      }
//    }
//
//    auto CIRCUIT = Q.mkcircuit(/*nc=*/1);
//    dump_info<Field>("decode_and_assert_decode_and_parse", n, Q);
//  }
//}
//}  // namespace
//}  // namespace proofs
