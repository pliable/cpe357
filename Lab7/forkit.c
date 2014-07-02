#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/**
 *  forkit forks a parent process into a child, and reports the 
 *  process id of both the parent and child, then exits
 *
 *  @author Steve Choo
 * */
int main(int argc, char *argv[]) {
    pid_t child;
    int stat;

    printf("Hello, World!\n");

    child = fork();

    if(child == 0) {
        printf("This is the child, pid %d\n", getpid());
        exit(0);
    } else {
        printf("This is the parent, pid %d\n", getpid());
        wait(&stat);
        printf("This is the parent, pid %d, signing off!\n", getpid());
        exit(0);
    }

    return 0;
}
