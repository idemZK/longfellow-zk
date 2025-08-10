# Copyright 2025 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

add_library(util OBJECT log.cc crypto.cc)
target_link_libraries(util $ENV{IDEMZK_DIR}/openssl-wasm/precompiled/lib/libcrypto.a $ENV{IDEMZK_DIR}/openssl-wasm/precompiled/lib/libssl.a $ENV{IDEMZK_DIR}/zstd-wasm/lib/libzstd.a )

proofs_add_tests(ceildiv_test)

