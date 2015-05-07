#!/usr/bin/env python

""" Based on Dmitry's "bfi.py" """

import argparse
import cStringIO
from multiprocessing import Lock, Manager, Queue
import os
import re
import random
import statistics

import signal
import subprocess
import testcase
import time
import utilities


SEPARATOR = "----------------------------------------------------------------\n"


class FiResult:
    def __init__(self, test, key, func_ranges, checksum, cso):
        self.test = test
        self.key = key
        self.func_ranges = func_ranges
        self.checksum = checksum  # reference checksum
        self.cso = cso  # path to reference "cso" file
        self.hist = utilities.ConcurrentDict()

        self.hist["TOTAL"] = 0  # total number of fault injections

        self.hist["VALID"] = 0  # number of valid fault injections (i.e. where error was inserted in the right function)

        self.hist["CORRECT"] = 0  # number of programs unaffected by fault injection

        self.hist["ANFAILURE"] = 0  # number of undetected faults (i.e. SDC):
        self.hist["UNEXPECTED"] = 0  # number of unexpected program results
        self.hist["HANG"] = 0  # number of hanging programs (more than 10x runtime than without injected faults)

        self.hist["ANSUCCESS"] = 0  # number of detected faults:
        self.hist["OSCRASH"] = 0  # fault detected by OS
        self.hist["ANCRASH"] = 0  # fault detected by AN encoder
        self.hist["NONDIAG"] = 0  # undiagnosed, i.e. weird return code

    def _inc_total(self):
        self.hist["TOTAL"] += 1

    def _inc_valid(self):
        self.hist["VALID"] += 1

    def _inc_correct(self):
        self.hist["CORRECT"] += 1

    def _inc_failure(self, kind):
        if kind == "UNEXPECTED" or kind == "HANG":
            self.hist[kind] += 1
        else:
            raise NameError("Failure kind %s undefined." % kind)
        self.hist["ANFAILURE"] += 1
        return

    def _inc_success(self, kind):
        if kind in ["OSCRASH", "ANCRASH", "NONDIAG"]:
            self.hist[kind] += 1
        else:
            raise NameError("Success kind %s undefined." % kind)
        self.hist["ANSUCCESS"] += 1
        return

    def _valid_fault(self, retcode, stderr):
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
                assert (s2 != "")
                if (re.match('.*enter', s2) is None and
                            re.match('.*leave', s2) is None):
                    trig_instr = trigger
                    break

        for func_range in self.func_ranges:
            if func_range[0] <= trig_instr <= func_range[1]:
                return True
            elif trig_instr < func_range[0]:
                """ Note that this "elif" statement only makes sense if 'func_range' is sorted in ascending order,
                which is indeed achieved by our implementation of '_get_function_ranges'. """
                return False
        return False

    def diagnose(self, limitedp, cso_file):
        self._inc_total()

        retcode, _, stderr = limitedp.get_result()
        if not self._valid_fault(retcode, stderr):
            return "INVALID"
        self._inc_valid()

        _, checksum = testcase.TestCase.extract_info(stderr.split('\n'))
        if retcode is None:
            kind = "HANG"
            self._inc_failure(kind)
        elif retcode == 0:
            check = checksum != self.checksum
            diff = testcase.TestCase.diff_files(cso_file, self.cso)
            # Fault did not crash the program.
            if diff:
                # Output is not correct: Silent Data Corruption.
                kind = "UNEXPECTED"
                self._inc_failure(kind)
                kind += " (checksum difference: %d)" % check
            else:
                # Output is correct: Fault didn't affect execution.
                kind = "CORRECT"
                self._inc_correct()
                kind += " (checksum difference: %d)" % check
        elif retcode == 1:
            kind = "ERROR"
        elif retcode == 2:
            kind = "ANCRASH"
            self._inc_success(kind)
        elif retcode > 10:
            kind = "OSCRASH"
            self._inc_success(kind)
        else:
            kind = "NONDIAG"
        return kind

    def write(self, logfile, prefix=""):
        hist = self.hist
        logfile.write("%stotal   = %d\n" % (prefix, hist["TOTAL"]))
        logfile.write("%svalid   = %d\n" % (prefix, hist["VALID"]))
        logfile.write("%scorrect = %d\n" % (prefix, hist["CORRECT"]))
        logfile.write("%sAN success = %d\n" % (prefix, hist["ANSUCCESS"]))
        logfile.write("%s    OS crashes  = %d\n" % (prefix, hist["OSCRASH"]))
        logfile.write("%s    AN crashes  = %d\n" % (prefix, hist["ANCRASH"]))
        logfile.write("%s    undiagnosed = %d\n" % (prefix, hist["NONDIAG"]))
        logfile.write("%sAN failed = %d\n" % (prefix, hist["ANFAILURE"]))
        logfile.write("%s    unexpected = %d\n" % (prefix, hist["UNEXPECTED"]))
        logfile.write("%s    hang       = %d\n" % (prefix, hist["HANG"]))
        logfile.flush()
        return

    def get_test(self):
        return self.test

    def get_key(self):
        return self.key

    def get_hist(self):
        return self.hist


