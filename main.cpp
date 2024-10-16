#include "branch_and_bound.hpp"
#include "knapsacks.hpp"
#include "knapsacks_highs.hpp"
#include "test-bench/catch.hpp"
#include "utilities.hpp"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>

auto select_start_opt(const Knapsack &prob, const Bounds &bounds, int select) {
  switch (select) {
  case 0:
    return OptimalSolution();
  case 1:
    return prob.solve_integer_naive(bounds);
  case 2:
    return prob.solve_integer_guess(bounds);
  default:
    throw std::out_of_range("0-2: no other starting solution available");
  }
}
auto select_solver_variant(const Knapsack &prob, const Bounds &bounds,
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
    return time_it(branch_bound<Knapsack, DepthFirst, PruneNone>);
  case 1:
    return time_it(branch_bound<Knapsack, DepthFirst, PruneUntill>);
  case 2:
    return time_it(branch_bound<Knapsack, DepthFirst, PruneAll>);
  case 3:
    return time_it(branch_bound<Knapsack, BestBoundFirst, PruneNone>);
  case 4:
    return time_it(branch_bound<Knapsack, BestBoundFirst, PruneUntill>);
  case 5:
    return time_it(branch_bound<Knapsack, BestBoundFirst, PruneAll>);
  default:
    throw std::out_of_range("0-5 are valid, no other variants are allowed");
  }
}
auto benchmark(float v, float m) {
  std::random_device rd;
  std::uniform_int_distribution<std::size_t> di(0);
  std::string filename{"bench-" + std::to_string(di(rd)) + ".csv"};
  std::ofstream result(filename);
  for (auto N{40}; N < 151; N += 10) {
    for (auto n_test(0); n_test < 10; ++n_test) {
      const auto seed{di(rd)};
      const auto problem{Knapsack(v, m, N, seed)};
      const auto bounds{Bounds(N, 0.f, 1.f)};
      const auto tester{KnapsackHighs(problem)};
      auto t0 = std::chrono::high_resolution_clock::now();
      const auto real_opt{tester.solve_integer(bounds)};
      auto t1 = std::chrono::high_resolution_clock::now();
      auto d_t = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0)
                     .count();
      for (auto sopt{0}; sopt < 3; ++sopt) {
        const auto start_opt{select_start_opt(problem, bounds, sopt)};
        for (auto solver{0}; solver < 6; ++solver) {
          auto [opt, root, d] =
              select_solver_variant(problem, bounds, start_opt, solver);
          result << N << "," << n_test << "," << seed << ","  // test id
                 << sopt << "," << solver << ","              // solver id
                 << opt.nodes << "," << d << ","              // bench result
                 << opt.value << "," << real_opt.value << "," // compare opt_sol
                 << std::to_string(real_opt.value == opt.value) << "," //
                 << d_t << std::endl;
        }
      }
    }
  }
}
constexpr std::size_t sProblem{0};
constexpr std::size_t sFirst{1};
constexpr std::size_t sPrune{2};
constexpr std::size_t sSeed{3};
constexpr std::size_t sN{4};
constexpr std::size_t sOptSol{5};

auto parser(int argc, char *argv[]) {
  std::random_device rd;
  std::uniform_int_distribution<std::size_t> di(0);
  auto i{1};
  std::bitset<6> settings;
  std::string flags{"-s-n-f-p-optnone"};
  std::size_t seed{di(rd)}, n{50}, first{3}, prune{1};
  OptimalSolution opt{};
  Knapsack problem(0);
  std::string first_arg;
  if (argc >= 2) {
    first_arg = argv[1];
    if (first_arg.find("-") != 0) {
      std::ifstream istrm(first_arg);
      istrm >> problem;
      settings.set(sProblem);
      ++i;
    }
  }
  while (i < argc) {
    first_arg = argv[i];
    ++i;
    switch (flags.find(first_arg)) {
    case 0:
      if (settings.test(sProblem) || settings.test(sSeed))
        std::clog
            << "[WARNING] problem or seed already provided: this is ignored"
            << std::endl;
      else {
        settings.set(sSeed);
        seed = std::stoi(argv[i]);
      }
      break;
    case 2:
      if (settings.test(sProblem) || settings.test(sN))
        std::clog << "[WARNING] problem or n already provided: this is ignored"
                  << std::endl;
      else {
        settings.set(sN);
        n = std::stoi(argv[i]);
      }
      break;
    case 4:
      if (settings.test(sFirst))
        std::clog
            << "[WARNING] order approach already provided: this is ignored"
            << std::endl;
      else {
        int i_f = std::string("dbb").find(argv[i]);
        if (i_f == flags.npos)
          throw std::invalid_argument("-f option require 'd' (depth-first) or "
                                      "'bb' (best-bound-first) as input");
        first = i_f * 3;
        settings.set(sFirst);
      }
      break;
    case 6:
      if (settings.test(sPrune))
        std::clog
            << "[WARNING] prune approach already provided: this is ignored"
            << std::endl;
      else {
        int i_p = std::string("nua").find(argv[i]);
        if (i_p == flags.npos)
          throw std::invalid_argument("-p option require 'n' (no prune) or "
                                      "'u' (untill) or 'a' (all) as input");
        prune = i_p;
        settings.set(sPrune);
      }
      break;
    case 8:
      opt = OptimalSolution();
      settings.set(sOptSol);
      break;
    default:
      throw std::invalid_argument("Unknown argument provided");
    }
    ++i;
  }
  if (problem.prices.size() == 0) {
    problem = Knapsack(5, 20.f, n, seed);
  }
  if (!settings.test(sOptSol)) {
    opt = problem.solve_integer_guess(Bounds(problem.prices.size(), 0.f, 1.f));
  }
  return std::make_tuple(problem, opt, first + prune, seed);
};

int main(int argc, char *argv[]) { /*
                                 std::string filename = "solution.txt";
                                 std::ifstream istrm(filename);
                                 OptimalSolution tmp;
                                 istrm >> tmp;
                                 std::cout << tmp;*/
  std::cout << "start benchmark" << std::endl;
  benchmark(5, 20.f);
  std::cout << "end benchmark" << std::endl;
  /*
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
  auto [prob, start_opt, select, seed] = parser(argc, argv);
  Bounds real(prob.prices.size(), 0.f, 1.f);
  const auto [opt, root, d] =
      select_solver_variant(prob, real, start_opt, select);
  std::cout << "seed:\t" << seed << std::endl;
  std::cout << prob.prices.size() << "\t" << opt.success << "\t" << opt.nodes
            << "\t" << opt.value << "\t"
            << typeid(PruneUntill<BestBoundFirst>).name() << std::endl;
  std::string filename{"test.json"};
  std::ofstream istrm(filename);
  istrm << root->to_json() << "\n";
  std::cout << "\n best upper: " << root->best_upper_bound() << std::endl;

  KnapsackHighs check{prob};
  auto opt_highs = check.solve_integer(real);
  std::cout << "highs value: " << opt_highs.value << std::endl;
*/
  return 0;
}
