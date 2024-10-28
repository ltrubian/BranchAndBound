#include "argparse.hpp"
#include "branch_and_bound.hpp"
#include "knapsacks.hpp"
#include "utilities.hpp"
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <thread>

// s_k = 8722141901008443932;
// s_k = 3178488925260001586;
// s_k = 1104010588739253986;
// s_k = 12968802468751711930;
// s_k = 13607581404834641350;
// s_k = 12696456601695067945;
// s_k = 1409891033439146690;

Solution select_start_opt(const Knapsack &prob, const Bounds &bounds,
                          int select);
std::tuple<Solution, std::unique_ptr<Node>, long int>
select_solver_variant(const Knapsack &prob, const Bounds &bounds,
                      const Solution start_opt, int select, std::chrono::seconds max_time);

int main(int argc, char *argv[]) {
  std::random_device rd;
  std::uniform_int_distribution<std::size_t> di(0);
  std::size_t n{10};
  std::size_t seed{di(rd)};
  Knapsack prob(2, 3, 10, 1);
  Solution start_opt;
  Bounds real(prob.prices.size(), 0.f, 1.f);
  std::size_t select;
  auto [opt, root, d] = select_solver_variant(prob, real, start_opt, select, std::chrono::seconds(300));




  std::cout << "seed:\t" << seed << std::endl;
  std::cout << prob.prices.size() << "\t" << opt.success << "\t"
            << std::setprecision(20) << opt.nodes << "\t" << opt.value
            << std::endl;
  std::cout << "\n gap: " << opt.gap << std::endl;
  std::cout << opt.is_integer() << "\t" << prob.is_feasible(opt) << std::endl;

  std::cout << "nodes in the tree: " << root->count_node() << std::endl;






  std::string filename{"test.json"};
  std::ofstream istrm(filename);
  istrm << root->to_json() << "\n";
  filename = "problem.txt";
  std::ofstream pr_file(filename);
  pr_file << prob;
  filename = "solution_my.txt";
  std::ofstream sol_my(filename);
  sol_my << opt;

  return 0;
}

Solution select_start_opt(const Knapsack &prob, const Bounds &bounds,
                          int select) {
  switch (select) {
  case 0:
    return Solution();
  case 1:
    return prob.solve_integer_naive(bounds);
  case 2:
    return prob.solve_integer_guess(bounds);
  default:
    throw std::out_of_range("0-2: no other starting solution available");
  }
}

std::tuple<Solution, std::unique_ptr<Node>, long int>
select_solver_variant(const Knapsack &prob, const Bounds &bounds,
                      const Solution start_opt, int select, std::chrono::seconds max_time) {
  auto time_it = [&](auto F) {
    auto t0 = std::chrono::high_resolution_clock::now();
    auto [opt, root] = F(prob, bounds, start_opt, max_time );
    auto t1 = std::chrono::high_resolution_clock::now();
    auto d =
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    return std::make_tuple(std::move(opt), std::move(root), std::move(d));
  };
  switch (select) {
  case 0:
    return time_it(branch_bound<Knapsack, QueueDepth>);
  case 1:
    return time_it(branch_bound<Knapsack, QueueBestBound>);
  default:
    throw std::out_of_range("0-1 are valid, no other variants are allowed");
  }
}
