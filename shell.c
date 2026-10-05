/**
 * Description: This program creates multiple simultaneous processes for countnames.c
 * Author names: Ebsan Iqbal, Raymond Okolo
 * Author emails: ebsan.iqbal@sjsu.edu, raymond.okolo@sjsu.edu
 * Last modified date: 9/23/2026
 * Creation date: 9/20/2026
 **/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAX_LINE 1024   // max characters read per prompt line
#define MAX_TOKENS 64   // max tokens (command + filenames) per line

void makeChildren(char *cmd, char *filename){
    pid_t pid = fork();

    if(pid < 0){
        printf("Fork failed");
    }
    else if(pid == 0){
        char childPidStr[16];
        snprintf(childPidStr, sizeof(childPidStr), "%d", getpid());

        execl(cmd, cmd, childPidStr, filename, NULL);

        // execl only returns if it failed
        fprintf(stderr,"error: cannot exec %s\n", cmd);
        exit(1);
    }

}
int main(int argc, char *argv[]) {
    /*
    if(argc > 1) {
        for(int i = 1; i < argc; i++){
            pid_t pid = fork();
            if(pid < 0){
                printf("Fork failed");
            }
            else if(pid == 0){
                pid_t childPid = getpid();
                char childPidStr[16];
                snprintf(childPidStr, sizeof(childPidStr), "%d", childPid);

                execl("./countnames", "countnames", childPidStr, argv[i], NULL);
                fprintf(stderr,"error: cannot open file\n");
                exit(1);
            }
        }
    }else if (argc == 1){
        pid_t pid = fork();
        if(pid < 0){
            printf("Fork failed");
        }
        if(pid == 0) {
            pid_t childPid = getpid();
            char childPidStr[16];
            snprintf(childPidStr, sizeof(childPidStr), "%d", childPid);

            execlp("./countnames", "countnames", childPidStr, "1", NULL);
            fprintf(stderr,"error: cannot open file\n");
            exit(1);
        }
    }
    */


    char line[MAX_LINE];

    while (1){
        printf("%% ");
        fflush(stdout);

        // EOF (Ctrl+D) ends the shell
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        // split the line into whitespace-separated tokens
        char *tokens[MAX_TOKENS];
        int count = 0;
        char *token = strtok(line, " \t\r\n");
        while (token != NULL && count < MAX_TOKENS) {
            tokens[count++] = token;
            token = strtok(NULL, " \t\r\n");
        }

        if (count == 0) {
            continue;   // blank line: just show the prompt again
        }
        if (strcmp(tokens[0], "exit") == 0) {
            break;
        }

        if (count == 1) {
            // no files given: one child reads stdin ("1" tells countnames so)
            makeChildren(tokens[0], "1");
        }
        else {
            // one child per input file, all running in parallel
            for (int i = 1; i < count; i++) {
                makeChildren(tokens[0], tokens[i]);
            }
        }


        // Parent: reap every child so none are left as zombies, and report
        // whether each exited normally or was killed by a signal.
        int status;
        pid_t pid;
        while ((pid = wait(&status)) > 0) {
            if (WIFEXITED(status)) {
                fprintf(stderr,
                        "Child %d terminated normally with exit code: %d\n",
                        pid,
                        WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status)) {
                fprintf(stderr,
                        "Child %d terminated abnormally with signal number: %d\n",
                        pid,
                        WTERMSIG(status));
            }
        }
    }

    return 0;
}