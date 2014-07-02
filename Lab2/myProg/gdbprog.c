#include <stdio.h>

/**
 *  This program converts char to ASCII 
 *
 *  @author Steve Choo
 * */
int main(int argc, char *argv[]) {
   int character;

   printf("This program will convert a character to its ASCII value and display it on the screen!\n");
   printf("Please enter the first character: ");

   while((character = getchar()) != EOF) {
      printf("The ASCII code is: %d\n", character);
      getchar();

      printf("Again? (y/n): ");
      character = getchar();

      if(character == 'n') {
         break;
      }
      getchar();

      printf("Enter character: ");
   }

   printf("Goodbye!\n");
   
   return 0;
}
