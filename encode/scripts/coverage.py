#!/usr/bin/env python

""" Based on Dmitry's "bfi.py" """

import argparse
import cStringIO
import os
import re
import random
import statistics
import subprocess
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
        _, checksum = testcase.TestCase.extract_info(self.stderr.split('\n'))
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

        def diagnose(self, retcode, is_valid, checksum, ref_checksum, cso_file, ref_cso_file):
            self.inc_total()

            if not is_valid:
                return "INVALID"
            self.inc_valid()

            if retcode is None:
                kind = "HANG"
                self.inc_failure(kind)
            elif retcode == 0:
                check = checksum != ref_checksum
                diff = testcase.TestCase.diff_files(cso_file, ref_cso_file)
                # Fault did not crash the program.
                if diff:
                    # Output is not correct: Silent Data Corruption.
                    kind = "UNEXPECTED"
                    self.inc_failure(kind)
                    kind += " (checksum difference: %d)" % check
                else:
                    # Output is correct: Fault didn't affect execution.
                    kind = "CORRECT"
                    self.inc_correct()
                    kind += " (checksum difference: %d)" % check
            elif retcode == 1:
                # Something has gone wrong while executing the test case binary.
                raise Exception("Result", "Unexpected error in execution of test case.")
            elif retcode == 2:
                kind = "ANCRASH"
                self.inc_success(kind)
            elif retcode > 10:
                kind = "OSCRASH"
                self.inc_success(kind)
            else:
                kind = "NONDIAG"
            return kind

        def write(self, logfile=None, prefix=""):
            if not logfile:
                logfile = os.sys.stderr

            logfile.write("%stotal   = %d\n" % (prefix, self.total))
            logfile.write("%svalid   = %d\n" % (prefix, self.valid))
            logfile.write("%scorrect = %d\n" % (prefix, self.correct))
            logfile.write("%sAN success = %d\n" % (prefix, self.ansuccess))
            logfile.write("%s    OS crashes  = %d\n" % (prefix, self.oscrash))
            logfile.write("%s    AN crashes  = %d\n" % (prefix, self.ancrash))
            logfile.write("%s    undiagnosed = %d\n" % (prefix, self.nondiag))
            logfile.write("%sAN failed = %d\n" % (prefix, self.anfailure))
            logfile.write("%s    unexpected = %d\n" % (prefix, self.unexp))
            logfile.write("%s    hang       = %d\n" % (prefix, self.hang))
            return

    def __init__(self, test, bfi):
        self.test = test
        self.bfi = bfi

        self.result = None

        print("Changing ptrace_scope to 0...")
        subprocess.call('echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope > /dev/null', shell=True)
        print("Changing randomize_va_space to 0...")
        subprocess.call('echo 0 | sudo tee /proc/sys/kernel/randomize_va_space > /dev/null', shell=True)

    def get_result(self):
        return self.result

    def _warmup(self, key, logfile=None):
        if not logfile:
            logfile = os.sys.stdout

        bfi_args = " -m %s" % self.test.get_function()
        test_cmd = self.test.commands[key] + " --cso /dev/null"
        logfile.write("Warming up ...\n")
        timings = []
        for i in range(self.test.warmups):
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
        logfile.write("Obtaining ranges of function %s ...\n" % self.test.get_function())
        bfi_args = " -m %s" % self.test.get_function()
        bfi_cmd = self.bfi + bfi_args + " -- " + self.test.commands[key] + " --cso /dev/null"
        p = Process(bfi_cmd)
        _, _, stderr = p.run(None, logfile)
        logfile.write("... finished running %s ...\n" % bfi_cmd)

        # write "stderr" from running "bfi" to file:
        rngs_name = os.path.join(self.test.outputs_dir, self.test.get_name() + (".%s.ranges" % key))
        rngs_file = open(rngs_name, "w")
        logfile.write("... writing <stderr> to %s ...\n" % rngs_name)
        rngs_file.write("<stderr> from running %s:\n" % bfi_cmd)
        rngs_file.write(stderr)
        rngs_file.write(SEPARATOR)
        logfile.write("... done writing <stderr> to %s ...\n" % rngs_name)

        func_ranges = []  # list of func ranges: pairs of enter/leave
        func_enters = []
        lines = cStringIO.StringIO(stderr)
        counter = 0
        while True:
            s = lines.readline()
            if s == "":
                break
            if len(s) > 0 and s[0] == '[':
                trigger = s.split('i =')[1].split(',')[0]
                trigger = int(trigger)
                # get the following line
                sf = lines.readline()
                assert(sf != "")
                if re.match(".*enter", sf):
                    func_enters.append((trigger, counter))
                    func_ranges.append(None)
                    counter += 1
                    continue
                if re.match(".*leave", sf):
                    entry, index = func_enters.pop()
                    # Make sure that the ranges are sorted by the entry into the range (in ascending order):
                    func_ranges[index] = (entry, trigger)
                    continue

        rngs_file.write("ranges of function %s:\n" % self.test.get_function())
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
        logfile.write("... done obtaining ranges of function %s\n" % self.test.get_function())
        logfile.write(SEPARATOR)
        return func_ranges

    def run(self, key, masktype, ref_checksum, ref_file, logfile=None):
        """ masktype is one of { RANDOM_8BITS, RANDOM_32BITS, RANDOM_8BITS, RANDOM_1BITFLIP, RANDOM_2BITFLIPS } """
        def get_random_mask(type_string):
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

        def valid_fault(func_ranges, retcode, stderr):
            if retcode is None:
                # We consider programs that hanged as valid (since we have no output that can be validated):
                return True
            # --- check if fault was injected inside function ---
            # instruction where fault was actually injected (triggered)
            trig_instr = 0
            lines = cStringIO.StringIO(stderr)
            while True:
                s1 = lines.readline()
                if s1 == "":
                    break
                if len(s1) > 0 and s1[0] == '[' and len(s1.split('i =')) > 1:
                    """ Note that if PIN crashes in such a way that no output lines are generated that match the
                        condition in the previous if clause, then, after the while loop, 'trig_instr' will still be 0.
                        Since no function range should start at 0, 'valid_fault' will return False. This behaviour is
                        desired since a test that has crashed the PIN tool should be considered invalid. """
                    trigger = int(s1.split('i =')[1].split(',')[0])
                    s2 = lines.readline()
                    assert(s2 != "")
                    if (re.match('.*enter', s2) is None and
                        re.match('.*leave', s2) is None):
                        trig_instr = trigger
                        break

            for func_range in func_ranges:
                if func_range[0] <= trig_instr <= func_range[1]:
                    return True
                elif trig_instr < func_range[0]:
                    """ Note that this "elif" statement only makes sense if 'func_range' is sorted in ascending order,
                        which is indeed achieved by our implementation of '_get_function_ranges'. """
                    return False
            return False

        if not logfile:
            logfile = os.sys.stdout

        self._warmup(key, logfile)
        func_ranges = self._get_function_ranges(key, logfile)

        logfile.write("Performing fault injections ...\n")
        # do NOT inject CF -- consider them Control Flow errors, not Data Flow
        inject_faults = list(set(FaultInjector.faults) - set(['CF']))
        self.result = FaultInjector.Result()
        for i in range(self.test.get_runs()):
            index = random.randint(0, len(func_ranges) - 1)
            instr = random.randint(func_ranges[index][0], func_ranges[index][1])
            fault = inject_faults[random.randint(0, len(inject_faults) - 1)]
            mask = get_random_mask(masktype)

            bfi_args = " -trigger %d -cmd %s -seed 1 -mask %d" % (instr, fault, mask)
            cs = os.path.join(self.test.get_cs_dir(), "%s.%d" % (key, i))
            bfi_cmd = self.bfi + bfi_args + " -- " + self.test.commands[key] + (" --cso %s" % cs)
            logfile.write("... no. %d: running %s ...\n" % (i, bfi_cmd))
            p = Process(bfi_cmd)
            retcode, _, stderr = p.run(self.timeout, logfile)
            logfile.write("... no. %d: finished running %s ...\n" % (i, bfi_cmd))

            # write "stderr" from running "bfi" to file:
            fi_name = os.path.join(self.test.outputs_dir, self.test.get_name() + (".%s.fi.%d" % (key, i)))
            fi_file = open(fi_name, "w")
            logfile.write("... no. %d: writing <stderr> to %s ...\n" % (i, fi_name))
            fi_file.write("<stderr> from running %s:\n" % bfi_cmd)
            fi_file.write(stderr)
            fi_file.write(SEPARATOR)
            fi_file.close()
            logfile.write("... no. %d: done writing <stderr> to %s ...\n" % (i, fi_name))
            logfile.write("... no. %d: cso file: %s ...\n" % (i, cs))

            checksum = p.extract_checksum()
            is_valid = valid_fault(func_ranges, retcode, stderr)
            kind = self.result.diagnose(retcode, is_valid, checksum, ref_checksum, cs, ref_file)
            logfile.write("... no. %d: %s (retcode=%s) %s ...\n" % (i, kind, str(retcode), bfi_cmd))

        logfile.write("... finished fault injections.\n")
        logfile.write(SEPARATOR)

        logfile.write("Result:\n")
        self.result.write(logfile)
        logfile.write(SEPARATOR)
        return


