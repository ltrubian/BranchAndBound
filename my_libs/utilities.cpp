#include "utilities.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <queue>

//typedef double double;

bool OptimalSolution::is_integer() const {
  double integral{0.f}, fractional{0.f};
  for (auto &x : this->solution) {
    fractional = std::modf(x, &integral);
    if (fractional != 0.f) {
      return false;
    }
  }
  return true;
}

std::ostream &operator<<(std::ostream &os, OptimalSolution &item) {
  os << item.value << "\n";
  for (auto &i : item.solution) {
    os << i << "\n";
  }
  return os;
};

std::istream &operator>>(std::istream &is, OptimalSolution &item) {
  is >> item.value;
  while (is) {
    item.solution.emplace_back(0.);
    is >> *--item.solution.end();
  }
  return is;
};

double Node::best_upper_bound() const {
  double up{std::numeric_limits<double>::infinity()};
  std::queue<std::reference_wrapper<const Node>> to_explore;
  std::size_t level_stopper{1};
  bool next_level_relevant{true};
  to_explore.emplace(std::ref(*this));

  while (!to_explore.empty() && next_level_relevant) {
    std::size_t curr_level_nodes{0};
    double curr_up{-std::numeric_limits<double>::infinity()};

    for (std::size_t ind{0}; ind < level_stopper; ++ind) {
      auto node{to_explore.front()};
      to_explore.pop();
      if (node.get().info.test(nInteger) || !node.get().info.test(nRelevant))
        continue;
      if (node.get().info.test(nSuccess)) {
        curr_up = std::max(curr_up, node.get().value);
      }
      if (node.get().info.test(nExplored)) {
        for (auto i{0}; i < 2; ++i) {
          if (node.get().childs[i] != nullptr) {
            to_explore.emplace(std::ref(*node.get().childs[i]));
            ++curr_level_nodes;
          }
        }
      } else { // if the node is not be explored then the next level is not a
               // reliable source for setting an correct upperbound
        next_level_relevant = false;
      }
    }
    level_stopper = curr_level_nodes;
    up = std::min(up, curr_up);
  }

  return up;
}
