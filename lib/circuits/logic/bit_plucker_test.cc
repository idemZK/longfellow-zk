#ifndef __wasm__
// Copyright 2025 Google LLC.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "circuits/logic/bit_plucker.h"

#include <stddef.h>

#include "algebra/fp.h"
#include "circuits/compiler/circuit_dump.h"
#include "circuits/compiler/compiler.h"
#include "circuits/logic/bit_plucker_constants.h"
#include "circuits/logic/bit_plucker_encoder.h"
#include "circuits/logic/compiler_backend.h"
#include "circuits/logic/evaluation_backend.h"
#include "circuits/logic/logic.h"
#include "gtest/gtest.h"

namespace proofs {
namespace {

using Field = Fp<1>;
using Elt = typename Field::Elt;
using CompilerBackend = CompilerBackend<Field>;
using LogicCircuit = Logic<Field, CompilerBackend>;
using EltWC = LogicCircuit::EltW;
using EvalBackend = EvaluationBackend<Field>;
using Logic = Logic<Field, EvalBackend>;
using EltW = Logic::EltW;

TEST(BitPlucker, Pluck) {
  const Field F("18446744073709551557");
  const EvalBackend ebk(F);
  const Logic L(&ebk, F);
  constexpr size_t LOGN = 5;
  constexpr size_t N = 1 << LOGN;
  const BitPluckerEncoder<Field, LOGN> PE(F);
  const BitPlucker<Logic, LOGN> P(L);

  for (size_t i = 0; i < N; ++i) {
    auto enc = PE.encode(i);
    auto got = P.pluck(L.konst(enc));
    for (size_t k = 0; k < LOGN; ++k) {
      EXPECT_EQ(L.eval(got[k]), L.konst((i >> k) & 1));
    }
  }
}

template <size_t LOGN>
static void pluck_size() {
  const Field F("18446744073709551557");
  const EvalBackend ebk(F);
  const Logic L(&ebk, F);
  QuadCircuit<Field> Q(F);
  const CompilerBackend cbk(&Q);
  const LogicCircuit LC(&cbk, F);
  const BitPlucker<LogicCircuit, LOGN> PC(LC);

  auto eC = Q.input();
  auto r = PC.pluck(eC);
  for (size_t k = 0; k < LOGN; ++k) {
    Q.output(LC.eval(r[k]), k);
  }
  auto CIRCUIT = Q.mkcircuit(/*nc=*/1);
  dump_info("pluck", LOGN, Q);
}

TEST(BitPlucker, PluckSize) {
  pluck_size<1>();
  pluck_size<2>();
  pluck_size<3>();
  pluck_size<4>();
  pluck_size<5>();
  pluck_size<6>();
  pluck_size<7>();
  pluck_size<8>();
}

TEST(BitPlucker, EltMuxer) {
  const Field F("257");
  const EvalBackend ebk(F);
  const Logic L(&ebk, F);
  using EltW = typename Logic::EltW;
  const EltW zero = L.konst(0);
  const EltW one = L.konst(1);

  EltW arr_z[] = {zero, one, one, one, one, one, one, one};
  EltW arr_e[] = {zero, one, zero, one, zero, one, zero, one};
  EltW arr_r[] = {zero, zero, one, one, zero, zero, one, one};
  EltW arr_s[] = {zero, zero, zero, zero, one, one, one, one};

  const EltMuxer<Logic, 8> em_z(L, arr_z);
  const EltMuxer<Logic, 8> em_e(L, arr_e);
  const EltMuxer<Logic, 8> em_r(L, arr_r);
  const EltMuxer<Logic, 8> em_s(L, arr_s);

  for (size_t i = 0; i < 8; ++i) {
    auto enc = bit_plucker_point<Field, 8>()(i, F);

    EltW range = em_z.mux(L.konst(enc));
    L.assert_eq(&range, arr_z[i]);

    range = em_e.mux(L.konst(enc));
    L.assert_eq(&range, arr_e[i]);

    range = em_r.mux(L.konst(enc));
    L.assert_eq(&range, arr_r[i]);

    range = em_s.mux(L.konst(enc));
    L.assert_eq(&range, arr_s[i]);
  }
}

// Test use of the EltMuxer machinery to test whether a smaller muxer input
// is in range. In this case, we want to test whether the muxed input is in
// {0,1,2,3,4,5,6,7}. We want to ensure that there are no false positives and
// thus the test iterates over the entire field.
TEST(BitPlucker, EltMuxer9) {
  const Field F("257");
  const EvalBackend ebk(F);
  const Logic L(&ebk, F);
  using EltW = typename Logic::EltW;
  const EltW zero = L.konst(0);
  const EltW one = L.konst(1);

  EltW arr_v[] = {zero, zero, zero, zero, zero, zero, zero, zero, one};
  const EltMuxer<Logic, 9, 8> em2(L, arr_v);
  for (size_t i = 0; i < 128; ++i) {
    auto enc = bit_plucker_point<Field, 8>()(i, F);
    EltW range = em2.mux(L.konst(enc));
    if (i < 9) {
      L.assert_eq(&range, arr_v[i]);
    } else {
      auto ee = range.elt();
      EXPECT_NE(ee, F.zero());
    }
  }
}

}  // namespace
}  // namespace proofs
#endif
