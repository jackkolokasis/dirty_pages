##############################################
#
# Author: 			Iacovos G. Kolokasis
# Date:					2014-03-24
# Description:	Makefile for the tests
#
#############################################

CXX=g++
TESTS = $(wildcard *.cpp)
TESTS_EXE = $(patsubst %.cpp,%.exe,$(TESTS))

all: $(TESTS_EXE)
	sudo chown -R kolokasis /dev/trace_dirty_pages && \
		sync && echo 3 | sudo tee /proc/sys/vm/drop_caches

%.exe: %.cpp
	$(CXX) -o $@ $<

clean:
	rm -f *.exe
	rm -f /mnt/fmap/file.txt
