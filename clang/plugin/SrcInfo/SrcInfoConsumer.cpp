#include "SrcInfoConsumer.h"

using namespace srcinfo_plugin;

SourceInfoConsumer::SourceInfoConsumer(const CompilerInstance &ci, MatcherOption &option) : CodemindASTConsumer(ci) {
  if (option.pre_build)
    InitPrebuild();
  if (option.build)
    InitBuild();
  if (option.code_generator)
    InitCodeGenerator(option.code_generated_path);
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