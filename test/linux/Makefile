SHELL = /bin/bash

CFLAGS = -Wall -Wno-unused-function -Wno-switch -g -rdynamic -fdiagnostics-color -fno-omit-frame-pointer -fmax-errors=5 -std=c++11 -I../../ -I.
LDFLAGS = -lpthread

.PHONY: bam

bam: bam.cpp
	g++ ${CFLAGS} bam.cpp ${LDFLAGS} -o bam
