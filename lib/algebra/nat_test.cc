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
//#include "algebra/nat.h"
//
//#include <array>
//#include <cstdint>
//#include <cstdlib>
//#include <string>
//
//#include "gtest/gtest.h"
//
//namespace proofs {
//namespace {
//TEST(Nat, Lt) {
//  constexpr size_t W = 4;
//  for (size_t i = 0; i < 16; ++i) {
//    for (size_t j = 0; j < 16; ++j) {
//      if (i < j) {
//        EXPECT_TRUE(Nat<W>(i) < Nat<W>(j));
//      } else {
//        EXPECT_FALSE(Nat<W>(i) < Nat<W>(j));
//      }
//    }
//  }
//}
//
//void oneTestInvModB64(uint64_t i) {
//  uint64_t j = inv_mod_b(i);
//  EXPECT_EQ(i * j, (uint64_t)1);
//}
//
//TEST(Nat, InvModB64) {
//  for (uint64_t i = 1; i < (uint64_t)1000000; i += 2) {
//    oneTestInvModB64(i);
//    oneTestInvModB64(i * i);
//    oneTestInvModB64(-i);
//    oneTestInvModB64(-i * i);
//    oneTestInvModB64(1 + 2 * (uint64_t)std::rand());
//  }
//  oneTestInvModB64(4891460686036598785ull);
//  oneTestInvModB64(4403968944856104961ull);
//}
//
//void oneTestInvModB32(uint32_t i) {
//  uint32_t j = inv_mod_b(i);
//  EXPECT_EQ(i * j, (uint32_t)1);
//}
//
//TEST(Nat, InvModB32) {
//  for (uint32_t i = 1; i < (uint32_t)1000000; i += 2) {
//    oneTestInvModB32(i);
//    oneTestInvModB32(i * i);
//    oneTestInvModB32(-i);
//    oneTestInvModB32(-i * i);
//    oneTestInvModB32(1 + 2 * (uint32_t)std::rand());
//  }
//  oneTestInvModB32(836598785u);
//  oneTestInvModB32(856104961u);
//}
//
//TEST(Nat, Parsing) {
//  uint8_t buf[32] = {
//      0x97, 0xc3, 0xbc, 0x78, 0x8f, 0x15, 0x79, 0x9c, 0xfe, 0x11, 0x10,
//      0x32, 0x9f, 0xd1, 0xba, 0x4f, 0xe9, 0xf4, 0xb1, 0x03, 0xa0, 0x03,
//      0x4d, 0x56, 0xc4, 0xa9, 0x45, 0xf6, 0x4d, 0x9c, 0x78, 0x6d,
//  };
//
//  Nat<4> a = Nat<4>::of_bytes(buf);
//
//  EXPECT_EQ(a.bit(0), 1u);
//  EXPECT_EQ(a.bit(8), 1u);
//  EXPECT_EQ(a.bit(255), 0u);
//  EXPECT_EQ(a.bit(254), 1u);
//
//  std::array<uint64_t, 4> a64 = {0x9c79158f78bcc397ull, 0x4fbad19f321011feull,
//                                 0x564d03a003b1f4e9ull, 0x6d789c4df645a9c4ull};
//  Nat<4> a2 = Nat<4>(a64);
//
//  EXPECT_EQ(a, a2);
//
//  uint8_t buf1[32];
//  a.to_bytes(buf1);
//  for (size_t i = 0; i < 32; ++i) {
//    EXPECT_EQ(buf[i], buf1[i]);
//  }
//}
//
//TEST(Nat, BadStrings) {
//  const char* bad_strings[] = {
//      "123456789abcdef",
//      "0x123J",
//      "wiejoifj",
//      "123QWEOQWU",
//      "000QIWDO",
//      "0xx21312",
//      "115792089237316195423570985008687907853269984665640564039457584007913129"
//      "639937",
//      "463168356949264781694283940034751631413079938662562256157830336031652518"
//      "559743559744",
//      "0x40000000000000000001230000000000000000000000000000000000000000000",
//      "000000000000000000000000000000000000000000000000000000000000000000000000"
//      "000000000000"};
//
//  for (auto s : bad_strings) {
//    EXPECT_FALSE(Nat<4>::of_untrusted_string(s).has_value());
//  }
//}
//
//TEST(Nat, BadDigits) {
//  std::string ok = "0123456789abcdefABCDEF";
//  for (uint8_t i = 0; i < (uint8_t)256; ++i) {
//    if (ok.find((char)i) == std::string::npos) {  // bad char
//      EXPECT_DEATH(digit((char)i), "bad char");
//    }
//  }
//}
//
//}  // namespace
//}  // namespace proofs
