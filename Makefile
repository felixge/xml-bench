all:
	go test -test.bench . | tee results.go.txt

.PHONY: all
