#ifndef INCLUDE_ROCO2_KERNELS_PCC64_ASM_KERNELS_H
#define INCLUDE_ROCO2_KERNELS_PPC64_ASM_KERNELS_H

#ifdef __cplusplus
#define EXTERN_C extern "C"
#include <cstdint>
#else
#define EXTERN_C
#include <stdint.h>
#endif

EXTERN_C uint64_t ppc64_linux_hmt_verylow(const uint64_t passes_desired);
EXTERN_C uint64_t ppc64_linux_hmt_medium(const uint64_t passes_desired);
EXTERN_C uint64_t ppc64_nop(const uint64_t passes_desired);
EXTERN_C uint64_t ppc64_exse(const uint64_t passes_desired);
EXTERN_C uint64_t ppc64_dfp_add(const uint64_t passes_desired);
EXTERN_C uint64_t ppc64_dfp_add_and_exse(const uint64_t passes_desired);
EXTERN_C uint64_t ppc64_xsdivqp(const uint64_t passes_desired);
EXTERN_C uint64_t ppc64_xsdivqp_and_exse(const uint64_t passes_desired);
EXTERN_C uint64_t ppc64_dfp_div_quad(const uint64_t passes_desired);
EXTERN_C uint64_t ppc64_dfp_div_quad_and_exse(const uint64_t passes_desired);

#endif // INCLUDE_ROCO2_KERNELS_PPC64_ASM_KERNELS_H
