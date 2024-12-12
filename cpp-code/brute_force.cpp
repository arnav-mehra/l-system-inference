#include "brute_force.hpp"
#include "brute_force_pruned.hpp"
#include "gen_data.hpp"
#include <chrono>

int main() {
    auto depth_range = Range(5, 5);
    auto alphabet_range = Range(4, 4);
    auto complexity_range = Range(3, 3);

    auto data_gen = DataGen(depth_range, alphabet_range, complexity_range);
    auto [iter, data] = data_gen.gen();
    cout << string(data) << "\n";

    auto start = std::chrono::high_resolution_clock::now();
    auto [ status, rule_set ] = BFP::solver(data.alphabet_size, data.depth, data.target, 30);
    auto end = std::chrono::high_resolution_clock::now();
    print_solver_result(status, rule_set);

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
}