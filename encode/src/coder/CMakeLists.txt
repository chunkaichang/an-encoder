cmake_minimum_required(VERSION 2.8)

add_library(coder ProfiledCoder.cpp
                  CallHandler.cpp
                  GEPExpander.cpp
                  InterfaceHandler.cpp
                  UsesVault.cpp)

# Link against LLVM libraries
target_link_libraries(coder ${llvm_libs})

if ("$LLVM_ENABLE_RTTI" MATCHES "ON")
  set(RTTI -frtti)
else()
  set(RTTI -fno-rtti)
endif()

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=${USE_CXX11_ABI})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 ${RTTI}")
