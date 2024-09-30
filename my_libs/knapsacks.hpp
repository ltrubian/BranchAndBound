#ifndef __KNAPSACKS__
#define __KNAPSACKS__

#include <limits>
#include <vector>

struct OptimalSolution {
  bool success;
  std::size_t nodes;
  float value;
  std::vector<float> solution;

  OptimalSolution()
      : success{false}, nodes{0}, value{std::numeric_limits<float>::infinity()},
        solution{std::vector<float>()} {};
};

struct Bounds {
  std::vector<float> lower;
  std::vector<float> upper;

  Bounds() : lower{std::vector<float>()}, upper{std::vector<float>()} {};
  Bounds(std::size_t n, float low, float up)
      : lower{std::vector<float>(n, low)}, upper{std::vector<float>(n, up)} {};
  explicit Bounds(std::size_t n)
      : Bounds(n, -std::numeric_limits<float>::infinity(),
               std::numeric_limits<float>::infinity()){};
};

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

  const float objective(const std::vector<float> &solution) const;

  const OptimalSolution branch_bound(Bounds &bounds) const;
};

#endif // __KNAPSACKS__
