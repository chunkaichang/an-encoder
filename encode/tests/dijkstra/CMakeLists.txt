cmake_minimum_required(VERSION 2.8)

include(../buildtest.cmake)

BUILD_TEST_CASE(dijkstra)

set(DIJKSTRA_INPUT dijkstra_input.dat)
add_custom_target(${DIJKSTRA_INPUT} ALL
                  COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/${DIJKSTRA_INPUT}
                          ${CMAKE_CURRENT_BINARY_DIR})

BUILD_TEST_CONFIG(dijkstra "${CMAKE_CURRENT_BINARY_DIR}/${DIJKSTRA_INPUT}" 20)
