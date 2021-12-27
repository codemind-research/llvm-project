#include "CodemindUtils.h"

namespace codemind_utils {
  /* declaration */
  vector<string> getTemplateArgumentToString(const TemplateArgument &ta);
  string getTemplateArgumentListToString(const TemplateArgumentList *list);
  string getTemplateParameterListToString(const TemplateParameterList *list);
  string getQualifiedNameString(const DeclContext *dc, bool showTmpl);
  string getNamespaceName(const DeclContext *dc);
  string getRecordName(const RecordDecl *rd, bool showTmpl);
  string getFunctionName(const FunctionDecl *fd, bool showTmpl);
  string getMethodName(const CXXMethodDecl *md, bool showTmpl);

  /* implementation */
  string getQualifiedTypeString(QualType qt, bool showTmpl) {
    string reference = "", array = "";
    bool prefix_const = false, suffix_const = false;
    if (qt->isArrayType()) {
      array = " [";
      if (auto type = dyn_cast<ConstantArrayType>(qt->getAsArrayTypeUnsafe()))
        array += to_string(*(type->getSize().getRawData()));
      array += "]";
      qt = qt->getAsArrayTypeUnsafe()->getElementType();
    }
    if (qt->isAnyPointerType() && qt.isLocalConstQualified()) {
      suffix_const = true;
      qt = qt.getLocalUnqualifiedType();
    }
    while (qt->isPointerType() || qt->isReferenceType()) {
      reference += (qt->isPointerType() ? "*" : "&");
      qt = qt->getPointeeType();
    }
    if (qt.isLocalConstQualified()) {
      prefix_const = true;
      qt = qt.getLocalUnqualifiedType();
    }

    return ((prefix_const) ? "const " : "") +
           ((qt->isRecordType()) ? getRecordName(qt->getAsRecordDecl(), showTmpl) : (qt.getAsString())) +
           reference + array +
           ((suffix_const) ? " const" : "");
  }

  string getQualifiedNameString(const NamedDecl *decl, bool showTmpl) {
    if (decl == nullptr)
      return "";
    auto result = decl->getNameAsString();
    if (auto rd = dyn_cast<RecordDecl>(decl))
      result = getRecordName(rd, showTmpl);
    else if (auto md = dyn_cast<CXXMethodDecl>(decl))
      result = getMethodName(md, showTmpl);
    else if (auto fd = dyn_cast<FunctionDecl>(decl))
      result = getFunctionName(fd, showTmpl);
    return result;
  }

  vector<string> getTemplateArgumentToString(const TemplateArgument &ta) {
    vector<string> result;
    switch (ta.getKind()) {
      case TemplateArgument::ArgKind::Type : {
        QualType qt = ta.getAsType();
        result.push_back(getQualifiedTypeString(qt));
        break;
      }
      case TemplateArgument::ArgKind::Integral : {
        QualType qt = ta.getIntegralType();
        result.push_back(getQualifiedTypeString(qt));
        break;
      }
      case TemplateArgument::ArgKind::Pack : {
        auto pack = ta.pack_elements();
        for (unsigned i = 0; i < pack.size(); i++) {
          auto packs = getTemplateArgumentToString(pack[i]);
          result.insert(result.end(), packs.begin(), packs.end());
        }
        break;
      }
      default :
        break;
    }
    return result;
  }

  string getTemplateArgumentListToString(const TemplateArgumentList *list) {
    if (list == nullptr)
      return "";
    vector<string> args;
    for (unsigned i = 0; i < list->size(); i++ ) {
      auto arg = getTemplateArgumentToString(list->get(i));
      if (!arg.empty())
        args.insert(args.end(), arg.begin(), arg.end());
    }

    return "<" + VectorToString<string>(args, [](string arg){ return arg; }, ",") + ">";
  }

  string getTemplateParameterListToString(const TemplateParameterList *list) {
    if (list == nullptr)
      return "";
    vector<string> args;
    for (unsigned i = 0; i < list->size(); i++ )
      args.push_back("type_" + to_string(i));

    return "<" + VectorToString<string>(args, [](string arg){ return arg; }, ",") + ">";
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

  string getRecordName(const RecordDecl *rd, bool showTmpl) {
    if (rd == nullptr)
      return ""; 
    string result = "";
    auto ord = rd->getOuterLexicalRecordContext();
    result = rd == ord ? getNamespaceName(rd) : getRecordName(ord, showTmpl);
    result += (result.empty() ? "" : "::");
    result += (rd->getNameAsString().empty() ? "(" + to_string(rd->getID()) + ")" : rd->getNameAsString());
    if (showTmpl) {
      if (auto trd = dyn_cast<ClassTemplateSpecializationDecl>(rd))
        result += "<" + getTemplateArgumentListToString(&trd->getTemplateInstantiationArgs()) + ">";
      else if (auto list = rd->getDescribedTemplateParams())
        result += "<" + getTemplateParameterListToString(list) + ">";
    }
    return result;
  }

  string getFunctionName(const FunctionDecl *fd, bool showTmpl) {
    if (fd == nullptr)
      return "";
    string result = getNamespaceName(fd);
    result += ((result.empty()) ? "" : "::") + fd->getNameAsString();
    if (showTmpl) {
      if (auto list = fd->getTemplateSpecializationArgs()) {
        result += "<" + getTemplateArgumentListToString(list) + ">";
      } else if (auto list = fd->getDescribedTemplateParams())
        result += "<" + getTemplateParameterListToString(list) + ">";
    }
    return result;
  }

  string getMethodName(const CXXMethodDecl *md, bool showTmpl) {
    if (md == nullptr)
      return "";
    return getRecordName(md->getParent(), showTmpl) + "::" + md->getNameAsString();
  }
}