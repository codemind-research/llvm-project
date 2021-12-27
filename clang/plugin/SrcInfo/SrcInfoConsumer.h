#ifndef SRCINFO_CONSUMER
#define SRCINFO_CONSUMER

#include "CodemindPlugin.h"

#include <map>
#include <memory>
#include <set>
#include <string>

namespace srcinfo_plugin {
  using namespace codemind_plugin;

  enum class ItemAttr {Prebuild, Build, CodeGenerator, Metric};

  using MatcherOption = map<ItemAttr, set<string>>;

  class SourceInfoItems {
    public:
      virtual void finalize() = 0;
  };

  class SourceInfoConsumer : public CodemindASTConsumer {
    private:
      map<ItemAttr, shared_ptr<SourceInfoItems>> items;
    protected:
      void InitPrebuild(set<string> detail);
      void InitBuild(set<string> detail);
      void InitCodeGenerator(set<string> detail);
      void InitMetric(set<string> detail);
    public:
      SourceInfoConsumer(const CompilerInstance &ci, MatcherOption &option);

      void HandleTranslationUnit(ASTContext &Ctx) override;

      SourceInfoItems *getSourceInfoItem(ItemAttr attr);
      void setSourceInfoItem(ItemAttr attr, shared_ptr<SourceInfoItems> item);
  };
}

#endif