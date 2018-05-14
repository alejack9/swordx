#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "trie.h"
#include "swordx.h"

static int cmpstringp ( const void* , const void* );
int isInArray ( char** , long , char* );

char* _getWord(FILE *pf) {
	char c, buf[500]; // c is the character, buf is the word
	int pos = 0; // index
	
	while(!isalpha(c = getc(pf)) && c != EOF); // remove all not-alphabetic character before a string
	ungetc(c,pf); // put the last char back

	while(isalpha((c = getc(pf)))) {
		buf[ pos++ ] = tolower(c);
		if(pos == 500) {
			fprintf(stderr,"Word is too long\n");
			exit(EXIT_FAILURE);
		}
	}
	if(c == EOF) return NULL;
	char *word = (char*)malloc((pos+1)*sizeof(char));
	strncpy(word,buf,pos);
	word[pos]='\0';
	return word;
}

char* getWord(FILE *pf, char *blacklist[], int blc) {
	char *ret = NULL;
	do {
		if(ret != NULL) free(ret);
		ret = _getWord(pf);
	} while(ret != NULL && isInArray(blacklist,blc,ret));
	return ret;
}

FILE* open_file(char* path) {
	FILE* pf = fopen(path,"rb");
	if(pf == NULL) {
		perror("Error reading");
		exit(EXIT_FAILURE);
	}
	return pf;	
}

FILE* makeFile() {
	FILE *pf = fopen("result.txt","wb");
	if(pf == NULL) {
		perror("Error writing");
		exit(EXIT_FAILURE);
	}
	return pf;
}

void writeWord(Trie *t,FILE *pf) {
	fprintf(pf,"%s: %d\r\n",t->value,t->occurrencies);
}

void visitTree(Trie *t, FILE *pf) {
	if(t == NULL) return;
	if(t->occurrencies > 0) // if the word occurrs at least one time...
		writeWord(t,pf); // ... write the word in the file
	for(int i = 0; i < 26; i++)
		visitTree(t->children[i],pf); // check each children of the node
}

void execute(char **files,int nfiles, char **blacklist, int blc, char **folders, int nfolders) {
	
	Trie *t = createTree();
	if(blc > 1) qsort(blacklist, blc, sizeof(char*),cmpstringp);
			
	char *str;
	for(int i=0;i<nfiles;i++) {
		FILE *pfread = open_file(files[i]);	
		while((str = getWord(pfread,blacklist,blc)) != NULL)
			add(str,t); // add the word to the trie (starting from the 1st level)
		fclose(pfread);		
	}

	FILE *pfwrite = makeFile();
	visitTree(t,pfwrite); // write trie status
	fclose(pfwrite);
}

int main(int argc, char *argv[]) {
	int opt = 0;
	char **files = NULL,**folders = NULL,**blacklist = NULL;
	int nfiles = 0,nwords = 0,nfolders = 0;
	for(int i=1; i<argc; i++) {
		if( (strcmp(argv[i],"-f")==0 && (opt=FILEARG)) || (strcmp(argv[i],"--remove")==0 && (opt=REMARG)) || (strcmp(argv[i],"-r")==0 && (opt=RECARG))) {
			int j = i+1;
			int farg_index = j;
			int larg_index = j;
			int nargs = 0;
			char **args;
			while(j < argc && (strlen(argv[j]) == 1 || *argv[j] != '-')) {
				larg_index++;
				j++;
			}
			
			if(farg_index==larg_index) { //no files! error!
				fprintf(stderr,"No file(s) provided\n");
				exit(EXIT_FAILURE);
			}
			else {				
				int len = 0;
				nargs = larg_index - farg_index;
				args = (char**)malloc(nargs*sizeof(char*));
				for ( int k = 0; k < nargs; k++ ) {
					 len = strlen(argv[farg_index+k]);
   					 args[k] = (char*) malloc((len+1)*sizeof(char));
   					 strncpy(args[k],argv[farg_index+k],len);
   					 args[k][len] = '\0';
				}
			}
			
			switch(opt) {
				case FILEARG:	files = args;  nfiles = nargs; break;
				case REMARG:	blacklist = args;  nwords = nargs; break;
				case RECARG:	folders = args;  nfolders = nargs; break;
			}
			i = j-1;
			
		}
		else if((strcmp(argv[i],"-h")==0 && (opt=HELPARG)) ) {
			printf("Usage: ./swordx [-r] [--remove {blacklist}] -f {files}\n\t-r: recoursive mode\n\t--remove {blacklist}: filter words adding a blacklist\n\t\t(elements must be separated by spaces)\n");
			exit(EXIT_SUCCESS);
		}
		else {
			fprintf(stderr,"Unknown argument %s\nUse -h to help\n",argv[i]);
			exit(EXIT_FAILURE);
		}

	}
	execute(files,nfiles,blacklist,nwords,folders,nfolders);
	exit(EXIT_SUCCESS);
}

int isInArray(char *array[], long length, char *str) {
	char **key = &str;
	return bsearch(key, array, length, sizeof(array[0]), cmpstringp) != NULL;
}

static int cmpstringp(const void *p1, const void *p2) { // from "man qsort": the qsort method accepts void pointers only
	return (strcmp(*(char **)p1, *(char **)p2));
}