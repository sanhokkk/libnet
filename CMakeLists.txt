cmake_minimum_required(VERSION 3.14)
project(libnet
    VERSION 0.1
    LANGUAGES CXX
)

add_library(libnet INTERFACE)
add_library(skymarlin::libnet ALIAS libnet)

target_compile_features(libnet INTERFACE cxx_std_20)

target_include_directories(libnet
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)


### Dependencies ###
include(FetchContent)

# Boost
find_package(Boost REQUIRED COMPONENTS system)

# flatbuffers
FetchContent_Declare(
    flatbuffers
    GIT_REPOSITORY https://github.com/google/flatbuffers.git
    GIT_TAG v24.3.25
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
    spdlog::spdlog
    flatbuffers
)


### Packaging ###
include(GNUInstallDirs)

install(
    TARGETS libnet Boost::system spdlog::spdlog flatbuffers
    EXPORT libnet-targets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT libnet-targets
    FILE libnet-targets.cmake
    NAMESPACE skymarlin::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skymarlin
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/libnet-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY ExactVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/libnet-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/libnet-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skymarlin
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libnet-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libnet-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skymarlin
)

export(EXPORT libnet-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/libnet-targets.cmake
    NAMESPACE skymarlin::
)


### Tests ###
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    enable_testing()
    #TODO
endif()
