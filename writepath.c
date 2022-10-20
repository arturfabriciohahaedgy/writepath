#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <dirent.h>

#define ARGUMENT_SIZE     256
#define FILE_MAX_LINE     2048
#define FILE_MAX_PATH     256
#define CORRECT_WORD      0
#define WRONG_WORD        1
#define IN_NO_FLAG        0
#define IN_FLAG_VAR       1
#define IN_FLAG_FILE      2

void usage(void);
char *shiftargs(int*, char***);
int returnpos(FILE *, char *);

struct GenericArray {
    char  *array[ARGUMENT_SIZE];
    int    used;
}; /* generic array structure to keep track of used values*/

void
printarguments(struct GenericArray *arg)
{
    for (int i = 0; i < arg->used; i++)
	printf("array: %s\n", arg->array[i]);
}

char *
shiftargs(int *argc, char ***argv)
/* shameless copied from tsoding */
{
    char *result;
    assert(*argc > 0);
    result = **argv;
    *argv += 1;
    *argc -= 1;
    return result;
}

char *
returnpath(char *relativepath)
{
    char *buffer = 0;
    buffer = (char *)malloc(FILE_MAX_PATH * sizeof(buffer));
    #ifdef _WIN32
    if (_fullpath(buffer, relativepath, FILE_MAX_PATH) != NULL) {
	fprintf(stderr, "Could not open the target file at \"%s\".\n", relativepath);
	exit(1);
    }
    else 
	return buffer;
    #elif __unix__ || __APPLE__
    realpath(relativepath, buffer);
    return buffer;
    #endif
}

int
returnpos(FILE *targetfile, char *var)
/* get position of variable declaration int *targetfile */
{
    char buffer[FILE_MAX_LINE] = {0};
    char currentword[ARGUMENT_SIZE] = {0};
    int  beginword = 0; /* reseted to 0 in the for-loop everytime so the values inserted in currentword stay corrected*/
    int  state = WRONG_WORD; /* if the word matches or not the variable */
    int  pos = 0; /* used to count the amount of characters for fseek later */
    int  buflen;

    rewind(targetfile);
    while (fgets(buffer, FILE_MAX_LINE, targetfile) != NULL) {
	buflen = strlen(buffer);
	for (int i = 0; i < buflen; i++) {
	    if (state != CORRECT_WORD) {
		/* don't take newlines, spaces, tabs and pointer syntax (the "*" and "&" before the variables) when getting the current word*/
		if (buffer[i] != '\n' && buffer[i] != ' ' && buffer[i] != '\t' && buffer[i] != '*' && buffer[i] != '&') {
		    currentword[beginword] = buffer[i];
		    beginword++;
		} else {
		    if (strcmp(var, currentword) == 0)
			state = CORRECT_WORD;
		    memset(currentword, 0, sizeof(currentword));
		    beginword = 0;
		}
	    } else {
		if (buffer[i] == ',' || buffer[i] == ';') {
		    state = WRONG_WORD;
		    continue;
		} else if (buffer[i] == '"') {
		    pos++;
		    return pos;
		}
	    }
	    pos++;
	}
    }
    /* if after searching the file, still doesn't find the word, return 0. */
    return WRONG_WORD;
}

void
insertnewfile(FILE *target, char *targetpath, int pos, char *file)
/* TODO: Look up how to get the absolute path of a file in windows */
{
    opendir(targetpath);
    FILE *bkp;
    int  currentpos = 0;
    char buffer[FILE_MAX_LINE] = {0};
    int  buflen;

    rewind(target);
    bkp = fopen("backup.c", "w+");

    while (fgets(buffer, FILE_MAX_LINE, target) != NULL) {
	/* TODO: Finish the loop*/
    }
}

void
usage(void)
{
    fprintf(stdout, "\nUsage: writepath [FILE TO WRITE ON] [-v NAME OF VARIABLES INSIDE FILE] [-f PATH TO USED FILES]\n");    
    fprintf(stdout, "-v  [NAME OF VARIABLES]       Variables which are to be filled with the path to another file.\n");    
    fprintf(stdout, "-f  [PATH TO USED FILES]          Path to files which are to be extracted the absolute path, which are then in turn put inside the variables given in the first parameter.\n");    
    fprintf(stdout, "\nExample: writpath foo.c -v barpath bazpath -f ../bar.txt ../../baz.txt\n");    
}

int
main(int argc, char *argv[])
{
    char               *targetfile; /* file which is gonna be written in */
    FILE               *targetIO; 
    char               *param; /* args */
    int                 state; /* if argument is on -v flag or -f flag */
    int                 pos; /* position of variable declaration */
    struct GenericArray vars;
    struct GenericArray files;

    memset(files.array, 0, sizeof(files.array));
    memset(vars.array, 0, sizeof(vars.array));
    state = IN_NO_FLAG;
    vars.used = 0;
    files.used = 0;
    targetfile = argv[1];

    if (argc < 6) {
	usage();
	exit(1);
    }

    while (argc > 0) {
	param = shiftargs(&argc, &argv);
	if ((strcmp(param, "-f")) == 0) {
	    if (state == IN_FLAG_VAR) {
		state = IN_FLAG_FILE;
		continue;
	    }
	    state = IN_FLAG_FILE;
	    if (argc <= 0) {
		usage();
		exit(1);
	    }
	}
	if (state == IN_FLAG_VAR) {
	    vars.array[vars.used] = param;
	    vars.used++;
	}
	if (state == IN_FLAG_FILE) {
	    param = returnpath(param);
	    files.array[files.used] = param;
	    files.used++;
	}
	if ((strcmp(param, "-v")) == 0) {
	    state = IN_FLAG_VAR;
	    if (argc <= 0) {
		usage();
		exit(1);
	    }
	}
    }

    if (vars.used != files.used) {
	usage();
	exit(1);
    }

    printf("Target file: %s\n", targetfile);
    targetIO = fopen(targetfile, "r+");
    if (targetIO == NULL) {
	fprintf(stderr, "ERROR: Could not open \"%s\": %s", targetfile, strerror(errno));
	fclose(targetIO);
	exit(1);
    } else {
	for (int i = 0; i < vars.used; i++) {
            pos = returnpos(targetIO, vars.array[i]);
	    if (pos != WRONG_WORD) {
	    } else {
		fprintf(stderr, "ERROR: It wasn't possibile to find the variable \"%s\" inside file \"%s\".\n", vars.array[i], targetfile);
		fclose(targetIO);
		exit(1);
	    }
        }
	fclose(targetIO);
    }

    printf("vars: \n");
    printarguments(&vars);
    printf("files: \n");
    printarguments(&files);

    for (int i = 0; i < files.used; i++) {
	free(files.array[i]);
    }

    return 0;
}
