#ifndef SRCINFO_ACTION
#define SRCINFO_ACTION

#include "llvm/ADT/StringRef.h"

#include <memory>
#include <string>
#include <vector>

// #define PLUGIN_AUTO_ACTION PluginASTAction::ActionType::ReplaceAction
#include "CodemindPlugin.h"
#include "SrcInfoConsumer.h"

namespace srcinfo_plugin {
  using namespace codemind_plugin;

  class SourceInfoAction : public CodemindASTAction {
    private:
      MatcherOption option;
    public:
      unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &ci, StringRef InFile) override;
      bool ParseArgs(const CompilerInstance &ci, const vector<string>& args) override;
      void printError();
      void printHelp();
  };
}

#endif