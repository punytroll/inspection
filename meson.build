project(
  'inspection',
  'cpp',
  version: '0.1',
  meson_version: '>=1.2.0',
  default_options: {
    'cpp_std': 'c++23',
    'werror': 'true',
    'default_library': 'static'
  }
)

xml_parser_dependency = dependency(
  'xml_parser',
  version: '>=2.0.2'
)

xml_puny_dom_dependency = dependency(
  'xml_puny_dom',
  version: '>=2.2.1'
)

python_program = import('python').find_installation('python3')

subdir('data')
subdir('tools')
subdir('source')

test(
  'integration test',
  python_program,
  args: [
    integration_tests_program,
    '--in',
    integration_test_suite
  ],
  depends: all_inspectors,
  is_parallel: false,
  verbose: true,
  env: {'PATH': meson.project_build_root() / 'source' / 'inspectors'}
)

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

run_target(
  'recheck',
  command: [
    python_program,
    integration_tests_program,
    '--only-relevant',
    '--in',
    integration_test_suite],
  depends: [all_inspectors],
  env: {'PATH': meson.project_build_root() / 'source' / 'inspectors'}
)
