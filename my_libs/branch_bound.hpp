#ifndef __BRANCH_AND_BOUND__
#define __BRANCH_AND_BOUND__

#include <iostream>
#include <limits>
#include <memory>
#include <valarray>
#include <vector>

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
      : lower{std::valarray<float>(low, n)},
        upper{std::valarray<float>(up, n)} {};
  Bounds(std::size_t n)
      : Bounds(n, -std::numeric_limits<float>::infinity(),
               std::numeric_limits<float>::infinity()){};

  float &operator()(std::size_t idx, BoundType btype) {
    switch (btype) {
    case BoundType::lower:
      return lower[idx];
    case BoundType::upper:
      return upper[idx];
    }
  };

  const float &operator()(std::size_t idx, BoundType btype) const {
    switch (btype) {
    case BoundType::lower:
      return lower[idx];
    case BoundType::upper:
      return upper[idx];
    }
  };
};

std::ostream &operator<<(std::ostream &os, const Bounds &b);

template <typename T> OptimalSolution branch_bound(const T &problem);

#endif // __BRANCH_AND_BOUND
