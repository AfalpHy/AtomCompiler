project(frontend)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/antlr ANTLR4)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/AST AST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/AST/tools AST_TOOLS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/llvmIR LLVM_IR)

add_library(${PROJECT_NAME} ${ANTLR4} ${AST} ${AST_TOOLS} ${LLVM_IR})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/antlr
  PUBLIC /usr/include/antlr4-runtime
  PUBLIC ${CMAKE_SOURCE_DIR}/include/frontend)

target_link_libraries(${PROJECT_NAME} antlr4-runtime "LLVM-14")
