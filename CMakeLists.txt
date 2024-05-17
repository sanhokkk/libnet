cmake_minimum_required(VERSION 3.28)
project(libnet
    VERSION 0.1
    LANGUAGES CXX
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
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)


### Targets ###
add_library(libnet INTERFACE
    skymarlin/net/listener_tcp.hpp
    skymarlin/net/peer_tcp.hpp
    skymarlin/net/peer_udp.hpp
    skymarlin/bip_buffer.hpp
    skymarlin/concurrent_map.hpp
    skymarlin/concurrent_queue.hpp
)
add_library(skymarlin::libnet ALIAS libnet)
target_compile_features(libnet INTERFACE cxx_std_20)
target_include_directories(libnet INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(libnet
    INTERFACE
    Boost::system
    spdlog
    flatbuffers
)

function(skymarlin_compile_schemas target options out_path schemas)
    message("Schemas to compile: ${schemas}")
    set(compile_target_name "skymarlin_compile_schemas_${target}")
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
        skymarlin/concurrent_map.test.cpp
        skymarlin/concurrent_queue.test.cpp

        tests/server.test.cpp
    )

    target_compile_features(tests PRIVATE cxx_std_20)
    target_link_libraries(tests PRIVATE skymarlin::libnet Catch2::Catch2WithMain)

    skymarlin_compile_schemas(tests "" tests tests/hello.fbs)

    enable_testing()
    add_test(NAME libnet-tests COMMAND tests)
endif()
