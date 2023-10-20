/* See LICENSE file for copyright and license details. */

/* locations where to search for executables */
static const char *secure_path = "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin";

/* rules */
static const Rule rules[] = {
    /* permit   user    group      target user   command     arguments (null terminated) */
    { true, NULL, "wheel", "root", NULL, NULL }
};
