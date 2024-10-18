#include "knapsacks.hpp"
#include "branch_and_bound.hpp"
#include "utilities.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

Knapsack::Knapsack(std::size_t v, MFloat m, std::size_t n, std::size_t seed)
    : Knapsack{n} {
  // pairs generation
  std::mt19937_64 gen(seed);
  std::uniform_real_distribution<MFloat> dis_w(1., 1000.);
  std::vector<MFloat> small_w(v), small_p(v);
  std::generate(std::begin(small_w), std::end(small_w),
                [&]() { return dis_w(gen); });
  for (auto i{0ul}; i < v; ++i) {
    std::uniform_real_distribution<MFloat> dis_p(small_w[i] + 95.,
                                                small_w[i] + 105.);
    small_p[i] = dis_p(gen);
  }
  // pair normalization
  small_p /= (m + 1);
  small_w /= (m + 1);
  // items generation
  std::uniform_real_distribution<MFloat> multiplier(1., m);
  std::uniform_int_distribution<std::size_t> choice(0, v - 1);
  for (auto i{0ul}; i < n; ++i) {
    std::size_t pair{choice(gen)};
    MFloat mult{multiplier(gen)};
    this->prices[i] = std::ceil(small_p[pair] * mult);
    this->weights[i] = std::ceil(small_w[pair] * mult);
  }
  // set capacity
  MFloat tmp{std::accumulate(this->weights.begin(), this->weights.end(), 0.)};
  this->capacity = std::ceil(tmp / 3);
};

const OptimalSolution Knapsack::solve_relaxed(const Bounds &bounds) const {
  OptimalSolution opt_sol{};
  MFloat correct_capacity{this->capacity -
                         std::inner_product(std::begin(bounds.lower),
                                            std::end(bounds.lower),
                                            std::begin(this->weights), 0.0)};
  // if the items the bounds make me take are too much => infeasible bounds
  if (correct_capacity < 0)
    return opt_sol;
  std::vector<MFloat> real_prices{bounds.upper * (1.0 - bounds.lower)};
  long int items_takable{std::count_if(std::begin(real_prices),
                                       std::end(real_prices),
                                       [](MFloat p) { return 0. != p; })};
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
    opt_sol.solution[*ind] = 1.0;
  }
  opt_sol.success = true;
  opt_sol.value = this->objective(opt_sol.solution);

  return opt_sol;
}
const OptimalSolution
Knapsack::solve_integer_naive(const Bounds &bounds) const {
  OptimalSolution opt{this->solve_relaxed(bounds)};
  std::for_each(opt.solution.begin(), opt.solution.end(),
                [](MFloat &x) { x = std::floor(x); });
  opt.value = this->objective(opt.solution);
  return opt;
};

const OptimalSolution
Knapsack::solve_integer_guess(const Bounds &bounds) const {
  OptimalSolution opt_sol{};
  MFloat correct_capacity{this->capacity -
                         std::inner_product(std::begin(bounds.lower),
                                            std::end(bounds.lower),
                                            std::begin(this->weights), 0.0)};
  // if the items the bounds make me take are too much => infeasible bounds
  if (correct_capacity < 0)
    return opt_sol;
  std::vector<MFloat> real_prices{bounds.upper * (1.0 - bounds.lower)};
  long int items_takable{std::count_if(std::begin(real_prices),
                                       std::end(real_prices),
                                       [](MFloat p) { return 0. != p; })};
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
      continue;
      ;
    }
    correct_capacity -= this->weights[*ind];
    opt_sol.solution[*ind] = 1.0f;
  }
  opt_sol.success = true;
  opt_sol.value = this->objective(opt_sol.solution);

  return opt_sol;
}
const MFloat Knapsack::objective(const std::vector<MFloat> &solution) const {
  MFloat res{0.0};
  auto sol{std::begin(solution)};
  for (auto price{std::begin(this->prices)}; price != std::end(this->prices);
       ++price) {
    res = std::move(res) + *price * *sol;
    ++sol;
  }
  return res;
}

const bool Knapsack::is_feasible(const OptimalSolution& opt) const {
  return std::inner_product(weights.begin(), weights.end(), opt.solution.begin(), 0.) <= this->capacity;
};

std::ostream &operator<<(std::ostream &os, Knapsack &item) {
  os << item.prices.size() << " " << item.capacity << "\n";
  for (auto i{0}; i < item.prices.size(); ++i) {
    os << item.prices[i] << " " << item.weights[i] << "\n";
  }
  return os;
};

std::istream &operator>>(std::istream &is, Knapsack &item) {
  std::size_t n;
  is >> n >> item.capacity;
  item.prices.resize(n);
  item.weights.resize(n);
  for (auto i{0}; i < n; ++i) {
    is >> item.prices[i] >> item.weights[i];
  }
  return is;
};

template <>
std::pair<OptimalSolution, std::unique_ptr<Node>>
branch_bound<Knapsack, DepthFirst, PruneNone>(const Knapsack &, const Bounds &,
                                              OptimalSolution);

template <>
std::pair<OptimalSolution, std::unique_ptr<Node>>
branch_bound<Knapsack, DepthFirst, PruneUntill>(const Knapsack &,
                                                const Bounds &,
                                                OptimalSolution);
template <>
std::pair<OptimalSolution, std::unique_ptr<Node>>
branch_bound<Knapsack, DepthFirst, PruneAll>(const Knapsack &, const Bounds &,
                                             OptimalSolution);
template <>
std::pair<OptimalSolution, std::unique_ptr<Node>>
branch_bound<Knapsack, BestBoundFirst, PruneNone>(const Knapsack &,
                                                  const Bounds &,
                                                  OptimalSolution);

template <>
std::pair<OptimalSolution, std::unique_ptr<Node>>
branch_bound<Knapsack, BestBoundFirst, PruneUntill>(const Knapsack &,
                                                    const Bounds &,
                                                    OptimalSolution);
template <>
std::pair<OptimalSolution, std::unique_ptr<Node>>
branch_bound<Knapsack, BestBoundFirst, PruneAll>(const Knapsack &,
                                                 const Bounds &,
                                                 OptimalSolution);
