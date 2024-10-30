#include "argparse.hpp"
#include "branch_and_bound.hpp"
#include "knapsacks.hpp"
#include "utilities.hpp"
#include <cassert>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ostream>
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

enum class Solver {
  Depth = 0,
  BestBound = 1,
};

Solver solver(std::string &str) {
  if (str == "depth")
    return Solver::Depth;
  if (str == "bestbound")
    return Solver::BestBound;
  throw std::bad_cast();
};

std::vector<Solver> solvers(std::vector<std::string> vec) {
  std::vector<Solver> res;
  auto last{std::unique(vec.begin(), vec.end())};
  for (auto item{vec.begin()}; item != last; ++item)
    res.emplace_back(solver(*item));
  return res;
};

std::string to_string(Solver &item) {
  switch (item) {
  case Solver::Depth:
    return "Depth First";
  case Solver::BestBound:
    return "Best Bound First";
  }
};

std::ostream &operator<<(std::ostream &os, Solver &item) {
  return os << to_string(item);
};
enum class StartSol {
  None = 0,
  Naive = 1,
  Guess = 2,
};
StartSol startsol(std::string &str) {
  if (str == "none")
    return StartSol::None;
  if (str == "naive")
    return StartSol::Naive;
  if (str == "guess")
    return StartSol::Guess;
  throw std::bad_cast();
};
std::vector<StartSol> startsols(std::vector<std::string> vec) {
  std::vector<StartSol> res;
  auto last{std::unique(vec.begin(), vec.end())};
  for (auto item{vec.begin()}; item != last; ++item)
    res.emplace_back(startsol(*item));
  return res;
};
std::string to_string(StartSol &item) {
  switch (item) {
  case StartSol::None:
    return "None";
  case StartSol::Naive:
    return "Naive";
  case StartSol::Guess:
    return "Guess";
  }
};

std::ostream &operator<<(std::ostream &os, StartSol &item) {
  return os << to_string(item);
};

std::string to_filename(std::string name) {
  std::replace(name.begin(), name.end(), ' ', '_');
  return name;
};

std::ostream &operator<<(std::ostream &os, Solution &item) {
  os << item.value;
  for (auto &i : item.solution) {
    os << "\n" << i;
  }
  return os;
};

std::vector<bool> mixed_integer(std::size_t n, std::size_t seed);
Solution select_start_opt(const Knapsack &prob, StartSol select);
std::tuple<Solution, std::unique_ptr<Node>, long int>
select_solver_variant(const Knapsack &prob, const Solution start_opt,
                      Solver select, std::chrono::seconds max_time);

