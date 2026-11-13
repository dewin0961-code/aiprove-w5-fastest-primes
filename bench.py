import subprocess, time, statistics
def bench(cmd, n, runs=25):
    ts = []
    exp = {10:"4",100:"25",1000:"168",100000:"9592",1000000:"78498",5000000:"348513"}[n]
    for _ in range(runs):
        t0 = time.perf_counter()
        p = subprocess.run(cmd, input=f"{n}\n".encode(), stdout=subprocess.PIPE)
        ts.append((time.perf_counter()-t0)*1000)
        assert p.stdout.decode().strip() == exp, (cmd, p.stdout)
    return min(ts), statistics.median(ts)
for n in (1000000, 5000000):
    for name, cmd in [("segmented ./primes", ["./primes"]), ("segmented ./run.sh", ["./run.sh"])]:
        mn, med = bench(cmd, n)
        print(f"N={n} {name}: min {mn:.2f} ms / median {med:.2f} ms")
