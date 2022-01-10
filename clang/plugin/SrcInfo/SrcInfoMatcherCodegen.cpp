#include "llvm/Support/Casting.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/SourceLocation.h"

#include "SrcInfoConsumer.h"
#include "SrcInfoMatcherCodegenItems.h"

using namespace std;
using namespace clang;
using namespace srcinfo_plugin;
using namespace srcinfo_matcher_codegen;

class CodegenMatcher : public CodemindMatcher<SourceInfoConsumer> {
  public :
    CodegenMatcher(SourceInfoConsumer *cs) : CodemindMatcher(cs) {}
    CodegenItems* getCodegenItem() { return static_cast<CodegenItems*>(consumer->getSourceInfoItem(ItemAttr::CodeGenerator)); }
};

class CodegenFunctionDecl : public CodegenMatcher {
  public:
    CodegenFunctionDecl(SourceInfoConsumer *cs) : CodegenMatcher(cs) {}
    void run(const MatchFinder::MatchResult &Results) override {
      CodegenItems *item = getCodegenItem();
      auto fd = Results.Nodes.getNodeAs<FunctionDecl>("function");
      
      if (getSourceManager().getMainFileID() != FullSourceLoc(fd->getLocation(), getSourceManager()).getFileID())
        return;
      if (item == nullptr)
        return;
      if (fd->isDefaulted() || fd->isTemplated() || !fd->isFirstDecl())
        return;
      item->addItem(fd);
    }
};

class CodegenVarDecl : public CodegenMatcher {
  public:
    CodegenVarDecl(SourceInfoConsumer *cs) : CodegenMatcher(cs) {}
    void run(const MatchFinder::MatchResult &Results) override {
      CodegenItems *item = getCodegenItem();
      auto vd = Results.Nodes.getNodeAs<VarDecl>("var");
      
      if (getSourceManager().getMainFileID() != FullSourceLoc(vd->getLocation(), getSourceManager()).getFileID())
        return;
      if (item == nullptr)
        return;
      item->addItem(vd);
    }
};

void SourceInfoConsumer::InitCodeGenerator(set<string> detail) {
  setSourceInfoItem(ItemAttr::CodeGenerator, make_shared<CodegenItems>(this, detail));
  MatchFinder &finder = getMatchFinder();

  auto codegenFunctionDecl = make_shared<CodegenFunctionDecl>(this);
  addMatcher(codegenFunctionDecl);
  finder.addMatcher(
    functionDecl().bind("function"),
    codegenFunctionDecl.get());

  auto codegenVarDecl = make_shared<CodegenVarDecl>(this);
  addMatcher(codegenVarDecl);
  finder.addMatcher(
    varDecl(hasGlobalStorage()).bind("var"),
    codegenVarDecl.get());
}