file(GLOB sy_files *.sy)

foreach(file ${sy_files})
  get_filename_component(test ${file} NAME)
  string(REGEX REPLACE "\\..*$" "" test ${test})
  add_test(NAME ${test} COMMAND ${CMAKE_BINARY_DIR}/bin/atc ${file})
  set_tests_properties(${test} PROPERTIES WORKING_DIRECTORY
                                          ${CMAKE_CURRENT_BINARY_DIR})
endforeach()
