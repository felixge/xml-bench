export PKG_CONFIG_PATH := /usr/local/opt/libxml2/lib/pkgconfig/
valgrind=/usr/local/Cellar/valgrind/3.8.1/bin/valgrind --suppressions=osx.supp --leak-check=full
gcc=gcc -Wall -g

all: bench_go bench_libxml2

bench_go:
	go test -test.bench . | tee results.go.txt

libxml2.txt: libxml2
	$(valgrind) ./$^ | tee $@

libxml2: libxml2.c
	$(gcc) $^ `pkg-config --cflags libxml-2.0` `pkg-config --libs libxml-2.0` -o $@

.PHONY: all libxml2.txt
