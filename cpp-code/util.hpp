#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>
using namespace std;

typedef char Symbol;
typedef vector<Symbol> Symbols;

typedef unordered_map<Symbol, int> Histogram;
typedef unordered_map<Symbol, Histogram> Histograms;

constexpr Symbol AXIOM_SYMBOL = Symbol('*');

struct RuleSet {
    unordered_map<Symbol, Symbols> map;

    Symbols apply_to(Symbols curr) {
        Symbols new_symbols;

        for (Symbol from_symbol : curr) {
            for (Symbol to_symbol : map[from_symbol]) {
                new_symbols.push_back(to_symbol);
            }
        }

        return new_symbols;
    }

    bool produces(Symbols& goal) {
        Symbols curr = { Symbol(AXIOM_SYMBOL) };

        while (curr.size() <= goal.size()) {
            int prev_len = curr.size();
            curr = apply_to(curr);

            // stuck at length.
            if (prev_len == curr.size()) {
                unordered_set<string> seen;
                while (prev_len == curr.size()) {
                    if (curr == goal) return true;

                    string new_str = string(curr.begin(), curr.end());
                    if (seen.count(new_str)) return false;
                    seen.insert(new_str);

                    curr = apply_to(curr);
                }
            }
        }

        return false;
    }

    operator std::string() const {
        string s = "[\n";
        for (auto [ from_symbol, to_symbols ] : map) {
            s += "\t";
            s += from_symbol;
            s += " -> [";
            for (Symbol& to_symbol : to_symbols) {
                s += to_symbol;
                if (&to_symbol != &to_symbols.back()) s += ", ";
            }
            s += "]\n";
        }
        s += "]\n";

        return s;
    }
};