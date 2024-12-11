#include "hist_to_sol.hpp"
#include "brute_force.hpp"
#include "brute_force_pruned.hpp"
#include "gen_data.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <future>

constexpr int TIMEOUT = 45;

typedef pair<SolverStatus, RuleSet> SolverResult;
typedef SolverResult (*Solver)(int alphabet_size, int depth, vector<Symbol> target, int timeout);

vector<Solver> solvers = {
    BF::solver,
    BFP::solver,
    HM::solver<HM::hist_solver_z3, HM::ruleset_solver_matching>,
    HM::solver<HM::hist_solver_jump, HM::ruleset_solver_matching>
};

SolverResult test_solver(Solver solver, Data data) {
    return solver(data.alphabet_size, data.depth, data.target, TIMEOUT);
}

vector<pair<Data, vector<SolverResult>>> test(DataGen data_gen, int samples) {
    vector<pair<Data, vector<SolverResult>>> result_table;

    for (int i = 0; i < samples; i++) {
        auto [ iter, data ] = data_gen.gen();
        cout << "sample generated...\n";
        cout << string(data) << "\n";

        vector<SolverResult> results;

        // cout << "results: ";
        // cout.flush();
        // for (auto solver : solvers) {
        //     auto result = test_solver(solver, data);
        //     results.push_back(result);

        //     cout << result.first << " ";
        //     cout.flush();
        // }
        // cout << "\n";

        result_table.push_back({ data, results });
    }

    return result_table;
}

int main() {
    srand(time(0) * 17);

    auto depth_range = Range(3, 3);
    auto alphabet_range = Range(3, 3);
    auto complexity_range = Range(5, 5);

    auto data_gen = DataGen(depth_range, alphabet_range, complexity_range);

    auto table = test(data_gen, 10);

    for (auto record : table) {
        cout << string(record.first) << "\n";
    }
}