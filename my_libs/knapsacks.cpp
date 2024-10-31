#include "knapsacks.hpp"
#include "branch_and_bound.hpp"
#include "utilities.hpp"
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

Knapsack::Knapsack(std::size_t v, double m, std::size_t n, std::size_t seed)
    : Knapsack{n} {
  this->_bounds = Bounds(n, 0., 1.);
  this->integrality = std::vector<bool>(n, true);
  // pairs generation
  std::mt19937_64 gen(seed);
  std::uniform_real_distribution<double> dis_w(1., 1000.);
  std::vector<double> small_w(v), small_p(v);
  std::generate(std::begin(small_w), std::end(small_w),
                [&]() { return dis_w(gen); });
  for (auto i{0ul}; i < v; ++i) {
    std::uniform_real_distribution<double> dis_p(small_w[i] + 95.,
                                                 small_w[i] + 105.);
    small_p[i] = dis_p(gen);
  }
  // pair normalization
  small_p /= (m + 1);
  small_w /= (m + 1);
  // items generation
  std::uniform_real_distribution<double> multiplier(1., m);
  std::uniform_int_distribution<std::size_t> choice(0, v - 1);
  for (auto i{0ul}; i < n; ++i) {
    std::size_t pair{choice(gen)};
    double mult{multiplier(gen)};
    this->prices[i] = std::ceil(small_p[pair] * mult);
    this->weights[i] = std::ceil(small_w[pair] * mult);
  }
  // set capacity
  double tmp{std::accumulate(this->weights.begin(), this->weights.end(), 0.)};
  this->capacity = std::ceil(tmp / 3);
};

const Solution Knapsack::solve_relaxed(const Bounds &bounds) const {
  Solution opt_sol{};
  double correct_capacity{this->capacity -
                          std::inner_product(std::begin(bounds.lower),
                                             std::end(bounds.lower),
                                             std::begin(this->weights), 0.0)};
  // if the items the bounds make me take are too much => infeasible bounds
  if (correct_capacity < 0)
    return opt_sol;
  std::vector<double> real_prices{bounds.upper * (1.0 - bounds.lower)};
  auto items_takable{std::count_if(std::begin(real_prices),
                                   std::end(real_prices),
                                   [](double p) { return 0. != p; })};
  // if there are no other items to take but the ones I must => the solution is
  // the item I must take
  opt_sol.solution =
      bounds.lower; // and the lower bounds are the "starting" optimal solution
  if (items_takable == 0) {
    opt_sol.success = true;
    opt_sol.value = this->objective(opt_sol);
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
  opt_sol.value = this->objective(opt_sol);

  return opt_sol;
}
const Solution Knapsack::solve_integer_naive() const {
  Solution opt{this->solve_relaxed(this->bounds())};
  std::for_each(opt.solution.begin(), opt.solution.end(),
                [](double &x) { x = std::floor(x); });
  opt.value = this->objective(opt);
  return opt;
};

const Solution Knapsack::solve_integer_guess() const {
  Bounds bounds{this->bounds()};
  Solution opt_sol{};
  double correct_capacity{this->capacity -
                          std::inner_product(std::begin(bounds.lower),
                                             std::end(bounds.lower),
                                             std::begin(this->weights), 0.0)};
  // if the items the bounds make me take are too much => infeasible bounds
  if (correct_capacity < 0)
    return opt_sol;
  std::vector<double> real_prices{bounds.upper * (1.0 - bounds.lower)};
  auto items_takable{std::count_if(std::begin(real_prices),
                                   std::end(real_prices),
                                   [](double p) { return 0. != p; })};
  // if there are no other items to take but the ones I must => the solution is
  // the item I must take
  opt_sol.solution =
      bounds.lower; // and the lower bounds are the "starting" optimal solution
  if (items_takable == 0) {
    opt_sol.success = true;
    opt_sol.value = this->objective(opt_sol);
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
  opt_sol.value = this->objective(opt_sol);

  return opt_sol;
}
const double Knapsack::objective(const Solution &opt) const {
  return std::inner_product(opt.solution.begin(), opt.solution.end(),
                            prices.begin(), 0.);
}

const bool Knapsack::is_feasible(const Solution &opt) const {
  return std::inner_product(weights.begin(), weights.end(),
                            opt.solution.begin(), 0.) <= this->capacity;
};

const bool Knapsack::is_integral(const Solution &opt) const {
  double fractional{0.}, integral{0.};
  for (auto &x : opt.solution) {
    fractional = std::modf(x, &integral);
    auto index = &x - &opt.solution[0];
    if (fractional != 0. && this->should_var_integer(index)) {
      return false;
    }
  }
  return true;
};

std::ostream &operator<<(std::ostream &os, Knapsack &item) {
  os << item.prices.size() << " " << item.capacity;
  for (auto i{0}; i < item.prices.size(); ++i) {
    os << "\n" << item.prices[i] << " " << item.weights[i];
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
std::pair<Solution, std::unique_ptr<Node>>
branch_bound<Knapsack, QueueDepth>(const Knapsack &, Solution,
                                   std::chrono::seconds);

template <>
std::pair<Solution, std::unique_ptr<Node>>
branch_bound<Knapsack, QueueBestBound>(const Knapsack &, Solution,
                                       std::chrono::seconds);
