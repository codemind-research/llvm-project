#ifndef CODEMIND_DEBUG
#define CODEMIND_DEBUG

#include <stdio.h>

#ifdef _WIN32
  #include <Windows.h>
  #include <DbgHelp.h>
  #pragma comment(lib, "dbghelp.lib")
#elif __linux__
  #include <execinfo.h>
#endif

namespace codemind_debug {
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
      unsigned count = CaptureStackBackTrace(0, BUFFER_SIZE, buffer, NULL);
      fprintf(stderr, "Found Call Stack: %d\n", count);
      for (unsigned i = 0; i < count; i++) {
        displacement = 0;
        address = reinterpret_cast<DWORD64>(buffer[i]);
        SymFromAddr(process, address, NULL, symbol);
        if (SymGetLineFromAddr64(process, address, &displacement, &line))
          fprintf(stderr, "  %s:%lx(%s) [0x%llx]\n", line.FileName, line.LineNumber, symbol->Name, symbol->Address);
        else
          fprintf(stderr, "  AnonymousFile(%s) [0x%llx]\n", symbol->Name, symbol->Address);
      }
      free(symbol);
    #elif __linux__
      int count = backtrace(buffer, BUFFER_SIZE);
      char **strings = backtrace_symbols(buffer, count);
      fprintf(stderr, "Found Call Stack: %d\n", count);
      if (strings != NULL) {
        for (int i = 0; i < count; i++)
          fprintf(stderr, "  %s\n", strings[i]);
        free(strings);
      } else
        fprintf(stderr, "  trace symbols print error\n");
    #endif
  }
}

#endif