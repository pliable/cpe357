/**
 *  @author Steve Choo
 *  @version CPE357
 *  @version Assignment 1
 * */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
/*
 * Offset used to convert ASCII to int and vice versa
 */
#define OFFSET 48

/*
 * Prototypes helper functions
 */
void PrintBarcode(char buff[], int counter);
void TableLookup(char lookup);

int main(int argc, char *argv[]) {
   char buffer[12];
   int character;
   int counter = 0;

   /*
    * While loop grabs stdin character by character and analyzes it.
    * If the character is valid, it's added to the buffer[] array,
    * otherwise it is skipped. When it reaches a new line character,
    * the helper methods are called to print out the result
    */
   while((character = fgetc(stdin)) != EOF) {
      if(isdigit(character) && counter < 12) {
         buffer[counter] = character;
         counter++;
      } else if(character == '\n') {
         PrintBarcode(buffer, counter);
         counter = 0;
      } else if(!isspace(character) || counter > 11) {
         printf("Invalid delivery data ");
         counter = 0;
      }
   }

   return 0;
}

/*
 * Helper method to print out the data
 */
void PrintBarcode(char buff[], int counter) {
   int sum = 0;
   int checkSum;
   int i;

   /*
    * Verifies length here. If it's an invalid length
    * (not a 5, 9, or 11 zip code), an error message
    * is printed.
    */
   if(counter == 5 || counter == 9 || counter == 11) {
      /*
       * Calculates checksum
       */
      for(i = 0; i < counter; i++) {
         sum += ((int)buff[i] - OFFSET);
      }

      checkSum = 10 - (sum % 10);
      checkSum = checkSum + OFFSET;

      /*
       * Adds checksum to char buffer for evaluation
       */
      buff[counter] = checkSum;

      /*
       * Prints zipcode to stdout for user
       */
      for(i = 0; i < counter; i++) {
         putchar(buff[i]);
      }

      counter++;

      /*
       * Prints out postnet code for user
       */
      printf(" -> %s", ":");
      for(i = 0; i < counter; i++) {
         TableLookup(buff[i]);
      }
      printf("%s", ":");

   } else {
      printf("Invalid delivery data\n");
      return;
   }

   printf("\n");
}

/*
 * The table to convert the number given
 * to the proper postnet code
 */
void TableLookup(char lookup) {

   if(lookup == '0') {
      printf("%s", "::...");
   } else if(lookup == '1') {
      printf("%s", "...::");
   } else if(lookup == '2') {
      printf("%s", "..:.:");
   } else if(lookup == '3') {
      printf("%s", "..::.");
   } else if(lookup == '4') {
      printf("%s", ".:..:");
   } else if(lookup == '5') {
      printf("%s", ".:.:.");
   } else if(lookup == '6') {
      printf("%s", ".::..");
   } else if(lookup == '7') {
      printf("%s", ":...:");
   } else if(lookup == '8') {
      printf("%s", ":..:.");
   } else if(lookup == '9') {
      printf("%s", ":.:..");
   } else {
      printf("You should not be here.");
   }
}
