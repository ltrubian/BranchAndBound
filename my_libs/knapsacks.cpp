#include "knapsacks.hpp"
#include "branch_bound.hpp"
#include <algorithm>
#include <cstddef>
#include <numeric>
#include <tuple>
#include <utility>
#include <valarray>

const OptimalSolution Knapsack::solve_relaxed(const Bounds &bounds) const {
  OptimalSolution opt_sol{};
  float correct_capacity{this->capacity -
                         std::inner_product(std::begin(bounds.lower),
                                            std::end(bounds.upper),
                                            std::begin(this->weights), 0.0f)};
  // if the items the bounds make me take are too much => infeasible bounds
  if (correct_capacity < 0)
    return opt_sol;
  std::valarray<float> real_prices{this->prices / this->weights * bounds.upper *
                                   (1.0f - bounds.lower)};
  long int items_takable{
      std::count(std::begin(real_prices), std::end(real_prices), 0.0f)};
  // if there are no other items to take but the ones I must => the solution is
  // the item I must take
  opt_sol.solution =
      bounds.lower; // and the lower bounds are the "starting" optimal solution
  if (items_takable == 0) {
    return opt_sol;
  }
  std::valarray<std::size_t> indexes(real_prices.size());
  std::iota(std::begin(indexes), std::end(indexes), 0);
  std::sort(std::begin(indexes), std::end(indexes),
            [&](std::size_t x, std::size_t y) {
              return real_prices[x] >= real_prices[y];
            });
  for (auto ind{std::begin(indexes)};
       ind != std::begin(indexes) + items_takable; ++ind) {
    if (correct_capacity < this->weights[*ind]) {
      opt_sol.solution[*ind] = correct_capacity / this->weights[*ind];
      break;
    }
    correct_capacity -= this->weights[*ind];
    opt_sol.solution[*ind] = 1.0f;
  }
  opt_sol.success = true;

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

/*
  std::valarray<std::pair<float, std::size_t>> indexes{real_prices.size()};
  std::generate(std::begin(indexes), std::end(indexes), [&]() {
    static int ind{-1};
    ++ind;
    return std::pair<float, std::size_t>(real_prices[ind], ind);
  });
  std::sort(
      std::begin(indexes), std::end(indexes),
      [&](std::tuple<float, std::size_t> x, std::tuple<float, std::size_t> y) {
        return std::get<0>(x) >= std::get<0>(y);
      });
*/
