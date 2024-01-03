##############################################
#
# Author: 			Iacovos G. Kolokasis
# Date:					2014-03-24
# Description:	Makefile for the tests
#
#############################################

TESTS = $(wildcard *.c)
TESTS_EXE = $(patsubst %.c,%.exe,$(TESTS))

all: $(TESTS_EXE)

%.exe: %.c
	gcc -o $@ $<

clean:
	rm -f *.exe
