#ifndef __BRANCH_BOUND__LT
#define __BRANCH_BOUND__LT

#include "utilities.hpp"
#include <cmath>

#include <chrono>
#include <limits>
#include <memory>
#include <utility>

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
                           Node *node, Queue &subproblems);
/**
 *
 */
template <typename T, typename Queue>
std::pair<Solution, std::unique_ptr<Node>>
branch_bound(const T &problem, const Bounds &bounds,
             const Solution start_opt = Solution(),
             std::chrono::seconds max_time = std::chrono::seconds(300)) {
  // set the timer
  auto stop = std::chrono::high_resolution_clock::now() + max_time;
  // set the initial values
  Solution opt_sol{start_opt};
  Bounds st_bounds{bounds};
  Queue active_subproblems;

  // Initialize the root of the tree of all the subproblems that this algorithm
  // will consider
  std::unique_ptr<Node> root{new Node(0.)};

  prune_or_branch<T, Queue>(problem, st_bounds, opt_sol, root.get(),
                            active_subproblems);

  while (!active_subproblems.empty() &&
         std::chrono::high_resolution_clock::now() < stop) {

    ExploreNode current_prob = active_subproblems.take_next();
    Bounds current_bounds{bounds};
    current_prob.node.initialize_bounds(current_bounds);
    double tmp{current_bounds.upper[current_prob.node.b_index]};
    for (auto i{0}; i < 2; ++i) {
      if (i == 1) {
        current_bounds.lower[current_prob.node.b_index] =
            current_prob.node.b_value + 1;
        current_bounds.upper[current_prob.node.b_index] = tmp;
      }
      if (i == 0)
        current_bounds.upper[current_prob.node.b_index] =
            current_prob.node.b_value;
      //Bounds current_bounds{bounds};
      current_prob.node.childs[i].reset(new Node(0., &current_prob.node));
      //current_prob.node.childs[i]->initialize_bounds(current_bounds);
      ++opt_sol.nodes;
      // solve the relaxed problem
      prune_or_branch<T, Queue>(problem, current_bounds, opt_sol,
                                current_prob.node.childs[i].get(),
                                active_subproblems);
    }

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
                           Node *node, Queue &subproblems) {

  // solve subproblem with specific bounds and associate a new Node
  Solution current_sol{problem.solve_relaxed(bounds)};
  node->value = current_sol.value;

  // Unfeasible subproblem
  if (!current_sol.success)
    return;
  // Irrelevant subproblem: its best (eventual) integer solution is lower than
  // or equal to the current one
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
  // If the solution is integer (that is higher than the current one it has
  // already been checked) update the solution and prune the queue of
  // subproblems
  if (fractional == 0.f) {
    opt.value = current_sol.value;
    opt.solution = current_sol.solution;
    subproblems.prune(opt.value);
  }
  // Update the node and add it to the queue of subproblems
  else {
    node->b_index = index;
    node->b_value = integral;
    subproblems.emplace(opt.nodes, *node);
  }
}
#endif // __BRANCH_BOUND__LT
