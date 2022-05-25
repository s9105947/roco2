#ifndef INCLUDE_ROCO2_KERNELS_HIGH_LOW_HPP
#define INCLUDE_ROCO2_KERNELS_HIGH_LOW_HPP

#include <chrono>
#include <cstdint>

#include <roco2/chrono/util.hpp>
#include <roco2/kernels/base_kernel.hpp>
#include <roco2/metrics/utility.hpp>
#include <roco2/metrics/cmd.hpp>
#include <roco2/scorep.hpp>

#ifdef HAS_PPC64LE_ASM_KERNELS
#include <roco2/kernels/ppc64le_asm_kernels.h>
#endif // HAS_PPC64LE_ASM_KERNELS

namespace roco2
{
namespace kernels
{
    /** encode a workload type
     * Each workload is assigned one bit of a 64 bit int,
     * s.t. one can store multiple workloads in the same metric.
     *
     * Note: use scoped enum to avoid cluttering namespace.
     */
    enum class workload_t : uint64_t {
        // typical operations
        stdlib_sleep = 1 << 0,
        roco2_busy_wait = 1 << 1,

        // copied from ppc64le linux kernel
        // note: only go to medium, anything above is privileged by default
        ppc64le_linux_hmt_verylow = 1 << 2,
        ppc64le_linux_hmt_medium = 1 << 3,

        // nop as specified by Power ISA manual
        ppc64le_nop = 1 << 4,
        ppc64le_exse = 1 << 5,

        // high latency-busy cycles ratio
        ppc64le_dfp_add = 1 << 6,
        ppc64le_dfp_add_and_exse = 1 << 7,

        // high latency
        ppc64le_xsdivqp = 1 << 8,
        ppc64le_xsdivqp_and_exse = 1 << 9,
        ppc64le_dfp_div_quad = 1 << 10,
        ppc64le_dfp_div_quad_and_exse = 1 << 11,

        // native compute
        roco2_compute = 1 << 12,

        // last item to help with iteration
        max
    };
    
    class high_low_bs : public base_kernel
    {
    public:
        template <class DT_HIGH, class DT_LOW>
        high_low_bs(DT_HIGH high_time,
                    DT_LOW low_time,
                    workload_t high_workload = workload_t::roco2_busy_wait,
                    workload_t low_workload = workload_t::stdlib_sleep)
        : high_workload(high_workload),
          low_workload(low_workload),
          high_time_(std::chrono::duration_cast<roco2::chrono::time_point::duration>(high_time)),
          low_time_(std::chrono::duration_cast<roco2::chrono::time_point::duration>(low_time))
        {
            // nop, only exists for initializer list
        }

        roco2::chrono::time_point::duration high_time() const {
            return high_time_;
        }

        roco2::chrono::time_point::duration low_time() const {
            return low_time_;
        }

        roco2::chrono::time_point::duration period() const {
            return low_time_ + high_time_;
        }

        virtual experiment_tag tag() const override
        {
            return 12;
        }

        /// workload to be executed during high phase, default: roco2_busy_wait
        const workload_t high_workload;
        /// workload to be executed during low phase, default stdlib_sleep
        const workload_t low_workload;

    private:
        /// copied from compute kernel
        template <typename Container>
        void compute_kernel(Container& A, Container& B, Container& C, std::size_t repeat)
        {
            double m = C[0];
            const auto size = thread_local_memory().vec_size;

            for (std::size_t i = 0; i < repeat; i++)
            {
                for (uint64_t i = 0; i < size; i++)
                {
                    m += B[i] * A[i];
                }
                C[0] = m;
            }
        }

        void run_compute_until(roco2::chrono::time_point deadline) {
            // already initialized, this is no allocation
            auto& vec_A = roco2::thread_local_memory().vec_A;
            auto& vec_B = roco2::thread_local_memory().vec_B;
            auto& vec_C = roco2::thread_local_memory().vec_C;

            std::size_t loops = 0;

            while (std::chrono::high_resolution_clock::now() <= deadline)
            {
                if (vec_C[0] == 123.12345)
                    vec_A[0] += 1.0;

                compute_kernel(vec_A, vec_B, vec_C, 32);

                loops++;
            }

            // just as a data dependency
            volatile int dd = 0;
            if (vec_C[0] == 42.0)
                dd++;
        }

