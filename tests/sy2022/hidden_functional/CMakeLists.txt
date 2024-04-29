file(GLOB sy_files *.sy)
file(GLOB sylib_path ../sylib.c)

foreach(sy_path ${sy_files})
  string(REGEX REPLACE "\\..*$" ".in" in_path ${sy_path})
  string(REGEX REPLACE "\\..*$" ".out" out_path ${sy_path})
  get_filename_component(sy_filename ${sy_path} NAME)

  foreach(Platform ${Platforms})
    string(REGEX REPLACE "\\..*$" "_${Platform}" test ${sy_filename})
    if(EXISTS ${in_path})
      add_test(
        NAME ${test}
        COMMAND
          ${CMAKE_BINARY_DIR}/bin/atc ${sy_path} --sy --sylib ${sylib_path}
          --platform ${Platform} --dump-ir -R --R-input ${in_path} --check
          --compare-file ${out_path})
    else()
      add_test(
        NAME ${test}
        COMMAND
          ${CMAKE_BINARY_DIR}/bin/atc ${sy_path} --sy --sylib ${sylib_path}
          --platform ${Platform} --dump-ir -R --check --compare-file
          ${out_path})
    endif()

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${test}")

    set_tests_properties(${test} PROPERTIES WORKING_DIRECTORY
                                            ${CMAKE_CURRENT_BINARY_DIR}/${test})
  endforeach()

endforeach()
