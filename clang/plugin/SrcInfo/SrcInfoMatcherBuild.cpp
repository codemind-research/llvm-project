#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Casting.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/SourceLocation.h"

#include "CodemindUtils.h"
#include "SrcInfoConsumer.h"
#include "SrcInfoMatcherBuildItems.h"

using namespace std;
using namespace clang;
using namespace codemind_utils;
using namespace srcinfo_plugin;
using namespace srcinfo_matcher_build;

class BuildMatcher : public CodemindMatcher<SourceInfoConsumer> {
  public :
    BuildMatcher(SourceInfoConsumer *cs) : CodemindMatcher(cs) {}
    BuildItems* getBuildItem() { return static_cast<BuildItems*>(consumer->getSourceInfoItem(ItemAttr::Build)); }
};

class BuildConstIntVarDecl : public BuildMatcher {
  public:
    BuildConstIntVarDecl(SourceInfoConsumer *cs) : BuildMatcher(cs) {}
    void run(const MatchFinder::MatchResult &Results) override {
      SourceManager &SourceMgr = getSourceManager();
      auto vd = Results.Nodes.getNodeAs<FieldDecl>("var");
      auto literal = Results.Nodes.getNodeAs<IntegerLiteral>("init");
      FullSourceLoc fsr(vd->getLocation(), SourceMgr);

      string varstr = vd->getNameAsString().c_str();
      string litstr = literal->getValue().toString(10, true).c_str();
      string fname = fsr.getFileEntry()->getName().str();
      int sline = fsr.getLineNumber();

      writeTo(outs(), "CV-I:", varstr, ":", sline, ":", litstr, ":", fname, "\n");
    }
};

class BuildConstFloatVarDecl : public BuildMatcher {
  public:
    BuildConstFloatVarDecl(SourceInfoConsumer *cs) : BuildMatcher(cs) {}
    void run(const MatchFinder::MatchResult &Results) override {
      SourceManager &SourceMgr = getSourceManager();
      auto vd = Results.Nodes.getNodeAs<FieldDecl>("var");
      auto literal = Results.Nodes.getNodeAs<FloatingLiteral>("init");
      FullSourceLoc fsr(vd->getLocation(), SourceMgr);

      string varstr = vd->getNameAsString().c_str();
      SmallVector<char, 64> litstr;
      literal->getValue().toString(litstr, 64, 0);
      string fname = fsr.getFileEntry()->getName().str();
      int sline = fsr.getLineNumber();
      *(litstr.end()) = 0;

      writeTo(outs(), "CV-F:", varstr, ":", sline, ":", litstr.data(), ":", fname, "\n");
    }
};