        /// run given workload until deadline
        void run_workload_until(roco2::chrono::time_point deadline, const workload_t workload_to_run) {
            switch(workload_to_run) {
            case workload_t::roco2_busy_wait:
                roco2::chrono::busy_wait_until(deadline);
                break;

            case workload_t::stdlib_sleep:
                std::this_thread::sleep_until(deadline);
                break;

            case workload_t::roco2_compute:
                run_compute_until(deadline);
                break;

#ifdef HAS_PPC64LE_ASM_KERNELS

                // use macro to avoid boilerplate
#define RUN_PPC64LE_ASM_KERNEL_UNTIL_DEADLINE(id)                       \
                case workload_t::ppc64le_ ## id:                        \
                    while(std::chrono::high_resolution_clock::now() < deadline) { \
                        ppc64le_ ## id (1 << 10);                       \
                    }                                                   \
                    break;

                RUN_PPC64LE_ASM_KERNEL_UNTIL_DEADLINE(linux_hmt_verylow)
                RUN_PPC64LE_ASM_KERNEL_UNTIL_DEADLINE(linux_hmt_medium)
                RUN_PPC64LE_ASM_KERNEL_UNTIL_DEADLINE(nop)
                RUN_PPC64LE_ASM_KERNEL_UNTIL_DEADLINE(exse)
                RUN_PPC64LE_ASM_KERNEL_UNTIL_DEADLINE(dfp_add)
                RUN_PPC64LE_ASM_KERNEL_UNTIL_DEADLINE(dfp_add_and_exse)
                RUN_PPC64LE_ASM_KERNEL_UNTIL_DEADLINE(xsdivqp)
                RUN_PPC64LE_ASM_KERNEL_UNTIL_DEADLINE(xsdivqp_and_exse)
                RUN_PPC64LE_ASM_KERNEL_UNTIL_DEADLINE(dfp_div_quad)
                RUN_PPC64LE_ASM_KERNEL_UNTIL_DEADLINE(dfp_div_quad_and_exse)

#endif // HAS_PPC64LE_ASM_KERNELS

            default:
                throw std::logic_error("unknown workload: " + std::to_string(static_cast<uint64_t>(workload_to_run)));
            }
        }

        void run_kernel(roco2::chrono::time_point tp) override
        {
#ifdef HAS_SCOREP
            SCOREP_USER_REGION("high_low_bs_kernel", SCOREP_USER_REGION_TYPE_FUNCTION)
#endif
            roco2::chrono::time_point deadline = std::chrono::high_resolution_clock::now();
            // align across all procs
            deadline -= deadline.time_since_epoch() % period();

            // record frequency of highlow pattern to trace
            roco2::metrics::utility::instance().write(std::chrono::seconds(1) / std::chrono::duration_cast<std::chrono::duration<double>>(period()));

            // encode both commands for high & low phase to trace
            // both are bitmask -> combine them
            roco2::metrics::cmd::instance().write(static_cast<uint64_t>(high_workload) | static_cast<uint64_t>(low_workload));

            std::size_t loops = 0;

            while (true)
            {
                {
#ifdef ROCO2_HIGHLOW_INSTRUMENT_PHASES
                    SCOREP_USER_REGION("high_low_bs_kernel: high phase", SCOREP_USER_REGION_TYPE_FUNCTION)
#endif
                    deadline += high_time_;
                    if (deadline >= tp)
                    {
                        run_workload_until(tp, high_workload);
                        break;
                    }
                    run_workload_until(deadline, high_workload);
                }

                {
#ifdef ROCO2_HIGHLOW_INSTRUMENT_PHASES
                    SCOREP_USER_REGION("high_low_bs_kernel: low phase", SCOREP_USER_REGION_TYPE_FUNCTION)
#endif
                    deadline += low_time_;
                    if (deadline >= tp)
                    {
                        run_workload_until(tp, low_workload);
                        break;
                    }
                    run_workload_until(deadline, low_workload);
                }

                loops++;
            }
        }

        roco2::chrono::time_point::duration high_time_;
        roco2::chrono::time_point::duration low_time_;
    };
} // namespace kernels
} // namespace roco2

#endif // INCLUDE_ROCO2_KERNELS_HIGH_LOW_HPP
