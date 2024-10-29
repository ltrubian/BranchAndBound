#ifndef __BRANCH_BOUND__LT
#define __BRANCH_BOUND__LT

#include "utilities.hpp"
#include <chrono>
#include <cmath>
#include <limits>
#include <memory>
#include <utility>

/***********************************************************
// __START_REQUIRED__: branch and bound algorithm
const Solution solve_relaxed(const Bounds &bounds) const;
const double objective(const Solution &solution) const;
const bool should_var_integer(const std::size_t index) const;
const Bounds& bounds() const;
// __END_REQUIRED__
***********************************************************/

/**
 * Given a subproblem, solve it, check if it is integer and therefore prune the
 * current queue of subproblems or not and add it to the same queue
 * input:
 *  - problem       : reference to the original problem
 *  - bounds        : bounds of the current subproblem
 *  - opt           : current optimal solution
 *  - node          : node of the tree of subproblems which correspond the
 *                    current one
 *  - subproblems   : queue of active subproblems
 */
template <typename T, typename Queue>
const void prune_or_branch(const T &problem, Bounds &bounds, Solution &opt,
                           Node &node, Queue &subproblems,
                           std::function<bool(double, double)> is_irrelevant);

/**
 * It returns the comparison operator that is suited for mixed-integer
 * programming (the first, in the case at least a variable it is not integral)
 * or integer programming (the last, in the case all variables are integral)
 */
template <typename T>
std::function<bool(double, double)> optimal_comparison(const T &problem) {
  for (std::size_t var{0}; var < problem.bounds().lower.size(); ++var) {
    if (!problem.should_var_integer(var)) {
      return [](double current, double optimum) { return current <= optimum; };
    }
  }
  return [](double current, double optimum) { return current < optimum + 1.; };
}
/**
 *
 */
template <typename T, typename Queue>
std::pair<Solution, std::unique_ptr<Node>>
branch_bound(const T &problem, const Solution start_opt = Solution(),
             std::chrono::seconds max_time = std::chrono::seconds(300)) {
  // set the timer
  auto stop = std::chrono::high_resolution_clock::now() + max_time;
  // set the initial values
  Solution opt_sol{start_opt};
  Bounds st_bounds{problem.bounds()};
  Queue active_subproblems;
  auto is_irrelevant = optimal_comparison(problem);

  // Initialize the root of the tree of all the subproblems that this
  // algorithm will consider
  std::unique_ptr<Node> root{new Node()};
  // solve the relaxed problem and branch
  prune_or_branch<T, Queue>(problem, st_bounds, opt_sol, *root,
                            active_subproblems, is_irrelevant);

  while (!active_subproblems.empty() &&
         std::chrono::high_resolution_clock::now() < stop) {

    // consider the next subproblem in the queue
    ExploreNode current_prob = active_subproblems.take_next();

    // copy the original bounds and set them according the considered subproblem
    Bounds current_bounds{problem.bounds()};
    current_prob.node.initialize_bounds(current_bounds);

    double tmp{current_bounds.upper[current_prob.node.b_index]};

    for (auto i{0}; i < 2; ++i) {
      // inizialize the leaf node and set its parent node
      current_prob.node.childs[i].reset(new Node(0., &current_prob.node));

      // for each leaf of the current subproblem adjust the bounds accordingly
      current_bounds[i][current_prob.node.b_index] =
          current_prob.node.b_value + 1. * i;
      // since the bounds are the same a correction must take place
      if (i == 1)
        current_bounds.upper[current_prob.node.b_index] = tmp;

      // solve the relaxed subproblem and
      // prune  (if integer solution occurs) or
      // branch (otherwise)
      prune_or_branch<T, Queue>(problem, current_bounds, opt_sol,
                                *current_prob.node.childs[i],
                                active_subproblems, is_irrelevant);
    }

    // update the current gap between the best optimal integer solution and the
    // worst non-integer solution (the highest one)
    if (!active_subproblems.empty())
      opt_sol.gap = active_subproblems.max_value() - opt_sol.value;
  }
  // in case the loop is stopped, active_subproblems could contains subproblems
  // to explore
  opt_sol.success = active_subproblems.empty() &&
                    opt_sol.value != -std::numeric_limits<double>::infinity();

  return std::make_pair(std::move(opt_sol), std::move(root));
}

template <typename T, typename Queue>
const void prune_or_branch(const T &problem, Bounds &bounds, Solution &opt,
                           Node &node, Queue &subproblems,
                           std::function<bool(double, double)> is_irrelevant) {
  ++opt.nodes;
  // solve subproblem with specific bounds and associate a new Node
  Solution current_sol{problem.solve_relaxed(bounds)};
  node.value = current_sol.value;

  // Unfeasible subproblem
  if (!current_sol.success)
    return;
  // Irrelevant subproblem: its best (eventual) integer solution is lower than
  // or equal to the current one
  if (is_irrelevant(current_sol.value, opt.value))
    return;

  // search for the first non integer value of the solution
  double integral{0.f}, fractional{0.f};
  auto index{0};
  bool found_branch_var{false};
  for (auto &x : current_sol.solution) {
    fractional = std::modf(x, &integral);
    index = &x - &current_sol.solution[0];
    found_branch_var = fractional != 0.f && problem.should_var_integer(index);
    if (found_branch_var) {
      break;
    }
  }
  // If the solution is integer (that is higher than the current one it has
  // already been checked) update the solution and prune the queue of
  // subproblems
  if (!found_branch_var) {
    opt.value = current_sol.value;
    opt.solution = current_sol.solution;
    subproblems.prune(opt.value, is_irrelevant);
  }
  // Update the node and add it to the queue of subproblems
  else {
    node.b_index = index;
    node.b_value = integral;
    subproblems.emplace(node);
  }
}
#endif // __BRANCH_BOUND__LT
