#ifndef __KNAPSACKS__HIGHS__
#define __KNAPSACKS__HIGHS__

#include "Highs.h"
#include "branch_and_bound.hpp"
#include "knapsacks.hpp"
#include <valarray>

/**
 * Class of the Knapsack problem, interface for HiGHS
 */
struct KnapsackHighs {
  std::vector<float> prices;
  std::vector<float> weights;
  float capacity;
  HighsModel problem;

  explicit KnapsackHighs(const Knapsack &model)
      : prices(model.prices), weights(model.weights),
        capacity(model.capacity), problem{HighsModel()} {};

  /**
   * This method is called in the branch_bound template function.
   * No integrality constrains are guaranteed
   */
  const OptimalSolution solve_relaxed(const Bounds &bounds) const {
    return this->highs_solver(bounds, false);
  };

  /**
   * This method solve the problem with integrality constrains enforced
   */
  const OptimalSolution solve_integer(const Bounds &bounds) const {
    return this->highs_solver(bounds, true);
  }

  /**
   * This is an helper method:  the interface with the HiGHS library
   */
  const OptimalSolution highs_solver(const Bounds &bounds,
                                     bool integrality) const;

  const float objective(const std::vector<float> &solution) const;
};

#endif // __KNAPSACKS__HIGHS__
