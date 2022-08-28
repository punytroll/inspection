project(
  'inspection',
  'cpp',
  version: '0.1',
  meson_version: '>=0.63.0',
  default_options: [
    'cpp_std=c++20',
    'werror=true',
    'default_library=static'
  ]
)

subproject('string_cast')
string_cast_dependency = dependency('string_cast')
subproject('xml_parser')
xml_parser_dependency = dependency('xml_parser')
subproject('xml_puny_dom')
xml_puny_dom_dependency = dependency(
  'xml_puny_dom',
  version: '>=2.0.0'
)

python_program = import('python').find_installation('python3')

subdir('data')
subdir('tools')
subdir('source')

run_target(
  'check',
  command: [
    python_program,
    integration_tests_program,
    '--in',
    integration_test_suite],
  depends: [all_inspectors],
  env: {'PATH': meson.project_build_root() / 'source' / 'inspectors'}
)
