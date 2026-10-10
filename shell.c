/**
 * Description: This program creates multiple simultaneous processes for countnames.c and combines
 * the results to compute a total names count and print to stdout.
 * Author names: Ebsan Iqbal, Raymond Okolo
 * Author emails: ebsan.iqbal@sjsu.edu, raymond.okolo@sjsu.edu
 * Last modified date: 10/10/2026
 * Creation date: 10/5/2026
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "hashtable.h"

#define MAX_LINE 1024   // max characters read per prompt line
#define MAX_TOKENS 64   // max tokens (command + filenames) per line
#define NAME_LEN 32     // max length of name


//Message payload
typedef struct {
    char name[NAME_LEN];
    int count;
} NameCountData;

typedef enum {
    TYPE_NAMECOUNT,
    TYPE_OTHERTYPE // there is a possibility to extend with more types in the future
} MessageType;

//message header
typedef struct {
    MessageType type;
    size_t size; // Size of the following payload
} MessageHeader;

/**
  * This function reads and processes data from a returned message header and payload
  * Loops through hmesage header to get all bytes
  * Loops through message payload to get all bytes
  * inserts payload in hashtable
  * Returns: nothing
**/
void read_from_pipe(int fd) {
    MessageHeader header;

    while(1){
        char *p = (char *)&header;
        size_t g = 0;

        //looping to get all bytes from the header
        while(g < sizeof(header)){
            ssize_t n = read(fd, p + g, sizeof(header) - g);

            if (n < 0){
                fprintf(stderr, "error: bad message header\n");
                return;
            }
            if (n == 0) {
                if (g != 0) {
                    fprintf(stderr, "error: bad message header\n");
                }
                return;
            }
            g += n;
        }

        switch (header.type) {
        case TYPE_NAMECOUNT: {
                NameCountData d;
                char *q = (char *)&d;
                g = 0;
                while(g < sizeof(d)){
                    ssize_t n = read(fd, q + g, sizeof(d) - g);

                    if (n <= 0){
                        fprintf(stderr, "error: bad namecount payload\n");
                        return;
                    }
                    g += n;
                }

                // Process NameCountData data
                insertCount(d.name, d.count);
                break;
        }
        case TYPE_OTHERTYPE: {
                break;
        }
        default:
            // Handle unknown type error
            fprintf(stderr, "Unknown message type received: %d\n", header.type);
            return;
        }
    }
}

/**
  * This function creates child processes, each with its own pipe
  * each process executes the countnames program
  * Returns: -1 if pipe fails or data from the reading end of a pipe
**/
int makeChildren(char* cmd, char* filename){
    int fd[2];                       // fd[0] = reading end, fd[1] = writing end
    if (pipe(fd) == -1) {
        perror("pipe error");
        exit(1);
    }

    pid_t pid = fork();

    if(pid < 0){
        fprintf(stderr,"Fork failed\n");
        close(fd[0]);
        close(fd[1]);
        return -1;
    }
    if(pid == 0){
        close(fd[0]);                              // child only writes
        char childPidStr[16];
        char sfd[16];                              //string file descriptor
        snprintf(childPidStr, sizeof(childPidStr), "%d", getpid());
        snprintf(sfd, sizeof(sfd), "%d", fd[1]);

        execl(cmd, cmd, childPidStr, filename, sfd, NULL);

        // execl only returns if it failed
        fprintf(stderr,"error: cannot exec %s\n", cmd);
        exit(1);
    }
    // parent
    close(fd[1]);                              // parent only reads
    return fd[0];
}

/**
  * This function prints the names and number of occurrences in a readable format.
  * Returns: nothing
**/
void printNames(){
    for(int i = 0; i < nameCount; i++) {
        struct nlist *np = lookup(nameList[i]);
        for(int j = 0; j < np->nCount; j++) {
            if(strcmp(np->names[j], nameList[i]) == 0) {
                printf("%s: %d\n", nameList[i], np->counts[j]);
                break;
            }
        }
    }
}

int main() {
    int rEnd[MAX_TOKENS];   //stores read information from all children processes
    char line[MAX_LINE];

    while (1){
        int children = 0;
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
            // no files given: one child reads stdin ("1" tells countnames)
            int r = makeChildren(tokens[0], "1");
            if (r >= 0) {
                rEnd[children++] = r;
            }
        }else {
            // one child per input file, all running in parallel
            for (int i = 1; i < count; i++) {
                int r = makeChildren(tokens[0], tokens[i]);
                if (r >= 0) {
                    rEnd[children++] = r;
                }
            }
        }
        // after all children are forked: read each pipe until EOF, then close it
        for (int i = 0; i < children; i++) {
            read_from_pipe(rEnd[i]);
            close(rEnd[i]);
        }


        printNames();
        clearTable();       //clears hashtable for next command

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
            } else if (WIFSIGNALED(status)) {
                fprintf(stderr,
                        "Child %d terminated abnormally with signal number: %d\n",
                        pid,
                        WTERMSIG(status));
            }
        }
    }

    return 0;
}