#include "CodemindUtils.h"

#ifdef _WIN32
  // #define NOMINMAX
  // #include <Windows.h>
  // #include <DbgHelp.h>
  // #undef CALLBACK
#elif __linux__
  #include <execinfo.h>
#endif

namespace codemind_utils {
  void printCallStack(raw_fd_ostream &os) {
    #define BUFFER_SIZE 1024
    void *buffer[BUFFER_SIZE];

    #ifdef _WIN32
      // #define SYMBOL_SIZE 1024;
      // DWORD displacement;
      // DWORD64 address;
      // IMAGEHLP_LINE64 line;
      // line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
      // const unsigned symbol_size = SYMBOL_SIZE;
      // SYMBOL_INFO *symbol = (SYMBOL_INFO *)malloc(sizeof(SYMBOL_INFO) + sizeof(char) * (symbol_size - 1));
      // symbol->MaxNameLen = symbol_size;
      // symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
      // HANDLE process = GetCurrentProcess();
      // SymInitialize(process, NULL, TRUE);
      // int count = CaptureStackBackTrace(0, BUFFER_SIZE, buffer, NULL);
      // writeTo(os, "Found Call Stack: ", count, "\n");
      // for (unsigned i = 0; i < count; i++) {
      //   displacement = 0;
      //   address = reinterpret_cast<DWORD64>(buffer[i]);
      //   SymFromAddr(process, address, NULL, symbol);
      //   if (SymGetLineFromAddr64(process, address, &displacement, &line))
      //     writeTo(os, "  ", line.FileName, ":", line.LineNumber, "(", symbol->Name, ")", " [", format("0x%x", symbol->Address), "]", "\n");
      //   else
      //     writeTo(os, "  ", "AnonymousFile(", symbol->Name, ")", " [", format("0x%x", symbol->Address), "]", "\n");
      // }
      // free(symbol);
    #elif __linux__
      int count = backtrace(buffer, BUFFER_SIZE);
      char **strings = backtrace_symbols(buffer, count);
      writeTo(os, "Found Call Stack: ", count, "\n");
      if (strings != NULL) {
        for (int i = 0; i < count; i++)
          writeTo(os, "  ", strings[i], "\n");
        free(strings);
      } else
        writeTo(os, "  ", "trace symbols print error", "\n");
    #endif
  }

  string getQualTypeToString(QualType qt) {
    string prefix = "", suffix = "";
    while (qt->isPointerType() || qt->isReferenceType()) {
      suffix += (qt->isPointerType() ? "*" : "&");
      qt = qt->getPointeeType();
    }
    if (qt.isLocalConstQualified()) {
      prefix = "const ";
      qt = qt.getLocalUnqualifiedType();
    }
    return prefix + ((qt->isRecordType()) ? getRecordDeclToString(qt->getAsRecordDecl()) : (qt.getAsString())) + suffix;
  }

  string getTemplateArgumentToString(const TemplateArgument &ta) {
    string result = "";
    switch (ta.getKind()) {
      case TemplateArgument::ArgKind::Type : {
        QualType qt = ta.getAsType();
        result = getQualTypeToString(qt);
        break;
      }
      case TemplateArgument::ArgKind::Integral : {
        QualType qt = ta.getIntegralType();
        result = getQualTypeToString(qt);
        break;
      }
      case TemplateArgument::ArgKind::Pack : {
        auto pack = ta.pack_elements();
        for (unsigned i = 0; i < pack.size(); i++)
          result += ((i > 0) ? "," : "") + getTemplateArgumentToString(pack[i]);
        break;
      }
      default :
        break;
    }
    return result;
  }

  string getRecordDeclToString(const RecordDecl *rd) {
    if (rd == nullptr)
      return "";
    auto name = rd->getNameAsString();
    if (auto trd = dyn_cast<ClassTemplateSpecializationDecl>(rd)) {
      name += "<";
      for (unsigned i = 0; i < trd->getTemplateInstantiationArgs().size(); i++) {
        auto args = getTemplateArgumentToString(trd->getTemplateInstantiationArgs().get(i));
        if (args == "")
          return "";
        name += ((i > 0) ? "," : "") + args;
      }
      name += ">";
    } else if (rd->isTemplated()) {
      name += "<";
      auto templ = rd->getDescribedTemplateParams();
      for (unsigned i = 0; i < templ->size(); i++)
        name += ((i > 0) ? "," : "") + (string)"type" + to_string(i);
      name += ">";
    }
    return name;
  }
}