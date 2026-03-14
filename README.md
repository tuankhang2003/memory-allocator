# Thread-Safe Memory Allocator

## Overview

This project implements a custom thread-safe memory allocator that provides replacements for the standard `malloc()` and `free()` functions. The allocator reserves a large memory region using `mmap()` and manages allocations internally using a custom allocation policy.

The project also includes a small benchmark suite that compares the performance of this allocator against the default `glibc malloc()`.

The goal of this project is to explore memory management techniques and evaluate the performance and scalability of the custom allocator compared to the default **glibc `malloc()`** implementation.

---

## Features

- Custom implementations of:
  - `malloc(size_t size)`
  - `free(void *ptr)`
- Memory pool reserved using `mmap`
- Thread-safe allocation
- Thread-local arenas to reduce lock contention
- First-fit allocation policy
- Block splitting and coalescing
- Benchmark suite comparing against `glibc malloc`

---

## General Instructions

* **Language:** C/C++
* **Build system:** Makefile
* **Repository:** GitHub

The repository contains:

* Implementation of a custom memory allocator
* A benchmark suite to evaluate performance
* A Makefile to compile and run the project

---

## Repository Structure

```
.
├── src/                 # Source code for the allocator
├── tests/               # Test implementations and benchmarks
├── images/              # Plots and visualizations
├── Makefile             # Build automation
├── README.md            # Project overview
├── DESIGN_AND_BENCHMARK.md  # Design details and benchmarking results
```

---

## Building the Project

To compile the project, run:

```
make all
```

This will build the allocator and the benchmark program.

To clean compiled files:

```
make clean
```

---

## Running the Benchmark

After compiling, run the benchmark using:

```
make check
```

The benchmark performs repeated allocation and deallocation operations and compares the performance of the custom allocator with the default glibc `malloc()`.
You can choose whether to generate plot images by setting plot = True in **[test_performance.py](./tests/test_performance.py)** . 

Example output:

```
+++++ Benchmark for simple allocation test ++++++
=== Custom malloc ===
1 threads: 0.179591 s
2 threads: 0.178883 s
4 threads: 0.180692 s

=== glibc malloc ===
1 threads: 0.000789 s
2 threads: 0.000868 s
4 threads: 0.001540 s
```

When plot = True, the benchmark script also generates image files (e.g., **[simple_allocation_plot.png](./images/simple_allocation_plot.png)**) under the images/ directory. You can view these images to visually compare performance trends.


---
## Benchmark Metrics
The Benchmark measures:
- Total execution time for each test
- Scalability across threads (1, 2 and 4 threads)

The results are summarized in **[DESIGN_AND_BENCHMARK.md](DESIGN_AND_BENCHMARK.md)**.

--- 

## Conclusion

This project demonstrates the design and implementation of a basic thread-safe memory allocator. By managing memory manually and comparing performance with glibc `malloc()`, the project highlights the trade-offs involved in memory management systems.

---
## Literature
- [glibc malloc internals](https://sourceware.org/glibc/wiki/MallocInternals)
- [GNU C Library documentation](https://www.gnu.org/software/libc/manual/)