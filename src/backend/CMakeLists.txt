project(backend)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/arm ARM)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/riscv RISCV)

add_library(${PROJECT_NAME} ${ARM} ${RISCV})

target_include_directories(${PROJECT_NAME}
                           PUBLIC ${CMAKE_SOURCE_DIR}/include/backend)
