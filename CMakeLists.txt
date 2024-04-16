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
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/include>
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
    spdlog
    flatbuffers
)


### Packaging ###
include(GNUInstallDirs)

install(
    TARGETS libnet spdlog flatbuffers
    EXPORT libnet-targets
)
install(
    DIRECTORY skymarlin/net
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/skymarlin
)
install(
    DIRECTORY skymarlin/util
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/skymarlin
)

install(EXPORT libnet-targets
    FILE libnet-targets.cmake
    NAMESPACE skymarlin::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skymarlin
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/libnet-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/libnet-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skymarlin
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/libnet-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY ExactVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libnet-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libnet-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skymarlin
)


### Utilities ###
function(CompileSchemas target options out_path schemas)
    message("Schemas to compile: ${schemas}")
    set(compile_target_name "compile_schemas_${target}")
    add_custom_target(
        ${compile_target_name}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND
        flatc
        --cpp
        --cpp-std c++17
        --filename-suffix "\"\""
        --filename-ext hpp
        ${options}
        -o ${out_path}
        ${schemas}
        DEPENDS flatc
    )
    add_dependencies(${target} ${compile_target_name})
endfunction()


### Tests ###
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.4
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(tests
        skymarlin/util/BitConverter.test.cpp
        skymarlin/util/Map.test.cpp
        skymarlin/util/Queue.test.cpp

        test/Network.test.cpp
    )

    target_compile_features(tests PRIVATE cxx_std_20)
    target_link_libraries(tests PRIVATE skymarlin::libnet Catch2::Catch2WithMain)

    CompileSchemas(tests "" test test/SimpleMessage.fbs)

    enable_testing()
    add_test(NAME libnet-tests COMMAND tests)
endif()
