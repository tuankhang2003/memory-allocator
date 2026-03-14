import subprocess
import os
from helper_function import *

# benchmark executable
THREAD_TEST = "./test_simple_performance"

COALESCE_TEST = "./test_coalesce"
# number of threads 
THREADS = [1, 2, 4]

def run_benchmark(program,  num_threads, ld_preload=None):
    env = os.environ.copy()
    if ld_preload:
        env["LD_PRELOAD"] = ld_preload

    # run benchmark with -t (thread) option
    result = subprocess.run(
        [program, "-t", str(num_threads)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        env=env,
        text=True
    )
    if result.returncode != 0:
        print(f"Error: program exited with {result.returncode}")
        print("STDERR:", result.stderr)
        exit()

    # benchmark prints only the time in seconds
    return float(result.stdout.strip())

# run custom malloc and compares tp glibc malloc
def run_and_print_statistic(program):
    print_green("=== Custom malloc ===")
    my_times = {}
    for t in THREADS:
        time_taken = run_benchmark(program, t, ld_preload=LIB_MALLOC_PATH)
        my_times[t] = time_taken
        print(f"{t} threads: {time_taken:.6f} s")

    print_green("\n=== glibc malloc ===")
    glibc_times = {}
    for t in THREADS:
        time_taken = run_benchmark(program, t)
        glibc_times[t] = time_taken
        print(f"{t} threads: {time_taken:.6f} s")

    print_green("\n=== Speedup vs glibc ===")    
    for t in THREADS:
        speedup = glibc_times[t] / my_times[t]
        print(f"{t} threads: {speedup:.2f}x")


if __name__ == "__main__":
    print_green("+++++ Benchmark for testThread ++++++")
    run_and_print_statistic(THREAD_TEST)
    print_green("+++++ Benchmark for coalesce ++++++")
    run_and_print_statistic(COALESCE_TEST)