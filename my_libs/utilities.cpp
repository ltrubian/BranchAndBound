#include "utilities.hpp"
#include <cmath>

bool Solution::is_integer() const {
  double integral{0.f}, fractional{0.f};
  for (auto &x : this->solution) {
    fractional = std::modf(x, &integral);
    if (fractional != 0.f) {
      return false;
    }
  }
  return true;
}

std::ostream &operator<<(std::ostream &os, Solution &item) {
  os << item.value << "\n";
  for (auto &i : item.solution) {
    os << i << "\n";
  }
  return os;
};

std::istream &operator>>(std::istream &is, Solution &item) {
  is >> item.value;
  while (is) {
    item.solution.emplace_back(0.);
    is >> *--item.solution.end();
  }
  return is;
};
