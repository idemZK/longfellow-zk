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

#include <stdlib.h>

#include <cstdint>
#include <cstdio>
#include <vector>

#include "algebra/blas.h"
#include "algebra/convolution.h"
#include "algebra/fp.h"
#include "algebra/reed_solomon.h"
#include "gf2k/gf2_128.h"
#include "gf2k/lch14_reed_solomon.h"
#include "ligero/ligero_param.h"
#include "ligero/ligero_prover.h"
#include "ligero/ligero_verifier.h"
#include "random/secure_random_engine.h"
#include "random/transcript.h"
#include "util/log.h"
#include "gtest/gtest.h"

namespace proofs {
namespace {

template <class Field, class ReedSolomonFactory>
void ligero_test(const ReedSolomonFactory &rs_factory, const Field &F) {
  using Elt = typename Field::Elt;
  set_log_level(INFO);
  static const constexpr size_t nw = 300000;
  static const constexpr size_t nq = 30000;
  static const constexpr size_t nreq = 189;
  static const constexpr size_t nl = 7;
  LigeroParam<Field> param(nw, nq, /*rateinv=*/4, nreq);
  log(INFO, "%zd %zd %zd %zd %zd %zd\n", param.r, param.w, param.block,
      param.block_enc, param.nrow, param.nqtriples);

  std::vector<Elt> W(nw);
  std::vector<Elt> A(nw);
  for (size_t i = 0; i < nw; ++i) {
    W[i] = F.of_scalar_field(random());
    A[i] = F.of_scalar_field(random());
  }

  // Set up semi-random quadratic constraints.  For simplicity
  // of testing, say that the first NQ odd-index witnesses are
  // the product of two even-index witnesses
  std::vector<LigeroQuadraticConstraint> lqc(nq);
  for (size_t i = 0; i < nq; ++i) {
    lqc[i].z = 2 * i + 1;
    lqc[i].x = 2 * ((random() % nw) / 2);
    lqc[i].y = 2 * ((random() % nw) / 2);
    W[lqc[i].z] = F.mulf(W[lqc[i].x], W[lqc[i].y]);
  }

  // Generate NL linear constraints.
  std::vector<LigeroLinearConstraint<Field>> llterm;
  std::vector<Elt> b(nl);
  Blas<Field>::clear(nl, &b[0], 1, F);
  for (size_t w = 0; w < nw; ++w) {
    LigeroLinearConstraint<Field> term = {
        w % nl,  // c
        w,       // w
        A[w],    // k
    };
    llterm.push_back(term);
    F.add(b[term.c], F.mulf(W[w], term.k));
  }

  LigeroCommitment<Field> commitment;
  LigeroProof<Field> proof(&param);

  const LigeroHash hash_of_llterm{0xde, 0xad, 0xbe, 0xef};

  {
    log(INFO, "start prover");
    SecureRandomEngine rng;
    LigeroProver<Field, ReedSolomonFactory> prover(param);
    Transcript ts((uint8_t *)"test", 4);
    prover.commit(commitment, ts, &W[0], /*subfield_boundary=*/0, &lqc[0],
                  rs_factory, rng, F);
    prover.prove(proof, ts, nl, llterm.size(), &llterm[0], hash_of_llterm,
                 &lqc[0], rs_factory, F);
    log(INFO, "end prover");
  }

  {
    log(INFO, "start verifier");
    Transcript ts((uint8_t *)"test", 4);
    LigeroVerifier<Field, ReedSolomonFactory>::receive_commitment(commitment,
                                                                  ts);
    const char *why = "";
    bool ok = LigeroVerifier<Field, ReedSolomonFactory>::verify(
        &why, param, commitment, proof, ts, nl, llterm.size(), &llterm[0],
        hash_of_llterm, &b[0], &lqc[0], rs_factory, F);
    EXPECT_TRUE(ok);
    log(INFO, "end verifier");
  }
}

TEST(Ligero, Fp) {
  using Field = Fp<1>;
  using ConvolutionFactory = FFTConvolutionFactory<Field>;
  using ReedSolomonFactory = ReedSolomonFactory<Field, ConvolutionFactory>;

  const Field F("18446744069414584321");
  const ConvolutionFactory conv_factory(F, F.of_scalar(1753635133440165772ull),
                                        1ull << 32);
  const ReedSolomonFactory rs_factory(conv_factory, F);

  ligero_test(rs_factory, F);
}

TEST(Ligero, GF2_128) {
  using Field = GF2_128<>;
  const Field F;
  using ReedSolomonFactory = LCH14ReedSolomonFactory<Field>;
  const ReedSolomonFactory rs_factory(F);

  ligero_test(rs_factory, F);
}

}  // namespace
}  // namespace proofs
#endif
