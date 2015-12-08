#include <stdlib.h>
#include <stdio.h>


int main() {
  long int value;
  unsigned int seed;
  FILE *fp;

  fp = fopen("/dev/urandom","r");
  if (fp == NULL) {
    perror("fopen");
    exit(1);
  }
  if (fread(&seed,sizeof(seed),1,fp) != 1) {
    perror("fread");
    exit(2);
  }
  fclose(fp);
  srandom(seed);

  for (;;) {
    value = random();
    if (fwrite(&value,sizeof(value),1,stdout) != 1) {
      perror("fwrite");
      exit(3);
    }
  }
  return 0;
}
