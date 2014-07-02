#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define BUFFSIZE 4096
#define ASCIISIZE 256

struct char_node {
   int count;
   unsigned char chr;
   struct char_node *next, *left, *right;
};
typedef struct char_node char_node;

char_node *new_node(int cnt, unsigned char character,
      char_node *left, char_node *right);
char_node *insert_sorted_ll(char_node *list, char_node *new);
char_node *insert_sorted_tree(char_node *list, char_node *new);
char_node *create_tree(char_node *list);
void encoding(char_node *list, char currCode[], char *codes[]);
char *strdup(const char *str);
int compare(char_node *one, char_node *two);
unsigned char *write_header(int numChars, size_t buffSize, int *counts);
unsigned char write_bytes(int fd, unsigned char *buff, char *codes[],
      int numBytes, unsigned char unfinByte, int *bitCount);
/*
void read_header(int *numChars, int *readCounts, unsigned char *buff);
*/
/*
 * Printing is commented out due to not needing to print
 * the tree.
 */
/*
void printing(char_node *list);
*/
/**
 *  This program reads in a file and produces a printout of
 *  huffman encodings
 *
 *  @author Steve Choo
 * */
int main(int argc, char *argv[]) {
   /* fdIn == file descriptor for file to be read
    * fdOut == file descriptor for file to be written
    * i == used as an index, but also as the numeric representation
    *      of a character
    * currCount == The current count when populating the linked list
    * numBytes == Current number of bytes read
    * counts[] == The array of counts
    * firstRead == A flag to help tell if an empty file has been
    *      encountered
    * numChars = A counter to keep track of the number of characters
    * byteCount == Keeping track of char array
    */
   int fdIn, fdOut, i, currCount;
   ssize_t numBytes;
   size_t writeBuffSize;
   int counts[ASCIISIZE];
   int firstRead = 1;
   int numChars = 0;
   int outputFile = 0;
   int bitCount = 0;
   int zero = 0;
   int *ptrBitCount = &bitCount;
   int pad;

   /* *inputFileName == The file name to be read
    * buffer == The buffer for reading in chunks
    * currChar == The current character when populating the linked list
    * codes == These will store the encoded version of the chars; the
    *    codes table
    * currCode == The current code to append to codes
    * writeBuffHeader == The buffer that holds the header for the
    *    output file
    */
   char *inputFileName;
   char *outputFileName;
   unsigned char buffer[BUFFSIZE];
   unsigned char currChar;
   char *codes[ASCIISIZE];
   char currCode[ASCIISIZE];
   unsigned char *writeBuffHeader;
   unsigned char byte = 0;

   /*
    * head == the linked list, and eventually, the tree
    * new == used to insert a new node
    */
   char_node *head = NULL;
   char_node *new;

   off_t currPos;

   /*
    * Emptying string
    */
   currCode[0] = '\0';

   /*
    * Checking argc for correct amount of arguments.
    * Needs to be 2, as it has to have a file name
    */
   /*
   if(argc != 2) {
      printf("Specify file to compress. Program exiting...\n");
      exit(EXIT_FAILURE);
   }
   */

   if(argc == 3) {
      if(argv[2] != NULL) {
         outputFile = 1;

         outputFileName = argv[2];
      }
   }

   /* Assigning the file name internally...should be a more
    * descriptive way to do this instead of using argv[1]
    */
   inputFileName = argv[1];

   /*
    * Opening file, assigning file descriptor to fd for future use
    */
   fdIn = open(inputFileName, O_RDONLY);
   
   /*
    * Checking if opening succeeded...if not, program terminates
    */
   if(fdIn == -1) {
      perror(inputFileName);
      exit(EXIT_FAILURE);
   }

   /*
    * Zeroing out each element of counts array
    */
   for(i = 0; i < ASCIISIZE; i++) {
      counts[i] = 0;
   }

   /* These loops reads through the file, counting up
    * every type of character
    */
   while((numBytes = read(fdIn, buffer, BUFFSIZE)) > 0) {
      /*
       * If the while loop evaluates to true, this flag is
       * set to false so an empty file won't be operated on
       */
      firstRead = 0;

      /*
       * The idea behind this is to collect the counts of each
       * character. Each index represents a different character,
       * and so when a character is encountered, the count
       * for that respective index increments by one.
       */
      for(i = 0; i < numBytes; i++) {
         counts[buffer[i]]++;
      }
   }

   /*
    * If outputFile is 1, then the program needs to write to a file
    * Otherwise, output it written to stdout
    */
   if(outputFile) {
      fdOut = open(outputFileName, O_RDWR | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
            S_IROTH | S_IWOTH);
   } else {
      fdOut = STDOUT_FILENO;
   }

   if(fdOut == -1) {
      perror("output_file_error");
      exit(-157);
   }

   if(firstRead) {
      numBytes = write(fdOut, &zero, 4);
      if(numBytes < 0) {
         perror("zero_write");
         exit(-985);
      }

      exit(-4576);
   }

   /*
    * Checking the flag to see if bytes were successfully grabbed.
    * If not, this means there was an error, or an empty file,
    * so the program will exit at this point.
    */
   /*
   if(firstRead) {
      exit(-1);
   }
   */

   /*
    * This loop will iterate through the counts array,
    * populating a linked list. If the count is 0, it is skipped
    */
   for(i = 0; i < ASCIISIZE; i++) {
      if(counts[i] != 0) {
         currCount = counts[i];
         currChar = (unsigned char)i;
         numChars++;

         new = new_node(currCount, currChar, NULL, NULL);
         head = insert_sorted_ll(head, new);
      }
   }


   /* testing printing of linked list */
   /*
   while(head) {
      printf("Character: %c Count: %d\n", head -> chr, head -> count);
      head = head -> next;
   }
   */

   /*
    * Creating Huffman encoding tree. Returns the head of the tree.
    */
   head = create_tree(head);

   /*
    * Recursive function that iterates through the entire tree,
    * populating codes with each characters respective encoding
    * value
    */
   encoding(head, currCode, codes);

   /*
    * Setting file's offset to beginning to re-read
    */
   currPos = lseek(fdIn, 0, SEEK_SET);

   if(currPos == -1) {
      perror("lseek");
      exit(-42);
   }


   /*
    * numBytes is being assigned as the size of the header
    * that needs to be written. The header will always be of size
    * 4 (for the int to represent the number count) + 5 (the size
    * of each block of a char and int, representing the character
    * and the count of that character, respectively) * numChars
    * (which is calculated during the populating of the linked list)
    */
   writeBuffSize = (size_t)(4 + 5 * numChars);

   /*
    * write_header writes the header internally, and assigns a
    * pointer to point to the header 
    *
    */
   writeBuffHeader = write_header(numChars, writeBuffSize, counts);

   /*
    * The actual writing to the file occurs with this write call
    */
   numBytes = write(fdOut, writeBuffHeader, writeBuffSize);

   if(numBytes < 0) {
      perror("write");
      exit(-2357);
   }

   /*
    * Freeing writeBuff as it's not needed anymore
    */
   free(writeBuffHeader);

   while((numBytes = read(fdIn, buffer, BUFFSIZE)) > 0) {

      byte = write_bytes(fdOut, buffer, codes, numBytes,
            byte, ptrBitCount);

   }

   if(bitCount == 8) {
      if((numBytes = write(fdOut, &byte, 1) < 0)) {
         perror("write");
         exit(-125);
      }
   } else if(bitCount > 0) {
      pad = 8 - bitCount;

      byte = byte << pad;

      if((numBytes = write(fdOut, &byte, 1) < 0)) {
         perror("write");
         exit(-125);
      }
   }

   /*
   for(i = 0; i < ASCIISIZE; i++) {
      if(readCounts[i] != 0) {
         printf("Character: %c Count: %d\n", i, readCounts[i]);
      }
   }
   */

   /* TESTING FOR PRINT */
   /*
   for(i = 0; i < ASCIISIZE; i++) {
      if(codes[i] != 0) {
         printf("Character: %d Code: %s Count: %d\n", i, codes[i],
               counts[i]);
      }
   }
   */

   /*
    * Printing out Huffman encodings for characters
    */
   /*
   for(i = 0; i < ASCIISIZE; i++) {
      if(codes[i] != NULL) {
         printf("0x%02x: %s\n", i, codes[i]);
      }
   }
   */

   return 0;
}


