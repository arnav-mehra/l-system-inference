#include <iostream>
#include <Eigen/Dense>
#include <unsupported/Eigen/MatrixFunctions>
#include "util.hpp"
#include "brute_force_pruned.hpp"

namespace BFH {

Eigen::MatrixXi mat_pow(Eigen::MatrixXi mat, int pow) {
    if (pow == 1) return mat;
    Eigen::MatrixXi res_sqrt = mat_pow(mat, pow / 2);
    Eigen::MatrixXi res = res_sqrt * res_sqrt;
    return pow % 2 ? res * mat : res;
}

struct RuleHist {
    Eigen::MatrixXi rule_mat;
    Eigen::VectorXi axiom_vec;

    RuleHist(int alphabet_size)
        : rule_mat(alphabet_size, alphabet_size)
        , axiom_vec(alphabet_size) {
        rule_mat.fill(0);
        axiom_vec.fill(0);
    }

    RuleHist(Eigen::MatrixXi rule_mat, Eigen::VectorXi axiom_vec)
        : rule_mat(rule_mat), axiom_vec(axiom_vec) {}

    Eigen::VectorXi compute_target_vec(int depth) {
        return mat_pow(rule_mat.transpose(), depth) * axiom_vec;
    }

    bool operator==(const RuleHist& other) const {
        return axiom_vec == other.axiom_vec
            && rule_mat == other.rule_mat;
    }

    operator string() const {
        stringstream ss;
        ss << "axiom:\n"   << axiom_vec
           << "\nrules:\n" << rule_mat << '\n';
        return ss.str();
    }
};

struct RuleOrder : BFP::RuleGen {
    Histograms rule_hist;

    RuleOrder(int alphabet_size, int depth, Symbols target, Histograms rule_hist)
        : BFP::RuleGen(alphabet_size, depth, target),
        rule_hist(rule_hist) {}

    bool is_over(RuleSet& new_rule_set) {
        // prune for hists
        for (Symbol from_symbol = AXIOM_SYMBOL; from_symbol <= alphabet_size; from_symbol++) {
            auto new_rule_set_hist = Histogram(alphabet_size).digest(new_rule_set[from_symbol]);

            for (Symbol to_symbol = AXIOM_SYMBOL; to_symbol <= alphabet_size; to_symbol++) {
                if (new_rule_set_hist[to_symbol] > rule_hist[from_symbol][to_symbol]) {
                    return true;
                }
            }
        }
        return false;
    }

