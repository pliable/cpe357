#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LEN 512
#define MAX_PIPE 10
#define MAX_ARGS 10

/*
 * Codes to tell where input/output is coming from
 *
 * INPUT_FROM_STDIN and OUTPUT_TO_STDOUT was going to be used,
 * but I just remembered that I wanted to use them when I looked
 * back at this section to write this comment. Dammit.
 */
#define INPUT_FROM_STDIN 1
#define INPUT_FROM_REDIRECT 2
#define INPUT_FROM_PIPE 3
#define OUTPUT_TO_STDOUT 4
#define OUTPUT_TO_REDIRECT 5
#define OUTPUT_TO_PIPE 6

/*
 * Codes for bad command lines
 */
#define VALID 0
#define INVALID_NULL_COMMAND 1
#define BAD_INPUT_REDIRECTION 2
#define BAD_OUTPUT_REDIRECTION 3
#define AMBIGUOUS_INPUT 4
#define AMBIGUOUS_OUTPUT 5

struct cmd_line {
   int input;
   int output;
   char *cmd;
   char *input_from;
   char *output_to;
   int argCount;
   char *argVars[MAX_ARGS];
};
typedef struct cmd_line cmd_line;

int parse(cmd_line *cmd, char *pipeToken, int stage, int totalStages);
void print_error(int type, char *cmd);
void print_stages(cmd_line cmd, int stage);
void initialize_cmd_structs(cmd_line *cmd);

int redirection_check(char *pipeToken);
int ambiguity_check(char *pipeToken, int stage, int totalStages);
int pipeline_empty_check(char *pipeToken);
int check_stages(char *str, size_t strLen);

/**
 *  This program will parse a given line into arguments, pipes,
 *  etc.
 *
 *  It is a colossal mess. I have no idea how it even partly works.
 *
 *  @author Steve Choo
 * */
int main(int argc, char *argv[]) {
   /*
    * cmdStr = the entire string
    * pipeToken = a command tokenized by pipes
    * saveptr = used for strtok_r to keep track
    * stageCmd = used to store the stages command string
    * strLength = length of the string
    * numStages = number of stages in pipeline
    * integrity = will be 0 if everything checks out okay,
    *    otherwise, an error code specified in the #define
    *    fields will be used to indicate the error
    * cmdLine = array of structs for commands
    */
   char cmdStr[MAX_LEN];
   char *pipeToken;
   char *saveptr;
   char *stageCmd;
   size_t strLength;
   int numStages, i;
   int integrity;
   cmd_line cmdLine[MAX_PIPE];

   /*
    * Prompt
    */
   printf("line: ");
   /*
    * Pulls in line
    */
   if(!fgets(cmdStr, MAX_LEN, stdin)) {
      printf("Error reading line\n");
      exit(-1);
   }

   /*
    * Checking for '\n'. If it isn't found, the command line must be longer
    * than the max specified, and prints a message and errors out
    */
   if(!strchr(cmdStr, '\n')) {
      fprintf(stderr, "command too long\n");
      exit(-1);
   }

   strLength = strlen(cmdStr);
   /*
    * getting rid of pesky '\n' for future printing
    */
   cmdStr[(strLength - 1)] = '\0';

   /*
    * checking depth of pipeline
    */
   if((numStages = check_stages(cmdStr, strLength)) > 10) {
      fprintf(stderr, "pipeline too deep\n");
      exit(-1);
   }

   /*
    * Initializing structs
    */
   initialize_cmd_structs(cmdLine);

   /*
    * tokenizing by pipe and checking integrity of each sub-string
    */
   pipeToken = strtok_r(cmdStr, "|", &saveptr);
   for(i = 0; i < numStages; i++) {

      /*
       * mallocing to store the entire command string
       */
      stageCmd = malloc(sizeof(char) * strlen(pipeToken) + 1);
      strcpy(stageCmd, pipeToken);
      cmdLine[i].cmd = stageCmd;

      if((integrity = parse(&cmdLine[i], pipeToken, i, numStages))
            != VALID) {
         print_error(integrity, cmdLine[i].cmd);
      }
      pipeToken = strtok_r(NULL, "|", &saveptr);
   }

   for(i = 0; i < numStages; i++) {
      print_stages(cmdLine[i], i);
   }

   return 0;
}

