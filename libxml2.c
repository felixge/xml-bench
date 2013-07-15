#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <libxml/xmlreader.h>

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
      /*printf("count: %d\n", ret);*/
      assert(ret == 189);
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

int countNodes(char *xml) {
  xmlTextReaderPtr reader = xmlReaderForMemory(xml, strlen(xml), NULL, NULL, 0);
  assert(reader != NULL);

  xmlTextReaderSetErrorHandler(reader, handleError, NULL);

  int count = 0;
  while (1) {
    int ret = xmlTextReaderRead(reader);
    if (ret != 1) {
      return count;
    }

    char * name = (char *)xmlTextReaderName(reader);

    if (strcmp(name, "stream:stream") != 0 && strcmp(name, "#text") != 0) {
      xmlNodePtr node = xmlTextReaderExpand(reader);
      if (node == NULL) {
        return count;
      } else {
        /*printf("name: %s\n", name);*/
        ret = xmlTextReaderNext(reader);
        assert(ret == 1);
      }
    }

    free(name);
    count++;
  }
  xmlFreeTextReader(reader);
  return count;
}
