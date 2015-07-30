cmake_minimum_required(VERSION 2.8)

add_library(parser ProfileLexer.cpp
                   Parser.cpp
                   Profile.cpp)

if ("$LLVM_ENABLE_RTTI" MATCHES "ON")
  set(RTTI -frtti)
else()
  set(RTTI -fno-rtti)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 ${RTTI}")
