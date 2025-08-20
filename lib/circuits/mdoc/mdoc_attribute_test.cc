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

#include <string>

#include "circuits/mdoc/mdoc_attribute_ids.h"
#include "util/log.h"
#include "gtest/gtest.h"

namespace proofs {
namespace {

TEST(MdocAttributeTest, MdocAttributeIdsAreSuffixFree) {
  for (const auto& attr : kMdocAttributes) {
    for (const auto& aj : kMdocAttributes) {
      if (aj.identifier.ends_with(attr.identifier) &&
          aj.identifier != attr.identifier) {
        log(INFO, "identifier %s is a suffix of %s\n", aj.identifier.data(),
            attr.identifier.data());
      }
      EXPECT_TRUE(!aj.identifier.ends_with(attr.identifier) ||
                  aj.identifier == attr.identifier);
    }
  }
}

TEST(MdocAttributeTest, DelimiterIsPresent) {
  // Make sure that the delimiter "ier" never occurs in an
  // attribute id. This property means that we can begin parsing for the
  // attribute id by checking for:
  // <delim> <attrid> <cbor-encoding of "elementValue"> <cbor-encoding of value>

  for (const auto& attr : kMdocAttributes) {
    EXPECT_TRUE(attr.identifier.find("ier") == std::string::npos);
    EXPECT_TRUE(attr.identifier.find("elementValue") == std::string::npos);
  }
}

}  // namespace
}  // namespace proofs
#endif
