# src/meson.build
srcs = ['main.c']

exe = executable('llce', srcs, install: true, c_args: ['-D_GNU_SOURCE'])

install_data(
  '../README.md',
  install_dir: get_option('datadir') / 'doc' / meson.project_name(),
)
