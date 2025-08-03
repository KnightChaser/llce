# src/meson.build
srcs = [
  'main.c',
  'utils/pid.c',
  'utils/probe.c',
  'utils/scan.c',
  'utils/poke.c',
  'datastructure/hashmap.c',
  'ui/logger.c',
  'ui/ui.c',
  'ui/handler/attach.c',
  'ui/handler/cleanup.c',
  'ui/handler/detect.c',
  'ui/handler/fullscan.c',
  'ui/handler/help.c',
  'ui/handler/poke.c',
  'ui/handler/search.c',
]

inc = include_directories(
  'utils',
  'datastructure',
  'ui',
)
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

test(
  'llce_hashmap_test',
  executable(
    'test_hashmap',
    'test/test_hashmap.c',
    'datastructure/hashmap.c',
    install: false,
  ),
)

install_data(
  '../README.md',
  install_dir: get_option('datadir') / 'doc' / meson.project_name(),
)
