//===-- llvm-c++filt.cpp --------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../demangler.h"
#include "llvm/Support/CommandLine.h"
#include <exception>

using namespace llvm;

#define EXIT_INVALID_ARGS -2
#define EXIT_INVALID_SIZE -1

class CXXFilt : public Demangler<CXXFilt> {
  private:
    using super = Demangler<CXXFilt>;
};

extern "C" void* __realloc(void *ptr, size_t size) {
  return realloc(ptr, size);
}

extern "C" void __free(void *ptr) {
  free(ptr);
}

extern "C" size_t __itanium_demangle(const char *in, char *out, size_t size) {
  if (!in || !out || !size)
    return 0;
  CXXFilt cxxfilt;
  std::string demangle = cxxfilt.demangle(std::string(in));
  if (demangle.size() >= size)
    return 0;
  memset(out, 0, size);
  strcpy(out, demangle.c_str());
  return demangle.size();
}