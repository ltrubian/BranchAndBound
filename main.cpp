#include "knapsacks.hpp"
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
  new_bounds.upper[index] = 3;
  std::cout << new_bounds;
  current_bounds.lower[index] = 3 + 1.0;
  std::cout << new_bounds;
  std::cout << current_bounds;
  std::valarray<std::size_t> in{3, 4, 0, 7};
  std::valarray<int> l{-1, 1, 2, 3, 4, 5, 6, 7};
  std::valarray<std::size_t> rep{in[std::slice(0, 2, 1)]};
  for (auto &x : rep)
    std::cout << x << "\t";
  std::cout << std::endl;
  for (auto &x : in)
    std::cout << x << "\t";
  std::cout << std::endl;

  return 0;
}
