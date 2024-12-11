#include "hist_to_sol.hpp"
#include "brute_force.hpp"
#include "brute_force_pruned.hpp"
#include "gen_data.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <future>

constexpr int TIMEOUT = 10;

typedef pair<SolverStatus, RuleSet> SolverResult;
typedef SolverResult (*Solver)(int alphabet_size, int depth, vector<Symbol> target, int timeout);

vector<Solver> solvers = {
    BF::solver,
    BFP::solver,
    // HM::solver<HM::hist_solver_z3, HM::ruleset_solver_matching>,
    // HM::solver<HM::hist_solver_jump, HM::ruleset_solver_matching>,
};

vector<string> solver_names = {
    "enumeration",
    "enumeration_pruned",
    // "count_order_z3",
    // "count_order_jump",
};

SolverResult test_solver(Solver solver, Data data) {
    return solver(data.alphabet_size, data.depth, data.target, TIMEOUT);
}

typedef pair<SolverResult, double> Entry;
typedef vector<Entry> Entries;
typedef pair<Data, Entries> Record;
typedef vector<Record> Table;

Table test(DataGen data_gen, int samples) {
    Table result_table;

    for (int i = 0; i < samples; i++) {
        auto [ iter, data ] = data_gen.gen();
        // cout << "sample generated...\n";
        // cout << string(data) << "\n";

        Entries results;

        // cout << "results: ";
        // cout.flush();
        for (auto solver : solvers) {
            auto t1 = chrono::high_resolution_clock::now();
            auto result = test_solver(solver, data);
            auto t2 = chrono::high_resolution_clock::now();

            chrono::duration<double> duration = t2 - t1;

            results.push_back({ result, duration.count() });

            // cout << result.first << " ";
            // cout.flush();
        }
        // cout << "\n";

        result_table.push_back({ data, results });
    }

    return result_table;
}

void write_table(Table& table) {
    std::ofstream outfile("benchmark_table.csv");

    // write headers
    outfile << "alphabet_size,depth,complexity,rule_set,target,";
    for (auto& solver_name : solver_names) {
        outfile << (solver_name + "_status") << ","
                << (solver_name + "_time") << ","
                << (solver_name + "_rule_set") << ",";
    }
    outfile << endl;

    // write rows
    for (auto& record : table) {
        auto& [ data, entries ] = record;
        outfile << data.alphabet_size << ","
                << data.depth << ","
                << data.rule_set.get_complexity() << ","
                << string(data.rule_set) << ","
                << string(data.target) << ",";

        for (auto& entry : entries) {
            auto& [ result, time ] = entry;
            auto& [ status, rule_set ] = result;
            outfile << solver_status_to_string(status) << ","
                    << time << ","
                    << string(rule_set) << ",";
        }

        outfile << endl;
    }

    outfile.close();
}

int main() {
    srand(time(0) * 17);

    auto depth_range = Range(4, 4);
    auto alphabet_range = Range(3, 3);
    auto complexity_range = Range(3, 3);

    auto data_gen = DataGen(depth_range, alphabet_range, complexity_range);

    auto table = test(data_gen, 10);

    write_table(table);
}