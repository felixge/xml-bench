package main

import (
	"bytes"
	"github.com/moovweb/gokogiri/xml"
	"github.com/moovweb/gokogiri/xpath"
	"fmt"
	"io"
	"os"
	"time"
)

var exampleXml = &bytes.Buffer{}
const loopSize = 100
const sampleSize = 100;

var xpathExpr = xpath.Compile("self::jc:iq/discoitems:query")

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
	count := 0;

	r, err := xml.NewReader(reader, xml.DefaultEncodingBytes, nil, xml.XML_PARSE_NOWARNING)
	if err != nil {
		return count, err
	}

	for {
		if err := r.Read(); err == io.EOF {
			break
		} else if err != nil {
			return count, err
		}

		depth := r.Depth()
		nodeType := r.NodeType()

		if nodeType == xml.XML_ELEMENT_NODE && depth == 6 {
			node, err := r.Expand()
			if err != nil {
				return count, err
			}

			ctxt := node.MyDocument().DocXPathCtx()
			if ok := ctxt.RegisterNamespace("jc", "jabber:client"); !ok {
				return count, fmt.Errorf("could not register namespace")
			}

			if ok := ctxt.RegisterNamespace("discoitems", "http://jabber.org/protocol/disco#info"); !ok {
				return count, fmt.Errorf("could not register namespace")
			}

			nodes, err := node.Search(xpathExpr)
			if err != nil {
				return count, err
			}

			count += len(nodes)
		}
	}


	return count, nil
}
