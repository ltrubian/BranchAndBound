#include "branch_and_bound.hpp"
#include "utilities.hpp"

/**
 * Naive interface for a model class for the branch and bound algorithm. Even
 * though it is possible to derive this class, it is highly recomended to write
 * a new one (with the methods that are needed) and specialize the template
 */
class ModelBase {
  /**
   * Given bounds stricter than the original ones, it must return a optimal
   * solution of the (non-integer) relaxation of the restricted subproblem
   */
  virtual const Solution solve_relaxed(const Bounds &bounds) const;

  /**
   * For each index of the variable, it must return true if the problem requires
   * integrality for that variable
   */
  virtual const bool should_var_integer(const std::size_t index) const;

  /**
   * It must return the minimum increase of the objective function on better
   * optimum                                                                 //
   * For pure-integer programming it is at leat 1                            //
   * For pure-integer
   * linear programming it is the GCD of the coefficient of the objective.
   *
   * In case the objective function increase continuosly then std::nullopt must
   * be return
   *
   * (pure-integer: all variable are required to be integer and all the
   * coefficients of the objective are integer)
   */
  virtual const std::optional<double> min_step_objective() const;

  /**
   * It must return the bounds on the variables of the problem
   */
  virtual const Bounds &bounds() const;
};

/**
 * Naive interface for a queue used in the branch and bound algorithm. Even
 * though it is possible to derive this class, it is highly recomended to write
 * a new one (with the methods that are needed) and specialize the template
 */
struct QueueBase {

  /**
   * This function is called to remove from the queue all the nodes that are no
   * more relevant when a better incumbent is found
   */
  virtual void prune(const double &value,
                     std::function<bool(double, double)> is_irrelevant);

  /**
   * It must return the maximum value of the explored nodes not yet explored
   * It is usefull to give a optimality gap on the solution
   */
  virtual double max_value() const;

  /**
   * It must return true if no other subproblems have to be solved
   */
  virtual bool empty() const;

  /**
   * It must return the reference to the next node to work with
   */
  virtual Node &take_next();

  /**
   * It adds a new node to explore to the queue
   */
  virtual void emplace(Node &node);
};

template <>
std::pair<Solution, std::unique_ptr<Node>>
branch_bound<ModelBase, QueueBase>(const ModelBase &, Solution,
                                   std::chrono::seconds);
