/* Required functions
clancmd
chat (move here?)
clan_lookup
*/
/*
use flags for clans, allows dual-clans
req'd changes
merc.h
pcdata-> clan, subclan
long clan;
long clan_aux;
long subclan;
*/



void do_guild(CHAR_DATA *ch, char *argument)
{
}

long clan_lookup(const char* clan)
{
    for(int c = 0; c < MAX_CLAN; c++)
    {
        if(LOWER(clan[0]) == LOWER(clan_table[c].name[0])
        && !str_prefix(clan, clan_table[c].name))
        {
            return clan_table[c].bit;
        }
    }
    return 0;
}

void do_clancmd(CHAR_DATA *ch, char *argument)
{
    char cmd[MAX_INPUT_LENGTH];
    char player[MAX_INPUT_LENGTH];
    char sub[MAX_INPUT_LENGTH];
    CHAR_DATA *target;
    
    argument = one_argument(argument, cmd);
    argument = one_argument(argument, player);
    argument = one_argument(argument, sub); /* for subclan, other info too*/
    
    if((target = get_char_room(ch, player)) == NULL)
    {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf, "%s is nowhere to be seen.\n\r", player);
        send_to_char(buf, ch);
        return;
    }
    
    if(!str_prefix(cmd, "induct"))
        clando_induct(target, sub);
    else if(!str_prefix(cmd, "discharge"))
        clando_discharge(target, sub);
    else if(!str_prefix(cmd, "promote"))
        clando_promote(target);
    else if(!str_prefix(cmd, "demote"))
        clando_demote(target);
    else if(!str_prefix(cmd, "subclan"))
        clando_subclan(target, sub);
    else
        send_to_char("No such clan command.\n\r", ch);
}

void clando_induct(CHAR_DATA *ch, char clan)
{
    
}

void clando_discharge(CHAR_DATA *ch, char clan)
{
}

void clando_promote(CHAR_DATA *ch)
{
}

void clando_demote(CHAR_DATA *ch)
{
}

void clando_subclan(CHAR_DATA *ch, char subclan)
{
}