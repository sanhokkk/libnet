project(
    'libnetwork',
    'cpp',
    version : '0.0.1',
    default_options : ['cpp_std=c++20', 'warning_level=3']
)

project_sources = [
    ## network
    'skymarlin/network/Listener.cpp',
    'skymarlin/network/Server.cpp',
    'skymarlin/network/Session.cpp',
    ## utility
    'skymarlin/utility/ConstByteBuffer.cpp',
    'skymarlin/utility/MutableByteBuffer.cpp',
    ## protocol
    'skymarlin/protocol/chat/ChatMessage.pb.cc',
]

project_tests = [
    # Unit Tests

    # Functional/Integration Tests
]

build_args = [
    '-DPROJECT_NAME=' + meson.project_name(),
    '-DPROJECT_VERSION=' + meson.project_version(),
]

### Target ###
#TODO: Compile proto defs to classes.

protobuf_dep = dependency('protobuf')
absl_base_dep = dependency('absl_base')
libthread_dep = dependency('libthread', fallback: ['libthread', 'libthread_dep'])

project_dependencies = [
    dependency('boost'),
    libthread_dep,
    protobuf_dep,
    absl_base_dep,
]

public_dependencies = [
    libthread_dep,
    protobuf_dep,
    absl_base_dep,
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
    dependencies: public_dependencies,
    include_directories : inc,
    link_with : project_target,
)
set_variable(meson.project_name() + '_dep', project_dep)

### Unit Tests ###
# if meson.is_subproject() == false
#   add_languages('cpp')
#   subdir('tests')

#   test('all_tests',
#     executable(
#       'run_tests',
#       files(project_test_files),
#       dependencies : [project_dep, test_dep],
#       install : false
#     )
#   )
# endif