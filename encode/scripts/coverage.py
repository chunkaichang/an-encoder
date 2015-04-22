#!/usr/bin/env python

""" Based on Dmitry's "bfi.py" """

import argparse
import subprocess
import os
import re
import random
import runner
import statistics
import testcase
import time
import signal


SEPARATOR = "----------------------------------------------------------------\n"


class Process:
    class Alarm(Exception):
        pass

    @staticmethod
    def _alarm_handler(signum, frame):
        raise Process.Alarm

    def __init__(self, args, timeout=None):
        self.args = args
        self.timeout = timeout
        self.retcode, self.stdout, self.stderr = None, "", ""

    def run(self, timeout=None, logfile=None):
        if not timeout:
            timeout = self.timeout
        if not logfile:
            logfile = os.sys.stdout

        p = subprocess.Popen(self.args, shell=True,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             preexec_fn=os.setsid)
        logfile.write("pid=%d: started process %s\n" % (p.pid, self.args))

        if timeout:
            logfile.write("pid=%d: timeout=%d\n" % (p.pid, timeout))
            signal.signal(signal.SIGALRM, Process._alarm_handler)
            signal.alarm(timeout)

        try:
            stdout, stderr = p.communicate()
            if timeout:
                signal.alarm(0)
            logfile.write("pid=%d: communicated with process\n" % p.pid)
            self.retcode, self.stdout, self.stderr = p.returncode, stdout, stderr
        except Process.Alarm:
            try:
                os.killpg(p.pid, signal.SIGKILL)
                logfile.write("pid=%d: killed process\n" % p.pid)
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

    def extract_checksum(self):
        if self.retcode is None:
            return None

        checksum = 0
        for line in self.stderr.split('\n'):
            if "checksum" in line:
                val = int(line.split('=')[1].strip(), 16)
                if "hi" in line:
                    val <<= 64
                checksum += val
        return checksum


