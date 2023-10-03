project(frontend)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/antlr ANTLR4)

add_library(${PROJECT_NAME} ${ANTLR4})

target_include_directories(${PROJECT_NAME} PUBLIC /usr/include/antlr4-runtime)

target_link_libraries(${PROJECT_NAME} antlr4-runtime)
