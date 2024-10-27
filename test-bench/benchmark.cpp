
#include "branch_and_bound.hpp"
#include "knapsacks.hpp"
#include "utilities.hpp"
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <random>
#include <string>

Solution select_start_opt(const Knapsack &prob, const Bounds &bounds,
                          int select);

std::tuple<Solution, std::unique_ptr<Node>, long int>
select_solver_variant(const Knapsack &prob, const Bounds &bounds,
                      const Solution start_opt, int select);

void print_statistics(std::vector<double> time[3][2],
                      std::vector<double> node[3][2]);

void print_progress(int N,
    int n_test, int max_n_test, int sopt, int solver,
    std::chrono::time_point<std::chrono::high_resolution_clock> &partial,
    std::chrono::time_point<std::chrono::high_resolution_clock> &total);

void benchmark(float v, float m);

int main() {
  std::cout << "start benchmark" << std::endl;
  benchmark(5, 20.f);
  std::cout << "end benchmark" << std::endl;
}

void benchmark(float v, float m) {
  std::random_device rd;
  //std::mt19937_64 gen(seed);
  std::uniform_int_distribution<std::size_t> di(0);
  std::string filename{"bench-" + std::to_string(di(rd)) + ".csv"};
  std::ofstream result(filename);
  auto max_n_test{100};
  auto total = std::chrono::high_resolution_clock::now();
  for (auto N{40}; N < 201; N += 10) {
    std::vector<double> time_res[3][2];
    std::vector<double> node_res[3][2];
    auto partial = std::chrono::high_resolution_clock::now();
    for (auto n_test(0); n_test < max_n_test; ++n_test) {
      const auto seed{di(rd)};
      const auto problem{Knapsack(v, m, N, seed)};
      const auto bounds{Bounds(N, 0.f, 1.f)};
      for (auto sopt{0}; sopt < 3; ++sopt) {
        const auto start_opt{select_start_opt(problem, bounds, sopt)};
        for (auto solver{0}; solver < 2; ++solver) {
          auto [opt, root, d] =
              select_solver_variant(problem, bounds, start_opt, solver);
          result << N << "," << n_test << "," << seed << "," // test id
                 << sopt << "," << solver << ","             // solver id
                 << opt.nodes << "," << d << ","             // bench result
                 << opt.value;                               // compare opt_sol
          time_res[sopt][solver].emplace_back(d);
          node_res[sopt][solver].emplace_back(opt.nodes);
          print_progress(N, n_test, max_n_test, sopt, solver, partial, total);
        }
      }
    }
    print_statistics(time_res, node_res);
  }
}

void print_statistics(std::vector<double> time[3][2],
                      std::vector<double> node[3][2]) {
  auto max_n_test{time[0][0].size()};
      std::cout << std::endl;
  std::cout << std::left << std::setprecision(4) << std::setw(40) << "time"
            << std::setw(40) << "nodes" << std::endl;
  std::cout << std::setw(20) << "depth" << std::setw(20) << "best bound" << "\t"
            << std::setw(20) << "depth" << std::setw(20) << "best bound"
            << std::endl;
  for (auto sopt{0}; sopt < 3; ++sopt) {
    for (auto solver{0}; solver < 2; ++solver) {
      double mean = std::accumulate(time[sopt][solver].begin(),
                                    time[sopt][solver].end(), 0.) /
                    max_n_test;
      auto diff = mean - time[sopt][solver];
      double std_dev = std::sqrt(
          std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.) /
          max_n_test);
      std::cout << std::setw(6) << std::left << mean << "(" << std::internal
                << std::setw(6) << std_dev << ")\t";
    }

    for (auto solver{0}; solver < 2; ++solver) {

      auto mean = std::accumulate(node[sopt][solver].begin(),
                                  node[sopt][solver].end(), 0.) /
                  max_n_test;
      auto diff = mean - node[sopt][solver];
      auto std_dev = std::sqrt(
          std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.) /
          max_n_test);
      std::cout << "\t";
      std::cout << std::setw(8) << std::left << mean << "(" << std::internal
                << std::setw(6) << std_dev << ")";
    }
    std::cout << "\n";
  }
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
                      const Solution start_opt, int select) {
  // Solution opt; Node root(0.);
  auto time_it = [&](auto F) {
    auto t0 = std::chrono::high_resolution_clock::now();
    auto [opt, root] = F(prob, bounds, start_opt, std::chrono::seconds(300));
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

void print_progress(int N,
    int n_test, int max_n_test, int sopt, int solver,
    std::chrono::time_point<std::chrono::high_resolution_clock> &partial,
    std::chrono::time_point<std::chrono::high_resolution_clock> &total) {
  int barWidth = 30;
  double progress{(n_test * 6. + sopt * 2 + solver) / (max_n_test * 6.)};
  auto c_time = std::chrono::high_resolution_clock::now();
  auto d_total =
      std::chrono::duration_cast<std::chrono::milliseconds>(c_time - total)
          .count() /
      1000.;
  auto d_n =
      std::chrono::duration_cast<std::chrono::milliseconds>(c_time - partial)
          .count() /
      1000.;
  std::cout << "size:\t" << N << "\tn test:\t" << n_test + 1 << "/"
            << max_n_test << "\t" << "["                 //
            << std::left << std::setw(barWidth)          //
            << std::string(barWidth * progress + 2, '=') //
            << "] "                                      //
            << std::ceil(progress * 100) << " %" << "\t " << d_n << "s\t "
            << d_total << "s" << "\r";
  std::cout.flush();
}
