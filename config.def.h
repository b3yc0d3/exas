/* See LICENSE file for copyright and license details. */

/* rules */
static const Rule rules[] = {
    /* permit   user    group      target user   command     arguments (null terminated) */
    {true, NULL, "wheel", "root", NULL, NULL},
};
