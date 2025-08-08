#include <cassert>
#include <cstdlib>
#include "../install/include/mdoc_zk.h"

// Example data
#include "circuits/mdoc/mdoc_examples.h"
#include "circuits/mdoc/mdoc_test_attributes.h"

using namespace proofs;

typedef struct {
  const char *test_name;
  RequestedAttribute claims[1];
  const MdocTests *mdoc;
} Claims;

int main() {
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

    size_t num_attrs = 1;
    static uint8_t *circuit1_;
    static size_t circuit_len1_;

    generate_circuit(&kZkSpecs[0], &circuit1_, &circuit_len1_);

    uint8_t *circuit = circuit1_;
    size_t circuit_len = circuit_len1_;
    const ZkSpecStruct zk_spec = kZkSpecs[0];

    uint8_t *zkproof;
    size_t proof_len;

    const MdocTests *test = tests[0].mdoc;
    const RequestedAttribute *attrs = tests[0].claims;

    MdocProverErrorCode retp = run_mdoc_prover(
        circuit, circuit_len, test->mdoc, test->mdoc_size,
        test->pkx.as_pointer, test->pky.as_pointer, test->transcript,
        test->transcript_size, attrs, num_attrs, (const char *)test->now,
        &zkproof, &proof_len, &zk_spec);

    assert(retp == MDOC_PROVER_SUCCESS);

    MdocVerifierErrorCode retv = run_mdoc_verifier(
        circuit, circuit_len, test->pkx.as_pointer, test->pky.as_pointer,
        test->transcript, test->transcript_size, attrs, num_attrs,
        (const char *)test->now, zkproof, proof_len, test->doc_type,
        &zk_spec);

    assert(retv == MDOC_VERIFIER_SUCCESS);
    free(zkproof);
}
