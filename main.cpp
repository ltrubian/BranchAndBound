#include "knapsacks.hpp"
#include "knapsacks_highs.hpp"
#include <cmath>
#include <forward_list>
#include <iostream>
#include <limits>
#include <random>

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
int main() {
  std::random_device rd;
  std::uniform_int_distribution<std::size_t> di(0);
  std::size_t n{100};
  std::size_t s_b{di(rd)}, s_k{di(rd)};
  // s_b = 14216009864108917669;
  s_k = 8722141901008443932 ;
  std::cout << "seed bounds: \t" << s_b << "\n"
            << "seed knapsa: \t" << s_k << std::endl;
  Bounds bounds(random_bounds(n, s_b));
  Bounds real(n, 0.f, 1.f);
  Knapsack prob{5, 20.f, n, s_k};
  KnapsackHighs prob_h{prob};
  OptimalSolution opt{prob.branch_bound(real)};
  std::cout << opt.success << "\t" << opt.nodes << "\t" << opt.value;
  std::cout << std::endl;
  /*std::vector<float> tmp{0., 2., 3.4, 4., 6.};
  auto t{0};
  float integral{0.f}, fractional{0.f};
  for(auto& x : tmp){
    std::cout << x  <<   "\t"
              << &x <<   "\t"
              << &x - &tmp[0] <<   "\t";
    std::cout << std::endl;
    fractional = std::modf(x, &integral);
    if (fractional != 0.0){
      t = &x - &tmp[0];
      break;
    }
  }
  std::cout << "t:\t " << t  << "\t" << typeid(t).name() << std::endl;
  std::cout << "int:\t" << integral << std::endl;
  std::cout << "frac:\t" << fractional << std::endl;

*/

  return 0;
}
