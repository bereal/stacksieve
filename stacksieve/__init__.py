import functools
import os.path
import threading
import stacksieve_c as sc


from contextlib import contextmanager


__all__ = ['Tracer']


class Tracer:
    def __init__(self, path):
        self.path = path

    @contextmanager
    def _open(self, mode='w'):
        my_id = threading.get_ident()
        main_id = threading.main_thread().ident
        name = 'main' if my_id == main_id else my_id

        path = self.path.format(thread=name)
        try:
            os.mkdir(os.path.dirname(path))
        except FileExistsError:
            pass

        with open(path, mode) as fp:
            yield fp

    @contextmanager
    def _trace(self):
        with self._open() as fp:
            sc.trace_thread(fp.fileno())
            try:
                yield
            finally:
                sc.cleanup_thread()

    @contextmanager
    def trace(self):
        with self._trace():
            yield

    @contextmanager
    def patch_threads(self):
        Thread = threading.Thread
        this = self

        class PatchedThread(Thread):
            def run(self):
                with this._trace():
                    super().run()

        with self.trace():
            threading.Thread = PatchedThread
            try:
                yield
            finally:
                thrading.Thread = Thread
