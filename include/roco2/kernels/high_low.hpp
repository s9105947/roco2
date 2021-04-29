#ifndef INCLUDE_ROCO2_KERNELS_HIGH_LOW_HPP
#define INCLUDE_ROCO2_KERNELS_HIGH_LOW_HPP

#include <roco2/chrono/util.hpp>
#include <roco2/kernels/base_kernel.hpp>
#include <roco2/metrics/utility.hpp>
#include <roco2/scorep.hpp>

namespace roco2
{
namespace kernels
{
    class high_low_bs : public base_kernel
    {
    public:
        template <class DT_HIGH, class DT_LOW>
        high_low_bs(DT_HIGH high_time, DT_LOW low_time)
        : high_time_(std::chrono::duration_cast<roco2::chrono::time_point::duration>(high_time)),
          low_time_(std::chrono::duration_cast<roco2::chrono::time_point::duration>(low_time))
        {
        }

        virtual experiment_tag tag() const override
        {
            return 12;
        }

    private:
        void run_kernel(roco2::chrono::time_point tp) override
        {
#ifdef HAS_SCOREP
            SCOREP_USER_REGION("high_low_bs_kernel", SCOREP_USER_REGION_TYPE_FUNCTION)
#endif
            roco2::chrono::time_point deadline = std::chrono::high_resolution_clock::now();

            std::size_t loops = 0;

            while (true)
            {
                {
#ifdef HAS_SCOREP
                    SCOREP_USER_REGION("high_low_bs_kernel: high phase", SCOREP_USER_REGION_TYPE_FUNCTION)
#endif

                    deadline += high_time_;
                    if (deadline >= tp)
                    {
                        roco2::chrono::busy_wait_until(tp);
                        break;
                    }
                    roco2::chrono::busy_wait_until(deadline);
                }
                {
#ifdef HAS_SCOREP
                    SCOREP_USER_REGION("high_low_bs_kernel: low phase", SCOREP_USER_REGION_TYPE_FUNCTION)
#endif
                    deadline += low_time_;
                    if (deadline >= tp)
                    {
                        std::this_thread::sleep_until(tp);
                        break;
                    }
                    std::this_thread::sleep_until(deadline);
                }

                loops++;
            }

            roco2::metrics::utility::instance().write(loops);
        }

        roco2::chrono::time_point::duration high_time_;
        roco2::chrono::time_point::duration low_time_;
    };
} // namespace kernels
} // namespace roco2

#endif // INCLUDE_ROCO2_KERNELS_HIGH_LOW_HPP
