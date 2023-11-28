#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>
#include <iostream>

namespace llvm {

template <typename Derived>
class Demangler {
  public:
    Derived &getDerived();
    bool shouldStripUnderscore();
    bool shouldAttemptType();
    std::string demangle(const std::string &Mangled);
    void SplitStringDelims(StringRef Source,
                           SmallVectorImpl<std::pair<StringRef, StringRef>> &OutFragments);
    void demangleLine(raw_ostream &OS, StringRef Mangled, bool Split);
    static bool IsLegalItaniumChar(char C);
};

template <typename Derived>
Derived &Demangler<Derived>::getDerived() {
  return *static_cast<Derived *>(this);
}

template <typename Derived>
bool Demangler<Derived>::shouldStripUnderscore() {
  // If none of them are set, use the default value for platform.
  // macho has symbols prefix with "_" so strip by default.
  return Triple(sys::getProcessTriple()).isOSBinFormatMachO();
}

template <typename Derived>
bool Demangler<Derived>::shouldAttemptType() {
  return false;
}

template <typename Derived>
std::string Demangler<Derived>::demangle(const std::string &Mangled) {
  int Status;
  std::string Prefix;

  const char *DecoratedStr = Mangled.c_str();
  if (getDerived().shouldStripUnderscore())
    if (DecoratedStr[0] == '_')
      ++DecoratedStr;
  size_t DecoratedLength = strlen(DecoratedStr);

  char *Undecorated = nullptr;

  if (getDerived().shouldAttemptType() ||
      ((DecoratedLength >= 2 && strncmp(DecoratedStr, "_Z", 2) == 0) ||
       (DecoratedLength >= 4 && strncmp(DecoratedStr, "___Z", 4) == 0)))
    Undecorated = itaniumDemangle(DecoratedStr, nullptr, nullptr, &Status);

  if (!Undecorated &&
      (DecoratedLength > 6 && strncmp(DecoratedStr, "__imp_", 6) == 0)) {
    Prefix = "import thunk for ";
    Undecorated = itaniumDemangle(DecoratedStr + 6, nullptr, nullptr, &Status);
  }

  std::string Result(Undecorated ? Prefix + Undecorated : Mangled);
  free(Undecorated);
  return Result;
}

// Split 'Source' on any character that fails to pass 'IsLegalChar'.  The
// returned vector consists of pairs where 'first' is the delimited word, and
// 'second' are the delimiters following that word.
template <typename Derived>
void Demangler<Derived>::SplitStringDelims(
    StringRef Source,
    SmallVectorImpl<std::pair<StringRef, StringRef>> &OutFragments) {
  // The beginning of the input string.
  const auto Head = Source.begin();

  // Obtain any leading delimiters.
  auto Start = std::find_if(Head, Source.end(), getDerived().IsLegalItaniumChar);
  if (Start != Head)
    OutFragments.push_back({"", Source.slice(0, Start - Head)});

  // Capture each word and the delimiters following that word.
  while (Start != Source.end()) {
    Start = std::find_if(Start, Source.end(), getDerived().IsLegalItaniumChar);
    auto End = std::find_if_not(Start, Source.end(), getDerived().IsLegalItaniumChar);
    auto DEnd = std::find_if(End, Source.end(), getDerived().IsLegalItaniumChar);
    OutFragments.push_back({Source.slice(Start - Head, End - Head),
                            Source.slice(End - Head, DEnd - Head)});
    Start = DEnd;
  }
}

// If 'Split' is true, then 'Mangled' is broken into individual words and each
// word is demangled.  Otherwise, the entire string is treated as a single
// mangled item.  The result is output to 'OS'.
template <typename Derived>
void Demangler<Derived>::demangleLine(raw_ostream &OS, StringRef Mangled, bool Split) {
  std::string Result;
  if (Split) {
    SmallVector<std::pair<StringRef, StringRef>, 16> Words;
    getDerived().SplitStringDelims(Mangled, Words);
    for (const auto &Word : Words)
      Result += getDerived().demangle(std::string(Word.first)) + Word.second.str();
  } else
    Result = getDerived().demangle(std::string(Mangled));
  OS << Result << '\n';
  OS.flush();
}

// This returns true if 'C' is a character that can show up in an
// Itanium-mangled string.
template <typename Derived>
bool Demangler<Derived>::IsLegalItaniumChar(char C) {
  // Itanium CXX ABI [External Names]p5.1.1:
  // '$' and '.' in mangled names are reserved for private implementations.
  return isalnum(C) || C == '.' || C == '$' || C == '_';
}

} // end namespace llvm