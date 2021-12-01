#include "llvm/Support/Casting.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/SourceLocation.h"

#include "SrcInfoConsumer.h"
#include "SrcInfoMatcherCodegenItems.h"

using namespace srcinfo_plugin;
using namespace srcinfo_matcher_codegen;

class CodegenExprDecl : public CodemindMatcher<SourceInfoConsumer> {
  public:
    CodegenExprDecl(SourceInfoConsumer *c) : CodemindMatcher(c) {}
    virtual void run(const MatchFinder::MatchResult &Results) {
      auto item = static_cast<CodegenItems*>(consumer->getSourceInfoItem(ItemAttr::CodeGenerator));
      SourceManager &SourceMgr = getSourceManager();
      auto e = Results.Nodes.getNodeAs<Expr>("expr");
      auto fd = Results.Nodes.getNodeAs<FunctionDecl>("function");
      FullSourceLoc fsr(fd->getBeginLoc(), SourceMgr);
      
      if (item == nullptr)
        return;
      if (SourceMgr.getMainFileID() != fsr.getFileID())
        return;
      if (fd->isDefaulted() || fd->isTemplated())
        return;
      if (auto nexpr = dyn_cast<CXXNewExpr>(e))
        item->addRelation(fd, nexpr->getConstructExpr()->getConstructor());
      else if (auto nd = dyn_cast_or_null<NamedDecl>(e->getReferencedDeclOfCallee())) {
        if (nd->getKind() == Decl::Var) {
          auto vd = dyn_cast<VarDecl>(nd);
          if (vd->hasInit()) {
            if (auto cexpr = dyn_cast<CXXConstructExpr>(vd->getInit()))
              nd = cexpr->getConstructor();
            else if (auto nexpr = dyn_cast<CXXNewExpr>(vd->getInit()))
              nd = nexpr->getConstructExpr() != nullptr ? nexpr->getConstructExpr()->getConstructor() : nd;
          }
        }
        item->addRelation(fd, nd);
      }
    }
};

class CodegenFunctionDecl : public CodemindMatcher<SourceInfoConsumer> {
  public:
    CodegenFunctionDecl(SourceInfoConsumer *c) : CodemindMatcher(c) {}
    virtual void run(const MatchFinder::MatchResult &Results) {
      auto item = static_cast<CodegenItems*>(consumer->getSourceInfoItem(ItemAttr::CodeGenerator));
      SourceManager &SourceMgr = getSourceManager();
      auto fd = Results.Nodes.getNodeAs<FunctionDecl>("function");
      FullSourceLoc fsr(fd->getBeginLoc(), SourceMgr);
      
      if (item == nullptr)
        return;
      if (SourceMgr.getMainFileID() != fsr.getFileID())
        return;
      if (fd->isTemplated())
        return;
      item->addRelation(fd);
    }
};

void SourceInfoConsumer::InitCodeGenerator(string path) {
  setSourceInfoItem(ItemAttr::CodeGenerator, make_shared<CodegenItems>(this, path));
  MatchFinder &finder = getMatchFinder();

  auto codegenExprDecl = make_shared<CodegenExprDecl>(this);
  addMatcher(codegenExprDecl);
  finder.addMatcher(
    expr(
      anyOf(
        cxxNewExpr(hasDescendant(cxxConstructExpr())),
        declRefExpr(),
        memberExpr()),
      hasAncestor(functionDecl().bind("function"))
    ).bind("expr"), codegenExprDecl.get());

  auto codegenFunctionDecl = make_shared<CodegenFunctionDecl>(this);
  addMatcher(codegenFunctionDecl);
  finder.addMatcher(functionDecl(unless(isDefaulted())).bind("function"), codegenFunctionDecl.get());
}