/**
 * Description: This program counts how many times each individual name appears across one or more files.
 * Author names: Ebsan Iqbal, Raymond Okolo
 * Author emails: ebsan.iqbal@sjsu.edu, raymond.okolo@sjsu.edu
 * Last modified date: 10/5/2026
 * Creation date: 9/2/2026
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "hashtable.h"

#define NAME_LEN 32

typedef struct {
    char name[NAME_LEN];
    int count;
} NameCountData;

typedef enum {
    TYPE_NAMECOUNT,
    TYPE_OTHERTYPE // there is a possibility to extend with more types in the future
} MessageType;

typedef struct {
    MessageType type;
    size_t size; // Size of the following payload
} MessageHeader;


/**
  * This function creates a message and header struct and writes both to parent process
  * Returns: nothing
**/
void write_struct_namecount(int fd, NameCountData *data) {
    MessageHeader header;
    header.type = TYPE_NAMECOUNT;
    header.size = sizeof(NameCountData);

    //combine header_payload
    char payload[sizeof(header) + sizeof(NameCountData)];
    memcpy(payload, &header, sizeof(header));
    memcpy(payload + sizeof(header), data, sizeof(NameCountData));

    write(fd, payload, sizeof(payload)); // Write the header+payload
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

/**
 * This function outputs the names and number of occurrences in a readable format to a PID.out file.
 * It also creates a NameCountData struct and calls write_struct_namecount
 * Returns: nothing
**/
void outputPIDs(int fd, char * pid) {
    char filename[32];
    snprintf(filename, sizeof(filename), "%s.out", pid);

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr,"error: cannot open file %s\n", filename);
        exit(1);
    }

    for(int i = 0; i < nameCount; i++) {
        struct nlist *np = lookup(nameList[i]);
        for(int j = 0; j < np->nCount; j++) {
            if(strcmp(np->names[j], nameList[i]) == 0)
            {
                NameCountData d = {0};
                strncpy(d.name, nameList[i], sizeof(d.name) - 1);
                d.count = np->counts[j];
                if (fd >= 0) {
                    write_struct_namecount(fd, &d);
                }
                fprintf(fp,"%s: %d\n", nameList[i], np->counts[j]);
                break;
            }
        }
    }

    fclose(fp);
}

int main(int argc, char *argv[]) {
    FILE *fp = NULL;

    char pidStr[16];
    char *pid;
    char *inputName;

    /*
     * countnames run directly with no filename:
     * read from stdin and use this process's PID for output files.
     */
    if (argc == 1) {
        snprintf(pidStr, sizeof(pidStr), "%d", getpid());
        pid = pidStr;
        inputName = "stdin";
        fp = stdin;
    }

    /*
     * Optional direct-file mode:
     * ./countnames names.txt
     */
    else if (argc == 2) {
        snprintf(pidStr, sizeof(pidStr), "%d", getpid());
        pid = pidStr;
        inputName = argv[1];

        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            fprintf(stderr, "error: cannot open file %s\n", argv[1]);
            exit(1);
        }
    }

    /*
     * Normal Assignment 2 mode:
     * shell calls:
     *./countnames PID filename
     * "1" is used by shell.c to indicate stdin.
     */
    else if (argc == 4) {
        pid = argv[1];

        if (strcmp(argv[2], "1") == 0) {
            fp = stdin;
            inputName = "stdin";
        }
        else {
            inputName = argv[2];
            fp = fopen(argv[2], "r");
            if (fp == NULL) {
                fprintf(stderr, "error: cannot open file %s\n", argv[2]);
                exit(1);
            }
        }
    }
    else {
        fprintf(stderr, "error: invalid arguments\n");
        return 1;
    }

    char buffer[32];
    int lineNum = 1;

    char errFile[32];
    snprintf(errFile, sizeof(errFile), "%s.err", pid);
    FILE *ep = fopen(errFile, "w");
    if (ep == NULL) {
        fprintf(stderr, "error: cannot open created error file.\n");

        if (fp != stdin) {
            fclose(fp);
        }
        exit(1);
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        buffer[strcspn(buffer, "\r\n")] = '\0';

        if (strlen(buffer) == 0) {
            fprintf(
                ep,
                "Warning - file %s line %d is empty.\n",
                inputName,
                lineNum
            );
        }
        else {
            char *trueLine = strdup(buffer);
            insert(trueLine);
        }

        lineNum++;
    }

    int sfd = (argc == 4) ? atoi(argv[3]) : -1;
    outputPIDs(sfd, pid);     //argv[3] has the file discriptors
    if(sfd >= 0) {
        close(sfd);
    }

    if (fp != stdin) {
        fclose(fp);
    }
    fclose(ep);

    return 0;
}
