# src/meson.build
srcs = [
  'main.c',
  'utils/pid.c',
  'utils/probe.c',
  'utils/scan.c',
  'datastructure/hashmap.c',
]

inc = include_directories('utils')
threads_dep = dependency('threads')

exe = executable(
  'llce',
  srcs,
  install: true,
  include_directories: inc,
  dependencies: [threads_dep],
  c_args: [
    '-D_GNU_SOURCE',
  ],
)

install_data(
  '../README.md',
  install_dir: get_option('datadir') / 'doc' / meson.project_name(),
)
