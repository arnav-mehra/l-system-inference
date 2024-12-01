#include "hist_to_sol.hpp"
#include "brute_force.hpp"
#include "gen_data.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <future>

typedef typeof(BF::solver) Solver;
typedef pair<SolverStatus, RuleSet> SolverResult;

template<Solver solver>
void promisedSolver(
    int alphabet_size, int depth, vector<Symbol> target,
    promise<SolverResult>&& result_promise
) {
    auto res = solver(alphabet_size, depth, target);
    result_promise.set_value(res);
}


typedef void (*PromisedSolver)(
    int alphabet_size, int depth, std::vector<Symbol> target,
    std::promise<SolverResult> &&result_promise);


vector<PromisedSolver> promised_solvers = {
    promisedSolver<BF::solver>,
    promisedSolver<HM::solver<HM::hist_solver_z3_ip, HM::ruleset_solver_matching>>
};

SolverResult test_solver(PromisedSolver promised_solver, Data data) {
    promise<SolverResult> result_promise;
    future<SolverResult> result_future = result_promise.get_future();
    thread workerThread(
        promised_solver,
        data.alphabet_size, data.depth, data.target,
        move(result_promise)
    );

    future_status status = result_future.wait_for(chrono::seconds(10));
    if (status != future_status::ready) {
        return { SolverStatus::UNSAT_TIMEOUT, RuleSet() };
    }
    return result_future.get();
}

vector<pair<Data, vector<SolverResult>>> test(DataGen data_gen, int samples) {
    int right = 0;
    int wrong = 0;
    int time_out = 0;

    vector<pair<Data, vector<SolverResult>>> result_table;

    for (int i = 0; i < samples; i++) {
        auto [ iter, data ] = data_gen.gen();

        vector<SolverResult> results;
        // for (auto promised_solver : promised_solvers) {
        //     auto res = test_solver(promised_solver, data);
        //     results.push_back(res);
        // }

        result_table.push_back({ data, results });
    }

    return result_table;
}

int main() {
    auto depth_range = Range(3, 3);
    auto alphabet_range = Range(3, 3);
    auto complexity_range = Range(2, 2);

    auto data_gen = DataGen(depth_range, alphabet_range, complexity_range);

    auto table = test(data_gen, 10);

    for (auto record : table) {
        cout << string(record.first) << "\n";
    }
}