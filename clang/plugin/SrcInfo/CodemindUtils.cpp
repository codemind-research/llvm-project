#include "clang/AST/Type.h"

#include "CodemindUtils.h"

using namespace std;
using namespace clang;

namespace codemind_utils {
  /* declaration */
  string getTemplateArgumentsToString(ArrayRef<TemplateArgument> array);
  string getTemplateParametersToString(ArrayRef<const NamedDecl*> array);
  string getFunctionName(const FunctionDecl *fd, bool showTmpl);
  string getMethodName(const CXXMethodDecl *md, bool showTmpl);
  string getRecordName(const RecordDecl *rd, bool showTmpl);
  string getNamespaceName(const DeclContext *dc);

  /* implementation */
  string getQualifiedTypeString(QualType qt, bool showTmpl) {
    string reference = "", array = "";
    bool prefix_const = false, suffix_const = false;
    while (qt->isArrayType()) {
      array = "[";
      if (auto type = dyn_cast<ConstantArrayType>(qt->getAsArrayTypeUnsafe()))
        array += to_string(*(type->getSize().getRawData()));
      array += "]";
      qt = qt->getAsArrayTypeUnsafe()->getElementType();
    }
    if (qt->isAnyPointerType() && qt.isLocalConstQualified()) {
      suffix_const = true;
      qt = qt.getLocalUnqualifiedType();
    }
    while (!qt->isTypedefNameType() && (qt->isPointerType() || qt->isReferenceType())) {
      reference += (qt->isPointerType() ? "*" : (qt->isLValueReferenceType() ? "&" : "&&"));
      qt = qt->getPointeeType();
    }
    if (qt.isLocalConstQualified()) {
      prefix_const = true;
      qt = qt.getLocalUnqualifiedType();
    }

    string name = "";
    if (qt->isRecordType()) {
      name = getRecordName(qt->getAsRecordDecl(), showTmpl);
    } else if (auto tt = dyn_cast<TypedefType>(qt.getTypePtr())) {
      name = getQualifiedNameString(tt->getDecl()->getDeclContext(), showTmpl);
      name += (name.empty() ? "" : "::") + tt->getDecl()->getNameAsString();
    } else if (auto et = dyn_cast_or_null<ElaboratedType>(qt.getTypePtr())) {
      name = getQualifiedTypeString(et->desugar());
    } else if (auto pt = dyn_cast<ParenType>(qt.getTypePtr())) {
      name = getQualifiedTypeString(pt->getInnerType());
    } else if (auto ft = dyn_cast<FunctionProtoType>(qt.getTypePtr())) {
      vector<string> vstr;
      name = getQualifiedTypeString(ft->getReturnType());
      name += " (" + VectorToString<string>(vstr, [](string e){ return e; }, ",") + ")";
    } else if (auto tst = dyn_cast<TemplateSpecializationType>(qt.getTypePtr())) {
      vector<string> vstr;
      name = tst->getTemplateName().getAsTemplateDecl()->getNameAsString();
      name += getTemplateArgumentsToString(tst->template_arguments());
    } else 
      name = qt.getAsString();

    return ((prefix_const) ? "const " : "") + name +
           reference + ((!array.empty()) ? " " : "") + array +
           ((suffix_const) ? " const" : "");
  }

  string getQualifiedNameString(const DeclContext *decl, bool showTmpl) {
    if (decl == nullptr)
      return "";
    else if (auto nd = dyn_cast<NamespaceDecl>(decl))
      return getNamespaceName(nd);
    else if (auto rd = dyn_cast<RecordDecl>(decl))
      return getRecordName(rd, showTmpl);
    else if (auto md = dyn_cast<CXXMethodDecl>(decl))
      return getMethodName(md, showTmpl);
    else if (auto fd = dyn_cast<FunctionDecl>(decl))
      return getFunctionName(fd, showTmpl);
    else
      return "";
  }

  string getTemplateArgumentsToString(ArrayRef<TemplateArgument> array) {
    auto vstr = getTemplateArgumentsToVector<string>(array, [](QualType qt){ return getQualifiedTypeString(qt); });
    return "<" + VectorToString<string>(vstr, [](string e){ return e; }, ",") + ">";
  }

  string getTemplateParametersToString(ArrayRef<const NamedDecl*> array) {
    vector<string> vstr;
    for (unsigned i = 0; i < array.size(); i++ )
      vstr.push_back("type_" + to_string(i));
    return "<" + VectorToString<string>(vstr, [](string e){ return e; }, ",") + ">";
  }

  string getFunctionName(const FunctionDecl *fd, bool showTmpl) {
    if (fd == nullptr)
      return "";
    string result = getNamespaceName(fd);
    result += ((result.empty()) ? "" : "::") + fd->getNameAsString();
    if (showTmpl) {
      if (auto list = fd->getTemplateSpecializationArgs()) {
        result += getTemplateArgumentsToString(list->asArray());
      } else if (auto list = fd->getDescribedTemplateParams())
        result += getTemplateParametersToString(list->asArray());
    }
    return result;
  }

  string getMethodName(const CXXMethodDecl *md, bool showTmpl) {
    if (md == nullptr)
      return "";
    return getRecordName(md->getParent(), showTmpl) + "::" + md->getNameAsString();
  }

  string getRecordName(const RecordDecl *rd, bool showTmpl) {
    if (rd == nullptr)
      return ""; 
    string result = "";
    auto parent = rd->getParent();
    result = (parent != rd && parent->isRecord()) ? getRecordName(cast<RecordDecl>(parent), showTmpl) : getNamespaceName(rd);
    result += (result.empty() ? "" : "::");
    result += (rd->getNameAsString().empty() ? "(" + to_string(rd->getID()) + ")" : rd->getNameAsString());
    if (showTmpl) {
      if (auto trd = dyn_cast<ClassTemplateSpecializationDecl>(rd))
        result += getTemplateArgumentsToString(trd->getTemplateArgs().asArray());
      else if (auto list = rd->getDescribedTemplateParams())
        result += getTemplateParametersToString(list->asArray());
    }
    return result;
  }

  string getNamespaceName(const DeclContext *dc) {
    if (dc == nullptr)
      return "";
    vector<const DeclContext*> list;
    dc = dc->getEnclosingNamespaceContext();
    while (isa<NamespaceDecl>(dc) && find(list.begin(), list.end(), dc) == list.end()) {
      list.push_back(dc);
      dc = dc->getParent()->getEnclosingNamespaceContext();
    }
    reverse(list.begin(), list.end());
    return VectorToString<const DeclContext*>(list, [](const DeclContext* arg){ return dyn_cast<NamespaceDecl>(arg)->getNameAsString(); }, "::");
  }
}