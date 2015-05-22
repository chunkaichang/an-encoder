#! /usr/bin/python

import argparse
import os
import shutil
import statistics
import subprocess


def add_result(results, test_name, slow_down, length):
    if test_name not in results.keys():
        results[test_name] = dict()
    if length not in results.keys():
        results[test_name][length] = []
    results[test_name][length].append(float(slow_down))


def write_results(results, path):
    for test_name in results.keys():
        file_path = os.path.join(path, test_name)
        file = open(file_path, "w")
        for k in results[test_name].keys():
            mean = statistics.mean(results[test_name][k])
            file.write("%s\t%f\n" % (k, mean))
        file.close()


def make(source_dir, build_type="RELEASE", length=10):
    cmd = "cmake -DLENGTH_PERF=%s -DCMAKE_BUILD_TYPE=%s -L %s" % (length, build_type, source_dir)
    p = subprocess.Popen(cmd, shell=True)
    p.communicate()
    cmd = "make -j 4"
    p = subprocess.Popen(cmd, shell=True,
                         stdout=open(os.devnull, "w"),
                         stderr=open(os.devnull, "w"))
    p.communicate()
    if p.returncode != 0:
        raise Exception("make", "make failed")


def analyze(results, summary_path, length):
    file = open(summary_path, "r")
    lines = file.readlines()
    file.close()
    while len(lines) > 0:
        l = lines.pop(0)
        id = l.split(':')[0].strip()
        if "base name for test" in id:
            test_name = l.split(':')[1].strip()
            while "-------" not in l:
                l = lines.pop(0)
                if "slow-down" in l:
                    slow_down = l.split(':')[1].strip()
                    add_result(results, test_name, slow_down, length)


def perf(results, script_dir, tests_dir, out_dir, length):
    script = os.path.join(script_dir, "runner.py")
    cmd = "%s --stats -d %s -o %s" % (script, tests_dir, out_dir)
    p = subprocess.Popen(cmd, shell=True)
    p.communicate()

    def _analyze(arg, dirname, names):
        if "runner.summary" not in names:
            return

        results, length = arg
        summary = os.path.join(dirname, "runner.summary")
        analyze(results, summary, length)

    def _sweep(arg, dirname, names):
        if "cs" not in names:
            return

        shutil.rmtree(os.path.join(dirname, "cs"))

    os.path.walk(out_dir, _analyze, (results, length))
    os.path.walk(out_dir, _sweep, None)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--directory",
                        default=r".",
                        help="directory to be searched for test cases")
    parser.add_argument("-b", "--build",
                        default=r"RELEASE",
                        help="CMake build type")

    parser.add_argument("-o", "--outd",
                        default=r".",
                        help="directory for output files")
    parser.add_argument("--scripts",
                        default=r".",
                        help="directory of script 'runner.py'")
    parser.add_argument("--source",
                        default=r".",
                        help="path to sources, i.e. top level directory in CMake hierarchy")

    args = parser.parse_args()

    results = dict()
    """lengths = ['10', '50', '100', '500', '1000', '2000', '3000', '5000', '7000', '9000',
               '10000', '15000', '20000', '30000', '50000', '100000']
    """
    lengths = []
    """for i in range(10, 4000, 10):
        lengths.append(i)
    for i in range(500, 3000, 200):
        lengths.append(i)
    for i in range(3000, 8000, 500):
        lengths.append(i)
    for i in range(8000, 16000, 1000):
        lengths.append(i)
    """
    for i in range(10000, 100000, 5000):
        lengths.append(i)

    for l in lengths:
        make(args.source, args.build, l)
        tests_outdir = os.path.join(".", "LENGTH.%s" % str(l))
        perf(results, args.scripts, args.directory, tests_outdir, l)

    write_results(results, args.outd)
