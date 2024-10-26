#include "util.hpp"
#include <queue>
#include <fstream>
#include <string>

#define DEBUG false

struct RuleGen {
    queue<pair<RuleSet, Histograms>> branches;
    Histogram rule_lengths;
    Symbols target;

    RuleGen(Symbols alphabet, Histograms hists, Symbols target) : target(target) {
        Symbols all_symbols = alphabet;
        all_symbols.push_back(AXIOM_SYMBOL);
        
        // init rule_lengths
        for (Symbol symbol : all_symbols) {
            rule_lengths[symbol] = 0;
            for (auto& [_, cnt] : hists[symbol]) {
                rule_lengths[symbol] += cnt;
            }
            if constexpr (DEBUG) cout << "symbol=" << symbol << " cnt=" << rule_lengths[symbol] << '\n';
        }

        // init first branch
        RuleSet rule_set;
        for (Symbol symbol : all_symbols) {
            rule_set.map[symbol] = {};
        }
        branches.push({rule_set, hists});
    }

    RuleSet& rule_set() {
        return branches.front().first;
    }

    Histograms& hists() {
        return branches.front().second;
    }

    void branch() {
        branches.push({ rule_set(), hists() });
    }

    void try_adding_symbol(const Symbol symbol, int idx_offset, const int depth, int iter_id) {
        auto& hist = hists()[symbol]; // histogram of remaining symbols.
        auto& to_symbols = rule_set().map[symbol]; // current rule for symbol.

        for (auto& [ next_symbol, cnt ] : hist) {
            if (cnt == 0) continue;
            if constexpr (DEBUG) cout << iter_id << ": next_symbol=" << next_symbol << " cnt=" << cnt << '\n'; 

            // add symbol to rule
            to_symbols.push_back(next_symbol);
            hist[next_symbol]--;

            // try added symbol
            auto [ succ, new_idx_offset ] = iter(next_symbol, idx_offset, depth - 1);

            // if it worked, duplicate the current {RuleSet, Histograms} context.
            if (succ) {
                if constexpr (DEBUG) cout << iter_id << ": dup branch\n";
                branch();
            }

            // remove symbol from rule
            to_symbols.pop_back();
            hist[next_symbol]++;
        }
    }

    int id = 0;
    pair<bool, int> iter(const Symbol symbol, int idx_offset, const int depth) {
        auto& to_symbols = rule_set().map[symbol]; // current rule for symbol.
        int needed_symbols = rule_lengths[symbol]; // number of symbols the rule needs to have.

        int iter_id = id++;
        if constexpr (DEBUG) cout << iter_id << ": symbol=" << symbol << " depth=" << depth << " offset=" << idx_offset << " needed_symbols=" << needed_symbols << '\n';

        if (depth == 0) {
            bool matches = target[idx_offset] == symbol;
            if constexpr (DEBUG) cout << iter_id << ": matches=" << matches << "\n";
            return { matches, idx_offset + 1 };
        }

        for (int i = 0; i < needed_symbols; i++) {
            if constexpr (DEBUG) cout << iter_id << ": i=" << i << '\n';

            // rule symbol already assigned, just need to check if its valid.
            if (i < to_symbols.size()) {
                Symbol next_symbol = to_symbols[i];
                auto [ succ, new_idx_offset ] = iter(next_symbol, idx_offset, depth - 1);
                idx_offset = new_idx_offset;
                if (!succ) return { false, idx_offset };
            }
            // rule symbols not assigned, try all possible additions.
            else {
                try_adding_symbol(symbol, idx_offset, depth, iter_id);
                return { false, idx_offset };
            }
        }

        // passed on all letters, return true.
        return { true, idx_offset };
    }

    pair<bool, RuleSet> run(int depth) {
        while (branches.size()) {
            if constexpr (DEBUG) cout << "\ntrying next branch:\n";
            id = 0;
            auto [succ, _] = iter(AXIOM_SYMBOL, 0, depth + 1); // + 1 since axiom is a psuedo-symbol
            if (succ) {
                return { true, rule_set() };
            }
            branches.pop();
        }
        return { false, RuleSet() };
    }
};

void write_inputs(int depth, Symbols target) {
    ofstream dataFile;
    dataFile.open("../outData", std::ios::binary | std::ios::in | std::ios::trunc);

    string str = to_string(depth);
    for (Symbol symbol : target) {
        str += "," + string(1, symbol);
    }

    dataFile << str;
    dataFile.close();
}

pair<bool, Histograms> read_histograms(Symbols alphabet) {
    ifstream dataFile;
    dataFile.open("../outData", std::ios::binary | std::ios::in);

    int dims[2];
    dataFile.read((char*)&dims, sizeof(dims));

    vector<int> data(dims[0] * dims[1]);
    dataFile.read((char*)&data[0], data.size() * 4);

    // for (int x : data) cout << x << ' ';
    // cout << '\n';

    dataFile.close();

    Histograms hists = { {AXIOM_SYMBOL, {}} };
    for (Symbol symbols : alphabet) {
        hists[symbols] = {};
    }

    for (int i = 0; i < dims[0] * dims[1]; i++) {
        int r = i / dims[1];
        int c = i % dims[1];
        
        if (c % 2 == 1) {
            Symbol from_symbol = data[i - c] == -1 ? '*' : data[i - c] + '0';
            Symbol to_symbol = data[i] + '0';
            int cnt = data[i + 1];
            hists[from_symbol][to_symbol] = cnt;
        }
    }

    return { data.size() > 0, hists };
}

int main() {
    // const Symbols alphabet = { '0', '1', '2' };
    // Symbols target = { '0', '2', '1', '1' }; 10132
    Symbols alphabet = { '0', '1', '2', '3' };
    string target_str = "0131013220";
    Symbols target(target_str.begin(), target_str.end());
    const int max_depth = 3;

    for (int depth = max_depth; depth >= 1; depth--) {
        write_inputs(depth, target);
        system("python -u ../python-code/main.py");

        auto [succ1, hists] = read_histograms(alphabet);
        if (!succ1) {
            cout << depth << ": no sat histograms\n";
            continue;
        }

        RuleGen gen(alphabet, hists, target);
        auto [succ2, rule_set] = gen.run(depth);
        if (!succ2) {
            cout << depth << ": no sat rules\n";
            continue;
        }

        cout << depth << ": sat\n";
        cout << string(rule_set);
        break;
    }
}

// Symbols target = { '0', '1', '2', '1' };

// const Histograms hists = {
//     { AXIOM_SYMBOL, {{'0', 1}, {'1', 1}, {'2', 0}} },
//     { '0', {{'0', 0}, {'1', 1}, {'2', 0}} },
//     { '1', {{'0', 1}, {'1', 0}, {'2', 1}} },
//     { '2', {{'0', 0}, {'1', 1}, {'2', 0}} }
// };

// for (auto [ from_symbol, hist ] : hists) {
//     cout << from_symbol << ":\n";
//     for (auto [ to_symbol, cnt ] : hist) {
//         cout << to_symbol << ' ' << cnt << "\n";
//     }
// }