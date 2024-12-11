#pragma once

#include "brute_force.hpp"

#define DEBUG false

namespace BFP {

struct RuleGen : BF::RuleGen {
    RuleSetHashSet seen;
    Histogram target_hist;

    RuleGen(int alphabet_size, int depth, Symbols target)
        : BF::RuleGen(alphabet_size, depth, target), target_hist(alphabet_size) {
        target_hist.digest(target);
    }

    void add_rule_set(RuleSet& new_rule_set) override {
        // prune transpositions
        if (seen.find(new_rule_set) != seen.end()) {
            return;
        }
        seen.insert(new_rule_set);

        // prune for each substring sequence.
        Symbols pre_fin_str({ AXIOM_SYMBOL });
        for (int i = 1; i <= depth - 1; i++) {
            pre_fin_str = new_rule_set.apply_to(pre_fin_str);
        }
        auto target_offset = target.begin();
        for (int i = 0; i < pre_fin_str.size(); i++) {
            Symbol from_symbol = pre_fin_str[i];
            Symbols& goal = new_rule_set[from_symbol];
            auto new_iter = search(target_offset, target.end(), goal.begin(), goal.end());

            if (i == 0) {
                if (new_iter != target.begin()) {
                    return;
                }
            } else {
                if (new_iter == target.end()) {
                    return;
                }
            }

            target_offset = new_iter;
            target_offset += goal.size();
        }

        results.push_back(new_rule_set);
    }
};

pair<SolverStatus, RuleSet> solver(
    int alphabet_size,
    int depth,
    vector<Symbol> target,
    int timeout
) {
    RuleGen gen(alphabet_size, depth, target);

    auto [ succ, rule_set ] = gen.find(timeout);
    if (!succ) {
        return { SolverStatus::UNSAT_NO_RULESET, RuleSet() };
    }

    if (DEBUG) {
        cout << "grammars checked: " << gen.results.size() << "\n";
    }
    return { SolverStatus::SAT, rule_set };
}

};
