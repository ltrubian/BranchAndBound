#include "catch.hpp"
#include "knapsacks.hpp"
#include "knapsacks_highs.hpp"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <random>

static float TOL{1e-3};

Bounds random_bounds(std::size_t n, std::size_t seed) {
  Bounds bounds(n, 0.f, 1.f);
  std::mt19937_64 gen(seed);
  std::uniform_int_distribution<std::size_t> dis(0, 2);
  for (auto i{0ul}; i < n; ++i) {
    switch (dis(gen)) {
    case 0:
      bounds.lower[i] = 1.f;
      break;
    case 1:
      bounds.upper[i] = 0.f;
      break;
    case 2:
      break;
    }
  }

  return bounds;
};

TEST_CASE("knapsack: relaxed solver") {
  std::size_t v{5};
  float m{20.f};
  std::size_t n = GENERATE(range(50, 101, 10));
  SECTION("default bounds, fixed problems") {
    std::size_t seed = GENERATE(range(0, 100, 1));
    Knapsack problem{v, m, n, seed};
    KnapsackHighs problem_highs(problem);
    Bounds bounds{n, 0.f, 1.f};
    auto sol_my{problem.solve_relaxed(bounds)};
    auto sol_hi{problem_highs.solve_relaxed(bounds)};

    REQUIRE(sol_my.success == sol_hi.success);
    REQUIRE_THAT(sol_my.value, Catch::Matchers::WithinAbs(sol_hi.value, TOL));
  }
  SECTION("default bounds, random problems") {
    std::size_t seed = GENERATE(take(20, random(0, 100000)));
    Knapsack problem{v, m, n, seed};
    KnapsackHighs problem_highs(problem);
    Bounds bounds{n, 0.f, 1.f};
    auto sol_my{problem.solve_relaxed(bounds)};
    auto sol_hi{problem_highs.solve_relaxed(bounds)};

    REQUIRE(sol_my.success == sol_hi.success);
    REQUIRE_THAT(sol_my.value, Catch::Matchers::WithinAbs(sol_hi.value, TOL));
  }
  SECTION("random bounds") {
    std::size_t seed = GENERATE(take(20, random(0, 100000)));
    Knapsack problem{v, m, n, seed};
    KnapsackHighs problem_highs(problem);
    Bounds bounds{random_bounds(n, seed)};
    auto sol_my{problem.solve_relaxed(bounds)};
    auto sol_hi{problem_highs.solve_relaxed(bounds)};

    REQUIRE(sol_my.success == sol_hi.success);
    REQUIRE_THAT(sol_my.value, Catch::Matchers::WithinAbs(sol_hi.value, TOL));
  }
}

TEST_CASE("knapsack: branch and bound") {
  std::size_t v{5};
  float m{20.f};
  std::size_t n = GENERATE(range(50, 101, 10));
  SECTION("default bounds, fixed problems") {
    std::size_t seed = GENERATE(range(0, 10, 1));
    Knapsack problem{v, m, n, seed};
    KnapsackHighs problem_highs(problem);
    Bounds bounds{n, 0.f, 1.f};
    auto sol_my{problem_highs.branch_bound(bounds)};
    auto sol_hi{problem_highs.solve_integer(bounds)};

    REQUIRE(sol_my.success == sol_hi.success);
    REQUIRE_THAT(sol_my.value, Catch::Matchers::WithinAbs(sol_hi.value, TOL));
  }
  SECTION("default bounds, random problems") {
    std::size_t seed = GENERATE(take(5, random(0, 100000)));
    Knapsack problem{v, m, n, seed};
    KnapsackHighs problem_highs(problem);
    Bounds bounds{n, 0.f, 1.f};
    auto sol_my{problem_highs.branch_bound(bounds)};
    auto sol_hi{problem_highs.solve_integer(bounds)};

    REQUIRE(sol_my.success == sol_hi.success);
    REQUIRE_THAT(sol_my.value, Catch::Matchers::WithinAbs(sol_hi.value, TOL));
  }
  SECTION("random bounds") {
    std::size_t seed = GENERATE(take(5, random(0, 100000)));
    Knapsack problem{v, m, n, seed};
    KnapsackHighs problem_highs(problem);
    Bounds bounds{random_bounds(n, seed)};
    auto sol_my{problem_highs.branch_bound(bounds)};
    auto sol_hi{problem_highs.solve_integer(bounds)};

    REQUIRE(sol_my.success == sol_hi.success);
    REQUIRE_THAT(sol_my.value, Catch::Matchers::WithinAbs(sol_hi.value, TOL));
  }
}

TEST_CASE("knapsack: all assembled") {
  std::size_t v{5};
  float m{20.f};
  std::size_t n = GENERATE(range(50, 101, 10));
  SECTION("default bounds, fixed problems") {
    std::size_t seed = GENERATE(range(0, 10, 1));
    Knapsack problem{v, m, n, seed};
    KnapsackHighs problem_highs(problem);
    Bounds bounds{n, 0.f, 1.f};
    auto sol_my{problem.branch_bound(bounds)};
    auto sol_hi{problem_highs.solve_integer(bounds)};

    REQUIRE(sol_my.success == sol_hi.success);
    REQUIRE_THAT(sol_my.value, Catch::Matchers::WithinAbs(sol_hi.value, TOL));
  }
  SECTION("default bounds, random problems") {
    std::size_t seed = GENERATE(take(5, random(0, 100000)));
    Knapsack problem{v, m, n, seed};
    KnapsackHighs problem_highs(problem);
    Bounds bounds{n, 0.f, 1.f};
    auto sol_my{problem.branch_bound(bounds)};
    auto sol_hi{problem_highs.solve_integer(bounds)};

    REQUIRE(sol_my.success == sol_hi.success);
    REQUIRE_THAT(sol_my.value, Catch::Matchers::WithinAbs(sol_hi.value, TOL));
  }
  SECTION("random bounds") {
    std::size_t seed = GENERATE(take(5, random(0, 100000)));
    Knapsack problem{v, m, n, seed};
    KnapsackHighs problem_highs(problem);
    Bounds bounds{random_bounds(n, seed)};
    auto sol_my{problem.branch_bound(bounds)};
    auto sol_hi{problem_highs.solve_integer(bounds)};

    REQUIRE(sol_my.success == sol_hi.success);
    REQUIRE_THAT(sol_my.value, Catch::Matchers::WithinAbs(sol_hi.value, TOL));
  }
}
