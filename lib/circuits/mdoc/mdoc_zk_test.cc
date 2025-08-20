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

#include "circuits/mdoc/mdoc_zk.h"

#include <stdio.h>
#include <sys/types.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include "circuits/mdoc/mdoc_examples.h"
#include "circuits/mdoc/mdoc_test_attributes.h"
#include "random/secure_random_engine.h"
#include "util/log.h"
#include "benchmark/benchmark.h"
#include "gtest/gtest.h"

namespace proofs {
namespace {

// Test fixture for MdocZK that handles 1 and 2 attribute circuits.
// This class produces static versions of the 1- and 2- attribute circuits
// and reuses them for all tests.
class MdocZKTest : public testing::Test {
 protected:
  MdocZKTest() { set_log_level(INFO); }

  static void SetUpTestCase() {
    if (circuit1_ == nullptr) {
      EXPECT_EQ(generate_circuit(&kZkSpecs[0], &circuit1_, &circuit_len1_),
                CIRCUIT_GENERATION_SUCCESS);
      EXPECT_EQ(generate_circuit(&kZkSpecs[1], &circuit2_, &circuit_len2_),
                CIRCUIT_GENERATION_SUCCESS);
    }
  }

  static void TearDownTestCase() {
    if (circuit1_ != nullptr) {
      free(circuit1_);
      free(circuit2_);
      circuit1_ = nullptr;
      circuit2_ = nullptr;
    }
  }

  void run_test(const char *test_name, size_t num_attrs,
                const RequestedAttribute *attrs, const MdocTests *test,
                MdocProverErrorCode want_ret = MDOC_PROVER_SUCCESS) {
    uint8_t *circuit = num_attrs == 1 ? circuit1_ : circuit2_;
    size_t circuit_len = num_attrs == 1 ? circuit_len1_ : circuit_len2_;
    const ZkSpecStruct zk_spec = num_attrs == 1 ? kZkSpecs[0] : kZkSpecs[1];
    EXPECT_TRUE(circuit != nullptr);

    uint8_t *zkproof;
    size_t proof_len;

    log(INFO, "========== Test %s", test_name);
    {
      log(INFO, "starting prover");
      MdocProverErrorCode ret = run_mdoc_prover(
          circuit, circuit_len, test->mdoc, test->mdoc_size,
          test->pkx.as_pointer, test->pky.as_pointer, test->transcript,
          test->transcript_size, attrs, num_attrs, (const char *)test->now,
          &zkproof, &proof_len, &zk_spec);
      EXPECT_EQ(ret, want_ret);
    }

    if (want_ret == MDOC_PROVER_SUCCESS) {
      log(INFO, "starting verifier");
      MdocVerifierErrorCode ret = run_mdoc_verifier(
          circuit, circuit_len, test->pkx.as_pointer, test->pky.as_pointer,
          test->transcript, test->transcript_size, attrs, num_attrs,
          (const char *)test->now, zkproof, proof_len, test->doc_type,
          &zk_spec);
      EXPECT_EQ(ret, MDOC_VERIFIER_SUCCESS);
      free(zkproof);
    }
  }

