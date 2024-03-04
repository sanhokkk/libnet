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
add_library(libnet)
target_compile_features(libnet PRIVATE cxx_std_20)
if (MSVC)
    target_compile_options(libnet PRIVATE /W4)
else ()
    target_compile_options(libnet PRIVATE -Wall -Wextra -Wpedantic)
endif ()

target_sources(libnet
    PRIVATE
    # net
    skymarlin/net/Client.cpp
    skymarlin/net/Connection.cpp
    skymarlin/net/Listener.cpp
    skymarlin/net/Server.cpp

    # utility
    skymarlin/util/MemoryPool.cpp)

target_include_directories(libnet PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


### Dependency ###
include(FetchContent)

# Boost
FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.7z
    DOWNLOAD_NO_EXTRACT FALSE
)
FetchContent_MakeAvailable(Boost)

# OpenSSL
find_package(OpenSSL REQUIRED)

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
    PRIVATE
    Boost::asio
    OpenSSL::SSL
    OpenSSL::Crypto
    flatbuffers
    spdlog)

### Schema ###
add_custom_command(
    TARGET libnet
    PRE_BUILD
#    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/flatbuffers
#    COMMAND flatc --cpp -o ${CMAKE_CURRENT_SOURCE_DIR}/skymarlin/msg ${CMAKE_CURRENT_SOURCE_DIR}/skymarlin/chat/schema/Chat.fbs
    COMMENT "Hey YOO!!!!!!!!!!!!!!!!!!!!!!!"
)

### Test ###
if (NOT IS_SUBPROJECT)
    add_executable(test
        # net
        skymarlin/net/Server.test.cpp
        skymarlin/net/Client.test.cpp

        # utility
        skymarlin/util/BitConverter.test.cpp
        skymarlin/util/Map.test.cpp
        skymarlin/util/MemoryPool.test.cpp
        skymarlin/util/Queue.test.cpp)
    target_compile_features(test PRIVATE cxx_std_20)

    if (MSVC)
        target_compile_options(test PRIVATE /W4)
    else ()
        target_compile_options(test PRIVATE -Wall -Wextra -Wpedantic)
    endif ()

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.3
    )
    FetchContent_MakeAvailable(Catch2)

    target_link_libraries(test PRIVATE libnet Catch2::Catch2WithMain)
endif ()