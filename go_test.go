package main

import (
	"bytes"
	"encoding/xml"
	"io"
	"os"
	"testing"
)

var exampleXml = &bytes.Buffer{}

func init() {
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
}

func BenchmarkHello(b *testing.B) {
	for i := 0; i < b.N; i++ {
		// copy exampleXml buffer
		b.StopTimer()
		reader := &bytes.Buffer{}
		*reader = *exampleXml
		b.StartTimer()

		if err := readStreamHeader(reader); err != nil {
			panic(err)
		}
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
