#include "llvm/Support/Casting.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/SourceLocation.h"

#include "SrcInfoConsumer.h"
#include "SrcInfoMatcherMetricItems.h"

using namespace std;
using namespace clang;
using namespace srcinfo_plugin;
using namespace srcinfo_matcher_codegen;

struct MetricOption {
  bool IncCase;
  bool IncMacro;
  bool IncLogical;
  bool IncTernary;
};

class MetricMatcher : public CodemindMatcher<SourceInfoConsumer> {
  protected:
    MetricOption option;
  public :
    MetricMatcher(SourceInfoConsumer *cs, MetricOption opt) : CodemindMatcher(cs), option(opt) {}
    MetricItems* getMetricItem() { return static_cast<MetricItems*>(consumer->getSourceInfoItem(ItemAttr::Metric)); }
};

class MetricFunctionDecl : public MetricMatcher {
  public:
    MetricFunctionDecl(SourceInfoConsumer *cs, MetricOption opt) : MetricMatcher(cs, opt) {}
    void run(const MatchFinder::MatchResult &Results) override {
      MetricItems *item = getMetricItem();
      SourceManager &SourceMgr = getSourceManager();
      auto fd = Results.Nodes.getNodeAs<FunctionDecl>("function");
      FullSourceLoc fsr(fd->getLocation(), SourceMgr);

      if (item == nullptr)
        return;
      if (SourceMgr.getMainFileID() != fsr.getFileID())
        return;
      if (fd->isTemplateInstantiation() || fd->isDefaulted())
        return;
      item->getFuncID(fd);
      item->addCyclomaticComplexity(fd, Stmt::DeclStmtClass, 1);
    }
};

class MetricGlobalVariableRef : public MetricMatcher {
  public:
    MetricGlobalVariableRef(SourceInfoConsumer *cs, MetricOption opt) : MetricMatcher(cs, opt) {}
    void run(const MatchFinder::MatchResult &Results) override {
      MetricItems *item = getMetricItem();
      SourceManager &SourceMgr = getSourceManager();
      auto fd = Results.Nodes.getNodeAs<FunctionDecl>("function");
      auto vd = Results.Nodes.getNodeAs<VarDecl>("var");
      auto e = Results.Nodes.getNodeAs<DeclRefExpr>("expr");
      FullSourceLoc fsr(fd->getLocation(), SourceMgr);

      if (item == nullptr)
        return;
      if (SourceMgr.isMacroBodyExpansion(e->getBeginLoc())) {
        if (!option.IncMacro)
          return;
      } else if (SourceMgr.getMainFileID() != fsr.getFileID())
        return;
      if (fd->isTemplateInstantiation() || fd->isDefaulted())
        return;
      item->addGlobalVariable(fd, vd);
    }
};

class MetricCyclomaticComplexity : public MetricMatcher {
  public:
    MetricCyclomaticComplexity(SourceInfoConsumer *cs, MetricOption opt) : MetricMatcher(cs, opt) {}
    void run(const MatchFinder::MatchResult &Results) override {
      MetricItems *item = getMetricItem();
      SourceManager &SourceMgr = getSourceManager();
      auto fd = Results.Nodes.getNodeAs<FunctionDecl>("function");
      auto sd = Results.Nodes.getNodeAs<Stmt>("decision");
      FullSourceLoc fsr(fd->getLocation(), SourceMgr);

      if (item == nullptr)
        return;
      if (SourceMgr.isMacroBodyExpansion(sd->getBeginLoc())) {
        if (!option.IncMacro)
          return;
      } else if (SourceMgr.getMainFileID() != fsr.getFileID())
        return;
      if (fd->isTemplateInstantiation() || fd->isDefaulted())
        return;

      int complexity = 0;
      if (option.IncCase && (sd->getStmtClass() == Stmt::SwitchStmtClass)) {
        for (auto sc = dyn_cast<SwitchStmt>(sd)->getSwitchCaseList(); sc != NULL; sc = sc->getNextSwitchCase())
          complexity++;
      } else
        complexity = 1;
      item->addCyclomaticComplexity(fd, sd->getStmtClass(), complexity);
    }
};

class MetricModuleCircularDependency : public MetricMatcher {
  public:
    MetricModuleCircularDependency(SourceInfoConsumer *cs, MetricOption opt) : MetricMatcher(cs, opt) {}
    void run(const MatchFinder::MatchResult &Results) override {
      MetricItems *item = getMetricItem();
      SourceManager &SourceMgr = getSourceManager();
      auto e = Results.Nodes.getNodeAs<Expr>("expr");
      auto fd = Results.Nodes.getNodeAs<FunctionDecl>("function");
      FullSourceLoc fsr(e->getBeginLoc(), SourceMgr);

      if (item == nullptr)
        return;
      if (SourceMgr.isMacroBodyExpansion(e->getBeginLoc())) {
        if (!option.IncMacro)
          return;
      } else if (SourceMgr.getMainFileID() != fsr.getFileID())
        return;
      if (fd->isTemplateInstantiation() || fd->isDefaulted())
        return;
      if (e->getReferencedDeclOfCallee() == nullptr)
        return;
      if (!isa<NamedDecl>(e->getReferencedDeclOfCallee()))
        return;

      auto nd = cast<NamedDecl>(e->getReferencedDeclOfCallee());
      switch (nd->getKind()) {
        case Decl::Kind::Var :
        case Decl::Kind::Field :
        case Decl::Kind::Function :
        case Decl::Kind::CXXMethod :
          item->addModuleCircularDependency(fd, nd);
          break;
        default :
          break;
      }
    }
};

