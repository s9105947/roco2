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
#include <iostream>

using namespace roco2::experiments::patterns;
using namespace std::chrono_literals;

void run_experiments(roco2::chrono::time_point starting_point, bool eta_only)
{
    std::vector<roco2::kernels::high_low_bs> kernels;

    for (double freq_hz = 495; freq_hz <= 505; freq_hz += 1) {
        std::chrono::duration<double> period_length = std::chrono::seconds(1) / freq_hz;
        kernels.push_back(roco2::kernels::high_low_bs(period_length / 2, period_length / 2));
    }

#pragma omp master
    {
        std::cerr << "F (Hz)\tT (µs)\tT/2 (µs)" << std::endl;

        for (const auto& k : kernels) {
            using us = std::chrono::duration<double, std::micro>;
            double freq_hz = std::chrono::duration<double>(1) / k.get_period();
            std::cerr << freq_hz << "\t" << us(k.get_period()).count() << "\t" << us(k.get_high_time()).count() << std::endl;
        }
    }

    roco2::memory::numa_bind_local nbl; 

    // ------ EDIT GENERIC SETTINGS BELOW THIS LINE ------

    auto experiment_duration = std::chrono::seconds(30);

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
