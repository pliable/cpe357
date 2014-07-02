#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define BUFFSIZE 4096
#define BITS 8
#define ASCIISIZE 256

/**
 *  This program decodes a huffman encoded file
 *
 *  @author Steve Choo
 * */
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
int compare(char_node *one, char_node *two);
int read_header(int *numChars, int *readCounts, int *sumCounts, 
      unsigned char *buff, ssize_t numBytes);

int main(int argc, char *argv[]) {
   /*
    * fdIn/fdOut = file descriptor for input and output files, respectively
    *    will be set whether it's pointing to a file or stdout/in
    * currCount = The current count for the character
    * standIn/standOut = flags to see if input/output needs to be taken
    *    from stdin/out
    * numChars = Number of distinct characters
    * counts = Counts array denoting all the counts for all bytes
    * sumCounts = Total number of characters needed to be written
    * sumTracker = tracks number of characters written
    * buffIndex = index for the buffer that takes from read
    * firstRead = flag to tell whether an empty file is present
    * headerFlag = flag to tell if header has been written already
    * specialCaseChar = flag to tell if a one char situation has
    *    been encountered
    * mask = the mask used for reading bytes
    * numBytes = used to check numBytes returned from read/write
    * written = same as above, used in different context
    */
   int fdIn, fdOut, i, currCount;
   int standIn, standOut;
   int numChars, counts[ASCIISIZE], sumCounts = 0, sumTracker = 0;
   int buffIndex;
   int firstRead = 1;
   int headerFlag = 1;
   int specialCaseChar;
   short mask;
   ssize_t numBytes;
   ssize_t written;

   char *inputFileName, *outputFileName;

   unsigned char buffer[BUFFSIZE];
   unsigned char currChar;
   unsigned char currByte;
   unsigned char checkByte;

   char_node *head = NULL;
   char_node *new;
   char_node *currNode;

   /*
    * Emptying counts
    */
   for(i = 0; i < ASCIISIZE; i++) {
      counts[i] = 0;
   }

   /*
    * reading argv, seeing what needs to be done
    */
   if(argc == 1) {
      standIn = 1;
      standOut = 1;
   } else if(argc == 2) {

      if(argv[1][0] == '-') {
         standIn = 1;
      } else {
         inputFileName = argv[1];
         standIn = 0;
      }

      standOut = 1;
   } else if(argc == 3) {
      
      if(argv[1][0] == '-') {
         standIn = 1;
      } else {
         inputFileName = argv[1];
         standIn = 0;
      }

      outputFileName = argv[2];
      standOut = 0;
   }

   if(standIn == 0) {
      fdIn = open(inputFileName, O_RDONLY);

   } else {
      fdIn = STDIN_FILENO;
   }

   if(fdIn == -1) {
      perror(inputFileName);
      exit(-818);
   }

   if(standOut == 0) {
      fdOut = open(outputFileName, O_RDWR | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH |
            S_IWOTH);
   } else {
      fdOut = STDOUT_FILENO;
   }

   if(fdOut == -1) {
      perror("output_file_error");
      exit(-1257);
   }

   while((numBytes = read(fdIn, buffer, BUFFSIZE)) > 0) {
      firstRead = 0;

      /*
       * headerFlag is a check to see if the header was already
       * made. If it is, then the buffer needs to be read from
       * the beginning, otherwise it needs to be read at the
       * point the buffer ends
       */
      if(headerFlag) {
         headerFlag = 0;

         buffIndex = read_header(&numChars, counts, &sumCounts, 
               buffer, numBytes);

         /*
          * Populating linked list
          */
         for(i = 0; i < ASCIISIZE; i++) {
            if(counts[i] != 0) {
               currCount = counts[i];
               currChar = (unsigned char)i;

               new = new_node(currCount, currChar, NULL, NULL);
               head = insert_sorted_ll(head, new);
            }
         }

         head = create_tree(head);
         currNode = head;
               
      } else {
         buffIndex = 0;
      }

      while(buffIndex < numBytes) {
         /*
          * Grabbing current byte
          */
         currByte = buffer[buffIndex++];

         mask = 0x80;

         /*
          * BITS #defined at top
          * Going through until this byte is filled.
          */
         for(i = 0; i < BITS; i++) {
            checkByte = currByte & mask;

            /*
             * A check to attempt to combat segfaults
             */
            if(currNode != NULL) {

               if(checkByte != mask) {
                  currNode = currNode -> left;
               } else {
                  currNode = currNode -> right;
               }

               /*
                * Another redundant check to combat
                * segfaults
                */
               if(currNode != NULL) {

                  if((currNode -> left == NULL) &&
                        (currNode -> right == NULL)) {
                     if((written = write(fdOut, &(currNode -> chr), 1) < 0)) {
                        perror("writing_chars");
                        exit(-1839);
                     }

                     sumTracker++;

                     if(sumTracker == sumCounts) {
                        break;
                     }
                     currNode = head;
                  }
                  mask = mask >> 1;
               }
            }
         }
      }
   }

   /*
    * Simple check to see if an empty file was read
    */
   if(firstRead) {
      exit(1);
   }

   if(numChars == 1) {
      i = 0;
      while(counts[i] != 0)
         i++;

      specialCaseChar = i;

      for(i = 0; i < sumCounts; i++) {
         if((written = write(fdOut, &(head -> chr), 1) < 0)) {
            perror("writing_special_case");
            exit(-1578);
         }
      }
   }

   /*
    * Test printer
    */
   /*
   printf("Total Char Count: %d\n", numChars);

   for(i = 0; i < ASCIISIZE; i++) {
      if(counts[i] != 0) {
         printf("Character: %c Count: %d\n", i, counts[i]);
      }
   }
   */

   return 0;
}


/*
 * numChars = This will be determined by this function, so just pass
 *    in the value.
 * readCounts = The count array...just pass this sucker in as well
 * buff = the buffer read from the file
 * Returns the index to continue reading from
 */
int read_header(int *numChars, int *readCounts, int *sumCounts,
      unsigned char *buff, ssize_t numBytes) {
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
   unsigned int check;
   unsigned char countsIndex;

   /*
    * Grabbing the first 4 bytes, which is the total number of chars.
    * This will be used to rebuild the tree and to decode the file.
    */
   totalChars = *(int *)(buff);

   check = 4 + 5 * totalChars;

   if(check > (unsigned int)numBytes) {
      perror("File_Corruption");
      exit(-1275);
   }

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

      readCounts[countsIndex] = *(int *)(buff + buffIndex);
      *sumCounts += *(int *)(buff + buffIndex);
      buffIndex += 4;

      i++;
   }

   return buffIndex;
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
