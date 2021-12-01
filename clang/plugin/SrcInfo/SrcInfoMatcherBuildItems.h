#ifndef SRCINFO_BUILDITEMS
#define SRCINFO_BUILDITEMS

#include "clang/AST/Type.h"

#include <set>
#include <string>
#include <tuple>

#include "SrcInfoConsumer.h"

namespace srcinfo_matcher_build {
  using namespace srcinfo_plugin;

  class BuildItems : public SourceInfoItems {
    private:
      set<tuple<string, TagTypeKind>> classnames;
      set<tuple<string, string, string>> red_chk_set;
    public:
      void finalize() override;
      void addClassName(string name, TagTypeKind kind);
      bool RedundantCheck(string fn, string cn, string mn);
  };
}

#endif