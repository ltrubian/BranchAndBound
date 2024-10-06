#include "knapsacks.hpp"
#include "knapsacks_highs.hpp"
#include <cmath>
#include <forward_list>
#include <iostream>
#include <limits>
#include <random>

Bounds random_bounds(std::size_t n, std::size_t seed)
{
    Bounds bounds(n, 0.f, 1.f);
    std::mt19937_64 gen(seed);
    std::uniform_int_distribution<std::size_t> dis(0, 2);
    for (auto i { 0ul }; i < n; ++i) {
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
int main()
{
    std::random_device rd;
    std::uniform_int_distribution<std::size_t> di(0);
    std::size_t n { 100 };
    std::size_t s_k { di(rd) };
    s_k = 8722141901008443932;
    Bounds real(n, 0.f, 1.f);
    Knapsack prob { 5, 20.f, n, s_k };
    KnapsackHighs prob_h { prob };
    OptimalSolution opt { prob.branch_bound(real) };
    std::cout << "seed \t\t\t n  \tS \tnodes \tvalue" << std::endl;
    std::cout << s_k << "\t" << n << "\t" << opt.success << "\t" << opt.nodes
              << "\t" << opt.value;
    std::cout << std::endl;
    opt = prob_h.solve_integer((real));
    std::cout << "seed \t\t\t n  \tS \tnodes \tvalue" << std::endl;
    std::cout << s_k << "\t" << n << "\t" << opt.success << "\t" << opt.nodes
              << "\t" << opt.value;
    std::cout << std::endl;
    return 0;
}
