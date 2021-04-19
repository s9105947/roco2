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

using namespace roco2::experiments::patterns;

void run_experiments(roco2::chrono::time_point starting_point, bool eta_only)
{
    // half (!) period length
    auto high_low_duration = std::chrono::milliseconds(100);
    roco2::kernels::high_low_bs high_low_kernel(high_low_duration, high_low_duration);

    roco2::cpu::shell cstatectl("", "elab cstate enable", "elab cstate enable");

    roco2::memory::numa_bind_local nbl;

    // ------ EDIT GENERIC SETTINGS BELOW THIS LINE ------

    auto experiment_duration = std::chrono::seconds(10);

    auto on_list = block_pattern(1);

    auto cstate_list =
        std::vector<roco2::cpu::shell::setting_type>{ { 0, "elab cstate enable --only POLL" },
                                                      { 1, "elab cstate enable C1" },
                                                      { 2, "elab cstate enable C2" } };

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

    auto setting = [&](auto lambda) { plan.push_back(roco2::task::make_lambda_task(lambda)); };

    // ------ EDIT TASK PLAN BELOW THIS LINE ------

    for (const auto& cstate_setting : cstate_list)
    {
        setting([&cstatectl, cstate_setting]() { cstatectl.change(cstate_setting); });

        for (const auto& on : on_list)
        {
            experiment(high_low_kernel, on);
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