/*
 * This function is used to populate the initial linked list
 */
char_node *insert_sorted_ll(char_node *list, char_node *new) {
   
   char_node *res;

   if(!list || (compare(list, new) > 0)) {
      new -> next = list;
      res = new;
   } else {
      res = list;

      while(list -> next && (compare(list -> next, new) < 0)) {
         list = list -> next;
      }

      new -> next = list -> next;
      list -> next = new;
   }

   return res;
}

/*
 * Slight variation of insert_sorted_ll...this is used to
 * populate the tree instead of the initial linked list.
 * This is only comparing the counts, instead of both the
 * counts and the character
 */
char_node *insert_sorted_tree(char_node *list, char_node *new) {
   
   char_node *res;

   if(!list || new -> count <= list -> count) {
      new -> next = list;
      res = new;
   } else {
      res = list;

      while(list -> next && list -> next -> count < new -> count) {
         list = list -> next;
      }

      new -> next = list -> next;
      list -> next = new;
   }

   return res;
}

/* Sets up a new node. Returns pointer to new node */
char_node *new_node(int cnt, unsigned char character, char_node *left,
      char_node *right) {

   char_node *new;

   new = malloc(sizeof(char_node));

   if(!new) {
      perror("new_node");
      exit(1);
   }

   new -> count = cnt;
   new -> chr = character;
   new -> next = NULL;
   new -> left = left;
   new -> right = right;

   return new;
}

