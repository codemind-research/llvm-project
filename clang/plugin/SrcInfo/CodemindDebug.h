#ifndef CODEMIND_DEBUG
#define CODEMIND_DEBUG

#include <iostream>

#ifdef _WIN32
  #define NOMINMAX
  #include <Windows.h>
  #include <DbgHelp.h>
  #undef CALLBACK
#elif __linux__
  #include <execinfo.h>
#endif

namespace codemind_debug {
  using namespace std;

  void printCallStack() {
    #define BUFFER_SIZE 1024
    void *buffer[BUFFER_SIZE];

    #ifdef _WIN32
      #define SYMBOL_SIZE 1024;
      DWORD displacement;
      DWORD64 address;
      IMAGEHLP_LINE64 line;
      line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
      const unsigned symbol_size = SYMBOL_SIZE;
      SYMBOL_INFO *symbol = (SYMBOL_INFO *)malloc(sizeof(SYMBOL_INFO) + sizeof(char) * (symbol_size - 1));
      symbol->MaxNameLen = symbol_size;
      symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
      HANDLE process = GetCurrentProcess();
      SymInitialize(process, NULL, TRUE);
      int count = CaptureStackBackTrace(0, BUFFER_SIZE, buffer, NULL);
      cerr << "Found Call Stack: " << count << "\n";
      for (unsigned i = 0; i < count; i++) {
        displacement = 0;
        address = reinterpret_cast<DWORD64>(buffer[i]);
        SymFromAddr(process, address, NULL, symbol);
        if (SymGetLineFromAddr64(process, address, &displacement, &line))
          cerr << "  " << line.FileName << ":" << line.LineNumber << "(" << symbol->Name << ")" << " [" << format("0x%x", symbol->Address) << "]\n";
        else
          cerr << "  " << "AnonymousFile(" << symbol->Name << ")" << " [" << format("0x%x", symbol->Address) << "]\n";
      }
      free(symbol);
    #elif __linux__
      int count = backtrace(buffer, BUFFER_SIZE);
      char **strings = backtrace_symbols(buffer, count);
      cerr << "Found Call Stack: " << count << "\n";
      if (strings != NULL) {
        for (int i = 0; i < count; i++)
          cerr << "  " << strings[i] << "\n";
        free(strings);
      } else
        cerr << "  " << "trace symbols print error" << "\n";
    #endif
  }
}

#endif