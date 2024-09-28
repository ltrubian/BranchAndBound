#include "knapsacks.hpp"
#include <algorithm>
#include <cstddef>
#include <numeric>
#include <tuple>
#include <utility>
#include <valarray>
#include <forward_list>

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

std::ostream &operator<<(std::ostream &os, const Bounds &b) {
  for (const auto &x : b.lower) {
    os << x << "\t";
  }
  os << "\n";
  for (const auto &x : b.upper) {
    os << x << "\t";
  }
  os << std::endl;
  return os;
};

const OptimalSolution Knapsack::branch_bound( Bounds &bounds) const {
  OptimalSolution opt_sol;
  float best_value{std::numeric_limits<float>::infinity()};

  std::forward_list<Bounds> active_problems;
  active_problems.emplace_front(bounds);

  for (; !active_problems.empty(); active_problems.pop_front()) {
    ++opt_sol.nodes;
    Bounds current_bounds{active_problems.front()};

    OptimalSolution current_sol{this->solve_relaxed(current_bounds)};

    if (!current_sol.success or
        best_value < this->objective(current_sol.solution)) {
      continue;
    }

    float fractional{0.0};
    float integral;
    auto index{0};
    for (auto &x : current_sol.solution) {
      integral = std::modf(x, &fractional);
      if (fractional != 0.0) {
        index = &x - &current_sol.solution[0];
        break;
      }
    }

    if (fractional == 0.0) {
      float new_value{this->objective(current_sol.solution)};
      if (new_value < best_value) {
        best_value = new_value;
        opt_sol.solution = current_sol.solution;
      }
      continue;
    }

    Bounds new_bounds{current_bounds};

    new_bounds.upper = integral;
    active_problems.emplace_front(new_bounds);

    current_bounds.lower = integral + 1.0;
    active_problems.emplace_front(current_bounds);
  }
  opt_sol.success = true;
  return opt_sol;
}

