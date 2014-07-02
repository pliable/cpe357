#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define BUFSIZE 20
#define ARE_EQUAL 0

/**
 *  Program
 *
 *  @author Steve Choo
 * */
char *read_long_line(FILE *file);

int main(int argc, char *argv[]) {
   char *a, *b;

   while((a = read_long_line(stdin)) != NULL) {
      b = read_long_line(stdin);

      if(strcmp(a,b) == ARE_EQUAL) {
         free(b);
      } else {
         printf("%s\n", a);
         printf("%s\n", b);
         free(a);
         a = b;
      }
   }

   printf("%s\n", a);
   free(a);
   
   return 0;
}

char *read_long_line(FILE *file) {
   char *str;
   char c;
   int counter = 0;

   str = (char*)malloc(sizeof(char) * BUFSIZE);

   while((c = fgetc(file)) != EOF) {
      if(c == '\n') {
         str[counter] = c;
         return str;
      }

      str[counter++] = c;

      if(counter > strlen(str)) {
         str = realloc(str, sizeof(char) * BUFSIZE * counter);
      }
   }

   return NULL;
}
