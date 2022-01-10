#ifndef SRCINFO_CODEGENITEMS
#define SRCINFO_CODEGENITEMS

#include "llvm/Support/raw_ostream.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Mangle.h"

#include <memory>
#include <set>

#include "SrcInfoConsumer.h"

namespace srcinfo_matcher_codegen {
  using namespace llvm;
  using namespace clang;
  using namespace srcinfo_plugin;
  
  class CodegenItems : public SourceInfoItems {
    private:
      string filename;
      SourceInfoConsumer *consumer;
      set<const Decl*> cached_decl;
    public:
      CodegenItems(SourceInfoConsumer *ci, set<string> detail);
      void finalize() override;
      void addItem(const NamedDecl *nd);
  };
}

#endif