#include "knapsacks.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

Knapsack::Knapsack(std::size_t v, float m, std::size_t n, std::size_t seed)
    : Knapsack { n }
{
    // pairs generation
    std::mt19937_64 gen(seed);
    std::uniform_real_distribution<float> dis_w(1.0f, 1000.0f);
    std::vector<float> small_w(v), small_p(v);
    std::generate(std::begin(small_w), std::end(small_w),
        [&]() { return dis_w(gen); });
    for (auto i { 0ul }; i < v; ++i) {
        std::uniform_real_distribution<float> dis_p(small_w[i] + 95.0f,
            small_w[i] + 105.0f);
        small_p[i] = dis_p(gen);
    }
    // pair normalization
    small_p /= (m + 1);
    small_w /= (m + 1);
    // items generation
    std::uniform_real_distribution<float> multiplier(1.f, m);
    std::uniform_int_distribution<std::size_t> choice(0, v - 1);
    for (auto i { 0ul }; i < n; ++i) {
        std::size_t pair { choice(gen) };
        float mult { multiplier(gen) };
        this->prices[i] = std::ceil(small_p[pair] * mult);
        this->weights[i] = std::ceil(small_w[pair] * mult);
    }
    // set capacity
    float tmp { std::accumulate(this->weights.begin(), this->weights.end(), 0.f) };
    this->capacity = std::ceil(tmp / 3);
};

const OptimalSolution Knapsack::solve_relaxed(const Bounds& bounds) const
{
    OptimalSolution opt_sol {};
    float correct_capacity { this->capacity - std::inner_product(std::begin(bounds.lower), std::end(bounds.lower), std::begin(this->weights), 0.0f) };
    // if the items the bounds make me take are too much => infeasible bounds
    if (correct_capacity < 0)
        return opt_sol;
    std::vector<float> real_prices { bounds.upper * (1.0f - bounds.lower) };
    long int items_takable { std::count_if(std::begin(real_prices),
        std::end(real_prices),
        [](float p) { return 0.f != p; }) };
    // if there are no other items to take but the ones I must => the solution is
    // the item I must take
    opt_sol.solution = bounds.lower; // and the lower bounds are the "starting" optimal solution
    if (items_takable == 0) {
        opt_sol.success = true;
        opt_sol.value = this->objective(opt_sol.solution);
        return opt_sol;
    }
    real_prices *= (this->prices / this->weights);
    std::vector<std::size_t> indexes(real_prices.size());
    std::iota(std::begin(indexes), std::end(indexes), 0ul);
    std::sort(indexes.begin(), indexes.end(),
        [&](std::size_t& x, std::size_t& y) {
            return real_prices[x] > real_prices[y];
        });
    for (auto ind { std::begin(indexes) };
         ind != std::begin(indexes) + items_takable; ++ind) {
        if (correct_capacity < this->weights[*ind]) {
            opt_sol.solution[*ind] = correct_capacity / this->weights[*ind];
            break;
        }
        correct_capacity -= this->weights[*ind];
        opt_sol.solution[*ind] = 1.0f;
    }
    opt_sol.success = true;
    opt_sol.value = this->objective(opt_sol.solution);

    return opt_sol;
}

const float Knapsack::objective(const std::vector<float>& solution) const
{
    float res { 0.0 };
    auto sol { std::begin(solution) };
    for (auto price { std::begin(this->prices) }; price != std::end(this->prices);
         ++price) {
        res = std::move(res) + *price * *sol;
        ++sol;
    }
    return res;
}

template <class T>
void prune_all(T& queue, const float value)
{
    for (auto i { queue.begin() }; i != queue.end();) {
        if (i->value <= value) {
            i = queue.erase(i);
        } else {
            ++i;
        }
    }
}

template <class T>
void prune_untill(T& queue, const float value)
{
    auto i { queue.begin() };
    while (i != queue.end() && i->value <= value) {
        i = queue.erase(i);
    }
}

const OptimalSolution Knapsack::branch_bound(Bounds& bounds,
    OptimalSolution opt) const
{
    OptimalSolution opt_sol;

    std::set<ExploreNode, BestBoundFirst> active_problems;

    Node root;

    OptimalSolution current_sol { this->solve_relaxed(bounds) };

    if (current_sol.success && opt_sol.value < current_sol.value) {
        // search for the first non integer value of the solution
        float integral { 0.f }, fractional { 0.f };
        auto index { 0 };
        for (auto& x : current_sol.solution) {
            fractional = std::modf(x, &integral);
            if (fractional != 0.f) {
                index = &x - &current_sol.solution[0];
                break;
            }
        }
        root.value = current_sol.value;
        // second bound: if the solution is integer, that is the best solution for
        // the entire tree of its subproblem
        if (fractional == 0.f) {
            opt_sol.value = current_sol.value;
            opt_sol.solution = current_sol.solution;
        } else {
            root.b_index = index;
            root.b_value = integral;
            root.value = current_sol.value;
            active_problems.emplace(ExploreNode(0, bounds, root));
        }
    }
    while (!active_problems.empty()) {
        ExploreNode current_prob = std::move(active_problems.extract(active_problems.cbegin()).value());

        for (auto i { 0 }; i < 2; ++i) {
            Bounds current_bounds { current_prob.bounds };
            current_bounds[i][current_prob.node.b_index] = current_prob.node.b_value + 1.f * i;

            // solve the relaxed problem
            OptimalSolution current_sol { this->solve_relaxed(current_bounds) };
            ++opt_sol.nodes;
            if (current_sol.success && opt_sol.value < current_sol.value) {
                // search for the first non integer value of the solution
                float integral { 0.f }, fractional { 0.f };
                auto index { 0 };
                for (auto& x : current_sol.solution) {
                    fractional = std::modf(x, &integral);
                    if (fractional != 0.f) {
                        index = &x - &current_sol.solution[0];
                        break;
                    }
                }
                current_prob.node.childs[i].reset(new Node(current_sol.value));
                if (fractional == 0.f) {
                    opt_sol.value = current_sol.value;
                    opt_sol.solution = current_sol.solution;
                    current_prob.node.childs[i]->integrality = true;
                    prune_all(active_problems, opt_sol.value);
                } else {
                    current_prob.node.childs[i]->b_index = index;
                    current_prob.node.childs[i]->b_value = integral;
                    active_problems.emplace(ExploreNode(opt_sol.nodes, current_bounds,
                        *current_prob.node.childs[i]));
                }
            }
        }
        current_prob.node.explored = true;
    }
    // in case the loop is stopped, active_problems could contains subproblems
    // to explore
    opt_sol.success = active_problems.empty() && opt_sol.value != -std::numeric_limits<float>::infinity();
    return opt_sol;
}
