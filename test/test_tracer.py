import os
import re
import tempfile

from pytest import fixture
from stacksieve import Tracer


@fixture
def tmpdir():
    with tempfile.TemporaryDirectory() as d:
        yield d


def func1():
    func2()
    func3()


def func2():
    ...


def func3():
    ...


def test_trace(tmpdir):
    tracer = Tracer(os.path.join(tmpdir, '{thread}.txt'))
    with tracer.trace():
        func1()

    with open(os.path.join(tmpdir, 'main.txt')) as fp:
        lines = [s.strip() for s in fp.readlines()][:6]

    assert len(lines) == 6
    patterns = [
        r'\>.*func1',
        r'\>.*func2',
        r'<',
        r'\>.*func3',
        r'<',
    ]

    for p, l in zip(patterns, lines):
        assert re.match(p, l)
