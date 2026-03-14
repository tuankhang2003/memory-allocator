import matplotlib.pyplot as plt
import subprocess
import os

LIB_MALLOC_PATH = "../libmymalloc.so"
# number of threads 
THREADS = [1, 2, 4]
# benchmark executable
SIMPLE_ALLOCATION_TEST = "./test_simple_performance"
RANDOM_ALLOCATION_TEST = "./test_random_performance"
COALESCE_TEST = "./test_coalesce"


SIMPLE_ALLOCATION_IMAGES = "../images/simple_allocation_plot.png"
COALESCE_ALLOCATION_IMAGES = "../images/coalesce_plot.png"

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

def plot_statistic(custom_result, glibc_result, image):
    plt.figure(figsize=(8,5))
    plt.plot(THREADS, custom_result, marker='o', label='Custom malloc')
    plt.plot(THREADS, glibc_result, marker='o', label='glibc malloc')
    plt.xlabel('Threads')
    plt.ylabel('Time (s)')
    plt.title('Simple Allocation Test')
    plt.xticks(THREADS)
    plt.legend()
    plt.grid(True)
    plt.savefig(image)
    plt.show()


# run custom malloc and compares tp glibc malloc
def run_and_print_statistic(program, dest_img, plot):
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
    
    if plot:
        custom_result = [my_times[t] for t in THREADS]
        glibc_result  = [glibc_times[t] for t in THREADS]
        os.makedirs(os.path.dirname(dest_img), exist_ok=True)  # create folder if missing
        plot_statistic(my_times.values(), glibc_times.values(), dest_img)


def green(text):
    return f"\033[92m{text}\033[0m"

def red(text):
    return f"\033[91m{text}\033[0m"

def print_green(text):
    print(green(text))

