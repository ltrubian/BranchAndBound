#include "knapsacks.hpp"
#include "branch_and_bound.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

Knapsack::Knapsack(std::size_t v, float m, std::size_t n, std::size_t seed)
    : Knapsack{n} {
  // pairs generation
  std::mt19937_64 gen(seed);
  std::uniform_real_distribution<float> dis_w(1.0f, 1000.0f);
  std::vector<float> small_w(v), small_p(v);
  std::generate(std::begin(small_w), std::end(small_w),
                [&]() { return dis_w(gen); });
  for (auto i{0ul}; i < v; ++i) {
    std::uniform_real_distribution<float> dis_p(small_w[i] + 95.0f,
                                                small_w[i] + 105.0f);
    small_p[i] = dis_p(gen);
  }
  // pair normalization
  small_p /= (m + 1);
  small_w /= (m + 1);
  // items generation
  std::uniform_real_distribution<float> multiplier(1.f, m);
  std::uniform_int_distribution<std::size_t> choice(0, v - 1);
  for (auto i{0ul}; i < n; ++i) {
    std::size_t pair{choice(gen)};
    float mult{multiplier(gen)};
    this->prices[i] = std::ceil(small_p[pair] * mult);
    this->weights[i] = std::ceil(small_w[pair] * mult);
  }
  // set capacity
  float tmp{std::accumulate(this->weights.begin(), this->weights.end(), 0.f)};
  this->capacity = std::ceil(tmp / 3);
};

const OptimalSolution Knapsack::solve_relaxed(const Bounds &bounds) const {
  OptimalSolution opt_sol{};
  float correct_capacity{this->capacity -
                         std::inner_product(std::begin(bounds.lower),
                                            std::end(bounds.lower),
                                            std::begin(this->weights), 0.0f)};
  // if the items the bounds make me take are too much => infeasible bounds
  if (correct_capacity < 0)
    return opt_sol;
  std::vector<float> real_prices{bounds.upper * (1.0f - bounds.lower)};
  long int items_takable{std::count_if(std::begin(real_prices),
                                       std::end(real_prices),
                                       [](float p) { return 0.f != p; })};
  // if there are no other items to take but the ones I must => the solution is
  // the item I must take
  opt_sol.solution =
      bounds.lower; // and the lower bounds are the "starting" optimal solution
  if (items_takable == 0) {
    opt_sol.success = true;
    opt_sol.value = this->objective(opt_sol.solution);
    return opt_sol;
  }
  real_prices *= (this->prices / this->weights);
  std::vector<std::size_t> indexes(real_prices.size());
  std::iota(std::begin(indexes), std::end(indexes), 0ul);
  std::sort(indexes.begin(), indexes.end(),
            [&](std::size_t &x, std::size_t &y) {
              return real_prices[x] > real_prices[y];
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
  opt_sol.value = this->objective(opt_sol.solution);

  return opt_sol;
}

const float Knapsack::objective(const std::vector<float> &solution) const {
  float res{0.0};
  auto sol{std::begin(solution)};
  for (auto price{std::begin(this->prices)}; price != std::end(this->prices);
       ++price) {
    res = std::move(res) + *price * *sol;
    ++sol;
  }
  return res;
}

template <>
const OptimalSolution branch_bound<Knapsack, DepthFirst, PruneAll>(
    const Knapsack &problem, Bounds &bounds, OptimalSolution opt);