class CmdParams:
    def __init__(self, trigger="", fault="", mask=""):
        if trigger != "": self.trigger = int(trigger)
        else: self.trigger = None
        if fault != "": self.fault = fault
        else: self.fault = None
        if mask != "": self.mask = int(mask)
        else: self.mask = None

    def unpack(self):
        return self.trigger, self.fault, self.mask


def bfi_worker(timeout, command, cso_name, std_prefix, logfile, log_prefix, result=None):
    def log(output):
        logfile.write(output, log_prefix, timed=True)

    def flush():
        logfile.flush()

    log("running %s ...\n" % command)
    p = utilities.LimitedProcess(command)
    retcode, stdout, stderr = p.run(timeout, logfile, log_prefix)
    log("finished running %s ...\n" % command)

    # write "stderr" and "stdout" from running "bfi" to file:
    def write_output(label, source, target_path):
        log("writing <%s> to %s ...\n" % (label, target_path))
        file = open(target_path, "w")
        file.write("<%s> from running %s:\n" % (label, command))
        file.write(source)
        file.write(SEPARATOR)
        file.close()
        log("done writing <%s> to %s ...\n" % (label, target_path))
        return

    write_output("stderr", stderr, std_prefix + ".stderr")
    write_output("stdout", stdout, std_prefix + ".stdout")

    kind = None
    if result:
        kind = result.diagnose(p, cso_name)
    log("%s (retcode=%s) %s ...\n" % (str(kind), str(retcode), command))
    flush()
    return


