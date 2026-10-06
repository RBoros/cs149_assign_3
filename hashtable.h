#ifndef HASHTABLE_H
#define HASHTABLE_H

#define HASHSIZE 101    // number of buckets in the hash table
#define MAXNAMES 1000   // max number of unique names that can be stored

/**
 * Table entry:
 * Each struct has a table of names if same hash value is used.
 **/
struct nlist {
    int nCount;     // number of names in struct.
    int nCapacity;  // used for when to increase allocated memory.
    int *counts;    // array of counts for each name in array of names.
    char **names;   // array of names.
};

// Defined in hashtable.c; shared by every file that includes this header.
extern char *nameList[MAXNAMES];   // list of unique names (in first-seen order).
extern int nameCount;              // number of unique names.

unsigned hash(char *temp);
struct nlist *lookup(char *s);
void insert(char *name);                 // add one occurrence of name
void insertCount(char *name, int n);    // add n occurrences of name
void clearTable();

#endif
