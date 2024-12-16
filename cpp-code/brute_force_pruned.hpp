#pragma once

#include "brute_force.hpp"

namespace BFP {

struct RuleGen : BF::RuleGen {
    RuleSetHashSet seen;
    Histogram target_hist;

    RuleGen(int alphabet_size, int depth, Symbols target)
        : BF::RuleGen(alphabet_size, depth, target),
        target_hist(Histogram(alphabet_size).digest(target)) {}

    bool is_transposition(RuleSet& new_rule_set) {
        // prune transpositions
        if (seen.find(new_rule_set) != seen.end()) {
            return true;
        }
        seen.insert(new_rule_set);
        return false;
    }

    bool is_sterile(RuleSet& new_rule_set) {
        // prune for each substring sequence.
        Symbols pre_fin_str({ AXIOM_SYMBOL });
        for (int i = 0; i <= depth - 1; i++) {
            pre_fin_str = new_rule_set.apply_to(pre_fin_str);
        }
        auto target_offset = target.begin();
        for (int i = 0; i < pre_fin_str.size(); i++) {
            Symbol from_symbol = pre_fin_str[i];
            Symbols& goal = new_rule_set[from_symbol];
            auto new_iter = search(target_offset, target.end(), goal.begin(), goal.end());

            if (i == 0) {
                if (new_iter != target.begin()) {
                    return true;
                }
            } else {
                if (new_iter == target.end()) {
                    return true;
                }
            }

            target_offset = new_iter;
            target_offset += goal.size();
        }

        return false;
    }

    virtual void add_result(RuleSet& new_rule_set) override {
        if (is_transposition(new_rule_set)) return;
        if (is_sterile(new_rule_set)) return;

        results.push_back(new_rule_set);
    }
};

};
