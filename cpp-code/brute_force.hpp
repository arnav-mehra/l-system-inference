#pragma once

#include "util.hpp"
#include <array>

namespace BF {

struct RuleGen {
    int alphabet_size;
    Symbols target;
    vector<vector<RuleSet>> results;

    RuleGen(int alphabet_size, Symbols target)
        : alphabet_size(alphabet_size), target(target) {}

    void gen_init() {
        Histogram target_hist = target.hist(alphabet_size);

        vector<RuleSet> curr_rule_sets = { RuleSet(alphabet_size) };

        for (Symbol from_symbol = AXIOM_SYMBOL; from_symbol <= alphabet_size; from_symbol++) {
            vector<RuleSet> new_rule_sets;

            for (RuleSet& old_rule_set : curr_rule_sets) {
                for (Symbol to_symbol = AXIOM_SYMBOL + 1; to_symbol <= alphabet_size; to_symbol++) {
                    if (target_hist[to_symbol] <= 0) continue;

                    RuleSet new_rule_set = old_rule_set;
                    new_rule_set[from_symbol].push_back(to_symbol);
                    new_rule_sets.push_back(new_rule_set);
                }
            }

            curr_rule_sets = new_rule_sets;
        }

        results = { curr_rule_sets };
    }

    vector<RuleSet> gen_deviants(RuleSet& curr_rule_set) {
        vector<RuleSet> sol;

        Histogram target_hist = target.hist(alphabet_size);

        for (int i = 0; i < alphabet_size; i++) {
            Symbol from_symbol = Symbol(i);
            auto& to_symbols = curr_rule_set[from_symbol];

            Histogram to_hist = to_symbols.hist(alphabet_size);

            for (Symbol added_symbol = AXIOM_SYMBOL + 1; added_symbol <= alphabet_size; added_symbol++) {
                if (to_hist[added_symbol] >= target_hist[added_symbol]) continue;

                RuleSet new_rule_set = curr_rule_set;
                
                Symbols new_to_symbols = to_symbols;
                new_to_symbols.push_back(added_symbol);
                new_rule_set[from_symbol] = new_to_symbols;

                sol.push_back(new_rule_set);
            }
        }

        return sol;
    }

    void gen_next_level() {
        vector<RuleSet> new_rule_sets;

        for (RuleSet& curr_rule_set : results.back()) {
            for (RuleSet new_rule_set : gen_deviants(curr_rule_set)) {
                bool is_unique = true;

                for (RuleSet& rs : new_rule_sets) {
                    is_unique &= rs != new_rule_set;
                }

                if (is_unique) {
                    new_rule_sets.push_back(new_rule_set);
                }
            }
        }

        results.push_back(new_rule_sets);
    }

    pair<bool, RuleSet> find(int depth) {
        if (results.size() == 0) {
            gen_init();
        }

        for (vector<RuleSet> rule_sets : results) {
            for (RuleSet rule_set : rule_sets) {
                if (rule_set.produces(target, depth)) {
                    return { true, rule_set };
                }
            }
        }

        while (results.back().size()) {
            gen_next_level();

            for (RuleSet new_rule_set : results.back()) {
                if (new_rule_set.produces(target, depth)) {
                    return { true, new_rule_set };
                }
            }
        }

        return { false, RuleSet(alphabet_size) };
    }

    vector<RuleSet> collect_results() {
        vector<RuleSet> sol;
        for (auto& rule_sets : results) {
            for (auto& rule_set : rule_sets) {
                sol.push_back(rule_set);
            }
        }
        return sol;
    }
};

pair<SolverStatus, RuleSet> solver(
    int alphabet_size,
    int depth,
    vector<Symbol> target
) {
    RuleGen gen(alphabet_size, target);

    auto [ succ, rule_set ] = gen.find(depth);
    if (!succ) {
        return { SolverStatus::UNSAT_NO_RULESET, RuleSet() };
    }

    return { SolverStatus::SAT, RuleSet() };
}

};