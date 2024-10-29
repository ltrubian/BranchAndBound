
#include "argparse.hpp"
#include "branch_and_bound.hpp"
#include "knapsacks.hpp"
#include "utilities.hpp"
#include <chrono>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <random>
#include <string>
#include <thread>

Solution select_start_opt(const Knapsack &prob, int select);

std::tuple<Solution, std::unique_ptr<Node>, long int>
select_solver_variant(const Knapsack &prob, const Solution start_opt,
                      int select);

std::vector<bool> mixed_integer(std::size_t n, std::size_t seed);

void print_statistics(std::vector<double> time[3][2],
                      std::vector<double> node[3][2]);

void print_progress(
    int N, int n_test, int max_n_test, int sopt, int solver,
    std::chrono::time_point<std::chrono::high_resolution_clock> &partial,
    std::chrono::time_point<std::chrono::high_resolution_clock> &total);

void benchmark(std::size_t v, float m, std::size_t max_n_test,
               std::vector<std::size_t> &sizes, std::string filename,
               std::size_t seed, bool progress, bool statistics,
               bool mixed_int);

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("benchmark");
  program.set_usage_max_line_width(60);
  program.set_usage_break_on_mutex();
  program.set_assign_chars(" =");
  program.add_description(
      "Run a benchmark of branch and bound algorithm:\n"
      "for 'sizes' in [start, end) with step, it will run a number of "
      "'samples' "
      "with all the algorithms that I was testing in all the conditions: \n "
      "(Depth First / Best Bound First) with (Empty, Naive, Guess) initial "
      "solution\n"
      "For a default run, launch the program without arguments");
  auto &random_group = program.add_mutually_exclusive_group();

  random_group.add_argument("-s", "--seed")
      .help("seed used for generating the sequence of problems")
      .scan<'u', std::size_t>()
      .default_value<std::size_t>(100)
      .nargs(1)
      .required();
  random_group.add_argument("--random")
      .help("choose a random initial seed for the sequence of problems")
      .flag();
  program.add_argument("--mixed-integer")
      .help("the problem will be mixed-integer programms")
      .flag();
  program.add_argument("-v")
      .help("number of sample used for generating the whole problem")
      .scan<'u', std::size_t>()
      .required()
      .nargs(1)
      .default_value<std::size_t>(5);
  program.add_argument("-m")
      .help("normalizer used for generating the whole problem")
      .scan<'g', double>()
      .required()
      .nargs(1)
      .default_value<double>(20.);
  program.add_argument("--samples")
      .help("number of samples for each problem dimension")
      .scan<'u', std::size_t>()
      .required()
      .nargs(1)
      .default_value<std::size_t>(10);
  program.add_argument("--sizes")
      .help(
          "start, end, step: size N of the problems will be in [start, end). ")
      .nargs(1, 3)
      .default_value(std::vector<std::size_t>{40, 100, 10})
      .required()
      .scan<'u', std::size_t>();
  program.add_argument("-o", "--output-file")
      .help("specify the file output, only for raw data")
      .required()
      .nargs(1)
      .default_value("bench.csv");
  program.add_argument("--quiet")
      .help("which part of the output should not be printed")
      .default_value(std::string("none"))
      .choices("none", "all", "prog", "stat")
      .nargs(1)
      .required();

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }
  std::size_t v = program.get<std::size_t>("-v");
  double m = program.get<double>("-m");
  std::size_t samples = program.get<std::size_t>("--samples");
  auto sizes = program.get<std::vector<std::size_t>>("--sizes");
  auto filename = program.get<std::string>("-o");
  auto seed = program.get<std::size_t>("-s");
  auto quiet = program.get<std::string>("--quiet");
  bool mixed_int{false};

  if (program["--mixed-integer"] == true)
    mixed_int = true;

  bool progress{true}, statistics{true};

  if (quiet == "all") {
    progress = false;
    statistics = false;
  } else if (quiet == "prog") {
    progress = false;
  } else if (quiet == "stat") {
    statistics = false;
  }

  if (program["--random"] == true) {
    std::random_device rd;
    seed = rd();
  }

  if (!program.is_used("-o")) {
    std::clog
        << "default name for output file lead to overwrite, stop if undesired"
        << std::endl;
    for (auto x{0}; x < 5; ++x) {
      std::clog << "\t" << std::left << std::setw(5) << std::string(x + 1, '.')
                << "  " << (5 - x - 1) << "s\r" << std::flush;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::clog << std::left << std::setw(20) << "that's  fine!" << std::endl;
  }

  if (sizes.size() != 3)
    sizes.emplace_back(10);
  if (sizes.size() != 3)
    sizes.insert(--sizes.end(), 100);

  std::clog << "start benchmark, starting seed: " << seed << std::endl;
  benchmark(v, m, samples, sizes, filename, seed, progress, statistics,
            mixed_int);
  std::clog << "end benchmark" << std::endl;

  return 0;
}