    void add_result(RuleSet& new_rule_set) override {
        if (is_transposition(new_rule_set)) return;
        if (is_over(new_rule_set)) return;
        if (is_sterile(new_rule_set)) return;

        results.push_back(new_rule_set);
    }
};

struct RuleHistHash {
    size_t operator()(const RuleHist& rule_hist) const {
        size_t seed = 0;
        hash<int> hasher;
        int n = rule_hist.axiom_vec.size();
        for (int i = 0; i < n; i++) {
            int x = rule_hist.axiom_vec(i);
            seed ^= hasher(x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int x = rule_hist.rule_mat(i, j);
                seed ^= hasher(x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
        }
        return seed;
    }
};

struct RuleGen : Enumerator<RuleHist> {
    Eigen::VectorXi target_vec;

    unordered_set<RuleHist, RuleHistHash> seen;

    RuleGen(int alphabet_size, int depth, Symbols target)
        : Enumerator<RuleHist>(alphabet_size, depth, target), target_vec(alphabet_size) {
        auto hist = Histogram(alphabet_size).digest(target);
        for (int i = 0; i < alphabet_size; i++) {
            target_vec(i) = hist[Symbol(i + 1)];
        }
    }

    bool is_transposition(RuleHist& rule_hist) {
        if (seen.find(rule_hist) != seen.end()) {
            return true;
        }
        seen.insert(rule_hist);
        return false;
    }

    bool is_over(RuleHist& rule_hist) {
        auto curr_target_vec = rule_hist.compute_target_vec(depth);
        for (int i = 0; i < alphabet_size; i++) {
            if (curr_target_vec(i) > target_vec(i)) {
                return true;
            }
        }
        return false;
    }

    void add_result(RuleHist& rule_hist) override {
        if (is_transposition(rule_hist)) return;
        if (is_over(rule_hist)) return;

        results.push_back(rule_hist);
    }

    pair<SolverStatus, RuleSet> check_result(RuleHist& rule_hist, Timer& timer) override {
        auto curr_target_vec = rule_hist.compute_target_vec(depth);
        if (target_vec != curr_target_vec) {
            return { SolverStatus::UNSAT_NO_HIST, RuleSet() };
        }

        Histograms hists(alphabet_size);
        for (int i = 0; i < alphabet_size; i++) {
            hists[AXIOM_SYMBOL][Symbol(i + 1)] = rule_hist.axiom_vec(i);
        }
        for (int i = 0; i < alphabet_size; i++) {
            for (int j = 0; j < alphabet_size; j++) {
                hists[Symbol(i + 1)][Symbol(j + 1)] = rule_hist.rule_mat(i, j);
            }
        }

        auto rule_order = RuleOrder(alphabet_size, depth, target, hists);
        auto res = rule_order.find((int)timer.time_left());
        checks += rule_order.results.size();
        return res;
    }

    // generate all rule sets of 1 to_symbol per from_symbol.
    // formally: { i -> [x_i] | for all i in [0,|S|], x_i in [1,|S|] }
    vector<RuleHist> gen_init() override {
        vector<RuleHist> curr_results = {};

        for (int i = 0; i < alphabet_size; i++) {
            auto base = RuleHist(alphabet_size);
            base.axiom_vec(i)++;
            curr_results.push_back(base);
        }

        for (int i = 0; i < alphabet_size; i++) {
            vector<RuleHist> new_results;

            for (auto& curr_result : curr_results) {
                for (int j = 0; j < alphabet_size; j++) {
                    auto new_result = curr_result;
                    new_result.rule_mat(i, j)++;
                    new_results.push_back(new_result);
                }
            }

            curr_results = new_results;
        }

        return curr_results;
    }

    // generate all deviants such that a symbol is appended any rule of curr_rule_set
    // and no rule contains more than |target| symbols.
    vector<RuleHist> gen_deviants(RuleHist& current) override {
        vector<RuleHist> deviants;

        for (int i = 0; i < alphabet_size; i++) {
            auto new_axiom_vec = current.axiom_vec;
            new_axiom_vec(i)++;
            deviants.push_back(RuleHist(current.rule_mat, new_axiom_vec));
        }

        for (int i = 0; i < alphabet_size; i++) {
            for (int j = 0; j < alphabet_size; j++) {
                auto new_rule_mat = current.rule_mat;
                new_rule_mat(i, j)++;
                deviants.push_back(RuleHist(new_rule_mat, current.axiom_vec));
            }
        }

        return deviants;
    }
};

};


// struct RuleOrder {
//     const int depth;
//     const int alphabet_size;
//     const Symbols target;

//     int target_idx = 0;
//     RuleSet rule_set;
//     Histograms rule_hists;

//     RuleSet solution;

//     RuleOrder(int depth, int alphabet_size, Histograms rule_hists, Symbols target)
//         : depth(depth),
//         alphabet_size(alphabet_size),
//         rule_set(alphabet_size),
//         rule_hists(rule_hists),
//         target(target) {}

//     bool all_assigned() {
//         return all_of(rule_set.begin(), rule_set.end(), [](Symbols& v) { return v.size() > 0; });
//     }

//     bool hist_matches_target(Symbol s) {
//         Histogram& hist = rule_hists[s];

//         // cout << "hist_matches_target(): "<< s <<'\n';
//         // not enough symbols remaining in target
//         int length = hist.sum();
//         if (target_idx + length > target.size()) {
//             return false;
//         }

//         auto start_iter = target.begin() + target_idx;
//         auto end_iter = target.begin() + target_idx + length;
//         auto target_hist = Histogram(alphabet_size).digest(start_iter, end_iter);
//         // cout << "target_hist: "<< string(target_hist) <<'\n';
//         if (target_hist != hist) {
//             return false;
//         }

//         rule_set[s] = Symbols(start_iter, end_iter);
//         target_idx += length;
//         auto res = infer_rules();
//         target_idx -= length;
//         rule_set[s] = {};
//         return res;
//     }

//     bool rule_matches_target(Symbol s) {
//         Symbols& successor = rule_set[s];

//         // not enough symbols remaining in target
//         int length = successor.size();
//         if (target_idx + length > target.size()) {
//             return false;
//         }

//         auto start_iter = target.begin() + target_idx;
//         auto end_iter = target.begin() + target_idx + length;
//         if (!equal(start_iter, end_iter, successor.begin())) {
//             return false;
//         }

//         target_idx += length;
//         auto res = infer_rules();
//         target_idx -= length;
//         return res;
//     }

//     bool infer_axiom() {
//         auto axiom_hist = rule_hists[AXIOM_SYMBOL];

//         if (axiom_hist.sum() == 0) {
//             if (rule_set.produces(target, depth)) {
//                 solution = rule_set;
//                 return true;
//             }
//             return false;
//         }

//         Symbols lstring({ AXIOM_SYMBOL });
//         for (int i = 0; i <= depth; i++) {
//             lstring = rule_set.apply_to(lstring);
//         }
//         if (!lstring.is_at((Symbols)target, 0)) {
//             return false;
//         }

//         for (Symbol s = AXIOM_SYMBOL + 1; s <= alphabet_size; s++) {
//             if (axiom_hist[s] == 0) continue;

//             axiom_hist[s]--;
//             rule_set[AXIOM_SYMBOL].push_back(s);
//             auto res = infer_axiom();
//             rule_set[AXIOM_SYMBOL].pop_back();
//             axiom_hist[s]++;

//             if (res) return true;
//         }

//         return false;
//     }

//     bool infer_rules() {
//         // cout << "\ninfer_rules():\n";
//         // cout << "rule_hists: " << string(rule_hists) << '\n'; 
//         // cout << "rule_set: " << string(rule_set) << '\n';
//         // cout << "target_idx: " << target_idx << '\n';

//         if (target_idx == target.size()) {
//             return true;
//         }

//         if (all_assigned()) {
//             // check if satisfying axiom exists.
//             return infer_axiom();
//         }

//         for (Symbol s = AXIOM_SYMBOL + 1; s <= alphabet_size; s++) {
//             auto is_assigned_rule = rule_set[s].size() > 0;
//             cout << "target_idx: " << target_idx << '\n';
//             if (is_assigned_rule) {
//                 cout << "successor: " << string(rule_set[s]) << '\n';
//                 if (rule_matches_target(s)) {
//                     cout << "matched: " << 1 << '\n';
//                     return true;
//                 }
//                 cout << "matched: " << 0 << '\n';
//             }
//             else {
//                 cout << "successor_hist: " << string(rule_hists[s]) << '\n';
//                 if (hist_matches_target(s)) {
//                     cout << "matched: " << 1 << '\n';
//                     return true;
//                 }
//                 cout << "matched: " << 0 << '\n';
//             }
//         }

//         return false;
//     }
// };
