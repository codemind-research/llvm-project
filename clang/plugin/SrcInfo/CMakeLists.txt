#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
                    SrcInfoMatcherMetric.cpp
                    SrcInfoMatcherMetricItems.cpp
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
