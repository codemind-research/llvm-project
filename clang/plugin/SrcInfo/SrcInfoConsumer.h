#ifndef SRCINFO_CONSUMER
#define SRCINFO_CONSUMER

#include "CodemindPlugin.h"

#include <map>
#include <memory>
#include <string>

namespace srcinfo_plugin {
  using namespace codemind_plugin;

  struct MatcherOption {
    bool pre_build = false;
    bool build = false;
    bool code_generator = false;
    string code_generated_path = "";
  };

  enum class ItemAttr {Prebuild, Build, CodeGenerator};

  class SourceInfoItems {
    public:
      virtual void finalize() = 0;
  };

  class SourceInfoConsumer : public CodemindASTConsumer {
    private:
      map<ItemAttr, shared_ptr<SourceInfoItems>> items;
    protected:
      void InitPrebuild();
      void InitBuild();
      void InitCodeGenerator(string path);
    public:
      SourceInfoConsumer(const CompilerInstance &ci, MatcherOption &option);

      void HandleTranslationUnit(ASTContext &Ctx) override;

      SourceInfoItems *getSourceInfoItem(ItemAttr attr);
      void setSourceInfoItem(ItemAttr attr, shared_ptr<SourceInfoItems> item);
  };
}

#endif