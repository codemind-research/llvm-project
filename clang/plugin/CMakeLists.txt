if(NOT CLANG_BUILD_PLUGIN)
  set_property(DIRECTORY PROPERTY EXCLUDE_FROM_ALL ON)
endif()

add_subdirectory(SrcInfo)
