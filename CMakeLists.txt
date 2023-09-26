project(atc)

cmake_minimum_required(VERSION 3.10)

# 重新生成antlr语法分析器
execute_process(COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/antlr)
execute_process(COMMAND antlr4 -Dlanguage=Cpp -no-listener -visitor -o ${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/antlr ${CMAKE_CURRENT_SOURCE_DIR}/ATC.g4)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/antlr ANTLR4)

add_executable(${PROJECT_NAME} src/main.cpp ${ANTLR4})

target_include_directories(${PROJECT_NAME} PUBLIC /usr/include/antlr4-runtime
PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/frontend/antlr)

target_link_libraries(${PROJECT_NAME} antlr4-runtime)

