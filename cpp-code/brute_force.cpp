#include "util.hpp"

struct RuleGen {
    Symbols symbol_set;
    vector<vector<RuleSet>> results;

    RuleGen(Symbols goal) {
       symbol_set = Symbols(goal.begin(), unique(goal.begin(), goal.end()));
    }

    void gen_init() {
        Symbols from_symbol_set = symbol_set;
        from_symbol_set.push_back(AXIOM_SYMBOL);

        vector<RuleSet> curr_rule_sets = { RuleSet() };
        for (auto from_symbol : from_symbol_set) {
            vector<RuleSet> new_rule_sets;
            for (RuleSet& old_rule_set : curr_rule_sets) {
                for (auto to_symbol : symbol_set) {
                    RuleSet new_rule_set = old_rule_set;
                    new_rule_set.map[from_symbol] = {to_symbol};
                    new_rule_sets.push_back(new_rule_set);
                }
            }
            curr_rule_sets = new_rule_sets;
        }

        results = { curr_rule_sets };
    }

    vector<RuleSet> gen_deviants(RuleSet& curr_rule_set) {
        vector<RuleSet> sol;

        for (auto& [ from_symbol, to_symbols ] : curr_rule_set.map) {
            for (Symbol added_symbol : symbol_set) {
                RuleSet new_rule_set = curr_rule_set;
                
                Symbols new_to_symbols = to_symbols;
                new_to_symbols.push_back(added_symbol);
                new_rule_set.map[from_symbol] = new_to_symbols;

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
                    is_unique &= rs.map != new_rule_set.map;
                }

                if (is_unique) {
                    new_rule_sets.push_back(new_rule_set);
                }
            }
        }

        results.push_back(new_rule_sets);
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

int main() {
    Symbols goal = {'0','1'};

    RuleGen gen(goal);

    gen.gen_init();
    while (gen.results.size() != 3) {
        gen.gen_next_level();
    }
    
    vector<RuleSet> res = gen.collect_results(); 

    for (auto& rule_set : res) {
        if (rule_set.produces(goal)) {
            cout << string(rule_set);
        }
    }
}