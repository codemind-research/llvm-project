#ifndef SRCINFO_CODEGENITEMS
#define SRCINFO_CODEGENITEMS

#include "llvm/Support/raw_ostream.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Mangle.h"

#include <set>
#include <map>
#include <memory>

#include "SrcInfoConsumer.h"

namespace srcinfo_matcher_codegen {
  using namespace llvm;
  using namespace srcinfo_plugin;
  
  enum class ParmDisplay {Self, ExplicitVoid};

  class ParmDisplayOption : public set<ParmDisplay> {
    public:
      ParmDisplayOption() = default;
      ParmDisplayOption(initializer_list<ParmDisplay> list);
      bool contain(ParmDisplay item);
      bool contain(initializer_list<ParmDisplay> items);
  };
  
  class CodegenItems : public SourceInfoItems {
    private:
      SourceInfoConsumer *consumer;
      unique_ptr<raw_fd_ostream> file;
      unique_ptr<MangleContext> mangler;
      map<const FunctionDecl*, set<const NamedDecl*>> data;

      raw_fd_ostream &getFileStream();
      string getMangleName(const NamedDecl *d);
      string getFunctionParamToString(const FunctionDecl *fd, ParmDisplayOption opt);
      void generateCode(string name, const QualType qt, const set<const NamedDecl*> &items);
      void generateParamCode(const pair<const FunctionDecl*, set<const NamedDecl*>> &data);
      void generateBody(const pair<const FunctionDecl*, set<const NamedDecl*>> &data);
    public:
      CodegenItems(SourceInfoConsumer *ci, string path);
      ~CodegenItems();
      void finalize() override;
      void addRelation(const FunctionDecl *fd, const NamedDecl *nd = nullptr);
  };
}

#endif