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
/*
 * Printing is commented out due to not needing to print
 * the tree.
 */
void printing(char_node *list);
/**
 *  This program reads in a file and produces a printout of
 *  huffman encodings
 *
 *  @author Steve Choo
 * */
int main(int argc, char *argv[]) {
   /* fd == file descriptor
    * numBytes == Current number of bytes read
    * i == used as an index, but also as the numeric representation
    *      of a character
    * currCount == The current count when populating the linked list
    * counts[] == The array of counts
    */
   int fd, i, currCount;
   int counts[ASCIISIZE];
   int firstRead = 1;
   ssize_t numBytes;

   /* *fileName == The file name to be read
    * buffer == The buffer for reading in chunks
    * currChar == The current character when populating the linked list
    * codes == These will store the encoded version of the chars
    * currCode == The current code to append to the codes
    */
   char *fileName;
   unsigned char buffer[BUFFSIZE];
   unsigned char currChar;
   char *codes[ASCIISIZE];
   char currCode[ASCIISIZE];

   /*
    * head == the linked list, and eventually, the tree
    * new == used to insert a new node
    */
   char_node *head = NULL;
   char_node *new;

   /*
    * Emptying string
    */
   currCode[0] = '\0';

   /*
    * Checking argc for correct amount of arguments.
    * Needs to be 2, as it has to have a file name
    */
   if(argc != 2) {
      printf("Specify file to compress. Program exiting...\n");
      exit(EXIT_FAILURE);
   }

   /* Assigning the file name internally...should be a more
    * descriptive way to do this instead of using argv[1]
    */
   fileName = argv[1];

   /*
    * Opening file, assigning file descriptor to fd for future use
    */
   fd = open(fileName, O_RDONLY);

   /*
    * Checking if opening succeeded...if not, program terminates
    */
   if(fd == -1) {
      perror(fileName);
      exit(EXIT_FAILURE);
   }

   /* 
    * The idea behind this is to collect the counts of each
    * character. Each index represents a different character,
    * and so when a character is encountered, the count
    * for that respective index increments by one.
    */
   for(i = 0; i < ASCIISIZE; i++) {
      counts[i] = 0;
   }

   /* These loops reads through the file, counting up
    * every type of character
    */
   while((numBytes = read(fd, buffer, BUFFSIZE)) > 0) {
      firstRead = 0;
      for(i = 0; i < numBytes; i++) {
         counts[buffer[i]]++;
      }
   }

   if(firstRead) {
      exit(-1);
   }

   /* This loop will iterate through the counts array,
    * populating a linked list. If the count is 0, it is skipped
    */
   for(i = 0; i < ASCIISIZE; i++) {
      if(counts[i] != 0) {
         currCount = counts[i];
         currChar = (unsigned char)i;

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

   head = create_tree(head);

   encoding(head, currCode, codes);

   for(i = 0; i < ASCIISIZE; i++) {
      if(codes[i] != NULL) {
         printf("0x%02x: %s\n", i, codes[i]);
      }
   }

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
