#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define BUFSIZE 20
#define ARE_EQUAL 0

/**
 *  This program is similar to the uniq program
 *
 *  @author Steve Choo
 * */
char *read_long_line(FILE *file);

int main(int argc, char *argv[]) {
   /*
    * setting up buffers
    */
   char *a, *b;

   /*
    * read in first value
    */
   a = read_long_line(stdin);

   /*
    * continuously read values, if it's null,
    * loop will terminate
    */
   while(a && (b = read_long_line(stdin))) {

      /*
       * compare strings to see if they are equal
       * if so, free the memory and grab the next string
       */
      if(strcmp(a,b) == ARE_EQUAL) {
         free(b);
      } else {
         printf("%s", a);
         free(a);
         a = b;
      }
   }

   /*
    * free last bit of memory!
    */
   printf("%s", a);
   free(a);
   
   return 0;
}

char *read_long_line(FILE *file) {
   /*
    * string for reading!
    * c is the current character
    * counter keeps track of amount of characters 
    *    for purposes of comparing length to current
    *    allocated amount
    */
   char *str;
   char c;
   int counter = 0;

   size_t buf_size = BUFSIZE;
   /*
    * allocating memory for reading
    */
   str = (char*)malloc(buf_size);

   /*
    * continue to grab until EOF is reached
    */
   while((c = fgetc(file)) != EOF) {
      if(c == '\n') {
         str[counter++] = c;
         /*
          * adding null terminator character
          */
         str[counter] = '\0';
         return str;
      }

      str[counter++] = c;

      /*
       * reallocate if needed!
       */
      if(counter == buf_size) {
         buf_size = BUFSIZE * counter;
         str = realloc(str, buf_size);
      }
   }

   /*
    * returns null when EOF is reached
    */
   return NULL;
}
