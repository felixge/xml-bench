package main

import (
	"bytes"
	"encoding/xml"
	"fmt"
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

var j = 0
func BenchmarkHello(b *testing.B) {
	for i := 0; i < b.N; i++ {
		// copy exampleXml buffer
		b.StopTimer()
		reader := &bytes.Buffer{}
		*reader = *exampleXml
		b.StartTimer()

		j++
		if _, err := readStreamHeader(reader); err != nil {
			panic(err)
		}
	}
}

func readStreamHeader(reader io.Reader) (string, error) {
	decoder := xml.NewDecoder(reader)
	elem, err := nextElement(decoder)
	if err != nil {
		return "", err
	}

	return XmlStartElementToString(elem), nil
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

func XmlStartElementToString(elem *xml.StartElement) string {
	var buf bytes.Buffer
	buf.WriteByte('<')
	buf.WriteString(elem.Name.Local)
	if elem.Name.Space != "" {
		fmt.Fprintf(&buf, " xmlns='%s'", elem.Name.Space)
	}
	for _, attr := range elem.Attr {
		fmt.Fprintf(&buf, " %s='%s'", attr.Name.Local, attr.Value)
	}
	buf.WriteByte('>')
	return buf.String()
}
