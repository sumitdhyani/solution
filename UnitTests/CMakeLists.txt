set (GTEST_ROOT "${CMAKE_SOURCE_DIR}/googletest") 
set(GTEST_LIBS "gtest gtest_main pthread")

set (INCLUDE_DIRS "${GTEST_ROOT}/googletest/include" "${CMAKE_SOURCE_DIR}" ..)
set (LIB_DIRS "${GTEST_ROOT}/build/lib" "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set (LIBS SolutionLib gtest gtest_main)
  
project(unitTests)

add_executable(MergingEmptyFiles MergingEmptyFiles.cpp)
target_include_directories(MergingEmptyFiles PUBLIC ${INCLUDE_DIRS})
target_link_directories(MergingEmptyFiles PUBLIC ${LIB_DIRS})
target_link_libraries(MergingEmptyFiles PUBLIC ${LIBS})

add_executable(MergingFilesOneEmptyAndOtherNonEmpty MergingFilesOneEmptyAndOtherNonEmpty.cpp)
target_include_directories(MergingFilesOneEmptyAndOtherNonEmpty PUBLIC ${INCLUDE_DIRS})
target_link_directories(MergingFilesOneEmptyAndOtherNonEmpty PUBLIC ${LIB_DIRS})
target_link_libraries(MergingFilesOneEmptyAndOtherNonEmpty PUBLIC ${LIBS})

add_executable(SortAlphabeticallyIfTimestampEqual SortAlphabeticallyIfTimestampEqual.cpp)
target_include_directories(SortAlphabeticallyIfTimestampEqual PUBLIC ${INCLUDE_DIRS})
target_link_directories(SortAlphabeticallyIfTimestampEqual PUBLIC ${LIB_DIRS})
target_link_libraries(SortAlphabeticallyIfTimestampEqual PUBLIC ${LIBS})

add_executable(SortByTimestamp SortByTimestamp.cpp)
target_include_directories(SortByTimestamp PUBLIC ${INCLUDE_DIRS})
target_link_directories(SortByTimestamp PUBLIC ${LIB_DIRS})
target_link_libraries(SortByTimestamp PUBLIC ${LIBS})

add_executable(MergeUnequalSizedFiles MergeUnequalSizedFiles.cpp)
target_include_directories(MergeUnequalSizedFiles PUBLIC ${INCLUDE_DIRS})
target_link_directories(MergeUnequalSizedFiles PUBLIC ${LIB_DIRS})
target_link_libraries(MergeUnequalSizedFiles PUBLIC ${LIBS})