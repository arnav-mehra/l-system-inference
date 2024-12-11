#pragma once

#include "brute_force.hpp"

namespace BFPruned {

struct RuleGen : BF::RuleGen {
    int depth;

    RuleSetHashSet seen;
    Histogram target_hist;

    RuleGen(int alphabet_size, int depth, Symbols target)
        : BF::RuleGen(alphabet_size, target), depth(depth), target_hist(alphabet_size) {
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
    vector<Symbol> target
) {
    RuleGen gen(alphabet_size, depth, target);

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

// // prune prefix mismatches
// Symbol str_first_symbol = AXIOM_SYMBOL;
// for (int i = 1; i <= depth - 1; i++) {
//     str_first_symbol = new_rule_set[str_first_symbol][0];
// }
// Symbols& final_rule_string = new_rule_set[str_first_symbol];
// if (!final_rule_string.is_at(target, 0)) {
//     return;
// }

// // prune symbol overuse
// Symbols str; str.push_back(AXIOM_SYMBOL);
// for (int i = 1; i <= depth; i++) {
//     str = new_rule_set.apply_to(str);
// }
// Histogram str_hist(alphabet_size); str_hist.digest(str);
// for (Symbol to_symbol = AXIOM_SYMBOL + 1; to_symbol <= alphabet_size; to_symbol++) {
//     if (str_hist[to_symbol] > target_hist[to_symbol]) {
//         return;
//     }
// }