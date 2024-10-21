#ifndef __KNAPSACKS__HIGHS__
#define __KNAPSACKS__HIGHS__

#include "Highs.h"
#include "knapsacks.hpp"
#include "utilities.hpp"
#include <lp_data/HConst.h>

/**
 * Class of the Knapsack problem, interface for HiGHS
 */
struct KnapsackHighs {
  HighsModel problem;
  Highs highs;

  explicit KnapsackHighs(const Knapsack &model)
      : problem{HighsModel()}, highs{Highs()} {
    auto problem_size = model.prices.size();
    problem.lp_.num_col_ = problem_size;
    problem.lp_.num_row_ = 1;
    problem.lp_.sense_ = ObjSense::kMaximize;
    problem.lp_.col_cost_ = model.prices;
    // std::vector<double>(model.prices.begin(), model.prices.end());

    // constraints
    problem.lp_.row_lower_ = {0.0};
    problem.lp_.row_upper_ = {model.capacity};
    problem.lp_.a_matrix_.num_row_ = 1;
    problem.lp_.a_matrix_.num_col_ = problem_size;
    problem.lp_.a_matrix_.format_ = MatrixFormat::kColwise;
    std::vector<int> ind(problem_size + 1);
    std::iota(ind.begin(), ind.end(), 0);
    problem.lp_.a_matrix_.start_ = ind;
    problem.lp_.a_matrix_.index_ = std::vector<int>(problem_size, 0);
    problem.lp_.a_matrix_.value_ = model.weights;
    // std::vector<double>(std::begin(model.weights), std::end(model.weights));

    problem.lp_.col_lower_ = std::vector<double>(problem_size);
    problem.lp_.col_upper_ = std::vector<double>(problem_size);
    problem.lp_.integrality_ =
        std::vector<HighsVarType>(problem_size, HighsVarType::kContinuous);

    HighsStatus return_status;

    highs.setOptionValue("mip_rel_gap", 1e-8);
    highs.setOptionValue("output_flag", false);

    return_status = highs.passModel(problem);

    assert(return_status == HighsStatus::kOk);
  };

  /**
   * This method is called in the branch_bound template function.
   * No integrality constrains are guaranteed
   */
  const OptimalSolution solve_relaxed(const Bounds &bounds);

  /**
   * This method solve the problem with integrality constrains enforced
   */
  const OptimalSolution solve_integer(const Bounds &bounds) {
    HighsStatus return_status;
    std::vector<HighsVarType> opt_integrality(bounds.lower.size(),
                                              HighsVarType::kInteger);
    return_status = highs.changeColsIntegrality(0, bounds.lower.size() - 1,
                                                opt_integrality.data());
    assert(return_status == HighsStatus::kOk);
    return this->solve_relaxed(bounds);
  }

  const double objective(const std::vector<double> &solution) const;
};

#endif // __KNAPSACKS__HIGHS__
