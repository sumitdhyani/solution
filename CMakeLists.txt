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

project(DefaultFileReader)
add_executable(DefaultFileReader DefaultFileReader.cpp)

project(SmartFileReader)
add_executable(SmartFileReader SmartFileReader.cpp)

project(WC)
add_executable(WC WC.cpp)

set (BOOST_ROOT "${CMAKE_SOURCE_DIR}/boost")
set (IO_UTILS_INCLUDE_DIRS "${BOOST_ROOT}")
set (IO_UTILS_LIB_DIRS "${BOOST_ROOT}/stage/lib")

project(IO_UTILS)
add_executable(IO_UTILS CommUtilities.cpp)
target_include_directories(IO_UTILS PUBLIC ${IO_UTILS_INCLUDE_DIRS})
target_link_directories(IO_UTILS PUBLIC ${IO_UTILS_LIB_DIRS})


#add_subdirectory("UnitTests")

