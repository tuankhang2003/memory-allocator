
from helper_function import *


def main():
    plot_img = False
    print_green("+++++ Benchmark for simple allocation test ++++++")
    run_and_print_statistic(SIMPLE_ALLOCATION_TEST, SIMPLE_ALLOCATION_IMAGES, plot_img)
    print()
    print_green("+++++ Benchmark for coalesce test ++++++")
    run_and_print_statistic(COALESCE_TEST, COALESCE_ALLOCATION_IMAGES, plot_img)
    

if __name__ == "__main__":
    main()
