#ifndef __BRANCH_AND_BOUND__
#define __BRANCH_AND_BOUND__

#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include <valarray>

struct OptimalSolution {
  bool success;
  std::size_t nodes;
  float value;
  std::vector<float> solution;

  OptimalSolution()
      : success{false}, nodes{0}, value{std::numeric_limits<float>::infinity()},
        solution{std::vector<float>()} {};
};

enum BoundType {
  lower = 0,
  upper = 1,
};

struct Bounds {
  std::valarray<float> low_bounds;
  std::valarray<float> up_bounds;

  Bounds() : low_bounds{std::valarray<float>()}, up_bounds{std::valarray<float>()} {};
  Bounds(std::size_t n, float low, float up)
      : low_bounds{std::valarray<float>(low, n)}, up_bounds{std::valarray<float>(up, n)} {
  };
  Bounds(std::size_t n)
  : Bounds(n, -std::numeric_limits<float>::infinity(),
           std::numeric_limits<float>::infinity()){};

  float &operator()(std::size_t idx, BoundType btype) {
    switch (btype) {
      case BoundType::lower:
        return low_bounds[idx];
      case BoundType::upper:
        return up_bounds[idx];
    }
  };

  const float &operator()(std::size_t idx, BoundType btype) const {
    switch (btype) {
      case BoundType::lower:
        return low_bounds[idx];
      case BoundType::upper:
        return up_bounds[idx];
    }
  };
};

std::ostream &operator<<(std::ostream &os, const Bounds &b);

template <typename T> OptimalSolution branch_bound(const T &problem);

#endif // __BRANCH_AND_BOUND
