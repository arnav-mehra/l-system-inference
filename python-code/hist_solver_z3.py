from enum import Enum
from collections import Counter
from z3 import *

IN_FILE = "../inData"
OUT_FILE = "../outData"

def read_inputs():
    target_hist = []
    depth = 0
    timeout = 0

    import csv
    with open(IN_FILE, 'r') as fd:
        reader = csv.reader(fd)
        for row in reader:
            for idx, x in enumerate(row):
                if idx == 0:
                    timeout = int(x)
                if idx == 1:
                    depth = int(x)
                else:
                    target_hist.append(int(x))

    # print(target_hist, depth, timeout)
    return target_hist, depth, timeout

def write_outputs(status, axiom_hist, rule_hists):
    result = [status]
    if status == 3:
        rule_hist_flat = [item for sublist in rule_hists for item in sublist]
        result += axiom_hist + rule_hist_flat
    result_string = ",".join([str(x) for x in result])
    # print(result_string)

    try:
        outputFile = open(OUT_FILE, 'w')
        outputFile.write(result_string)
        outputFile.close()
    except:
        print("Failed to write to file buffer, outData!\n")

def get_mat_id(m: str, from_symbol: int, to_symbol: int):
    return Int(m + "[" + str(from_symbol) + "]" + "[" + str(to_symbol) + "]")

def mat_mult(symbols: list, m1: str, m2: str, m3: str):
    conds = []
    for row_symbol in symbols:
        for col_symbol in symbols:
            mult_ops = []
            for iter_symbol in symbols:
                m1_ri = get_mat_id(m1, row_symbol, iter_symbol)
                m2_ic = get_mat_id(m2, iter_symbol, col_symbol)
                mult_ops.append(m1_ri * m2_ic)
            dot_prod = Sum(mult_ops)

            result_rc = get_mat_id(m3, row_symbol, col_symbol)
            conds.append(result_rc == dot_prod)
    return And(conds)

def mat_vec_mult(symbols: list, vi: str, m: str, vf: str):
    conds = []
    for col_symbol in symbols:
        mult_ops = []
        for iter_symbol in symbols:
            vi_i = get_mat_id(vi, 0, iter_symbol)
            m_ri = get_mat_id(m, iter_symbol, col_symbol)
            mult_ops.append(m_ri * vi_i)
        dot_prod = Sum(mult_ops)

        vf_c = get_mat_id(vf, 0, col_symbol)
        conds.append(vf_c == dot_prod)
    return And(conds)

def vec_eq(symbols: list, v1: str, v2: str):
    conds = []
    for iter_symbol in symbols:
        v1_i = get_mat_id(v1, 0, iter_symbol)
        v2_i = get_mat_id(v2, 0, iter_symbol)
        conds.append(v1_i == v2_i)
    return And(conds)

def hist_eq(symbol_hist: list, v: str):
    hist_conds = []
    for symbol_idx, count in enumerate(symbol_hist):
        symbol = symbol_idx + 1
        v_i = get_mat_id(v, 0, symbol)
        hist_conds.append(v_i == count)
    return And(hist_conds)

def cost(symbols: list, m: str, v: str):
    vars = []
    for row_symbol in symbols:
        for col_symbol in symbols:
            m_rc = get_mat_id(m, row_symbol, col_symbol)
            vars.append(m_rc)
    for symbol in symbols:
        v_r = get_mat_id(v, 0, symbol)
        vars.append(v_r)
    return Sum(vars)

# note: to use m_prefix + "^1" should already exist.
def mat_pow(symbols: list, m_prefix: str, p: int):
    if p == 1:
        return And([])

    conds = []

    # compute m^(p/2)
    p_half = p // 2
    m_p_half_cond = mat_pow(symbols, m_prefix, p_half)
    conds.append(m_p_half_cond)

    # m^(p/2*2) = m^(p/2) * m^(p/2)
    m_p_half = m_prefix + "^" + str(p_half)
    m_p = m_prefix + "^" + str(p_half * 2)
    m_p_cond = mat_mult(symbols, m_p_half, m_p_half, m_p)
    conds.append(m_p_cond)

    # m^p = m^(p/2) * m^(p/2) * m^1 (if p is odd)
    if p_half * 2 != p:
        m_1 = m_prefix + "^1"
        m_p_adj = m_prefix + "^" + str(p)
        m_p_adj_cond = mat_mult(symbols, m_p, m_1, m_p_adj)
        conds.append(m_p_adj_cond)

    return And(conds)

def solve(target_hist: list, p: int, timeout: int):
    symbols = range(1, len(target_hist) + 1)
    target_len = sum(target_hist)
    new_conds = []

    # m^p
    m_p = "m^" + str(p)
    mat_mult_cond = mat_pow(symbols, "m", p)
    new_conds.append(mat_mult_cond)

    # vf = vi * m^p
    mat_vec_mult_cond = mat_vec_mult(symbols, "vi", m_p, "vf")
    new_conds.append(mat_vec_mult_cond)
    # print(mat_vec_mult_cond)

    # vf = histogram(symbols)
    hist_cond = hist_eq(target_hist, "vf")
    new_conds.append(hist_cond)

    # m^1[r][c] >= 0
    for from_symbol in symbols:
        for to_symbol in symbols:
            cell_id = get_mat_id("m^1", from_symbol, to_symbol)
            new_conds.append(cell_id >= 0)

    # vi[r] >= 0
    for symbol in symbols:
        cell_id = get_mat_id("vi", 0, symbol)
        new_conds.append(cell_id >= 0)

    # sum(m^1[r][:]) >= 1. each symbol must map to a string of size >= 1. 
    for from_symbol in symbols:
        terms = []
        for to_symbol in symbols:
            cell_id = get_mat_id("m^1", from_symbol, to_symbol)
            terms.append(cell_id)
        to_sum = Sum(terms)
        new_conds.append(to_sum >= 1)

    # sum(m^1[:][c]) >= 1. each symbol needs to be mapped to >= 1.
    for to_symbol in symbols:
        terms = []
        for from_symbol in symbols:
            cell_id = get_mat_id("m^1", from_symbol, to_symbol)
            terms.append(cell_id)
        from_sum = Sum(terms)
        new_conds.append(from_sum >= 1)

    # cost = sum_rc(m^1[r][c]) + sum_r(v[r])
    cost_expr = cost(symbols, "m^1", "vi")

    solver = Optimize()
    solver.set("timeout", timeout * 1000)
    solver.add(new_conds)
    solver.minimize(cost_expr)

    if solver.check() == sat:
        model = solver.model()
        # print(model)
        
        rule_hists = []
        for from_symbol in symbols:
            hist = []
            for to_symbol in symbols:
                m1_rc = get_mat_id("m^1", from_symbol, to_symbol)
                cnt = model[m1_rc].as_long()
                hist.append(cnt)
            rule_hists.append(hist)

        axiom_hist = []
        for symbol in symbols:
            v_rc = get_mat_id("vi", 0, symbol)
            cnt = model[v_rc].as_long()
            axiom_hist.append(cnt)

        return 3, axiom_hist, rule_hists
    elif solver.statistics().get_key_value('time') >= timeout:
        return 2, None, None
    else:
        return 0, None, None

if __name__ == '__main__':
    target_hist, depth, timeout = read_inputs()
    # print(target_hist, depth)

    status, axiom_hist, rule_hists = solve(target_hist, depth, timeout)
    # print(axiom_hist)
    # print(rule_hists)

    write_outputs(status, axiom_hist, rule_hists)