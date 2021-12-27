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
  
  class CodegenItems : public SourceInfoItems {
    private:
      SourceInfoConsumer *consumer;
      map<string, size_t> cached_type;
      map<const NamedDecl*, size_t> cached_function;
    public:
      CodegenItems(SourceInfoConsumer *ci);
      void finalize() override;
      size_t addType(QualType qt);
      size_t addItem(const NamedDecl *nd);
  };
}

#endif