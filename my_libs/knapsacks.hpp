#ifndef __KNAPSACKS__
#define __KNAPSACKS__

#include <limits>
#include <ostream>
#include <valarray>

struct OptimalSolution {
  bool success;
  std::size_t nodes;
  float value;
  std::valarray<float> solution;

  OptimalSolution()
      : success{false}, nodes{0}, value{std::numeric_limits<float>::infinity()},
        solution{std::valarray<float>()} {};
};

enum BoundType {
  lower = 0,
  upper = 1,
};

struct Bounds {
  std::valarray<float> lower;
  std::valarray<float> upper;

  Bounds() : lower{std::valarray<float>()}, upper{std::valarray<float>()} {};
  Bounds(std::size_t n, float low, float up)
      : lower{std::valarray<float>(low, n)}, upper{std::valarray<float>(up,
                                                                        n)} {};
  explicit Bounds(std::size_t n)
      : Bounds(n, -std::numeric_limits<float>::infinity(),
               std::numeric_limits<float>::infinity()){};
};

std::ostream &operator<<(std::ostream &os, const Bounds &b);

/**
 * My class of the Knapsack problem
 */
struct Knapsack {
  std::valarray<float> prices;
  std::valarray<float> weights;
  float capacity;

  const OptimalSolution solve_relaxed(const Bounds &bounds) const;

  const float objective(const std::valarray<float> &solution) const;

  const OptimalSolution branch_bound(Bounds &bounds) const;
};

#endif // __KNAPSACKS__
