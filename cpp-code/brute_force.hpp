#pragma once

#include "util.hpp"
#include <array>

#define DEBUG false

namespace BF {

struct RuleGen {
    int alphabet_size;
    Symbols target;

    vector<RuleSet> results;
    int last_deviated_idx;

    RuleGen(int alphabet_size, Symbols target)
        : alphabet_size(alphabet_size), target(target) {}

    virtual void add_rule_set(RuleSet& rule_set) {
        results.push_back(rule_set);
    }

    // generate all rule sets of 1 to_symbol per from_symbol.
    // formally: { i -> [x_i] | for all i in [0,|S|], x_i in [1,|S|] }
    void gen_init() {
        vector<RuleSet> curr_rule_sets = { RuleSet(alphabet_size) };

        for (Symbol from_symbol = AXIOM_SYMBOL; from_symbol <= alphabet_size; from_symbol++) {
            vector<RuleSet> new_rule_sets;

            for (RuleSet& old_rule_set : curr_rule_sets) {
                for (Symbol to_symbol = AXIOM_SYMBOL + 1; to_symbol <= alphabet_size; to_symbol++) {
                    RuleSet new_rule_set = old_rule_set;
                    new_rule_set[from_symbol].push_back(to_symbol);
                    new_rule_sets.push_back(new_rule_set);
                }
            }

            curr_rule_sets = new_rule_sets;
        }

        for (RuleSet& rule_set : curr_rule_sets) {
            add_rule_set(rule_set);
        }
        last_deviated_idx = -1;
    }

    // generate all deviants such that a symbol is appended any rule of curr_rule_set
    // and no rule contains more than |target| symbols.
    vector<RuleSet> gen_deviants(RuleSet& curr_rule_set) {
        vector<RuleSet> deviants;

        for (Symbol from_symbol = AXIOM_SYMBOL; from_symbol <= alphabet_size; from_symbol++) {
            auto& to_symbols = curr_rule_set[from_symbol];

            for (Symbol added_symbol = AXIOM_SYMBOL + 1; added_symbol <= alphabet_size; added_symbol++) {
                if (to_symbols.size() > target.size()) continue; // don't generate deviants with rule strings longer than the target.

                RuleSet new_rule_set = curr_rule_set;
                
                Symbols new_to_symbols = to_symbols;
                new_to_symbols.push_back(added_symbol);
                new_rule_set[from_symbol] = new_to_symbols;

                deviants.push_back(new_rule_set);
            }
        }

        if constexpr (DEBUG) {
            // cout << "original:\n";
            // cout << string(curr_rule_set) << '\n';
            // cout << "deviants:\n";
            // for (RuleSet rs : deviants) {
            //     cout << string(rs) << '\n';
            // }
        }

        return deviants;
    }

    void gen_next_level() {
        if (results.size() == 0) {
            // gen initial set of rule sets.
            gen_init();
        }
        else {
            // deviate from next undeviated rule set.
            RuleSet& base_rule_set = results[++last_deviated_idx];
            for (RuleSet& new_rule_set : gen_deviants(base_rule_set)) {
                add_rule_set(new_rule_set);
            }
        }
    }

    // check if there is a rule set to deviate from.
    bool can_deviate() { 
        return results.size() == 0
            || last_deviated_idx + 1 < results.size();
    }

    pair<bool, RuleSet> find(int depth) {
        while (can_deviate()) { // loop until there are no rule sets to deviate from.
            int prior_len = results.size();
            gen_next_level();
            int new_len = results.size();

            if constexpr (DEBUG) {
                // cout << "iter: " << prior_len << ' ' << new_len << '\n';
                // cout << (new_len - prior_len) << ' ';
            }

            for (int i = prior_len; i < new_len; i++) {
                auto& new_rule_set = results[i];
                if (new_rule_set.produces(target, depth)) {
                    return { true, new_rule_set };
                }
            }
        }

        return { false, RuleSet(alphabet_size) };
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

    if (DEBUG) {
        cout << "grammars checked: " << gen.results.size() << "\n";
    }
    return { SolverStatus::SAT, rule_set };
}

};