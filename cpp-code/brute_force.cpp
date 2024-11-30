#include "brute_force.hpp"
#include "gen_data.hpp"

int main() {
    auto depth_range = Range(4, 4);
    auto alphabet_range = Range(4, 4);
    auto complexity_range = Range(2, 2);

    auto data_gen = DataGen(depth_range, alphabet_range, complexity_range);
    auto [iter, data] = data_gen.gen();
    cout << string(data);

    auto [ status, rule_set ] = BF::solver(data.alphabet_size, data.depth, data.target);
    printSolverResult(status, rule_set);
}