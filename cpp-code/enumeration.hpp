#include "util.hpp"

template<class ResultType>
struct Enumerator {
    int depth;
    int alphabet_size;
    Symbols target;

    vector<ResultType> results;
    int last_deviated_idx;

    int checks = 0;

    Enumerator(int alphabet_size, int depth, Symbols target)
        : alphabet_size(alphabet_size), depth(depth), target(target) {}

    virtual void add_result(ResultType& result) {
        results.push_back(result);
    }

    virtual pair<SolverStatus, RuleSet> check_result(ResultType& result, Timer& timer) {
        return { SolverStatus::UNSAT_NO_RULESET, RuleSet(alphabet_size) };
    }

    virtual vector<ResultType> gen_init() {
        return {};
    }

    virtual vector<ResultType> gen_deviants(ResultType& curr_rule_set) {
        return {};
    }

    virtual void push_stats() {
        if (DEBUG) {
            cout << "grammars checked: " << checks << "\n";
        }
        misc_buffer.push_back(checks);
    }

    void gen_next_level() {
        if (results.size() == 0) {
            // gen initial set of rule sets.
            last_deviated_idx = -1;
            for (ResultType& result : gen_init()) {
                add_result(result);
            }
        }
        else {
            // deviate from next undeviated rule set.
            ResultType& base_result = results[++last_deviated_idx];
            for (ResultType& new_result : gen_deviants(base_result)) {
                add_result(new_result);
            }
        }
    }

    // check if there is a rule set to deviate from.
    bool can_deviate() { 
        return results.size() == 0
            || last_deviated_idx + 1 < results.size();
    }

    pair<SolverStatus, RuleSet> find(int timeout) {
        auto timer = Timer(timeout).start();

        while (can_deviate() && !timer.timed_out()) { // loop until there are no rule sets to deviate from.
            int prior_len = results.size();
            gen_next_level();
            int new_len = results.size();

            for (int i = prior_len; i < new_len; i++) {
                checks++;
                auto [succ, new_rule_set] = check_result(results[i], timer);
                if (succ == SolverStatus::SAT) {
                    push_stats();
                    return { SolverStatus::SAT, new_rule_set };
                }
            }
        }

        if (timer.timed_out()) {
            push_stats();
            return { SolverStatus::UNSAT_TIMEOUT, RuleSet() };
        }

        push_stats();
        return { SolverStatus::UNSAT_NO_RULESET, RuleSet() };
    }
};

template<class EnumeratorType>
pair<SolverStatus, RuleSet> solver(
    int alphabet_size,
    int depth,
    vector<Symbol> target,
    int timeout
) {
    return EnumeratorType(alphabet_size, depth, target).find(timeout);
}