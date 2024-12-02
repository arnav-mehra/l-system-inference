#pragma once

#include "util.hpp"
#include <queue>
#include <fstream>
#include <sstream>
#include <string>

#define DEBUG false

namespace HM {

struct RuleGen {
    queue<pair<RuleSet, Histograms>> branches;
    Histogram rule_lengths;
    Symbols target;
    int alphabet_size;

    RuleGen(int alphabet_size, Histograms hists, Symbols target)
        : target(target), alphabet_size(alphabet_size), rule_lengths(alphabet_size) {
        // init rule_lengths
        for (Symbol from_symbol = AXIOM_SYMBOL; from_symbol <= alphabet_size; from_symbol++) {
            for (Symbol to_symbol = AXIOM_SYMBOL; to_symbol <= alphabet_size; to_symbol++) {
                rule_lengths[from_symbol] += hists[from_symbol][to_symbol];
            }
            if constexpr (DEBUG) cout << "symbol=" << from_symbol << " cnt=" << rule_lengths[from_symbol] << '\n';
        }

        // init first branch
        RuleSet rule_set(alphabet_size);
        for (Symbol symbol = AXIOM_SYMBOL; symbol <= alphabet_size; symbol++) {
            rule_set[symbol] = {};
        }
        branches.push({rule_set, hists});
    }

    RuleSet& rule_set() {
        return branches.front().first;
    }

    Histograms& hists() {
        return branches.front().second;
    }

    void branch() {
        branches.push({ rule_set(), hists() });
    }

    void try_adding_symbol(const Symbol symbol, int idx_offset, const int depth, int iter_id) {
        auto& hist = hists()[symbol]; // histogram of remaining symbols.
        auto& to_symbols = rule_set()[symbol]; // current rule for symbol.

        for (Symbol next_symbol = AXIOM_SYMBOL + 1; next_symbol <= alphabet_size; next_symbol++) {
            int& cnt = hist[next_symbol];
            if (cnt == 0) continue;

            if constexpr (DEBUG) cout << iter_id << ": next_symbol=" << next_symbol << " cnt=" << cnt << '\n'; 

            // add symbol to rule
            to_symbols.push_back(next_symbol);
            cnt--;

            // try added symbol
            auto [ succ, new_idx_offset ] = iter(next_symbol, idx_offset, depth - 1);

            // if it worked, duplicate the current {RuleSet, Histograms} context.
            if (succ) {
                if constexpr (DEBUG) cout << iter_id << ": dup branch\n";
                branch();
            }

            // remove symbol from rule
            to_symbols.pop_back();
            cnt++;
        }
    }

    int id = 0;
    pair<bool, int> iter(const Symbol symbol, int idx_offset, const int depth) {
        auto& to_symbols = rule_set()[symbol]; // current rule for symbol.
        int needed_symbols = rule_lengths[symbol]; // number of symbols the rule needs to have.

        int iter_id = id++;
        if constexpr (DEBUG) cout << iter_id << ": symbol=" << symbol << " depth=" << depth << " offset=" << idx_offset << " needed_symbols=" << needed_symbols << '\n';

        if (depth == 0) {
            bool matches = target[idx_offset] == symbol;
            if constexpr (DEBUG) cout << iter_id << ": matches=" << matches << "\n";
            return { matches, idx_offset + 1 };
        }

        for (int i = 0; i < needed_symbols; i++) {
            if constexpr (DEBUG) cout << iter_id << ": i=" << i << '\n';

            // rule symbol already assigned, just need to check if its valid.
            if (i < to_symbols.size()) {
                Symbol next_symbol = to_symbols[i];
                auto [ succ, new_idx_offset ] = iter(next_symbol, idx_offset, depth - 1);
                idx_offset = new_idx_offset;
                if (!succ) return { false, idx_offset };
            }
            // rule symbols not assigned, try all possible additions.
            else {
                try_adding_symbol(symbol, idx_offset, depth, iter_id);
                return { false, idx_offset };
            }
        }

        // passed on all letters, return true.
        return { true, idx_offset };
    }

    pair<bool, RuleSet> run(int depth) {
        while (branches.size()) {
            if constexpr (DEBUG) cout << "\ntrying next branch:\n";

            id = 0;
            auto [succ, _] = iter(AXIOM_SYMBOL, 0, depth + 1); // + 1 since axiom is a psuedo-symbol
            if (succ) {
                return { true, rule_set() };
            }
            branches.pop();
        }
        return { false, RuleSet(alphabet_size) };
    }
};

const char* IN_BUFFER_FILE = "../inData";
const char* OUT_BUFFER_FILE = "../outData";

void write_inputs(int alphabet_size, int depth, Symbols target) {
    ofstream dataFile;
    dataFile.open(IN_BUFFER_FILE, std::ios::binary | std::ios::in | std::ios::trunc);

    Histogram h(alphabet_size);
    h.digest(target);

    string str = to_string(depth);
    for (Symbol symbol : h) {
        str += "," + to_string(symbol);
    }

    dataFile << str;
    dataFile.close();
}

pair<bool, Histograms> read_histograms(int alphabet_size) {
    ifstream file("../outData");

    // read CSV into values.
    vector<int> values;
    string line, value;
    while (getline(file, line)) {
        stringstream ss(line);
        while (getline(ss, value, ',')) {
            values.push_back(stoi(value));
        }
    }

    Histograms hists(alphabet_size);

    // return false if status=failed(0).
    int status = values[0];
    if (status == 0) {
        return { false, hists };
    }
    values.erase(values.begin());

    for (int i = 0; i < alphabet_size * (alphabet_size + 1); i++) {
        int from_symbol = i / alphabet_size;
        int to_symbol = i % alphabet_size;
        hists[from_symbol][to_symbol] = values[i];
    }
    return { true, hists };
}

// HIST SOLVERS

typedef pair<bool, Histograms> (*HistSolver)(int alphabet_size, int depth, vector<Symbol> target);

pair<bool, Histograms> hist_solver_z3(
    int alphabet_size,
    int depth,
    vector<Symbol> target
) {
    write_inputs(alphabet_size, depth, target);
    system("python -u ../python-code/hist_solver_z3.py");
    return read_histograms(alphabet_size);
}

pair<bool, Histograms> hist_solver_jump(
    int alphabet_size,
    int depth,
    vector<Symbol> target
) {
    write_inputs(alphabet_size, depth, target);
    system("julia ../julia-code/hist_solver_jump.jl");
    return read_histograms(alphabet_size);
}

// RULESET SOLVERS

typedef pair<bool, RuleSet> (*RuleSetSolver)(int alphabet_size, int depth, Histograms hists, vector<Symbol> target);

pair<bool, RuleSet> ruleset_solver_matching(
    int alphabet_size,
    int depth,
    Histograms hists,
    vector<Symbol> target
) {
    RuleGen gen(alphabet_size, hists, target);
    return gen.run(depth);
}

// MASTER SOLVER

template<HistSolver hist_solver, RuleSetSolver ruleset_solver>
pair<SolverStatus, RuleSet> solver(
    int alphabet_size,
    int depth,
    vector<Symbol> target
) {
    auto [succ1, hists] = hist_solver(alphabet_size, depth, target);
    if (!succ1) {
        return { SolverStatus::UNSAT_NO_HIST, RuleSet(alphabet_size) };
    }

    auto [succ2, rule_set] = ruleset_solver(alphabet_size, depth, hists, target);
    if (!succ2) {
        return { SolverStatus::UNSAT_NO_RULESET, RuleSet(alphabet_size) };
    }

    return { SolverStatus::SAT, rule_set };
}

}