add_llvm_loadable_module(CodingStyleCheck 
CodingStyleCheck.cpp
CodingStyleCheck.hpp
CustomPluginUtil.hpp
PLUGIN_TOOL clang
)

if(LLVM_ENABLE_PLUGINS AND (WIN32 OR CYGWIN))
  target_link_libraries(CodingStyleCheck PRIVATE
    clangAST
    clangBasic
    clangFrontend
    clangLex
    LLVMSupport
    )
endif()