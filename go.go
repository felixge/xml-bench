package main

import (
	"bytes"
	"encoding/xml"
	"fmt"
	"io"
	"os"
	"time"
)

var exampleXml = &bytes.Buffer{}
const loopSize = 10000
const sampleSize = 100;

func main() {
	// Load doc into memory buffer before running the benchmark. We just want to
	// to benchmark the decoding here.
	file, err := os.Open("example.xml")
	if err != nil {
		panic(err)
	}
	defer file.Close()

	if _, err := io.Copy(exampleXml, file); err != nil {
		panic(err)
	}

  fmt.Printf("sample\tops_per_sec\tloop_size\n");
	for sample := 1; sample <= sampleSize; sample++ {
		start := time.Now()
		for i := 0; i < loopSize; i++ {
			// copy exampleXml buffer
			reader := &bytes.Buffer{}
			*reader = *exampleXml

			if err := readStreamHeader(reader); err != nil {
				panic(err)
			}
		}
		sec := time.Since(start).Seconds()
		opsPerSec := 1 / (sec / loopSize)
		fmt.Printf("%d\t%f\t%d\n", sample, opsPerSec, loopSize)
	}
}

func readStreamHeader(reader io.Reader) (error) {
	decoder := xml.NewDecoder(reader)
	_, err := nextElement(decoder)
	return err
}

func nextElement(decoder *xml.Decoder) (*xml.StartElement, error) {
	for {
		tok, err := decoder.Token()

		if err != nil {
			return nil, err
		}

		switch elem := tok.(type) {
		case xml.StartElement:
			return &elem, nil
		case xml.EndElement:
			return nil, nil
		}
	}
	panic("unreachable")
}
