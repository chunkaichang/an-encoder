cmake_minimum_required(VERSION 2.8)

add_subdirectory(coder)
add_subdirectory(parser)

add_executable(encode encode.cpp
                      GlobalsEncoder.cpp
                      ConstantsEncoder.cpp
                      OperationsEncoder.cpp
                      OperationsExpander.cpp
                      Linkage.cpp
                      AccuPromoter.cpp)
add_dependencies(encode anlib.bc)


# Find the libraries that correspond to the LLVM components
# that we wish to use
# use llvm_map_components_to_libraries() when llvm version < 3.5
#llvm_map_components_to_libraries(llvm_libs support
llvm_map_components_to_libnames(llvm_libs support
                                          core
                                          bitreader
                                          bitwriter
                                          ipo
                                          linker
                                          engine
                                          irreader
                                          instrumentation
                                          objcarcopts)

# Link against LLVM libraries
target_link_libraries(encode ${llvm_libs}
                             parser
                             coder)

if ("$LLVM_ENABLE_RTTI" MATCHES "ON")
  set(RTTI -frtti)
else()
  set(RTTI -fno-rtti)
endif()

if (LOG_ACCU)
  add_definitions(-DLOG_ACCU)
endif ()
add_definitions(-DCODE_VALUE_A=${CODE_VALUE_A} -D_GLIBCXX_USE_CXX11_ABI=${USE_CXX11_ABI})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 ${RTTI}")

set(EXECUTABLE_OUTPUT_PATH ${ENCODE_BIN_DIR})
