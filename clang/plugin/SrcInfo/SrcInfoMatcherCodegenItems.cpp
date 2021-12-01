#include "llvm/ADT/APFloat.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/FileSystem.h"

#include "CodemindUtils.h"
#include "SrcInfoMatcherCodegenItems.h"

using namespace codemind_utils;
using namespace srcinfo_matcher_codegen;

enum class CommentKind {None, Begin, End, Line};

string getLinePrefix(CommentKind kind, unsigned int prefix_tab, unsigned int suffix_tab, string white_space = "  ") {
  string result = "";
  for (unsigned i = 0; i < prefix_tab; i++)
    result += white_space;
  switch (kind) {
    case CommentKind::Line  : result += "// "; break;
    case CommentKind::Begin : result += "{";   break;
    case CommentKind::End   : result += "}";   break;
    default : break;
  }
  for (unsigned i = 0; i < suffix_tab; i++)
    result += white_space;
  return result;
}

/* ParmDisplayOption */
ParmDisplayOption::ParmDisplayOption(initializer_list<ParmDisplay> list) : set(list) {
}
 
bool ParmDisplayOption::contain(ParmDisplay item) {
  return find(item) != end();
}

bool ParmDisplayOption::contain(initializer_list<ParmDisplay> items) {
  bool result = true;
  auto item = items.begin();
  while (result && item != items.end()) {
    result = contain(*item);
    item = next(item, 0);
  }
  return result;
}

/* CodegenItems */
CodegenItems::CodegenItems(SourceInfoConsumer *ci, string path) {
  error_code ec;
  consumer = ci;
  mangler = unique_ptr<MangleContext>(consumer->getASTContext().createMangleContext());
  file = make_unique<raw_fd_ostream>(((path != "") ? path : ("./cmpl." + string((consumer->getLangOpts().CPlusPlus) ? "cpp" : "c"))),
                                      ec,
                                      sys::fs::CreationDisposition::CD_CreateAlways,
                                      sys::fs::FileAccess::FA_Write,
                                      sys::fs::OpenFlags::OF_Text);
  if (ec) {
    file->close();
    file.reset(nullptr);
    throw runtime_error("File open error");
  }
}

CodegenItems::~CodegenItems() {
  if (file != nullptr) {
    file->flush();
    file->close();
  }
}

raw_fd_ostream &CodegenItems::getFileStream() {
  return ((file != nullptr) ? *file : outs());
}

string CodegenItems::getFunctionParamToString(const FunctionDecl *fd, ParmDisplayOption opt) {
  string result = "(";
  auto md = dyn_cast<CXXMethodDecl>(fd);
  unsigned size = fd->getNumParams();
  if (md != nullptr && opt.contain(ParmDisplay::Self))
    result += getRecordDeclToString(md->getParent()) + "* self" + ((size > 0) ? ", " : "");
  if (size > 0) {
    for (unsigned i = 0; i < size; i ++) {
      auto pd = fd->getParamDecl(i);
      result += ((i > 0) ? "," : "") + getQualTypeToString(pd->getType()) + " " + pd->getNameAsString();
    }
  } else if (fd->hasPrototype()) {
    if (!consumer->getLangOpts().CPlusPlus || (!opt.contain(ParmDisplay::Self) && opt.contain(ParmDisplay::ExplicitVoid)))
      result += "void";
  }
  result += ")";
  return result;
}

string CodegenItems::getMangleName(const NamedDecl *d) {
  string result = "";
  if (mangler->shouldMangleDeclName(d)) {
    GlobalDecl gd;
    raw_string_ostream buffer(result);
    gd = gd.getWithDecl(d);
    if (consumer->getTargetInfo().getCXXABI().isItaniumFamily()) {
      if (isa<CXXConstructorDecl>(d))
        gd = gd.getWithCtorType(CXXCtorType::Ctor_Base);
      else if (isa<CXXDestructorDecl>(d))
        gd = gd.getWithDtorType(CXXDtorType::Dtor_Base);
    } else {
      if (isa<CXXConstructorDecl>(d))
        gd = gd.getWithCtorType(CXXCtorType::Ctor_Complete);
      else if (auto dd = dyn_cast<CXXDestructorDecl>(d)) {
        if (gd.getDtorType() == CXXDtorType::Dtor_Complete && dd->getParent()->getNumVBases() == 0)
          gd = gd.getWithDtorType(CXXDtorType::Dtor_Base);
      }
    }
    mangler->mangleName(gd, buffer);
  } else if (auto id = d->getIdentifier())
    result = id->getName().str();
  return result;
}

