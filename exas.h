/* getopt(3) stuff */
extern char *optarg;
extern int optind, opterr, optopt;

typedef enum { false, true } bool;

typedef struct {
    const bool permit;
    const char *user;
    const char *group;
    const char *tuser;
    const char *command;
    const char **arguments;
} Rule;

void usage(void);
