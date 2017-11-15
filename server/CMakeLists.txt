cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(server)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

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

# include flatbuffers headers
set(FLATBUFFERS_HEADERS "~/flatbuffers/include/flatbuffers" 
    CACHE PATH "Location of flatbuffers headers")
if (NOT EXISTS "${FLATBUFFERS_HEADERS}")
    message(FATAL_ERROR "Given path to flatbuffers headers does not exist!")
endif ()
configure_files(
    "${FLATBUFFERS_HEADERS}"
    "${PROJECT_SOURCE_DIR}/lib/flatbuffers"
)

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

# require C++17
set_target_properties(
    ${EXECUTABLE_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
        set(CMAKE_CXX_FLAGS "\
            -Wall -Wextra \
            -Werror \
            -Wno-missing-braces \
        ")

        # add IWYU to build chain
        set(IWYU_ROOT_PATH "/usr/lib/clang/"
            CACHE PATH "Root location of IWYU executable")
        find_program(iwyu_path NAMES include-what-you-use iwyu
                     PATHS ${IWYU_ROOT_PATH})
        if (NOT iwyu_path)
            message(WARNING "Could not find the program include-what-you-use")
        else ()
            message(STATUS "Found IWYU: " ${iwyu_path})
            set_property(TARGET ${EXECUTABLE_NAME} PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
        endif ()

    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "\
            -Wall -Wextra \
            -Werror -pedantic \
            -Wlogical-op \
            -Wuseless-cast \
            -Wdouble-promotion \
            -Wformat=2\
        ")
    endif ()
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
                       test/test_server.cpp
                       test/test_playerProvider.cpp)

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

    # require C++17
    set_target_properties(
        ${EXECUTABLE_NAME_TEST} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    target_link_libraries(${EXECUTABLE_NAME_TEST} ${Boost_LIBRARIES})
    target_link_libraries(${EXECUTABLE_NAME_TEST} ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(${EXECUTABLE_NAME_TEST} ${ZMQ_LIB})
    target_link_libraries(${EXECUTABLE_NAME_TEST} ${CRYPTOPP_LIB})

    if (iwyu_path)
        set_property(TARGET ${EXECUTABLE_NAME_TEST} PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
    endif ()

endif ()