void print_stages(cmd_line cmd, int stage) {
   int i;

   printf("\n--------\n");
   printf("Stage %d: \"%s\"\n", stage, cmd.cmd);
   printf("--------\n");

   if(!(strcmp("pipe from stage ", cmd.input_from))) {
      printf("     input: %s%d\n", cmd.input_from, stage - 1);
   } else {
      printf("     input: %s\n", cmd.input_from);
   }

   if(!(strcmp("pipe to stage ", cmd.output_to))) {
      printf("    output: %s%d\n", cmd.output_to, stage + 1);
   } else {
      printf("    output: %s\n", cmd.output_to);
   }

   printf("      argc: %d\n", cmd.argCount);
   printf("      argv: ");

   for(i = 0; i < cmd.argCount; i++) {
      printf("\"%s\"", cmd.argVars[i]);

      if(i + 1 == cmd.argCount) {
         break;
      }

      putchar(',');
   }
   putchar('\n');
}


/*
 * Initializes the command structs to zeroed out vals
 */
void initialize_cmd_structs(cmd_line *cmd) {
   int i;

   for(i = 0; i < MAX_PIPE; i++) {
      cmd[i].input = 0;
      cmd[i].output = 0;
      cmd[i].cmd = '\0';
      cmd[i].input_from = '\0';
      cmd[i].output_to = '\0';
      cmd[i].argCount = 0;
   }
}

/*
 * Checks the integrity of each sub-string, and also populates
 * the command structs if the integrity checks out
 *
 * returns 0 if command checks out okay, otherwise:
 *
 * returns 1 if there is an invalid null command
 * returns 2 if there is a bad input redirection
 * returns 3 if there is a bad output redirection
 * returns 4 is there is ambiguous input
 * returns 5 if there is ambiguous output
 *
 * This thing is an absolute mess. I will honestly be surprised
 * if it can be remotely followed. I don't even know how I followed it.
 *
 */
int parse(cmd_line *cmd, char *pipeToken, int stage, int totalStages) {
   /*
    * cmdTok = the command line for this stage
    * input = used for the command struct to print a string
    * output = same as input, except for output
    * temp = used as a temp string pointer for argv's
    * argC = counter used for argC
    * argV = indexer for argV
    * result = the final result of parsing
    */
   char *cmdTok;
   char *input;
   char *output;
   char *temp;
   int argC = 0;
   int argV = 0;
   int result;
   int redirOutFlag = 0;
   int pipeOutFlag = 0;
   int redirInFlag = 0;
   int pipeInFlag = 0;

   if(pipeline_empty_check(pipeToken)) {
      result = INVALID_NULL_COMMAND;
   } else if((result = redirection_check(pipeToken)) != VALID) {
      /*result is set, do nothing here*/
   } else if((result = ambiguity_check(pipeToken, stage, totalStages))
         != VALID) {
      /*result is set, do nothing here*/
   } else {
      cmdTok = strtok(pipeToken, " ");

      /*
       * Idea here is to set inputs/outputs if there is a
       * pipeline.
       *
       * It sucks, I know.
       */
      if(stage == 0 && totalStages > 1) {
         cmd -> output = OUTPUT_TO_PIPE;
         cmd -> output_to = "pipe to stage ";
         pipeOutFlag = 1;
      } else if((stage > 0) && ((stage + 1) < totalStages)) {
         cmd -> input = INPUT_FROM_PIPE;
         cmd -> input_from = "pipe from stage ";
         cmd -> output = OUTPUT_TO_PIPE;
         cmd -> output_to = "pipe to stage ";
         pipeInFlag = 1;
         pipeOutFlag = 1;
      } else if((stage > 0) && (stage + 1) == totalStages) {
         cmd -> input = INPUT_FROM_PIPE;
         cmd -> input_from = "pipe from stage ";
         pipeInFlag = 1;
      } 

      /*
       * Tokenizing each subtoken, by space
       *
       * Again, a mess. I'm not even sure what the hell I did here
       * I'll be cleaning it up later just cause this is terribly
       * embarassing logic. Don't try reading it, your head will
       * cave in, and your chest will explode. Mine certainly did.
       */
      while(cmdTok) {
         if(strlen(cmdTok) > 1) {
            argC++;
            cmd -> argCount = argC;

            temp = malloc(sizeof(char) * strlen(cmdTok) + 1);

            strcpy(temp, cmdTok);

            cmd -> argVars[argV] = temp;

            temp = NULL;
            argV++;

         } else {

            if(strcmp(cmdTok, "<") == 0) {
               redirInFlag = 1;
               cmd -> input = INPUT_FROM_REDIRECT;
               cmdTok = strtok(NULL, " ");

               if(cmdTok == NULL || strchr(cmdTok, '>')) {
                  result = BAD_INPUT_REDIRECTION;
                  break;
               }
               
               input = malloc(sizeof(char) * strlen(cmdTok) + 1);

               strcpy(input, cmdTok);

               cmd -> input_from = input;

            } else if(strcmp(cmdTok, ">") == 0) {
               redirOutFlag = 1;
               cmd -> output = OUTPUT_TO_REDIRECT;
               cmdTok = strtok(NULL, " ");

               if(cmdTok == NULL || strchr(cmdTok, '<')) {
                  result = BAD_OUTPUT_REDIRECTION;
                  break;
               }

               output = malloc(sizeof(char) * strlen(cmdTok) + 1);

               strcpy(output, cmdTok);

               cmd -> output_to = output;
            } else {
               argC++;
               cmd -> argCount = argC;

               temp = malloc(sizeof(char) * strlen(cmdTok) + 1);

               strcpy(temp, cmdTok);

               cmd -> argVars[argV] = temp;

               temp = NULL;
               argV++;
            }

         }

         cmdTok = strtok(NULL, " ");
      }

      if(redirInFlag == 0 && pipeInFlag == 0) {
         cmd -> input_from = "original stdin";
      }

      if(redirOutFlag == 0 && pipeOutFlag == 0) {
         cmd -> output_to = "original stdout";
      }

      result = VALID;
   }

   return result;
}


