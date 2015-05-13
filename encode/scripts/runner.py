#! /usr/bin/python

import argparse
import os
import statistics
import subprocess
import testcase
import time

SEPARATOR = "----------------------------------------------\n"


class TestRunner:
    def __init__(self, test, runs=None):
        self.test = test
        self.cycles, self.checks, self.cs = dict(), dict(), dict()
        if runs:
            self.runs = runs
        else:
            self.runs = test.runs

    def run(self, cso=True, logfile=None):
        if not logfile:
            logfile = os.sys.stdout

        for k in self.test.commands.keys():
            logfile.write("key=%s, runs=%d ...\n" % (k, self.runs))
            logfile.write("command=%s\n" % self.test.commands[k])
            logfile.write(SEPARATOR)

            self.cycles[k], self.checks[k], self.cs[k] = [], [], []
            for i in range(self.runs):
                if cso: cs = os.path.join(self.test.get_cs_dir(), "%s.%d" % (k, i))
                else:   cs = os.devnull
                cmd = self.test.commands[k] + (" --cso %s" % cs)
                p = subprocess.Popen(cmd.split(),
                                     bufsize=-1,
                                     stdout=open(os.devnull, "w"),
                                     stderr=subprocess.PIPE)

                _, stderr = p.communicate()

                cycles, checksum = testcase.TestCase.extract_info(stderr.split('\n'))
                self.cycles[k].append(cycles)
                self.checks[k].append(checksum)
                self.cs[k].append(cs)

                logfile.write("key=%s, no.=%d, <stderr>:\n" % (k, i))
                logfile.write(stderr)
                logfile.write("key=%s, no.=%d, <checksum>:\n" % (k, i))
                logfile.write("0x%X\n" % checksum)
                logfile.write("key=%s, no.=%d, cso file:\n" % (k, i))
                logfile.write("%s\n" % cs)
                logfile.write(SEPARATOR)

    def get_checks(self):
        return self.checks

    def get_cycles(self):
        return self.cycles

    def get_timings(self):
        return self.timings

    def get_cycle_stats(self, key):
        return statistics.mean(self.cycles[key]), statistics.stdev(self.cycles[key])

    def get_ratio(self):
        plain,   _ = self.get_cycle_stats("plain")
        encoded, _ = self.get_cycle_stats("encoded")
        return float(encoded)/float(plain)

    def statistics(self, logfile=None):
        """ Purge those measurements where the distance from the mean
            is greater than 5 times the standard deviation: """
        purge = True
        count = 0
        if not logfile:
            logfile = os.sys.stdout

        while purge:
            logfile.write("Purge cycle no. " + str(count) + "\n")
            count += 1

            mean, dev = dict(), dict()
            for k in self.test.commands.keys():
                mean[k], dev[k] = self.get_cycle_stats(k)

            purge = False
            for k0 in self.test.commands.keys():
                k1 = set(self.test.commands.keys()).difference(set([k0])).pop()
                assert(len(self.cycles[k0]) == len(self.cycles[k1]))
                i = 0
                while i < len(self.cycles[k0]):
                    dist = abs(self.cycles[k0][i] - mean[k0])
                    if dist > 5*dev[k0]:
                        logfile.write("Purging  ... " + k0 + ": " + str(self.cycles[k0][i]))
                        logfile.write(           ", " + k1 + ": " + str(self.cycles[k1][i]))
                        logfile.write(" ... reason: " + k0 + ", distance=" + str(dist) + "\n")
                        self.cycles[k0].pop(i)
                        self.cycles[k1].pop(i)
                        purge = True
                    else:
                        i += 1

        logfile.write("Nothing left to purge in this cycle ... done.\n")
        logfile.write("samples left: " + str(len(self.cycles[k0])) + "\n")
        logfile.write("mean:  " + str(mean) + "\n")
        logfile.write("stdev: " + str(dev) + "\n")
        logfile.write("mean slow-down: " + str(self.get_ratio()) + "\n")

    def verify(self, logfile=None):
        assert(len(self.test.commands.keys()) == 2)

        k0, k1 = self.test.commands.keys()[0], self.test.commands.keys()[1]
        success = True
        if not logfile:
            logfile = os.sys.stdout

        for i in range(len(self.checks[k0])):
            check = (self.checks[k0][i] != self.checks[k1][i])
            diff = testcase.TestCase.diff_files(self.cs[k0][i], self.cs[k1][i])

            if check or diff:
                logfile.write("index=" + str(i) + ":\n")
                if check:
                    logfile.write("\tchecksums differ:\n")
                    logfile.write("\t\t" + k0 + "=" + str(self.checks[k0][i]) + "\n")
                    logfile.write("\t\t" + k1 + "=" + str(self.checks[k1][i]) + "\n")
                    success = False
                if diff:
                    logfile.write("\tfiles differ:\n")
                    logfile.write("\t\t" + self.cs[k0][i] + "\n")
                    logfile.write("\t\t" + self.cs[k1][i] + "\n")
                    success = False

        logfile.write("Success: " + str(int(success)) + "\n")
        return success


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--directory",
                        default=r".",
                        help="directory to be searched for test cases")
    parser.add_argument("-o", "--outd",
                        default=r".",
                        help="directory for output files")
    parser.add_argument("-s", "--summary",
                        default=r"",
                        help="filename for test suite summary")
    parser.add_argument("--veri", action="store_true",
                        help="enable verification")
    parser.set_defaults(veri=False)
    parser.add_argument("--stats", action="store_true",
                        help="enable gathering of statistics")
    parser.set_defaults(veri=False)

    args = parser.parse_args()
    suite = testcase.TestSuite(args.directory, args.outd)
    if args.summary == "":
        args.summary = os.path.join(suite.get_outd(), "runner.summary")

    verified = dict()
    ratios = dict()

    if args.veri:
        for test in suite.get_tests_with_profile("veri"):
            runner = TestRunner(test)
            name = test.get_base_name()

            runlog = os.path.join(test.get_outputs_dir(), name + ".veri.log")
            print "Opening file for writing: %s" % runlog
            runlog_file = open(runlog, "w")
            runner.run(True, runlog_file)
            runlog_file.close()

            veri = os.path.join(test.get_outputs_dir(), name + ".veri")
            print "Opening file for writing: %s" % veri
            veri_file = open(veri, "w")
            verified[name] = (test.get_path(), runner.verify(veri_file))
            veri_file.close()

    if args.stats:
        for test in suite.get_tests_with_profile("perf"):
            runner = TestRunner(test)
            name = test.get_base_name()

            runlog = os.path.join(test.get_outputs_dir(), name + ".perf.log")
            print "Opening file for writing: %s" % runlog
            runlog_file = open(runlog, "w")
            runner.run(False, runlog_file)
            runlog_file.close()

            stats = os.path.join(test.get_outputs_dir(), name + ".stats")
            print "Opening file for writing: %s" % stats
            stats_file = open(stats, "w")
            runner.statistics(stats_file)
            stats_file.close()
            ratios[name] = (test.get_path(), runner.get_ratio())

    summary = open(args.summary, "w")
    keys = set(verified.keys()).union(ratios.keys())
    for base_name in list(keys):
        summary.write("base name for test: %s\n" % base_name)
        if args.veri:
            summary.write("\tverification test: %s\n" % verified[base_name][0])
            summary.write("\tverified: %d\n" % verified[base_name][1])
        if args.stats:
            summary.write("\tperformance test: %s\n" % ratios[base_name][0])
            summary.write("\tslow-down: %f\n" % ratios[base_name][1])
        summary.write(SEPARATOR)
    summary.close()




