"""
A simple code execution timer.

Example usage:

    time = StopWatch()
    # Do some work
    time.stop()

    print('time:', time)

Alternate usage:

    with timer('stuff'):
        # do some stuff

Usage of ProgressCheck:

    check = ProgressCheck(60)
    for iteration in range(max_iterations):
        # Do one iteration.
        ...

        if check:
            print(f'progress: {iteration=} time={check}')

"""
from __future__ import annotations

__author__ = 'Barry Drake'


import timeit as _timeit
from typing import Tuple


class StopWatch:
    __slots__ = ('start_time', 'stop_time', 'offset_seconds', 'multiplier')

    def __init__(self, offset_seconds=0, multiplier=1, running=True):
        """
        Create a StopWatch to start timing, by using timeit.default_timer().
        A StopWatch will be created in the running state.
        Call self.stop() to stop (or pause) the StopWatch.

        :param offset_seconds: is an initial time offset.
        :param multiplier: is an initial time multiplier (also applied to offset_seconds).
        :param running: is a Boolean flag to set the stopwatch running (default True).
        """
        assert multiplier > 0, 'multiplier must be positive'
        self.start_time = _timeit.default_timer()
        self.stop_time = None if running else self.start_time
        self.offset_seconds = offset_seconds
        self.multiplier = multiplier

    def clone(self, running=None):
        """
        Return a clone of this stop watch.

        :param running: controls the running state of the clone.
            If True, the clone will be running (continued),
            if False, the clone will be stopped,
            if None, the clone will be in the same state as this stop watch.
        """
        result = StopWatch(
            offset_seconds=self.offset_seconds,
            multiplier=self.multiplier,
        )
        result.start_time = self.start_time
        result.stop_time = self.stop_time

        if running is not None:
            if running:
                if self.stop_time is not None:
                    # starting
                    result.continu()
            else:
                if self.stop_time is None:
                    # stopping
                    result.stop()
        return result

    def start(self, offset_seconds=0, multiplier=1):
        """
        Mark the start time for the timer as now.
        Cancels any previous start and stop.

        :param offset_seconds: is an initial time offset.
        :param multiplier: is an initial time multiplier (also applied to offset_seconds).
        """
        assert multiplier > 0, 'multiplier must be positive'
        self.start_time = _timeit.default_timer()
        self.stop_time = None
        self.offset_seconds = offset_seconds
        self.multiplier = multiplier

    def stop(self):
        """
        Mark the stop time for the timer as now.
        If the stop watch was already stopped, then this overrides the previous stop.
        """
        self.stop_time = _timeit.default_timer()

    def continu(self):
        """
        Continue the timer, cancelling any previous stop.
        Any 'pause' time between a stop and continu is not included in the elapsed time.
        """
        if self.stop_time is not None:
            paused_seconds = _timeit.default_timer() - self.stop_time
            self.offset_seconds -= paused_seconds
            self.stop_time = None

    def running(self) -> bool:
        """
        Is this stopwatch running?
        :return: a bool
        """
        return self.stop_time is None

    def set(self, seconds, multiplier=1):
        """
        Set the stopwatch to the given number of seconds.
        This stops the stopwatch and resets the time multiplier.

        :param seconds: is the value to set the stop watch to.
        :param multiplier: is reset the time multiplier (also applied to seconds).
        """
        self.start_time = _timeit.default_timer()
        self.stop_time = self.start_time
        self.offset_seconds = seconds
        self.multiplier = multiplier

    def add(self, seconds):
        """
        Add the given number of seconds to the stopwatch.
        The number of seconds added is not affected by the time multiplier.
        """
        if isinstance(seconds, StopWatch):
            seconds = seconds.seconds()
        self.offset_seconds += seconds / self.multiplier

    def subtract(self, seconds):
        """
        Subtract the given number of seconds from the stopwatch.
        The number of seconds subtracted is not affected by the time multiplier.
        """
        if isinstance(seconds, StopWatch):
            seconds = seconds.seconds()
        self.offset_seconds -= seconds / self.multiplier

    def multiply(self, multiplier):
        """
        Multiply the rate of time by the given multiplier.
        Multiplication is accumulative.
        """
        assert multiplier > 0, 'multiplier must be positive'
        self.multiplier *= multiplier

    def seconds(self) -> float:
        """Number of seconds of elapsed time."""
        if self.stop_time is None:
            time = _timeit.default_timer() - self.start_time
        else:
            time = self.stop_time - self.start_time
        return (time + self.offset_seconds) * self.multiplier

    def minutes(self) -> float:
        """Number of minutes elapsed."""
        return self.seconds() / 60.0

    def hours(self) -> float:
        """Number of hours elapsed."""
        return self.seconds() / 3600.0

    def hms(self) -> Tuple[int, int, float]:
        """
        (hours, minutes, seconds) of elapsed time.
        Hours and minutes will always be integers.
        Only the absolute value of time will be reported
        (i.e., if negative time offsets are used).
        """
        elapsed = abs(self.seconds())
        hours, rem = divmod(elapsed, 3600)
        minutes, seconds = divmod(rem, 60)
        return int(hours), int(minutes), seconds

    def __str__(self) -> str:
        (hours, minutes, seconds) = self.hms()
        if hours > 0:
            return f'{hours:}:{minutes:0>2}:{seconds:06.3f}'
        elif minutes > 0:
            return f'{minutes:}:{seconds:06.3f}'
        elif seconds >= 0.1:
            return f'{seconds:.3f}'
        elif seconds >= 0.01:
            return f'{seconds:.4f}'
        elif seconds >= 0.001:
            return f'{seconds:.5f}'
        elif seconds >= 0.0001:
            return f'{seconds:.6f}'
        else:
            return str(seconds)

    def __repr__(self) -> str:
        offset_seconds = self.seconds()
        multiplier = self.multiplier
        running = self.running()
        name = self.__class__.__name__
        return f'{name}(offset_seconds={offset_seconds}, multiplier={multiplier}, running={running})'

    def __float__(self) -> float:
        return self.seconds()

    def __add__(self, other) -> StopWatch:
        """The returned stop watch will be stopped."""
        s = self.clone(running=False)
        s.add(other)
        return s

    def __iadd__(self, other) -> StopWatch:
        self.add(other)
        return self

    def __sub__(self, other) -> StopWatch:
        """The returned stop watch will be stopped."""
        s = self.clone(running=False)
        s.subtract(other)
        return s

    def __isub__(self, other) -> StopWatch:
        self.subtract(other)
        return self

    def __mul__(self, multiplier) -> StopWatch:
        """The returned stop watch will be stopped."""
        s = self.clone(running=False)
        s.multiply(multiplier)
        return s

    def __imul__(self, multiplier) -> StopWatch:
        self.multiply(multiplier)
        return self

    def __eq__(self, other) -> bool:
        return self.seconds() == other.seconds()

    def __ne__(self, other) -> bool:
        return self.seconds() != other.seconds()

    def __lt__(self, other) -> bool:
        return self.seconds() < other.seconds()

    def __gt__(self, other) -> bool:
        return self.seconds() > other.seconds()

    def __le__(self, other) -> bool:
        return self.seconds() <= other.seconds()

    def __ge__(self, other) -> bool:
        return self.seconds() >= other.seconds()

    def __hash__(self):
        return hash(self.seconds())


