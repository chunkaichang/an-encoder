#! /usr/bin/python

import argparse
import os
import pickle
import shutil
import statistics
import subprocess

from matplotlib import pyplot as plt

bins = ["CORRECT", "ANCRASH", "OSCRASH", "HANG", "UNEXPECTED"]
colors = ["r", "g", "b", "m", "c"]

profiles = ["baseline", "pinned", "improved", "imp., pinned"]

titles = {"matmul": "Matrix-Vector",
          "ptrarith": "Array Copy",
          "bubblesort.random": "Bubblesort",
          "quicksort.random": "Quicksort"}
tests = ["matmul", "ptrarith", "bubblesort.random", "quicksort.random"]


def collect_tests(kind, dirname):
    tests = dict()

    for d in os.listdir(dirname):
        test_dir = os.path.join(dirname, d)
        if not os.path.isdir(test_dir):
            continue

        log_name = os.path.join(test_dir, "cover", "%s.cover.%s.log" % (d, kind))
        if os.path.exists(log_name): print "Found log file: %s" % log_name
        else: continue

        tests[d] = dict()
        for b in bins:
            tests[d][b] = 0

        log_file = open(log_name, "r")
        for line in log_file.readlines():
            for b in bins:
                if b in line:
                  tests[d][b] += 1
        log_file.close()

    return tests


def collect_profiles(kind, profiles, dirnames):
    results = dict()

    for p in profiles:
        tests = collect_tests(kind, dirnames[p])
        for name, data in tests.items():
            if name not in results.keys():
                results[name] = dict()
            results[name][p] = data

    return results


def plot_test(name, data, outdir, legend):
    norm_data, norm_err_data = dict(), dict()

    for p in profiles:
        sum = reduce(lambda x, y: x+y, data[p].values())
        norm_data[p] = dict([(k, float(v)/float(sum)) for k, v in data[p].items()])

        err_sum = sum - data[p]["CORRECT"]
        err_data = dict([(k, v) for k, v in data[p].items() if k is not "CORRECT"])
        norm_err_data[p] = dict([(k, float(v)/float(err_sum)) for k, v in err_data.items()])

    bars, err_bars = dict(), dict()
    for b in bins:
        bars[b], err_bars[b] = [], []
        for p in profiles:
            bars[b].append(norm_data[p][b])
            if b is not "CORRECT":
              err_bars[b].append(norm_err_data[p][b])

    fig, ax = plt.subplots(1, 1, figsize=(5, 7))
    fig.set_facecolor("w")
    ax.set_title(titles[name], fontsize=16)
    ax.set_ylim([0,1])
    ax.yaxis.grid()

    n_prof = len(profiles)
    width = 1.0
    dist = 0.4
    offset = 0.4

    lefts = [offset + (width+dist)*r for r in range(n_prof)]
    ticks = [offset + (width+dist)*r + 0.5*width for r in range(n_prof)]
    bottoms = [0.0]*n_prof

    for i in range(len(bins)):
        label = "SDC" if bins[i] is "UNEXPECTED" else bins[i]
        ax.bar(lefts, bars[bins[i]], bottom=bottoms, label=label, width=width, alpha=0.5, color=colors[i], lw=0.0)
        for j in range(n_prof):
            bottoms[j] += bars[bins[i]][j]

    offset = 6.8
    lefts = [offset + (width+dist)*r for r in range(n_prof)]
    ticks += [offset + (width+dist)*r + 0.5*width for r in range(n_prof)]
    bottoms = [0.0]*n_prof

    for i in range(1, len(bins)):
        ax.bar(lefts, err_bars[bins[i]], bottom=bottoms, width=width, alpha=0.5, color=colors[i], lw=0.0)
        for j in range(n_prof):
            bottoms[j] +=err_bars[bins[i]][j]

    if (legend):
      ax.legend(loc="lower right", ncol=1, fontsize=12)

    plt.xticks(ticks, profiles*2, rotation=55, size=12, ha="right")
    plt.savefig(os.path.join(outdir, "%s.eps" % name), bbox_inches="tight")


def plot_results(results, outdir, legend):
    for name, data in results.items():
        plot_test(name, data, outdir, legend)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--base",
                        default=r".")
    parser.add_argument("--pin",
                        default=r".")
    parser.add_argument("--imp",
                        default=r".")
    parser.add_argument("--imppin",
                        default=r".")
    parser.add_argument("--pickle",
                        default=r"",
                        help="path for dumping data dictionary")
    parser.add_argument("--unpickle",
                        default=r"",
                        help="path for reading in data dictionary")
    parser.add_argument("-l", "--legend",
                        action='store_true')
    parser.add_argument("-o", "--output",
                        default=r"",
                        help="output directory")

    args = parser.parse_args()

    dirnames = dict()
    dirnames["baseline"] = args.base
    dirnames["pinned"] = args.pin
    dirnames["improved"] = args.imp
    dirnames["imp., pinned"] = args.imppin

    if args.unpickle is not "":
        f = open(args.unpickle, "r")
        res = pickle.load(f)
        f.close()
    else:
        res = collect_profiles("encoded", profiles, dirnames)
        if args.pickle is not "":
            f = open(args.pickle, "w")
            pickle.dump(res, f)
            f.close()

    plot_results(res, args.output, args.legend)
