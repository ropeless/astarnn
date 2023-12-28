"""
A base class for classes that should not be instantiated.
"""
__author__ = 'Barry Drake'


class NoInstance(object):
    def __new__(cls):
        raise RuntimeError("do not instantiate")

    def __init__(self):
        raise RuntimeError("do not instantiate")
