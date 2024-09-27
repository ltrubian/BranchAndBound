#include "knapsacks_highs.hpp"
#include "Highs.h"
#include "branch_bound.hpp"
#include <algorithm>
#include <numeric>
#include <utility>
#include <valarray>

const OptimalSolution KnapsackHighs::solve(const Bounds &bounds) const {
  OptimalSolution opt_sol{};

  opt_sol.success = true;
  return opt_sol;
}

const float
KnapsackHighs::objective(const std::valarray<float> &solution) const {
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
