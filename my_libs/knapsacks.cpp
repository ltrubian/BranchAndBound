#include "knapsacks.hpp"
#include "branch_bound.hpp"
#include <utility>
#include <vector>

const OptimalSolution Knapsack::solve(Bounds &bounds) const {
  OptimalSolution opt_sol;

  return opt_sol;
}

const float Knapsack::objective(const std::vector<float> &solution) const {
  float res{0.0};
  auto sol{solution.begin()};
  for (auto price{this->prices.begin()}; price != this->prices.end(); ++price) {
    res = std::move(res) + *price * *sol;
    ++sol;
  }
  return -res;
}
