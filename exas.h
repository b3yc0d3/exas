/* exas.h, v 1.0.1 2023/10/28 */
/*  
 * Copyright (c) 2023 Niklas Kellerer <b3yc0d3@gmail.com>
 *   
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* getopt(3) stuff */
extern char *optarg;
extern int optind, opterr, optopt;

extern char **environ;

static const char *safepath = "/bin:/sbin:/usr/bin:/usr/sbin:"
                              "/usr/local/bin:/usr/local/sbin";

typedef enum { false, true } bool_t;

typedef struct {
    const bool_t permit;
    const char *user;
    const char *group;
    const char *tuser;
    const char *command;
    const char **arguments;
} Rule;

void usage(void);
void makeenv(const char **keeplist, char ***new_env);
void execcmd(struct passwd caller, struct passwd usertgt, const char *command, int paramc, char **params);
char *pwdprompt(struct passwd userclr);
bool_t hasgroup(const char *gname, int ngroups, uid_t *groups);
bool_t hasparam(int arr1c, const char **arr1, int arr2c, const char **arr2);
bool_t permit(struct passwd caller, struct passwd target, const char *cmd, size_t paramc, const char **params);
bool_t passwdcheck(struct passwd user, const char *password);
