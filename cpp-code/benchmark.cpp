#include "hist_to_sol.hpp"
#include "brute_force.hpp"
#include "gen_data.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <future>

constexpr auto TIMEOUT = chrono::seconds(5);

typedef pair<SolverStatus, RuleSet> SolverResult;
typedef SolverResult (*Solver)(int alphabet_size, int depth, std::vector<Symbol> target);

vector<Solver> solvers = {
    // BF::solver,
    HM::solver<HM::hist_solver_z3_ip, HM::ruleset_solver_matching>
};

SolverResult test_solver(Solver solver, Data data) {
    future<SolverResult> result_future = async(launch::async, solver, data.alphabet_size, data.depth, data.target);
    future_status future_status = result_future.wait_for(TIMEOUT);

    if (future_status != future_status::ready) {
        terminate();
        return { SolverStatus::UNSAT_TIMEOUT, RuleSet() };
    }
    return result_future.get();
}

vector<pair<Data, vector<SolverResult>>> test(DataGen data_gen, int samples) {
    vector<pair<Data, vector<SolverResult>>> result_table;

    for (int i = 0; i < samples; i++) {
        auto [ iter, data ] = data_gen.gen();
        cout << "sample generated...\n";

        vector<SolverResult> results;

        cout << "results: ";
        cout.flush();
        for (auto solver : solvers) {
            auto result = test_solver(solver, data);
            results.push_back(result);

            cout << result.first << " ";
            cout.flush();
        }
        cout << "\n";

        result_table.push_back({ data, results });
    }

    return result_table;
}

int main() {
    srand(time(0) * 17);

    auto depth_range = Range(3, 3);
    auto alphabet_range = Range(3, 3);
    auto complexity_range = Range(1, 2);

    auto data_gen = DataGen(depth_range, alphabet_range, complexity_range);

    auto table = test(data_gen, 2);

    for (auto record : table) {
        cout << string(record.first) << "\n";
    }
}