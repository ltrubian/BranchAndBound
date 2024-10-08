#ifndef __BRANCH_BOUND__LT
#define __BRANCH_BOUND__LT

#include "utilities.hpp"
#include <cmath>
#include <memory>
#include <set>
/**
 * Aim:
 */
template <typename T, typename Order, template <typename> typename Prune>
const void core_solve_choose(const T &problem, Bounds &bounds,
                             OptimalSolution &opt, std::unique_ptr<Node> &node,
                             std::set<ExploreNode, Order> &subproblems);

template <typename T, typename Order, template <typename> typename Prune>
const OptimalSolution
branch_bound(const T &problem, Bounds &bounds,
             const OptimalSolution opt = OptimalSolution()) {

  OptimalSolution opt_sol{opt};

  std::set<ExploreNode, Order> active_problems;

  std::unique_ptr<Node> root{new Node()};
  core_solve_choose<T, Order, Prune>(problem, bounds, opt_sol, root,
                                     active_problems);

  while (!active_problems.empty()) {
    ExploreNode current_prob =
        std::move(active_problems.extract(--active_problems.end()).value());

    for (auto i{0}; i < 2; ++i) {
      Bounds current_bounds{current_prob.bounds};
      current_bounds[i][current_prob.node.b_index] =
          current_prob.node.b_value + 1.f * i;
      ++opt_sol.nodes;
      // solve the relaxed problem
      core_solve_choose<T, Order, Prune>(problem, current_bounds, opt_sol,
                                         current_prob.node.childs[i],
                                         active_problems);
    }
    current_prob.node.explored = true;
  }
  // in case the loop is stopped, active_problems could contains subproblems
  // to explore
  opt_sol.success = active_problems.empty() &&
                    opt_sol.value != -std::numeric_limits<float>::infinity();
  return opt_sol;
}

template <typename T, typename Order, template <typename> typename Prune>
const void core_solve_choose(const T &problem, Bounds &bounds,
                             OptimalSolution &opt, std::unique_ptr<Node> &node,
                             std::set<ExploreNode, Order> &subproblems) {
  OptimalSolution current_sol{problem.solve_relaxed(bounds)};
  node.reset(new Node(current_sol.value));
  if (current_sol.success && opt.value < current_sol.value) {
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

    if (fractional == 0.f) {
      opt.value = current_sol.value;
      opt.solution = current_sol.solution;
      node->integrality = true;
      Prune<std::set<ExploreNode, Order>>()(subproblems, opt.value);
    } else {
      node->b_index = index;
      node->b_value = integral;
      subproblems.emplace(ExploreNode(opt.nodes, bounds, *node));
    }
  };
}
#endif // __BRANCH_BOUND__LT
