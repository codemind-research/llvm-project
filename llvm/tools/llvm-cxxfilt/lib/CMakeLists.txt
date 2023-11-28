add_llvm_library(
  LLVMCXXFilt
    SHARED
      llvm-cxxfilt.cpp
    LINK_COMPONENTS
      Demangle
      Support
)

target_compile_options(LLVMCXXFilt PUBLIC
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-fexceptions -fno-rtti>
  $<$<CXX_COMPILER_ID:MSVC>:/EHa /GR->
)
