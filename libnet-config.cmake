include(CMakeFindDependencyMacro)

find_dependency(Boost REQUIRED COMPONENTS system)
find_dependency(flatbuffers REQUIRED)
find_dependency(spdlog REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/libnet-targets.cmake)