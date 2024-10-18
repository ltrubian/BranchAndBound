#ifndef __UTILITIES__LT
#define __UTILITIES__LT

#include <bitset>
#include <string>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

typedef double MFloat;


struct OptimalSolution {
  bool success;
  std::size_t nodes;
  MFloat value;
  std::vector<MFloat> solution;

  OptimalSolution()
      : success{false}, nodes{0},
        value{-std::numeric_limits<MFloat>::infinity()},
        solution{std::vector<MFloat>()} {};

  bool is_integer() const;
};

std::ostream &operator<<(std::ostream &os, OptimalSolution &item);

std::istream &operator>>(std::istream &is, OptimalSolution &item);

struct Bounds {
  std::vector<MFloat> lower;
  std::vector<MFloat> upper;

  Bounds() : lower{std::vector<MFloat>()}, upper{std::vector<MFloat>()} {};
  Bounds(std::size_t n, MFloat low, MFloat up)
      : lower{std::vector<MFloat>(n, low)}, upper{std::vector<MFloat>(n, up)} {};
  explicit Bounds(std::size_t n)
      : Bounds(n, -std::numeric_limits<MFloat>::infinity(),
               std::numeric_limits<MFloat>::infinity()){};
  std::vector<MFloat> &operator[](std::size_t i) {
    switch (i) {
    case 0:
      return upper;
    case 1:
      return lower;
    default:
      throw std::out_of_range("only 0: upper, 1:lower bounds available");
    }
  };
  const std::vector<MFloat> &operator[](std::size_t i) const {
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

constexpr const std::size_t nSuccess{0};
constexpr const std::size_t nRelevant{1};
constexpr const std::size_t nExplored{2};
constexpr const std::size_t nInteger{3};

struct Node {
  MFloat value;
  std::size_t b_index;
  MFloat b_value; // for the knapsack it is always 0, but it is not so in general
  std::bitset<4> info;
  std::unique_ptr<Node> childs[2];

  Node()
      : value(-std::numeric_limits<MFloat>::infinity()), b_index{0},
        b_value{0.f}, info{}, childs{nullptr, nullptr} {};
  Node(MFloat value) : Node() { this->value = value; };
  ~Node(){};

  MFloat best_upper_bound() const ;

  std::string to_json() const{
    std::string js = "{";
    js += "\"value\": \"" + std::to_string(value) + "\",";
    js += "\"info\" : \"" + info.to_string() + "\"";
    if (childs[0] != nullptr || childs[1] != nullptr){
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
  std::size_t node_id;
  MFloat value;
  Bounds bounds;
  Node &node;

  ExploreNode(std::size_t id, Bounds &bounds, Node &node)
      : node_id(id), value(node.value), bounds(bounds), node(node){};
};
struct DepthFirst {
  constexpr bool operator()(const ExploreNode &a, const ExploreNode &b) const {
    return a.node_id < b.node_id;
  };
};
struct BestBoundFirst {
  constexpr bool operator()(const ExploreNode &a, const ExploreNode &b) const {
    return (a.value < b.value) || (a.value == b.value && a.node_id > b.node_id);
  }
};
template <typename T> struct PruneAll {
  constexpr void operator()(T &queue, const MFloat value) {
    for (auto i{queue.begin()}; i != queue.end();) {
      if (i->value <= value) {
        i = queue.erase(i);
      } else {
        ++i;
      }
    }
  }
};

template <typename T> struct PruneUntill {
  constexpr void operator()(T &queue, const MFloat value) {
    auto i{queue.begin()};
    while (i != queue.end() && i->value <= value) {
      i = queue.erase(i);
    }
  }
};
template <typename T> struct PruneNone {
  constexpr void operator()(T &queue, const MFloat value) {};
};

#endif // __UTILITIES__LT
