
# Firestarter is only for 64-bit x86
set(USE_FIRESTARTER_DEFAULT ON)
if(NOT "x86_64" STREQUAL "${CMAKE_SYSTEM_PROCESSOR}")
    set(USE_FIRESTARTER_DEFAULT OFF)
endif()

option(USE_FIRESTARTER "enable firestarter in build" ${USE_FIRESTARTER_DEFAULT})

# asm kernels are only built for x86_64
set(USE_ASM_KERNELS_DEFAULT ON)
if(NOT "x86_64" STREQUAL "${CMAKE_SYSTEM_PROCESSOR}")
    set(USE_ASM_KERNELS_DEFAULT OFF)
endif()

option(USE_ASM_KERNELS "build support for asm kernels" ${USE_ASM_KERNELS_DEFAULT})

# ppc64le asm kernels are only built for ppc64le
set(USE_PPC64LE_ASM_KERNELS_DEFAULT ON)
if(NOT "ppc64le" STREQUAL "${CMAKE_SYSTEM_PROCESSOR}")
    set(USE_PPC64LE_ASM_KERNELS_DEFAULT OFF)
endif()

option(USE_PPC64LE_ASM_KERNELS "build support for ppc64le asm kernels" ${USE_PPC64LE_ASM_KERNELS_DEFAULT})
