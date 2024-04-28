project(atc)

cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/antlr")
  execute_process(
    COMMAND
      antlr4 -Dlanguage=Cpp -no-listener -visitor -o
      ${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/antlr
      ${CMAKE_CURRENT_SOURCE_DIR}/ATC.g4)
  message("generate antlr code success!")
endif()

add_subdirectory(src/frontend)
add_subdirectory(src/backend)

add_executable(${PROJECT_NAME} src/main.cpp src/CmdOption.cpp)

target_include_directories(${PROJECT_NAME}
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(LLVM REQUIRED CONFIG)

target_link_libraries(${PROJECT_NAME} frontend backend LLVM)

set_target_properties(atc PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                     ${CMAKE_BINARY_DIR}/bin)

enable_testing()
add_subdirectory(tests)
