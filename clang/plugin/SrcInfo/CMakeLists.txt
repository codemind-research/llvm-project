#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if( NOT MSVC )
  if( NOT LLVM_REQUIRES_RTTI )
    if( NOT LLVM_REQUIRES_EH )
      set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/SrcInfo.exports)
    endif()
  endif()
endif()

add_llvm_library(SrcInfo
                  MODULE
                    CodemindUtils.cpp
                    SrcInfoAction.cpp
                    SrcInfoConsumer.cpp
                    SrcInfoMatcherPrebuild.cpp
                    SrcInfoMatcherBuild.cpp
                    SrcInfoMatcherBuildItems.cpp
                    SrcInfoMatcherCodegen.cpp
                    SrcInfoMatcherCodegenItems.cpp
                    ${LLVM_EXTERNAL_CLANG_SOURCE_DIR}/lib/ASTMatchers/ASTMatchersInternal.cpp
                  PLUGIN_TOOL
                    clang
                )

if(LLVM_ENABLE_PLUGINS AND (WIN32 OR CYGWIN))
  set(LLVM_LINK_COMPONENTS Support)
  clang_target_link_libraries(SrcInfo PRIVATE
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
   )
endif()