#include "knapsacks_highs.hpp"
#include "Highs.h"
#include "branch_and_bound.hpp"
#include "utilities.hpp"
#include <cstdio>
#include <lp_data/HConst.h>
#include <lp_data/HStruct.h>
#include <lp_data/HighsStatus.h>
#include <numeric>
#include <utility>
#include <vector>

const Solution KnapsackHighs::solve_relaxed(const Bounds &bounds)  {
  Solution opt_sol{};

  HighsStatus return_status;

  return_status = highs.changeColsBounds(0, bounds.lower.size()-1, bounds.lower.data(),
                         bounds.upper.data());


  assert(return_status == HighsStatus::kOk);

  return_status = highs.run();
  assert(return_status == HighsStatus::kOk);

  const HighsSolution &info = highs.getSolution();
  opt_sol.success = highs.getInfo().primal_solution_status;

  if (opt_sol.success) {
    opt_sol.solution =
        std::vector<double>(info.col_value.begin(), info.col_value.end());
    opt_sol.value = highs.getInfo().objective_function_value;
  }
  return opt_sol;
}

const double
KnapsackHighs::objective(const std::vector<double> &solution) const {
  return std::inner_product(solution.begin(), solution.end(), problem.lp_.col_cost_.begin(), 0.);
}

template <>
std::pair<Solution, std::shared_ptr<Node>>
branch_bound<KnapsackHighs, QueueDepth>(const KnapsackHighs &,
                                                   const Bounds &,
                                                   Solution, std::chrono::seconds);

template <>
std::pair<Solution, std::shared_ptr<Node>>
branch_bound<KnapsackHighs, QueueBestBound>(const KnapsackHighs &,
                                                     const Bounds &,
                                                     Solution, std::chrono::seconds);