void benchmark(std::size_t v, float m, std::size_t max_n_test,
               std::vector<std::size_t> &sizes, std::string filename,
               std::size_t seed, bool progress, bool statistics,
               bool mixed_int) {
  std::mt19937_64 rd(seed);
  std::uniform_int_distribution<std::size_t> di(0);
  std::ofstream result(filename);
  auto total = std::chrono::high_resolution_clock::now();
  for (auto N{sizes[0]}; N < sizes[1]; N += sizes[2]) {
    std::vector<double> time_res[3][2];
    std::vector<double> node_res[3][2];
    auto partial = std::chrono::high_resolution_clock::now();
    for (auto n_test(0); n_test < max_n_test; ++n_test) {
      const auto seed{di(rd)};
      auto problem{Knapsack(v, m, N, seed)};
      if (mixed_int)
        problem.integrality = mixed_integer(N, seed);
      for (auto sopt{0}; sopt < 3; ++sopt) {
        const auto start_opt{select_start_opt(problem, sopt)};
        for (auto solver{0}; solver < 2; ++solver) {
          auto [opt, root, d] =
              select_solver_variant(problem, start_opt, solver);
          result << N << "," << n_test << "," << seed << "," // test id
                 << sopt << "," << solver << ","             // solver id
                 << opt.nodes << "," << d << ","             // bench result
                 << opt.value;                               // compare opt_sol
          time_res[sopt][solver].emplace_back(d);
          node_res[sopt][solver].emplace_back(opt.nodes);
          if (progress)
            print_progress(N, n_test, max_n_test, sopt, solver, partial, total);
        }
      }
    }
    if (progress)
      std::cout << std::endl;

    if (statistics)
      print_statistics(time_res, node_res);
  }
}

Solution select_start_opt(const Knapsack &prob, int select) {
  switch (select) {
  case 0:
    return Solution();
  case 1:
    return prob.solve_integer_naive();
  case 2:
    return prob.solve_integer_guess();
  default:
    throw std::out_of_range("0-2: no other starting solution available");
  }
}
std::tuple<Solution, std::unique_ptr<Node>, long int>
select_solver_variant(const Knapsack &prob, const Solution start_opt,
                      int select) {
  // Solution opt; Node root(0.);
  auto time_it = [&](auto F) {
    auto t0 = std::chrono::high_resolution_clock::now();
    auto [opt, root] = F(prob, start_opt, std::chrono::seconds(300));
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
void print_statistics(std::vector<double> time[3][2],
                      std::vector<double> node[3][2]) {
  auto max_n_test{time[0][0].size()};
  std::vector<std::string> solutions = {"empty", "naive", "guess"};

  std::cout << "\t\t" << std::left << std::setprecision(4) << std::setw(40)
            << "time (milliseconds)" << std::setw(40) << "number of nodes"
            << std::endl;
  std::cout << "\t" << std::setw(16) << "depth" << std::setw(20) << "best bound"
            << "\t" << std::setw(24) << "depth" << std::setw(20) << "best bound"
            << std::endl;
  for (auto sopt{0}; sopt < 3; ++sopt) {
    std::cout << solutions[sopt] << ":\t";
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

void print_progress(
    int N, int n_test, int max_n_test, int sopt, int solver,
    std::chrono::time_point<std::chrono::high_resolution_clock> &partial,
    std::chrono::time_point<std::chrono::high_resolution_clock> &total) {
  int barWidth = 30;
  double progress{((n_test) * 6. + (sopt) * 2 + 1 + solver) /
                  (max_n_test * 6.)};
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
            << max_n_test << "\t" << "["             //
            << std::left << std::setw(barWidth)      //
            << std::string(barWidth * progress, '=') //
            << "] "                                  //
            << std::ceil(progress * 100) << " %" << "\t " << d_n << "s\t "
            << d_total << "s" << "\r";
  std::cout.flush();
}

std::vector<bool> mixed_integer(std::size_t n, std::size_t seed) {
  std::vector<bool> res(n);
  std::mt19937_64 gen(seed);
  std::uniform_int_distribution<std::size_t> dis(0, 1);
  for (auto i{0ul}; i < n; ++i) {
    switch (dis(gen)) {
    case 0:
      res[i] = true;
      break;
    case 1:
      res[i] = false;
      break;
    }
  }
  return res;
};
