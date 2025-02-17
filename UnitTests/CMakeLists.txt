project(unitTests)

set (INCLUDE_DIRS ..)

add_executable(MergingEmptyFiles MergingEmptyFiles.cpp)
target_include_directories(MergingEmptyFiles PUBLIC ${INCLUDE_DIRS})
target_link_directories(MergingEmptyFiles PUBLIC "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
target_link_libraries(MergingEmptyFiles PUBLIC libSolution)

#add_executable(LinkedListAllocatorTests LinkedListAllocatorTests.cpp)
#target_include_directories(LinkedListAllocatorTests PUBLIC ${LIB_DIRS})
#target_link_libraries(LinkedListAllocatorTests ${LIBS})