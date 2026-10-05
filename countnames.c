/**
 * Description: This program counts how many times each individual name appears across one or more files.
 * Author names: Ebsan Iqbal, Raymond Okolo
 * Author emails: ebsan.iqbal@sjsu.edu, raymond.okolo@sjsu.edu
 * Last modified date: 9/23/2026
 * Creation date: 9/2/2026
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define HASHSIZE 101

/**
 * Table entry:
 * Each struct has a table of names if same hash value is used.
 **/
struct nlist{
    int nCount;     // number of names in struct.
    int nCapacity;  // used for when to increase allocated memory.
    int* counts;    // array of counts for each name in array of names.
    char** names;   // array of names.

};

static struct nlist *hashtab[HASHSIZE];     // pointer table
char* nameList[101];                        // list of unique names.
int nameCount = 0;                          // number of unique names.

/**
 * This is the hash functionL form hash value for string temp
 * Assumption: temp is a char*
 * Input parameters: temp
 * Returns: a hash value
**/
unsigned hash(char *temp) {
    unsigned hashval;
    for (hashval = 0; *temp != '\0'; temp++) {
        hashval = *temp + 31 * hashval;
    }
    return hashval % HASHSIZE;
}

/**
 * This function performs a lookup of a name in hashtab
 * Assumption: s is a char*
 * Input parameters: s
 * Returns: a pointer to a struct or NULL if no struct exists.
**/
struct nlist *lookup(char *s) {
    struct nlist *np = hashtab[hash(s)];
    if (np != NULL) {
        return np;
    }
    return NULL; /* not found */
}

/**
 * This function creates and inserts a struct in hashtab or updates an existing struct.
 * Assumption: name is a char*
 * Input parameters: name
 * Returns: nothing
**/
void insert(char *name) {
    char* temp = name; // so name remains unaltered for hash()
    struct nlist *np = lookup(temp);
    const int hVal = hash(name);

    if (np == NULL) { //name is not in a struct yet.
        np = malloc(sizeof(*np));
        if (np == NULL) {
            fprintf(stderr, "Allocation failed\n");
            exit(1);
        }
        //initializing variables and allocating memory.
        np->nCount = 1;
        np->nCapacity = 4;
        np->names = malloc(np->nCapacity * sizeof(char*));
        np->names[0] = strdup(name);
        np->counts = malloc(np->nCapacity * sizeof(int));
        np->counts[0] = 1;
        hashtab[hVal] = np;

        nameList[nameCount++] = strdup(name); //insert struct in table.
    } else {
        int found = 0;
        //this loop iterates through a struct's names array
        for(int i = 0; i < np->nCount; i++){
            if(strcmp(np->names[i], name) == 0) { // name is found in struct name array
                found = 1;
                hashtab[hVal]->counts[i]++;     //increment count
                break;
            }
        }
        //name is not found but uses same has value,
        //so insert name in existing struct names array.
        //also allocate more memory if needed and increment nCounts
        if(found == 0) {
            if(hashtab[hVal]->nCount == hashtab[hVal]->nCapacity) {
                hashtab[hVal]->nCapacity *=2;
                hashtab[hVal]->names = realloc(hashtab[hVal]->names, hashtab[hVal]->nCapacity * sizeof(char*));
                hashtab[hVal]->counts = realloc(hashtab[hVal]->counts, hashtab[hVal]->nCapacity * sizeof(int));
            }
            hashtab[hVal]->names[hashtab[hVal]->nCount] = strdup(name);
            hashtab[hVal]->counts[hashtab[hVal]->nCount] = 1;
            hashtab[hVal]->nCount++;

            nameList[nameCount++] = strdup(name);
        }
    }
}


 //This is function prints the names and number of occurrences in a readable format.
 //Returns: nothing

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

 //This is function outputs the names and number of occurrences in a readable format to a PID.out file.
 //Returns: nothing
void outputPIDs(char * pid) {
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
            if(strcmp(np->names[j], nameList[i]) == 0) {
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


    //Optional direct-file mode:
    //./countnames names.txt
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


     //Normal Assignment 2 mode:
     //shell calls:
     //./countnames PID filename
     //"1" is used by shell.c to indicate stdin.

    else if (argc == 3) {
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

    outputPIDs(pid);

    if (fp != stdin) {
        fclose(fp);
    }

    fclose(ep);

    return 0;
}