#===============================================================================
# MAIN
#===============================================================================
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--directory",
                        default=r".",
                        help="directory to be searched for test cases")
    parser.add_argument("-o", "--outd",
                        default=r".",
                        help="directory for output files")
    parser.add_argument("--pin",
                        default=r"./pin.sh",
                        help="path to Intel PIN tool")
    parser.add_argument("--bfi",
                        default=r"./bfi.so",
                        help="path to 'bfi' plugin ('bfi.so')")
    parser.add_argument("-s", "--summary",
                        default=r"",
                        help="filename for test suite summary")

    args = parser.parse_args()
    suite = testcase.TestSuite(args.directory, args.outd)
    if args.summary == "":
        args.summary = os.path.join(suite.get_outd(), "coverage.summary")
    bfi = "%s -t %s" % (args.pin, args.bfi)

    results = dict()
    for test in suite.get_tests_with_profile("cover"):
        results[test.get_name()] = []

        # Get the reference output from a 'plain' run:
        print "Golden 'plain' run ..."
        ref_file = os.path.join(test.get_cs_dir(), "golden.plain")
        p = Process(test.commands["plain"] + (" --cso %s" % ref_file))
        retcode, _, _ = p.run()
        if retcode != 0:
            raise Exception("Coverage", test.commands["plain"])
        print "... finished golden run."
        ref_checksum = p.extract_checksum()

        fi = FaultInjector(test, bfi)

        ref = os.path.join(test.get_outputs_dir(), test.get_name() + ".ref")
        ref_logfile = open(ref, "w")
        ref_logfile.write("reference checksum=0x%X\n" % ref_checksum)
        ref_logfile.write("reference file: %s\n" % ref_file)
        ref_logfile.write(SEPARATOR)
        fi.run("plain", "RANDOM_8BITS", ref_checksum, ref_file, ref_logfile)
        results[test.get_name()].append(("ref", fi.get_result()))
        ref_logfile.close()

        cov = os.path.join(test.get_outputs_dir(), test.get_name() + ".cov")
        cov_logfile = open(cov, "w")
        cov_logfile.write("reference checksum=0x%X\n" % ref_checksum)
        cov_logfile.write("reference file: %s\n" % ref_file)
        cov_logfile.write(SEPARATOR)
        fi.run("encoded", "RANDOM_8BITS", ref_checksum, ref_file, cov_logfile)
        results[test.get_name()].append(("cov", fi.get_result()))
        cov_logfile.close()

    summary = open(args.summary, "w")
    for name in results.keys():
        ref_result = [r[1] for r in results[name] if r[0] == "ref"]
        assert(len(ref_result) == 1)
        cov_result = [r[1] for r in results[name] if r[0] == "cov"]
        assert(len(cov_result) == 1)

        summary.write("test name: %s\n" % name)
        summary.write("reference result:\n")
        ref_result[0].write(summary, "\t")
        summary.write("coverage result:\n")
        cov_result[0].write(summary, "\t")
        summary.write(SEPARATOR)
    summary.close()
