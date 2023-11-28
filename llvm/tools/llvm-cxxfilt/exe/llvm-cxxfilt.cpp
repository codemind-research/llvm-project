//===-- llvm-c++filt.cpp --------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../demangler.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

enum Style {
  Auto,  ///< auto-detect mangling
  GNU,   ///< GNU
  Lucid, ///< Lucid compiler (lcc)
  ARM,
  HP,    ///< HP compiler (xCC)
  EDG,   ///< EDG compiler
  GNUv3, ///< GNU C++ v3 ABI
  Java,  ///< Java (gcj)
  GNAT   ///< ADA compiler (gnat)
};
static cl::opt<Style>
    Format("format", cl::desc("decoration style"),
           cl::values(clEnumValN(Auto, "auto", "auto-detect style"),
                      clEnumValN(GNU, "gnu", "GNU (itanium) style")),
           cl::init(Auto));
static cl::alias FormatShort("s", cl::desc("alias for --format"),
                             cl::aliasopt(Format));

static cl::opt<bool> StripUnderscore("strip-underscore",
                                     cl::desc("strip the leading underscore"),
                                     cl::init(false));
static cl::alias StripUnderscoreShort("_",
                                      cl::desc("alias for --strip-underscore"),
                                      cl::aliasopt(StripUnderscore));
static cl::opt<bool>
    NoStripUnderscore("no-strip-underscore",
                      cl::desc("do not strip the leading underscore"),
                      cl::init(false));
static cl::alias
    NoStripUnderscoreShort("n", cl::desc("alias for --no-strip-underscore"),
                           cl::aliasopt(NoStripUnderscore));

static cl::opt<bool>
    Types("types",
          cl::desc("attempt to demangle types as well as function names"),
          cl::init(false));
static cl::alias TypesShort("t", cl::desc("alias for --types"),
                            cl::aliasopt(Types));

static cl::list<std::string>
Decorated(cl::Positional, cl::desc("<mangled>"), cl::ZeroOrMore);

static cl::extrahelp
    HelpResponse("\nPass @FILE as argument to read options from FILE.\n");

class CXXFilt : public Demangler<CXXFilt> {
  private:
    using super = Demangler<CXXFilt>;
  public:
    bool shouldStripUnderscore();
    bool shouldAttemptType();
};

bool CXXFilt::shouldStripUnderscore() {
  if (StripUnderscore)
    return true;
  if (NoStripUnderscore)
    return false;
  // If none of them are set, use the default value for platform.
  // macho has symbols prefix with "_" so strip by default.
  return super::shouldStripUnderscore();
}

bool CXXFilt::shouldAttemptType() {
  return Types;
}

int main(int argc, char **argv) {
  InitLLVM X(argc, argv);
  CXXFilt cxxfilt;

  cl::ParseCommandLineOptions(argc, argv, "llvm symbol undecoration tool\n");

  if (Decorated.empty())
    for (std::string Mangled; std::getline(std::cin, Mangled);)
      cxxfilt.demangleLine(llvm::outs(), Mangled, true);
  else
    for (const auto &Symbol : Decorated)
      cxxfilt.demangleLine(llvm::outs(), Symbol, false);

  return EXIT_SUCCESS;
}