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
├── allocator.cpp      # implementation of malloc/free
│
├── benchmark/
│   └── test_basic_func.py      # benchmark comparing allocators
│
├── Makefile               # build instructions
└── README.md              # project documentation
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

Example output:

```
Running allocation benchmark...

Custom allocator: 10.4M ops/s
glibc malloc(): 8.9M ops/s
```

---
## Benchmark Metrics
The Benchmark measures:
- Operations per second
- Total execution time
- Scalability across threads

The results are summarized in **[DESIGN_AND_BENCHMARK.md](DESIGN_AND_BENCHMARK.md)**.

--- 

## Conclusion

This project demonstrates the design and implementation of a basic thread-safe memory allocator. By managing memory manually and comparing performance with glibc `malloc()`, the project highlights the trade-offs involved in memory management systems.

---
## Literature
- [glibc malloc internals](https://sourceware.org/glibc/wiki/MallocInternals)
- [GNU C Library documentation](https://www.gnu.org/software/libc/manual/)