#ifndef __KNAPSACKS__HIGHS__
#define __KNAPSACKS__HIGHS__

#include "Highs.h"
#include "knapsacks.hpp"
#include "utilities.hpp"
#include <lp_data/HConst.h>
#include <algorithm>

/**
 * Class of the Knapsack problem, interface for HiGHS
 */
struct KnapsackHighs {
  HighsModel problem;

  explicit KnapsackHighs(const Knapsack &model) : problem{HighsModel()} {
    auto problem_size = model.prices.size();
    problem.lp_.num_col_ = problem_size;
    problem.lp_.num_row_ = 1;
    problem.lp_.sense_ = ObjSense::kMaximize;
    problem.lp_.col_cost_ = model.prices;

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

    problem.lp_.col_lower_ = std::vector<double>(problem_size, 0.);
    problem.lp_.col_upper_ = std::vector<double>(problem_size, 1.);

    auto tmp = std::vector<HighsVarType>(problem_size, HighsVarType::kContinuous);
    for (std::size_t ind{0}; ind < problem_size; ++ind){
      if(model.should_var_integer(ind))
        tmp[ind] = HighsVarType::kInteger;
    }
    problem.lp_.integrality_ = tmp;

  };

  /**
   * This method is called in the branch_bound template function.
   * No integrality constrains are guaranteed
   */
  const Solution solve_highs(Highs &highs, const Bounds &bounds) const;

  /**
   * This method solve the problem with integrality constrains enforced
   */
  const Solution solve_integer(const Bounds &bounds) const {
    Highs highs;
    highs.setOptionValue("mip_rel_gap", 1e-8);
    highs.setOptionValue("output_flag", false);
    HighsStatus return_status;
    // pass model to highs
    return_status = highs.passModel(problem);
    assert(return_status == HighsStatus::kOk);
    // solve it
    return this->solve_highs(highs, bounds);
  }

  // __START_REQUIRED__: branch and bound algorithm
  const Solution solve_relaxed(const Bounds &bounds) const {
    Highs highs;
    highs.setOptionValue("mip_rel_gap", 1e-8);
    highs.setOptionValue("output_flag", false);
    HighsStatus return_status;
    // pass model to highs
    return_status = highs.passModel(problem);
    assert(return_status == HighsStatus::kOk);
    // set integrality constraints
    std::vector<HighsVarType> opt_integrality(bounds.lower.size(),
                                              HighsVarType::kContinuous);
    return_status = highs.changeColsIntegrality(0, bounds.lower.size() - 1,
                                                opt_integrality.data());
    assert(return_status == HighsStatus::kOk);
    // solve it
    return this->solve_highs(highs, bounds);
  };
  const double objective(const Solution &solution) const;
  inline const bool should_var_integer(const std::size_t index) const;
  const Bounds &bounds() const;
  // __END_REQUIRED__
};

#endif // __KNAPSACKS__HIGHS__
