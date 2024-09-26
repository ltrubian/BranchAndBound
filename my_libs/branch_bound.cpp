#include "branch_bound.hpp"
#include "knapsacks.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>
#include <vector>

std::ostream& operator<<(std::ostream& os, const Bounds& b) {
  for(const auto& x: b.lower){
    os << x << "\t";
  }
  os << "\n";
  for(const auto& x: b.upper){
    os << x << "\t";
  }
  os << std::endl;
  return os;
};


template <typename T> OptimalSolution branch_bound(const T &problem) {
  OptimalSolution opt_sol;
  float best_value{std::numeric_limits<float>::infinity()};

  std::list<Bounds> active_problems;
  active_problems.emplace_front(problem.bounds);

  for (; !active_problems.empty(); active_problems.pop_front()) {
    ++opt_sol.nodes;
    Bounds current_bounds{active_problems.front()};

    OptimalSolution current_sol{problem.solve(current_bounds)};

    if (!current_sol.success or
        best_value < problem.objective(current_sol.solution)) {
      continue;
    }

    float fractional{0.0};
    float integral;
    auto index{0};
    for (auto &x : current_sol.solution) {
      integral = std::modf(x, &fractional);
      if (fractional != 0.0) {
        index = &x - &current_sol.solution[0];
        break;
      }
    }

    if (fractional == 0.0) {
      float new_value{problem.objective(current_sol.solution)};
      if (new_value < best_value) {
        best_value = new_value;
        opt_sol.solution = current_sol.solution;
      }
      continue;
    }

    Bounds new_bounds{current_bounds};

    new_bounds(index, BoundType::upper) = integral;
    active_problems.emplace_front(new_bounds);

    current_bounds(index, BoundType::lower) = integral + 1.0;
    active_problems.emplace_front(current_bounds);
  }
  opt_sol.success = true;
  return opt_sol;
}

template OptimalSolution branch_bound(const Knapsack &problem);