class FaultInjector:
    class FiCmdFactory():
        def __init__(self, bfi, binary):
            self.bfi = bfi
            self.binary = binary

        @staticmethod
        def _get_cs_suffix(cso, csl):
            suffix = ""
            if cso: suffix += " --cso %s" % cso
            if csl: suffix += " --csl %s" % csl
            return suffix

        def get_function_cmd(self, function, cso=None, csl=None):
            bfi_args = " -m %s" % function
            bfi_cmd = self.bfi + bfi_args + " -- " + \
                self.binary + self._get_cs_suffix(cso, csl)
            return bfi_cmd

        def get_timing_cmd(self, cso=None, csl=None):
            bfi_cmd = self.bfi + " -- " + \
                self.binary + self._get_cs_suffix(cso, csl)
            return bfi_cmd

        def get_fi_cmd(self, trigger, fault, mask, cso=None, csl=None):
            bfi_args = " -trigger %d -cmd %s -seed 1 -mask %d" % (trigger, fault, mask)
            bfi_cmd = self.bfi + bfi_args + " -- " + \
                self.binary + self._get_cs_suffix(cso, csl)
            return bfi_cmd

    def __init__(self, test, key, bfi, logfile, processes=1, cmd_params=None):
        self.test = test
        self.key = key
        self.bfi_factory = FaultInjector.FiCmdFactory(bfi, test.commands[key])
        self.logfile = logfile
        self.processes = processes
        self.params = cmd_params
        self.func_ranges = None

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

    def log(self, output):
        self.logfile.write(output)
    
    def flush(self):
        self.logfile.flush()

    def get_logfile_fd(self):
        return self.logfile.get_fd()

    def get_logfile_lock(self):
        return self.logfile.get_lock()

    def _warmup(self):
        self.log("Warming up ...\n")
        self.flush()
        cso = "/dev/null"
        command = self.bfi_factory.get_timing_cmd(cso, "/dev/null")

        timings = []
        for i in range(self.test.warmups):
            """ To obtain meaningful timing information (which will later be used to set the timeout for tests), we need
                to execute as many "timing runs" in parallel as the number of tests we will later run in parallel. That
                number is 'self.processes'. """
            args = []
            for j in range(self.processes):
                stdlog = os.path.join(self.test.outputs_dir, self.test.get_name() + (".%s.timing.%d.%d" % (self.key, i, j)))
                prefix = " ... timing (%d,%d): " % (i, j)
                args.append((None, command, cso, stdlog, self.logfile, prefix,))

            alp = utilities.ArgListProcessor(self.processes, bfi_worker, args)
            start = time.time()
            alp.run()
            end = time.time()
            timings.append(end - start)

        self.log("... done with timing runs ...\n")
        mean = statistics.mean(timings)
        stdev = statistics.stdev(timings)
        """ We set the timeout for faulty runs to 10x the time a sane run takes. The motivation for this is that if a
            fault-injected run takes more than 10x the runtime of the corresponding sane run, we might as well regard
            this as an undetected error since the use may not be prepared to wait that long.
            ("+ 1.0" necessary in case "10.0*mean" is still below one second.) """
        self.timeout = int(10.0 * mean + 1.0)
        self.log("timings: mean=%f, stdev=%f, timeout=%d\n" % (mean, stdev, self.timeout))
        self.log("... done warming up\n")
        self.log(SEPARATOR)
        self.flush()

    def _get_function_ranges(self):
        # find the range of the "coverage_function":
        self.log("Obtaining ranges of function %s ...\n" % self.test.get_function())
        command = self.bfi_factory.get_function_cmd(self.test.get_function(), "/dev/null", "/dev/null")

        p = utilities.LimitedProcess(command)
        _, _, stderr = p.run(None, self.logfile)
        self.log("... finished running %s ...\n" % command)

        # write "stderr" from running "bfi" to file:
        rngs_name = os.path.join(self.test.outputs_dir, self.test.get_name() + (".%s.ranges" % self.key))
        self.log("... writing <stderr> to %s ...\n" % rngs_name)
        rngs_file = open(rngs_name, "w")
        rngs_file.write("<stderr> from running %s:\n" % command)
        rngs_file.write(stderr)
        rngs_file.write(SEPARATOR)
        self.log("... done writing <stderr> to %s ...\n" % rngs_name)
        self.flush()

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
                assert (sf != "")
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
        rngs_file.write(SEPARATOR)
        rngs_file.close()

        self.log("... done obtaining ranges of function %s\n" % self.test.get_function())
        self.log(SEPARATOR)
        self.flush()
        return func_ranges

    def run(self, key, mask_type, ref_checksum, ref_cso_file):
        """ 'mask_type' is one of { RANDOM_8BITS, RANDOM_32BITS, RANDOM_8BITS, RANDOM_1BITFLIP, RANDOM_2BITFLIPS } """
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

        self._warmup()
        self.func_ranges = self._get_function_ranges()

        self.log("Performing fault injections ...\n")
        # do NOT inject CF -- consider them Control Flow errors, not Data Flow
        inject_faults = list(set(FaultInjector.faults) - set(['CF']))

        result = FiResult(self.test, self.key, self.func_ranges, ref_checksum, ref_cso_file)
        resq = Queue()
        resq.put(result)
        args = []
        for i in range(self.test.get_runs()):
            trigger, fault, mask = self.params.unpack()
            if trigger is None:
                index = random.randint(0, len(self.func_ranges) - 1)
                trigger = random.randint(self.func_ranges[index][0], self.func_ranges[index][1])
            if fault is None:
                fault = inject_faults[random.randint(0, len(inject_faults) - 1)]
            if mask is None:
                mask = get_random_mask(mask_type)

            cs = os.path.join(self.test.get_cs_dir(), "%s.%d" % (key, i))
            cso, csl = cs + ".cso", cs + ".csl"
            self.log("... no. %d: cso file: %s ...\n" % (i, cso))
            self.log("... no. %d: csl file: %s ...\n" % (i, csl))
            self.flush()

            command = self.bfi_factory.get_fi_cmd(trigger, fault, mask, cso, csl)

            stdfile_prefix = os.path.join(self.test.outputs_dir, self.test.get_name() + (".%s.fi.%d" % (key, i)))
            prefix = "... no. %d:" % i

            args.append((self.timeout, command, cso, stdfile_prefix, self.logfile, prefix, result,))

        alp = utilities.ArgListProcessor(self.processes, bfi_worker, args)
        alp.run()

        self.log("... finished fault injections.\n")
        self.log(SEPARATOR)

        self.log("Result:\n")
        result.write(self.logfile)
        self.log(SEPARATOR)
        self.flush()
        return result


