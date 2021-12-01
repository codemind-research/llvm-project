#ifndef CODEMIND_UTILS
#define CODEMIND_UTILS

#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/TemplateBase.h"
#include "clang/AST/Type.h"

#include <exception>
#include <set>
#include <string>

namespace codemind_utils {
  using namespace std;
  using namespace llvm;
  using namespace clang;

  void printCallStack(raw_fd_ostream &os);
  string getQualTypeToString(QualType qt);
  string getTemplateArgumentToString(const TemplateArgument &ta);
  string getRecordDeclToString(const RecordDecl *rd);

  template<typename Arg>
  void writeTo(raw_fd_ostream &os, Arg arg) {
    os << arg;
  }

  template<typename Arg, typename... Args>
  void writeTo(raw_fd_ostream &os, Arg arg, Args... args) {
    os << arg;
    writeTo(os, args...);
  }
}

#endif