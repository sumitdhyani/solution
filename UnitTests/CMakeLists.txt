set (GTEST_ROOT "${CMAKE_SOURCE_DIR}/googletest")
set(GTEST_LIBS "gtest gtest_main pthread")

set (INCLUDE_DIRS "${GTEST_ROOT}/googletest/include" "${CMAKE_SOURCE_DIR}" ..)
set (LIB_DIRS "${GTEST_ROOT}/build/lib" "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set (LIBS SolutionLib gtest gtest_main)
  
project(unitTests)

add_executable(PositiveCaseTests PositiveCaseTests.cpp)
target_include_directories(PositiveCaseTests PUBLIC ${INCLUDE_DIRS})
target_link_directories(PositiveCaseTests PUBLIC ${LIB_DIRS})
target_link_libraries(PositiveCaseTests PUBLIC ${LIBS})

add_executable(NegativeCaseTests NegativeCaseTests.cpp)
target_include_directories(NegativeCaseTests PUBLIC ${INCLUDE_DIRS})
target_link_directories(NegativeCaseTests PUBLIC ${LIB_DIRS})
target_link_libraries(NegativeCaseTests PUBLIC ${LIBS})