def plain_run(test):
    logname = os.path.join(test.get_outputs_dir(), test.get_name() + ".golden.plain.log")
    logfile = utilities.ConcurrentFile(logname)

    logfile.write("Golden 'plain' run ...")
    ref_cs_file_prefix = os.path.join(test.get_cs_dir(), "golden.plain")
    ref_cso_file, ref_csl_file = ref_cs_file_prefix + ".cso", ref_cs_file_prefix + ".csl"

    p = utilities.LimitedProcess(test.commands["plain"] + (" --cso %s --csl %s" % (ref_cso_file, ref_csl_file)))
    retcode, _, stderr = p.run(None, logfile)
    if retcode != 0:
        raise Exception("Coverage", test.commands["plain"])
    logfile.write("... finished golden run.")

    _, checksum = testcase.TestCase.extract_info(stderr.split("\n"))
    return checksum, ref_cso_file


def fi_run(test, key, ref_checksum, ref_cso_file, processes, summary):
    logname = os.path.join(test.get_outputs_dir(), test.get_name() + "." + key + ".log")
    logfile = utilities.ConcurrentFile(logname)

    logfile.write("reference checksum=0x%X\n" % ref_checksum)
    logfile.write("reference file: %s\n" % ref_cso_file)
    logfile.write(SEPARATOR)
    logfile.flush()

    fi = FaultInjector(test, key, bfi, logfile, processes, cmd_params)
    result = fi.run(key, "RANDOM_8BITS", ref_checksum, ref_cso_file)

    summary.write("test name: %s, key=%s\n" % (test.get_name(), key))
    result.write(summary, "\t")
    summary.write(SEPARATOR)
    summary.flush()
    return


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
    parser.add_argument("--trigger",
                        default=r"",
                        help="instruction counter where fault injection is triggered")
    parser.add_argument("--cmd",
                        default=r"",
                        help="kind of fault to be injected")
    parser.add_argument("--mask",
                        default=r"",
                        help="mask for fault injection")
    parser.add_argument("-p0", "--processes0",
                        default=r"1",
                        help="number of processes across which the test cases should be spread")
    parser.add_argument("-p1", "--processes1",
                        default=r"1",
                        help="number of processes for parallel fualt injection runs")

    args = parser.parse_args()
    suite = testcase.TestSuite(args.directory, args.outd)
    if args.summary == "": args.summary = os.path.join(suite.get_outd(), "coverage.summary")
    bfi = "%s -t %s" % (args.pin, args.bfi)
    cmd_params = CmdParams(args.trigger, args.cmd, args.mask)

    print("Changing ptrace_scope to 0...")
    subprocess.call('echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope > /dev/null', shell=True)
    print("Changing randomize_va_space to 0...")
    subprocess.call('echo 0 | sudo tee /proc/sys/kernel/randomize_va_space > /dev/null', shell=True)

    summary = utilities.ConcurrentFile(args.summary)
    fi_args = []
    for test in suite.get_tests_with_profile("cover"):
        checksum, cso = plain_run(test)
        for key in ["plain", "encoded"]:
            fi_args.append((test, key, checksum, cso, int(args.processes1), summary,))

    alp = utilities.ArgListProcessor(int(args.processes0), fi_run, fi_args)
    alp.run()
