#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hmap.h"



char *dict_file = "dictionary-small.txt";
// char *dict_file = "dictionary-big.txt";

int short_words = 0;

// struct for an entry in the DP table
typedef struct {
  int feasible;  // T/F
  int nsolns;    // number of distinct legal partitions
  int prev;
} SOLN;

/*
* builds hash-table storing dictionary (read from file).
*/
HMAP_PTR build_dict(){
  HMAP_PTR dict = hmap_create(0,1.0);
  FILE *fptr;
  char word[128];

  fptr = fopen(dict_file, "r");
  if(fptr == NULL) {
    fprintf(stderr, "ERROR:  failed to open dictionary file '%s'\n",
        dict_file);
    return NULL;
  }
  // exclude any 1-letter "words" besides "a" and "I"
  hmap_set(dict, "a", NULL);
  hmap_set(dict, "I", NULL);
  hmap_set(dict, "A", NULL);
  hmap_set(dict, "i", NULL);
  while(fscanf(fptr, "%s", word) == 1) {
    if(strlen(word) > 1)
      hmap_set(dict, word, NULL);
  }
  fclose(fptr);
  return dict;
}

/*
* partition constructs the DP table and returns it
*/
SOLN *partition(char *s, HMAP_PTR dict) {
int i, j;
int n=strlen(s);
SOLN *opt = malloc((n+1)*sizeof(SOLN));
char word[n+1];

  // opt[i].feasible = true iff the string
  //   s[0]...s[i-1] is legal / partitionable
  // base case:  opt[0] corresponds to the empty
  //   string which is always feasible.
  opt[0].feasible = 1;
  opt[0].nsolns = 1;
  opt[0].prev= -1;

  for(i=1; i<=n; i++){
    opt[i].feasible = 0;
    opt[i].nsolns = 0;
    opt[i].prev = -1;
    for(j=i-1; j >= 0; j--){
      if(opt[j].feasible){  // prefix must be partitionable
        strncpy(word, s+j, i-j);  // extract candidate last word
        word[i-j] = '\0';
        // printf("lookup attempt on '%s'\n", word);
        if(hmap_contains(dict, word)){
          if(short_words && !opt[i].feasible)      // first solution found
            opt[i].prev = j;  // prefix ends at j-1
          else if(!short_words)
            opt[i].prev = j;  // prefix ends at j-1
          opt[i].feasible = 1;
          opt[i].nsolns += opt[j].nsolns;
        }
      }
    }
  }
  return opt;
}
	    
/**
* This is a table-dump function
*/
void print_solns(SOLN *opt, char *s) {
int n=strlen(s);
int i;

  printf("----- TABLE -----\n");
  for(i=0; i<=n; i++) {
	printf("%2i '%c' feas=%i j=%i nsolns=%i\n", i, s[i], 
		opt[i].feasible, opt[i].prev, opt[i].nsolns);
  }
  printf("----- END TABLE -----\n");
}

/*
* This is the 'traceback' function for extracting and printing
*   actual feasible sentence
*/
void extract(SOLN *opt, int idx, char *s) {
  char word[idx+1];
  int j;

  if(idx == 0)
    return;
  j = opt[idx].prev;

  extract(opt, j, s);
  strncpy(word, s+j, idx-j);
  word[idx-j] = '\0';

  if(j != 0)
    printf(" ");
  printf("%s", word);
}
  

int main(int argc, char *argv[]){
  HMAP_PTR dict;
  char *tests[] = {"appl", "applecart"};
  SOLN *opt;
  int i, n;
  int ntests = sizeof(tests)/sizeof(char *);
  int verbose = 0;
  char buff[128];

  for(i=1; i<argc; i++) {
    if(strcmp(argv[i], "-v") == 0)
      verbose = 1;
    else if (strcmp(argv[i], "-b") == 0)
      dict_file = "dictionary-big.txt";
    else if (strcmp(argv[i], "-short") == 0)
      short_words = 1;
    else
      printf("unknown flag '%s' ignored\n", argv[i]);

  }
  dict = build_dict();

  if(dict == NULL)
    return 0;

  while(fgets(buff, 128, stdin) != NULL) {
    n = strlen(buff);
    buff[n-1] = '\0';
    n--;
    printf("input: '%s'\n", buff);
    opt = partition(buff,  dict);
    if(opt[n].feasible) {
      printf("  %d viable partitions\n", opt[n].nsolns);
      printf("  Legal sentence: '");
      extract(opt, n, buff);
      printf("'\n");
    }
    else {
      printf("string '%s' is illegal\n", buff);
    }
    if(verbose) {
      printf("VERBOSE TABLE DUMP\n");
      print_solns(opt, buff);
    }
    free(opt);
    printf("\n");
  }
  hmap_free(dict, 0);
}
