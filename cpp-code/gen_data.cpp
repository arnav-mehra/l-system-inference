#include "gen_data.hpp"

int main() {
    auto depth_range = Range(3, 3);
    auto alphabet_range = Range(3, 3);
    auto complexity_range = Range(2, 2);

    auto data_gen = DataGen(depth_range, alphabet_range, complexity_range);
    auto [iter, data] = data_gen.gen();
    cout << "iters: " << iter << '\n';
    cout << string(data);

    cout << data.rule_set.produces(data.target, 4);
}