#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hashtable.h"
#define INITIAL_SIZE 10000
#define CHUNK 1000
#define DEFAULT_WORDS 10

char *readLongLine(FILE *where);
void quickSort(HashNode arr[], int a, int b);
void swap(int *a, int *b);
/**
 *  Program
 *
 *  @author Steve Choo
 * */
int main(int argc, char *argv[]) {
   HashNode *table;
   HashNode *sortedTable;

   int occupiedCells = 0;
   int *ptrToOC;

   int tabSize = INITIAL_SIZE;
   int *ptrToTS;

   FILE *fp;

   char *str;
   char *wordToProcess;
   char *num;
   char delimiters[] = " ,./<>?;':[]\\|\"{}~!@#$%^&*()_+-=0987654321`"; 
   int opt = 0;
   int i, j;
   int arguments = argc;
   unsigned int finalSize;

   ptrToOC = &occupiedCells;
   ptrToTS = &tabSize;

   /*create an array of HashNodes for the hash table*/
   table = createTable(ptrToTS, ptrToOC);

   if(argc != 1) {
      /*needs to be read from file*/
      /*num = argv[2] sucks, need to change to do proper error checking*/
      if(!(strcmp(argv[1], "-n")) && (num = argv[2])) {
         opt = atoi(num);

         /*not liking this triple loop...*/
         while(--arguments >= 3) {
            fp = fopen(argv[arguments], "r");
            if(!fp) {
               perror("File failed to open");
               exit(42);
            }

            while((str = readLongLine(fp))) {
               wordToProcess = strtok(str, delimiters);

               while(wordToProcess) {
                  insert(table, wordToProcess, ptrToTS, ptrToOC);
                  wordToProcess = strtok(NULL, delimiters);
               }
            }
         }

         if(!fclose(fp)) {
            perror("File Didn't Close");
            exit(300);
         }
      }
   } else {
      /*reading from stdin at this point*/
      while((str = readLongLine(stdin))) {
         wordToProcess = strtok(str, delimiters);
         while(wordToProcess) {
            insert(table, wordToProcess, ptrToTS, ptrToOC);
            wordToProcess = strtok(NULL, delimiters);
         }
      }
   }

   finalSize = sizeof(HashNode) * tabSize;

   sortedTable = malloc(finalSize);

   j = 0;
   for(i = 0; i < finalSize; i++) {
      if(table[i].word == NULL) {
         /*do nothing*/
      } else {
         /* idea here is to just to have an array of pointers,
          * then sort the pointers
          */
         sortedTable[j] = table[i];
         j++;
      }
   }

   quickSort(sortedTable, sortedTable[0].occurrences,
         sortedTable[finalSize].occurrences);

   if(opt > 0) {
      printf("The top 10 words (out of %d) are:", occupiedCells);

      for(i = 0; i < opt; i++) {
         printf("%-9d %s\n", sortedTable[i].occurrences,
               sortedTable[i].word);
      }
   }

   return 0;
}

char *readLongLine(FILE *where) {
   char *buff;
   int sofar, len;

   buff = malloc(CHUNK);
   if(!buff) {
      perror("ReadLongLine");
      exit(1);
   }

   sofar = 0;

   for(;;) {
      if(!fgets(buff + sofar, CHUNK, where)){
         break;
      }

      len = strlen(buff + sofar);
      sofar += len;

      if(buff[sofar - 1] == '\n') {
         break;
      }

      if(len == 0) {
         break;
      }

      buff = realloc(buff, sofar + CHUNK);

      if(!buff) {
         perror("ReadLongLine");
         exit(42);
      }

      if(!sofar) {
         free(buff);
         buff = NULL;
      } else {
         buff = realloc(buff, sofar + 1);

         if(!buff) {
            perror("ReadLongLine");
            exit(97);
         }
      }
   }

   return buff;
}

void swap(int *a, int *b) {
   int temp;
   temp = *a;
   *a = *b;
   *b = temp;
}

int pivot(int a, int b) {
   return ((a + b) / 2);
}

void quickSort(HashNode arr[], int a, int b) {
   int key, i, j, k;

   if(a < b) {
      k = pivot(a, b);
      swap(&(arr[a].occurrences), &(arr[k].occurrences));
      key = arr[a].occurrences;
      i = a + 1;
      j = b;

      while(i <= j) {

         while((i <= b) && (arr[i].occurrences <= key)) {
            i++;
         }
         
         while((j >= a) && (arr[j].occurrences > key)) {
            j--;
         }

         if(i < j) {
            swap(&arr[i].occurrences, &arr[j].occurrences);
         }
      }

      swap(&arr[a].occurrences, &arr[j].occurrences);
      quickSort(arr, a, j - 1);
      quickSort(arr, j + 1, b);
   }
}