char_node *create_tree(char_node *list) {

   char_node *new, *head, *left, *right;
   int sum;

   head = list;

   while(head -> next != NULL) {

      /*
       * Assign left as the head node, then increment head
       * to next node.
       */
      left = head;
      head = head -> next;

      /*
       * If head is null, end of list has been reached
       */
      if(!head) {
         break;
      }

      /*
       * Set right side to head now to compare, increment head
       * to set new head of list.
       */
      right = head;
      head = head -> next;
      /*
       * Attempting to grab the first two nodes, and add up their 
       * sum to make the new node
       */
      sum = (left -> count) + (right -> count);

      /*
       * Creating new node
       */
      new = new_node(sum, 0, left, right);

      /*
       * Inserting new node with sum of frequencies. Points
       * back at head of list for next iteration.
       */
      head = insert_sorted_tree(head, new); 
   }

   return head;
}

/*
 * Recursively going through the tree, populating codes[]
 * with each characters code. The index in the array is
 * that character, while the string pointed to by the element
 * is the code itself for that character
 */
void encoding(char_node *list, char currCode[], char *codes[]) {

   if(!(list -> left) && !(list -> right)) {
      codes[list -> chr] = strdup(currCode);
      currCode[strlen(currCode) - 1] = '\0';
   } else {
      encoding(list -> left, strcat(currCode, "0"), codes);
      encoding(list -> right, strcat(currCode, "1"), codes);
      currCode[strlen(currCode) - 1] = '\0';
   }
}

/*This is apparently in the man page, so it should be in string.h,
 * but it's not.
 */
char *strdup(const char *str) {
   int n = strlen(str) + 1;
   char *dup = malloc(n);
   if(dup) {
      strcpy(dup, str);
   }
   return dup;
}
/*
 * Comparator for sorting.
 *
 * Returns < 0 if one < two
 * Returns > 0 if one > two
 */
int compare(char_node *one, char_node *two) {
   int result;

   result = (one -> count) - (two -> count);

   if(!result) {
      result = (one -> chr) - (two -> chr);
   }

   return result;
}

unsigned char *write_header(int numChars, size_t buffSize, int *counts) {

   unsigned char *buffer;
   int i = 4;
   int countsIndex;

   /*
    * buffer should be 4 (number of chars, int) + 5 (each block is
    * one byte for the char, then four bytes for the count of that
    * char) * numChars
    */
   buffer = malloc(buffSize);

   if(!buffer) {
      perror("write_header");
      exit(-1256);
   }

   /*
    * Writing initial size
    */
   *(int *)buffer = numChars;


   for(countsIndex = 0; countsIndex < ASCIISIZE; countsIndex++) {
      if(counts[countsIndex] != 0) {
         buffer[i] = (unsigned char)countsIndex;
         i++;
         *(int *)(buffer + i) = counts[countsIndex];
         i += 4;
      }
   }

   return buffer;
}

unsigned char write_bytes(int fd, unsigned char *buff, char *codes[],
      int numBytes, unsigned char unfinByte, int *bitCount) {
   short mask;
   unsigned char byte = unfinByte;
   int bCount = *bitCount;
   char *codeVal;
   ssize_t written;
   int i, j;

   for(i = 0; i < numBytes; i++) {
      codeVal = codes[buff[i]];

      for(j = 0; j < (int)strlen(codeVal); j++) {

         if(bCount == 8) {

            if((written = write(fd, &byte, 1)) < 0) {
               perror("write_byte");
               exit(-5778);
            }

            bCount = 0;
            byte = 0;
         }

         if(bCount != 8) {
            byte = byte << 1;

            if(codeVal[j] == '1') {
               mask = 1;
            } else {
               mask = 0;
            }

            byte = byte | mask;
            bCount++;
         }
      }
   }

   *bitCount = bCount;

   return byte;
}

void read_header(int *numChars, int *readCounts, unsigned char *buff) {
   /*
    * buffIndex == The index used to keep track of where you are in
    *    the buffer. It will be incremented by one or four bytes,
    *    depending on what will be read next
    * i == A simple indexer.
    * totalChars == This is used as both the total number of chars
    *    count (grabbed from the first four bytes of the file) and
    *    as an end index. When i == totalChars, the loop breaks
    * countsIndex == The actual character, and the index where
    *    the count for that character should be stored.
    */
   int i, totalChars, buffIndex;
   unsigned char countsIndex;

   /*
    * Grabbing the first 4 bytes, which is the total number of chars.
    * This will be used to rebuild the tree and to decode the file.
    */
   totalChars = *(int *)(buff);

   *numChars = totalChars;

   /*
    * Setting buff index to 4, to skip over the first 4 bytes
    * as that was the int
    */
   buffIndex = 4;
   i = 0;

   while(i < totalChars) {
      countsIndex = buff[buffIndex];
      buffIndex++;

      readCounts[countsIndex] = (int)buff[buffIndex];
      buffIndex += 4;

      i++;
   }
}


/*
 * This function is used to test the printing of the tree
 * Commented out because the output is not required in this
 * assignment
 */
/*
void printing(char_node *list) {

   if(!(list -> left) && !(list -> right)) {
      printf("Character: %c Count: %d\n", list -> chr, list -> count);
   } else {
      printf("Count left: %d\n", list -> left -> count);
      printing(list -> left);
      printf("Count right: %d\n", list -> right -> count);
      printing(list -> right);
   }
}
*/
