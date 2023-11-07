file(GLOB sy_files *.sy)
file(GLOB sylib_source ../sylib.c)

foreach(sy_path ${sy_files})
  string(REGEX REPLACE "\\..*$" ".in" in_path ${sy_path})
  string(REGEX REPLACE "\\..*$" ".out" out_path ${sy_path})
  get_filename_component(sy_filename ${sy_path} NAME)
  string(REGEX REPLACE "\\..*$" "" test ${sy_filename})

  if(EXISTS ${in_path})
    add_test(
      NAME ${test}
      COMMAND
        ${CMAKE_BINARY_DIR}/bin/atc ${sy_path} --emit-llvm --other-src
        ${sylib_source} -R --R-input ${in_path} --check --compare-file ${out_path}
        --compare-result ../compare_result.txt)
  else()
    add_test(
      NAME ${test}
      COMMAND
        ${CMAKE_BINARY_DIR}/bin/atc ${sy_path} --emit-llvm --other-src
        ${sylib_source} -R --check --compare-file ${out_path} --compare-result
        ../compare_result.txt)
  endif()

  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${test}")

  set_tests_properties(${test} PROPERTIES WORKING_DIRECTORY
                                          ${CMAKE_CURRENT_BINARY_DIR}/${test})
endforeach()