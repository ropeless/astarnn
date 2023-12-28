"""
Test stop_watch
"""
__author__ = 'Barry Drake'


import time
import unittest
from stop_watch import StopWatch


class Test_StopWatch(unittest.TestCase):

    def test_default(self):
        stop_watch = StopWatch()
        time.sleep(0.001)  # sleep a millisecond
        seconds = stop_watch.seconds()
        self.assertGreater(seconds, 0)

    def test_stopped(self):
        stop_watch = StopWatch(running=False)
        time.sleep(0.001)  # sleep a millisecond
        seconds = stop_watch.seconds()
        self.assertEqual(seconds, 0)

    def test_add(self):
        stop_watch = StopWatch(running=False)
        stop_watch.add(1.5)
        seconds = stop_watch.seconds()
        self.assertEqual(seconds, 1.5)

    def test_multiply(self):
        stop_watch = StopWatch(offset_seconds=5, running=False)
        stop_watch.multiply(4)
        seconds = stop_watch.seconds()
        self.assertEqual(seconds, 20)

    def test_multiply_add(self):
        stop_watch = StopWatch(offset_seconds=5, running=False)
        stop_watch.multiply(4)
        stop_watch.add(7)
        seconds = stop_watch.seconds()
        self.assertEqual(seconds, 27)

    def test_add_multiply(self):
        stop_watch = StopWatch(offset_seconds=5, running=False)
        stop_watch.add(7)
        stop_watch.multiply(4)
        seconds = stop_watch.seconds()
        self.assertEqual(seconds, 48)

    def test_add_operator(self):
        stop_watch = StopWatch(running=False)
        stop_watch += 1.5
        seconds = stop_watch.seconds()
        self.assertEqual(seconds, 1.5)

    def test_mul_operator(self):
        stop_watch = StopWatch(offset_seconds=5, running=False)
        stop_watch *= 4
        seconds = stop_watch.seconds()
        self.assertEqual(seconds, 20)

    def test_hms(self):
        seconds = 2*60*60 + 3*60 + 4
        stop_watch = StopWatch(offset_seconds=seconds, running=False)
        h, m, s = stop_watch.hms()
        self.assertEqual(stop_watch.seconds(), seconds)
        self.assertEqual(h, 2)
        self.assertEqual(m, 3)
        self.assertEqual(s, 4)

    def test_hms_negative(self):
        seconds = -(2*60*60 + 3*60 + 4)
        stop_watch = StopWatch(offset_seconds=seconds, running=False)
        h, m, s = stop_watch.hms()
        self.assertEqual(stop_watch.seconds(), seconds)
        self.assertEqual(h, 2)
        self.assertEqual(m, 3)
        self.assertEqual(s, 4)

    def test_render_hms(self):
        seconds = 2*60*60 + 3*60 + 4
        stop_watch = StopWatch(offset_seconds=seconds, running=False)
        as_str = str(stop_watch)
        self.assertEqual(as_str, '2:03:04.000')

    def test_render_ms(self):
        seconds = 3*60 + 4
        stop_watch = StopWatch(offset_seconds=seconds, running=False)
        as_str = str(stop_watch)
        self.assertEqual(as_str, '3:04.000')

    def test_render_s(self):
        seconds = 4
        stop_watch = StopWatch(offset_seconds=seconds, running=False)
        as_str = str(stop_watch)
        self.assertEqual(as_str, '4.000')

    def test_render_s_with_fractions_1(self):
        seconds = 4.1234
        stop_watch = StopWatch(offset_seconds=seconds, running=False)
        as_str = str(stop_watch)
        self.assertEqual(as_str, '4.123')

    def test_render_s_with_fractions_2(self):
        seconds = 4.1236
        stop_watch = StopWatch(offset_seconds=seconds, running=False)
        as_str = str(stop_watch)
        self.assertEqual(as_str, '4.124')

    def test_render_s_with_fractions_3(self):
        seconds = 0.12367856785678
        stop_watch = StopWatch(offset_seconds=seconds, running=False)
        as_str = str(stop_watch)
        self.assertEqual(as_str, '0.124')

    def test_render_s_with_fractions_4(self):
        seconds = 0.012367856785678
        stop_watch = StopWatch(offset_seconds=seconds, running=False)
        as_str = str(stop_watch)
        self.assertEqual(as_str, '0.0124')

    def test_render_s_with_fractions_5(self):
        seconds = 0.0012367856785678
        stop_watch = StopWatch(offset_seconds=seconds, running=False)
        as_str = str(stop_watch)
        self.assertEqual(as_str, '0.00124')

    def test_render_s_with_fractions_6(self):
        seconds = 0.00012367856785678
        stop_watch = StopWatch(offset_seconds=seconds, running=False)
        as_str = str(stop_watch)
        self.assertEqual(as_str, '0.000124')


if __name__ == '__main__':
    unittest.main()
