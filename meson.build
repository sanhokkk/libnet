project(
    'libnet',
    'cpp',
    version : '0.0.1',
    license : 'MIT',
    default_options : ['cpp_std=c++20', 'warning_level=3']
)

build_args = []

include = include_directories('.')

#flatbuffers_dep = dependency('flatbuffers')
dependencies = [
    dependency('boost', modules : ['system']),
    #    flatbuffers_dep,
    dependency('flatbuffers'),
    dependency('spdlog'),
]

libnet = shared_library(
    meson.project_name(),
    [],
    cpp_args : build_args,
    dependencies : dependencies,
    include_directories : include
)

libnet_dep = declare_dependency(
    dependencies : dependencies,
    include_directories : include,
    link_with : libnet,
)

### TEST ###
test_files = [
    'skymarlin/util/BitConverter.test.cpp',
    'skymarlin/util/Map.test.cpp',
    'skymarlin/util/Queue.test.cpp',
    'test/Network.cpp',
]

if not meson.is_subproject()
    doctest_dep = dependency('doctest')
    tests_exec = executable(
        'tests',
        test_files,
        cpp_args : build_args,
        dependencies : [libnet_dep, doctest_dep],
    )

    #    flatc = find_program('flatc')
    #    run_command(flatc, '--version', check: true)
endif