#ifndef __KNAPSACKS__
#define __KNAPSACKS__

#include "utilities.hpp"
#include <iostream>
#include <istream>
#include <vector>
#include <optional>

/**
 * My class of the Knapsack problem
 */
struct Knapsack {
  double capacity;
  std::vector<double> prices;
  std::vector<double> weights;
  std::vector<bool> integrality;
  Bounds _bounds;

  explicit Knapsack(std::size_t n)
      : prices(n), weights(n), capacity{0.f}, integrality(n) {};

  /**
   * Random generator of Knapsack. It follows the step of the file
   * "project.pdf", section "Computational evaluation"
   */
  explicit Knapsack(std::size_t v, double m, std::size_t n, std::size_t seed);

  const Solution solve_integer_naive() const;
  const Solution solve_integer_guess() const;
  const bool is_feasible(const Solution &) const;
  const bool is_integral(const Solution &) const;
  const double objective(const Solution &solution) const;

  // START_REQUIRED: branch and bound algorithm
  const Solution solve_relaxed(const Bounds &bounds) const;

  inline const bool should_var_integer(const std::size_t index) const {
    return integrality[index];
  };
  const std::optional<double> min_step_objective() const;
  const Bounds &bounds() const { return this->_bounds; };
  // END_REQUIRED
  Bounds &bounds() { return this->_bounds; };
};

std::ostream &operator<<(std::ostream &os, Knapsack &item);

std::istream &operator>>(std::istream &is, Knapsack &item);
// The definitions of the operations between vector (that are element-wise
// operations) follow the syntax of numpy
template <typename T> std::vector<T> operator-(T b, const std::vector<T> &a) {
  std::vector<T> res(a.size());
  for (std::size_t i{0}; i < a.size(); ++i) {
    res[i] = b - a[i];
  }
  return res;
};

template <typename T>
std::vector<T> operator*(const std::vector<T> &a, const std::vector<T> &b) {
  std::vector<T> res(a.size());
  for (std::size_t i{0}; i < a.size(); ++i) {
    res[i] = a[i] * b[i];
  }
  return res;
};

template <typename T>
std::vector<T> operator*=(std::vector<T> &a, const std::vector<T> &b) {
  for (std::size_t i{0}; i < a.size(); ++i) {
    a[i] *= b[i];
  }
  return a;
};
template <typename T>
std::vector<T> operator*=(std::vector<T> &&a, const std::vector<T> &b) {
  return std::forward<std::vector<T>>(a *= b);
};

template <typename T>
std::vector<T> operator/(const std::vector<T> &a, const std::vector<T> &b) {
  std::vector<T> res(a.size());
  for (std::size_t i{0}; i < a.size(); ++i) {
    res[i] = a[i] / b[i];
  }
  return res;
};

template <typename T> std::vector<T> operator/=(std::vector<T> &a, const T &b) {
  for (std::size_t i{0}; i < a.size(); ++i) {
    a[i] /= b;
  }
  return a;
};
#endif // __KNAPSACKS__
