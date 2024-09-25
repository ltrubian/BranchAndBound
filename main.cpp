#include "branch_bound.hpp"
#include <forward_list>
#include <iostream>
#include <limits>

int main() {
  std::cout << "Hello World! " << (-std::numeric_limits<float>::infinity() > 5)
            << std::endl;
  std::cout << 5 / 6 << std::endl;
  std::cout << 10 / 6 << std::endl;
  const int &A{3};

  std::forward_list<char> chars{'A', 'B', 'C', 'D'};

  for (; !chars.empty(); chars.pop_front())
    std::cout << "chars.front(): '" << chars.front() << "'\n";

  std::size_t index = 1;
  Bounds current_bounds(3);

  std::cout << current_bounds;
  Bounds new_bounds{current_bounds};
  std::cout << new_bounds;
  new_bounds(index, BoundType::upper) = 3;
  std::cout << new_bounds;
  current_bounds(index, BoundType::lower) = 3 + 1.0;
  std::cout << new_bounds;
  std::cout << current_bounds;

  return 0;
}
