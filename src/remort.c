/* remort.c - Remort code for Lance
 * Inspired by Dribble's remort code
 * (c) 2002 JN
 */
 
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "recycle.h"

void do_remor(CHAR_DATA *ch, char *argument)
{
    send_to_char("If you want to REMORT, type out the whole command.\n\r", ch);
}

void do_remort(CHAR_DATA *ch, char *argument)
{
    bool ansi;
    char buf[MAX_STRING_LENGTH];
    char pfile[MAX_STRING_LENGTH];
    int i;
    CHAR_DATA new_ch;
    DESCRIPTOR_DATA *d;
 

    if(ch->desc == NULL && !IS_IMMORTAL(ch))
        return;

    if(ch->level < LEVEL_HERO)
    {
        send_to_char("You must be level 101 to remort.\n\r", ch);
        return;
    }

    if(IS_IMMORTAL(ch) || IS_NPC(ch))
    {
        send_to_char("You cannot remort.\n\r", ch);
        return;
    }

    if(argument[0] == '\0')
    {
        // assign a random number for remorting
        sprintf(buf, "%d", number_range(1000, 9999));
        ch->short_descr = str_dup(buf);
        send_to_char("You have decided to remort. This command is {RIRREVERSIBLE{x!\n\r", ch);
        send_to_char("Type the following command to initiate the remort process.\n\r", ch);
        send_to_char("remort ", ch);
        send_to_char(buf, ch);
        send_to_char("\n\r", ch);
        return;
    }

    if(str_cmp(ch->short_descr, argument))
    {
        send_to_char("The number you entered does not match the one you were\n\r", ch);
        send_to_char("assigned. Please try again. If you were not given a number\n\r", ch);
        send_to_char("or you have lost it, type \"remort\" to receive one.\n\r", ch);
        return;
    }

    // Passed safety check
    sprintf(pfile, "%s%s", PLAYER_DIR, capitalize(ch->name));

    d = ch->desc;
    ansi = d->ansi;
/*
    // Save some old character info.
    new_ch.pcdata = new_pcdata();
    new_ch.name = str_dup(ch->name);
    new_ch.pcdata->pwd = str_dup(ch->pcdata->pwd);
    new_ch.played = ch->played;
    new_ch.pcdata->true_sex = ch->pcdata->true_sex;
    new_ch.race = ch->race;
    new_ch.class = ch->class;
    new_ch.pcdata->remorts = ch->pcdata->remorts;
    new_ch.clan = ch->clan;
    new_ch.clanrank = ch->clanrank;

    extract_char(ch, TRUE);
    unlink(pfile);
    load_char_obj(d, new_ch.name);

    d->character->name = str_dup(new_ch.name);
    free_string(new_ch.name);
    d->character->pcdata->pwd = str_dup(new_ch.pcdata->pwd);
    free_string(new_ch.pcdata->pwd);
    d->character->clan = new_ch.clan;
    d->character->clanrank = new_ch.clanrank;
    d->character->level = 15;
    d->character->played = new_ch.played;
    d->character->pcdata->true_sex = new_ch.pcdata->true_sex;
    d->character->race = new_ch.race;
    d->character->class = new_ch.class;
    d->character->pcdata->remorts = ++new_ch.pcdata->remorts;
    d->ansi = ansi;
*/

        d->character->level = 15;
	d->connected = CON_GET_NEW_CLASS;
    write_to_buffer(d,"The following classes are available:\n\r",0);
    send_to_desc("{D+---------+--------------------------------------------------+{x\n\r",d);
    send_to_desc("{D| {cClass{D   |               {xBrief Description{D              |\n\r{x",d);
    send_to_desc("{D+---------+--------------------------------------------------+{x\n\r",d);


    for(i = 0; i < MAX_CLASS; i++)
    {
        if(pc_race_table[d->character->race].bit & class_table[i].race_restrict)
            continue;

        if(d->character->pcdata->remorts <= class_table[i].remort
        && class_table[d->character->class].bit & class_table[i].previous_class)
            continue;

        sprintf( buf, "{D| {c%-7s{D | {x%-48s{D |{x\n\r",
        capitalize(class_table[i].name),
        class_table[i].desc );

        send_to_desc(buf, d );

    }
    send_to_desc("{D+---------+--------------------------------------------------+{x\n\r\n\r",d);
    write_to_buffer(d, "What is your class: ", 0);
    // Processing goes to nanny() in comm.c
}
