#include "../install/include/mdoc_zk.h"

int main() {
    size_t num_attrs = 1;
    static uint8_t *circuit1_, *circuit2_;
    static size_t circuit_len1_, circuit_len2_;

    uint8_t *circuit = num_attrs == 1 ? circuit1_ : circuit2_;
    size_t circuit_len = num_attrs == 1 ? circuit_len1_ : circuit_len2_;
    const ZkSpecStruct zk_spec = num_attrs == 1 ? kZkSpecs[0] : kZkSpecs[1];

    uint8_t *zkproof;
    size_t proof_len;

    const RequestedAttribute *attrs = NULL;

    // MdocProverErrorCode ret = run_mdoc_prover(
    //     circuit, circuit_len, NULL, 0,
    //     NULL, NULL, NULL,
    //     0, attrs, num_attrs, NULL,
    //     &zkproof, &proof_len, &zk_spec);
}
