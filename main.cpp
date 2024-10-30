#include "branch_and_bound.hpp"
#include "knapsacks.hpp"
#include "utilities.hpp"
#include <fstream>
#include <iostream>
#include <string>

std::ostream &operator<<(std::ostream &os, Solution &item) {
  os << item.value;
  for (auto &i : item.solution) {
    os << "\n" << i;
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

int main(int argc, char *argv[]) {
  std::string filename = "solution.txt";
  std::ifstream istrm(filename);
  Solution tmp;
  istrm >> tmp;
  std::cout << tmp;

  return 0;
}
