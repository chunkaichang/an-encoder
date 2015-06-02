cmake_minimum_required(VERSION 2.8)

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
                      CheckInserter.cpp
                      Coder.cpp
                      UsesVault.cpp)
add_dependencies(encode anlib.bc)

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

if ("$LLVM_ENABLE_RTTI" MATCHES "ON")
  set(RTTI -frtti)
else()
  set(RTTI -fno-rtti)
endif()

if (CHECK_BEFORE)
  add_definitions(-DCHECK_BEFORE)
endif ()
if (CHECK_AFTER)
  add_definitions(-DCHECK_AFTER)
endif ()
if (CHECK_BB)
  add_definitions(-DCHECK_BB)
endif ()
if (CHECK_FUN)
  add_definitions(-DCHECK_FUN)
endif ()
if (ACCU_ARGS)
  add_definitions(-DACCU_ARGS)
endif ()
if (ACCU_RES)
  add_definitions(-DACCU_RES)
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 ${RTTI}")

set(EXECUTABLE_OUTPUT_PATH ${ENCODE_BIN_DIR})
