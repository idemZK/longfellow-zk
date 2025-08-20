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

#include "circuits/sha3/sha3_circuit.h"

#include <stddef.h>

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "arrays/dense.h"
#include "circuits/compiler/circuit_dump.h"
#include "circuits/compiler/compiler.h"
#include "circuits/logic/compiler_backend.h"
#include "circuits/logic/evaluation_backend.h"
#include "circuits/logic/logic.h"
#include "circuits/sha3/sha3_reference.h"
#include "gf2k/gf2_128.h"
#include "random/transcript.h"
#include "sumcheck/circuit.h"
#include "sumcheck/prover.h"
#include "sumcheck/verifier.h"
#include "util/log.h"
#include "util/panic.h"
#include "gtest/gtest.h"

namespace proofs {
namespace {
using Field = GF2_128<>;
const Field F;
typedef CompilerBackend<Field> CompilerBackend;
typedef Logic<Field, CompilerBackend> LogicCircuit;
typedef LogicCircuit::BitW bitWC;
typedef typename LogicCircuit::template bitvec<64> v64;

typedef EvaluationBackend<Field> EvalBackend;
typedef Logic<Field, EvalBackend> Logic;
typedef Logic::BitW bitW;

std::unique_ptr<Circuit<Field>> mk_keccak_circuit(size_t nc) {
  set_log_level(INFO);
  QuadCircuit<Field> Q(F);
  const CompilerBackend cbk(&Q);
  const LogicCircuit LC(&cbk, F);
  Sha3Circuit<LogicCircuit> SHAC(LC);

  struct awrap {
    v64 a[5][5];
  };

  auto aw = std::make_unique<awrap>();
  for (size_t x = 0; x < 5; ++x) {
    for (size_t y = 0; y < 5; ++y) {
      aw->a[x][y] = LC.vinput<64>();
    }
  }

  SHAC.keccak_f_1600(aw->a);
  for (size_t x = 0; x < 5; ++x) {
    for (size_t y = 0; y < 5; ++y) {
      LC.voutput(aw->a[x][y], 64 * (y + 5 * x));
    }
  }

  auto CIRCUIT = Q.mkcircuit(nc);
  dump_info("sha3", Q);

  return CIRCUIT;
}

TEST(SHA3_Circuit, Keccak_F_1600) {
  constexpr size_t nc = 1;
  const EvalBackend ebk(F);
  const Logic L(&ebk, F);

  auto CIRCUIT = mk_keccak_circuit(nc);

  uint64_t st[5][5];
  auto W = std::make_unique<Dense<Field>>(nc, /*constant one*/ 1 + 64 * 5 * 5);
  W->v_[0] = F.one();
  for (size_t x = 0; x < 5; ++x) {
    for (size_t y = 0; y < 5; ++y) {
      st[x][y] = 3 * x + 1000 * y;
      for (size_t z = 0; z < 64; ++z) {
        W->v_[1 + z + 64 * (y + 5 * x)] =
            L.eval(L.bit((st[x][y] >> z) & 1)).elt();
      }
    }
  }

  Sha3Reference::keccak_f_1600_DEBUG_ONLY(st);
  Prover<Field>::inputs pin;
  Prover<Field> prover(F);
  auto V = prover.eval_circuit(&pin, CIRCUIT.get(), W->clone(), F);
  for (size_t x = 0; x < 5; ++x) {
    for (size_t y = 0; y < 5; ++y) {
      for (size_t z = 0; z < 64; ++z) {
        EXPECT_EQ(V->v_[z + 64 * (y + 5 * x)],
                  L.eval(L.bit((st[x][y] >> z) & 1)).elt());
      }
    }
  }
}

TEST(SHA3_Circuit, Keccak_F_1600_Copies) {
  constexpr size_t nc = 23;
  const EvalBackend ebk(F);
  const Logic L(&ebk, F);

  auto CIRCUIT = mk_keccak_circuit(nc);

  struct State {
    uint64_t s[5][5];
  };
  std::vector<State> st(nc);
  auto W = std::make_unique<Dense<Field>>(nc, /*constant one*/ 1 + 64 * 5 * 5);
  for (size_t c = 0; c < nc; ++c) {
    W->v_[0 * nc + c] = F.one();
    for (size_t x = 0; x < 5; ++x) {
      for (size_t y = 0; y < 5; ++y) {
        st[c].s[x][y] = 3 * x + 1000 * y + 1000000 * c;
        for (size_t z = 0; z < 64; ++z) {
          W->v_[(1 + z + 64 * (y + 5 * x)) * nc + c] =
              L.eval(L.bit((st[c].s[x][y] >> z) & 1)).elt();
        }
      }
    }
  }

  {
    Prover<Field>::inputs pin;
    Prover<Field> prover(F);
    auto V = prover.eval_circuit(&pin, CIRCUIT.get(), W->clone(), F);

    for (size_t c = 0; c < nc; ++c) {
      Sha3Reference::keccak_f_1600_DEBUG_ONLY(st[c].s);
      for (size_t x = 0; x < 5; ++x) {
        for (size_t y = 0; y < 5; ++y) {
          for (size_t z = 0; z < 64; ++z) {
            EXPECT_EQ(V->v_[(z + 64 * (y + 5 * x)) * nc + c],
                      L.eval(L.bit((st[c].s[x][y] >> z) & 1)).elt());
          }
        }
      }
    }
  }

  {
    Prover<Field> prover(F);
    Prover<Field>::inputs pin;
    auto V = prover.eval_circuit(&pin, CIRCUIT.get(), W->clone(), F);

    Transcript tsp((uint8_t *)"test", 4);
    Proof<Field> proof(CIRCUIT->nl);
    prover.prove(&proof, nullptr, CIRCUIT.get(), pin, tsp);

    const char *why = "ok";
    Transcript tsv((uint8_t *)"test", 4);
    check(Verifier<Field>::verify(&why, CIRCUIT.get(), &proof, std::move(V),
                                  std::move(W), tsv, F),
          why);
  }
}

}  // namespace
}  // namespace proofs
#endif
