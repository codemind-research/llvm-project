if(NOT CLANG_BUILD_PLUGIN)
  set_property(DIRECTORY PROPERTY EXCLUDE_FROM_ALL ON)
endif()

if(EXISTS SrcInfo)
  add_subdirectory(SrcInfo)
endif()
