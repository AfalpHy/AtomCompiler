project(frontend)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/antlr ANTLR4)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/visitors VISITORS)

add_library(${PROJECT_NAME} ${ANTLR4} ${VISITORS})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/antlr
  PUBLIC /usr/include/antlr4-runtime
  PUBLIC ${CMAKE_SOURCE_DIR}/include/frontend/visitors)

target_link_libraries(${PROJECT_NAME} antlr4-runtime)
