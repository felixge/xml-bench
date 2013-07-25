gcc=gcc -Wall -g -std=c99 -pedantic
CFLAGS=`xml2-config --cflags`
LIBS=`xml2-config --libs`

all: go.txt libxml2.txt

go.txt: go
	./$^ | tee $@

go: go.go
	go build $^

gokogiri.txt: gokogiri
	./$^ | tee $@

gokogiri: gokogiri.go
	go build $^

libxml2.txt: libxml2
	./$^ | tee $@

libxml2: libxml2.c
	$(gcc) $^ $(CFLAGS) $(LIBS) -o $@

bug: bug.c
	$(gcc) $^ $(CFLAGS) $(LIBS) -o $@

.PHONY: all libxml2.txt go.txt
