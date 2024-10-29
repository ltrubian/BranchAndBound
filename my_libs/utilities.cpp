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
