project(
    'libnetwork',
    'cpp',
    version : '0.0.1',
    default_options : ['cpp_std=c++20', 'warning_level=3']
)

project_sources = [
    'skymarlin/network/Connector.hpp',
    'skymarlin/network/Listener.hpp',
    'skymarlin/network/Listener.cpp',
    'skymarlin/network/Server.hpp',
    'skymarlin/network/Server.cpp',
    'skymarlin/network/Session.hpp',
    'skymarlin/network/Session.cpp',
    'skymarlin/network/TypeDefinitions.hpp',
    'skymarlin/network/packet/ByteBufferExceptions.hpp',
    'skymarlin/network/packet/ConstByteBuffer.hpp',
    'skymarlin/network/packet/ConstByteBuffer.cpp',
    'skymarlin/network/packet/MutableByteBuffer.hpp',
    'skymarlin/network/packet/MutableByteBuffer.cpp',
    'skymarlin/network/packet/Packet.hpp',
    'skymarlin/network/packet/PacketResolver.hpp',
]

project_tests = [
    # Unit Tests

    # Functional/Integration Tests
]

build_args = [
]

### Target ###
libthread_dep = dependency('libthread', fallback: ['libthread', 'libthread_dep'])

project_dependencies = [
    dependency('boost'),
    libthread_dep
]

public_dependencies = [
    libthread_dep
]

inc = include_directories('.')

build_args += [
    '-DPROJECT_NAME=' + meson.project_name(),
    '-DPROJECT_VERSION=' + meson.project_version(),
]

project_target = library(
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