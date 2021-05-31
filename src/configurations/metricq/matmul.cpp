#include <roco2/initialize.hpp>
#include <roco2/chrono/chrono.hpp>
#include <roco2/kernels/matmul.hpp>

#include <memory>

#include <experiment.hpp>

void run_experiments(roco2::chrono::time_point starting_point, bool eta_only)
{
    run_p9_triangle(starting_point, eta_only, std::make_shared<roco2::kernels::matmul>());
}
