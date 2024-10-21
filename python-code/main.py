from enum import Enum
from collections import Counter
from z3 import *

def gen_mat_id(from_symbol: int, to_symbol: int):
    return Int("mat[" + str(from_symbol) + "]" + "[" + str(to_symbol) + "]")

def solve(goal: list[int]):
    symbol_hist = Counter(goal)

    new_conds = []

    for from_symbol in symbol_hist:
        for to_symbol in symbol_hist:
            cell_id = gen_mat_id(from_symbol, to_symbol)
            new_conds.append(cell_id >= 0)
            new_conds.append(cell_id <= symbol_hist[from_symbol])

    return None


# Here's an example of the kind of `(schedule, array)` pair you should
# return. This one exhibits a serializablility counterexample for the
# lost-update-x.par examples.
def example_lost_update_schedule():
    schedule = [(Schedule.PROG1, 0),
                (Schedule.PROG2, 0),
                (Schedule.PROG1, 1),
                (Schedule.PROG2, 1)]
    # You can use a normal python list here, but defaultlist may
    # more closely correspond to the model you get from Z3.
    # https://defaultlist.readthedocs.io/en/1.0.0/
    array = defaultlist(lambda: 0)
    return (schedule, array)

# You may freely modify this suite to include your own tests. (The
# grading script will ignore this method and provide its own test
# suite.)
def test_suite():
    # Each tuple represents a test: (prog1, prog2, expected result)
    return [
        ('examples/lost-update-1.par', 'examples/lost-update-2.par', Expected.CEX),
        ('examples/nonrep-read-1.par', 'examples/nonrep-read-2.par', Expected.CEX),
        ('examples/serializable-1.par', 'examples/serializable-2.par', Expected.NO_CEX),
    ]


# --------------------------------------------------------------------
# Below this line is setup that you shouldn't need to touch.

class Expected(Enum):
    NO_CEX = 1
    CEX = 2

class ParTestCase(unittest.TestCase):
    def __init__(self, prog1, prog2, expected):
        super().__init__('test_serializability_cex')
        self._testMethodDoc = '(' + prog1 + ', ' + prog2 + ')'
        self.prog1 = prog1
        self.prog2 = prog2
        self.expected = expected

    def test_serializability_cex(self):
        prog1 = parse_parlang(self.prog1)
        prog2 = parse_parlang(self.prog2)
        result = find_serializability_cex(prog1, prog2)
        if result == None:
            self.assertEqual(Expected.NO_CEX, self.expected)
        else:
            self.assertEqual(Expected.CEX, self.expected)
            (schedule, array) = result
            validate_schedule(prog1, prog2, schedule)
            (_, array_seq1) = eval_scheduled(prog1, prog2, array.copy())
            (_, array_seq2) = eval_scheduled(prog2, prog1, array.copy())
            (_, array_sched) = eval_scheduled(prog1, prog2, array.copy(), schedule)
            self.assertNotEqual(array_seq1, array_sched,
               "The scheduled and serial (prog1; prog2) executions gave the same result.")
            self.assertNotEqual(array_seq2, array_sched,
               "The scheduled and serial (prog2; prog1) executions gave the same result.")

if __name__ == '__main__':
    runner = unittest.TextTestRunner()
    suite = unittest.TestSuite()
    for (prog1, prog2, expected) in test_suite():
        suite.addTest(ParTestCase(prog1, prog2, expected))
    runner.run(suite)
