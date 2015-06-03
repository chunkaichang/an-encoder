#! /usr/bin/python

import argparse
import os
import shutil
import statistics
import subprocess

dispres = ["CORRECT", "OSCRASH", "ANCRASH", "HANG", "UNEXPECTED", "NONDIAG"]
results = ["INVALID", "CORRECT", "UNEXPECTED", "HANG", "OSCRASH", "ANCRASH", "NONDIAG"]


def collect_coverage(dirname):
    global results

    tests = dict()
    test_dirs = [d for d in os.listdir(dirname) if os.path.isdir(os.path.join(dirname, d))]
    for test in test_dirs:
        for kind in ["plain", "encoded"]:
            log_name = os.path.join(dirname, test, "cover", "%s.cover.%s.log" % (test, kind))
            if os.path.exists(log_name): print "Found log file: %s" % log_name
            else: continue

            res = tests["%s-%s" % (test, kind)] = dict()
            for r in results:
                res[r] = 0

            log_file = open(log_name, "r")
            for line in log_file.readlines():
                for r in results:
                    if r in line:
                        res[r] += 1
            log_file.close()

            sum = 0
            for r in dispres:
                sum += res[r]
            res["SUM"] = sum
    return tests


def write_histo(testres, out_name):
    global results

    out_file = open(out_name, "w")
    columns = dispres + ["SUM"]

    out_file.write("TEST ")
    for c in columns:
        out_file.write(c + " ")
    out_file.write("\n")

    for t in sorted(testres.keys()):
        out_file.write(t + " ")
        for c in columns:
            out_file.write(str(testres[t][c]) + " ")
        out_file.write("\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--directory",
                        default=r".",
                        help="directory to be traversed for *.cover.*.log files")
    parser.add_argument("-o", "--output",
                        default=r"",
                        help="output path")

    args = parser.parse_args()

    test_res = collect_coverage(args.directory)
    write_histo(test_res, args.output)
