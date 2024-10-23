#include "util.hpp"
#include <queue>

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
        if constexpr (DEBUG) cout << "dup branch\n";
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
                if (!succ) return { idx_offset, false };
            }
            // rule symbols not assigned, try all possible additions.
            else {
                try_adding_symbol(symbol, idx_offset, depth, iter_id);
                return { idx_offset, false };
            }
        }

        // passed on all letters, return true.
        return { idx_offset, true };
    }

    pair<bool, RuleSet> run(int depth) {
        while (branches.size()) {
            if constexpr (DEBUG) cout << "\ntrying next branch:\n";
            id = 0;
            auto [_, succ] = iter(AXIOM_SYMBOL, 0, depth);
            if (succ) {
                return { true, rule_set() };
            }
            branches.pop();
        }
        return { false, RuleSet() };
    }
};

int main() {
    const Symbols alphbet = { '0', '1', '2' };

    const Histograms hists = {
        { AXIOM_SYMBOL, {{'0', 1}, {'1', 1}, {'2', 0}} },
        { '0', {{'0', 0}, {'1', 1}, {'2', 0}} },
        { '1', {{'0', 1}, {'1', 0}, {'2', 1}} },
        { '2', {{'0', 0}, {'1', 1}, {'2', 0}} }
    };

    const int depth = 2;

    vector<Symbols> targets = {
        { '0', '1', '2', '1' },
        { '1', '1', '0', '2' }
    };

    for (auto target : targets) {
        RuleGen gen(alphbet, hists, target);
        auto [succ, rule_set] = gen.run(depth);
        cout << string(rule_set);
        cout << succ << '\n';
    }
}