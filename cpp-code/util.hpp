#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <chrono>

using namespace std;

typedef int Symbol;

const auto AXIOM_SYMBOL = Symbol(0);

struct Histogram : vector<int> {
    Histogram(int alphabet_size) {
        this->resize(alphabet_size + 1, 0);
    }

    void digest(vector<Symbol> symbols) {
        for (Symbol symbol : symbols) {
            this->at(symbol)++;
        }
    }

    operator string() const {
        string s = "{ ";
        for (Symbol symbol = AXIOM_SYMBOL; symbol < size(); symbol++) {
            s += to_string(symbol) + ": " + to_string((*this)[symbol]);
            if (symbol != size() - 1) s += ", ";
        }
        return s + " }";
    }
};

struct Histograms : vector<Histogram> {
    Histograms(int alphabet_size) {
        this->resize(alphabet_size + 1, Histogram(alphabet_size));
    }

    operator string() const {
        string s = "[\n";
        for (Symbol symbol = AXIOM_SYMBOL; symbol < size(); symbol++) {
            s += "\t" + to_string(symbol) + ": " + string((*this)[symbol]);
            if (symbol != size() - 1) s += ",\n";
        }
        return s + "]";
    }
};

struct Symbols : vector<Symbol> {
    Symbols() {}
    Symbols(vector<Symbol> symbols) : vector<Symbol>(symbols) {}

    Histogram hist(int alphabet_size) {
        Histogram hist(alphabet_size);
        for (Symbol symbol : *this) {
            hist[symbol]++;
        }
        return hist;
    }

    operator string() const {
        string s = "[";
        for (int i = 0; i < size(); i++) {
            s += to_string((*this)[i]) + (i == size() - 1 ? "" : " "); 
        }
        return s + "]";
    }
};

struct RuleSet : vector<Symbols> {
    RuleSet() {}

    RuleSet(int alphabet_size) {
        this->resize(alphabet_size + 1, {}); // +1 for axiom = [0]
    }

    RuleSet pushed(Symbol from_symbol, Symbol to_symbol) {
        RuleSet copy = *this;
        copy[from_symbol].push_back(to_symbol);
        return copy;
    }

    Symbols apply_to(Symbols curr) {
        Symbols new_symbols;

        for (Symbol from_symbol : curr) {
            for (Symbol to_symbol : (*this)[from_symbol]) {
                new_symbols.push_back(to_symbol);
            }
        }

        return new_symbols;
    }

    pair<bool, int> produced_iter(Symbols& goal, int depth, Symbol symbol, int idx) {
        // cout << depth << ' ' << symbol << ' ' << idx << '\n';
        if (idx >= goal.size()) return { false, idx + 1 };

        if (depth == 0) {
            // cout << goal[idx] << ' ' << symbol << '\n';
            return { goal[idx] == symbol, idx + 1 };
        }

        for (Symbol to_symbol : (*this)[symbol]) {
            auto [ succ, new_idx ] = produced_iter(goal, depth - 1, to_symbol, idx);
            idx = new_idx;

            if (!succ) return { false, idx };
        }

        return { true, idx };
    }

    bool produces(Symbols& target, int depth) {
        auto [ succ, idx ] = produced_iter(target, depth + 1, AXIOM_SYMBOL, 0);
        return succ && idx == target.size();
    }

    int get_complexity() {
        int c = 0;
        for (Symbol from_symbol = AXIOM_SYMBOL; from_symbol < size(); from_symbol++) {
            auto to_symbols = (*this)[from_symbol];
            c += to_symbols.size() - 1;
        }
        return c;
    }

    operator string() const {
        string s = "[";
        for (Symbol from_symbol = AXIOM_SYMBOL; from_symbol < size(); from_symbol++) {
            auto to_symbols = (*this)[from_symbol];
            s += to_string(from_symbol) + " -> " + string(to_symbols) + ";";
            s += from_symbol == size() - 1 ? "" : " ";
        }
        s += "]";
        return s;
    }
};

struct RuleSetHash {
    size_t operator()(const RuleSet& rule_set) const {
        hash<int> hasher;
        size_t seed = 0;
        for (const Symbols& to_symbols : rule_set) {
            // use size to encode separation between rule strings. 
            seed ^= hasher(to_symbols.size()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            for (Symbol symbol : to_symbols) {
                seed ^= hasher(symbol) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
        }
        return seed;
    }
};

typedef unordered_set<RuleSet, RuleSetHash> RuleSetHashSet;

enum SolverStatus {
    UNSAT_NO_HIST,
    UNSAT_NO_RULESET,
    UNSAT_TIMEOUT,
    SAT
};

string solver_status_to_string(SolverStatus status) {
    switch (status) {
        case SolverStatus::UNSAT_NO_HIST: return "UNSAT_NO_HIST";
        case SolverStatus::UNSAT_NO_RULESET: return "UNSAT_NO_RULESET";
        case SolverStatus::UNSAT_TIMEOUT: return "UNSAT_TIMEOUT";
        case SolverStatus::SAT: return "SAT";
    }
    return "INVALID_STATUS";
}

void print_solver_result(SolverStatus status, RuleSet rule_set) {
    cout << solver_status_to_string(status) << "\n";
    if (status == SolverStatus::SAT) {
        cout << string(rule_set);
    }
}

struct Timer {
    int timeout;
    chrono::system_clock::time_point start_time;

    Timer(int timeout) : timeout(timeout) {}

    Timer& start() {
        start_time = chrono::high_resolution_clock::now();
        return *this;
    }

    bool timed_out() {
        auto curr_time = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = curr_time - start_time;
        double time_elapsed = duration.count();
        return time_elapsed >= timeout;
    }
};
