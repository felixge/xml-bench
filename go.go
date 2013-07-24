package main

import (
	"bytes"
	"encoding/xml"
	g "github.com/moovweb/gokogiri"
	"github.com/moovweb/gokogiri/xpath"
	"fmt"
	"io"
	"os"
	"time"
)

var exampleXml = &bytes.Buffer{}
const loopSize = 100
const sampleSize = 100;

var xpathExpr = xpath.Compile("/jc:iq/discoitems:query")

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

  fmt.Printf("sample\tmb_per_sec\n");
	for sample := 1; sample <= sampleSize; sample++ {
		start := time.Now()
		for i := 0; i < loopSize; i++ {
			// copy exampleXml buffer
			reader := &bytes.Buffer{}
			*reader = *exampleXml

			if count, err := countNodes(reader); err != nil {
				panic(err)
			} else if count != 104 {
				panic(fmt.Errorf("wrong node count: %d", count))
			}
		}
		sec := time.Since(start).Seconds()
		opsPerSec := 1 / (sec / loopSize)
		mbPerSec := (opsPerSec * float64(exampleXml.Len()))/1024/1024
		fmt.Printf("%d\t%f\n", sample, mbPerSec)
	}
}

type raw struct {
	Content string `xml:",innerxml"`
}

func countNodes(reader io.Reader) (int, error) {
	count := 0

	decoder := xml.NewDecoder(reader)
	for {
		elem, err := nextElement(decoder)
		if err == io.EOF {
			return count, nil
		} else if err != nil {
			return count, err
		}

		if elem == nil {
			continue;
		}

		if elem.Name.Local != "iq" {
			continue;
		}

		head := XmlStartElementToString(elem)

		tail := &raw{}
		err = decoder.DecodeElement(tail, elem)
		if err != nil {
			return count, err
		}

		xmlData := []byte(head + tail.Content)
		doc, err := g.ParseXml(xmlData)
		defer doc.Free()
		if err != nil {
			return count, err
		}


		if ok := doc.XPathCtx.RegisterNamespace("jc", "jabber:client"); !ok {
			return count, fmt.Errorf("could not register namespace")
		}

		if ok := doc.XPathCtx.RegisterNamespace("discoitems", "http://jabber.org/protocol/disco#info"); !ok {
			return count, fmt.Errorf("could not register namespace")
		}

		root := doc.Root()
		nodes, err := root.Search(xpathExpr)
		if err != nil {
			return count, err
		}

		count += len(nodes);
	}
	panic("unreachable")
}

func xmlEscape(s string) string {
	b := []byte(s)
	// pre-allocate buf capacity with estimate of length
	buf := bytes.NewBuffer(make([]byte, 0, len(b)))
	xml.EscapeText(buf, b)
	return buf.String()
}

func escapeArgs(a []interface{}) {
	for i, v := range a {
		switch v := v.(type) {
		case string:
			a[i] = xmlEscape(v)
		}
	}
}

func Fprintf(w io.Writer, format string, a ...interface{}) (n int, err error) {
	escapeArgs(a)
	return fmt.Fprintf(w, format, a...)
}

func XmlStartElementToString(elem *xml.StartElement) string {
	var buf bytes.Buffer
	buf.WriteByte('<')
	buf.WriteString(elem.Name.Local)
	if elem.Name.Space != "" {
		Fprintf(&buf, " xmlns='%s'", elem.Name.Space)
	}
	for _, attr := range elem.Attr {
		Fprintf(&buf, " %s='%s'", attr.Name.Local, attr.Value)
	}
	buf.WriteByte('>')
	return buf.String()
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
