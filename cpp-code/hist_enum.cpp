#include "hist_enum.hpp"
#include "gen_data.hpp"
#include "brute_force_pruned.hpp"

int main() {
    auto depth_range = Range(4, 4);
    auto alphabet_range = Range(3, 3);
    auto complexity_range = Range(8, 8);

    auto data_gen = DataGen(depth_range, alphabet_range, complexity_range);
    auto [iter, data] = data_gen.gen();
    cout << string(data) << "\n";

    // auto start = std::chrono::high_resolution_clock::now();
    auto [ status, rule_hist ] = solver<BFH::RuleGen>(data.alphabet_size, data.depth, data.target, 30);
    // auto end = std::chrono::high_resolution_clock::now();
    // print_solver_result(status, rule_set);

    cout << solver_status_to_string(status) << '\n'
         << string(rule_hist) << '\n'
         << string(Histogram(data.alphabet_size).digest(data.target)) << '\n';

    // std::chrono::duration<double> elapsed = end - start;
    // std::cout << "Elapsed time: " << elapsed.count() << " seconds" << std::endl;
}