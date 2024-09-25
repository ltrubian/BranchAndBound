#ifndef __KNAPSACKS__
#define __KNAPSACKS__

#include "branch_bound.hpp"
#include <vector>

struct Knapsack {
  std::vector<int> prices;
  std::vector<int> weights;
  float capacity;
  Bounds bounds;

  const OptimalSolution solve(Bounds &bounds) const;

  const float objective(const std::vector<float> &solution) const;
};

#endif // __KNAPSACKS__
