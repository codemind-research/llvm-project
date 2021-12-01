#include "CodemindUtils.h"
#include "SrcInfoMatcherBuildItems.h"

using namespace codemind_utils;
using namespace srcinfo_matcher_build;

void BuildItems::finalize() {
  for (auto x : classnames) {
    switch (get<TagTypeKind>(x)) {
      case TagTypeKind::TTK_Class :
        writeTo(outs(), "CN:", get<string>(x), "\n");
        break;
      case TagTypeKind::TTK_Struct :
      case TagTypeKind::TTK_Union :
        writeTo(outs(), "SN:", get<string>(x), "\n");
        break;
      default:
        break;
    }
  }
}

void BuildItems::addClassName(string name, TagTypeKind kind) {
  classnames.insert(make_tuple(name, kind));
}

bool BuildItems::RedundantCheck(string fn, string cn, string mn) {
  auto key = make_tuple(fn, cn, mn);
  auto has = red_chk_set.find(key) != red_chk_set.end();
  if (!has)
    red_chk_set.insert(key);
  return !has;
}