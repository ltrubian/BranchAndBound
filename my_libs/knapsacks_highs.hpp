#ifndef __KNAPSACKS__HIGHS__
#define __KNAPSACKS__HIGHS__

#include "branch_bound.hpp"
#include <valarray>

/**
 * My class of the Knapsack problem
 */
struct KnapsackHighs {
  std::valarray<float> prices;
  std::valarray<float> weights;
  float capacity;
  Bounds bounds;

  const OptimalSolution solve(const Bounds &bounds) const;

  const float objective(const std::valarray<float> &solution) const;
};

#endif // __KNAPSACKS__HIGHS__
