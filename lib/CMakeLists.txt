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

cmake_minimum_required(VERSION 3.13)
project(proofs)
set(CMAKE_CXX_STANDARD 17)
include(CMake/proofs.cmake)
enable_testing()
if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
  link_directories("/opt/homebrew/lib")
  include_directories("/opt/homebrew/include")
endif()

# add compiler flags for all known ISA's
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" ARCHITECTURE)

if(ARCHITECTURE MATCHES "x86_64")
    set(ARCH_FLAGS -mpclmul)
elseif(ARCHITECTURE MATCHES "i386")
    set(ARCH_FLAGS -msse2 -mpclmul)
elseif(ARCHITECTURE MATCHES "aarch64")
    set(ARCH_FLAGS -march=armv8-a -march=armv8-a+crypto)
elseif(ARCHITECTURE MATCHES "armv7l")
    set(ARCH_FLAGS -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=hard)
elseif(ARCHITECTURE MATCHES "arm64")
    # Mac Arm
    set(ARCH_FLAGS -mcpu=apple-m1 -march=armv8-a+crypto)
elseif(ARCHITECTURE MATCHES "wasm32")
    include_directories("$ENV{IDEMZK_DIR}/openssl-wasm/precompiled/include")
    include_directories("$ENV{IDEMZK_DIR}/zstd-wasm/zstd/lib")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    set(ARCH_FLAGS -D__wasm__ )
else()
    set(ARCH_FLAGS "")
endif()

if(ARCH_FLAGS)
    add_compile_options( "$<$<COMPILE_LANGUAGE:CXX>:${ARCH_FLAGS}>")
    message(STATUS "Compiler flags added for architecture ${ARCHITECTURE}: ${ARCH_FLAGS}")
else()
     message(STATUS "Architecture ${ARCHITECTURE} not recognized, no specific compiler flags added.")
endif()


include_directories(${proofs_SOURCE_DIR})

# add_subdirectory(testing)
add_subdirectory(util)
add_subdirectory(algebra)
add_subdirectory(arrays)
add_subdirectory(merkle)
add_subdirectory(proto)
add_subdirectory(random)
add_subdirectory(sumcheck)
add_subdirectory(gf2k)
add_subdirectory(cbor)
add_subdirectory(ec)
add_subdirectory(zk)
add_subdirectory(circuits/anoncred)
add_subdirectory(circuits/base64)
add_subdirectory(circuits/cbor_parser)
add_subdirectory(circuits/compiler)
add_subdirectory(circuits/ecdsa)
add_subdirectory(circuits/jwt)
add_subdirectory(circuits/logic)
add_subdirectory(circuits/mac)
add_subdirectory(circuits/mdoc)
add_subdirectory(circuits/sha)
add_subdirectory(circuits/sha3)
