#include "hashtable.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

HashNode *createTable(int *tabSize, int *occupiedCells) {
   HashNode *table;
   int i;
   size_t size = sizeof(HashNode) * nextPrime(2 * (*tabSize));

   table = malloc(size); 

   for(i = 0; i < size; i++) {
      table[i].word = NULL;
      table[i].occurrences = 0;
   }

   *tabSize = size;
   *occupiedCells = 0;

   return table;
}

int nextPrime(int n) {
   if(isPrime(n)) {
      return n;
   }

   n++;
   return nextPrime(n);
}

int isPrime(int n) {
   int i;

   if(n < 2) {
      return 0;
   }

   if(n % 2 == 0) {
      if(n == 2) {
         return 1;
      } else {
         return 0;
      }
   }

   for(i = 3; i * i <= n; i += 2) {
      if(n % i == 0) {
         return 0;
      }
   }

   return 1;
}

void insert(HashNode *table, char *str, int *tabSize, int *occupiedCells) {
   unsigned int index = findPosition(table, str, tabSize);

   if(table[index].word != NULL) {
      strcpy(table[index].word, str);
      table[index].occurrences += 1;
      *occupiedCells += 1;

      /*tabSize here needs to constantly be updated*/
      if(*occupiedCells >= (*tabSize / 2)) {
         rehash(table, tabSize, occupiedCells); /*needs to be written*/
      }
   }
}

unsigned int hash(char *str) {
  unsigned int hsh = 5381;
  int c;

  while((c = (*str)++)) {
     hsh = ((hsh << 5) + hsh) + c;
  }

  return hsh;
}

void rehash(HashNode *table, int *tabSize, int *occupiedCells) {
   HashNode *temp;
   int i, oldTableSize;
   int sizeConverted;
   int *ptrToSC;
   size_t size = sizeof(HashNode) * nextPrime(2 * (*tabSize));

   sizeConverted = (int)size;

   /*Second array set up*/
   temp = malloc(size);
   *occupiedCells = 0;
   oldTableSize = *tabSize;

   ptrToSC = &sizeConverted;

   /*clearing out array just in case*/
   for(i = 0; i < size; i++) {
      temp[i].word = NULL;
      temp[i].occurrences = 0;
   }

   for(i = 0; i < oldTableSize; i++) {
      if((table[i].word != NULL)) {
         insert(temp, table[i].word, ptrToSC, occupiedCells);
      }
   }

   free(table);
   table = temp;
}

unsigned int findPosition(HashNode *n, char *str, int *tabSize) {
   int probeNum = 0;
   unsigned int hashValue = hash(str) % (unsigned int)(*tabSize);
   unsigned int index = hashValue;

   while((n[index].word != NULL) && (strcmp(n[index].word, str) != 0)) {
      probeNum++;
      index = (hashValue + probeNum * probeNum) % (*tabSize);
   }

   return index;
}

