#include "llvm/ADT/APFloat.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/FileSystem.h"

#include <functional>

#include "CodemindUtils.h"
#include "SrcInfoMatcherMetricItems.h"

using namespace codemind_utils;
using namespace srcinfo_matcher_codegen;

MetricItems::MetricItems(SourceInfoConsumer *ci) : consumer(ci) {
}

void MetricItems::finalize() {
  for(auto e : gv) {
    writeTo(outs(), "GV-", e.first);
    for(auto var : e.second)
      writeTo(outs(), "'", var.first);
    writeTo(outs(), "\n");
  }
  for(auto e : cc) {
    int cnt = 0;
    for(auto cond : e.second)
      cnt += cond.second;
    writeTo(outs(), "CC-", e.first, "'", cnt, "\n");
  }
  for(auto x : mcd) {
    writeTo(outs(), "MCD-", x.first);
    for(auto y : x.second)
      writeTo(outs(), "'", y);
    writeTo(outs(), "\n");
  }
  for(auto x : cp) {
    writeTo(outs(), "CBO-", x.first);
    for(auto y : x.second)
      writeTo(outs(), "'", y);
    writeTo(outs(), "\n");
  }
}

size_t MetricItems::getNameID(const NamedDecl *nd) {
  auto it = cached_name.find(nd);
  if(it == cached_name.end()) {
    SourceManager &SourceMgr = consumer->getSourceManager();
    FullSourceLoc fsl(nd->getLocation(), SourceMgr);

    int id = cached_name.size();
    cached_name[nd] = id;
    writeTo(outs(),
            "NI-",
            id, "'",
            SourceMgr.getFilename(fsl.getFileLoc()).str(), "'",
            getQualifiedNameString(nd->getDeclContext(), false), "'",
            SourceMgr.getExpansionLineNumber(nd->getBeginLoc()), "'",
            SourceMgr.getExpansionColumnNumber(nd->getBeginLoc()), "\n");
    return id;
  } else
    return it->second;
}

size_t MetricItems::getFuncID(const FunctionDecl *fd) {
  auto it = cached_func.find(fd);
  if(it == cached_func.end()) {
    SourceManager &SourceMgr = consumer->getSourceManager();
    FullSourceLoc fsl(fd->getLocation(), SourceMgr);
    auto fun_type = getQualifiedTypeString(fd->getReturnType(), false) + " " + "(";
    for (unsigned i = 0; i < fd->getNumParams(); i++)
      fun_type += ((i > 0) ? "," : "") + getQualifiedTypeString(fd->getParamDecl(i)->getType(), false);
    fun_type += ")";

    auto id = cached_func.size();
    cached_func[fd] = id;
    writeTo(outs(),
            "FI-",
            id, "'",
            SourceMgr.getFilename(fsl.getFileLoc()).str(), "'",
            getQualifiedNameString(fd, false), "'",
            fun_type, "'",
            SourceMgr.getExpansionLineNumber(fd->getBeginLoc()), "'",
            SourceMgr.getExpansionColumnNumber(fd->getBeginLoc()), "\n");
    return id;
  } else
    return it->second;
}

void MetricItems::addGlobalVariable(const FunctionDecl *fd, const VarDecl *var) {
  gv[getFuncID(fd)][getNameID(var)]++;
}

void MetricItems::addCyclomaticComplexity(const FunctionDecl *fd, Stmt::StmtClass stmt, int complexity) {
  cc[getFuncID(fd)][stmt] += complexity;
}

void MetricItems::addModuleCircularDependency(const NamedDecl *src, const NamedDecl *dst) {
  if (src == dst)
    return;
  mcd[getNameID(src)].push_back(getNameID(dst));
}

void MetricItems::addCouplingBetwwenObjects(const RecordDecl *src, const RecordDecl *dst) {
  if (src == dst)
    return;
  cp[getNameID(src)].insert(getNameID(dst));
}