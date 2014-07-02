#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

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
#define TOO_MANY_ARGS 6

#define READ_END 0
#define WRITE_END 1

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
void clear_cmd_structs(cmd_line *cmd, int numStages);

int redirection_check(char *pipeToken, cmd_line *cmd);
int ambiguity_check(char *pipeToken, cmd_line *cmd,
      int stage, int totalStages);
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
   size_t strLength;
   int numStages, i, j, eofStatus, parseError = 0;
   int integrity;
   cmd_line cmdLine[MAX_PIPE];
   int pipes[MAX_PIPE][2];
   pid_t child[MAX_PIPE];
   int numChildren;
   FILE *strm;
   int fdIn;
   int fdOut;
   int childStat;

   if(argc == 1) {
      strm = stdin;
   } else {
      if(!(strm = fopen(argv[1], "r"))) {
         perror("fopen");
         exit(-1);
      }
   }
      
   /*
    * Pulls in line. Using outer infinite loop so if an error occurs,
    * it can break and reloop through.
    */
   while(1) {
      while(!(eofStatus = feof(strm))) {
         /*
          * Prompt
          */
         if(argc == 1) {
            printf("8-P ");
         }

         /*
          * Grab line
          */
         if(!fgets(cmdStr, MAX_LEN, strm)) {
            if(!(eofStatus = feof(strm))) {
               fprintf(stderr, "Error reading line\n");
            }
            break;
         }

         /*
          * Checking for '\n'. If it isn't found, the command line must be 
          * longer than the max specified, and prints a message and errors out
          */
         if(!strchr(cmdStr, '\n')) {
            fprintf(stderr, "command too long\n");
            break;
         }

         if((strLength = strlen(cmdStr)) == 1) {
            break;
         }
         /*
          * getting rid of pesky '\n' for future printing
          */
         cmdStr[(strLength - 1)] = '\0';

         /*
          * checking depth of pipeline
          */
         if((numStages = check_stages(cmdStr, strLength)) > 10) {
            fprintf(stderr, "pipeline too deep\n");
            break;
         }

         numChildren = numStages;

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
             * mallocing and copying to store the entire command string, 
             * as strtok will mutate string with each call
             */
            cmdLine[i].cmd = malloc(sizeof(char) * strlen(pipeToken) + 1);
            strcpy(cmdLine[i].cmd, pipeToken);

            if((integrity = parse(&cmdLine[i], pipeToken, i, numStages))
                  != VALID) {
               print_error(integrity, cmdLine[i].argVars[0]);
               parseError = 1;
               break;
            }
            pipeToken = strtok_r(NULL, "|", &saveptr);
         }

         if(parseError) {
            /* resetting parseError flag */
            parseError = 0;
            break;
         }

         /*
          * Parsing complete at this point
          */

         /*
          * Checking for cd. If there, shell will
          * change dirs
          */
         if(strcmp(cmdLine[0].argVars[0], "cd") == 0) {
            if(chdir(cmdLine[0].argVars[1]) == -1) {
               perror("chdir");
               break;
            }
            break;
         }

         /*
          * Initializing pipeline
          */
         if(numStages > 1) {
            for(i = 0; i < numStages - 1; i++) {
               if(pipe(pipes[i])) {
                  perror("Pipeline");
                  exit(-1);
               }
            }
         }

         /*
          * Note for self: remember off by one...i = 0 is the 1st
          * stage of pipeline, stage 0, and FIRST child.
          */
         for(i = 0; i < numStages; i++) {

            if(i == 0) {

               if(!(child[0] = fork())) {
                  /* child stuff */

                  /*
                   * Checking if input is from redirect
                   */
                  if(cmdLine[0].input == INPUT_FROM_REDIRECT) {
                     if((fdIn = open(cmdLine[0].input_from, O_RDONLY))
                           == -1) {
                        perror("open input");
                        exit(-1);
                     }

                     if(dup2(fdIn, STDIN_FILENO) == -1) {
                        perror("dup2 input");
                        exit(-1);
                     }
                  }

                  /*
                   * Checking where to output
                   */
                  if(cmdLine[0].output == OUTPUT_TO_REDIRECT) {
                     if((fdOut = open(cmdLine[0].output_to, O_WRONLY |
                                 O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR |
                                 S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH))
                           == -1) {
                        perror("open output");
                        exit(-1);
                     }

                     if(dup2(fdOut, STDOUT_FILENO) == -1) {
                        perror("dup2 output");
                        exit(-1);
                     }
                  } else if(cmdLine[0].output == OUTPUT_TO_PIPE) {
                     if(dup2(pipes[0][WRITE_END], STDOUT_FILENO) == -1) {
                        perror("dup2 output");
                        exit(-1);
                     }
                  }

                  /* cleaning up */
                  if(numStages > 1) {
                     for(j = 0; j < numStages - 1; j++) {
                        close(pipes[j][READ_END]);
                        close(pipes[j][WRITE_END]);
                     }
                  }

                  /* executing */
                  execvp(cmdLine[0].argVars[0], cmdLine[0].argVars);
                  perror(cmdLine[0].argVars[0]);
                  exit(-1);
               }
            } else if(i > 0 && (i + 1 < numStages)) {

               if(!(child[i] = fork())) {
                  /*child stuff*/
                  if(dup2(pipes[i - 1][READ_END], STDIN_FILENO) == -1) {
                     perror("dup2 input");
                     exit(-1);
                  }

                  if(dup2(pipes[i][WRITE_END], STDOUT_FILENO) == -1) {
                     perror("dup2 output");
                     exit(-1);
                  }

                  /* cleaning up */
                  for(j = 0; j < numStages - 1; j++) {
                     close(pipes[j][READ_END]);
                     close(pipes[j][WRITE_END]);
                  }

                  /* executing */
                  execvp(cmdLine[i].argVars[0], cmdLine[i].argVars);
                  perror(cmdLine[i].argVars[0]);
                  exit(-1);
               }
            } else if(i > 0 && (i + 1 == numStages)) {

               if(!(child[i] = fork())) {
                  /*child stuff*/
                  if(dup2(pipes[i - 1][READ_END], STDIN_FILENO) == -1) {
                     perror("dup2 input");
                     exit(-1);
                  }

                  /*
                   * For the last stage, figuring out if
                   * output needs to go to stdout or
                   * be redirected
                   */
                  if(cmdLine[i].output == OUTPUT_TO_REDIRECT) {
                     if((fdOut = open(cmdLine[i].output_to, O_WRONLY |
                                 O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR |
                                 S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH))
                           == -1) {
                        perror("open output");
                        exit(-1);
                     }

                     if(dup2(fdOut, STDOUT_FILENO) == -1) {
                        perror("dup2 output");
                        exit(-1);
                     }
                  }

                  /* cleaning up */
                  for(j = 0; j < numStages - 1; j++) {
                     close(pipes[j][READ_END]);
                     close(pipes[j][WRITE_END]);
                  }

                  /* executing */
                  execvp(cmdLine[i].argVars[0], cmdLine[i].argVars);
                  perror(cmdLine[i].argVars[0]);
                  exit(-1);
               }
            }
         }

         /* parent stuff */

         /* cleaning up */
         for(i = 0; i < numStages - 1; i++) {
            close(pipes[i][READ_END]);
            close(pipes[i][WRITE_END]);
         }

         i = 0;
         while(numChildren) {
            while(waitpid(child[i], &childStat, 0) == -1)
               ;

            if(!(WIFEXITED(childStat))) {
               fprintf(stderr, "Child exited abnormally: %d\n",
                     WEXITSTATUS(childStat));
            }

            if(WIFSIGNALED(childStat)) {
               fprintf(stderr, "Child terminated by signal: %d\n",
                     WTERMSIG(childStat));
            }
            numChildren--;
            i++;
         }

         clear_cmd_structs(cmdLine, numStages);
      }

     clear_cmd_structs(cmdLine, numStages);

      if(eofStatus) {
         break;
      }
   }

   putchar('\n');

   /*
   for(i = 0; i < numStages; i++) {
      print_stages(cmdLine[i], i);
   }
   */

   return 0;
}

