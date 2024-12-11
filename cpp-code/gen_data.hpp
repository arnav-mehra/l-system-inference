#pragma once

#include "util.hpp"
#include <string>
#include <algorithm>

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
    int alphabet_size;
    RuleSet rule_set;
    int depth;
    Symbols target;

    Data() {}

    Data(int alphabet_size, RuleSet rule_set, int depth, Symbols target)
        : alphabet_size(alphabet_size), rule_set(rule_set), depth(depth), target(target) {}
    
    operator std::string() const {
        string s = "{\n";
        s += "\talphabet: 1-" + to_string(alphabet_size) + "\n";
        s += "\trule set: " + string(rule_set);
        s += "\tdepth: " + to_string(depth) + "\n";
        s += "\ttarget (" + to_string(target.size()) + "): " + string(target) + "\n";
        s += "}\n";
        return s;
    }
};

const int MAX_GEN_ITER = 100;

struct DataGen {
    Range depths;
    Range alphabet_sizes;
    Range complexities;

    DataGen(Range depths, Range alphabet_sizes, Range complexities) 
        : depths(depths), alphabet_sizes(alphabet_sizes), complexities(complexities) {}

    // generates samples (l-system, depth, and target). ensures all rules are used to achieve target (no useless rules).
    // returns [iterations to generate, sample].
    pair<int, Data> gen() {
        for (int i = 1; i <= MAX_GEN_ITER; i++) {
            int depth = depths.gen_uniform_rand();
            int alphabet_size = alphabet_sizes.gen_uniform_rand();
            int complexity = complexities.gen_uniform_rand();

            RuleSet rule_set = gen_rule_set(alphabet_size, complexity);

            Symbols target({ AXIOM_SYMBOL });
            unordered_set<Symbol> seen_symbols; // symbols seen prior to final target == # of rules used.

            for (int i = 0; i <= depth; i++) {
                target = rule_set.apply_to(target);
                if (i != depth) seen_symbols.insert(target.begin(), target.end());
            }

            if (seen_symbols.size() == alphabet_size) { // all rules used.
                return { i, Data(alphabet_size, rule_set, depth, target) };
            }
        }

        return { -1, Data() };
    }

    RuleSet gen_rule_set(int alphabet_size, int complexity) {
        auto alphabet_idx_range = Range(1, alphabet_size); // [first_alphabet_symbol = 1, last_alphabet_symbol = alphabet_size]
        auto symbols_idx_range = Range(0, alphabet_size);  // [axiom_symbol = 0, last_alphabet_symbol = alphabet_size]

        RuleSet rule_set(alphabet_size);

        // generate the random set of symbols, noting there needs to be at least one of each.
        vector<Symbol> rand_symbols;
        for (Symbol symbol = AXIOM_SYMBOL + 1; symbol <= alphabet_size; symbol++) {
            rand_symbols.push_back(symbol);
        }
        for (int i = 0; i < complexity + 1; i++) { // + 1 for the axiom's symbol.
            Symbol symbol = alphabet_idx_range.gen_uniform_rand();
            rand_symbols.push_back(symbol);
        }
        random_shuffle(rand_symbols.begin(), rand_symbols.end());

        // initialize each rule to at least 1 symbol.
        for (Symbol from_symbol = AXIOM_SYMBOL; from_symbol <= alphabet_size; from_symbol++) {
            Symbol init_to_symbol = rand_symbols.back();
            rand_symbols.pop_back();
            rule_set[from_symbol].push_back(init_to_symbol);
        }

        // add a symbol to a random rule for each added complexity.
        for (int i = 0; i < complexity; i++) {
            Symbol from_symbol = symbols_idx_range.gen_uniform_rand();
            Symbol to_symbol = rand_symbols.back();
            rand_symbols.pop_back();
            rule_set[from_symbol].push_back(to_symbol);
        }

        return rule_set;
    }
};