#pragma once

#include "enumeration.hpp"
#include <array>

namespace BF {

struct RuleGen : Enumerator<RuleSet> {
    RuleGen(int alphabet_size, int depth, Symbols target)
        : Enumerator<RuleSet>(alphabet_size, depth, target) {}

    virtual void add_result(RuleSet& rule_set) override {
        results.push_back(rule_set);
    }

    pair<SolverStatus, RuleSet> check_result(RuleSet& result, Timer& timer) override {
        auto status = result.produces(target, depth) ? SolverStatus::SAT : SolverStatus::UNSAT_NO_RULESET;
        return { status, result };
    }

    // generate all rule sets of 1 to_symbol per from_symbol.
    // formally: { i -> [x_i] | for all i in [0,|S|], x_i in [1,|S|] }
    vector<RuleSet> gen_init() override {
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

        return curr_rule_sets;
    }

    // generate all deviants such that a symbol is appended any rule of curr_rule_set
    // and no rule contains more than |target| symbols.
    vector<RuleSet> gen_deviants(RuleSet& curr_rule_set) override {
        vector<RuleSet> deviants;

        for (Symbol from_symbol = AXIOM_SYMBOL; from_symbol <= alphabet_size; from_symbol++) {
            auto& to_symbols = curr_rule_set[from_symbol];

            for (Symbol added_symbol = AXIOM_SYMBOL + 1; added_symbol <= alphabet_size; added_symbol++) {
                if (to_symbols.size() >= target.size()) continue; // don't generate deviants with rule strings longer than the target.

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
};

};