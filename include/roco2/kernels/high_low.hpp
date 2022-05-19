#ifndef INCLUDE_ROCO2_KERNELS_HIGH_LOW_HPP
#define INCLUDE_ROCO2_KERNELS_HIGH_LOW_HPP

#include <chrono>
#include <cstdint>

#include <roco2/chrono/util.hpp>
#include <roco2/kernels/base_kernel.hpp>
#include <roco2/metrics/utility.hpp>
#include <roco2/scorep.hpp>

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
        stdlib_sleep = 1 << 0,
        roco2_busy_wait = 1 << 1,
        ppc64le_linux_hmt_verylow = 1 << 2,
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
        /// run given workload until deadline
        void run_workload_until(roco2::chrono::time_point deadline, const workload_t workload_to_run) {
            switch(workload_to_run) {
            case workload_t::roco2_busy_wait:
                roco2::chrono::busy_wait_until(deadline);
                break;

            case workload_t::stdlib_sleep:
                std::this_thread::sleep_until(deadline);
                break;
                
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
