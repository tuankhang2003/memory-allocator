# Thread-Safe Memory Allocator

## Overview

This project implements a custom thread-safe memory allocator that provides replacements for the standard `malloc()` and `free()` functions. The allocator reserves a large memory region using `mmap()` and manages allocations internally using a custom allocation policy.

The goal of this project is to explore memory management techniques and evaluate the performance and scalability of the custom allocator compared to the default **glibc `malloc()`** implementation.

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

## Allocator Design

### Memory Reservation

At initialization, the allocator reserves a large block of virtual memory using `mmap()`. This memory region is then managed internally by the allocator.

### Block Structure

Each memory block contains metadata describing the allocation:

* block size
* allocation status
* pointer to next free block

Example layout:

```
[ Header | User Data ]
```

### Free List

Free blocks are stored in a linked list (free list). During allocation, the allocator searches this list to find a suitable block.

---

## Allocation Policy

The allocator uses a **first-fit strategy**:

1. Traverse the free list
2. Find the first block large enough for the requested allocation
3. Split the block if it is larger than needed
4. Mark the block as allocated

This strategy is simple and provides good performance for many workloads.

---

## Memory Deallocation

When `free()` is called:

1. The block is marked as free
2. The block is returned to the free list
3. Adjacent free blocks are merged (coalescing) to reduce fragmentation

---

## Thread Safety

Thread safety is ensured using a **mutex** protecting the allocator's internal data structures.

All operations that modify the free list are protected by this lock to prevent race conditions during concurrent allocations.

---

## Benchmark Methodology

The benchmark evaluates allocator performance using repeated allocation and deallocation patterns.

Metrics measured:

* allocation time
* throughput (operations per second)
* performance under multiple threads

The same workload is executed using both:

* the custom allocator
* the default glibc `malloc()`

---

## Results

Example benchmark results:

| Allocator        | Throughput  |
| ---------------- | ----------- |
| Custom Allocator | 10.4M ops/s |
| glibc malloc()   | 8.9M ops/s  |

The custom allocator performs well for small allocations due to simplified metadata management and direct memory control.

---

## Limitations

Current limitations include:

* global lock may reduce scalability under heavy multi-threaded workloads
* possible memory fragmentation over long execution times
* simple allocation strategy

---

## Future Improvements

Possible improvements include:

* per-thread memory arenas
* lock-free data structures
* size-segregated free lists
* improved fragmentation handling

---

## Conclusion

This project demonstrates the design and implementation of a basic thread-safe memory allocator. By managing memory manually and comparing performance with glibc `malloc()`, the project highlights the trade-offs involved in memory management systems.
