add_library(roco2_core STATIC
    metrics.cpp
    cpu.cpp
    c_state_limit.c
)
target_link_libraries(roco2_core PUBLIC roco2)

add_library(roco2_main STATIC main.cpp)
target_link_libraries(roco2_main PUBLIC roco2)

add_subdirectory(configurations)

add_subdirectory(test)
