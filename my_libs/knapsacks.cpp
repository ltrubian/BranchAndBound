#include "knapsacks.hpp"
#include "branch_bound.hpp"
#include <numeric>
#include <utility>
#include <valarray>
#include <algorithm>

const OptimalSolution Knapsack::solve(const Bounds &bounds) const {
  OptimalSolution opt_sol{};
  float correct_capacity{this->capacity -
                         std::inner_product(std::begin(bounds.lower),
                                            std::end(bounds.upper),
                                            std::begin(this->weights), 0.0f)};
  // if the items the bounds make me take are too much => infeasible bounds
  if (correct_capacity < 0)
    return opt_sol;
  std::valarray<float> real_prices{this->prices * bounds.upper *
                                   (1.0f - bounds.lower)};
  long int items_takable{
      std::count(std::begin(real_prices), std::end(real_prices), 0.0f)};
  // if there are no other items to take but the ones I must => the solution is
  // the item I must take
  if (items_takable == 0) {
    opt_sol.solution = bounds.lower;
    return opt_sol;
  }
  std::valarray<std::size_t> indexes{real_prices.size()};
  std::iota(std::begin(indexes), std::end(indexes), 0);
  std::sort(std::begin(indexes), std::end(indexes), [&](std::size_t x, std::size_t y) {return real_prices[x] > real_prices[y];});

  return opt_sol;
}

const float Knapsack::objective(const std::valarray<float> &solution) const {
  float res{0.0};
  auto sol{std::begin(solution)};
  for (auto price{std::begin(this->prices)}; price != std::begin(this->prices);
       ++price) {
    res = std::move(res) + *price * *sol;
    ++sol;
  }
  return -res;
}
