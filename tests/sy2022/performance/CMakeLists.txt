file(GLOB sy_files *.sy)

foreach(sy_path ${sy_files})
  create_sy_test(${sy_path})
endforeach()
