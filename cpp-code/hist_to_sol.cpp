#include "hist_to_sol.hpp"

int main() {
    int alphabet_size = 2;
    Symbols target({ 2, 1, 1, 2, 1, 2, 2, 1 });
    const int depth = 3;

    auto [status, rule_set] = HM::solver<
        HM::hist_solver_z3_ip,
        HM::ruleset_solver_matching
    >(alphabet_size, depth, target);

    printSolverResult(status, rule_set);
}

// Symbols target = { '0', '1', '2', '1' };

// const Histograms hists = {
//     { AXIOM_SYMBOL, {{'0', 1}, {'1', 1}, {'2', 0}} },
//     { '0', {{'0', 0}, {'1', 1}, {'2', 0}} },
//     { '1', {{'0', 1}, {'1', 0}, {'2', 1}} },
//     { '2', {{'0', 0}, {'1', 1}, {'2', 0}} }
// };

// for (auto [ from_symbol, hist ] : hists) {
//     cout << from_symbol << ":\n";
//     for (auto [ to_symbol, cnt ] : hist) {
//         cout << to_symbol << ' ' << cnt << "\n";
//     }
// }