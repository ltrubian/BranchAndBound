#include "knapsacks_highs.hpp"
#include "Highs.h"
#include "branch_bound.hpp"
#include <algorithm>
#include <lp_data/HConst.h>
#include <lp_data/HStruct.h>
#include <lp_data/HighsStatus.h>
#include <numeric>
#include <utility>
#include <valarray>
#include <vector>

const OptimalSolution KnapsackHighs::highs_solver(const Bounds &bounds,
                                                  bool integrality) const {
  OptimalSolution opt_sol{};
  HighsModel problem{this->problem};
  problem.lp_.num_col_ = this->prices.size();
  problem.lp_.num_row_ = 1;
  problem.lp_.sense_ = ObjSense::kMaximize;
  problem.lp_.col_cost_ =
      std::vector<double>(std::begin(this->prices), std::end(this->prices));

  // bounds
  problem.lp_.col_lower_ =
      std::vector<double>(std::begin(bounds.lower), std::end(bounds.lower));
  problem.lp_.col_upper_ =
      std::vector<double>(std::begin(bounds.upper), std::end(bounds.upper));

  // constraints
  problem.lp_.row_lower_ = {0.0};
  problem.lp_.row_upper_ = {this->capacity};
  problem.lp_.a_matrix_.value_ =
      std::vector<double>(std::begin(this->weights), std::end(this->weights));
  // integrality ?
  if (integrality) {
    problem.lp_.integrality_.resize(problem.lp_.num_col_);
    for (auto &variable : problem.lp_.integrality_)
      variable = HighsVarType::kInteger;
  }
  // solve
  Highs highs;
  HighsStatus return_status;
  return_status = highs.passModel(problem);
  assert(return_status == HighsStatus::kOk);

  return_status = highs.run();
  assert(return_status == HighsStatus::kOk);

  const HighsSolution &info = highs.getSolution();
  opt_sol.success = highs.getInfo().primal_solution_status;

  std::vector<float> tmp{info.col_value.begin(), info.col_value.end()};
  opt_sol.solution = std::valarray<float>(tmp.data(), tmp.size());

  return opt_sol;
}
const float
KnapsackHighs::objective(const std::valarray<float> &solution) const {
  float res{0.0};
  auto sol{std::begin(solution)};
  for (auto price{std::begin(this->prices)}; price != std::begin(this->prices);
       ++price) {
    res = std::move(res) + *price * *sol;
    ++sol;
  }
  return -res;
}

/*
  std::valarray<std::pair<float, std::size_t>> indexes{real_prices.size()};
  std::generate(std::begin(indexes), std::end(indexes), [&]() {
    static int ind{-1};
    ++ind;
    return std::pair<float, std::size_t>(real_prices[ind], ind);
  });
  std::sort(
      std::begin(indexes), std::end(indexes),
      [&](std::tuple<float, std::size_t> x, std::tuple<float, std::size_t> y) {
        return std::get<0>(x) >= std::get<0>(y);
      });
*/
