#! /usr/bin/python

import argparse
import os
import shutil
import statistics
import subprocess

dispres = ["CORRECT", "OSCRASH", "ANCRASH", "HANG", "UNEXPECTED", "NONDIAG"]
results = ["INVALID", "CORRECT", "UNEXPECTED", "HANG", "OSCRASH", "ANCRASH", "NONDIAG"]

regs_only = False
no_txt = False

def collect_coverage(dirname):
    global results

    tests = dict()
    tests["plain"], tests["encoded"] = dict(), dict()
    test_dirs = [d for d in os.listdir(dirname) if os.path.isdir(os.path.join(dirname, d))]
    for test in test_dirs:
        for kind in ["plain", "encoded"]:
            log_name = os.path.join(dirname, test, "cover", "%s.cover.%s.log" % (test, kind))
            if os.path.exists(log_name): print "Found log file: %s" % log_name
            else: continue

            res = tests[kind]["%s" % test] = dict()
            for r in results:
                res[r] = 0

            log_file = open(log_name, "r")
            for line in log_file.readlines():
                for r in results:
                    if r not in line:
                      continue
                    elif regs_only or no_txt:
                        if regs_only and ("RREG" in line or "WREG" in line):
                          res[r] += 1
                        elif no_txt and ("TXT" not in line):
                          res[r] += 1
                    else:
                      res[r] += 1
            log_file.close()

            sum = 0
            for r in dispres:
                sum += res[r]
            res["SUM"] = sum
            res["ERRSUM"] = sum - res["CORRECT"]
    return tests


def write_histos(testres, out_name):
    global results
    
    columns = dispres + ["SUM", "ERRSUM"]
    for kind in ["plain", "encoded"]:
      out_file = open("%s.%s.histo" % (out_name, kind) , "w")

      out_file.write("TEST ")
      for c in columns:
        out_file.write(c + " ")
      out_file.write("\n")

      for t in sorted(testres[kind].keys()):
        out_file.write(t + " ")
        for c in columns:
          out_file.write(str(testres[kind][t][c]) + " ")
        out_file.write("\n")
    out_file.close()

def write_histo(testres, out_name):
    global results
    
    columns = dispres + ["SUM", "ERRSUM"]
    out_file = open("%s.combined.histo" % out_name , "w")
      
    out_file.write("TEST ")
    for c in columns:
      out_file.write(c + " ")
    out_file.write("\n")
    
    for t in sorted(testres["plain"].keys()):
      for kind in ["plain", "encoded"]:
        out_file.write(t + "(%s) " % kind[0])
        for c in columns:
          out_file.write(str(testres[kind][t][c]) + " ")
        out_file.write("\n")
    out_file.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--directory",
                        default=r".",
                        help="directory to be traversed for *.cover.*.log files")
    parser.add_argument("-o", "--output",
                        default=r"",
                        help="output path")
    parser.add_argument("--regs",
                        action='store_true',
                        help="only count experiments that injected '{W|R}REG' faults")
    parser.add_argument("--no_txt",
                        action='store_true',
                        help="disregard 'TXT' faults")

    args = parser.parse_args()

    regs_only = args.regs
    no_txt = args.no_txt

    test_res = collect_coverage(args.directory)
    write_histo(test_res, args.output)
    write_histos(test_res, args.output)
