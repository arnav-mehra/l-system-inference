from enum import Enum
from collections import Counter
from z3 import *

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

def mat_vec_mult(symbols: list, m1: str, v1: str, v2: str):
    conds = []
    for row_symbol in symbols:
        mult_ops = []
        for iter_symbol in symbols:
            m1_ri = get_mat_id(m1, row_symbol, iter_symbol)
            v1_i0 = get_mat_id(v1, iter_symbol, 0)
            mult_ops.append(m1_ri * v1_i0)
        dot_prod = Sum(mult_ops)

        result_rc = get_mat_id(v2, row_symbol, 0)
        conds.append(result_rc == dot_prod)
    return And(conds)

def vec_eq(symbols: list, v1: str, v2: str):
    conds = []
    for iter_symbol in symbols:
        v1_i0 = get_mat_id(m1, iter_symbol, 0)
        v2_i0 = get_mat_id(v1, iter_symbol, 0)
        conds.append(v1_i0 == v2_i0)
    return And(conds)

def hist_eq(symbol_hist: Counter, v: str):
    hist_conds = []
    for symbol in symbol_hist:
        count = symbol_hist[symbol]
        v1_i0 = get_mat_id(v, symbol, 0)
        hist_conds.append(v1_i0 == count)
    return And(hist_conds)

def cost(symbols: list, m: str, v: str):
    vars = []
    for row_symbol in symbols:
        for col_symbol in symbols:
            m_rc = get_mat_id(m, row_symbol, col_symbol)
            vars.append(m_rc)
    for symbol in symbols:
        v_r0 = get_mat_id(v, symbol, 0)
        vars.append(v_r0)
    return Sum(vars)

def solve(goal: list):
    symbols = list(set(goal))
    symbol_hist = Counter(goal)

    new_conds = []

    # m1 = m0^2
    mat_mult_cond = mat_mult(symbols, "m0", "m0", "m1")
    new_conds.append(mat_mult_cond)

    # v1 = m1 * v0
    mat_vec_mult_cond = mat_vec_mult(symbols, "m1", "v0", "v1")
    new_conds.append(mat_vec_mult_cond)

    # v1 = histogram(symbols)
    hist_cond = hist_eq(symbol_hist, "v1")
    new_conds.append(hist_cond)

    # 0 <= m0[r][c] <= hist[r]
    for from_symbol in symbols:
        for to_symbol in symbols:
            cell_id = get_mat_id("m0", from_symbol, to_symbol)
            new_conds.append(cell_id >= 0)
            new_conds.append(cell_id <= symbol_hist[from_symbol])
    
    # 0 <= v0[r] <= hist[r]
    for symbol in symbols:
        cell_id = get_mat_id("v0", symbol, 0)
        new_conds.append(cell_id >= 0)
        new_conds.append(cell_id <= symbol_hist[symbol])

    # cost = sum_rc(m0[r][c]) + sum_r(v[r])
    cost_expr = cost(symbols, "m0", "v0")
    # new_conds.append(cost_expr < 4)

    solver = Optimize()
    solver.add(new_conds)
    solver.minimize(cost_expr)

    if solver.check() == sat:
        print("sat")
        print(solver.model())
    else:
        print("not sat")
    
    return solver

if __name__ == '__main__':
    solve([0, 1, 2, 1])
