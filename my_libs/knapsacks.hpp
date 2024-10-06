#ifndef __KNAPSACKS__
#define __KNAPSACKS__

#include <limits>
#include <set>
#include <stdexcept>
#include <vector>

struct OptimalSolution {
  bool success;
  std::size_t nodes;
  float value;
  std::vector<float> solution;

  OptimalSolution()
      : success{false}, nodes{0},
        value{-std::numeric_limits<float>::infinity()},
        solution{std::vector<float>()} {};
};

struct Bounds {
  std::vector<float> lower;
  std::vector<float> upper;

  Bounds() : lower{std::vector<float>()}, upper{std::vector<float>()} {};
  Bounds(std::size_t n, float low, float up)
      : lower{std::vector<float>(n, low)}, upper{std::vector<float>(n, up)} {};
  explicit Bounds(std::size_t n)
      : Bounds(n, -std::numeric_limits<float>::infinity(),
               std::numeric_limits<float>::infinity()){};
  std::vector<float> &operator[](std::size_t i) {
    switch (i) {
    case 0:
      return upper;
    case 1:
      return lower;
    default:
      throw std::out_of_range("only 0: upper, 1:lower bounds available");
    }
  };
  const std::vector<float> &operator[](std::size_t i) const {
    switch (i) {
    case 0:
      return upper;
    case 1:
      return lower;
    default:
      throw std::out_of_range("only 0: upper, 1:lower bounds available");
    }
  };
};

struct Node {
  float value;
  std::size_t b_index;
  float b_value; // for the knapsack it is always 0, but it is not so in general
  bool integrality;
  bool explored;
  Node *childs[2];

  Node()
      : value(-std::numeric_limits<float>::infinity()), b_index{0},
        b_value{0.f}, integrality{false}, explored{false},
        childs{nullptr, nullptr} {};
  Node(float value) : Node() { this->value = value; };
  ~Node(){};
};

struct ExploreNode {
  std::size_t node_id;
  float value;
  Bounds bounds;
  Node &node;

  ExploreNode(std::size_t id, Bounds &bounds, Node &node)
      : node_id(id), value(node.value), bounds(bounds), node(node){};
};
struct DepthFirst {
  constexpr bool operator()(const ExploreNode &a, const ExploreNode &b) const {
    return a.node_id > b.node_id;
  };
};
struct BestBoundFirst {
  constexpr bool operator()(const ExploreNode &a, const ExploreNode &b) const {
    return (a.value > b.value) || (a.value == b.value && a.node_id > b.node_id);
  }
};
void prune_unitll(std::set<ExploreNode> &queue, float value);
void prune_all(std::set<ExploreNode> &queue, float value);

/**
 * My class of the Knapsack problem
 */
struct Knapsack {
  std::vector<float> prices;
  std::vector<float> weights;
  float capacity;
  explicit Knapsack(std::size_t n)
      : prices{std::vector<float>(n)}, weights{std::vector<float>(n)},
        capacity{0.f} {};

  /**
   * Random generator of Knapsack. It follows the step of the file
   * "project.pdf", section "Computational evaluation"
   */
  explicit Knapsack(std::size_t v, float m, std::size_t n, std::size_t seed);

  const OptimalSolution solve_relaxed(const Bounds &bounds) const;

  const float objective(const std::vector<float> &solution) const;

  const OptimalSolution
  branch_bound(Bounds &bounds, OptimalSolution opt = OptimalSolution()) const;
};

// The definitions of the operations between vector (that are element-wise
// operations) follow the syntax of numpy
template <typename T> std::vector<T> operator-(T b, const std::vector<T> &a) {
  std::vector<T> res(a.size());
  for (std::size_t i{0}; i < a.size(); ++i) {
    res[i] = b - a[i];
  }
  return res;
};

template <typename T>
std::vector<T> operator*(const std::vector<T> &a, const std::vector<T> &b) {
  std::vector<T> res(a.size());
  for (std::size_t i{0}; i < a.size(); ++i) {
    res[i] = a[i] * b[i];
  }
  return res;
};

template <typename T>
std::vector<T> operator*=(std::vector<T> &a, const std::vector<T> &b) {
  for (std::size_t i{0}; i < a.size(); ++i) {
    a[i] *= b[i];
  }
  return a;
};
template <typename T>
std::vector<T> operator*=(std::vector<T> &&a, const std::vector<T> &b) {
  return std::forward<std::vector<T>>(a *= b);
};

template <typename T>
std::vector<T> operator/(const std::vector<T> &a, const std::vector<T> &b) {
  std::vector<T> res(a.size());
  for (std::size_t i{0}; i < a.size(); ++i) {
    res[i] = a[i] / b[i];
  }
  return res;
};

template <typename T> std::vector<T> operator/=(std::vector<T> &a, const T &b) {
  for (std::size_t i{0}; i < a.size(); ++i) {
    a[i] /= b;
  }
  return a;
};
#endif // __KNAPSACKS__