class FaultInjector:
    """ from Dmitry's "bfi_const.py":
        7 types of faults to inject:
         - WVAL: some address that is written is overwritten with an
                 arbitrary value
         - RVAL: some address that is read is written with an arbitrary
                 value before being read
         - WADDR: some pointer that is written is corrupted (value is
                  correct)
         - RADDR: some pointer that is read is corrupted
         - RREG: some register that is read is overwritten with an
                 arbitrary value before being read
         - WREG: some register that is written is overwritten with an
                 arbitrary value after being written
         - CF:   change instruction pointer
         - TXT:  corrupt code segment
    """
    faults = ['WVAL', 'RVAL', 'WADDR', 'RADDR', 'RREG', 'WREG', 'CF', 'TXT']

    class Result:
        def __init__(self):
            self.total = 0  # total number of fault injections

            self.valid = 0  # number of valid fault injections (i.e. where error was inserted in the right function)

            self.correct = 0  # number of programs unaffected by fault injection

            self.anfailure = 0  # number of undetected faults (i.e. SDC):
            self.unexp = 0      #    number of unexpected program results
            self.hang = 0       #    number of hanging programs (more than 10x runtime than without injected faults)

            self.ansuccess = 0  # number of detected faults:
            self.oscrash = 0    #   fault detected by OS
            self.ancrash = 0    #   fault detected by AN encoder
            self.nondiag = 0    #   undiagnosed, i.e. weird return code

        def inc_total(self):
            self.total += 1

        def inc_valid(self):
            self.valid += 1

        def inc_correct(self):
            self.correct += 1

        def inc_failure(self, kind):
            if kind == "UNEXPECTED":
                self.unexp += 1
            elif kind == "HANG":
                self.hang += 1
            else:
                raise NameError("Failure kind %s undefined." % kind)
            self.anfailure += 1
            return

        def inc_success(self, kind):
            if kind == "OSCRASH":
                self.oscrash += 1
            elif kind == "ANCRASH":
                self.ancrash += 1
            elif kind == "NONDIAG":
                self.nondiag += 1
            else:
                raise NameError("Success kind %s undefined." % kind)
            self.ansuccess += 1
            return

        def write(self, logfile=None):
            if not logfile:
                logfile = os.sys.stderr

            logfile.write("Results:\n")
            # logfile.write(time.strftime('finished: %H:%M:%S\n', time.gmtime()))
            logfile.write("total   = %d\n" % self.total)
            logfile.write("valid   = %d\n" % self.valid)
            logfile.write("correct = %d\n" % self.correct)
            logfile.write("AN success = %d\n" % self.ansuccess)
            logfile.write("    OS crashes  = %d\n" % self.oscrash)
            logfile.write("    AN crashes  = %d\n" % self.ancrash)
            logfile.write("    undiagnosed = %d\n" % self.nondiag)
            logfile.write("AN failed = %d\n" % self.anfailure)
            logfile.write("    unexpected = %d\n" % self.unexp)
            logfile.write("    hang       = %d\n" % self.hang)
            logfile.write(SEPARATOR)
            return

    def __init__(self, test, bfi):
        self.test = test
        self.bfi = bfi

        self.result = FaultInjector.Result()

        print("Changing ptrace_scope to 0...")
        subprocess.call('echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope > /dev/null', shell=True)
        print("Changing randomize_va_space to 0...")
        subprocess.call('echo 0 | sudo tee /proc/sys/kernel/randomize_va_space > /dev/null', shell=True)

    def _warmup(self, key, logfile=None):
        if not logfile:
            logfile = os.sys.stdout

        bfi_args = " -m %s" % self.test.coverage_function
        test_cmd = self.test.commands[key]
        logfile.write("Warming up ...\n")
        timings = []
        for i in range(self.test.timing_runs):
            logfile.write("... timing run no. %d ...\n" % i)
            p = Process(self.bfi + bfi_args + " -- " + test_cmd)
            start = time.time()
            p.run(None, logfile)
            end = time.time()
            timings.append(end - start)
        logfile.write("... done with timing runs ...\n")
        mean = statistics.mean(timings)
        stdev = statistics.stdev(timings)
        """ We set the timeout for faulty runs to 10x the time a sane run takes. The motivation for this is that if a
            fault-injected run takes more than 10x the runtime of the corresponding sane run, we might as well regard
            this as an undetected error since the use may not be prepared to wait that long.
            ("+ 1.0" necessary in case "10.0*mean" is still below one second.) """
        self.timeout = int(10.0 * mean + 1.0)
        logfile.write("timings: mean=%f, stdev=%f, timeout=%d\n" % (mean, stdev, self.timeout))
        logfile.write("... done warming up\n")
        logfile.write(SEPARATOR)

    def _get_function_ranges(self, key, logfile=None):
        if not logfile:
            logfile = os.sys.stdout

        # find the range of the "coverage_function":
        logfile.write("Obtaining ranges of function %s ...\n" % self.test.coverage_function)
        bfi_args = " -m %s" % self.test.coverage_function
        bfi_cmd = self.bfi + bfi_args + " -- " + self.test.commands[key]
        p = Process(bfi_cmd)
        _, _, stderr = p.run(None, logfile)
        logfile.write("... finished running %s ...\n" % bfi_cmd)

        # write "stderr" from running "bfi" to file:
        rngs_name = os.path.join(self.test.outputs_dir, self.test.cfg_name + (".%s.ranges" % key))
        rngs_file = open(rngs_name, "w")
        logfile.write("... writing <stderr> to %s ...\n" % rngs_name)
        rngs_file.write("<stderr> from running %s:\n" % bfi_cmd)
        rngs_file.write(stderr)
        rngs_file.write(SEPARATOR)
        logfile.write("... done writing <stderr> to %s ...\n" % rngs_name)

        func_ranges = []  # list of func ranges: pairs of enter/leave
        func_enters = []
        lines = stderr.split('\n')
        i = 0
        while i < len(lines):
            s = lines[i]
            if len(s) > 0 and s[0] == '[':
                trigger = s.split('i =')[1].split(',')[0]
                trigger = int(trigger)
                # get the following line
                sf = lines[i + 1]
                if re.match(".*enter", sf):
                    func_enters.append(trigger)
                    i += 2
                    continue
                if re.match(".*leave", sf):
                    func_ranges.append((func_enters.pop(), trigger))
                    i += 2
                    continue
            i += 1

        rngs_file.write("ranges of function %s:\n" % self.test.coverage_function)
        for idx, func_range in enumerate(func_ranges):
            rngs_file.write("[   %10d - %10d ]\n" % (func_range[0], func_range[1]))
            # We always assume "PRECISE=True":
            """ if PRECISE == False:
                func_ranges[idx] = (func_range[0] + 10, func_range[1] - 10)
            """
        rngs_file.write(SEPARATOR)

        """ This only yields different ranges from the above for loop if "PRECISE" is set to false. We ignore
            "PRECISE", so we do not need this loop either: """
        """ rngs_file.write("ranges for fault injection in function %s:\n" % self.test.coverage_function)
            for func_range in func_ranges:
                rngs_file.write("[   %10d - %10d ]\n" % (func_range[0], func_range[1]))
            rngs_file.write(SEPARATOR)
        """
        rngs_file.close()
        logfile.write("... done obtaining ranges of function %s\n" % self.test.coverage_function)
        logfile.write(SEPARATOR)
        return func_ranges


    def _diagnose(self, retcode, is_valid, checksum, ref_checksum):
        self.result.inc_total()

        if not is_valid:
            return "INVALID"
        self.result.inc_valid()

        if retcode is None:
            kind = "HANG"
            self.result.inc_failure(kind)
        elif retcode == 0:
            # Fault did not crash the program.
            if checksum != ref_checksum:
                # Output is not correct: Silent Data Corruption.
                kind = "UNEXPECTED"
                self.result.inc_failure(kind)
            else:
                # Output is correct: Fault didn't affect execution.
                kind = "CORRECT"
                self.result.inc_correct()
        elif retcode == 2:
            kind = "ANCRASH"
            self.result.inc_success(kind)
        elif retcode > 10:
            kind = "OSCRASH"
            self.result.inc_success(kind)
        else:
            kind = "NONDIAG"

        return kind


    def run(self, key, masktype, ref_checksum, logfile=None):
        """ masktype is one of { RANDOM_8BITS, RANDOM_32BITS, RANDOM_8BITS, RANDOM_1BITFLIP, RANDOM_2BITFLIPS } """
        def get_mask(type_string):
            if type_string == 'RANDOM_32BITS':
                # inject a random low-32-bit fault
                return random.randint(1, 2 ** 32 - 1)
            elif type_string == 'RANDOM_8BITS':
                # inject a random low-8-bit fault
                return random.randint(1, 2 ** 8 - 1)
            elif type_string == 'RANDOM_1BITFLIP':
                # inject one random bit-flip in one of low 32 bits
                bit = random.randint(0, 32 - 1)
                return 2 ** bit
            elif type_string == 'RANDOM_2BITFLIPS':
                # inject two random bit-flips in two of low 32 bits
                bit1 = random.randint(0, 32 - 1)
                bit2 = random.randint(0, 32 - 1)
                while bit1 == bit2:
                    bit2 = random.randint(0, 32 - 1)
                return 2 ** bit1 + 2 ** bit2
            else:
                raise NameError('MASKTYPE_NOT_DEFINED')

        def valid_fault(func_ranges, stderr):
            # --- check if fault was injected inside function ---
            # instruction where fault was actually injected (triggered)
            trig_instr = 0
            """ We search the lines in "stderr" backwards. Usually the last line that was printed to "stderr" will
                contain information about the injected fault: """
            lines = stderr.split('\n')
            i = 0
            while i < len(lines):
                # Lines are in reverse order now, so "pop" the 2nd line first and then the 1st line:
                s1 = lines[i]
                s2 = lines[i+1]
                if len(s1) > 0 and s1[0] == '[' and len(s1.split('i =')) > 1:
                    trigger = int(s1.split('i =')[1].split(',')[0])
                    """ Since we are inspecting lines in reverse order, we can break out of the while loop as soon as
                        we see the first line that does not correspond to entering or leaving a function. In that case
                        the line is indeed the one specifiying the kind of injected fault: """
                    if (re.match('.*enter', s2) is None and
                        re.match('.*leave', s2) is None):
                        trig_instr = trigger
                        break
                i += 1

            for func_range in func_ranges:
                if func_range[0] <= trig_instr <= func_range[1]:
                    return True
            return False

        if not logfile:
            logfile = os.sys.stdout

        self._warmup(key, logfile)
        func_ranges = self._get_function_ranges(key, logfile)

        logfile.write("Performing fault injections ...\n")
        # do NOT inject CF -- consider them Control Flow errors, not Data Flow
        inject_faults = list(set(FaultInjector.faults) - set(['CF']))
        mask = get_mask(masktype)
        for i in range(self.test.coverage_runs):
            index = random.randint(0, len(func_ranges) - 1)
            instr = random.randint(func_ranges[index][0], func_ranges[index][1])
            fault = inject_faults[random.randint(0, len(inject_faults) - 1)]

            bfi_args = " -trigger %d -cmd %s -seed 1 -mask %d" % (instr, fault, mask)
            bfi_cmd = self.bfi + bfi_args + " -- " + self.test.commands[key]
            logfile.write("... no. %d: running %s ...\n" % (i, bfi_cmd))
            p = Process(bfi_cmd)
            retcode, _, stderr = p.run(self.timeout, logfile)
            logfile.write("... no. %d: finished running %s ...\n" % (i, bfi_cmd))

            # write "stderr" from running "bfi" to file:
            fi_name = os.path.join(self.test.outputs_dir, self.test.cfg_name + (".%s.fi.%d" % (key, i)))
            fi_file = open(fi_name, "w")
            logfile.write("... no. %d: writing <stderr> to %s ...\n" % (i, fi_name))
            fi_file.write("<stderr> from running %s:\n" % bfi_cmd)
            fi_file.write(stderr)
            fi_file.write(SEPARATOR)
            fi_file.close()
            logfile.write("... no. %d: done writing <stderr> to %s ...\n" % (i, fi_name))

            checksum = p.extract_checksum()
            is_valid = valid_fault(func_ranges, stderr)
            kind = self._diagnose(retcode, is_valid, checksum, ref_checksum)
            logfile.write("... no. %d: %s (retcode=%s) %s ...\n" % (i, kind, str(retcode), bfi_cmd))

        logfile.write("... finished fault injections.\n")
        logfile.write(SEPARATOR)

        self.result.write(logfile)
        return


