subdir('source')

common_library = static_library(
  'common',
  sources: common_library_sources,
  include_directories: [
    'include',
    'source'
  ],
  dependencies: [
    xml_puny_dom_dependency
  ]
)

common_library_dependency = declare_dependency(
  include_directories: [
    include_directories('include')
  ],
  link_with: [common_library],
  dependencies: [
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
