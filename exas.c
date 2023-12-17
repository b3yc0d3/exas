/* exas.c, v 1.0.1 2023/10/28 */
/*
 * Exas entry fille
 *
 * Copyright (c) 2023 Niklas Kellerer <b3yc0d3@gmail.com>
 *                    NikOverflow <niklasst2102@gmail.com>
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

void makeenv(const char **keeplist, char ***new_env)
{
    int new_env_size = 1;
    int index = 0;
    (*new_env) = malloc(new_env_size * sizeof(char *));
    int i, j;

    for (i = 0; environ[i] != NULL; ++i)
    {
        char *envname = strndup(environ[i], (strchr(environ[i], '=') - environ[i]));

        for (j = 0; keeplist[j] != NULL; ++j)
        {

            if (strcmp(envname, keeplist[j]) == 0)
            {
                ++new_env_size;
                (*new_env) = realloc((*new_env), new_env_size * sizeof(char *));
                (*new_env)[index] = strdup(environ[i]);

                ++index;
                break;
            }
        }

        free(envname);
    }

    (*new_env)[index] = NULL;
}

void execcmd(struct passwd caller, struct passwd usertgt, const char *command, int paramc, char **params)
{
    pid_t fpid = fork();
    if (fpid == 0)
    {
        /**
         * Fixes issue #2 from JorianWoltjer
         * Privilege escalation via $PATH
         *
         * $PATH gets overwritten with the value of ``safepath``
         * which is defined in the config.h
         */
        setenv("PATH", safepath, 1);

        setuid(usertgt.pw_uid);
        setgid(usertgt.pw_gid);

        int ngroups = 0;
        getgrouplist(usertgt.pw_name, usertgt.pw_gid, NULL, &ngroups);
        gid_t groups[ngroups];
        getgrouplist(usertgt.pw_name, usertgt.pw_gid, groups, &ngroups);
        setgroups(ngroups, groups);

        /* setting up new enviorment */
        char **new_env;
        const char *keep_env[4] = {"DISPLAY", "COLORTERM", "TERM", NULL};
        makeenv(keep_env, &new_env);
        environ = new_env;

        /* Set environment variables */
        setenv("HOME", usertgt.pw_dir, 1);
        setenv("LOGNAME", usertgt.pw_name, 1);
        setenv("PATH", safepath, 1);
        setenv("SHELL", usertgt.pw_shell, 1);
        setenv("USER", usertgt.pw_name, 1);
        setenv("EXAS_USER", caller.pw_name, 1);

        if (execvp(command, params) == -1)
            fprintf(stderr, "%s\n", strerror(errno));

        exit(1);
    }
    else
    {
        waitpid(fpid, 0, 0);
    }
}

bool_t hasparam(int a1c, const char *a1[], int a2c, const char *a2[])
{
    int matches = 0;
    int i = 0;
    int j = 1;

    for (; i < a2c; ++i)
    {
        for (; j < a1c; ++j)
            if (strcmp(a2[i], a1[j]) == 0)
                ++matches;
    }

    return matches > 0;
}

bool_t hasgroup(const char *gname, int ngroups, gid_t *groups)
{
    struct group *tgroup = NULL;

    if ((tgroup = getgrnam(gname)) == NULL)
    {
        fprintf(stderr, "Error: Unknown group of '%s'\n", gname);
        return false;
    }

    for (int i = 0; i < ngroups; ++i)
    {
        if (groups[i] == tgroup->gr_gid)
            return true;
    }

    return false;
}

bool_t passwdcheck(struct passwd user, const char *password)
{
    struct passwd *pwdent = getpwnam(user.pw_name);
    struct spwd *spwdent = getspnam(user.pw_name);

    if (pwdent == NULL)
    {
        fprintf(stderr, "Error: User '%s' does not exist.\n", user.pw_name);
        return false;
    }

    if (spwdent == NULL)
    {
        fprintf(stderr, "Error: Failed to compare password for '%s'.\n", user.pw_name);
        return false;
    }

    char *cryptic = crypt(password, spwdent->sp_pwdp);
    if (cryptic == NULL)
        return 0;
    return strcmp(spwdent->sp_pwdp, cryptic) == 0;
}

bool_t permit(struct passwd caller, struct passwd target, const char *cmd, size_t paramc, const char **params)
{
    int ngroups = 0;
    /* just there to get the group count */
    getgrouplist(caller.pw_name, caller.pw_gid, NULL, &ngroups);
    gid_t callergrs[ngroups];
    getgrouplist(caller.pw_name, caller.pw_gid, callergrs, &ngroups);

    size_t nrules = sizeof(rules) / sizeof(rules[0]);

    for (size_t i = nrules; i > 0; --i)
    {
        const Rule currul = rules[i - 1];

        int argc = 0;
        if (currul.arguments != NULL)
        {
            for (; currul.arguments[argc] != NULL; ++argc)
                ;
        }

        bool_t matchuser = (currul.user == NULL || strcmp(caller.pw_name, currul.user) == 0);
        bool_t matchgrp = (currul.group == NULL || hasgroup(currul.group, ngroups, callergrs));
        bool_t matchtgt = (currul.tuser == NULL || strcmp(target.pw_name, currul.tuser) == 0);
        bool_t matchcmd = (currul.command == NULL || strcmp(cmd, currul.command) == 0);
        bool_t matchargs = (currul.arguments == NULL || hasparam(paramc, params, argc, currul.arguments));

        if (matchuser && matchgrp && matchtgt && matchcmd && matchargs)
        {
            if (currul.permit == true)
            {
                /*
                 * Give the user three attempts for password inputs
                 * if incorrect
                 */
                int c = 0;
                for (; c < 3; ++c)
                {
                    char *userpwd = pwdprompt(caller);
                    if (passwdcheck(caller, userpwd))
                    {
                        userpwd = realloc(userpwd, sizeof(userpwd));
                        return true;
                    }
                    else
                    {
                        fprintf(stderr, "Sorry, try again\n");
                    }
                }

                fprintf(stderr, "3 incorrect password attempts\n");
            }

            return false;
        }
    }

    return false;
}

char *pwdprompt(struct passwd userclr)
{
    char *text = NULL;
    size_t len = 0;
    ssize_t read = 0;
    struct termios oldterm, newterm;

    fprintf(stderr, "password for %s: ", userclr.pw_name);

    if (tcgetattr(STDIN_FILENO, &oldterm) != 0)
        return NULL;

    newterm = oldterm;
    newterm.c_lflag &= ~ECHO;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &newterm) != 0)
        return NULL;

    read = getline(&text, &len, stdin);

    /* remove line break */
    if (text[read - 1] == '\n')
        text[read - 1] = '\0';

    (void)tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldterm);
    putc('\n', stderr);

    return text;
}

int main(int argc, char **argv)
{
    /* disable getopts(3) error message */
    opterr = 0;

    int opt;
    int paramc = 0;
    char *usertgtname = NULL;
    char **params = NULL;
    char *command = NULL;
    struct passwd usertgt;    /* user to execute process as */
    struct passwd usercaller; /* user that executed exas (aka. this program) */
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

    /* get real caller uid */
    getpwuid_r(getuid(), &usercaller, bufusercl, bufsize, &result);

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

    if (!permit(usercaller, usertgt, command, paramc, (const char **)params))
    {
        fprintf(stderr, "Operation not permitted.\n");
        exit(1);
    }

    execcmd(usercaller, usertgt, command, paramc, params);
    return 0;
}