void CodegenItems::generateCode(string name, QualType qt, const set<const NamedDecl*> &items) {
  raw_fd_ostream &os = getFileStream();
  while (qt->isPointerType() || qt->isReferenceType()) {
    name = (qt->isPointerType() ? "*" + name : name);
    qt = qt->getPointeeType();
  }
  if (auto rd = qt->getAsRecordDecl()) {
    for (auto fd : rd->fields()) {
      // if (items.find(fd) != items.end())
        generateCode("(" + name + ")." + fd->getNameAsString(), fd->getType(), items);
    }
  } else if (const auto *bt = dyn_cast_or_null<BuiltinType>(qt->getCanonicalTypeInternal().getTypePtr())) {
    if (bt->isInteger()) {
      writeTo(os, getLinePrefix(CommentKind::Line, 0, 1));
      writeTo(os, "USYM_I", consumer->getASTContext().getTypeAlign(qt), "(", name, ")");
      writeTo(os, ";\n");
    } else if (bt->isFloatingPoint()) {
      writeTo(os, getLinePrefix(CommentKind::Line, 0, 1));
      writeTo(os, "USYM_F", APFloatBase::semanticsSizeInBits(consumer->getASTContext().getFloatTypeSemantics(qt)), "(", name, ");");
      writeTo(os, ";\n");
    }
  }
}

void CodegenItems::generateParamCode(const pair<const FunctionDecl*, set<const NamedDecl*>> &data) {
  auto fd = data.first;
  for (unsigned i = 0; i < fd->getNumParams(); i ++) {
    auto pd = fd->getParamDecl(i);
    auto qt = pd->getType();
    if (qt->isPointerType() || qt->isReferenceType())
      generateCode(pd->getNameAsString(), qt, data.second);
  }
}

void CodegenItems::generateBody(const pair<const FunctionDecl*, set<const NamedDecl*>> &data) {
  QualType qt = data.first->getReturnType();
  if (!qt->isVoidType()) {
    raw_fd_ostream &os = getFileStream();
    string result = "result";
    bool isPointer = qt->isPointerType();
    if (isPointer) {
      writeTo(os, getLinePrefix(CommentKind::Line, 0, 1), "static bool flag = (bool)0;\n");
      writeTo(os, getLinePrefix(CommentKind::Line, 0, 1), "USYM_I1(flag);\n");
    }
    while (qt->isPointerType() || qt->isReferenceType()) {
      result = (qt->isReferenceType() ? "*" : "") + result;
      qt = qt->getPointeeType();
    }
    qt = qt.getLocalUnqualifiedType();
    writeTo(os, getLinePrefix(CommentKind::Line, 0, 1));
    writeTo(os, "static ", getQualTypeToString(qt));
    if (qt->isRecordType()) {
      writeTo(os, " = NULL\n");
    } else
      writeTo(os, result, " = (", getQualTypeToString(qt), ")0");
    writeTo(os, ";\n");
    generateParamCode(data);
    generateCode(result, qt, data.second);
    writeTo(os, getLinePrefix(CommentKind::Line, 0, 1), "return ");
    writeTo(os, (isPointer ? "(flag ? " : ""), result, (isPointer ? " : NULL)" : ""));
    writeTo(os, ";\n");
  } else
    generateParamCode(data);
}

void CodegenItems::finalize() {
  raw_fd_ostream &os = getFileStream();
  for (unsigned i = 0; i < data.size(); i++) {
    auto element = *next(data.begin(), i);
    auto fd = element.first;
    auto items = element.second;
    string rtype = getQualTypeToString(fd->getReturnType());
    string prefix = getLinePrefix(CommentKind::Line, 0, 0);
    writeTo(os, ((i > 0) ? "\n" : ""));
    writeTo(os, prefix, "[COYOTE STUB TARGET] ", rtype, " ",
                ((isa<CXXMethodDecl>(fd)) ? getRecordDeclToString(cast<CXXMethodDecl>(fd)->getParent()) + "::" : ""),
                fd->getNameAsString(),
                getFunctionParamToString(fd, ParmDisplayOption()),
                "\n");
    writeTo(os, prefix, "__attribute__((annotate(\"STUB:@\\\"", getMangleName(fd), "\\\")", "\n");
    writeTo(os, prefix, rtype, " __COYOTE_USER_STUB_", i,
                getFunctionParamToString(fd, ParmDisplayOption({ParmDisplay::Self, ParmDisplay::ExplicitVoid})),
                "\n");
    writeTo(os, prefix, "{", "\n");
    // generateBody(element);
    writeTo(os, prefix, "}", "\n");
  }
}

void CodegenItems::addRelation(const FunctionDecl *fd, const NamedDecl *nd) {
  data.insert(make_pair(fd, set<const NamedDecl*>()));
  if (nd != nullptr) {
    data[fd].insert(nd);
    if (auto fd = dyn_cast<FunctionDecl>(nd))
      addRelation(fd, nullptr);
  }
}