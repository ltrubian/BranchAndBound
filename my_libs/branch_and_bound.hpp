#ifndef __BRANCH_BOUND__LT
#define __BRANCH_BOUND__LT

#include "utilities.hpp"
#include <cmath>

#include <chrono>
#include <memory>
#include <utility>
using namespace std::chrono_literals;
/**
 * Aim:
 */
template <typename T, typename Queue>
const void core_solve_choose(const T &problem, Bounds &bounds, Solution &opt,
                             std::unique_ptr<Node> &node, Queue &subproblems);

template <typename T, typename Queue>
std::pair<Solution, std::unique_ptr<Node>>
branch_bound(const T &problem, const Bounds &bounds,
             const Solution start_opt = Solution(),
             std::chrono::seconds max_time = std::chrono::seconds(300)) {
  auto stop = std::chrono::high_resolution_clock::now() + max_time;
  Solution opt_sol{start_opt};
  Bounds st_bounds{bounds};

  Queue active_problems;

  std::unique_ptr<Node> root{new Node()};
  core_solve_choose<T, Queue>(problem, st_bounds, opt_sol, root,
                              active_problems);

  while (!active_problems.empty() &&
         std::chrono::high_resolution_clock::now() < stop) {
    ExploreNode current_prob = active_problems.take_next();

    for (auto i{0}; i < 2; ++i) {
      Bounds current_bounds{current_prob.bounds};
      current_bounds[i][current_prob.node.b_index] =
          current_prob.node.b_value + 1. * i;
      ++opt_sol.nodes;
      // solve the relaxed problem
      core_solve_choose<T, Queue>(problem, current_bounds, opt_sol,
                                  current_prob.node.childs[i], active_problems);
    }

    if (!active_problems.empty())
      opt_sol.gap = active_problems.max_value() - opt_sol.value;
  }
  // in case the loop is stopped, active_problems could contains subproblems
  // to explore
  opt_sol.success = active_problems.empty() &&
                    opt_sol.value != -std::numeric_limits<double>::infinity();
  if (opt_sol.success)
    opt_sol.gap = 0.;

  return std::make_pair(std::move(opt_sol), std::move(root));
}

template <typename T, typename Queue>
const void core_solve_choose(const T &problem, Bounds &bounds, Solution &opt,
                             std::unique_ptr<Node> &node, Queue &subproblems) {
  Solution current_sol{problem.solve_relaxed(bounds)};
  node.reset(new Node(current_sol.value));
  if (!current_sol.success)
    return;


  if (opt.value + 1 > current_sol.value)
    return;


  // search for the first non integer value of the solution
  double integral{0.f}, fractional{0.f};
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
    subproblems.prune(opt.value);
  } else {
    node->b_index = index;
    node->b_value = integral;
    subproblems.emplace(opt.nodes, bounds, *node);
  }
}
#endif // __BRANCH_BOUND__LT
