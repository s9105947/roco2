#include <iostream>
#include <cstdint>
#include <chrono>
#include <roco2/kernels/ppc64le_asm_kernels.h>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;

void minibench(const char* name, uint64_t (*benchmarked)(uint64_t)) {
    auto start = high_resolution_clock::now();
    uint64_t passes = (*benchmarked)(1 << 22);
    auto end = high_resolution_clock::now();

    double time_per_pass_ns = duration_cast<nanoseconds>(end-start).count() / (double) passes;

    std::cout << name << " per pass: " << time_per_pass_ns << "ns" << std::endl;
}

int main() {
    minibench("linux hmt verlow", &ppc64le_linux_hmt_verylow);
    minibench("linux hmt medium", &ppc64le_linux_hmt_medium);
    minibench("nop", &ppc64le_nop);
    minibench("exse", &ppc64le_exse);
    minibench("dfp add", &ppc64le_dfp_add);
    minibench("dfp add and exse", &ppc64le_dfp_add_and_exse);
    minibench("dfp div quad", &ppc64le_dfp_div_quad);
    minibench("dfp div quad and exse", &ppc64le_dfp_div_quad_and_exse);
    minibench("xsdivqp", &ppc64le_xsdivqp);
    minibench("xsdivqp and exse", &ppc64le_xsdivqp_and_exse);
    return 0;
}

