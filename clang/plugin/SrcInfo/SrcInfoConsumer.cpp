#include "SrcInfoConsumer.h"

using namespace srcinfo_plugin;

SourceInfoConsumer::SourceInfoConsumer(const CompilerInstance &ci, MatcherOption &option) : CodemindASTConsumer(ci) {
  for (auto opt : option) {
    switch (opt.first) {
      case ItemAttr::Prebuild      : InitPrebuild(opt.second);      break;
      case ItemAttr::Build         : InitBuild(opt.second);         break;
      case ItemAttr::CodeGenerator : InitCodeGenerator(opt.second); break;
      case ItemAttr::Metric        : InitMetric(opt.second);        break;
    }
  }
}

void SourceInfoConsumer::HandleTranslationUnit(ASTContext &Ctx) {
  CodemindASTConsumer::HandleTranslationUnit(Ctx);
  for (auto item : items)
    item.second.get()->finalize();
}

SourceInfoItems *SourceInfoConsumer::getSourceInfoItem(ItemAttr attr) {
  return items[attr].get();
}

void SourceInfoConsumer::setSourceInfoItem(ItemAttr attr, shared_ptr<SourceInfoItems> item) {
  items[attr] = item;
}