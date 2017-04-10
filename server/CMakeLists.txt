cmake_minimum_required(VERSION 3.3)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif ()

project(gravioli)

# set version information in a config.h file
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(RES_PATH "res")
configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
)
include_directories("${PROJECT_BINARY_DIR}")

# define sources and executable
set(EXECUTABLE_NAME "gravioli")
file(GLOB CPP_FILES src/*)
set(SOURCE_FILES ${CPP_FILES})
add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES})

# include header files
include_directories(.)

# include SimpleLog
include_directories(lib/SimpleLog)

# include beast
include_directories(lib/Beast/include)

# add resources to build directory
file(COPY res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# require C++11
set_target_properties(
    gravioli PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

# add Boost library
find_package(Boost 1.40.0 COMPONENTS program_options system REQUIRED)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    target_link_libraries(${EXECUTABLE_NAME} ${Boost_LIBRARIES})
endif ()

# add threading support
find_package(Threads REQUIRED)
if (Threads_FOUND)
    target_link_libraries(${EXECUTABLE_NAME} ${CMAKE_THREAD_LIBS_INIT})
endif ()

# install target
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)
