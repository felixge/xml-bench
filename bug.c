#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>

int count_normal(char * filename, char * xpath);
int count_text_reader(char * filename, char * xpath);
void apply_namespaces(xmlXPathContextPtr ctx);

int main(int argc, char ** argv) {
  LIBXML_TEST_VERSION;

  if (argc != 3) {
    printf("usage: ./bug <filename> <xpath>\n");
    exit(1);
  }

  printf("libxml version: %s\n", LIBXML_DOTTED_VERSION);

  char *filename = argv[1];
  char *xpath = argv[2];

  int normal = count_normal(filename, xpath);
  printf("normal: %d\n", normal);
  int text_reader = count_text_reader(filename, xpath);
  printf("text_reader: %d\n", text_reader);

  if (normal != text_reader) {
    printf("error: different counts\n");
    exit(1);
  }

  xmlCleanupParser();
  return 0;
}

void print_node(xmlNodePtr node) {
  char * properties = "";
  for (xmlAttrPtr p = node->properties; p != NULL; p = p->next) {
    char * pair;
    char * val = (char *)xmlGetProp(node, p->name);
    asprintf(&pair, " %s=\"%s\"", p->name, val);
    xmlFree(val);

    if (strlen(properties) > 0) {
      char * combined;
      asprintf(&combined, "%s%s", properties, pair);
      free(properties);
      free(pair);
      properties = combined;
    } else {
      properties = pair;
    }

    if (p->next == NULL) {
      break;
    }
    p = p->next;
  }

  char * name;
  if (node->ns) {
    asprintf(&name, "%s:%s", node->ns->href, node->name);
  } else {
    asprintf(&name, "%s", node->name);
  }

  printf("<%s%s>\n", name, properties);
  free(name);
  if (strlen(properties) > 0) {
    free(properties);
  }
}

void print_nodes(xmlNodeSetPtr nodes) {
  printf("Result (%d nodes):\n", nodes->nodeNr);
  for(int i = 0; i < nodes->nodeNr; i++) {
    assert(nodes->nodeTab[i]);

    if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
      print_node(nodes->nodeTab[i]);
    } else {
      printf("unhandled node type: %d\n", nodes->nodeTab[i]->type);
    }
  }
}

int count_normal(char * filename, char * xpath) {
  int count = 0;
  xmlDocPtr doc = xmlReadFile(filename, NULL, XML_PARSE_NOWARNING);
  assert(doc);
  xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
  assert(xpathCtx);

  apply_namespaces(xpathCtx);

  xmlXPathObjectPtr result = xmlXPathEvalExpression(BAD_CAST xpath, xpathCtx);
  if (!xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    count = result->nodesetval->nodeNr;
  }

  xmlXPathFreeObject(result);
  xmlXPathFreeContext(xpathCtx);
  xmlFreeDoc(doc);

  return count;
}

int count_text_reader(char * filename, char * xpath) {
  int ret;
  xmlTextReaderPtr reader = xmlReaderForFile(filename, NULL, XML_PARSE_NOWARNING);

  int count = 0;
  while (1) {
    ret = xmlTextReaderRead(reader);
    if (ret == 0) {
      return count;
    }
    assert(ret == 1);

    char * name = (char *)xmlTextReaderLocalName(reader);
    int type = xmlTextReaderNodeType(reader);
    int depth = xmlTextReaderDepth(reader);

    if (depth == 1 && type == XML_ELEMENT_NODE) {
      xmlNodePtr node = xmlTextReaderExpand(reader);
      assert(node);
      print_node(node);
      xmlDocPtr doc = xmlTextReaderCurrentDoc(reader);
      assert(doc);
      xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
      assert(xpathCtx);

      /*ret = xmlXPathSetContextNode(node, xpathCtx);*/
      /*assert(ret == 0);*/

      apply_namespaces(xpathCtx);

      xmlXPathObjectPtr result = xmlXPathNodeEval(node, BAD_CAST xpath, xpathCtx);
      if (!xmlXPathNodeSetIsEmpty(result->nodesetval)) {
        count += result->nodesetval->nodeNr;
        print_nodes(result->nodesetval);
      }

      xmlXPathFreeObject(result);
      xmlXPathFreeContext(xpathCtx);
      printf("\n\n");
    }
    free(name);
  }
  assert(0); // unreachable
}

// add any namespace you may need for the file you're testing with
void apply_namespaces(xmlXPathContextPtr ctx) {
  int ret;
  ret = xmlXPathRegisterNs(ctx, BAD_CAST "discoinfo", BAD_CAST "http://jabber.org/protocol/disco#info");
  assert(ret == 0);
  ret = xmlXPathRegisterNs(ctx, BAD_CAST "jc", BAD_CAST "jabber:client");
  assert(ret == 0);
}
