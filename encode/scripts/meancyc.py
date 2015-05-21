#! /usr/bin/python

import argparse
import os
import shutil
import statistics
import subprocess


def walk_length_dirs(test_name, dirname):
    def extract_cycles(f):
        for line in f.readlines():
            if "mean:" not in line:
                continue

            data = line.split('{')[1].split('}')[0]
            plain = float(data.split(',')[0].split(':')[1].strip())
            encoded = float(data.split(',')[1].split(':')[1].strip())

            return plain, encoded

    res, rf = dict(), dict()
    res["plain"], res["encoded"] = dict(), dict()
    rf["plain"] = open(test_name + ".cycles.plain", "w")
    rf["encoded"] = open(test_name + ".cycles.encoded", "w")
    quot = open(test_name + ".slow-down", "w")

    print os.listdir(dirname)
    length_dirs = [d for d in os.listdir(dirname) if "LENGTH" in d]
    for ld in length_dirs:
        len = ld.rsplit('.')[1].strip()

        def _grab(args, dirname, names):
            stats_name = test_name + ".stats"
            if stats_name not in names:
                return

            stats_path = os.path.join(dirname, stats_name)
            sf = open(stats_path, "r")
            plain, enc = extract_cycles(sf)
            print "%f, %f" % (plain, enc)
            sf.close()

            res["plain"][len] = plain
            res["encoded"][len] = enc

        os.path.walk(os.path.join(dirname, ld), _grab, None)

    for k in res.keys():
        for l in res[k].keys():
            rf[k].write("%s %f\n" % (l, res[k][l]))

    for l in res["plain"].keys():
        q = res["encoded"][l] / res["plain"][l]
        quot.write("%s %f\n" % (l, q))

    for k in rf.keys():
        rf[k].close()
    quot.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--directory",
                        default=r".",
                        help="directory to be traversed for *.stats files")
    parser.add_argument("-t", "--test",
                        default=r"",
                        help="test name")

    args = parser.parse_args()

    walk_length_dirs(args.test, args.directory)