/*
 * checks if a stage is empty
 *
 * returns 0 if everything is a-okay
 * returns 1 if this token sucks
 */
int pipeline_empty_check(char *pipeToken) {
   unsigned int i;
   int check = 1;

   for(i = 0; i < strlen(pipeToken); i++) {
      if(!isspace(pipeToken[i])) {
         check = 0;
         break;
      }
   }

   return check;
}


/*
 * checks if there are multiple redirections
 *
 * returns 0 if everything is okay
 * returns 2 if bad input redirection
 * returns 3 if bad output redirection
 */
int redirection_check(char *pipeToken) {
   int input_count = 0;
   int output_count = 0;
   unsigned int i;

   for(i = 0; i < strlen(pipeToken); i++) {
      if(pipeToken[i] == '<') {
         input_count++;
      } else if(pipeToken[i] == '>') {
         output_count++;
      }
   }

   if(input_count >= 2) {
      return BAD_INPUT_REDIRECTION;
   } else if(output_count >= 2) {
      return BAD_OUTPUT_REDIRECTION;
   } else {
      return VALID;
   }
}

/*
 * checks for ambiguity in input/output
 *
 * returns 0 if everything is okay
 * returns 4 for ambiguous input
 * returns 5 for ambiguous output
 */
int ambiguity_check(char *pipeToken, int stage, int totalStages) {
   int check = VALID;

   if(totalStages == 1) {
      return check;
   }

   if(stage == 0) {
      if(strchr(pipeToken, '>')) {
         check = AMBIGUOUS_OUTPUT;
      }
   } else if((stage + 1) < totalStages) {
      if(strchr(pipeToken, '>')) {
         check = AMBIGUOUS_OUTPUT;
      } else if(strchr(pipeToken, '<')) {
         check = AMBIGUOUS_INPUT;
      }
   } else if((stage + 1) == totalStages) {
      if(strchr(pipeToken, '<')) {
         check = AMBIGUOUS_INPUT;
      }
   }

   return check;
}

/*
 * used to print errors if they occured, and will also exit
 * the program with a non-zero number
 */
void print_error(int type, char *cmd) {
   switch(type) {
      case INVALID_NULL_COMMAND:
         fprintf(stderr, "invalid null command\n");
         break;
      case BAD_INPUT_REDIRECTION:
         fprintf(stderr, "%s: bad input redirection\n", cmd);
         break;
      case BAD_OUTPUT_REDIRECTION:
         fprintf(stderr, "%s: bad output redirection\n", cmd);
         break;
      case AMBIGUOUS_INPUT:
         fprintf(stderr, "%s: ambiguous input\n", cmd);
         break;
      case AMBIGUOUS_OUTPUT:
         fprintf(stderr, "%s: amibigous output\n", cmd);
         break;
      default:
         fprintf(stderr, "How the heck did you get here?\n");
         break;
   }

   exit(-2);
}

/*
 * checks the length of the pipeline
 *
 * returns length of the pipeline
 */
int check_stages(char *str, size_t strLen) {
   int pipeCount = 0;
   unsigned int i;

   for(i = 0; i < strLen; i++) {
      if(str[i] == '|') {
         pipeCount++;
      }
   }

   return ++pipeCount;
}
