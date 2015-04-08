#! /usr/bin/python

import sys
import os.path
import subprocess
import operator
import statistics

cycles = dict(); hi_check = dict(); lo_check = dict()
veri = dict(); stat = dict()


def run_test_pair(cmds, n):
  """ Run each of the commands 'n' times, verify that outputs agree
      between 'plain' and 'encoded' version, and extract the
      number of cycles taken from the respective outputs. """
  """ Argument cmds = should be dictinary of thsi structure:
      {'plain': "", 'enc': ""} """

  for k in cmds.keys():
    cycles[k] = []; hi_check[k] = []; lo_check[k] = []

    for i in range(n):
      p = subprocess.Popen(cmds[k].split(),
                           bufsize=-1,
                           stdout=open(os.devnull, "w"),
                           stderr=subprocess.PIPE)

      for l in p.stderr.readlines():
        def test_and_append(search_string, split_string, dictionary, base):
          if search_string in l:
            c = l.split(split_string)[1].strip()
            dictionary[k].append(int(c, base))

        test_and_append("elapsed cycles", ":", cycles, 10)
        test_and_append("hi(checksum)", "=", hi_check, 16)
        test_and_append("lo(checksum)", "=", lo_check, 16)

      p.wait()


def verify_test_pair(cmds, n, filename):
  assert(len(cmds.keys()) == 2)
  k0 = cmds.keys()[0]; k1 = cmds.keys()[1]

  assert(len(hi_check[k0]) == len(lo_check[k0]))
  assert(len(hi_check[k1]) == len(lo_check[k1]))

  f = open(filename, "w")
  success = True;

  for i in range(len(hi_check[k0])):
    if (hi_check[k0][i] != hi_check[k1][i]):
      f.write("hi: index=" + str(i) + ", "      \
              + k0 + "=" + str(hi_check[k0][i]) \
              + k1 + "=" + str(hi_check[k1][i]) + "\n")
      success = False
    if (lo_check[k0][i] != lo_check[k1][i]):
      f.write("lo: index=" + str(i) + ", "      \
              + k0 + "=" + str(lo_check[k0][i]) \
              + k1 + "=" + str(lo_check[k1][i]) + "\n")
      success = False

  f.write(str(int(success)) + "\n")
  f.close()
  return success


def run_stats(cmds, n, filename):
  """ Now purge those measurements where the distance from the mean
      is greater than 5 times the standard deviation: """
  purge = True
  count = 0

  f = open(filename, "w")

  while purge:
    f.write("Purge cycle no. " + str(count) + "\n")
    count += 1

    means = dict()
    devs = dict()
    for k in cmds.keys():
      means[k] = statistics.mean(cycles[k])
      devs[k] = statistics.stdev(cycles[k])

    purge = False
    assert(len(cmds.keys()) == 2)

    for i in range(len(cmds.keys())):
      k = cmds.keys()[i]
      other = cmds.keys()[i ^ 1]

      assert(len(cycles[k]) == len(cycles[other]))
      i = 0
      while i < len(cycles[k]):
        dist = abs(cycles[k][i] - means[k])
        if dist > 5*devs[k]:
          f.write("Purging ... " + k + ": " + str(cycles[k][i]) + ", "     \
                                 + other + ": " + str(cycles[other][i])    \
                                 + " ... reason: " + k + " distance= " + str(dist) + "\n")
          cycles[k].pop(i)
          cycles[other].pop(i)
          purge = True
        else:
          i += 1

  f.write("Nothing left to purge in this cycle ... done.\n")

  f.write("samples left: " + str(len(cycles[k])) + "\n")
  f.write("means:  " + str(means) + "\n")
  f.write("stdevs: " + str(devs) + "\n")

  ratio = float(means['encode']) / float(means['plain'])
  f.write("mean slow-down: " + str(ratio) + "\n")
  f.close()
  return ratio


if __name__=="__main__":
  if len(sys.argv) < 2:
    print 'Expected two arguments'
    exit(-1)

  test_dir = sys.argv[1]
  if not os.path.isdir(test_dir):
    print 'Path does not exist: ', test_dir
    exit(-1)

  tests = [os.path.join(test_dir, d) for d in os.listdir(test_dir)
                                     if os.path.isdir(os.path.join(test_dir, d))]
  for t in tests:
    cfgs = [os.path.join(t, f) for f in os.listdir(t) if f.endswith(".cfg")]

    for c in cfgs:
      cfg = open(c, "r")
      cmds = dict()
      n = 0

      for l in cfg.readlines():
        data = l.split(':')[1].strip()
        if "plain" in l:
          cmds['plain'] = data
        elif "encode" in l:
          cmds['encode'] = data
        elif "run" in l:
          n = int(data)

      cfg.close()

      print "Running test from config file", c, "..."
      run_test_pair(cmds, n)
      print " ... verifying ..."
      veri[c] = verify_test_pair(cmds, n, c + ".veri")
      if veri[c]:
        print " ... obtaining statistics ..."
        stat[c] = run_stats(cmds, n, c + ".stat")
        print " ... done."
      else:
        print " ... FAILED."

  for k in veri.keys():
    print "TEST:", k
    print "\tverified:", veri[k]
    if veri[k]:
      print "\tslow-down:", stat[k]

    print "-----------------------"
