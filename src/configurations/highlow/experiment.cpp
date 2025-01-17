#include <roco2/initialize.hpp>

#include <roco2/cpu/shell.hpp>
#include <roco2/cpu/topology.hpp>

#include <roco2/memory/numa.hpp>

#include <roco2/chrono/chrono.hpp>
#include <roco2/log.hpp>

#include <roco2/experiments/const_length.hpp>
#include <roco2/experiments/patterns/pattern_generator.hpp>

#include <roco2/kernels/high_low.hpp>

#include <roco2/task/experiment_task.hpp>
#include <roco2/task/lambda_task.hpp>
#include <roco2/task/task_plan.hpp>

#include <string>
#include <vector>
#include <chrono>
#include <ratio>

using namespace roco2::experiments::patterns;
using namespace std::chrono_literals;

void run_experiments(roco2::chrono::time_point starting_point, bool eta_only)
{
    std::vector<roco2::kernels::high_low_bs> kernels = {
        roco2::kernels::high_low_bs(5s, 5s),
        roco2::kernels::high_low_bs(1s, 1s),
        roco2::kernels::high_low_bs(500ms, 500ms),
        roco2::kernels::high_low_bs(80ms, 80ms),
        roco2::kernels::high_low_bs(41ms, 41ms),
        roco2::kernels::high_low_bs(40ms, 40ms),
        roco2::kernels::high_low_bs(21ms, 21ms),
        roco2::kernels::high_low_bs(20ms, 20ms),
//        roco2::kernels::high_low_bs(1010us, 1010us),
//        roco2::kernels::high_low_bs(1000us, 1000us),
//        roco2::kernels::high_low_bs(510us, 510us),
//        roco2::kernels::high_low_bs(500us, 500us),
//        roco2::kernels::high_low_bs(260us, 260us),
//        roco2::kernels::high_low_bs(250941ns, 250941ns),
//        roco2::kernels::high_low_bs(250us, 250us),
//        roco2::kernels::high_low_bs(249066ns, 249066ns),
//        roco2::kernels::high_low_bs(130us, 130us),
//        roco2::kernels::high_low_bs(125us, 125us),
    };

    roco2::memory::numa_bind_local nbl;

    // ------ EDIT GENERIC SETTINGS BELOW THIS LINE ------

    auto experiment_duration = std::chrono::seconds(10);

    //auto on_list = block_pattern(176) >> block_pattern(88);
    auto on_list = block_pattern(176);

    // ------ EDIT GENERIC SETTINGS ABOVE THIS LINE ------

    roco2::task::task_plan plan;

#pragma omp master
    {
        roco2::log::info() << "Number of placements:  " << on_list.size() << on_list;
    }

#pragma omp barrier

    auto experiment_startpoint =
        roco2::initialize::thread(starting_point, experiment_duration, eta_only);

    roco2::experiments::const_lenght exp(experiment_startpoint, experiment_duration);

    auto experiment = [&](auto& kernel, const auto& on) {
        plan.push_back(roco2::task::experiment_task(exp, kernel, on));
    };

    // ------ EDIT TASK PLAN BELOW THIS LINE ------

    for (const auto& on : on_list) {
        // note: const kernels are not accepted by compiler
        for (auto& k : kernels) {
            experiment(k, on);
        }
    }

    // ------ EDIT TASK PLAN ABOVE THIS LINE ------

#pragma omp master
    {
        roco2::log::info() << "ETA for whole execution: "
                           << std::chrono::duration_cast<std::chrono::seconds>(plan.eta());
    }

    if (!eta_only)
    {
#pragma omp barrier

        plan.execute();
    }
}
