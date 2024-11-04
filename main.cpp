#include "branch_and_bound.hpp"
#include "knapsacks.hpp"
#include "utilities.hpp"
#include "interface.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
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
  if (argc < 2) {
    throw std::invalid_argument(
        "Provide the file name of a problem as first input (required) and "
        "custom file name for solution as second input (optional)");
  }

  std::string file_prob = argv[1];
  std::ifstream istrm(file_prob);

  Knapsack tmp(0);
  istrm >> tmp;

  auto N{tmp.prices.size()};
  tmp.bounds() = Bounds(N, 0., 1.);
  tmp.integrality = std::vector<bool>(N, true);

  auto [solution, root] = branch_bound<Knapsack, QueueBestBound>(tmp);

  auto pos = file_prob.rfind('.');
  if (pos != std::string::npos) {
    file_prob.erase(pos);
  }

  std::string solution_file = file_prob + "_sol.txt";
  if (argc >= 3) {
    solution_file = argv[2];
  }

  std::ofstream ostrm(solution_file);
  ostrm << solution;

  return 0;
}
