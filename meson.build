project(
  'libnetwork',
  'cpp',
  version: '0.0.1',
  default_options: ['cpp_std=c++20', 'warning_level=3']
)
project_description = 'Skymarlin TCP network library'

project_headers = [
  'include/skymarlin/network/Listener.hpp',
  'include/skymarlin/network/Session.hpp',
  'include/skymarlin/network/TypeDefinitions.hpp',
  'include/skymarlin/network/packet/ByteBufferExceptions.hpp',
  'include/skymarlin/network/packet/ConstByteBuffer.hpp',
  'include/skymarlin/network/packet/MutableByteBuffer.hpp',
  'include/skymarlin/network/packet/Packet.hpp',
]

project_sources = [
  'src/Listener.cpp',
  'src/Session.cpp',
  'src/packet/ConstByteBuffer.cpp',
  'src/packet/MutableByteBuffer.cpp',
]

project_tests = [
]

build_args = [
]

### Target ###
project_dependencies = [
  dependency('boost')
]

inc = include_directories('include')

build_args += [
  '-DPROJECT_NAME=' + meson.project_name(),
  '-DPROJECT_VERSION=' + meson.project_version(),
]

# Only make public interfaces visible
if target_machine.system() == 'windows' or target_machine.system() == 'cygwin'
  build_args += '-DMYLIB_PUBLIC="__declspec(dllexport)"'
else
  build_args += '-DMYLIB_PUBLIC=__attribute__((visibility("default")))'
endif

project_target = shared_library(
  meson.project_name(),
  project_sources,
  install : true,
  cpp_args : build_args,
  gnu_symbol_visibility : 'hidden',
  include_directories : inc,
  dependencies: project_dependencies,
)

### Project ###
project_dep = declare_dependency(
  include_directories: inc,
  link_with: project_target,
)
set_variable(meson.project_name() + '_dep', project_dep)

install_headers(project_headers, subdir : meson.project_name())
pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name : meson.project_name(),
  filebase : meson.project_name(),
  description : project_description,
  subdirs : meson.project_name(),
  libraries: project_target
)

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