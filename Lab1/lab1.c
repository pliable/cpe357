#include <stdio.h>

/**
 *  Program
 *
 *  @author Steve Choo
 * */
int main(int argc, char **argv) {

    long num;
    char ans = 0;

    printf("Welcome to SUPER NUMBER GUESSER!!!!!!! WOAAAAAAAHHHHHH!!!!!!\n");
    printf("Please, tell your number to my partner here, and I will guess your number using the power of 0's and 1's\n");
   
    while(1) {
        printf("\nHi, it's the computers partner, you can tell me your number it's safe and it will have no idea. Seriously: ");
        scanf("%dl", &num);
        printf("\nHi there, it's the computer again. Was your number %d? (y/n): ", num);

        scanf("%c", &ans);
        /*Second call to clear new line character*/
        scanf("%c", &ans);

        if(ans != 'y') {
            printf("\nNo, I'm pretty sure that's right. Why would you lie to a computer?\n");
        } else {
            printf("\nI'm the best number guesser in the world.\n");
        }

        printf("\nWould you like me to blow your mind again? (y/n): ");

        scanf("%c", &ans);
        /*Second call to clear new line character*/
        scanf("%c", &ans);

        if(ans == 'n') {
            break;
        }
    }

    printf("\nYour loss, sir. GOODBYE FOREVER!!!!!!\n");
    return 0;
}
