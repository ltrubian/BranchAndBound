#ifndef __KNAPSACKS__HIGHS__
#define __KNAPSACKS__HIGHS__

#include "branch_bound.hpp"
#include <valarray>
#include "Highs.h"

/**
 * Class of the Knapsack problem, interface for HiGHS
 */
struct KnapsackHighs {
  std::valarray<float> prices;
  std::valarray<float> weights;
  float capacity;
  Bounds bounds;
  HighsModel problem;

  /**
   * This method is called in the branch_bound template function.
   * No integrality constrains are guaranteed
   */
  const OptimalSolution solve_relaxed(const Bounds &bounds) const {
    return this->highs_solver(bounds, false );
  };

  /**
   * This method solve the problem with integrality constrains enforced
   */
  const OptimalSolution solve_integer(const Bounds &bounds) const{
    return this->highs_solver(bounds, true );
  }

  /**
   * This is an helper method:  the interface with the HiGHS library
   */
  const OptimalSolution highs_solver(const Bounds &bounds,
                                     bool integrality) const;

  const float objective(const std::valarray<float> &solution) const;
};

#endif // __KNAPSACKS__HIGHS__
