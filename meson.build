project(
    'libnetwork',
    'cpp',
    version : '0.0.1',
    license : 'MIT',
    default_options : ['cpp_std=c++20', 'warning_level=3']
)

project_sources = [
    # network
    'skymarlin/net/Client.cpp',
    'skymarlin/net/Listener.cpp',
    'skymarlin/net/Server.cpp',
    'skymarlin/net/Session.cpp',

    # protocol
    ## chat
    'skymarlin/protocol/chat/ChatMessagePacket.cpp',
    'skymarlin/protocol/chat/proto/ChatMessage.pb.cc',
]

project_tests = [
    # Unit Tests
    ## network
    'skymarlin/net/details/BitConverter.test.cpp',

    ## protocol
    'skymarlin/protocol/auth/AuthPackets.test.cpp',
    'skymarlin/protocol/chat/ChatPackets.test.cpp',

    ## utility
    'skymarlin/utility/Map.test.cpp',
    'skymarlin/utility/Queue.test.cpp',

    # Functional/Integration Tests
    'tests/network/Networking.cpp',
]

build_args = [
    '-DPROJECT_NAME=' + meson.project_name(),
    '-DPROJECT_VERSION=' + meson.project_version(),
]


### Project ###
project_dependencies = [
    dependency('absl_base'), # Required by protobuf
    dependency('boost'),
    dependency('openssl'),
    dependency('protobuf', modules : ['protobuf::libprotobuf']),
    dependency('spdlog'),
]
inc = include_directories('.')

project_target = static_library(
    meson.project_name(),
    project_sources,
    cpp_args : build_args,
    dependencies : project_dependencies,
    include_directories : inc,
)

project_dep = declare_dependency(
    dependencies : project_dependencies,
    include_directories : inc,
    link_with : project_target,
)
set_variable(meson.project_name() + '_dep', project_dep)


### Tests ###
if meson.is_subproject() == false
    gtest_dep = dependency('gtest', main : true)
    tests_exec = executable(
        'tests',
        project_tests,
        cpp_args: [
            '-DPROJECT_ROOT="' + meson.current_source_dir() + '"',
        ],
        dependencies : [project_dep, gtest_dep],
    )
endif
