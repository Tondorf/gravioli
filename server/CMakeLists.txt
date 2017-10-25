cmake_minimum_required(VERSION 3.5.1)

#set(CMAKE_BUILD_TYPE release)
set(CMAKE_BUILD_TYPE debug)

project(server)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)

# set version information in config.h file
configure_file(
    "${PROJECT_SOURCE_DIR}/config.hpp.in"
    "${PROJECT_BINARY_DIR}/config.hpp"
)

# Copy files from source directory to destination directory and
# creates destination directory if it does not exist.
macro(configure_files srcDir destDir)
    message(STATUS "Configuring directory ${destDir}")
    make_directory(${destDir})

    file(GLOB templateFiles RELATIVE ${srcDir} ${srcDir}/*)
    foreach(templateFile ${templateFiles})
        set(srcTemplatePath ${srcDir}/${templateFile})
        if(NOT IS_DIRECTORY ${srcTemplatePath})
            message(STATUS "Configuring file ${templateFile}")
            configure_file(
                ${srcTemplatePath}
                ${destDir}/${templateFile}
                COPYONLY
            )
        endif(NOT IS_DIRECTORY ${srcTemplatePath})
    endforeach(templateFile)
endmacro(configure_files)

# main include directory
include_directories("include")

# include *.hpp.in
include_directories("${PROJECT_BINARY_DIR}")

# include libraries
include_directories("lib")

# include generated flatbuffers files
include_directories("fbs")

# define sources and executable
set(EXECUTABLE_NAME "server")
set(SOURCE_FILES src/userInputParser.cpp
                 src/greenwich.cpp
                 src/runnable.cpp
                 src/server.cpp
                 src/crypto.cpp
                 src/simpleMsgQueue.cpp
                 src/world.cpp
                 src/playerProvider.cpp
                 src/serializableWorld.cpp
                 src/fbmessage.cpp
                 src/player.cpp
                 src/webclient.cpp)
add_executable(${EXECUTABLE_NAME} src/main.cxx ${SOURCE_FILES})

# require C++14
set_target_properties(
    ${EXECUTABLE_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)
if (CMAKE_COMPILER_IS_GNUCXX AND CMAKE_BUILD_TYPE STREQUAL "debug")
    set(CMAKE_CXX_FLAGS "\
        -Wall -Wextra \
        -Werror -pedantic \
        -Wlogical-op \
        -Wuseless-cast \
        -Wdouble-promotion \
        -Wformat=2\
    ")
endif ()

# add Boost library
find_package(Boost COMPONENTS system thread program_options REQUIRED)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    target_link_libraries(${EXECUTABLE_NAME} ${Boost_LIBRARIES})
endif ()

# add threading support
find_package(Threads REQUIRED)
if (Threads_FOUND)
    target_link_libraries(${EXECUTABLE_NAME} ${CMAKE_THREAD_LIBS_INIT})
endif ()

# add zeromq library
find_library(ZMQ_LIB zmq)
if (NOT ZMQ_LIB)
    message(FATAL_ERROR "zeromq library not found")
else ()
    target_link_libraries(${EXECUTABLE_NAME} ${ZMQ_LIB})
endif ()

# add crypto++ library
find_library(CRYPTOPP_LIB cryptopp)
if (NOT CRYPTOPP_LIB)
    message(FATAL_ERROR "crypto++ library not found")
else ()
    target_link_libraries(${EXECUTABLE_NAME} ${CRYPTOPP_LIB})
endif ()

# add Google Testing Framework and enable testing
find_package(GTest)
if (GTEST_FOUND)
    include_directories("test")

    set(UNITTEST_FILES test/helper.cpp
                       test/test_userInputParser.cpp
                       test/test_crypto.cpp
                       test/test_simpleMsgQueue.cpp
                       test/test_server.cpp)

    configure_files(
        "${PROJECT_SOURCE_DIR}/test/KAT_AES"
        "${PROJECT_BINARY_DIR}/test/KAT_AES"
    )

    set(EXECUTABLE_NAME_TEST "runAllUnitTests")

    enable_testing()
    add_executable(${EXECUTABLE_NAME_TEST} test/main.cxx
                                           ${UNITTEST_FILES}
                                           ${SOURCE_FILES})
    target_link_libraries(${EXECUTABLE_NAME_TEST} gtest)
    add_test(NAME ${EXECUTABLE_NAME_TEST} COMMAND ${EXECUTABLE_NAME_TEST})

    # require C++14
    set_target_properties(
        ${EXECUTABLE_NAME_TEST} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
    )

    target_link_libraries(${EXECUTABLE_NAME_TEST} ${Boost_LIBRARIES})
    target_link_libraries(${EXECUTABLE_NAME_TEST} ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(${EXECUTABLE_NAME_TEST} ${ZMQ_LIB})
    target_link_libraries(${EXECUTABLE_NAME_TEST} ${CRYPTOPP_LIB})
endif ()

