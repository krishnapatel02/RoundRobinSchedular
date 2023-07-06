#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    long long i;
    int minutes = 0, j;
    int seconds = 0;
    char name[128];
    int opt;

/*
 * process environment variable and command line arguments
 */
    opterr = 0;
    while ((opt = getopt(argc, argv, "m:s:n:")) != -1) {
        switch(opt) {
        case 'm': minutes = atoi(optarg); break;
        case 's': seconds = atoi(optarg); break;
        case 'n': strcpy(name, optarg); break;
	default:
            fprintf(stderr, "Illegal flag: `-%c'\n", optopt);
            return EXIT_FAILURE;
        }
    }
    seconds += 60 * minutes;
    for (j = 0; j < seconds; j++) {
        for (i = 0; i < 666666667; i++) {
            ;
        }
    }
    return EXIT_SUCCESS;
}
