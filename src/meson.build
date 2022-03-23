jsonbomb_sources = [
  'main.cpp',
]

jsonbomb_deps = [
]

libcurlDependency = dependency('libcurl')

executable('jsonbomb', jsonbomb_sources,
  cpp_args : '-DJSON_BOMB_VERSION="@0@"'.format(meson.project_version()),
  dependencies: jsonbomb_deps,
  install: true,
  link_args : '-lcurl',
  include_directories : incdir
)


