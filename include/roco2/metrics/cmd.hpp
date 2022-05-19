#ifndef INCLUDE_ROCO2_METRICS_CMD_HPP
#define INCLUDE_ROCO2_METRICS_CMD_HPP

#include <cstdint>
#include <roco2/scorep.hpp>

#ifdef HAS_SCOREP
SCOREP_USER_METRIC_EXTERNAL(cmd_metric)
#endif

namespace roco2
{
namespace metrics
{

    class cmd
    {
        cmd()
        {
#ifdef HAS_SCOREP
            SCOREP_USER_METRIC_INIT(cmd_metric, "Command", "#",
                                    SCOREP_USER_METRIC_TYPE_UINT64,
                                    SCOREP_USER_METRIC_CONTEXT_GLOBAL)
#endif
        }

    public:
        using value_type = std::uint64_t;

        cmd(const cmd&) = delete;
        cmd& operator=(const cmd&) = delete;

        static cmd& instance()
        {
            static cmd e;

            return e;
        }

        void write(std::uint64_t value);
    };
} // namespace metrics
} // namespace roco2

#endif // INCLUDE_ROCO2_METRICS_CMD_HPP
