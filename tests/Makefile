CC=gcc
CFLAGS=-Wall -g
PYTHON3=python3

ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

PROGS = \
    alloc_free_test \
	test_simple_performance \
	test_coalesce

# Default target
check: $(PROGS)
	$(PYTHON3) test_basic_function.py
	$(PYTHON3) test_performance.py

# Compile all programs
${ROOT_DIR}/alloc_free_test: ${ROOT_DIR}/alloc_free_test.c
	$(CC) $(CFLAGS) -Og -g -o $@ $<

${ROOT_DIR}/test_simple_performance: ${ROOT_DIR}/test_simple_performance.c
	$(CC) $(CFLAGS) -Og -g -o $@ $< -lpthread	

${ROOT_DIR}/test_coalesce: ${ROOT_DIR}/test_coalesce.c
	$(CC) $(CFLAGS) -Og -g -o $@ $<


clean:
	rm -rf $(PROGS)
