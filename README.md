# Branch and bound for Mixed-Integer Linear Programming
This is an educational project of a numerical optimization course.

## Aim
Implementation of a branch and bound algorithm for solving Mixed-Integer Linear Programming. 
The toy model used as test case is a classic Knapsack problem, generated as reported in the [project](project.pdf) file

## Dependencies 
ONLY for testing: [HiGHS](https://ergo-code.github.io/HiGHS/stable/)

HiGHS is software for the definition, modification and solution of large scale sparse linear optimization models.
HiGHS is freely available from [GitHub](https://github.com/ERGO-Code/HiGHS) under the MIT licence and has no third-party dependencies.

HiGHS refers to the folowing article: Parallelizing the dual revised simplex method, Q. Huangfu and J. A. J. Hall, Mathematical Programming Computation, 10 (1), 119-142, 2018. DOI: [10.1007/s12532-017-0130-5](https://link.springer.com/article/10.1007/s12532-017-0130-5)

## Special thanks to very usefull projects
These projects have greatly simplified the creation of benchmarks and tests, making it possible to fully focus on the correcteness and the results.
Since they are both header files, they are already included in the project.
  - [Catch2](https://github.com/catchorg/Catch2): mainly a unit testing framework for C++, but it also provides basic micro-benchmarking features;
  - [argparse](http://github.com/p-ranav/argparse): single header file for argparsing command line arguments.

## Benchmarking
It is possible to compare the behaviour of the algorithm varying two main factor:
  - the solver of the relaxed problem (empty | naive | guess)
  - the order of exploration of the subproblems tree (depth first | best bound first)

It is relevant to notice that:
  - best bound first is significantly better of the depth first approach; 
  - when using best bound first, the choice of the solver for the relaxed problem is irrelevant.

(these facts are true both in time and number of explored nodes). A run of this type

    ./test-bench/benchmark.x --sizes 500 510 --samples 30

gives as results something like (mean results of 30 runs)
|solver|depth |  best bound |  depth   |   best bound  |
|:--:|:--:|:--:|:--:|:--:|
| | time (ms)| time (ms) | n° nodes| n° nodes |
|empty|99.2 |20.1  |8188 |1235 |
|naive|97.7 |21.6  |8177 |1235 |
|guess|82 |21.0  |6510 |1232 |

