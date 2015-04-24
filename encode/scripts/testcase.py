#! /usr/bin/python

import argparse
import os
import subprocess
import sys
import time


class TestCase:
    commands = ["plain", "encoded"]
    profiles = ["veri", "perf", "cover"]

    def __init__(self, cfg_file, outd):
        if not cfg_file.name.endswith(".cfg"):
            sys.stderr.write("Unexpected file format: " + cfg_file.name + "\n")
            raise Exception("TestCase", path)

        self.path = cfg_file.name
        test_file = os.path.split(self.path)[1]
        test_ids  = test_file.rsplit('.', 2)
        self.name = test_ids[0] + '.' + test_ids[1]
        self.base_name = test_ids[0]
        if not test_ids[1] in TestCase.profiles:
            raise Exception("TestCase", "undefined profile: %s" % test_ids[1])
        self.profile = test_ids[1]

        self.commands = dict()
        cfg_file.seek(0)
        for line in cfg_file:
            id  = line.split(':')[0].strip()
            val = line.split(':')[1].strip()
            if id in TestCase.commands:
                self.commands[id] = val
            elif id == "runs":
                self.runs = int(val)

        if not set(TestCase.commands).issubset(self.commands.keys()):
            excp_msg = "Test case %s does not specify \'plain\' *and* \'encoded\' profiles." % self.path
            raise Exception("TestCase", excp_msg)

        self.outputs_dir = os.path.join(outd, self.base_name, self.profile)
        os.makedirs(self.outputs_dir)
        self.cs_dir = os.path.join(self.outputs_dir, "cs")
        os.makedirs(self.cs_dir)

    def get_path(self):
        return self.path

    def get_name(self):
        return self.name

    def get_base_name(self):
        return self.base_name

    def get_command(self, id):
        return self.commands[id]

    def get_runs(self):
        return self.runs

    def get_outputs_dir(self):
        return self.outputs_dir

    def get_cs_dir(self):
        return self.cs_dir

    def __repr__(self):
        return self.name + " @path: " + self.path

    @staticmethod
    def extract_info(lines):
        cycles, checksum = 0, 0
        for l in lines:
            if "elapsed cycles" in l:
                val = l.split(':')[1].strip()
                cycles = int(val, 10)
            elif "checksum" in l:
                val = int(l.split('=')[1].strip(), 16)
                if "hi" in l:
                    val <<= 64
                checksum += val
        return cycles, checksum

    @staticmethod
    def diff_files(file1, file2):
        # Suppress verbose output, we are only interested in the return value of 'diff':
        cmd = "diff %s %s &> /dev/null" % (file1, file2)
        return subprocess.call(cmd, shell=True)


class VeriTestCase(TestCase):
    def __init__(self, cfg_file, outd):
        TestCase.__init__(self, cfg_file, outd)

        if not self.profile == "veri":
            excp_msg = "Wrong profile (%s) in verification test %s" % (self.profile, self.path)
            raise Exception("PerfTestCase", excp_msg)


class PerfTestCase(TestCase):
    def __init__(self, cfg_file, outd):
        TestCase.__init__(self, cfg_file, outd)

        if not self.profile == "perf":
            excp_msg = "Wrong profile (%s) in performance test %s" % (self.profile, self.path)
            raise Exception("PerfTestCase", excp_msg)


class CoverTestCase(TestCase):
    def __init__(self, cfg_file, outd):
        TestCase.__init__(self, cfg_file, outd)

        if not self.profile == "cover":
            excp_msg = "Wrong profile (%s) in coverage test %s" % (self.profile, self.path)
            raise Exception("CoverTestCase", excp_msg)

        cfg_file.seek(0)
        for line in cfg_file:
            id  = line.split(':')[0].strip()
            val = line.split(':')[1].strip()
            if id == "function":
                self.function = val
            elif id == "warmups":
                self.warmups = int(val)

    def get_function(self):
        return self.function

    def get_warmups(self):
        return self.warmups


class TestSuite:
    def __init__(self, path, output_dir):
        if not os.path.isdir(path):
            excp_msg = "Directory %s does not exist." % path
            raise Exception("TestSuite", excp_msg)

        self.tests = dict()

        def _add_test(arg, dirname, names):
            for filename in names:
                if not filename.endswith(".cfg"):
                    continue

                cfg_path = os.path.join(dirname, filename)
                cfg_file = open(cfg_path, "r")
                if filename.endswith(".perf.cfg"):
                    test = PerfTestCase(cfg_file, arg)
                elif filename.endswith(".cover.cfg"):
                    test = CoverTestCase(cfg_file, arg)
                elif filename.endswith(".veri.cfg"):
                    test = VeriTestCase(cfg_file, arg)
                else:
                    excp_msg = "Found configuration file for undefined profile: %s" % cfg_path
                    raise Exception("TestSuite", excp_msg)

                # We use the full path of the cfg-file as key since it is unique:
                self.tests[cfg_path] = test

        self.outd = os.path.join(output_dir, time.strftime("%Y-%m-%d_%H:%M:%S", time.localtime()))
        os.path.walk(path, _add_test, self.outd)

    def get_tests(self):
        return self.tests

    def get_tests_with_profile(self, profile):
        return filter(lambda x: x.profile == profile, self.tests.values())

    def get_names(self):
        return self.tests.keys()

    def get_outd(self):
        return self.outd

    def __repr__(self):
        result = ""
        for k, v in self.tests.iteritems():
            result += k + ": " + v.__repr__() + "\n"
        return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--directory",
                        default=r".",
                        help="directory to be searched for test cases")
    parser.add_argument("-o", "--outd",
                        default=r".",
                        help="directory for output files")
    args = parser.parse_args()

    suite = TestSuite(args.directory, args.outd)

    print "Verification tests:"
    print suite.get_tests_with_profile("veri")
    print "----------------------------------"

    print "Perfomance tests:"
    print suite.get_tests_with_profile("perf")
    print "----------------------------------"

    print "Coverage tests:"
    print suite.get_tests_with_profile("cover")
    print "----------------------------------"