class BuildCallExpr : public BuildMatcher {
  public:
    BuildCallExpr(SourceInfoConsumer *cs) : BuildMatcher(cs) {}
    void run(const MatchFinder::MatchResult &Results) override {
      SourceManager &SourceMgr = getSourceManager();
      auto ce = Results.Nodes.getNodeAs<CallExpr>("call_expr");
      FullSourceLoc fsr(ce->getBeginLoc(), SourceMgr);
      if (SourceMgr.getMainFileID() != fsr.getFileID())
        return;

      if (ce->getCalleeDecl() == nullptr)
        return;

      switch (ce->getCalleeDecl()->getKind()) {
        case Decl::Kind::Function : {
#ifdef __CALL_EXPR_INFO__
          if (auto fd = dyn_cast<FunctionDecl>(ce->getCalleeDecl())) {
            int sline = SourceMgr.getExpansionLineNumber(ce->getBeginLoc());
            int scol = SourceMgr.getExpansionColumnNumber(ce->getBeginLoc());
            writeTo(outs(), "CE:", sline, "'", scol, "'", fd->getQualifiedNameAsString(),
                    "'", "'", fd->getType().getAsString(), "\n");
          }
#endif
          break;
        }
        case Decl::Kind::CXXMethod : {
          if (auto md = dyn_cast<CXXMethodDecl>(ce->getCalleeDecl())) {
            int sline = SourceMgr.getExpansionLineNumber(ce->getBeginLoc());
            int scol = SourceMgr.getExpansionColumnNumber(ce->getBeginLoc());
#ifndef __CALL_EXPR_INFO__
            if (!md->isVirtual())
              return;
#endif
            if (isa<CXXMemberCallExpr>(ce)) {
              auto me = Results.Nodes.getNodeAs<MemberExpr>("member");
              if (me != nullptr) {
                sline = SourceMgr.getExpansionLineNumber(me->getMemberNameInfo().getBeginLoc());
                scol = SourceMgr.getExpansionColumnNumber(me->getMemberNameInfo().getBeginLoc());
              }
            }
            writeTo(outs(), "CE:", sline, "'", scol, "'", md->getQualifiedNameAsString(), "'",
                    md->getThisType().getAsString(), "'", md->getType().getAsString(), "\n");
          }
          break;
        }
        case Decl::Kind::Var : {
          auto ice = Results.Nodes.getNodeAs<ImplicitCastExpr>("cast");
          auto dre = Results.Nodes.getNodeAs<DeclRefExpr>("ref_expr");
          if ((ice == nullptr) || (dre == nullptr))
            return;
          int sline = SourceMgr.getExpansionLineNumber(ce->getBeginLoc());
          int scol = SourceMgr.getExpansionColumnNumber(ce->getBeginLoc());
          writeTo(outs(), "CE:", sline, "'", scol, "'", dre->getNameInfo().getAsString(),
                  "'", "'", ice->getType().getAsString(), "\n");
          break;
        }
        case Decl::Kind::Field : {
          auto ice = Results.Nodes.getNodeAs<ImplicitCastExpr>("cast");
          auto me = Results.Nodes.getNodeAs<MemberExpr>("member");
          if ((ice == nullptr) || (me == nullptr))
            return;
          int sline = SourceMgr.getExpansionLineNumber(ce->getBeginLoc());
          int scol = SourceMgr.getExpansionColumnNumber(ce->getBeginLoc());
          writeTo(outs(), "CE:", sline, "'", scol, "'", me->getMemberNameInfo().getAsString(),
                  "'", "'", ice->getType().getAsString(), "\n");
          break;
        }
        default:
          break;
      }
    }
};

class BuildMethodDecl : public BuildMatcher {
  public:
    BuildMethodDecl(SourceInfoConsumer *cs) : BuildMatcher(cs) {}
    void run(const MatchFinder::MatchResult &Results) override {
      SourceManager &SourceMgr = getSourceManager();
      auto rd = Results.Nodes.getNodeAs<RecordDecl>("record");
      FullSourceLoc fsr(rd->getLocation(), SourceMgr);

      if (SourceMgr.getMainFileID() != fsr.getFileID())
        return;
      if (rd->isTemplated() || rd->isLambda() || rd->getNameAsString().empty())
        return;

      auto name = getQualifiedNameString(rd);
      if (name.length() > 0) {
        BuildItems *item = getBuildItem();
        if (item != nullptr)
          item->addClassName(name, rd->getTagKind());
      }
    }
};

class BuildFunctionDecl : public BuildMatcher {
  public:
    BuildFunctionDecl(SourceInfoConsumer *cs) : BuildMatcher(cs) {}
    void run(const MatchFinder::MatchResult &Results) override {
      SourceManager &SourceMgr = getSourceManager();
      auto fd = Results.Nodes.getNodeAs<FunctionDecl>("fun");
      auto rd = Results.Nodes.getNodeAs<RecordDecl>("record");
      FullSourceLoc fsr(fd->getLocation(), SourceMgr);
      auto filename = SourceMgr.getFilename(fsr.getFileLoc());

      if (fd->isTemplated())
        return;
      if (filename.size() == 0)
        return;
        
      string fun_name = getQualifiedNameString(fd, false) + "'" + "(";
      for (unsigned i = 0; i < fd->getNumParams(); i++)
        fun_name += ((i > 0) ? "," : "") + getQualifiedTypeString(fd->getParamDecl(i)->getType());
      fun_name += ")";
      if (rd != nullptr) {
        // 익명 클래스는 배제
        if (rd->getNameAsString().size() > 0) {
          BuildItems *item = getBuildItem();
          if ((item != nullptr) && item->RedundantCheck(filename.str(), rd->getNameAsString(), fun_name))
            writeTo(outs(), "FN-M:", filename.str(), "'", fun_name, "\n");
        }
      } else {
        BuildItems *item = getBuildItem();
        if ((item != nullptr) && item->RedundantCheck(filename.str(), "", fun_name))
          writeTo(outs(), "FN-N:", filename.str(), "'", fun_name, "\n");
      }
    }
};

