#include "util.hpp"
#include <string>

struct Range {
    int start, end;

    Range(int start, int end) : start(start), end(end) {}

    int gen_uniform_rand() {
        int range = end - start + 1;
        int idx = rand() % range; // [0, end - start]
        return idx + start; // [start, end]
    }

    vector<int> iterable() {
        vector<int> nums;
        for (int i = start; i <= end; i++) {
            nums.push_back(i);
        }
        return nums;
    }
};

struct Data {
    RuleSet rule_set;
    int depth;
    Symbols target;

    Data(RuleSet rule_set, int depth, Symbols target)
        : rule_set(rule_set), depth(depth), target(target) {}
    
    operator std::string() const {
        string s = "{\n";
        s += "rule set: " + string(rule_set);
        s += "depth: " + to_string(depth) + "\n";
        s += "target: " + string(target.begin(), target.end()) + "\n";
        s += "}\n";
        return s;
    }
};

struct DataGen {
    Range depths;
    Range symbol_counts;
    Range complexities;

    DataGen(Range depths, Range symbol_counts, Range complexities) 
        : depths(depths), symbol_counts(symbol_counts), complexities(complexities) {}

    Data gen() {
        int depth = depths.gen_uniform_rand();
        int symbol_count = symbol_counts.gen_uniform_rand();
        int complexity = complexities.gen_uniform_rand();

        RuleSet rule_set = gen_rule_set(symbol_count, complexity);

        Symbols target = { AXIOM_SYMBOL };
        for (int i = 0; i <= depth; i++) {
            target = rule_set.apply_to(target);
        }
        return Data(rule_set, depth, target);
    }

    RuleSet gen_rule_set(int symbol_count, int complexity) {
        Symbols symbols = { AXIOM_SYMBOL };
        for (int i = 0; i < symbol_count; i++) {
            symbols.push_back(Symbol(i + '0'));
        }

        RuleSet rule_set;
        for (Symbol symbol : symbols) {
            int init_to_symbol_idx = Range(1, symbols.size() - 1).gen_uniform_rand();
            Symbol init_to_symbol = symbols[init_to_symbol_idx];
            rule_set.map[symbol] = { init_to_symbol };
        }
        for (int i = 0; i < complexity; i++) {
            Symbol from_symbol_idx = Range(1, symbols.size() - 1).gen_uniform_rand();
            Symbol to_symbol_idx = Range(1, symbols.size() - 1).gen_uniform_rand();
            auto from_symbol = symbols[from_symbol_idx];
            auto to_symbol = symbols[to_symbol_idx];
            rule_set.map[from_symbol].push_back(to_symbol);
        }
        return rule_set;
    }
};

int main() {
    auto depth_range = Range(4, 4);
    auto symbol_range = Range(4, 4);
    auto complexity_range = Range(3, 3);
    auto data_gen = DataGen(depth_range, symbol_range, complexity_range);
    cout << string(data_gen.gen());
}