project(atc)

cmake_minimum_required(VERSION 3.10)

# 重新生成antlr前端
if(UPDATE_ANTLR STREQUAL "ON")
  execute_process(COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/antlr)
  execute_process(
    COMMAND
      antlr4 -Dlanguage=Cpp -no-listener -visitor -o
      ${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/antlr
      ${CMAKE_CURRENT_SOURCE_DIR}/ATC.g4)
  message("update antlr code success!")
endif()

add_subdirectory(src/frontend)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} frontend)

set_target_properties(atc PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                     ${CMAKE_BINARY_DIR}/bin)
enable_testing()
add_subdirectory(tests)
