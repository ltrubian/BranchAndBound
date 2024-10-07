
#include "branch_and_bound.hpp"
#include <cmath>
#include <set>

template <typename T, typename B, typename Prune>
const OptimalSolution branch_bound(const T &problem, Bounds &bounds,
                                   OptimalSolution opt = OptimalSolution()) {
  OptimalSolution opt_sol;

  std::set<ExploreNode, B> active_problems;

  Node root;

  OptimalSolution current_sol{problem.solve_relaxed(bounds)};

  if (current_sol.success && opt_sol.value < current_sol.value) {
    // search for the first non integer value of the solution
    float integral{0.f}, fractional{0.f};
    auto index{0};
    for (auto &x : current_sol.solution) {
      fractional = std::modf(x, &integral);
      if (fractional != 0.f) {
        index = &x - &current_sol.solution[0];
        break;
      }
    }
    root.value = current_sol.value;
    // second bound: if the solution is integer, that is the best solution for
    // the entire tree of its subproblem
    if (fractional == 0.f) {
      opt_sol.value = current_sol.value;
      opt_sol.solution = current_sol.solution;
    } else {
      root.b_index = index;
      root.b_value = integral;
      root.value = current_sol.value;
      active_problems.emplace(ExploreNode(0, bounds, root));
    }
  }
  while (!active_problems.empty()) {
    ExploreNode current_prob =
        std::move(active_problems.extract(active_problems.cbegin()).value());

    for (auto i{0}; i < 2; ++i) {
      Bounds current_bounds{current_prob.bounds};
      current_bounds[i][current_prob.node.b_index] =
          current_prob.node.b_value + 1.f * i;

      // solve the relaxed problem
      OptimalSolution current_sol{problem.solve_relaxed(current_bounds)};
      ++opt_sol.nodes;
      if (current_sol.success && opt_sol.value < current_sol.value) {
        // search for the first non integer value of the solution
        float integral{0.f}, fractional{0.f};
        auto index{0};
        for (auto &x : current_sol.solution) {
          fractional = std::modf(x, &integral);
          if (fractional != 0.f) {
            index = &x - &current_sol.solution[0];
            break;
          }
        }
        current_prob.node.childs[i].reset(new Node(current_sol.value));
        if (fractional == 0.f) {
          opt_sol.value = current_sol.value;
          opt_sol.solution = current_sol.solution;
          current_prob.node.childs[i]->integrality = true;
          Prune(active_problems, opt_sol.value);
        } else {
          current_prob.node.childs[i]->b_index = index;
          current_prob.node.childs[i]->b_value = integral;
          active_problems.emplace(ExploreNode(opt_sol.nodes, current_bounds,
                                              *current_prob.node.childs[i]));
        }
      }
    }
    current_prob.node.explored = true;
  }
  // in case the loop is stopped, active_problems could contains subproblems
  // to explore
  opt_sol.success = active_problems.empty() &&
                    opt_sol.value != -std::numeric_limits<float>::infinity();
  return opt_sol;
}