#===============================================================================
# MAIN
#===============================================================================
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--directory",
                        default=r".",
                        help="directory to be searched for test cases")
    parser.add_argument("--pin",
                        default=r"./pin.sh",
                        help="path to Intel PIN tool")
    parser.add_argument("--bfi",
                        default=r"./bfi.so",
                        help="path to 'bfi' plugin ('bfi.so')")
    args = parser.parse_args()
    suite = testcase.TestSuite(args.directory)
    bfi = "%s -t %s" % (args.pin, args.bfi)

    for name in suite.get_names():
        test = suite.get_test(name)
        runner = runner.TestRunner(test)

        # Get the reference output from a 'plain' run:
        print "Golden 'plain' run ..."
        p = Process(test.commands["plain"])
        retcode, _, _ = p.run()
        if retcode != 0:
            raise Exception("Coverage", test.commands["plain"])
        print "... finished golden run."
        print SEPARATOR
        ref_checksum = p.extract_checksum()

        fi = FaultInjector(test, bfi)

        logfile = open(test.outputs["ref"], "w")
        fi.run("plain", "RANDOM_8BITS", ref_checksum, logfile)
        logfile.close()

        logfile = open(test.outputs["cov"], "w")
        fi.run("encoded", "RANDOM_8BITS", ref_checksum, logfile)
        logfile.close()
