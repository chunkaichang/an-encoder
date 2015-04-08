find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(encode encode.cpp
                      GlobalsEncoder.cpp
                      ConstantsEncoder.cpp
                      GEPHandler.cpp
                      OperationsEncoder.cpp
                      OperationsExpander.cpp
                      EncodeDecodeRemover.cpp
                      AccumulateRemover.cpp
                      CallHandler.cpp
                      Linkage.cpp
                      SExtTrunc.cpp
                      ModuleChecker.cpp
                      InterfaceHandler.cpp
                      BoolExtHandler.cpp
                      Coder.cpp
                      UsesVault.cpp)
add_dependencies(encode ${CMAKE_BINARY_SOURCE_DIR}/../runtime/anlib.bc
                        ${CMAKE_BINARY_SOURCE_DIR}/../runtime/rdtsc.bc)


# Find the libraries that correspond to the LLVM components
# that we wish to use
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
target_link_libraries(encode ${llvm_libs})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")

set(EXECUTABLE_OUTPUT_PATH ${ENCODE_BIN_DIR})
