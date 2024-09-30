#include "knapsacks.hpp"
#include <forward_list>
#include <iostream>
#include <limits>
#include <random>

Bounds random_bounds(std::size_t n, std::size_t seed) {
  Bounds bounds(n, 0.f, 1.f);
  std::mt19937_64 gen(seed);
  std::uniform_int_distribution<std::size_t> dis(0, 2);
  for (auto i{0ul}; i < n; ++i) {
    switch (dis(gen)) {
    case 0:
      bounds.lower[i] = 1.f;
      break;
    case 1:
      bounds.upper[i] = 0.f;
      break;
    case 2:
      break;
    }
  }

  return bounds;
};
int main() {
  std::random_device rd;
  std::uniform_int_distribution<std::size_t> di(0);
  std::size_t n{20};
  Bounds bounds(random_bounds(n,di(rd)));
  Knapsack prob{5, 20.f, 20, di(rd)};
  prob.solve_relaxed(bounds);


  return 0;
}
