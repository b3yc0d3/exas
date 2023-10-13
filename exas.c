#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "exas.h"
#include "config.h"

void usage(void)
{
    fprintf(stderr, "Usage: exas [-u user] command [args ...]\n");
    exit(1);
}

int main(int argc, char **argv)
{
    /* disable getopts default error message */
    opterr = 0;

    char *usertgt = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "+u:")) != -1)
    {
        switch (opt)
        {
        case 'u':
            usertgt = strdup(optarg);
            break;

        default:
            usage();
            break;
        }
    }

    if (optind >= argc)
        usage();

    printf("Target User: %s\n", usertgt);

    return 0;
}
