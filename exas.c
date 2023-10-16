/* exas.c, v 1.0 2023/10/15 */
/*
 * Exas entry fille
 * 
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

#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <shadow.h>
#include <termios.h>
#include <wait.h>
#include <errno.h>

#include "exas.h"
#include "config.h"

void usage(void)
{
    fprintf(stderr, "Usage: exas [-u user] command [args ...]\n");
    exit(1);
}

void exec_cmd(struct passwd usertgt, const char *command, int paramc, char **params)
{
    pid_t fpid = fork();
    if (fpid == 0)
    {
        setuid(usertgt.pw_uid);
        execvp(command, params);
        exit(1);
    }
    else
    {
        waitpid(fpid, 0, 0);
    }
}

bool_t inarray(const char **array1, const char **array2)
{
    size_t narray1 = sizeof(array1) / sizeof(array1[0]);
    size_t narray2 = sizeof(array2) / sizeof(array2[0]);

    for (int i = 0; i < narray2; ++i)
    {
        bool_t found = false;
        for (int j = 0; j < narray1; ++j)
        {
            if (array2[i] == array1[j])
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            return false;
        }
    }
    return true;
}

bool_t hasgroup(const char *gname, int ngroups, uid_t *groups)
{
    for (int i = 0; i < ngroups; ++i)
    {
        struct group *gr = getgrgid(groups[i]);
        if (strcmp(gr->gr_name, gname) == 0)
            return true;
    }

    return false;
}

bool_t check_password(struct passwd user, const char *password)
{
    struct passwd *pwdent = getpwnam(user.pw_name);
    struct spwd *spwdent = getspnam(user.pw_name);

    if (pwdent == NULL)
    {
        fprintf(stderr, "Error: User '%s' does not exist.\n", user.pw_name);
        return false;
    }

    if (pwdent == NULL)
    {
        fprintf(stderr, "Error: Failed to compare password for '%s'.\n", user.pw_name);
        return false;
    }

    char *cryptic = crypt(password, spwdent->sp_pwdp);
    return strcmp(spwdent->sp_pwdp, cryptic) == 0;
}

bool_t user_auth(struct passwd caller, struct passwd target, const char *cmd, size_t parmc, const char **params)
{
    bool_t success = false;
    int ngroups = 0;
    // Just there to get the "group count"
    getgrouplist(caller.pw_name, caller.pw_gid, NULL, &ngroups);
    gid_t callergrs[ngroups];
    getgrouplist(caller.pw_name, caller.pw_gid, callergrs, &ngroups);

    size_t nrules = sizeof(rules) / sizeof(rules[0]);

    for (size_t i = nrules; i > 0; --i)
    {
        const Rule currul = rules[i - 1];
        bool_t matchuser = (currul.user == NULL || strcmp(caller.pw_name, currul.user) == 0);
        bool_t matchgrp = (currul.group == NULL || hasgroup(currul.group, ngroups, callergrs));
        bool_t matchtgt = (currul.tuser == NULL || strcmp(target.pw_name, currul.tuser) == 0);
        bool_t matchcmd = (currul.command == NULL || strcmp(cmd, currul.command) == 0);
        bool_t matchargs = (currul.arguments == NULL || inarray(params, currul.arguments));

        if (matchuser && matchgrp && matchtgt && matchcmd && matchargs)
            success = true;
    }

    if (success)
    {
        char *userpwd = prompt_password(caller);
        if (check_password(caller, userpwd))
        {
            userpwd = realloc(userpwd, sizeof(userpwd));
            return true;
        }
    }

    return false;
}

char *prompt_password(struct passwd userclr)
{
    char *text = NULL;
    size_t len = 0;
    ssize_t read = 0;
    struct termios oldTerm, newTerm;

    fprintf(stderr, "password for %s: ", userclr.pw_name);

    if (tcgetattr(STDIN_FILENO, &oldTerm) != 0)
        return NULL;

    newTerm = oldTerm;
    newTerm.c_lflag &= ~ECHO;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &newTerm) != 0)
        return NULL;

    read = getline(&text, &len, stdin);

    // remove line break
    if (text[read - 1] == '\n')
        text[read - 1] = '\0';

    (void)tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldTerm);
    putc('\n', stderr);

    return text;
}

int main(int argc, char **argv)
{
    opterr = 0; // disable getopts error message

    int opt;
    int paramc = 0;
    char *usertgtname = NULL;
    char **params = NULL;
    char *command = NULL;
    struct passwd usertgt; // user to execute process as
    struct passwd usercl;  // user that executed exas (aka. this program)
    struct passwd *result;
    char *bufusertgt = NULL;
    char *bufusercl = NULL;
    long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);

    if ((bufsize = sysconf(_SC_GETPW_R_SIZE_MAX)) == -1)
        bufsize = 16384; // should be enough as fallback (at least)

    if ((bufusertgt = malloc(bufsize)) == NULL)
    {
        fprintf(stderr, "Error: malloc l%d\n", __LINE__);
        exit(1);
    }

    if ((bufusercl = malloc(bufsize)) == NULL)
    {
        fprintf(stderr, "Error: malloc l%d\n", __LINE__);
        exit(1);
    }

    // Get real caller uid
    getpwuid_r(getuid(), &usercl, bufusercl, bufsize, &result);

    while ((opt = getopt(argc, argv, "+u:")) != -1)
    {
        switch (opt)
        {
        case 'u':
            usertgtname = strdup(optarg);
            break;

        default:
            usage();
            break;
        }
    }

    if (optind >= argc)
        usage();

    if (optind < argc)
    {
        for (paramc = 0; optind + paramc < argc; ++paramc)
            ;

        params = malloc((paramc + 1) * sizeof(char *));

        for (size_t i = 0; i < paramc; ++i)
            params[i] = strdup(argv[optind + i]);

        params[paramc] = NULL;
        /* point "command" => params[0] */
        command = params[0];
    }

    if (usertgtname == NULL)
        getpwuid_r(geteuid(), &usertgt, bufusertgt, bufsize, &result);
    else
        getpwnam_r(usertgtname, &usertgt, bufusertgt, bufsize, &result);

    if (!user_auth(usercl, usertgt, command, paramc, (const char **)params))
    {
        fprintf(stderr, "Operation not permitted.\n");
        exit(1);
    }

    exec_cmd(usertgt, command, paramc, params);
    return 0;
}
