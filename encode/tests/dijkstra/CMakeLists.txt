cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

set(DIJKSTRA_INPUT dijkstra_input.dat)
add_custom_target(${DIJKSTRA_INPUT} ALL
                  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${DIJKSTRA_INPUT}
                          ${CMAKE_CURRENT_BINARY_DIR})

BUILD_TEST_CASE(dijkstra dijkstra.veri)
BUILD_TEST_CONFIG(dijkstra dijkstra.veri
                           ${CMAKE_CURRENT_BINARY_DIR}/${DIJKSTRA_INPUT}
                           ${CMAKE_CURRENT_SOURCE_DIR}/dijkstra.veri.cfg)

BUILD_TEST_CASE(dijkstra dijkstra.perf)
BUILD_TEST_CONFIG(dijkstra dijkstra.perf
                           ${CMAKE_CURRENT_BINARY_DIR}/${DIJKSTRA_INPUT}
                           ${CMAKE_CURRENT_SOURCE_DIR}/dijkstra.perf.cfg)

BUILD_TEST_CASE(dijkstra dijkstra.cover)
BUILD_TEST_CONFIG(dijkstra dijkstra.cover
                           ${CMAKE_CURRENT_BINARY_DIR}/${DIJKSTRA_INPUT}
                           ${CMAKE_CURRENT_SOURCE_DIR}/dijkstra.cover.cfg)

