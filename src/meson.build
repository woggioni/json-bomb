jsonbomb_sources = [
  'main.cpp',
]

jsonbomb_deps = [
]

libcurlDependency = dependency('libcurl')

executable('jsonbomb', jsonbomb_sources,
  dependencies: jsonbomb_deps,
  install: true,
  link_args : '-lcurl',
  include_directories : incdir
)


