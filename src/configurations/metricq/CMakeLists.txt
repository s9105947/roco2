macro(roco2_metricq_exp EXP_NAME)
    add_executable(roco2_metricq_${EXP_NAME} ${EXP_NAME}.cpp)

    target_link_libraries(
        roco2_metricq_${EXP_NAME}
        roco2_main
        roco2_core
        )
    target_compile_features(roco2_metricq_${EXP_NAME} PRIVATE cxx_std_14)
    target_include_directories(roco2_metricq_${EXP_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

    install(TARGETS roco2_metricq_${EXP_NAME} RUNTIME DESTINATION bin)
endmacro()

roco2_metricq_exp(busywait)
roco2_metricq_exp(sinus)
roco2_metricq_exp(matmul)
roco2_metricq_exp(compute)
roco2_metricq_exp(mem_read)
roco2_metricq_exp(mem_write)
roco2_metricq_exp(mem_copy)

Roco2Configuration(metricq)
target_compile_features(roco2_metricq PRIVATE cxx_std_14)
