if(NOT CLANG_BUILD_PLUGIN)
  set_property(DIRECTORY PROPERTY EXCLUDE_FROM_ALL ON)
endif()

if(IS_SYMLINK ${CMAKE_CURRENT_SOURCE_DIR}/SrcInfo)
  add_subdirectory(SrcInfo)
endif()
