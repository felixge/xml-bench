#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <libxml/xmlreader.h>

char *read_file_contents(char *name);
int readStreamHeader(char *xml);
const int loop_size = 1000;
const int sample_size = 100;

int main() {
  printf("sample\tops_per_sec\tloop_size\n");
  char *example_xml = read_file_contents("example.xml");
  xmlInitParser();

  int sample = 0;
  while (sample < sample_size) {
    sample++;

    int i;
    clock_t start = clock();
    for (i = 0; i < loop_size; i++) {
      readStreamHeader(example_xml);
    }

    float sec = ((float)clock() - (float)start) / CLOCKS_PER_SEC;
    float ops_per_sec = 1 / (sec / loop_size);
    printf("%d\t%f\t%d\n", sample, ops_per_sec, loop_size);
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
  file_contents = malloc(input_file_size * (sizeof(char))+1);
  fread(file_contents, sizeof(char), input_file_size, input_file);
  fclose(input_file);
  file_contents[input_file_size] = 0;
  return file_contents;
}

int readStreamHeader(char *xml) {
  xmlTextReaderPtr reader;
  int ret;

  reader = xmlReaderForMemory(xml, strlen(xml), "", NULL, 0);
  if (reader == NULL) {
    return 1;
  }

  ret = xmlTextReaderRead(reader);
  if (ret == 1) {
    xmlChar *name = xmlTextReaderName(reader);
    assert(strcmp((char *)name, "stream:stream") == 0);
    free(name);
  } else {
    return 2;
  }
  xmlFreeTextReader(reader);
  return 0;
}
