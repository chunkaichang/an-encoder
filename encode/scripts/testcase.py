#! /usr/bin/python

import argparse
import os
import sys
import subprocess
import time


class TestCase:
    def __init__(self, path):
        if not os.path.isfile(path):
            sys.stderr.write("File " + path + "does not exist.\n")
            raise Exception("TestCase", path)
        if not path.endswith(".cfg"):
            sys.stderr.write("Unexpected file format: " + path + "\n")
            raise Exception("TestCase", path)

        self.path = path
        self.name = path
        # possible commands: "plain", "encode"
        commands = ["plain", "encoded"]
        self.commands = dict()
        # possible outputs: "veri", "stat", "ref", "cov"
        outputs = ["veri", "stat", "ref", "cov"]
        self.outputs = dict()
        self.performance_runs = 10
        self.coverage_runs = 100

        cfg_file = open(path, "r")
        for line in cfg_file.readlines():
            id  = line.split(':')[0].strip()
            val = line.split(':')[1].strip()
            if id in commands:
                self.commands[id] = val
            elif id in outputs:
                self.outputs[id] = val
            elif id == "name":
                self.name = val
            elif id in ["run", "performance_runs"]:
                self.performance_runs = int(val)
            elif id == "coverage_runs":
                self.coverage_runs = int(val)
            elif id == "timing_runs":
                self.timing_runs = int(val)
            elif id == "coverage_function":
                self.coverage_function = val
            else:
                sys.stderr.write("Unexpected configuration for test case " + path + "\n")
                raise Exception("TestCase", path)
        cfg_file.close()

        if not set(commands).issubset(self.commands.keys()):
            sys.stderr.write("Test case " + self.name + " does not specify \'plain\' *and* \'encoded\' profiles.\n")
            raise Exception("TestCase", path)

        directory = os.path.split(path)[0]
        self.cfg_name = os.path.split(path)[1].rsplit('.', 1)[0]
        fmt_time = time.strftime("%Y-%m-%d_%H:%M:%S", time.gmtime())
        self.outputs_dir = os.path.join(directory, fmt_time)
        os.makedirs(self.outputs_dir)
        for k in set(outputs).difference(self.outputs.keys()):
            self.outputs[k] = os.path.join(self.outputs_dir, self.cfg_name + "." + k)

    def get_name(self):
        return self.name

    def get_command(self, id):
        return self.commands[id]

    def get_runs(self):
        return self.runs

    def __repr__(self):
        return self.name + " @path: " + self.path


class TestSuite:
    def __init__(self, path):
        if not os.path.isdir(path):
            sys.stderr.write("Directory " + path + "does not exist.")
            sys.exit(-1)

        self.tests = dict()

        def _add_test(arg, dirname, names):
            for filename in names:
                if not filename.endswith(".cfg"):
                    continue
                try:
                    cfg_filepath = os.path.join(dirname, filename)
                    test = TestCase(cfg_filepath)
                    # We use the full path of the cfg-file as key since it is unique:
                    self.tests[cfg_filepath] = test
                except:
                    continue

        os.path.walk(path, _add_test, None)

    def get_test(self, name):
        return self.tests[name]

    def get_tests(self):
        return self.tests

    def get_names(self):
        return self.tests.keys()

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
    args = parser.parse_args()

    suite = TestSuite(args.directory)
    print suite