class ProgressCheck(StopWatch):
    """
    A class to support simple progress checking in a loop.
    """

    def __init__(self, reporting_seconds, first_check=None):
        """
        :param reporting_seconds: how often (in seconds) should 'check' return True.
            This is the minimum time between 'check' returning True.
        :param first_check: when should 'check' first return True (in seconds).
            If first_check is None the default value is reporting_seconds.
        """
        self.reporting_seconds = reporting_seconds
        self.next_check = reporting_seconds if first_check is None else first_check
        super().__init__()

    def check(self) -> bool:
        """
        Returns True only if it has been long enough since the last True check.
        """
        seconds = self.seconds()
        if seconds > self.next_check:
            self.next_check = seconds + self.reporting_seconds
            return True
        else:
            return False

    def __bool__(self) -> bool:
        return self.check()


class timer(StopWatch):

    def __init__(
            self,
            label='a',
            start_message='{label} timer started',
            stop_message='{label} timer stopped: {time}',
            file=None,
            logger=None
    ):
        """
        Create a timer that will use a stop watch to time a section of code within a 'with' statement.
        The timer label will be printed on entering the 'with' statement.
        The timer label and time taken will be printed on exiting the 'with' statement.

        :param label: A text string to label the timer.
        :param start_message: How the 'enter' message will be formatted.
        :param stop_message: How the 'exit' message will be formatted.
        :param file: Where messages should be printed - an output stream.
        :param logger: Where messages should be printed - a print function.

        Either file or logger may be specified, not both. If neither,
        then the standard output is used.
        """
        super().__init__(running=False)
        self._label = '' if label is None else label
        self._start_message = start_message
        self._stop_message = stop_message
        self._file = file
        self._logger = logger

        if self._file is not None:
            if self._logger is not None:
                raise RuntimeError('cannot specify both file and logger')
            self._print = self._print_file
        elif self._logger is not None:
            self._print = self._print_logger
        else:
            self._print = self._print_stdout

    def _print(self, *args):
        pass  # dynamically set at construction time

    @staticmethod
    def _print_stdout(*args):
        print(*args)

    def _print_file(self, *args):
        print(*args, file=self._file)

    def _print_logger(self, *args):
        self._logger(*args)

    def __enter__(self):
        if self._start_message is not None:
            self._print(self._start_message.format(label=self._label))
        self.start()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.stop()
        if self._stop_message is not None:
            self._print(self._stop_message.format(label=self._label, time=self))
        return exc_val is None
