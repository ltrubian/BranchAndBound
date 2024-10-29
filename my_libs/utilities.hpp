#ifndef __UTILITIES__LT
#define __UTILITIES__LT

#include <limits>
#include <list>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

struct Solution {
  bool success;
  std::size_t nodes;
  double value;
  double gap;
  std::vector<double> solution;

  Solution()
      : success{false}, nodes{0},
        value{-std::numeric_limits<double>::infinity()},
        gap{std::numeric_limits<double>::infinity()}, solution() {};

  bool is_integer() const;
};


struct Bounds {
  std::vector<double> lower;
  std::vector<double> upper;

  Bounds() = default;
  Bounds(std::size_t n, double low, double up) : lower(n, low), upper(n, up) {};
  explicit Bounds(std::size_t n)
      : Bounds(n, -std::numeric_limits<double>::infinity(),
               std::numeric_limits<double>::infinity()) {};

  std::vector<double> &operator[](std::size_t i) {
    switch (i) {
    case 0:
      return upper;
    case 1:
      return lower;
    default:
      throw std::out_of_range("only 0: upper, 1:lower bounds available");
    }
  };
  const std::vector<double> &operator[](std::size_t i) const {
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
  double value;
  std::size_t b_index;
  double b_value;
  std::unique_ptr<Node> childs[2];
  const Node *parent;

  Node() = default;
  explicit Node(double value)
      : value(value), b_index{0}, b_value{0.}, childs{nullptr},
        parent(nullptr) {};

  Node(double value, Node *parent) : Node(value) { this->parent = parent; };
  ~Node() {};

  void initialize_bounds(Bounds &bounds) const {
    if (parent == nullptr)
      return;
    auto i{this == parent->childs[1].get()};
    bounds[i][parent->b_index] = parent->b_value + 1. * i;
    parent->initialize_bounds(bounds);
  };

  int count_node() const {
    int n{1};
    if (childs[0])
      n += childs[0]->count_node();
    if (childs[1])
      n += childs[1]->count_node();
    return n;
  }

  std::string to_json() const {
    std::string js = "{";
    js += "\"value\": \"" + std::to_string(value) + "\",";
    if (childs[0] != nullptr || childs[1] != nullptr) {
      js += ",\"childs\": [ ";
      if (childs[0] != nullptr)
        js += childs[0]->to_json() + ",";
      if (childs[1] != nullptr)
        js += childs[1]->to_json();
      js += "]";
    }
    return js + "}";
  };
};

struct ExploreNode {
  double value;
  Node &node;

  ExploreNode(Node &node) : value(node.value), node(node) {};
};

/**
 * Order to explore the queue starting from the largest upper bound available
 * Due to complications in "pruning" the set using reverse iterator, the
 * smallest value is in the front for easy removal while the largest is at the
 * end
 */
struct BestBoundFirst {
  inline bool operator()(const ExploreNode &a, const ExploreNode &b) const {
    return (a.value < b.value);
  }
};
/**
 * thin wrapper over the set container
 */
struct QueueBestBound {
  std::multiset<ExploreNode, BestBoundFirst> queue;

  void prune(const double &value) {
    auto ex_node{queue.begin()};
    while (ex_node != queue.end() && ex_node->value <= value) {
      ex_node = queue.erase(ex_node);
    }
  };

  double max_value() const { return (--queue.end())->value; };

  bool empty() const { return queue.empty(); };
  std::size_t size() const { return queue.size(); }

  /**
   * Take the node with the largest value available
   */
  ExploreNode take_next() {
    return std::move(queue.extract(--queue.end()).value());
  };

  template <class... Args> void emplace(Args &&...args) {
    queue.emplace(std::forward<Args>(args)...);
  };
};

/**
 * Thin wrapper over the list container to simulate stack
 */
struct QueueDepth {
  std::list<ExploreNode> queue;

  void prune(const double &value) {
    for (auto ex_node{queue.begin()}; ex_node != queue.end();) {
      if (ex_node->value <= value)
        ex_node = queue.erase(ex_node);
      else
        ++ex_node;
    }
  };

  double max_value() const {
    double max{-std::numeric_limits<double>::infinity()};
    for (auto ex_node{queue.begin()}; ex_node != queue.end(); ++ex_node) {
      if (ex_node->value > max)
        max = ex_node->value;
    }
    return max;
  };

  bool empty() const { return queue.empty(); };
  std::size_t size() const { return queue.size(); }

  ExploreNode take_next() {
    ExploreNode tmp{std::move(queue.front())};
    queue.pop_front();
    return tmp;
  };

  template <class... Args> void emplace(Args &&...args) {
    queue.emplace_front(std::forward<Args>(args)...);
  };
};

#endif // __UTILITIES__LT
