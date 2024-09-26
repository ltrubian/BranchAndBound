#ifndef __KNAPSACKS__
#define __KNAPSACKS__

#include "branch_bound.hpp"
#include <valarray>

/**
 * My class of the Knapsack problem
 */
struct Knapsack {
  std::valarray<float> prices;
  std::valarray<float> weights;
  float capacity;
  Bounds bounds;

  const OptimalSolution solve(const Bounds &bounds) const;

  const float objective(const std::valarray<float> &solution) const;
};

#endif // __KNAPSACKS__
