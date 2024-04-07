cmake_minimum_required(VERSION 3.28)

if (DEFINED PROJECT_NAME)
    set(IS_SUBPROJECT ON)
else ()
    set(IS_SUBPROJECT OFF)
endif ()

project(libnet
    VERSION 0.1
    LANGUAGES CXX
    DESCRIPTION "A network library of Skymarlin")


### Target ###
add_library(libnet INTERFACE)
target_compile_features(libnet INTERFACE cxx_std_20)
if (MSVC)
    target_compile_options(libnet INTERFACE /W4)
else ()
    target_compile_options(libnet INTERFACE -Wall -Wextra -Wpedantic)
endif ()

target_include_directories(libnet INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>)


### Dependency ###
include(FetchContent)

# Boost
find_package(Boost REQUIRED
    COMPONENTS system)

# OpenSSL
#find_package(OpenSSL REQUIRED)

# Flatbuffers
FetchContent_Declare(
    flatbuffers
    GIT_REPOSITORY https://github.com/google/flatbuffers.git
    GIT_TAG v23.5.26
)
FetchContent_MakeAvailable(flatbuffers)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

target_link_libraries(libnet
    INTERFACE
    Boost::system
    spdlog
    flatbuffers
#    OpenSSL::SSL
#    OpenSSL::Crypto
)


## Installation ###
add_library(skymarlin::libnet ALIAS libnet)

include(GNUInstallDirs)
install(TARGETS libnet spdlog
    EXPORT libnet-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT libnet-targets
    FILE libnet-targets.cmake
    NAMESPACE skymarlin::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skymarlin
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    libnet-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY ExactVersion
)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libnet-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libnet-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skymarlin
)


### Test ###
if (NOT IS_SUBPROJECT)
    add_executable(tests
        # util
        skymarlin/util/BitConverter.test.cpp
        skymarlin/util/Map.test.cpp
        skymarlin/util/Queue.test.cpp

        # integration
        test/Network.cpp)
    target_compile_features(tests PRIVATE cxx_std_20)

    if (MSVC)
        target_compile_options(tests PRIVATE /W4)
    else ()
        target_compile_options(tests PRIVATE -Wall -Wextra -Wpedantic)
    endif ()

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.3
    )
    FetchContent_MakeAvailable(Catch2)

    #TODO: Compile test/SimpleMessage.fbs

    target_link_libraries(tests PRIVATE libnet Catch2::Catch2WithMain)
endif ()