class BuildCatchStmt : public BuildMatcher {
  public:
    BuildCatchStmt(SourceInfoConsumer *cs) : BuildMatcher(cs) {}
    void run(const MatchFinder::MatchResult &Results) override {
      SourceManager &SourceMgr = getSourceManager();
      auto stmt = Results.Nodes.getNodeAs<CXXCatchStmt>("catch");
      auto srcrange = stmt->getSourceRange();
      FullSourceLoc fsbegin(srcrange.getBegin(), SourceMgr);
      auto filename = SourceMgr.getFilename(fsbegin.getFileLoc());
      
      if (SourceMgr.getMainFileID() != fsbegin.getFileID())
        return;
      if (filename.size() == 0)
        return;

      FullSourceLoc fsend(srcrange.getEnd(), SourceMgr);
      int sline = SourceMgr.getExpansionLineNumber(fsbegin, NULL);
      int send = SourceMgr.getExpansionLineNumber(fsend, NULL);
      writeTo(outs(), "X:", sline, ",", send, "\n");
    }
};

void SourceInfoConsumer::InitBuild(set<string> detail) {
  setSourceInfoItem(ItemAttr::Build, make_shared<BuildItems>());
  MatchFinder &finder = getMatchFinder();

  auto buildConstIntVarDecl = make_shared<BuildConstIntVarDecl>(this);
  addMatcher(buildConstIntVarDecl);
  finder.addMatcher(
    fieldDecl(
      hasType(isConstQualified())
      , hasInClassInitializer(ignoringImplicit(integerLiteral().bind("init")))
    ).bind("var"),
    buildConstIntVarDecl.get());

  auto buildConstFloatVarDecl = make_shared<BuildConstFloatVarDecl>(this);
  addMatcher(buildConstFloatVarDecl);
  finder.addMatcher(
    fieldDecl(
      hasType(isConstQualified()),
      hasInClassInitializer(ignoringImplicit(floatLiteral().bind("init")))
    ).bind("var"),
    buildConstFloatVarDecl.get());

  if (getLangOpts().CPlusPlus) {
    auto buildCallExpr = make_shared<BuildCallExpr>(this);
    addMatcher(buildCallExpr);
    finder.addMatcher(
      callExpr(
        anyOf(
          has(memberExpr().bind("member")),
          has(implicitCastExpr(
                anyOf(
                  has(declRefExpr().bind("ref_expr")),
                  has(memberExpr().bind("member")))
              ).bind("cast")),
          anything())
      ).bind("call_expr"),
      buildCallExpr.get());

    auto buildMethodDecl = make_shared<BuildMethodDecl>(this);
    addMatcher(buildMethodDecl);
    finder.addMatcher(
      recordDecl(
        isDefinition(),
        unless(isImplicit()),
        unless(hasAncestor(functionDecl())),
        anyOf(
          classTemplateSpecializationDecl(hasSpecializedTemplate(classTemplateDecl())),
          unless(classTemplateSpecializationDecl()))
      ).bind("record"),
      buildMethodDecl.get());

    auto buildFunctionDecl = make_shared<BuildFunctionDecl>(this);
    addMatcher(buildFunctionDecl);
    finder.addMatcher(
      functionDecl(
        isDefinition(),
        anyOf(
          hasAncestor(cxxRecordDecl().bind("record")),
          unless(hasAncestor(cxxRecordDecl())))
      ).bind("fun"),
      buildFunctionDecl.get());

    auto buildCatchStmt = make_shared<BuildCatchStmt>(this);
    addMatcher(buildCatchStmt);
    finder.addMatcher(cxxCatchStmt().bind("catch"), buildCatchStmt.get());
  }
}