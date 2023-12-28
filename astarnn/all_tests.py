#!/usr/bin/env python
"""
Unit tests for everything.
"""
__author__ = 'Barry Drake'


from importlib import import_module as _import
from unittest import TextTestRunner, defaultTestLoader, TestSuite


TEST_VERBOSITY = 2

TEST_MODULES = [
    # Include project tests.
    'astarnn.astarnn_test',
    'astarnn.astarnn_test_extra',
]


def make_suit(test_modules):
    """
    Construct a TestSuite object containing all the unit test in the named test modules.

    :param test_modules: a collection of strings, each naming a test module.
    :returns: a TestSuite object.
    """
    suite = TestSuite()
    for t in test_modules:
        try:
            # If the module defines a suite() function, call it to get the suite.
            module_t = _import(t)
            suite_t = getattr(module_t, 'suite')
            suite.addTest(suite_t())
        except (ImportError, AttributeError):
            # Load all the test cases from the module.
            suite.addTest(defaultTestLoader.loadTestsFromName(t))
    return suite


def main():
    suite = make_suit(TEST_MODULES)
    TextTestRunner(verbosity=TEST_VERBOSITY).run(suite)


if __name__ == '__main__':
    main()
