if(NOT CMAKE_BUILD_TYPE )
        set(CMAKE_BUILD_TYPE Release)
endif()


if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
	set(CMAKE_CXX_STANDARD 20)
	cmake_minimum_required(VERSION 3.5)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
	set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
endif()

project(Solution)
add_executable(Solution MDEntry.cpp Solution.cpp main.cpp)
add_library(SolutionLib STATIC Solution.cpp MDEntry.cpp)

project(DefaultFileReader)
add_executable(DefaultFileReader DefaultFileReader.cpp)

project(SmartFileReader)
add_executable(SmartFileReader SmartFileReader.cpp)

project(WC)
add_executable(WC WC.cpp)

add_subdirectory("UnitTests")

