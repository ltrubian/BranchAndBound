#ifndef __KNAPSACKS__
#define __KNAPSACKS__

#include "utilities.hpp"
#include <iostream>
#include <istream>
#include <vector>

/**
 * My class of the Knapsack problem
 */
struct Knapsack {
  std::vector<float> prices;
  std::vector<float> weights;
  float capacity;
  explicit Knapsack(std::size_t n)
      : prices{std::vector<float>(n)}, weights{std::vector<float>(n)},
        capacity{0.f} {};

  /**
   * Random generator of Knapsack. It follows the step of the file
   * "project.pdf", section "Computational evaluation"
   */
  explicit Knapsack(std::size_t v, float m, std::size_t n, std::size_t seed);

  const OptimalSolution solve_relaxed(const Bounds &bounds) const;
  const OptimalSolution solve_integer_naive(const Bounds &bounds) const;
  const OptimalSolution solve_integer_guess(const Bounds &bounds) const;

  const float objective(const std::vector<float> &solution) const;

  const bool is_feasible(const OptimalSolution&) const;
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
