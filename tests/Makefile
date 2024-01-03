# This folder contains individual test files. Please write me a makefile that
# will compile all of these files into individual executables.

TESTS = $(wildcard *.c)
TESTS_EXE = $(patsubst %.c,%.exe,$(TESTS))

all: $(TESTS_EXE)

%.exe: %.c
	gcc -o $@ $<

clean:
	rm -f *.exe
