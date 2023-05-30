#!/usr/bin/python

import os, platform, re, subprocess, sys

def run_bench(args):
    bench = "bench.exe" if "Windows" == platform.system() else "bench.out"
    with subprocess.Popen(
        [os.path.join(os.path.dirname(sys.argv[0]), bench)] + args,
        text=True, encoding="utf-8",
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT) as pipe:
        while True:
            line = pipe.stdout.readline()
            if not line:
                break
            sys.stdout.write(line);
            yield line
        exitcode = pipe.wait()
        if exitcode:
            raise subprocess.CalledProcessError(exitcode, pipe.args)

def bench2csv(args, filename):
    with open(filename, "wb") as file:
        for line in run_bench(args):
            m = re.match(r"(\w+)\.\.\.\.+ (.*)OK ([\d.]+)s", line)
            if m:
                name, info, time = m.group(1), m.group(2).strip(), m.group(3)
                file.write(("%s,%s,%s\n" % (name, info, time)).encode('utf-8'))

abbrev = {
    "Windows": "win",
    "Linux": "lnx",
}
for i in range(3):
    bench2csv(["+*"], "perf-%s-%s.csv" % (abbrev[platform.system()], i + 1))
