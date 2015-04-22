#! /usr/bin/python

import argparse
import os
import statistics
import subprocess
import testcase
import time


class TestRunner:
    def __init__(self, test, runs=None):
        self.test = test
        self.cycles, self.checks, self.timings = dict(), dict(), dict()
        if runs:
            self.runs = runs
        else:
            self.runs = test.performance_runs

    def run(self, logfile=None):
        if not logfile:
            logfile = os.sys.stdout

        for k in self.test.commands.keys():
            self.cycles[k], self.checks[k], self.timings[k] = [], [], []
            for i in range(self.runs):
                p = subprocess.Popen(self.test.commands[k].split(),
                                     bufsize=-1,
                                     stdout=open(os.devnull, "w"),
                                     stderr=subprocess.PIPE)

                _, stderr = p.communicate()
                logfile.write(stderr)

                checksum = 0
                for line in stderr.split('\n'):
                    if "elapsed cycles" in line:
                        val = line.split(':')[1].strip()
                        self.cycles[k].append(int(val, 10))
                    elif "checksum" in line:
                        val = int(line.split('=')[1].strip(), 16)
                        if "hi" in line:
                            val <<= 64
                        checksum += val
                self.checks[k].append(checksum)
                logfile.write("checksum=" + hex(checksum) + "\n")

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
            if (self.checks[k0][i] != self.checks[k1][i]):
                logfile.write("index=" + str(i) + ", ")
                logfile.write(k0 + "=" + str(self.checks[k0][i]) + " ")
                logfile.write(k1 + "=" + str(self.checks[k1][i]) + "\n")
                success = False

        logfile.write("Success: " + str(int(success)) + "\n")
        return success


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--directory",
                        default=r".",
                        help="directory to be searched for test cases")
    args = parser.parse_args()
    suite = testcase.TestSuite(args.directory)

    for name in suite.get_names():
        test = suite.get_test(name)
        runner = TestRunner(test)

        print "Opening file for writing", test.outputs["veri"]
        f_veri = open(test.outputs["veri"], "w")
        print "Opening file for writing", test.outputs["stat"]
        f_stat = open(test.outputs["stat"], "w")

        runner.run(f_veri)
        runner.verify(f_veri)
        runner.statistics(f_stat)

        f_veri.close()
        f_stat.close()

        timings = runner.get_timings()
        for k in timings.keys():
            print "%s: mean=%f stddev=%f" % (k, statistics.mean(timings[k]), statistics.stdev(timings[k]))




