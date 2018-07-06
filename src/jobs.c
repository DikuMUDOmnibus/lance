/*
 * This is the file that contains the necessary information for the jobs
 * functions on the mud. Any changes required outside of this file will
 * be listed here. If there's no list, then I'm doing a great job =)
 * (C) 2002 Legends of the Lance. All rights reserved.
 *
 * I've decided that in the interest of "modularity", I will keep the functions
 * required to the appropriate files. Hopefully this will keep things neat
 * and tidy.
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "jobs.h"

#define JOBS_FILE "jobs.txt"

/* Lists the jobs available to the players */
void do_jobs(CHAR_DATA *ch, char *argument)
{
    char arg_cmd[MAX_INPUT_LENGTH];
    argument = one_argument(argument, arg_cmd);
    // add
    if(!str_cmp(arg_cmd, "add"))
    {
        
    }
    // remove
    if(!str_cmp(arg_cmd, "remove"))
    // apply
    if(!str_prefix(arg_cmd, "apply"))
    // drop
    if(!str_prefix(arg_cmd, "drop"))
    ;
}