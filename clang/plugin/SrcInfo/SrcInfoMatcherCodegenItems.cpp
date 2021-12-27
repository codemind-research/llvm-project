#include "llvm/ADT/APFloat.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/FileSystem.h"
#include "clang/Lex/Lexer.h"

#include <functional>
#include <vector>

#include "CodemindUtils.h"
#include "SrcInfoMatcherCodegenItems.h"

using namespace codemind_utils;
using namespace srcinfo_matcher_codegen;

CodegenItems::CodegenItems(SourceInfoConsumer *ci) : consumer(ci) {
}

void CodegenItems::finalize() {
}

size_t CodegenItems::addType(QualType qt) {
  auto key = getQualifiedTypeString(qt);
  auto it = cached_type.find(key);
  if (it == cached_type.end()) {
    // 재귀 특성을 갖는 type처리를 위해 cache를 먼저 처리
    // struct list {
    //   :
    //   list *next;
    // };
    auto id = cached_type.size();
    cached_type[key] = id;

    string group = "", comment = "";
    vector<size_t> args;
    if (qt->isArrayType()) {
      group = "a";
      args.push_back(addType(qt->getAsArrayTypeUnsafe()->getElementType()));
      if (auto type = dyn_cast<ConstantArrayType>(qt->getAsArrayTypeUnsafe()))
        args.push_back(*(type->getSize().getRawData()));
    } else if (qt->isPointerType() && qt.isLocalConstQualified()) {
      group = "pc";
      args.push_back(addType(qt.getLocalUnqualifiedType()));
    } else if (qt->isAnyPointerType()) {
      group = "p";
      args.push_back(addType(qt->getPointeeType()));
    } else if (qt->isReferenceType()) {
      group = "r";
      args.push_back(addType(qt->getPointeeType()));
    } else if (qt.isLocalConstQualified()) {
      group = "c";
      args.push_back(addType(qt.getLocalUnqualifiedType()));
    } else if (qt->isRecordType()) {
      auto rd = qt->getAsRecordDecl();
      if (isa<ClassTemplateSpecializationDecl>(rd)) { group = "tp"; }
      else if (rd->isClass())  { group = "cl"; }
      else if (rd->isStruct()) { group = rd->getNameAsString().empty() ? "ast" : "st"; }
      else if (rd->isUnion())  { group = rd->getNameAsString().empty() ? "aun" : "un"; }
      for (auto fd : rd->fields())
        args.push_back(addType(fd->getType()));
      
      if (rd->getNameAsString().empty()) {
        auto &SourceMgr = consumer->getSourceManager();
        auto tokenLoc = Lexer::getLocForEndOfToken(rd->getBeginLoc(), 0, SourceMgr, consumer->getLangOpts());
        unsigned line = SourceMgr.getExpansionLineNumber(tokenLoc);
        unsigned column = SourceMgr.getExpansionColumnNumber(tokenLoc);
        comment = to_string(line) + "'" + to_string(column);
      }
    } else  if (const auto *bt = dyn_cast_or_null<BuiltinType>(qt->getCanonicalTypeInternal().getTypePtr())) {
      if (bt->isInteger()) {
        if (bt->isSignedInteger())        { group = "si"; }
        else if (bt->isUnsignedInteger()) { group = "ui"; }
        args.push_back(consumer->getASTContext().getTypeAlign(qt));
      } else if (bt->isFloatingPoint()) {
        group = "fp";
        args.push_back(APFloatBase::semanticsSizeInBits(consumer->getASTContext().getFloatTypeSemantics(qt)));
      } else
        group = "v";
    }
    writeTo(outs(),
            "TYPE-",
            id, ";",
            group, ";",
            (!args.empty() ? VectorToString<size_t>(args, "'") + ";" : ""),
            (!comment.empty() ? comment + ";" : ""),
            key, "\n");
    return id;
  } else
    return it->second;
}

size_t CodegenItems::addItem(const NamedDecl *nd) {
  auto it = cached_function.find(nd);
  if (it == cached_function.end()) {
    if (auto fd = dyn_cast<FunctionDecl>(nd)) {
      struct code_pos { unsigned line = 0, column = 0; } decl, body_st, body_ed;
      vector<size_t> atype;
      auto rtype = addType(fd->getReturnType());
      auto &SourceMgr = consumer->getSourceManager();
      decl.line = SourceMgr.getExpansionLineNumber(fd->getLocation());
      decl.column = SourceMgr.getExpansionColumnNumber(fd->getLocation());
      if (fd->hasBody()) {
        body_st.line = SourceMgr.getExpansionLineNumber(fd->getBody()->getBeginLoc());
        body_ed.line = SourceMgr.getExpansionLineNumber(fd->getBody()->getEndLoc());
        body_st.column = SourceMgr.getExpansionColumnNumber(fd->getBody()->getBeginLoc());
        body_ed.column = SourceMgr.getExpansionColumnNumber(fd->getBody()->getEndLoc());
      }
      for (unsigned i = 0; i < fd->getNumParams(); i++)
        atype.push_back(addType(fd->getParamDecl(i)->getType()));

      auto id = cached_function.size();
      cached_function[nd] = id;
      writeTo(outs(),
              "FUNC-",
              id, ";",
              SourceMgr.getPresumedLoc(fd->getLocation()).getFilename(), ";",
              decl.line, "'", decl.column, ";",
              fd->hasBody(), "'", body_st.line, "'", body_st.column, "'", body_ed.line, "'", body_ed.column, ";",
              fd->isVariadic(), ";",
              rtype, ";",
              (!atype.empty() ? VectorToString<size_t>(atype, "'") + ";" : ""),
              getQualifiedNameString(fd), "\n");
      return id;
    } else
      return -1;
  } else
    return it->second;
}