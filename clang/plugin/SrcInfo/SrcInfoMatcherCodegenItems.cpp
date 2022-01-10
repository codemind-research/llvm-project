#include "llvm/ADT/APFloat.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/FileSystem.h"
#include "clang/Lex/Lexer.h"
#include "clang/AST/DeclTemplate.h"

#include <fstream>
#include <functional>
#include <vector>
#include <tuple>

// #define __TEST_MODE_
#ifndef __TEST_MODE_
#include "cxxinfo.pb.h"
#include "funinfo.pb.h"
#endif
#include "CodemindUtils.h"
#include "SrcInfoMatcherCodegenItems.h"

using namespace std;
using namespace clang;
using namespace codemind_utils;
using namespace srcinfo_matcher_codegen;

#ifndef __TEST_MODE_
using namespace highlander::proto;
#endif

CodegenItems::CodegenItems(SourceInfoConsumer *ci, set<string> detail) : consumer(ci) {
  for (auto str : detail)
    filename = str;
}

void CodegenItems::finalize() {
  #ifndef __TEST_MODE_
  SourceManager &SourceMgr = consumer->getSourceManager();
  int id = 0;
  cxxinfo::Info info;
  map<string, int> file_map;
  map<string, cxxinfo::Namespace*> name_map;
  map<string, int> type_map;
  map<const RecordDecl*, cxxinfo::Record*> record_map;

  auto setCodeGenLoc = [&](cxxinfo::Loc *loc, int line, int col) {
    loc->set_line(line);
    loc->set_col(col);
  };
  auto setCodeGenRange = [&](cxxinfo::Range *range, int sline, int scol, int eline, int ecol) {
    range->set_sline(sline);
    range->set_scol(scol);
    range->set_eline(eline);
    range->set_ecol(ecol);
  };
  auto setCodeGenVar = [&](cxxinfo::TVar *var, const ValueDecl *vd, function<int(QualType)>func) {
    *var->mutable_name() = vd->getNameAsString();
    var->set_type_id(func(vd->getType()));
  };
  auto setCodeGenNamespace = [&](string qname) {
    if (name_map.find(qname) == name_map.end()) {
      name_map[qname] = info.add_namespaces();
      name_map[qname]->add_qname(qname);
    }
    return name_map[qname];
  };
  function<int(QualType)> setCodeGenType = [&](QualType qt) {
    auto getNamespace = [](QualType qt) {
      while (qt->isArrayType())
        qt = qt->getAsArrayTypeUnsafe()->getElementType();
      while (qt->isPointerType() || qt->isReferenceType())
        qt = qt->getPointeeType();
      qt = qt.getLocalUnqualifiedType();

      DeclContext *dc = nullptr;
      if (const auto *tt = dyn_cast_or_null<TypedefType>(qt.getTypePtr())) {
        dc = tt->getDecl()->getDeclContext();
        dc = dc->isRecord() ? dc->getOuterLexicalRecordContext()
                            : dc->getEnclosingNamespaceContext();
      } else if (qt->isRecordType())
        dc = qt->getAsRecordDecl()->getParent();
      return getQualifiedNameString(dc);
    };
    function<void(cxxinfo::TypeInfo*, QualType)> setTypeData = [&](cxxinfo::TypeInfo *type, QualType qt) {
      if (const auto *bt = dyn_cast_or_null<BuiltinType>(qt.getTypePtr())) {
        auto &context = consumer->getASTContext();
        if (bt->isSignedInteger()) {
          auto tsint = type->mutable_tsint();
          *tsint->mutable_name() = qt.getAsString();
          tsint->set_size(context.getTypeAlign(qt));
        } else if (bt->isUnsignedInteger()) {
          auto tuint = type->mutable_tuint();
          *tuint->mutable_name() = qt.getAsString();
          tuint->set_size(context.getTypeAlign(qt));
        } else if (bt->isFloatingPoint()) {
          auto tfloat = type->mutable_tfloat();
          *tfloat->mutable_name() = qt.getAsString();
          tfloat->set_size(APFloatBase::semanticsSizeInBits(context.getFloatTypeSemantics(qt)));
        } else
          type->mutable_tvoid();
      } else if (auto tt = dyn_cast_or_null<TypedefType>(qt.getLocalUnqualifiedType().getTypePtr())) {
        auto tnamed = type->mutable_tnamed();
        *tnamed->mutable_name() = tt->getDecl()->getNameAsString();
        tnamed->set_type_id(setCodeGenType(qt->getCanonicalTypeInternal()));
      } else if (qt->isArrayType()) {
        auto tarray = type->mutable_tarray();
        while (qt->isArrayType()) {
          if (auto type = dyn_cast<ConstantArrayType>(qt->getAsArrayTypeUnsafe()))
            tarray->add_idxs(*(type->getSize().getRawData()));
          qt = qt->getAsArrayTypeUnsafe()->getElementType();
        }
        tarray->set_type_id(setCodeGenType(qt));
      } else if (const auto *pt = dyn_cast_or_null<ParenType>(qt.getTypePtr())) { // parentheses type
        setTypeData(type, pt->getInnerType());
      } else if (const auto *ft = dyn_cast_or_null<FunctionProtoType>(qt.getTypePtr())) {
        auto tfunc = type->mutable_tfunc();
        tfunc->set_retty(setCodeGenType(ft->getReturnType()));
        tfunc->set_is_varg(ft->isVariadic());
        for (auto param : ft->param_types())
          tfunc->add_args(setCodeGenType(param));
      } else if (qt->isPointerType()) {
        auto tptr = type->mutable_tptr();
        tptr->set_is_const(qt.isLocalConstQualified());
        tptr->set_type_id(setCodeGenType(qt->getPointeeType()));
      } else if (qt->isLValueReferenceType()) {
        auto tlref = type->mutable_tlref();
        tlref->set_type_id(setCodeGenType(qt->getPointeeType()));
      } else if (qt->isRValueReferenceType()) {
        auto trref = type->mutable_trref();
        trref->set_type_id(setCodeGenType(qt->getPointeeType()));
      } else if (qt->isUnionType()) {
        auto tunion = type->mutable_tunion();
        *tunion->mutable_name() = qt->getAsRecordDecl()->getNameAsString();
        if (auto trd = dyn_cast<ClassTemplateSpecializationDecl>(qt->getAsRecordDecl())) {
          for (auto param : getTemplateArgumentListToVector<int>(&trd->getTemplateInstantiationArgs(), setCodeGenType))
            tunion->add_tparams(param);
        }
        for (auto field : qt->getAsRecordDecl()->fields())
          setCodeGenVar(tunion->add_flds(), field, setCodeGenType);
      } else if (qt->isStructureType()) {
        auto tstruct = type->mutable_tstruct();
        *tstruct->mutable_name() = qt->getAsRecordDecl()->getNameAsString();
        if (auto trd = dyn_cast<ClassTemplateSpecializationDecl>(qt->getAsRecordDecl())) {
          for (auto param : getTemplateArgumentListToVector<int>(&trd->getTemplateInstantiationArgs(), setCodeGenType))
            tstruct->add_tparams(param);
        }
        for (auto field : qt->getAsRecordDecl()->fields())
          setCodeGenVar(tstruct->add_flds(), field, setCodeGenType);
      } else if (qt->isClassType()) {
        auto tclass = type->mutable_tclass();
        *tclass->mutable_name() = qt->getAsRecordDecl()->getNameAsString();
        if (auto trd = dyn_cast<ClassTemplateSpecializationDecl>(qt->getAsRecordDecl())) {
          for (auto param : getTemplateArgumentListToVector<int>(&trd->getTemplateInstantiationArgs(), setCodeGenType))
            tclass->add_tparams(param);
        }
        for (auto field : qt->getAsRecordDecl()->fields())
          setCodeGenVar(tclass->add_flds(), field, setCodeGenType);
      } else if (qt.isLocalConstQualified()) {
        auto tconst = type->mutable_tconst();
        tconst->set_type_id(setCodeGenType(qt.getLocalUnqualifiedType()));
      }
    };
    const string key = getQualifiedTypeString(qt);
    if (type_map.find(key) == type_map.end()) {
      type_map[key] = id++;
      auto type = setCodeGenNamespace(getNamespace(qt))->add_type();
      type->set_id(type_map[key]);
      setTypeData(type, qt);
    }
    return type_map[key];
  };
  auto setCodeGenFunction = [&](cxxinfo::Function *func, const FunctionDecl *fd) {
    func->set_id(id++);
    func->set_name(fd->getNameAsString());
    func->set_is_varg(fd->isVariadic());
    if (auto list = fd->getTemplateSpecializationArgs()) {
      vector<int> tparams;
      tparams = getTemplateArgumentListToVector<int>(list, setCodeGenType);
      for (auto param : tparams)
        func->add_tparams(param);
    }
    func->set_ret_type(setCodeGenType(fd->getReturnType()));
    for (auto param : fd->parameters())
      setCodeGenVar(func->add_params(), param, setCodeGenType);
    for (auto redecl : fd->redecls()) {
      setCodeGenLoc(func->add_decl_pos(),
                    SourceMgr.getExpansionLineNumber(redecl->getBeginLoc()),
                    SourceMgr.getExpansionColumnNumber(redecl->getBeginLoc()));
    }
    if (fd->hasBody()) {
      setCodeGenRange(func->mutable_body_range(),
                      SourceMgr.getExpansionLineNumber(fd->getBody()->getBeginLoc()),
                      SourceMgr.getExpansionColumnNumber(fd->getBody()->getBeginLoc()),
                      SourceMgr.getExpansionLineNumber(fd->getBody()->getEndLoc()),
                      SourceMgr.getExpansionColumnNumber(fd->getBody()->getEndLoc()));
      auto filename = SourceMgr.getFilename(fd->getBody()->getBeginLoc()).str();
      if (file_map.find(filename) == file_map.end()) {
        file_map[filename] = id++;
        auto file = info.add_files();
        file->set_id(file_map[filename]);
        file->set_name(filename);
      }
      func->set_defined_file_id(file_map[filename]);
    }
  };
  function<cxxinfo::Record*(const RecordDecl*)> setCodeGenRecord = [&](const RecordDecl *rd) {
    if (record_map.find(rd) == record_map.end()) {
      auto qname = getQualifiedNameString(rd->getParent());
      record_map[rd] = setCodeGenNamespace(qname)->add_records();
      record_map[rd]->set_id(id++);
      record_map[rd]->set_type_id(setCodeGenType(rd->getTypeForDecl()->getCanonicalTypeInternal()));
      // body_range
      if (auto crd = dyn_cast<CXXRecordDecl>(rd)) {
        for (auto base : crd->bases())
          record_map[rd]->add_parents(setCodeGenRecord(base.getType()->getAsRecordDecl())->id());
        for (auto decl : crd->decls()) {
          if (isa<AccessSpecDecl>(decl)) {
            setCodeGenRange(record_map[rd]->add_access_specifiers(),
                            SourceMgr.getExpansionLineNumber(decl->getBeginLoc()),
                            SourceMgr.getExpansionColumnNumber(decl->getBeginLoc()),
                            SourceMgr.getExpansionLineNumber(decl->getEndLoc()),
                            SourceMgr.getExpansionColumnNumber(decl->getEndLoc()));
          }
        }
      }
    }
    return record_map[rd];
  };

  for (auto decl : cached_decl) {
    if (auto md = dyn_cast<clang::CXXMethodDecl>(decl)) {
      outs() << md->getQualifiedNameAsString() << "\n";
      auto method = setCodeGenRecord(md->getParent())->add_methods();
      setCodeGenFunction(method->mutable_func(), md);
      method->set_is_virtual(md->isVirtual());
      // setCodeGenRange(method->mutable_pure_virtual_pos(),
      //                 SourceMgr.getExpansionLineNumber(md->getBeginLoc()),
      //                 SourceMgr.getExpansionColumnNumber(md->getBeginLoc()),
      //                 SourceMgr.getExpansionLineNumber(md->getEndLoc()),
      //                 SourceMgr.getExpansionColumnNumber(md->getEndLoc()));

      // outs() << md->getQualifiedNameAsString() <<  "  " << md->isPure() << "  ";
      // outs() << format("%d:%d-%d:%d", SourceMgr.getExpansionLineNumber(md->getBeginLoc()),
      //                                 SourceMgr.getExpansionColumnNumber(md->getBeginLoc()),
      //                                 SourceMgr.getExpansionLineNumber(md->getEndLoc()),
      //                                 SourceMgr.getExpansionColumnNumber(md->getEndLoc()));
      // outs() << "\n";
    } else if (auto fd = dyn_cast<FunctionDecl>(decl)) {
      auto qname = getQualifiedNameString(decl->getDeclContext()->getParent());
      setCodeGenFunction(setCodeGenNamespace(qname)->add_funcs(), fd);
    } else if (auto vd = dyn_cast<VarDecl>(decl)) {
      auto qname = getQualifiedNameString(decl->getDeclContext()->getParent());
      setCodeGenVar(setCodeGenNamespace(qname)->add_gvars(), vd, setCodeGenType);
    }
  }
  fstream output(filename, ios::out | ios::trunc | ios::binary);
  if (!info.SerializeToOstream(&output))
    writeTo(errs(), "Failed to write", "\n");
  #endif
}

void CodegenItems::addItem(const NamedDecl *nd) {
  cached_decl.insert(nd);
}