int main(int argc, char *argv[]) {
  argparse::ArgumentParser program("one-run");
  program.set_usage_max_line_width(60);
  program.set_usage_break_on_mutex();
  program.set_assign_chars(" =");
  program.add_description(
      "Solve a single Knapsack problem with one or both "
      "the solvers starting with no or any solution\n The "
      "solvers depends on how the queue of subproblems is "
      "processed (depth-first or best-bound-first).\n More solvers and "
      "starting solution can be provided but all the algorithms are run to "
      "solve the same problem");

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
  program.add_argument("-N")
      .help("Size of the problem")
      .scan<'u', std::size_t>()
      .required()
      .nargs(1)
      .default_value<std::size_t>(100);
  program.add_argument("--solver")
      .help("types of solver to compare (depth/bestbound)")
      .required()
      .nargs(1, 3)
      .default_value(std::vector<std::string>({"bestbound"}));
  program.add_argument("--start-sol")
      .help("types of starting solution (none/naive/guess)")
      .required()
      .nargs(1, 3)
      .default_value(std::vector<std::string>({"none"}));
  program.add_argument("--max-time")
      .help("number of seconds given to the solver to solve the problem")
      .scan<'u', std::size_t>()
      .required()
      .nargs(1)
      .default_value<std::size_t>(300);
  program.add_argument("--output")
      .help("output the problem and solution files")
      .flag();

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  std::size_t v = program.get<std::size_t>("-v");
  double m = program.get<double>("-m");
  std::size_t N = program.get<std::size_t>("-N");
  auto seed = program.get<std::size_t>("-s");
  auto solvs = solvers(program.get<std::vector<std::string>>("--solver"));
  auto stsols = startsols(program.get<std::vector<std::string>>("--start-sol"));
  auto max_time = std::chrono::seconds(program.get<std::size_t>("--max-time"));

  if (program["--random"] == true) {
    std::random_device rd;
    seed = rd();
  }

  auto problem{Knapsack(v, m, N, seed)};

  auto id_problem = std::to_string(v) + "_" + std::to_string(m) + "_" //
                    + std::to_string(N) + "_" + std::to_string(seed); //

  if (program["--mixed-integer"] == true)
    problem.integrality = mixed_integer(N, seed);

  if (program["--output"] == true) {
    std::string filename = id_problem + "_prob.txt";
    std::ofstream pr_file(filename);
    pr_file << problem;
  }

  std::cout << std::left << "problem paramters" << std::endl;
  std::cout << std::setw(5) << "v:" << std::setw(5) << v //
            << std::setw(5) << "m:" << std::setw(5) << m //
            << std::setw(8) << "N:" << std::setw(5) << N //
            << std::setw(8) << "seed:" << seed << std::endl;
  for (auto &sol : solvs) {
    for (auto &starting_sol : stsols) {
      Solution start_opt{select_start_opt(problem, starting_sol)};

      auto [opt, root, d] =
          select_solver_variant(problem, start_opt, sol, max_time);

      if (program["--output"] == true) {
        std::string filename = id_problem + "_" + to_filename(to_string(sol)) +
                               "_" + to_filename(to_string(starting_sol)) +
                               "_sol.txt";
        std::ofstream sol_my(filename);
        sol_my << opt;
      }

      std::cout << std::endl;
      std::cout << sol << " ---- " << starting_sol << std::endl;
      std::cout << std::boolalpha << std::setprecision(10) //
                << "success: " << opt.success              //
                << std::right << std::setw(15)
                << "integrality: " << opt.is_integer() //
                << std::setw(15) << "feasibility: " << problem.is_feasible(opt)
                << std::endl;
      std::cout << std::left;
      std::cout << std::setw(8) << "nodes:" << std::setw(10) << opt.nodes //
                << std::setw(10) << "time(ms):" << std::setw(7) << d      //
                << std::endl;
      std::cout << std::setw(8) << "value:" << std::setw(10) << opt.value  //
                << std::setw(10) << "last gap:" << std::setw(7) << opt.gap //
                << std::endl;
    }
  }

  /*
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
*/
  return 0;
}

Solution select_start_opt(const Knapsack &prob, StartSol select) {
  switch (select) {
  case StartSol::None:
    return Solution();
  case StartSol::Naive:
    return prob.solve_integer_naive();
  case StartSol::Guess:
    return prob.solve_integer_guess();
  default:
    throw std::out_of_range("0-2: no other starting solution available");
  }
}

std::tuple<Solution, std::unique_ptr<Node>, long int>
select_solver_variant(const Knapsack &prob, const Solution start_opt,
                      Solver select, std::chrono::seconds max_time) {
  auto time_it = [&](auto F) {
    auto t0 = std::chrono::high_resolution_clock::now();
    auto [opt, root] = F(prob, start_opt, max_time);
    auto t1 = std::chrono::high_resolution_clock::now();
    auto d =
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    return std::make_tuple(std::move(opt), std::move(root), std::move(d));
  };
  switch (select) {
  case Solver::Depth:
    return time_it(branch_bound<Knapsack, QueueDepth>);
  case Solver::BestBound:
    return time_it(branch_bound<Knapsack, QueueBestBound>);
  default:
    throw std::out_of_range("0-1 are valid, no other variants are allowed");
  }
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
