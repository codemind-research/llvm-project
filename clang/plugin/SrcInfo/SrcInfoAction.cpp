#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/FrontendPluginRegistry.h"

#include <sstream>

#include "CodemindUtils.h"
#include "SrcInfoAction.h"
#include "SrcInfoConsumer.h"

using namespace std;
using namespace llvm;
using namespace clang;
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
    stringstream arg(args[i]);
    string key, temp;
    set<string> detail;
    getline(arg, key, '=');
    while (getline(arg, temp, ','))
      detail.insert(temp);

    if (key == "-show-error")
      show_err = true;
    else if (key == "-pre-build")
      option[ItemAttr::Prebuild] = detail;
    else if (key == "-build")
      option[ItemAttr::Build] = detail;
    else if (key == "-code-gen")
      option[ItemAttr::CodeGenerator] = detail;
    else if (key == "-metric") {
      option[ItemAttr::Metric] = detail;
    } else if (key == "-help") {
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

