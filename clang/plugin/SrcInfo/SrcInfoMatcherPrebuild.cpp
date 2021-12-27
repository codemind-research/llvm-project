#include "llvm/Support/Casting.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/SourceLocation.h"

#include "CodemindUtils.h"
#include "SrcInfoConsumer.h"

using namespace codemind_utils;
using namespace srcinfo_plugin;

class PrebuildVarDecl : public CodemindMatcher<SourceInfoConsumer> {
  public:
    PrebuildVarDecl(SourceInfoConsumer *cs) : CodemindMatcher(cs) {}
    void run(const MatchFinder::MatchResult &Results) override {
      SourceManager &SourceMgr = getSourceManager();
      auto vd = Results.Nodes.getNodeAs<VarDecl>("var");
      FullSourceLoc fsr(vd->getLocation(), SourceMgr);

      // skip if other file element
      if (SourceMgr.getMainFileID() != fsr.getFileID())
        return;

      auto *parent = vd->getParentFunctionOrMethod();
      if (!vd->isStaticLocal() && parent != nullptr && parent->getDeclKind() == Decl::Function) {
        // local && !static
      } else if (vd->getInit() != nullptr) {
        SourceRange sr = vd->getSourceRange();
        int eline = SourceMgr.getExpansionLineNumber(sr.getEnd());
        int ecol = SourceMgr.getExpansionColumnNumber(Lexer::getLocForEndOfToken(sr.getEnd(), 0, SourceMgr, consumer->getLangOpts()));
        auto loc = vd->getInit()->getExprLoc();

        // init expr 의 바로 앞에 있는 token 인 '=' 를 찾기.
        auto p = sr.getBegin();
        auto last_eq_token = p;
        while (p < loc) {
          auto token_opt = Lexer::findNextToken(p, SourceMgr, consumer->getLangOpts());
          if (token_opt.hasValue()) {
            auto token = token_opt.getValue();
            if (token.getKind() == tok::TokenKind::equal)
              last_eq_token = token.getLocation();

            p = token.getLocation();
          } else
            return;
        }

        // 못 찾은 경우 실패 처리
        if (last_eq_token == sr.getBegin())
          return;

        // '=' token
        int sline = SourceMgr.getExpansionLineNumber(last_eq_token);
        int scol = SourceMgr.getExpansionColumnNumber(last_eq_token);
        if (sline == 0)
          return;
        writeTo(outs(), "GI:", sline, ":", scol, ",", eline, ":", ecol, "\n");
      }
    }
};

class PrebuildClassDecl : public CodemindMatcher<SourceInfoConsumer> {
  public:
    PrebuildClassDecl(SourceInfoConsumer *cs) : CodemindMatcher(cs) {}
    void run(const MatchFinder::MatchResult &Results) override {
      SourceManager &SourceMgr = getSourceManager();
      auto cd = Results.Nodes.getNodeAs<CXXRecordDecl>("class");
      
      // skip if has no field
      if (cd->field_empty())
        return;

      auto loc = cd->getBraceRange().getBegin();
      int sline = SourceMgr.getExpansionLineNumber(loc, NULL);
      int scol = SourceMgr.getExpansionColumnNumber(loc, NULL);
      if (sline == 0)
        return;
      // cxxRecordDecl 에 union, class, struct 가 모두 걸림
      if (!cd->isClass())
        return;
      // template class인 경우 CXXRecord와 ClassTemplateSpecialization 두개가 걸림
      if (cd->getKind() != Decl::CXXRecord)
        return;
      writeTo(outs(), "CD:", sline, ":", scol + 1, "\n");
    }
};

void SourceInfoConsumer::InitPrebuild(set<string> detail) {
  MatchFinder &finder = getMatchFinder();

  auto prebuildVarDecl = make_shared<PrebuildVarDecl>(this);
  addMatcher(prebuildVarDecl);
  finder.addMatcher(
    varDecl(
      hasDescendant(declRefExpr())
    ).bind("var"),
    prebuildVarDecl.get());

  if (getLangOpts().CPlusPlus) {
    auto prebuildClassDecl = make_shared<PrebuildClassDecl>(this);
    addMatcher(prebuildClassDecl);
    finder.addMatcher(cxxRecordDecl().bind("class"), prebuildClassDecl.get());
  }
}