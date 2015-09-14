cmake_minimum_required(VERSION 2.8)

set(LENGTH_PERF "1000" CACHE STRING "")

add_subdirectory(mylibs)

add_subdirectory(array_sum)

add_subdirectory(bubblesort.best)
add_subdirectory(bubblesort.random)
add_subdirectory(bubblesort.random-binary)
add_subdirectory(bubblesort.worst)

add_subdirectory(crc)
add_subdirectory(des)
#add_subdirectory(dijkstra)
add_subdirectory(matmul)
#add_subdirectory(matmul_overflow)
#add_subdirectory(matmul_while)
add_subdirectory(ptrarith)

add_subdirectory(quicksort.best)
add_subdirectory(quicksort.random)
add_subdirectory(quicksort.random-binary)
add_subdirectory(quicksort.worst)

add_subdirectory(profiles)
