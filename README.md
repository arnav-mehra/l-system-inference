### What is this?

In a prior project, I created a web app for visualizing D0L-systems (deterministic, context-free Lidenmayer systems).
For this project, I created a tool for inferring D0L-systems from a string generated by the system and the step at which it was generated.
Such a tool would be useful for hypothesizing about the underlying growth pattern of plants and other processes for which L-systems are commonly used to model.

### What did I do?

Here, I have implemented and tested a couple of methods for D0L-system inference. This includes:
1. Enumeration: Generating and testing all possible grammars until a solution is reached.
2. Two-step method: Because each rule is applied in parallel between steps, the number of each symbol in a generated string can be computed via the formula v_A * M_R^step = v_T. v_A represents a histogram vector where v_A[i] represents the number of occurrences of the i^th symbol in the axiom string, M_R represents a histogram matrix where M_R[i][j] represents the number of times the j^th symbol appears in the successor string of the i^th symbol, and v_T represents a histogram vector of the generated string. Using this fact, we can split inference into two distinct steps: first determining the possible number of occurrences of each symbol in the axiom and successor strings, and second determining a valid ordering in which these symbols can be placed to generate the target string at the given step. As we will see, splitting the problem into two smaller problems can drastically reduce the search space and improve inference time.

## How did I do it?

To test each method, we tried a few different implementations:
- Bottom-up enumeration: Generate all possible systems by taking an existing system and appending any symbol to any successor string, thus creating deviants of the original system. Now, do this in a BFS manner and you obtain the code in `cpp-code/brute_force.hpp`.
- Bottom-up enumeration with pruning: Follow the former method, but prune systems with 0 potential for generating the given string. Without too much additional inference, we landed on the following strategy: compute the string generated at step - 1, map these symbols to a list of their corresponding successor strings, and ensure the target string starts with the first successor string and contains each subsequent successor string in order. Our implementation of this can be seen in `cpp-code/brute_force_pruned.hpp`.
- Two-step method via SMT solver: To determine the frequencies of each symbol in the axiom and successor strings, set it up as an integer program in Z3 solver (`python-code/hist_solver_z3.py`). To determine the ordering of these symbols, use a simple bottom-up enumeration approach similar to what we did before (`cpp-code/hist_enum.hpp::BFH::RuleOrder`).
- Two-step method via JuMP framework: Follow the former method, but use the JuMP framework to use almost any solver you like over Z3 (`julia-code/hist_solver_jump.py`).
- Two-step method via enumeration with pruning: Follow the former method, but instead of using an integer program solver, generate possible axiom and successor histograms via bottom-up enumeration (`cpp-code/hist_enum.hpp::BFH::RuleGen`). We prune ensuring that we would not generate any more of a given symbol than is present in the target string at the given step.

For testing and benchmarking, we opted to infer from randomly generated grammars that followed conditions common to meaningful D0L-systems. Our implementation of this generation can be found at `cpp-code/gen_data.hpp`.

## What were the results?

While we are still analyzing results and reasoning about their differences, the performance of each method can be summarized as so: bottom-up enumeration is the slowest, the two-step method via Z3 and JuMP are similar in performance and usually faster (depending on the parameters of system generation), bottom-up enumeration with pruning is even faster, and two-step method via enumeration with pruning is the fastest. Our main takeaways are that the two-step approach appears to provide a promising way to compress the search space (by eliminating ordering) while still ensuring solid pruning and that the use of IP solvers in the first step is likely only better than pruned enumeration for high frequencies (where enumeration cannot practically reach).
