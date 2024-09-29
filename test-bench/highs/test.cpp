#include "catch.hpp"

#include "knapsacks.hpp"
#include "knapsacks_highs.hpp"
#include <algorithm>
#include <cstddef>
#include <iterator>

TEST_CASE("knapsack: relaxed solver") {
  std::size_t v{5};
  float m{20.f};
  std::size_t n = GENERATE(range(20, 100, 10));
  SECTION("default bounds") {
    std::size_t seed = GENERATE(range(0, 1000, 1));
    Knapsack problem{v, m, n, seed};
    KnapsackHighs problem_highs(problem);
    Bounds bounds{n, 0.f, 1.f};
    auto sol_my{problem.solve_relaxed(bounds)};
    auto sol_hi{problem_highs.solve_relaxed(bounds)};

    REQUIRE(sol_my.success == true);
    REQUIRE(sol_hi.success == true);
    std::valarray<bool> sol_cmp{sol_hi.solution == sol_my.solution};
    REQUIRE(problem.objective(sol_my.solution) ==
            problem.objective(sol_hi.solution));
  }
}
