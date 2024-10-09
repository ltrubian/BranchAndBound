#include "branch_and_bound.hpp"
#include "knapsacks.hpp"
#include "knapsacks_highs.hpp"
#include "utilities.hpp"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <random>
#include <iostream>

template <typename T>
std::tuple<OptimalSolution, std::unique_ptr<Node>, double>
select_solver_variant(const T &prob, Bounds &bounds,
                      const OptimalSolution start_opt, int select);

constexpr std::size_t sProblem{0};
constexpr std::size_t sSearch{1};
constexpr std::size_t sPrune{2};
constexpr std::size_t sStartOp{3};

auto parser(int argc, char *argv[]) {
  auto i{1};
  std::bitset<4> settings;
  std::string flags{"-s-n-f-p-o"};
  std::string first{argv[1]};
  Knapsack problem(0);
  std:size_t seed;
  if (first.find("-") != 0) {
    std::ifstream istrm(first);
    istrm >> problem;
    settings.set(sProblem);
    ++i;
  }
  while (i < argc) {
    first = argv[i];
    ++i;
    switch (flags.find(first)) {
    case 0:
      if (settings.test(sProblem))
        std::clog << "[WARNING] argument already provided: this is ignored" << std::endl;
      else
        seed = std::stoi(argv[i]);
      break;
    case 2:
      break;
    case 4:
      break;
    case 6:
      break;
    case 8:
      break;
    default:
      throw std::invalid_argument("Unknown argument provided");
    }
  }
};

int main(int argc, char *argv[]) { /*
   std::string filename = "solution.txt";
   std::ifstream istrm(filename);
   OptimalSolution tmp;
   istrm >> tmp;
   std::cout << tmp;*/

  std::random_device rd;
  std::uniform_int_distribution<std::size_t> di(0);
  std::size_t n{10};
  std::size_t s_k{di(rd)};
  // s_k = 8722141901008443932;
  // s_k = 3178488925260001586;
  // s_k = 1104010588739253986;
  // s_k = 12968802468751711930;
  // s_k = 13607581404834641350;
  // s_k = 12696456601695067945;
  s_k = 1409891033439146690; // n = 300; ha oltre 3 milioni di nodi, circa 24
  // secodi
  std::cout << "seed:\t" << s_k << std::endl;
  Bounds real(n, 0.f, 1.f);
  Knapsack prob{5, 20.f, n, s_k};
  KnapsackHighs prob_h{prob};
  const auto [opt, root, d] =
      select_solver_variant(prob, real, OptimalSolution(), 5);
  std::cout << n << "\t" << opt.success << "\t" << opt.nodes << "\t"
            << opt.value << "\t" << typeid(PruneUntill<BestBoundFirst>).name()
            << std::endl;
  std::string filename{"test.json"};
  std::ofstream istrm(filename);
  istrm << root->to_json() << "\n";
  std::cout << "\n best upper: " << root->best_upper_bound() << std::endl;

  return 0;
}
template <typename T>
auto select_solver_variant(const T &prob, Bounds &bounds,
                           const OptimalSolution start_opt, int select) {
  auto time_it = [&](auto F) {
    auto t0 = std::chrono::high_resolution_clock::now();
    auto [opt, root] = F(prob, bounds, start_opt);
    auto t1 = std::chrono::high_resolution_clock::now();
    auto d =
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    return std::make_tuple(opt, std::move(root), d);
  };
  switch (select) {
  case 0:
    return time_it(branch_bound<T, DepthFirst, PruneNone>);
  case 1:
    return time_it(branch_bound<T, DepthFirst, PruneUntill>);
  case 2:
    return time_it(branch_bound<T, DepthFirst, PruneAll>);
  case 3:
    return time_it(branch_bound<T, BestBoundFirst, PruneNone>);
  case 4:
    return time_it(branch_bound<T, BestBoundFirst, PruneUntill>);
  case 5:
    return time_it(branch_bound<T, BestBoundFirst, PruneAll>);
  default:
    throw std::out_of_range("0-5 are valid, no other variants are allowed");
  }
}
