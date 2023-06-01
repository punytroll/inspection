type_library_path_h = custom_target(
  'type_library_path.h',
  output: 'type_library_path.h',
  input: update_type_library_path_program,
  command: [
    python_program,
    '@INPUT@',
    type_library_path,
    '@OUTPUT@'
  ]
)

common_library = static_library(
  'common',
  sources: [
    'source/exception_printing.cpp',
    'source/execution_context.cpp',
    'source/getters.cpp',
    'source/guid.cpp',
    'source/helper.cpp',
    'source/id3_de_unsynchronization_eager_filter.cpp',
    'source/inspector.cpp',
    'source/output_operators.cpp',
    'source/query.cpp',
    'source/reader.cpp',
    'source/type_definition.cpp',
    'source/type_repository.cpp',
    'source/type.cpp',
    type_library_path_h
  ],
  include_directories: [include_directories('include/common')],
  dependencies: [
    string_cast_dependency,
    xml_puny_dom_dependency
  ]
)

common_library_dependency = declare_dependency(
  include_directories: [include_directories('include')],
  link_with: [common_library],
  dependencies: [
    string_cast_dependency,
    xml_puny_dom_dependency
  ]
)

common_test_tests = [
  'test/get_ascii.cpp',
  'test/get_buffer.cpp',
  'test/get_id3.cpp',
  'test/get_iso_iec_8859_1_1998.cpp',
  'test/get_iso_iec_10646_1_1993.cpp',
  'test/get_signed_integer.cpp',
  'test/get_unsigned_integer.cpp',
  'test/read_bits.cpp'
]

test(
  'common',
  executable(
    'common_test',
    sources: ['test/test.cpp'] + common_test_tests,
    dependencies: [common_library_dependency]
  )
)