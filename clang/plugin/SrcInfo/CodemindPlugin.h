/*
  Manually execute
    clang -Xclang -load -Xclang {plugin file} -Xclang -plugin -Xclang {regist name} [-Xclang -plugin-arg-{regist name} - Xclang {plugin arg} ...] {source file}

  Automatically execute (defined PLUGIN_AUTO_ACTION)
    clang -Xclang -load -Xclang {plugin file} [-Xclang -plugin-arg-{regist name} - Xclang {plugin arg} ...] {source file}
*/
#ifndef CODEMIND_PLUGIN
#define CODEMIND_PLUGIN

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"

#include <memory>
#include <vector>

namespace codemind_plugin {
  using namespace std;
  using namespace clang;
  using namespace ast_matchers;

  class CodemindASTAction : public PluginASTAction {
    public:
      #if defined(PLUGIN_AUTO_ACTION)
      PluginASTAction::ActionType getActionType() override {
        return (PluginASTAction::ActionType)PLUGIN_AUTO_ACTION;
      }
      #endif
  };

  class CodemindASTConsumer : public ASTConsumer {
    private:
      MatchFinder finder;
      const CompilerInstance &compiler;
      vector<shared_ptr<MatchFinder::MatchCallback>> callbacks;
    public:
      CodemindASTConsumer(const CompilerInstance &ci) : compiler(ci) {
      }

      void HandleTranslationUnit(ASTContext &ctx) override {
        finder.matchAST(ctx);
      }

      virtual MatchFinder &getMatchFinder() {
        return finder;
      }

      const CompilerInstance &getCompilerInstance() {
        return compiler;
      }

      SourceManager &getSourceManager() {
        return compiler.getSourceManager();
      }

      ASTContext &getASTContext() {
        return compiler.getASTContext();
      }

      const LangOptions &getLangOpts() {
        return compiler.getLangOpts();
      }

      const TargetInfo &getTargetInfo() {
        return compiler.getTarget();
      }

      void addMatcher(shared_ptr<MatchFinder::MatchCallback> callback) {
        callbacks.push_back(callback);
      }
  };

  template<typename Consumer = CodemindASTConsumer>
  class CodemindMatcher : public MatchFinder::MatchCallback {
    protected:
      Consumer *consumer;
    public:
      CodemindMatcher(Consumer *c) : consumer(c) {
      }

      SourceManager &getSourceManager() {
        return consumer->getSourceManager();
      }
  };
}

#endif