void print_stages(cmd_line cmd, int stage) {
   int i;

   printf("\n--------\n");
   printf("Stage %d: \"%s\"\n", stage, cmd.cmd);
   printf("--------\n");

   if(cmd.input == INPUT_FROM_STDIN) {
      printf("     input: original stdin\n");
   } else if(cmd.input == INPUT_FROM_REDIRECT) {
      printf("     input: %s\n", cmd.input_from);
   } else if(cmd.input == INPUT_FROM_PIPE) {
      printf("     input: pipe from stage %d\n", stage - 1);
   }

   if(cmd.output == OUTPUT_TO_STDOUT) {
      printf("    output: original stdout\n");
   } else if(cmd.output == OUTPUT_TO_REDIRECT) {
      printf("    output: %s\n", cmd.output_to);
   } else if(cmd.output == OUTPUT_TO_PIPE) {
      printf("    output: pipe to stage %d\n", stage + 1);
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
   int i, j;

   for(i = 0; i < MAX_PIPE; i++) {
      cmd[i].input = 0;
      cmd[i].output = 0;
      cmd[i].cmd = '\0';
      cmd[i].input_from = '\0';
      cmd[i].output_to = '\0';
      cmd[i].argCount = 0;

      for(j = 0; j < MAX_ARGS; j++) {
         cmd[i].argVars[j] = '\0';
      }
   }
}

/*
 * Clears command structs for new command line
 */
void clear_cmd_structs(cmd_line *cmd, int numStages) {
   int i;

   for(i = 0; i < numStages; i++) {
      free(cmd[i].cmd);
   }

   initialize_cmd_structs(cmd);
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
    * argV = indexer for argV
    * result = the final result of parsing
    */
   char *cmdTok;
   int argV = 0;
   int result = -1;
   int redirOutFlag = 0;
   int pipeOutFlag = 0;
   int redirInFlag = 0;
   int pipeInFlag = 0;

   if(pipeline_empty_check(pipeToken)) {
      result = INVALID_NULL_COMMAND;
   } else if((result = redirection_check(pipeToken, cmd)) != VALID) {
      /*result is set, do nothing here*/
   } else if((result = ambiguity_check(pipeToken, cmd, stage, totalStages))
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
         pipeOutFlag = 1;
      } else if((stage > 0) && (stage + 1 < totalStages)) {
         cmd -> input = INPUT_FROM_PIPE;
         cmd -> output = OUTPUT_TO_PIPE;
         pipeInFlag = 1;
         pipeOutFlag = 1;
      } else if((stage > 0) && (stage + 1 == totalStages)) {
         cmd -> input = INPUT_FROM_PIPE;
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
            cmd -> argCount++;

            if((cmd -> argCount) > MAX_ARGS) {
               result = TOO_MANY_ARGS;
               break;
            }

            cmd -> argVars[argV] = cmdTok;

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

               cmd -> input_from = cmdTok;
             
            } else if(strcmp(cmdTok, ">") == 0) {
               redirOutFlag = 1;
               cmd -> output = OUTPUT_TO_REDIRECT;
               cmdTok = strtok(NULL, " ");

               if(cmdTok == NULL || strchr(cmdTok, '<')) {
                  result = BAD_OUTPUT_REDIRECTION;
                  break;
               }

               cmd -> output_to = cmdTok;

            } else {
               cmd -> argCount++;

               if((cmd -> argCount) > MAX_ARGS) {
                  result = TOO_MANY_ARGS;
                  break;
               }

               cmd -> argVars[argV] = cmdTok;

               argV++;
            }

         }

         cmdTok = strtok(NULL, " ");
      }

      if(redirInFlag == 0 && pipeInFlag == 0) {
         cmd -> input = INPUT_FROM_STDIN;
      }

      if(redirOutFlag == 0 && pipeOutFlag == 0) {
         cmd -> output = OUTPUT_TO_STDOUT;
      }

      /*
       * Set to -1 originally, if everything checked out okay,
       * it should still be -1 at this point
       */
      if(result == -1) {
         result = VALID;
      }
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
int redirection_check(char *pipeToken, cmd_line *cmd) {
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
      cmd -> argVars[0] = strtok(pipeToken, " ");
      return BAD_INPUT_REDIRECTION;
   } else if(output_count >= 2) {
      cmd -> argVars[0] = strtok(pipeToken, " ");
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
int ambiguity_check(char *pipeToken, cmd_line *cmd,
      int stage, int totalStages) {
   int check = VALID;

   if(totalStages == 1) {
      return check;
   }

   if(stage == 0) {
      if(strchr(pipeToken, '>')) {
         cmd -> argVars[0] = strtok(pipeToken, " ");
         check = AMBIGUOUS_OUTPUT;
      }
   } else if((stage + 1) < totalStages) {
      if(strchr(pipeToken, '>')) {
         cmd -> argVars[0] = strtok(pipeToken, " ");
         check = AMBIGUOUS_OUTPUT;
      } else if(strchr(pipeToken, '<')) {
         cmd -> argVars[0] = strtok(pipeToken, " ");
         check = AMBIGUOUS_INPUT;
      }
   } else if((stage + 1) == totalStages) {
      if(strchr(pipeToken, '<')) {
         cmd -> argVars[0] = strtok(pipeToken, " ");
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
      case TOO_MANY_ARGS:
         fprintf(stderr, "%s: too many arguments\n", cmd);
         break;
      default:
         fprintf(stderr, "How the heck did you get here?\n");
         break;
   }
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
