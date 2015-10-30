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
profiles = ["original", "improved"]
tests = ["bubblesort.best",
         "bubblesort.random",
         "bubblesort.random-binary",
         "bubblesort.worst",
         "quicksort.best",
         "quicksort.random",
         "quicksort.random-binary",
         "quicksort.worst"]


def collect_tests(kind, dirname, only_reg):
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
                if only_reg:
                  if b in line and ("RREG" in line or "WREG" in line):
                    tests[d][b] += 1
                else:
                  if b in line and "TXT" not in line:
                    tests[d][b] += 1
        log_file.close()

    return tests


def collect_profiles(kind, profiles, dirnames, only_reg=False):
    results = dict()

    for p in profiles:
        tests = collect_tests(kind, dirnames[p], only_reg)
        for name, data in tests.items():
            if name not in results.keys():
                results[name] = dict()
            results[name][p] = data

    return results


def plot_test(name, data, outdir, ax, legend=False):
    profs = profiles
    norm_data, norm_err_data = dict(), dict()

    for p in profs:
        sum = reduce(lambda x, y: x+y, data[p].values())
        norm_data[p] = dict([(k, float(v)/float(sum)) for k, v in data[p].items()])

        err_sum = sum - data[p]["CORRECT"]
        err_data = dict([(k, v) for k, v in data[p].items() if k is not "CORRECT"])
        norm_err_data[p] = dict([(k, float(v)/float(err_sum)) for k, v in err_data.items()])

    bars, err_bars = dict(), dict()
    for b in bins:
        bars[b], err_bars[b] = [], []
        for p in profs:
            bars[b].append(norm_data[p][b])
            if b is not "CORRECT":
              err_bars[b].append(norm_err_data[p][b])

    #fig, ax = plt.subplots(index, 1, figsize=(5, 7))
    ax.set_title(name.replace("bubblesort", "bs").replace("quicksort", "qs"))

    n_prof = len(profs)
    width = 2.2
    spacing = 0.8
    offset = 0.8

    lefts = [offset + (width+spacing)*r for r in range(n_prof)]
    ticks = [offset + (width+spacing)*r + 0.5*width for r in range(n_prof)]
    bottoms = [0.0]*n_prof

    handles = []

    for i in range(len(bins)):
        label = "SDC" if bins[i] is "UNEXPECTED" else bins[i]
        h = ax.bar(lefts, bars[bins[i]], bottom=bottoms, label=label, width=width, alpha=0.5, color=colors[i], lw=0.0)
        handles.append(h)
        for j in range(n_prof):
            bottoms[j] += bars[bins[i]][j]

    offset = 8.8
    lefts = [offset + (width+spacing)*r for r in range(n_prof)]
    ticks += [offset + (width+spacing)*r + 0.5*width for r in range(n_prof)]
    bottoms = [0.0]*n_prof

    for i in range(1, len(bins)):
        ax.bar(lefts, err_bars[bins[i]], bottom=bottoms, width=width, alpha=0.5, color=colors[i], lw=0.0)
        for j in range(n_prof):
            bottoms[j] +=err_bars[bins[i]][j]

    ax.yaxis.grid()
    ax.set_ylim([0,1])
    ax.set_xticks(ticks)
    ax.set_xticklabels(profs*2, rotation=90, size=12)

    if legend:
      ax.legend(loc="lower right", ncol=1, fontsize=8)

    return handles

def plot_results(results, outdir, legend):
    fig, axes = plt.subplots(1, len(results.keys()), sharey=True, figsize=(18,4))
    fig.set_facecolor("w")
    index = 0
    for name in tests:
      data = results[name]
      handles = plot_test(name, data, outdir, axes[index], (not legend) and (index == 3))
      index += 1

    if legend:
      fig.legend(handles,
                 ("correct execution",
                  "fault detected",
                  "crash",
                  "hang",
                  "silent data corruption"),
                  loc=(0.083,0.24),
                  ncol=5, fontsize=9)
                
    plt.savefig(outdir, bbox_inches="tight")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--directory",
                        default=r".",
                        help="directory to be traversed for *.cover.*.log files")
    parser.add_argument("--original",
                        default=r".",
                        help="directory with results for profile 'original'")
    parser.add_argument("--improved",
                        default=r".",
                        help="directory with results for profile 'improved'")
    parser.add_argument("--pickle",
                        default=r"",
                        help="path for dumping data dictionary")
    parser.add_argument("--unpickle",
                        default=r"",
                        help="path for reading in data dictionary")
    parser.add_argument("-r", "--regs",
                        action='store_true',
                        help="only count experiments that injected '{W|R}REG' faults")
    parser.add_argument("-l", "--legend",
                        action='store_true',
                        help="add a legend to the plots")
    parser.add_argument("-o", "--output",
                        default=r"",
                        help="output file name")

    args = parser.parse_args()

    dirnames = dict()
    dirnames["original"] = args.original
    dirnames["improved"] = args.improved

    if args.unpickle is not "":
        f = open(args.unpickle, "r")
        res = pickle.load(f)
        f.close()
    else:
        res = collect_profiles("encoded", profiles, dirnames, args.regs)
        if args.pickle is not "":
            f = open(args.pickle, "w")
            pickle.dump(res, f)
            f.close()

    plot_results(res, args.output, args.legend)
