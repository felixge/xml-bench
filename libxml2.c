#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlreader.h>
#include <libxml/xpath.h>

char *read_file_contents(char *name);
int countNodes(char *xml);
const int loop_size = 100;
const int sample_size = 100;

int main() {
  LIBXML_TEST_VERSION;

  printf("sample\tmb_per_sec\n");
  char *example_xml = read_file_contents("example.xml");
  int example_xml_size = strlen(example_xml);

  int sample = 0;
  int ret;
  while (sample < sample_size) {
    sample++;

    int i;
    clock_t start = clock();
    for (i = 0; i < loop_size; i++) {
      ret = countNodes(example_xml);
      fflush(stdout);
      printf("count: %d\n", ret);
      /*assert(ret == 202);*/
      exit(0);
    }

    float sec = ((float)clock() - (float)start) / CLOCKS_PER_SEC;
    float ops_per_sec = 1 / (sec / loop_size);
    float mb_per_sec = (ops_per_sec * example_xml_size) / 1024 / 1024;
    printf("%d\t%f\n", sample, mb_per_sec);
    fflush(stdout);
  }

  free(example_xml);
  xmlCleanupParser();
  return 0;
}

char *read_file_contents(char *name) {
  char *file_contents;
  long input_file_size;
  FILE *input_file = fopen(name, "rb");
  fseek(input_file, 0, SEEK_END);
  input_file_size = ftell(input_file);
  rewind(input_file);
  file_contents = malloc(input_file_size*(sizeof(char))+1);
  fread(file_contents, sizeof(char), input_file_size, input_file);
  fclose(input_file);
  file_contents[input_file_size] = 0;
  return file_contents;
}

void	handleError(void * arg, const char * msg, xmlParserSeverities severity, xmlTextReaderLocatorPtr locator) {
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
  xmlNodePtr cur;

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

int countNodes(char *xml) {
  xmlTextReaderPtr reader = xmlReaderForMemory(xml, strlen(xml), NULL, NULL, 0);
  assert(reader != NULL);

  xmlTextReaderSetErrorHandler(reader, handleError, NULL);

  int count = 0;
  while (1) {
    int ret = xmlTextReaderRead(reader);
    assert(ret == 1);

    char * name = (char *)xmlTextReaderName(reader);

    if (strcmp(name, "stream:stream") != 0 && strcmp(name, "#text") != 0 && strcmp(name, "#comment") != 0) {
      /*xmlChar * str = xmlTextReaderReadInnerXml(reader);*/
      /*printf("str: %s\n", str);*/
      xmlNodePtr node = xmlTextReaderExpand(reader);
      if (node != NULL) {
        xmlXPathContextPtr xpathCtx = xmlXPathNewContext(node->doc);
        xpathCtx->node = node;
        /*xmlDocPtr doc = xmlTextReaderCurrentDoc(reader);*/
        /*xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);*/
        assert(xpathCtx != NULL);

        ret = xmlXPathRegisterNs(xpathCtx, BAD_CAST "discoinfo", BAD_CAST "http://jabber.org/protocol/disco#info");
        assert(ret == 0);
        ret = xmlXPathRegisterNs(xpathCtx, BAD_CAST "jc", BAD_CAST "jabber:client");
        assert(ret == 0);

        /*xmlXPathObjectPtr result = xmlXPathEval(BAD_CAST "/jc:iq/discoinfo:query", xpathCtx);*/
        xmlXPathObjectPtr result = xmlXPathEval(BAD_CAST "//jc:iq/discoinfo:query", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(result->nodesetval)) {
          print_nodes(result->nodesetval);
          print_node(node);
          printf("\n\n");
          /*printf("WE GOT A MATCH\n");*/
        }

        /*if (result->nodesetval->nodeNr > 0) {*/
          /*print_xpath_nodes(result->nodesetval, stdout);*/
        /*}*/
        /*printf("HOLY BULL");*/
        /*fflush(stdout);*/

        xmlXPathFreeObject(result);
        xmlXPathFreeContext(xpathCtx);

        ret = xmlTextReaderNext(reader);
        assert(ret == 1);
      } else {
        return count;
      }
    }

    free(name);
    count++;
  }
  xmlFreeTextReader(reader);
  return count;
}
