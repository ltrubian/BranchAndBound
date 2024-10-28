#include "branch_and_bound.hpp"
#include "knapsacks.hpp"
#include "utilities.hpp"
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  std::string filename = "solution.txt";
  std::ifstream istrm(filename);
  Solution tmp;
  istrm >> tmp;
  std::cout << tmp;

  return 0;
}
