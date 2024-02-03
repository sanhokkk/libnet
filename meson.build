project(
    'libnetwork',
    'cpp',
    version : '0.0.1',
    default_options : ['cpp_std=c++20', 'warning_level=3']
)

project_sources = [
    # protocol
    'skymarlin/protocol/chat/proto/ChatMessage.pb.cc',
]

project_tests = [
    # Unit Tests
    'skymarlin/network/PacketResolver.test.cpp',
    'skymarlin/utility/MutableByteBuffer.test.cpp',
    'skymarlin/utility/BitConverter.test.cpp',

    # Functional/Integration Tests
    'tests/network/Networking.cpp',
]

build_args = [
    '-DPROJECT_NAME=' + meson.project_name(),
    '-DPROJECT_VERSION=' + meson.project_version(),
]

### Target ###
#TODO: Compile proto defs to classes.

protobuf_dep = dependency('protobuf')
absl_base_dep = dependency('absl_base')
spdlog_dep = dependency('spdlog')
libthread_dep = dependency('libthread', fallback : ['libthread', 'libthread_dep'])

project_dependencies = [
    dependency('boost'),
    libthread_dep,
    protobuf_dep,
    absl_base_dep,
    spdlog_dep
]

public_dependencies = [
    libthread_dep,
    protobuf_dep,
    absl_base_dep,
    spdlog_dep
]

inc = include_directories('.')

project_target = static_library(
    meson.project_name(),
    project_sources,
    cpp_args : build_args,
    include_directories : inc,
    dependencies : project_dependencies,
    #  cpp_pch: 'pch/pch.hpp',
)

### Project ###
project_dep = declare_dependency(
    dependencies : public_dependencies,
    include_directories : inc,
    link_with : project_target,
)
set_variable(meson.project_name() + '_dep', project_dep)

### Tests ###
if meson.is_subproject() == false
    gtest_dep = dependency('gtest', main : true)
    test_exec = executable(
        'run_tests',
        files(project_tests),
        dependencies : [project_dep, gtest_dep],
    )

    test('all_tests', test_exec)
endif