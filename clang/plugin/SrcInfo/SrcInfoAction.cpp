#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/FrontendPluginRegistry.h"

#include "CodemindUtils.h"
#include "SrcInfoAction.h"
#include "SrcInfoConsumer.h"

using namespace codemind_utils;
using namespace srcinfo_plugin;

// plugin 등록
static FrontendPluginRegistry::Add<SourceInfoAction> X("src-info", "Print the names of functions inside the file.");

unique_ptr<ASTConsumer> SourceInfoAction::CreateASTConsumer(CompilerInstance &ci, StringRef InFile) {
  return make_unique<SourceInfoConsumer>(ci, option);
}

bool SourceInfoAction::ParseArgs(const CompilerInstance &ci, const vector<string>& args) {
  if (args.empty()) {
    printError();
    return false;
  }

  bool show_err = false;
  auto lang = ci.getLangOpts();
  for (unsigned i = 0, size = args.size(); i < size; i++) {
    auto arg = args[i];
    if (arg == "-show-error")
      show_err = true;
    else if (arg == "-pre-build")
      option.pre_build = true;
    else if (arg == "-build")
      option.build = true;
    else if (arg == "-code-gen") {
      if (i + 1 >= size) {
        printError();
        return false;
      }
      option.code_generator = true;
      option.code_generated_path = args[++i];
    } else if (arg == "-help") {
      printHelp();
      return false;
    }
  }
  if (!show_err)
    ci.getDiagnostics().setClient(new IgnoringDiagConsumer());

  return true;
}

void SourceInfoAction::printError() {
  writeTo(errs(), "Try -help for more information.\n");
}

void SourceInfoAction::printHelp() {
  writeTo(errs(), "Help for SourceInfo plugin goes here\n");
}

