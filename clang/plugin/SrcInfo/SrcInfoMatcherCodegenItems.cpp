#include "llvm/ADT/APFloat.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/FileSystem.h"
#include "clang/Lex/Lexer.h"
#include "clang/AST/DeclTemplate.h"

#include <fstream>
#include <functional>
#include <vector>
#include <tuple>

#include "cxxinfo.pb.h"
#include "funinfo.pb.h"
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
    info_path = str;
}

void CodegenItems::finalize() {
  SourceManager &SourceMgr = consumer->getSourceManager();
  int id = 1; // protobuf3에서 default와 NoneValue가 차이가 없어 1부터 시작
  cxxinfo::Info hi;
  funinfo::Funcs mi;
  map<string, int> file_map;
  map<string, cxxinfo::Namespace*> name_map;
  map<string, int> type_map;
  map<const RecordDecl*, cxxinfo::Record*> record_map;

  auto setHighlanderLoc = [&](cxxinfo::Loc *loc, int line, int col) {
    loc->set_line(line);
    loc->set_col(col);
  };
  auto setHighlanderRange = [&](cxxinfo::Range *range, int sline, int scol, int eline, int ecol) {
    range->set_sline(sline);
    range->set_scol(scol);
    range->set_eline(eline);
    range->set_ecol(ecol);
  };
  auto setHighlanderVar = [&](cxxinfo::TVar *var, const ValueDecl *vd, function<int(QualType)>func) {
    *var->mutable_name() = vd->getNameAsString();
    var->set_type_id(func(vd->getType()));
  };
  auto setHighlanderNamespace = [&](const DeclContext *dc) {
    auto key = getQualifiedNameString(dc);
    if (name_map.find(key) == name_map.end()) {
      name_map[key] = hi.add_namespaces();
      vector<const DeclContext*> list;
      while (isa_and_nonnull<NamedDecl>(dc) && find(list.begin(), list.end(), dc) == list.end()) {
        list.push_back(dc);
        dc = dc->getParent();
        if (!dc->isRecord())
          dc = dc->getEnclosingNamespaceContext();
      }
      reverse(list.begin(), list.end());
      for (auto dc : list) {
        auto name = cast<NamedDecl>(dc)->getNameAsString();
        if (auto trd = dyn_cast<ClassTemplateSpecializationDecl>(dc)) {
          auto vstr = getTemplateArgumentsToVector<string>(trd->getTemplateArgs().asArray(),
                                                           [](QualType qt){ return getQualifiedTypeString(qt); });
          name += "<" + VectorToString<string>(vstr, [](string e){ return e; }, ",") + ">";
        }
        *name_map[key]->add_qname() = name;
      }
    }
    return name_map[key];
  };
  function<int(QualType)> setHighlanderType = [&](QualType qt) {
    auto getEnclosingContext = [](QualType qt) {
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
      return dc;
    };
    function<void(cxxinfo::TypeInfo*, QualType)> setTypeData = [&](cxxinfo::TypeInfo *type, QualType qt) {
      /*
        type 분석 순서 : 원형을 제외한 모든 type을 파생으로 구분하기 위함 (예:void*)
          array -> pointer -> reference -> const -> record(class -> union -> struct) -> ...
      */
      if (qt->isArrayType()) {
        auto tarray = type->mutable_tarray();
        while (qt->isArrayType()) {
          if (auto type = dyn_cast<ConstantArrayType>(qt->getAsArrayTypeUnsafe()))
            tarray->add_idxs(*(type->getSize().getRawData()));
          qt = qt->getAsArrayTypeUnsafe()->getElementType();
        }
        tarray->set_type_id(setHighlanderType(qt));
      } else if (qt->isPointerType()) {
        auto tptr = type->mutable_tptr();
        tptr->set_is_const(qt.isLocalConstQualified());
        tptr->set_type_id(setHighlanderType(qt->getPointeeType()));
      } else if (qt->isLValueReferenceType()) {
        auto tlref = type->mutable_tlref();
        tlref->set_type_id(setHighlanderType(qt->getPointeeType()));
      } else if (qt->isRValueReferenceType()) {
        auto trref = type->mutable_trref();
        trref->set_type_id(setHighlanderType(qt->getPointeeType()));
      } else if (qt.isLocalConstQualified()) {
        auto tconst = type->mutable_tconst();
        tconst->set_type_id(setHighlanderType(qt.getLocalUnqualifiedType()));
      } else if (qt->isRecordType()) {
        vector<int> tparams;
        if (auto trd = dyn_cast<ClassTemplateSpecializationDecl>(qt->getAsRecordDecl()))
          tparams = getTemplateArgumentsToVector<int>(trd->getTemplateArgs().asArray(), setHighlanderType);
        if (qt->isClassType()) {
          auto tclass = type->mutable_tclass();
          *tclass->mutable_name() = qt->getAsRecordDecl()->getNameAsString();
          for (auto param : tparams)
            tclass->add_tparams(param);
          for (auto field : qt->getAsRecordDecl()->fields())
            setHighlanderVar(tclass->add_flds(), field, setHighlanderType);
        } else if (qt->isUnionType()) {
          auto tunion = type->mutable_tunion();
          *tunion->mutable_name() = qt->getAsRecordDecl()->getNameAsString();
          for (auto param : tparams)
            tunion->add_tparams(param);
          for (auto field : qt->getAsRecordDecl()->fields())
            setHighlanderVar(tunion->add_flds(), field, setHighlanderType);
        } else {
          auto tstruct = type->mutable_tstruct();
          *tstruct->mutable_name() = qt->getAsRecordDecl()->getNameAsString();
          for (auto param : tparams)
            tstruct->add_tparams(param);
          for (auto field : qt->getAsRecordDecl()->fields())
            setHighlanderVar(tstruct->add_flds(), field, setHighlanderType);
        }
      } else if (const auto *bt = dyn_cast_or_null<BuiltinType>(qt.getTypePtr())) {
        auto &context = consumer->getASTContext();
        if (bt->isVoidType())
          type->mutable_tvoid();
        else if (bt->isSignedInteger()) {
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
        } else {
          auto tbuiltin = type->mutable_tbuiltin();
          *tbuiltin->mutable_name() = qt.getAsString();
        }
      } else if (auto dt = dyn_cast_or_null<DecltypeType>(qt.getTypePtr())) {
        auto tnamed = type->mutable_tnamed();
        *tnamed->mutable_name() = qt.getAsString();
        tnamed->set_type_id(setHighlanderType(dt->desugar()));
      } else if (auto tt = dyn_cast_or_null<TypedefType>(qt.getTypePtr())) {
        auto tnamed = type->mutable_tnamed();
        *tnamed->mutable_name() = tt->getDecl()->getNameAsString();
        tnamed->set_type_id(setHighlanderType(tt->desugar()));
      } else if (auto et = dyn_cast_or_null<ElaboratedType>(qt.getTypePtr())) {
        setTypeData(type, et->desugar());
      } else if (auto pt = dyn_cast_or_null<ParenType>(qt.getTypePtr())) {
        // parentheses type
        setTypeData(type, pt->getInnerType());
      } else if (auto ft = dyn_cast_or_null<FunctionProtoType>(qt.getTypePtr())) {
        auto tfunc = type->mutable_tfunc();
        tfunc->set_retty(setHighlanderType(ft->getReturnType()));
        tfunc->set_is_varg(ft->isVariadic());
        for (auto param : ft->param_types())
          tfunc->add_args(setHighlanderType(param));
      } else if (auto tst = dyn_cast_or_null<TemplateSpecializationType>(qt.getTypePtr())) {
        // Record 이후에 동작할 경우 Using처리된 type
        auto tnamed = type->mutable_tnamed();
        *tnamed->mutable_name() = tst->getTemplateName().getAsTemplateDecl()->getNameAsString();
        tnamed->set_type_id(setHighlanderType(tst->desugar()));
        for (auto param : tst->template_arguments())
          tnamed->add_tparams(setHighlanderType(param.getAsType()));
      } else if (auto sts = dyn_cast_or_null<SubstTemplateTypeParmType>(qt.getTypePtr())) {
        setTypeData(type, sts->desugar());
      } else {
        outs() << "unknown type dump:" << "\n";
        qt->dump();
      }
    };
    const string key = getQualifiedTypeString(qt);
    if (type_map.find(key) == type_map.end()) {
      type_map[key] = id++;
      auto type = setHighlanderNamespace(getEnclosingContext(qt))->add_type();
      type->set_id(type_map[key]);
      setTypeData(type, qt);
    }
    return type_map[key];
  };
  auto setHighlanderFunction = [&](cxxinfo::Function *func, const FunctionDecl *fd) {
    func->set_id(id++);
    func->set_name(fd->getNameAsString());
    func->set_is_varg(fd->isVariadic());
    if (auto list = fd->getTemplateSpecializationArgs()) {
      for (auto param : getTemplateArgumentsToVector<int>(list->asArray(), setHighlanderType))
        func->add_tparams(param);
    }
    func->set_ret_type(setHighlanderType(fd->getReturnType()));
    for (auto param : fd->parameters())
      setHighlanderVar(func->add_params(), param, setHighlanderType);
    for (auto redecl : fd->redecls()) {
      setHighlanderLoc(func->add_decl_pos(),
                       SourceMgr.getExpansionLineNumber(redecl->getBeginLoc()),
                       SourceMgr.getExpansionColumnNumber(redecl->getBeginLoc()));
    }
    if (fd->hasBody()) {
      setHighlanderRange(func->mutable_body_range(),
                         SourceMgr.getExpansionLineNumber(fd->getBody()->getBeginLoc()),
                         SourceMgr.getExpansionColumnNumber(fd->getBody()->getBeginLoc()),
                         SourceMgr.getExpansionLineNumber(fd->getBody()->getEndLoc()),
                         SourceMgr.getExpansionColumnNumber(fd->getBody()->getEndLoc()));
      auto filename = SourceMgr.getPresumedLoc(fd->getBody()->getBeginLoc()).getFilename();
      if (file_map.find(filename) == file_map.end()) {
        file_map[filename] = id++;
        (*hi.mutable_files())[file_map[filename]] = filename;
      }
      func->set_defined_file_id(file_map[filename]);
    }
    return func->id();
  };
  function<cxxinfo::Record*(const RecordDecl*)> setHighlanderRecord = [&](const RecordDecl *rd) {
    if (record_map.find(rd) == record_map.end()) {
      record_map[rd] = setHighlanderNamespace(rd->getParent())->add_records();
      record_map[rd]->set_id(id++);
      record_map[rd]->set_type_id(setHighlanderType(rd->getTypeForDecl()->getCanonicalTypeInternal()));
      setHighlanderRange(record_map[rd]->mutable_body_range(),
                         SourceMgr.getExpansionLineNumber(rd->getBraceRange().getBegin()),
                         SourceMgr.getExpansionColumnNumber(rd->getBraceRange().getBegin()),
                         SourceMgr.getExpansionLineNumber(rd->getBraceRange().getEnd()),
                         SourceMgr.getExpansionColumnNumber(rd->getBraceRange().getEnd()));
      if (auto crd = dyn_cast<CXXRecordDecl>(rd)) {
        for (auto base : crd->bases()) 
          record_map[rd]->add_parents(setHighlanderRecord(base.getType()->getAsRecordDecl())->id());
        for (auto decl : crd->decls()) {
          if (isa<AccessSpecDecl>(decl)) {
            setHighlanderRange(record_map[rd]->add_access_specifiers(),
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
    if (auto md = dyn_cast<CXXMethodDecl>(decl)) {
      auto rd = md->getParent();
      auto method = setHighlanderRecord(rd)->add_methods();
      setHighlanderFunction(method->mutable_func(), md);
      method->set_is_virtual(md->isVirtual());
      // is_defined_in_class_decl
      // pure_virtual_pos

      // setHighlanderRange(method->mutable_pure_virtual_pos(),
      //                    SourceMgr.getExpansionLineNumber(md->getBeginLoc()),
      //                    SourceMgr.getExpansionColumnNumber(md->getBeginLoc()),
      //                    SourceMgr.getExpansionLineNumber(md->getEndLoc()),
      //                    SourceMgr.getExpansionColumnNumber(md->getEndLoc()));

      // outs() << md->getQualifiedNameAsString() <<  "  " << md->isPure() << "  ";
      // outs() << format("%d:%d-%d:%d", SourceMgr.getExpansionLineNumber(md->getBeginLoc()),
      //                                 SourceMgr.getExpansionColumnNumber(md->getBeginLoc()),
      //                                 SourceMgr.getExpansionLineNumber(md->getEndLoc()),
      //                                 SourceMgr.getExpansionColumnNumber(md->getEndLoc()));
      // outs() << "\n";
      // auto func = mi.add_funcs();
      // auto ns = setHighlanderNamespace(rd->getParent());
      
    } else if (auto fd = dyn_cast<FunctionDecl>(decl)) {
      auto ns = setHighlanderNamespace(decl->getDeclContext()->getParent());
      setHighlanderFunction(ns->add_funcs(), fd);
    } else if (auto vd = dyn_cast<VarDecl>(decl)) {
      auto ns = setHighlanderNamespace(decl->getDeclContext()->getParent());
      setHighlanderVar(ns->add_gvars(), vd, setHighlanderType);
    }
  }
  fstream output(info_path, ios::out | ios::trunc | ios::binary);
  if (!hi.SerializeToOstream(&output))
    writeTo(errs(), "Failed to write", "\n");
}

void CodegenItems::addItem(const NamedDecl *nd) {
  cached_decl.insert(nd);
}