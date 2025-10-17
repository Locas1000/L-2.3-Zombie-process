# Makefile for Homework 2: Zombie Process Handler

# --- Variables ---
# CC is the C compiler we'll use.
CC = gcc
# CFLAGS are the flags we pass to the compiler.
# -Wall:  Enable all warnings, which is good practice.
# -g:     Include debugging information in the executables.
# -pthread: Link against the POSIX threads library (needed for our mutex in libzombie).
CFLAGS = -Wall -g -pthread
# LDFLAGS are flags for the linker.
# -L.:    Look for libraries in the current directory.
# -lzombie: Link with the libzombie.a library.
LDFLAGS_LIB = -L. -lzombie

# List of all source files in the src/ directory.
SRCS = src/zombie_creator.c src/zombie_detector.c src/zombie_reaper.c \
       src/process_daemon.c src/zombie.c

# List of the final executable programs we want to build.
TARGETS = zombie_creator zombie_detector zombie_reaper process_daemon test_lib

# --- Targets ---

# The 'all' target is the default. Running 'make' will build everything here.
# It depends on all the executables in the TARGETS list.
.PHONY: all
all: $(TARGETS)

# --- Executable Targets ---

# Rule to build zombie_creator
zombie_creator: src/zombie_creator.c
	$(CC) $(CFLAGS) -o $@ $<

# Rule to build zombie_detector
zombie_detector: src/zombie_detector.c
	$(CC) $(CFLAGS) -o $@ $<

# Rule to build zombie_reaper
zombie_reaper: src/zombie_reaper.c
	$(CC) $(CFLAGS) -o $@ $<

# Rule to build process_daemon
process_daemon: src/process_daemon.c
	$(CC) $(CFLAGS) -o $@ $<

# Rule to build the test_lib program.
# This one is special because it depends on our static library, libzombie.a.
test_lib: test_lib.c libzombie.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_LIB)

# --- Library Target ---

# Rule to build the static library libzombie.a.
# It first depends on zombie.o (the object file).
libzombie.a: zombie.o
	ar rcs $@ $<

# Rule to build the object file zombie.o from its source.
zombie.o: src/zombie.c src/zombie.h
	$(CC) $(CFLAGS) -c -o $@ $<

# --- Utility Targets ---

# The 'clean' target removes all compiled files.
# It's marked as .PHONY because 'clean' is not a file we are actually creating.
.PHONY: clean
clean:
	rm -f $(TARGETS) *.o *.a

# The 'run-tests' target will execute all your test scripts in order.
.PHONY: run-tests
run-tests: all
	@echo "\n>>> Running Creator and Detector Test..."
	@bash tests/test_creator.sh
	@echo "\n>>> Running Reaper Test..."
	@bash tests/test_reaper.sh
	@echo "\n>>> Running Daemon Test..."
	@bash tests/test_daemon.sh
	@echo "\n>>> Running Library Test..."
	@./test_lib
	@echo "\nAll tests completed."