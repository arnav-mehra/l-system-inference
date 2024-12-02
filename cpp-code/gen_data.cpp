#include "gen_data.hpp"

int main() {
    auto depth_range = Range(4, 4);
    auto alphabet_range = Range(4, 4);
    auto complexity_range = Range(6, 6);

    auto data_gen = DataGen(depth_range, alphabet_range, complexity_range);
    auto [iter, data] = data_gen.gen();
    cout << "iters: " << iter << '\n';
    cout << string(data);

    auto h = Histogram(data.alphabet_size);
    h.digest(data.target);
    cout << string(h);

    // cout << data.rule_set.produces(data.target, 4);
}