  // The two circuits are generated once and reused for all tests.
  static uint8_t *circuit1_, *circuit2_;
  static size_t circuit_len1_, circuit_len2_;
};

uint8_t *MdocZKTest::circuit1_ = nullptr;
uint8_t *MdocZKTest::circuit2_ = nullptr;
size_t MdocZKTest::circuit_len1_ = 0;
size_t MdocZKTest::circuit_len2_ = 0;

typedef struct {
  const char *test_name;
  RequestedAttribute claims[1];
  const MdocTests *mdoc;
} Claims;

typedef struct {
  const char *test_name;
  RequestedAttribute claims[2];
  const MdocTests *mdoc;
} TwoClaims;

TEST_F(MdocZKTest, one_claim) {
  const Claims tests[] = {
      {"+18-mdoc[0]", {test::age_over_18}, &mdoc_tests[0]},
      {"+18-mdoc[1]", {test::age_over_18}, &mdoc_tests[1]},
      {"+18-mdoc[2]", {test::age_over_18}, &mdoc_tests[2]},
      {"familyname_mustermann-mdoc[3]",
       {test::familyname_mustermann},
       &mdoc_tests[3]},
      {"birthdate_1971_09_01-mdoc[3]",
       {test::birthdate_1971_09_01},
       &mdoc_tests[3]},
      {"height_175-mdoc[3]", {test::height_175}, &mdoc_tests[3]},
      // Test Google IDPass which uses a different docType.
      {"birthdate_1998_09_04-idpass-mdoc[4]",
       {test::birthdate_1998_09_04},
       &mdoc_tests[4]},
      // Website explainer example.
      {"age_over_18-website-mdoc[5]", {test::age_over_18}, &mdoc_tests[5]},
      // Large mdoc from 2025-06-10.
      {"not_over_18-large-mdoc[6]", {test::not_over_18}, &mdoc_tests[6]}};

  for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
    run_test(tests[i].test_name, 1, tests[i].claims, tests[i].mdoc);
  }
}

TEST_F(MdocZKTest, long_attribute) {
  uint8_t *zkproof;
  size_t proof_len;
  RequestedAttribute attrs[1] = {test::age_over_18};
  auto test = &mdoc_tests[0];
  {
    log(INFO, "starting prover");
    MdocProverErrorCode ret = run_mdoc_prover(
        circuit1_, circuit_len1_, test->mdoc, test->mdoc_size,
        test->pkx.as_pointer, test->pky.as_pointer, test->transcript,
        test->transcript_size, attrs, 1, (const char *)test->now, &zkproof,
        &proof_len, &kZkSpecs[0]);
    EXPECT_EQ(ret, MDOC_PROVER_SUCCESS);
  }

  // Attr is too long.
  RequestedAttribute long_attr[1] = {
      {{'a', 'g', 'e', '_', 'o', 'v', 'e', 'r', '_', '1', '8',
        '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
        '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'},
       {0xf5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       32,
       64,
       kString}};

  MdocVerifierErrorCode ret = run_mdoc_verifier(
      circuit1_, circuit_len1_, test->pkx.as_pointer, test->pky.as_pointer,
      test->transcript, test->transcript_size, long_attr, 1,
      (const char *)test->now, zkproof, proof_len, test->doc_type,
      &kZkSpecs[0]);
  EXPECT_EQ(ret, MDOC_VERIFIER_GENERAL_FAILURE);
  free(zkproof);
}

TEST_F(MdocZKTest, two_claims) {
  const TwoClaims two_tests[] = {
      {
          "18+,familyname_mustermann-mdoc[3]",
          {
              test::age_over_18,
              test::familyname_mustermann,
          },
          &mdoc_tests[3],
      },
      {
          "18+,birthdate_1971_09_01-mdoc[3]",
          {
              test::age_over_18,
              test::birthdate_1971_09_01,
          },
          &mdoc_tests[3],
      },
      {
          "height175,issue_date_2024-03-15-mdoc[3]",
          {
              test::height_175,
              test::issue_date_2024_03_15,
          },
          &mdoc_tests[3],
      },
  };

  for (size_t i = 0; i < sizeof(two_tests) / sizeof(two_tests[0]); ++i) {
    run_test(two_tests[i].test_name, 2, two_tests[i].claims, two_tests[i].mdoc);
  }
}

TEST_F(MdocZKTest, wrong_witness) {
  const Claims fail_tests[] = {
      {"fail-not_over_18-mdoc[0]", {test::not_over_18}, &mdoc_tests[0]},
      {"fail-not_over_18-mdoc[1]", {test::not_over_18}, &mdoc_tests[1]},
      {"fail-not_over_18-mdoc[2]", {test::not_over_18}, &mdoc_tests[2]},
      {
          "fail-birthdate_1971_09_01-mdoc[3]",
          {{{'b', 'i', 'r', 't', 'h', '_', 'd', 'a', 't', 'e'},
            {'0', '9', '7', '1', '-', '0', '9', '-', '0', '1'},
            10,
            10,
            kDate}},
          &mdoc_tests[3],
      },
      {
          "fail-birthdate_1871_09_01-mdoc[3]",
          {{{'b', 'i', 'r', 't', 'h', '_', 'd', 'a', 't', 'e'},
            {'1', '8', '7', '1', '-', '0', '9', '-', '0', '1'},
            10,
            10,
            kDate}},
          &mdoc_tests[3],
      },
      {
          "fail-birthdate_1971_09_01-mdoc[3]",
          {{{'b', 'i', 'r', 't', 'h', '_', 'd', 'a', 't', 'e'},
            {0xD9, 0x03, 0xEC, 0x6A, '1', '9', '7', '1', '-', '0', '9', '-',
             '0', '1', '0'},
            10,
            15,
            kDate}},
          &mdoc_tests[3],
      },
  };

  for (size_t i = 0; i < sizeof(fail_tests) / sizeof(fail_tests[0]); ++i) {
    run_test(fail_tests[i].test_name, 1, fail_tests[i].claims,
             fail_tests[i].mdoc, MDOC_PROVER_GENERAL_FAILURE);
  }
}

TEST_F(MdocZKTest, bad_arguments) {
  constexpr int num_attrs = 1;
  const ZkSpecStruct &zk_spec_1 = kZkSpecs[0];
  RequestedAttribute attrs[num_attrs] = {
      test::age_over_18,
  };
  uint8_t tr[100] = {0};
  uint8_t zkproof[30000] = {0};
  uint8_t circuit[60000] = {0};
  uint8_t mdoc[60000] = {0};
  const char *pk = "0x15";
  const char *pk2 = "bad_pk";
  const char *now = "2023-11-02T09:00:00Z";
  size_t proof_len;
  // ZStd encoding for "hello".
  uint8_t bad_circuit[50001] = {0x28, 0xb5, 0x2f, 0xfd, 0x20, 0x05, 0x29,
                                0x00, 0x00, 0x68, 0x65, 0x6c, 0x6c, 0x6f};

  // Invalid arguments to generate_circuit.
  size_t circuit_len;
  EXPECT_EQ(generate_circuit(nullptr, (uint8_t **)&circuit, &circuit_len),
            CIRCUIT_GENERATION_NULL_INPUT);
  EXPECT_EQ(generate_circuit(&zk_spec_1, nullptr, &circuit_len),
            CIRCUIT_GENERATION_NULL_INPUT);
  EXPECT_EQ(generate_circuit(&zk_spec_1, (uint8_t **)&circuit, nullptr),
            CIRCUIT_GENERATION_NULL_INPUT);

  // Basic prover tests that pass in a null ptr.
  EXPECT_EQ(run_mdoc_prover(nullptr, sizeof(circuit), mdoc, sizeof(mdoc), pk,
                            pk, tr, sizeof(tr), attrs, num_attrs, now,
                            (uint8_t **)&zkproof, &proof_len, &zk_spec_1),
            MDOC_PROVER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_prover(circuit, sizeof(circuit), nullptr, sizeof(mdoc), pk,
                            pk, tr, sizeof(tr), attrs, num_attrs, now,
                            (uint8_t **)&zkproof, &proof_len, &zk_spec_1),
            MDOC_PROVER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_prover(circuit, sizeof(circuit), mdoc, sizeof(mdoc),
                            nullptr, pk, tr, sizeof(tr), attrs, num_attrs, now,
                            (uint8_t **)&zkproof, &proof_len, &zk_spec_1),
            MDOC_PROVER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_prover(circuit, sizeof(circuit), mdoc, sizeof(mdoc), pk,
                            nullptr, tr, sizeof(tr), attrs, num_attrs, now,
                            (uint8_t **)&zkproof, &proof_len, &zk_spec_1),
            MDOC_PROVER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_prover(circuit, sizeof(circuit), mdoc, sizeof(mdoc), pk,
                            pk, nullptr, sizeof(tr), attrs, num_attrs, now,
                            (uint8_t **)&zkproof, &proof_len, &zk_spec_1),
            MDOC_PROVER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_prover(circuit, sizeof(circuit), mdoc, sizeof(mdoc), pk,
                            pk, tr, sizeof(tr), nullptr, num_attrs, now,
                            (uint8_t **)&zkproof, &proof_len, &zk_spec_1),
            MDOC_PROVER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_prover(circuit, sizeof(circuit), mdoc, sizeof(mdoc), pk,
                            pk, tr, sizeof(tr), attrs, num_attrs, nullptr,
                            (uint8_t **)&zkproof, &proof_len, &zk_spec_1),
            MDOC_PROVER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_prover(circuit, sizeof(circuit), mdoc, sizeof(mdoc), pk,
                            pk, tr, sizeof(tr), attrs, num_attrs, now, nullptr,
                            &proof_len, &zk_spec_1),
            MDOC_PROVER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_prover(circuit, sizeof(circuit), mdoc, sizeof(mdoc), pk,
                            pk, tr, sizeof(tr), attrs, num_attrs, now,
                            (uint8_t **)&zkproof, nullptr, &zk_spec_1),
            MDOC_PROVER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_prover(circuit, sizeof(circuit), mdoc, sizeof(mdoc), pk,
                            pk, tr, sizeof(tr), attrs, num_attrs, now,
                            (uint8_t **)&zkproof, &proof_len, nullptr),
            MDOC_PROVER_NULL_INPUT);

  // Invalid pk.
  EXPECT_EQ(run_mdoc_prover(circuit, sizeof(circuit), mdoc, sizeof(mdoc), pk2,
                            pk, tr, sizeof(tr), attrs, num_attrs, now,
                            (uint8_t **)&zkproof, &proof_len, &zk_spec_1),
            MDOC_PROVER_INVALID_INPUT);

  // Invalid circuit.
  EXPECT_EQ(run_mdoc_prover(circuit, sizeof(circuit), mdoc, sizeof(mdoc), pk,
                            pk, tr, sizeof(tr), attrs, num_attrs, now,
                            (uint8_t **)&zkproof, &proof_len, &zk_spec_1),
            MDOC_PROVER_CIRCUIT_PARSING_FAILURE);
  EXPECT_EQ(
      run_mdoc_prover(bad_circuit, sizeof(bad_circuit), mdoc, sizeof(mdoc), pk,
                      pk, tr, sizeof(tr), attrs, num_attrs, now,
                      (uint8_t **)&zkproof, &proof_len, &zk_spec_1),
      MDOC_PROVER_CIRCUIT_PARSING_FAILURE);

  // Basic verifier tests that pass in a null ptr.
  // Broken circuit.
  EXPECT_EQ(run_mdoc_verifier(nullptr, sizeof(circuit), pk, pk, tr, sizeof(tr),
                              attrs, num_attrs, now, zkproof, sizeof(zkproof),
                              kDefaultDocType, &zk_spec_1),
            MDOC_VERIFIER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_verifier(circuit, 49999, pk, pk, tr, sizeof(tr), attrs,
                              num_attrs, now, zkproof, sizeof(zkproof),
                              kDefaultDocType, &zk_spec_1),
            MDOC_VERIFIER_ARGUMENTS_TOO_SMALL);
  EXPECT_EQ(run_mdoc_verifier(bad_circuit, sizeof(bad_circuit), pk, pk, tr,
                              sizeof(tr), attrs, num_attrs, now, zkproof,
                              sizeof(zkproof), kDefaultDocType, &zk_spec_1),
            MDOC_VERIFIER_CIRCUIT_PARSING_FAILURE);

  // Broken pk.
  EXPECT_EQ(run_mdoc_verifier(circuit, sizeof(circuit), nullptr, pk, tr,
                              sizeof(tr), attrs, num_attrs, now, zkproof,
                              sizeof(zkproof), kDefaultDocType, &zk_spec_1),
            MDOC_VERIFIER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_verifier(circuit, sizeof(circuit), pk, nullptr, tr,
                              sizeof(tr), attrs, num_attrs, now, zkproof,
                              sizeof(zkproof), kDefaultDocType, &zk_spec_1),
            MDOC_VERIFIER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_verifier(circuit, sizeof(circuit), pk, pk2, tr, sizeof(tr),
                              attrs, num_attrs, now, zkproof, sizeof(zkproof),
                              kDefaultDocType, &zk_spec_1),
            MDOC_VERIFIER_INVALID_INPUT);

  // Broken transcript.
  EXPECT_EQ(run_mdoc_verifier(circuit, sizeof(circuit), pk, pk, nullptr,
                              sizeof(tr), attrs, num_attrs, now, zkproof,
                              sizeof(zkproof), kDefaultDocType, &zk_spec_1),
            MDOC_VERIFIER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_verifier(circuit, sizeof(circuit), pk, pk, tr, 0, attrs,
                              num_attrs, now, zkproof, sizeof(zkproof),
                              kDefaultDocType, &zk_spec_1),
            MDOC_VERIFIER_ARGUMENTS_TOO_SMALL);
  // Broken attrs.
  EXPECT_EQ(run_mdoc_verifier(circuit, sizeof(circuit), pk, pk, tr, sizeof(tr),
                              nullptr, num_attrs, now, zkproof, sizeof(zkproof),
                              kDefaultDocType, &zk_spec_1),
            MDOC_VERIFIER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_verifier(circuit, sizeof(circuit), pk, pk, tr, sizeof(tr),
                              attrs, 0, now, zkproof, sizeof(zkproof),
                              kDefaultDocType, &zk_spec_1),
            MDOC_VERIFIER_ARGUMENTS_TOO_SMALL);

  // Broken now.
  EXPECT_EQ(run_mdoc_verifier(circuit, sizeof(circuit), pk, pk, tr, sizeof(tr),
                              attrs, num_attrs, nullptr, zkproof,
                              sizeof(zkproof), kDefaultDocType, &zk_spec_1),
            MDOC_VERIFIER_NULL_INPUT);

  // Broken zkproof.
  EXPECT_EQ(run_mdoc_verifier(circuit, sizeof(circuit), pk, pk, tr, sizeof(tr),
                              attrs, num_attrs, now, nullptr, sizeof(zkproof),
                              kDefaultDocType, &zk_spec_1),
            MDOC_VERIFIER_NULL_INPUT);
  EXPECT_EQ(run_mdoc_verifier(circuit1_, circuit_len1_, pk, pk, tr, sizeof(tr),
                              attrs, num_attrs, now, zkproof, 100,
                              kDefaultDocType, &zk_spec_1),
            MDOC_VERIFIER_ARGUMENTS_TOO_SMALL);

  uint8_t id[32];
  EXPECT_EQ(circuit_id(nullptr, circuit1_, circuit_len1_, &zk_spec_1), 0);
  EXPECT_EQ(circuit_id(id, nullptr, 0, &zk_spec_1), 0);
  EXPECT_EQ(circuit_id(id, circuit1_, circuit_len1_, nullptr), 0);
  EXPECT_EQ(circuit_id(id, circuit1_, 10, &zk_spec_1), 0);
  EXPECT_EQ(circuit_id(id, circuit1_, circuit_len1_ - 8, &zk_spec_1), 0);
}

TEST_F(MdocZKTest, attr_mismatch) {
  uint8_t *zkproof;
  size_t proof_len;
  constexpr int num_attrs = 2;
  const ZkSpecStruct &zk_spec_2 = kZkSpecs[1];
  RequestedAttribute attrs[num_attrs] = {test::age_over_18, test::age_over_18};
  const struct MdocTests *test = &mdoc_tests[0];

  {
    MdocProverErrorCode ret = run_mdoc_prover(
        circuit2_, circuit_len2_, test->mdoc, test->mdoc_size,
        test->pkx.as_pointer, test->pky.as_pointer, test->transcript,
        test->transcript_size, attrs, num_attrs, (const char *)test->now,
        &zkproof, &proof_len, &zk_spec_2);
    EXPECT_EQ(ret, MDOC_PROVER_SUCCESS);
  }
  {
    MdocVerifierErrorCode ret = run_mdoc_verifier(
        circuit2_, circuit_len2_, test->pkx.as_pointer, test->pky.as_pointer,
        test->transcript, test->transcript_size, attrs, num_attrs - 1,
        (const char *)test->now, zkproof, proof_len, kDefaultDocType,
        &zk_spec_2);
    EXPECT_EQ(ret, MDOC_VERIFIER_ATTRIBUTE_NUMBER_MISMATCH);
  }
  free(zkproof);
}

TEST_F(MdocZKTest, bad_proofs) {
  set_log_level(ERROR);
  constexpr int num_attrs = 1;
  const ZkSpecStruct &zk_spec_1 = kZkSpecs[0];
  RequestedAttribute attrs[num_attrs] = {test::age_over_18};
  const struct MdocTests *test = &mdoc_tests[0];

  constexpr size_t kMaxProofLen = 100000;
  uint8_t zkproof[kMaxProofLen];
  SecureRandomEngine rng;
  rng.bytes(zkproof, sizeof(zkproof));
  for (size_t proof_len = 0; proof_len < kMaxProofLen; proof_len += 1000) {
    MdocVerifierErrorCode ret = run_mdoc_verifier(
        circuit1_, circuit_len1_, test->pkx.as_pointer, test->pky.as_pointer,
        test->transcript, test->transcript_size, attrs, num_attrs,
        (const char *)test->now, zkproof, proof_len, kDefaultDocType,
        &zk_spec_1);
    EXPECT_NE(ret, MDOC_VERIFIER_SUCCESS);
  }
}

TEST(CircuitGenerationTest, attempt_to_generate_old_circuit) {
  set_log_level(ERROR);
  constexpr int num_attrs = 1;

  // Find the smallest version of the circuit for the given number of
  // attributes.
  const ZkSpecStruct *old_zk_spec = nullptr;
  int num_circuits = 0;
  for (int i = 0; i < kNumZkSpecs; ++i) {
    if (kZkSpecs[i].num_attributes == num_attrs) {
      num_circuits++;
      if (old_zk_spec == nullptr ||
          kZkSpecs[i].version < old_zk_spec->version) {
        old_zk_spec = &kZkSpecs[i];
      }
    }
  }

  EXPECT_GE(num_circuits, 1);
  if (num_circuits == 1) {
    return;  // No old circuit to test against, it's OK to skip this test.
  }

  static uint8_t *circuit = nullptr;
  static size_t circuit_len;
  EXPECT_EQ(generate_circuit(old_zk_spec, &circuit, &circuit_len),
            CIRCUIT_GENERATION_INVALID_ZK_SPEC_VERSION);
}

static const Claims benchmark_claim = {
    "benchmark",
    {test::age_over_18},
    &mdoc_tests[0],
};

void BM_MdocProver(benchmark::State &state) {
  set_log_level(ERROR);

  const ZkSpecStruct &zk_spec_1 = kZkSpecs[0];
  size_t circuit_len;
  uint8_t *circuit;
  EXPECT_EQ(generate_circuit(&zk_spec_1, &circuit, &circuit_len),
            CIRCUIT_GENERATION_SUCCESS);

  const RequestedAttribute *attrs = benchmark_claim.claims;
  const MdocTests *test = benchmark_claim.mdoc;
  size_t num_attrs = 1;
  const ZkSpecStruct zk_spec = kZkSpecs[0];

  for (auto _ : state) {
    uint8_t *zkproof;
    size_t proof_len;

    MdocProverErrorCode ret = run_mdoc_prover(
        circuit, circuit_len, test->mdoc, test->mdoc_size, test->pkx.as_pointer,
        test->pky.as_pointer, test->transcript, test->transcript_size, attrs,
        num_attrs, (const char *)test->now, &zkproof, &proof_len, &zk_spec);
    EXPECT_EQ(ret, MDOC_PROVER_SUCCESS);
    free(zkproof);
  }
}

BENCHMARK(BM_MdocProver);

void BM_MdocVerifier(benchmark::State &state) {
  set_log_level(ERROR);

  const ZkSpecStruct &zk_spec_1 = kZkSpecs[0];
  size_t circuit_len;
  uint8_t *circuit;
  EXPECT_EQ(generate_circuit(&zk_spec_1, &circuit, &circuit_len),
            CIRCUIT_GENERATION_SUCCESS);

  const RequestedAttribute *attrs = benchmark_claim.claims;
  const MdocTests *test = benchmark_claim.mdoc;
  size_t num_attrs = 1;
  const ZkSpecStruct zk_spec = kZkSpecs[0];

  uint8_t *zkproof;
  size_t proof_len;

  MdocProverErrorCode retp = run_mdoc_prover(
      circuit, circuit_len, test->mdoc, test->mdoc_size, test->pkx.as_pointer,
      test->pky.as_pointer, test->transcript, test->transcript_size, attrs,
      num_attrs, (const char *)test->now, &zkproof, &proof_len, &zk_spec);
  EXPECT_EQ(retp, MDOC_PROVER_SUCCESS);

  for (auto _ : state) {
    MdocVerifierErrorCode retv = run_mdoc_verifier(
        circuit, circuit_len, test->pkx.as_pointer, test->pky.as_pointer,
        test->transcript, test->transcript_size, attrs, num_attrs,
        (const char *)test->now, zkproof, proof_len, test->doc_type, &zk_spec);
    EXPECT_EQ(retv, MDOC_VERIFIER_SUCCESS);
  }

  free(zkproof);
}

BENCHMARK(BM_MdocVerifier);

}  // namespace
}  // namespace proofs
#endif

