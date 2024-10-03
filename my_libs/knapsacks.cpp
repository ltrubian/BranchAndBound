#include "knapsacks.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <forward_list>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <tuple>
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

const OptimalSolution Knapsack::branch_bound(Bounds &bounds) const {
  OptimalSolution opt_sol;

  std::forward_list<Bounds> active_problems;
  active_problems.emplace_front(bounds);

  while (!active_problems.empty()) {
    ++opt_sol.nodes;
    Bounds current_bounds{active_problems.front()};
    active_problems.pop_front();

    // solve the relaxed problem
    OptimalSolution current_sol{this->solve_relaxed(current_bounds)};
    // first bound: the current upper bound is relevant only if greater than the
    // value of the best integer solution (that works as a lower bound of the
    // solution)
    if (!current_sol.success or opt_sol.value > current_sol.value) {
      continue;
    }
    // search for the first non integer value of the solution
    float integral{0.f}, fractional{0.f};
    auto index{0};
    for (auto &x : current_sol.solution) {
      fractional = std::modf(x, &integral);
      if (fractional != 0.f) {
        index = &x - &current_sol.solution[0];
        break;
      }
    }
    // second bound: if the solution is integer, that is the best solution for
    // the entire tree of its subproblem
    if (fractional == 0.f) {
      if (current_sol.value > opt_sol.value) {
        opt_sol.value = current_sol.value;
        opt_sol.solution = current_sol.solution;
      }
      continue;
    }
    // ... oherwise branch
    Bounds new_bounds{current_bounds};

    new_bounds.upper[index] = integral;
    active_problems.emplace_front(new_bounds);

    current_bounds.lower[index] = integral + 1.0;
    active_problems.emplace_front(current_bounds);
  }
  // in case the loop is stopped, active_problems could contains subproblems to
  // explore
  opt_sol.success = active_problems.empty() &&
                    opt_sol.value != -std::numeric_limits<float>::infinity();
  return opt_sol;
}
