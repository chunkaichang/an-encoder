#!/usr/bin/env python

import multiprocessing
import os
import signal
import subprocess
import time


class LimitedProcess:
    class Alarm(Exception):
        pass

    @staticmethod
    def _alarm_handler(signum, frame):
        raise LimitedProcess.Alarm

    def __init__(self, args, timeout=None):
        self.args = args
        self.timeout = timeout
        self.pid = None
        self.retcode, self.stdout, self.stderr = None, "", ""

    def run(self, timeout, logfile, prefix=""):
        if not timeout:
            timeout = self.timeout
        if not logfile:
            logfile = os.sys.stdout

        p = subprocess.Popen(self.args, shell=True,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             preexec_fn=os.setsid)
        self.pid = p.pid
        logfile.write("pid=%d: started process %s\n" % (p.pid, self.args), prefix)

        if timeout:
            logfile.write("pid=%d: timeout=%d\n" % (p.pid, timeout), prefix)
            signal.signal(signal.SIGALRM, LimitedProcess._alarm_handler)
            signal.alarm(timeout)

        try:
            stdout, stderr = p.communicate()
            if timeout:
                signal.alarm(0)
            logfile.write("pid=%d: communicated with process\n" % p.pid, prefix)
            self.retcode, self.stdout, self.stderr = p.returncode, stdout, stderr
        except LimitedProcess.Alarm:
            try:
                os.killpg(p.pid, signal.SIGKILL)
                logfile.write("pid=%d: killed process\n" % p.pid, prefix)
            except OSError:
                pass
            self.retcode, self.stdout, self.stderr = None, "", ""

        return self.retcode, self.stdout, self.stderr

    def get_retcode(self):
        return self.retcode

    def get_stdout(self):
        return self.stdout

    def get_stderr(self):
        return self.stderr

    def get_result(self):
        return self.retcode, self.stdout, self.stderr

    def get_pid(self):
        return self.pid


class ArgListProcessor:
    def __init__(self, processes, target, arg_list):
        self.processes = processes
        self.target = target
        self.arg_list = arg_list
        self.n = len(arg_list)
        self.finished = multiprocessing.Value('i', 0)
        self.sema = multiprocessing.Manager().Semaphore(processes)

    def target_with_sema(self, *args):
        result = self.target(*args)
        self.sema.release()
        return result

    def run(self):
        started = []
        while len(self.arg_list) > 0:
            p = multiprocessing.Process(target=self.target_with_sema, args=self.arg_list.pop(0))
            p.daemon = False
            """ Apparently it leads to more robust execution of multiple processes if the semaphore
                is acquired here (rather than in 'target_with_sema'). Perhaps Python's
                'multiprocessing' module cannot deal efficiently with too many processes being
                started and immediately suspended (on the semaphore).
            """
            self.sema.acquire()
            started.append(p)
            p.start()
        for p in started:
            p.join()
        return


class ConcurrentFile():
    def __init__(self, path, prefix="", skip=False):
        self.path = path
        self.prefix = prefix
        self.skip = skip

        self.buf = ""
        self.lock = multiprocessing.Manager().Lock()
        # ensure that the file exists and is empty:
        f = open(self.path, "w")
        f.close()

    def get_path(self):
        return self.path

    def get_lock(self):
        return self.lock

    def write(self, output, prefix="", timed=True):
        if self.skip:
            return

        self.lock.acquire()
        if timed:
            self.buf += time.strftime("%Y-%m-%d_%H:%M:%S $$ ", time.localtime())
        prefix = (self.prefix + " " + prefix).strip()
        if prefix != "":
            self.buf += (prefix + " $$ ")
        self.buf += output
        self.lock.release()
        return

    def flush(self):
        if self.skip:
            return

        self.lock.acquire()
        file = open(self.path, "a")
        file.write(self.buf)
        file.close()
        self.buf = ""
        self.lock.release()
        return


class ConcurrentDict():
    def __init__(self):
        self.data = multiprocessing.Manager().dict()

    def __getitem__(self, key):
        return self.data.__getitem__(key)

    def __setitem__(self, key, value):
        return self.data.__setitem__(key, value)

    def __delitem__(self, key):
        return self.data.__delitem__(key)

    def __missing__(self, key):
        return self.data.__missing__(key)
