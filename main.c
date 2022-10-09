#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#define FILE_CONTENTS_SIZE 5126
#define ARGUMENT_SIZE 256
#define IN_FLAG_VAR   0
#define IN_FLAG_FILE  1

void usage(void);
char *shiftargs(int*, char***);

typedef struct {
    char  *array[ARGUMENT_SIZE];
    int    used;
} GenericArray; /* generic array structure to keep track of used values*/


void
printarguments(GenericArray *arg)
{
    int i;
    for (i = 0; i < arg->used; i++)
	printf("array: %s\n", arg->array[i]);
}

char *shiftargs(int *argc, char ***argv)
/* shameless copied from tsoding */
{
    char *result;
    assert(*argc > 0);
    result = **argv;
    *argv += 1;
    *argc -= 1;
    return result;
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
    FILE        *targetIO;
    char        *targetfile;
    char        filecontent[FILE_CONTENTS_SIZE];
    char        *param;
    int          state; /* if argument is on -v flag of -f flag */
    GenericArray vars;
    GenericArray files;

    memset(files.array, 0, sizeof(files.array));
    memset(vars.array, 0, sizeof(vars.array));
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
	    if (state == IN_FLAG_VAR)
		continue;
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

    /* TODO: Get the absolute path of files and files and re-store them in the files.array[] */

    printf("Target file: %s\n", targetfile);
    targetIO = fopen(targetfile, "r+");
    if (targetIO == NULL) {
	fprintf(stderr, "ERROR: Could not open \"%s\": %s", targetfile, strerror(errno));
	exit(1);
    } else {
	/* TODO: Filter output of string to match varialbes */
	/* TODO: Write the values inside arrays to the file*/
	fread(&filecontent, sizeof(int), sizeof(filecontent), targetIO);

	printf("Conteudo: %s\n", filecontent);

	fclose(targetIO);
    }

    printarguments(&vars);
    printarguments(&files);

    return 0;
}