class MectricCouplingBetweenObjectsDecl : public MetricMatcher {
  private:
    void addCouplingInfo(const CXXRecordDecl *src, QualType qt) {
      if (qt.isNull())
        return;
      while (qt->isPointerType())
        qt = qt->getPointeeType();
      if (!qt->isRecordType())
        return;

      auto dst = qt->getAsRecordDecl();
      if (src == nullptr || dst == nullptr) 
        return;
      MetricItems *item = getMetricItem();
      if (item != nullptr)
        item->addCouplingBetwwenObjects(src, dst);
    }
  public:
    MectricCouplingBetweenObjectsDecl(SourceInfoConsumer *cs, MetricOption opt) : MetricMatcher(cs, opt) {}
    void run(const MatchFinder::MatchResult &Results) override {
      SourceManager &SourceMgr = getSourceManager();
      auto d = Results.Nodes.getNodeAs<Decl>("decl");
      FullSourceLoc fsr(d->getLocation(), SourceMgr);
      string fn = SourceMgr.getFilename(fsr.getFileLoc()).str();

      if (SourceMgr.isMacroBodyExpansion(d->getBeginLoc())) {
        if (!option.IncMacro)
          return;
      } else if (SourceMgr.getMainFileID() != fsr.getFileID()) {
        if (!(fn.length() > 3 && fn.c_str()[fn.length()-2] == '.' && fn.c_str()[fn.length()-1] == 'h'))
          return;
      }

      switch (d->getKind()) {
        case Decl::Kind::CXXRecord : {
          if (isa<CXXRecordDecl>(d)) {
            auto rd = cast<CXXRecordDecl>(d);
            if (rd->hasDefinition() && !rd->isParsingBaseSpecifiers()) {
              for (auto base : rd->bases())
                addCouplingInfo(rd, base.getType());
            }
          }
          break;
        }
        case Decl::Kind::CXXMethod : {
          auto rd = Results.Nodes.getNodeAs<CXXRecordDecl>("record");
          if (isa<CXXMethodDecl>(d)) {
            auto md = cast<CXXMethodDecl>(d);
            if (!md->isTemplateInstantiation() && !md->isDefaulted())
              addCouplingInfo(rd, md->getReturnType());
          }
          break;
        }
        case Decl::Kind::Field : {
          auto rd = Results.Nodes.getNodeAs<CXXRecordDecl>("record");
          if (isa<FieldDecl>(d))
            addCouplingInfo(rd, cast<FieldDecl>(d)->getType());
          break;
        }
        case Decl::Kind::Var : {
          auto md = Results.Nodes.getNodeAs<CXXMethodDecl>("method");
          auto rd = md->getParent();
          if (isa<VarDecl>(d) && !md->isTemplateInstantiation() && !md->isDefaulted())
            addCouplingInfo(rd, cast<VarDecl>(d)->getType());
          break;
        }
        default:
          break;
      }
    }
};

class MetricCouplingBetweenObjectsRef : public MetricMatcher {
  private:
    void addCouplingInfo(const CXXRecordDecl *src, const RecordDecl *dst) {
      if(src == nullptr || dst == nullptr)
        return;

      MetricItems *item = getMetricItem();
      if (item != nullptr)
        item->addCouplingBetwwenObjects(src, dst);
    }
  public:
    MetricCouplingBetweenObjectsRef(SourceInfoConsumer *cs, MetricOption opt) : MetricMatcher(cs, opt) {}
    void run(const MatchFinder::MatchResult &Results) override {
      SourceManager &SourceMgr = getSourceManager();
      auto e = Results.Nodes.getNodeAs<Expr>("expr");
      auto md = Results.Nodes.getNodeAs<CXXMethodDecl>("method");
      FullSourceLoc fsr(e->getBeginLoc(), SourceMgr);
      string fn = SourceMgr.getFilename(fsr.getFileLoc()).str();

      if (SourceMgr.isMacroBodyExpansion(e->getBeginLoc())) {
        if(!option.IncMacro)
          return;
      } else if (SourceMgr.getMainFileID() != fsr.getFileID()) {
        if (!(fn.length() > 3 && fn.c_str()[fn.length()-2] == '.' && fn.c_str()[fn.length()-1] == 'h'))
          return;
      }
      if (md->isTemplateInstantiation() || md->isDefaulted())
        return;
      if (e->getReferencedDeclOfCallee() == nullptr)
        return;
      if (!isa<NamedDecl>(e->getReferencedDeclOfCallee()))
        return;
      
      auto nd = cast<NamedDecl>(e->getReferencedDeclOfCallee());
      switch (nd->getKind()) {
        case Decl::Kind::CXXMethod :
          addCouplingInfo(md->getParent(), cast<CXXMethodDecl>(nd)->getParent());
          break;
        case Decl::Kind::Field :
          addCouplingInfo(md->getParent(), cast<FieldDecl>(nd)->getParent());
          break;
        default:
          break;
      }
    }
};

