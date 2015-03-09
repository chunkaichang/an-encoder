#! /usr/bin/python

import sys
import os.path
import subprocess
import operator
import statistics

if __name__=="__main__":
  if len(sys.argv) < 3:
    print 'Expected three arguments'
    exit(-1)

  source = sys.argv[1]
  if not os.path.isfile(source):
    print 'File does not exist: ', source
    exit(-1)

  n = int(sys.argv[2])

  subprocess.call(['./apply-an.sh', source])
  execs = {'cnt': source + '.cnt', 'enc': source + '.enc'}

  results = [os.path.isfile(x) for x in execs.values()]
  if not reduce(operator.__and__, results):
    print 'command \'appy-an.sh', source, '\' did not succeeed'
    exit(-1);

  """ Run each of the generated executables 'n' times and extract the
      number of cycles taken from the output to 'stderr': """
  cycles = dict();
  for k in execs.keys():
    cycles[k] = []
    for i in range(n):
      p = subprocess.Popen(execs[k],
                           stdout=subprocess.PIPE,
                           stderr=subprocess.PIPE)
      lines = [l for l in p.stderr.readlines() if "elapsed cycles" in l]
      assert(len(lines) == 1)
      c = lines[0].split(':')[1].strip();
      cycles[k].append(int(c))

  """ Now purge those measurements where the distance from the mean
      is greater than 5 times the standard deviation: """
  purge = True
  count = 0
  while purge:
    print 'Purge cycle no.', count
    count += 1

    means = dict()
    devs = dict()
    for k in execs.keys():
      means[k] = statistics.mean(cycles[k])
      devs[k] = statistics.stdev(cycles[k])

    purge = False
    for k in execs.keys():
      if k == 'cnt':
        other = 'enc'
      else:
        other = 'cnt'

      assert(len(cycles[k]) == len(cycles[other]))
      i = 0
      while i < len(cycles[k]):
        dist = abs(cycles[k][i] - means[k])
        if dist > 5*devs[k]:
          print 'Purging ... cnt:', cycles['cnt'][i], \
                           ' enc:', cycles['enc'][i], \
                           ' ... reason:', k, 'distance =', dist
          cycles[k].pop(i)
          cycles[other].pop(i)
          purge = True
        else:
          i += 1

  print 'Nothing left to purge in this cycle ... done.'

  print 'samples left: ', len(cycles['cnt'])
  print 'means:  ', means
  print 'stdevs: ', devs
  print 'mean slow-down: ', float(means['enc'])/float(means['cnt'])

