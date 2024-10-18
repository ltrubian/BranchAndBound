#include "knapsacks_highs.hpp"
#include "Highs.h"
#include "branch_and_bound.hpp"
#include "utilities.hpp"
#include <algorithm>
#include <cstdio>
#include <lp_data/HConst.h>
#include <lp_data/HStruct.h>
#include <lp_data/HighsStatus.h>
#include <numeric>
#include <utility>
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
  problem.lp_.a_matrix_.num_row_ = 1;
  problem.lp_.a_matrix_.num_col_ = this->prices.size();
  problem.lp_.a_matrix_.format_ = MatrixFormat::kColwise;
  std::vector<int> ind(this->prices.size() + 1);
  std::iota(ind.begin(), ind.end(), 0);
  problem.lp_.a_matrix_.start_ = ind;
  problem.lp_.a_matrix_.index_ = std::vector<int>(this->prices.size(), 0);
  problem.lp_.a_matrix_.value_ =
      std::vector<double>(std::begin(this->weights), std::end(this->weights));
  // integrality ?
  if (integrality) {
    problem.lp_.integrality_.resize(problem.lp_.num_col_);
    std::fill(problem.lp_.integrality_.begin(), problem.lp_.integrality_.end(),
              HighsVarType::kInteger);
  }
  // solve
  Highs highs;
  HighsStatus return_status;
  highs.setHighsOutput(tmpfile());
  return_status = highs.passModel(problem);
  assert(return_status == HighsStatus::kOk);

  return_status = highs.run();
  assert(return_status == HighsStatus::kOk);

  const HighsSolution &info = highs.getSolution();
  opt_sol.success = highs.getInfo().primal_solution_status;

  if (opt_sol.success) {
    opt_sol.solution =
        std::vector<MFloat>(info.col_value.begin(), info.col_value.end());
    opt_sol.value = this->objective(opt_sol.solution);
  }
  return opt_sol;
}

const MFloat KnapsackHighs::objective(const std::vector<MFloat> &solution) const {
  MFloat res{0.0};
  auto sol{std::begin(solution)};
  for (auto price{std::begin(this->prices)}; price != std::end(this->prices);
       ++price) {
    res = std::move(res) + *price * *sol;
    ++sol;
  }
  return res;
}

template <>
std::pair<OptimalSolution, std::unique_ptr<Node>>
branch_bound<KnapsackHighs, DepthFirst, PruneNone>(const KnapsackHighs &,
                                                   const Bounds &,
                                                   OptimalSolution);

template <>
std::pair<OptimalSolution, std::unique_ptr<Node>>
branch_bound<KnapsackHighs, DepthFirst, PruneUntill>(const KnapsackHighs &,
                                                     const Bounds &,
                                                     OptimalSolution);
template <>
std::pair<OptimalSolution, std::unique_ptr<Node>>
branch_bound<KnapsackHighs, DepthFirst, PruneAll>(const KnapsackHighs &,
                                                  const Bounds &,
                                                  OptimalSolution);
template <>
std::pair<OptimalSolution, std::unique_ptr<Node>>
branch_bound<KnapsackHighs, BestBoundFirst, PruneNone>(const KnapsackHighs &,
                                                       const Bounds &,
                                                       OptimalSolution);

template <>
std::pair<OptimalSolution, std::unique_ptr<Node>>
branch_bound<KnapsackHighs, BestBoundFirst, PruneUntill>(const KnapsackHighs &,
                                                         const Bounds &,
                                                         OptimalSolution);

template <>
std::pair<OptimalSolution, std::unique_ptr<Node>>
branch_bound<KnapsackHighs, BestBoundFirst, PruneAll>(const KnapsackHighs &,
                                                      const Bounds &,
                                                      OptimalSolution);
