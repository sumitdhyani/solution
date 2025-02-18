project(unitTests)

set (INCLUDE_DIRS ..)

add_executable(MergingEmptyFiles MergingEmptyFiles.cpp)
target_include_directories(MergingEmptyFiles PUBLIC ${INCLUDE_DIRS})
target_link_directories(MergingEmptyFiles PUBLIC "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
target_link_libraries(MergingEmptyFiles PUBLIC libSolution)

add_executable(MergingFilesOneEmptyAndOtherNonEmpty MergingFilesOneEmptyAndOtherNonEmpty.cpp)
target_include_directories(MergingFilesOneEmptyAndOtherNonEmpty PUBLIC ${INCLUDE_DIRS})
target_link_directories(MergingFilesOneEmptyAndOtherNonEmpty PUBLIC "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
target_link_libraries(MergingFilesOneEmptyAndOtherNonEmpty PUBLIC libSolution)

add_executable(SortAlphabeticallyIfTimestampEqual SortAlphabeticallyIfTimestampEqual.cpp)
target_include_directories(SortAlphabeticallyIfTimestampEqual PUBLIC ${INCLUDE_DIRS})
target_link_directories(SortAlphabeticallyIfTimestampEqual PUBLIC "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
target_link_libraries(SortAlphabeticallyIfTimestampEqual PUBLIC libSolution)

add_executable(SortByTimestamp SortByTimestamp.cpp)
target_include_directories(SortByTimestamp PUBLIC ${INCLUDE_DIRS})
target_link_directories(SortByTimestamp PUBLIC "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
target_link_libraries(SortByTimestamp PUBLIC libSolution)

add_executable(MergeUnequalSizedFiles MergeUnequalSizedFiles.cpp)
target_include_directories(MergeUnequalSizedFiles PUBLIC ${INCLUDE_DIRS})
target_link_directories(MergeUnequalSizedFiles PUBLIC "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
target_link_libraries(MergeUnequalSizedFiles PUBLIC libSolution)