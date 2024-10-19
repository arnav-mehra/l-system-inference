#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <iostream>
using namespace std;

typedef char Symbol; 
typedef vector<Symbol> Symbols;

struct Rules {
    unordered_map<Symbol, Symbols> rules;

    Symbols apply_to(Symbols curr) {
        Symbols new_symbols;

        for (Symbol from_symbol : curr) {
            for (Symbol to_symbol : rules[from_symbol]) {
                new_symbols.push_back(to_symbol);
            }
        }

        return new_symbols;
    }

    operator std::string() const {
        string s = "[\n";
        for (auto& p : rules) {
            s += "\t" + p.first;
            s += " -> [";
            for (Symbol to_symbol : p.second) {
                s += to_symbol;
                if (&to_symbol != &p.second.back()) s += ", ";
            }
            s += "]\n";
        }
        s += "]\n";

        return s;
    }
};

bool is_valid(Symbols& goal, Symbols& axiom, Rules& rules) {
    Symbols curr = axiom;
    while (curr.size() < goal.size()) {
        curr = rules.apply_to(curr);
    }
    return curr == axiom;
}

vector<Rules> gen_next(Rules& curr, Symbols& symbol_set) {
    vector<Rules> sol;

    for (auto& p : curr.rules) {
        Symbol key_symbol = p.first;

        for (Symbol added_symbol : symbol_set) {
            Rules new_rules = curr;
            Symbols to_symbols = p.second;
            to_symbols.push_back(added_symbol);
            new_rules.rules[key_symbol] = to_symbols;

            sol.push_back(new_rules);
        }
    }

    return sol;
}

int main() {
    vector<char> goal = {'0','1'};

    Symbols symbol_set(goal.begin(), unique(goal.begin(), goal.end()));
    unordered_set<Symbol> (goal.begin(), goal.end());

    for (auto x: goal) cout << x <<' ';
    cout << "\n";
    for (auto x: symbol_set) cout << x <<' ';
    cout << "\n";

    Rules initial_rules;
    for (Symbol s : symbol_set) {
        initial_rules.rules[s] = {};
    }

    for (Rules rules : gen_next(initial_rules, symbol_set)) {
        cout << string(rules);
    }
}