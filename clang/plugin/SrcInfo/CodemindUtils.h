#ifndef CODEMIND_UTILS
#define CODEMIND_UTILS

#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/TemplateBase.h"
#include "clang/AST/Type.h"

#include <string>
#include <vector>

namespace codemind_utils {
  using namespace std;
  using namespace llvm;
  using namespace clang;

  string getQualifiedTypeString(QualType qt, bool showTmpl = true);
  string getQualifiedNameString(const NamedDecl *decl, bool showTmpl = true);

  template<typename Arg>
  void writeTo(raw_fd_ostream &os, Arg arg) {
    os << arg;
  }

  template<typename Arg, typename... Args>
  void writeTo(raw_fd_ostream &os, Arg arg, Args... args) {
    os << arg;
    writeTo(os, args...);
  }

  template<typename Type>
  string VectorToString(vector<Type> vec, function<string (Type)> func, string sep) {
    string result = "";
    for (unsigned i = 0; i < vec.size(); i++)
      result += ((i > 0) ? sep : "") + func(vec[i]);
    return result;
  }

  template<typename Type>
  string VectorToString(vector<Type> vec, string sep) {
    return VectorToString<Type>(vec, [](Type element){ return to_string(element); }, sep);
  }
}

#endif