void SourceInfoConsumer::InitMetric(set<string> detail) {
  setSourceInfoItem(ItemAttr::Metric, make_shared<MetricItems>(this));
  MatchFinder &finder = getMatchFinder();
  MetricOption option;
  option.IncCase = detail.find("case") != detail.end();
  option.IncMacro = detail.find("macro") != detail.end();
  option.IncLogical = detail.find("logical") != detail.end();
  option.IncTernary = detail.find("ternary") != detail.end();

  if (getLangOpts().CPlusPlus) {
    // auto metricCouplingBetweenObjectsDecl = make_shared<MetricCouplingBetweenObjectsDecl>(this, option);
    // addMatcher(metricCouplingBetweenObjectsDecl);
    // finder.addMatcher(
    //   decl(
    //     anyOf(
    //       cxxRecordDecl(isDefinition()),
    //       cxxMethodDecl(hasAncestor(cxxRecordDecl().bind("record"))),
    //       fieldDecl(hasAncestor(cxxRecordDecl().bind("record"))),
    //       varDecl(hasAncestor(cxxMethodDecl().bind("method")))
    //     )
    //   ).bind("decl"),
    //   metricCouplingBetweenObjectsDecl.get());

    auto metricCouplingBetweenObjectsRef = make_shared<MetricCouplingBetweenObjectsRef>(this, option);
    addMatcher(metricCouplingBetweenObjectsRef);
    finder.addMatcher(
      expr(
        anyOf(
          memberExpr(),
          declRefExpr()
        ),
        hasAncestor(cxxMethodDecl().bind("method"))
      ).bind("expr"),
      metricCouplingBetweenObjectsRef.get());  
  } else {
    auto metricGlobalVariableRef = make_shared<MetricGlobalVariableRef>(this, option);
    addMatcher(metricGlobalVariableRef);
    finder.addMatcher(
      declRefExpr(
        to(
          varDecl(
            hasGlobalStorage(),
            unless(isStaticStorageClass()),
            unless(hasType(isConstQualified()))
          ).bind("var")
        ),
        hasAncestor(functionDecl().bind("function"))
      ).bind("expr"),
      metricGlobalVariableRef.get());
  } 
  auto metricFunctionDecl = make_shared<MetricFunctionDecl>(this, option);
  addMatcher(metricFunctionDecl);
  finder.addMatcher(
    functionDecl(
      isDefinition(),
      anyOf(
        hasAncestor(recordDecl()),
        anything()
      )
    ).bind("function"),
    metricFunctionDecl.get());

  auto metricCyclomaticComplexity = make_shared<MetricCyclomaticComplexity>(this, option);
  addMatcher(metricCyclomaticComplexity);
  if (!option.IncLogical && !option.IncTernary) {
    finder.addMatcher(
      stmt(
        hasAncestor(functionDecl().bind("function")),
        anyOf(
          doStmt(),
          whileStmt(),
          forStmt(),
          cxxForRangeStmt(),
          ifStmt(),
          switchStmt()
        )
      ).bind("decision"),
      metricCyclomaticComplexity.get());
  } else {
    auto opStmt = stmt(anyOf(binaryOperator(hasOperatorName("&&")), binaryOperator(hasOperatorName("||")), binaryConditionalOperator(), conditionalOperator()));
    if(!option.IncLogical)
      opStmt = stmt(anyOf(binaryConditionalOperator(), conditionalOperator()));
    else if(!option.IncTernary)
      opStmt = stmt(anyOf(binaryOperator(hasOperatorName("&&")), binaryOperator(hasOperatorName("||"))));
    finder.addMatcher(
      stmt(
        hasAncestor(functionDecl().bind("function")),
        anyOf(
          doStmt(),
          whileStmt(),
          forStmt(),
          cxxForRangeStmt(),
          ifStmt(),
          switchStmt(),
          opStmt
        )
      ).bind("decision"),
      metricCyclomaticComplexity.get());
  }

  auto metricModuleCircularDependency = make_shared<MetricModuleCircularDependency>(this, option);
  addMatcher(metricModuleCircularDependency);
  finder.addMatcher(
    expr(
      anyOf(
        memberExpr(),
        declRefExpr()
      ),
      hasAncestor(functionDecl().bind("function"))
    ).bind("expr"),
    metricModuleCircularDependency.get());
}