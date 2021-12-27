#ifndef SRCINFO_CODEGENITEMS
#define SRCINFO_CODEGENITEMS

#include "llvm/Support/raw_ostream.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Mangle.h"

#include <map>
#include <memory>

#include "SrcInfoConsumer.h"

namespace srcinfo_matcher_codegen {
  using namespace llvm;
  using namespace srcinfo_plugin;
  
  class MetricItems : public SourceInfoItems {
    private:
      SourceInfoConsumer *consumer;
      map<const NamedDecl*, size_t> cached_name;
      map<const FunctionDecl*, size_t> cached_func;
      map<size_t, map<size_t, int>> gv;
      map<size_t, map<Stmt::StmtClass, int>> cc;
      map<size_t, vector<size_t>> mcd;
      map<size_t, set<size_t>> cp;
    public:
      MetricItems(SourceInfoConsumer *ci);
      void finalize() override;
      size_t getNameID(const NamedDecl *nd);
      size_t getFuncID(const FunctionDecl *fd);
      void addGlobalVariable(const FunctionDecl *fd, const VarDecl *var);
      void addCyclomaticComplexity(const FunctionDecl *fd, Stmt::StmtClass stmt, int complexity);
      void addModuleCircularDependency(const NamedDecl *src, const NamedDecl *dst);
      void addCouplingBetwwenObjects(const RecordDecl *src, const RecordDecl *dst);
  };
}

#endif