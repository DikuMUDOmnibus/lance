/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1998 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@hypercube.org)				   *
*	    Gabrielle Taylor (gtaylor@hypercube.org)			   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stddef.h>
#include <sys/stat.h>
#include <dirent.h>

#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"

/*
 * Externals.
 * Add this to top of act_wiz.c
 */
extern  AFFECT_DATA        *affect_free;

// For multiplayer check - Jon
extern	CHAR_DATA*	check_multi	args( (CHAR_DATA *ch) );

/*
 *  Tables for smite
 */
char *const pos_table[]=
{
"dead", "mortal", "incap", "stun", "sleep", "rest", "sit", "fight", "stand",
NULL
};

char *const eq_table[]=
{
"light", "finger_l", "finger_r", "neck_1", "neck_2", "torso", "head", "legs",
"feet", "hands", "arms", "shield", "body", "waist", "wrist_l", "wrist_r",
"wield", "hold", "float", "ankle", "ear", NULL
};






/*
 * Local functions.
 */
ROOM_INDEX_DATA *	find_location	args( ( CHAR_DATA *ch, char *arg ) );


CHAR_DATA *get_char( CHAR_DATA *ch )
{
    if ( !ch->pcdata )
        return ch->desc->original;
    else
        return ch;
}

void do_unreply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *pch;
    char arg[MAX_INPUT_LENGTH];
    
    one_argument(argument, arg);
    
    if(arg[0] == '\0')
    {
    	send_to_char("Take away reply from whom?\n\r", ch);
    	return;
    }
    
    if((pch = get_char_world(ch, argument)) == NULL)
    {
    	send_to_char("They aren't here.\n\r", ch);
    	return;
    }
    
    if(pch->reply != ch)
    {
    	act("$N doesn't reply to you.", ch, NULL, pch,TO_CHAR);
    	return;
    }
    
    pch->reply = NULL;
    act("$N no longer replies to you.", ch, NULL, pch, TO_CHAR);
}

void do_disable( CHAR_DATA *ch, char *argument )
{
    bool bFound = FALSE;
    int cmd;
    DO_FUN* func;
    
    if(argument[0] == '\0')
    {
    	/* Show the list */
    	send_to_char("The following commands have been disabled:\n\r", ch);
    	for(cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
    	{
    	    if(cmd_table[cmd].disabled)
    	    {
    	    	bFound = TRUE;
    	    	send_to_char(cmd_table[cmd].name, ch);
    	    	send_to_char("\n\r", ch);
    	    }
    	}
    	if(!bFound)
    	    send_to_char("None.\n\r", ch);
    	
    	return;
    }
    
    /* Check to see the command exists */
    for(cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
    {
    	if(!str_cmp(cmd_table[cmd].name, argument))
    	{
    	    bFound = TRUE;
    	    break;
    	}
    }
    
    if(!bFound)
    {
    	send_to_char("Command does not exist. You must type the whole command.\n\r", ch);
    	return;
    }
    
    /* Can't disable IMP commands */
    if(cmd_table[cmd].level == IMPLEMENTOR)
    {
    	send_to_char("Cannot disable IMP level commands.\n\r", ch);
    	return;
    }
    
    func = cmd_table[cmd].do_fun;
    
    /* Disable/enable the command */
    if(!cmd_table[cmd].disabled)
    {
    	cmd_table[cmd].disabled = TRUE;
    	printf_to_char(ch, "%s has been disabled.\n\r", cmd_table[cmd].name);
    }
    else
    {
    	cmd_table[cmd].disabled = FALSE;
    	printf_to_char(ch, "%s has been enabled.\n\r", cmd_table[cmd].name);
    }
    
    /* Find commands that are the same in usage */
    for(cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
    {
    	if(!str_cmp(cmd_table[cmd].name, argument))
    	    continue;
    	if(func == cmd_table[cmd].do_fun)
    	    cmd_table[cmd].disabled = !cmd_table[cmd].disabled;
    }
}

void do_home( CHAR_DATA *ch, char *argument )
{
    int vnum;
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *pRoom;
    CHAR_DATA *victim;

    if(argument[0] == '\0')
    {
    	send_to_char("Who do you want to send home?", ch);
    	return;
    }
        
    argument = one_argument(argument, arg);
    
    if((victim = get_char_world(ch, arg)) == NULL)
    {
    	send_to_char("They aren't here.\n\r", ch);
    	return;
    }
    
    vnum = (victim->recall ? victim->recall : (is_clan(victim) ? clan_table[victim->clan].recall : god_table[victim->god].recall));

    if(vnum == 0)
    	vnum = god_table[victim->god].recall;
    	
    if((pRoom = get_room_index(vnum)) == NULL)
    {
    	send_to_char("Cannot recall: Room does not exist.\n\r", ch);
    	return;
    }
    
    act("$n is sent home.", victim, NULL, NULL, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, pRoom);
    do_function(victim, &do_look, "auto");
    act("$n appears in the room.", victim, NULL, NULL, TO_ROOM);
    send_to_char("Player sent home.\n\r", ch);
    pet_follow(victim, victim->pet);
}

void do_arset( CHAR_DATA *ch, char* argument )
{
    AREA_DATA* pInArea;
    AREA_DATA* pArea;
    ROOM_INDEX_DATA* pRoom;
    RESET_DATA* pReset;
    int vnum;
    bool bFound = FALSE;
    char buf[MAX_STRING_LENGTH];
    
    /* Find out where the player is */
    pInArea = ch->in_room->area;
    /* Search each area for reset objs from this area */
    sprintf(buf, "Resets found from %s:\n\r", pInArea->name);
    send_to_char(buf, ch);
    for(pArea = area_first; pArea != NULL; pArea = pArea->next)
    {
        if(pArea != pInArea)
        {
            /* Search each room in the area */
            for(vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
            {
                if((pRoom = get_room_index(vnum)) != NULL)
                {
                    /* Scan the resets from each room */
                    for(pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next)
                    {
                        switch(pReset->command)
                        {
                        case 'M':
                            if(pReset->arg1 >= pInArea->min_vnum
                                && pReset->arg1 <= pInArea->max_vnum)
                            {
                                bFound = TRUE;
                                sprintf(buf, "Mob %d is reset in room %d [%s]\n\r",
                                  pReset->arg1, pRoom->vnum, pArea->name);
                                send_to_char(buf, ch);
                            }
                            break;
                        case 'O': case 'P': case 'G': case 'E':
                            if(pReset->arg1 >= pInArea->min_vnum
                                && pReset->arg1 <= pInArea->max_vnum)
                            {
                                bFound = TRUE;
                                sprintf(buf, "Obj %d is reset in room %d [%s]\n\r",
                                  pReset->arg1, pRoom->vnum, pArea->name);
                                send_to_char(buf, ch);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    if(!bFound)
        send_to_char("None.\n\r", ch);
}

void do_rlink( CHAR_DATA *ch, char *argument )
{
    const char* dir_name[] = {"n","e","s","w","u","d"};
    char buf[MAX_STRING_LENGTH];
    int door, vnum;
    bool bFoundExit = FALSE;
    EXIT_DATA* pExit;
    AREA_DATA* pArea;
    AREA_DATA* pCurrArea;
    ROOM_INDEX_DATA* pRoom;
    
    pCurrArea = ch->in_room->area;
    sprintf(buf, "The following rooms link %s (%d-%d):\n\r", pCurrArea->name, pCurrArea->min_vnum, pCurrArea->max_vnum);
    send_to_char(buf, ch);
    /* Find the first room vnum for the area */
    for(pArea = area_first; pArea != NULL; pArea = pArea->next)
    {
        if(pArea == pCurrArea)
            continue;
            
        for(vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
        {
            /* Check each exit for the link */
            if( (pRoom = get_room_index(vnum)) != NULL)
            {
                for(door = 0; door <= 5; door++)
                {
                    pExit = pRoom->exit[door];
                    if(pExit != NULL && pExit->u1.to_room != NULL)
                    /* Exit exists */
                    {
                        /* Check whether that exit leads into the current area */
                        if(pExit->u1.to_room->vnum >= pCurrArea->min_vnum
                            && pExit->u1.to_room->vnum <= pCurrArea->max_vnum)
                        {
                            /* Room exist does not lead to a room of the same area */
                            sprintf(buf, "%d[{c%s{x][{y%s{x].\n\r", pRoom->vnum, dir_name[door], pArea->name);
                            send_to_char(buf, ch);
                            if(!str_cmp(argument, "unlink"))
                            {
                                sh_int rev;
                                ROOM_INDEX_DATA *pToRoom;
                                
                                rev = rev_dir[door];
                                pToRoom = pExit->u1.to_room;
                                
                                free_exit( pRoom->exit[door] );
                                pRoom->exit[door] = NULL;
                                
                                if( pToRoom->exit[rev] )
                                {
                                    free_exit( pToRoom->exit[rev] );
                                    pToRoom->exit[rev] = NULL;
                                }
                                send_to_char("Exit unlinked.\n\r", ch);
                            }
                            bFoundExit = TRUE;
                        }
                    }
                }
            }
        }
    }    
    if(!bFoundExit)
        send_to_char("None.\n\r", ch);
}
    
void do_wiznet( CHAR_DATA *ch, char *argument )
{
    int flag;
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
      	if (IS_SET(ch->wiznet,WIZ_ON))
      	{
            send_to_char("Signing off of Wiznet.\n\r",ch);
            REMOVE_BIT(ch->wiznet,WIZ_ON);
      	}
      	else
      	{
            send_to_char("Welcome to Wiznet!\n\r",ch);
            SET_BIT(ch->wiznet,WIZ_ON);
      	}
      	return;
    }

    if (!str_prefix(argument,"on"))
    {
	send_to_char("Welcome to Wiznet!\n\r",ch);
	SET_BIT(ch->wiznet,WIZ_ON);
	return;
    }

    if (!str_prefix(argument,"off"))
    {
	send_to_char("Signing off of Wiznet.\n\r",ch);
	REMOVE_BIT(ch->wiznet,WIZ_ON);
	return;
    }

    /* show wiznet status */
    if (!str_prefix(argument,"status")) 
    {
	buf[0] = '\0';

	if (!IS_SET(ch->wiznet,WIZ_ON))
	    strcat(buf,"off ");

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
	    {
		strcat(buf,wiznet_table[flag].name);
		strcat(buf," ");
	    }

	strcat(buf,"\n\r");

	send_to_char("Wiznet status:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }

    if (!str_prefix(argument,"show"))
    /* list of all wiznet options */
    {
	buf[0] = '\0';

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	{
	    if (wiznet_table[flag].level <= get_trust(ch))
	    {
	    	strcat(buf,wiznet_table[flag].name);
	    	strcat(buf," ");
	    }
	}

	strcat(buf,"\n\r");

	send_to_char("Wiznet options available to you are:\n\r",ch);
	send_to_char(buf,ch);
	return;
    }
   
    flag = wiznet_lookup(argument);

    if (flag == -1 || get_trust(ch) < wiznet_table[flag].level)
    {
	send_to_char("No such option.\n\r",ch);
	return;
    }
   
    if (IS_SET(ch->wiznet,wiznet_table[flag].flag))
    {
	sprintf(buf,"You will no longer see %s on wiznet.\n\r",
	        wiznet_table[flag].name);
	send_to_char(buf,ch);
	REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
    	return;
    }
    else
    {
    	sprintf(buf,"You will now see %s on wiznet.\n\r",
		wiznet_table[flag].name);
	send_to_char(buf,ch);
    	SET_BIT(ch->wiznet,wiznet_table[flag].flag);
	return;
    }

}

void wiznet(char *string, CHAR_DATA *ch, OBJ_DATA *obj,
	    long flag, long flag_skip, int min_level) 
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if (d->connected == CON_PLAYING
	&&  IS_IMMORTAL(d->character) 
	&&  IS_SET(d->character->wiznet,WIZ_ON) 
	&&  (!flag || IS_SET(d->character->wiznet,flag))
	&&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
	&&  get_trust(d->character) >= min_level
	&&  d->character != ch)
        {
	    if (IS_SET(d->character->wiznet,WIZ_PREFIX))
	  	send_to_char("{g--> ",d->character);
	    else
	  	send_to_char( "{g", d->character );
            act_new(string,d->character,obj,ch,TO_CHAR,POS_DEAD);
	    send_to_char( "{x", d->character );
        }
    }
 
    return;
}

void do_guild( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int clan;
    int subclan;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    clan = clan_lookup(arg2);
    subclan = subclan_lookup(arg3);

    if ( arg1[0] == '\0' || arg2[0] == '\0' || ( clan_table[clan].subclan && arg3[0] == '\0' ) )
    {
        send_to_char( "Syntax: guild <char> <clan name> <subclan name>\n\r",ch);
        return;
    }
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't playing.\n\r", ch );
        return;
    }
    
    if (!str_prefix(arg2,"none"))
    {
	send_to_char("They are now clanless.\n\r",ch);
	send_to_char("You are now a member of no clan!\n\r",victim);
	victim->clan = 0;
	victim->subclan = 0;
	victim->clanlevel = 0;
	return;
    }

    if (clan == 0)
    {
	send_to_char("No such clan exists.\n\r",ch);
	return;
    }

    if (clan_table[clan].subclan)
    {
        if( strcmp( arg3, "none" ) )
        {
		if (subclan == 0)
		{
    			send_to_char("No such subclan exists.\n\r",ch);
    			return;
    		}
    		if (subclan_table[subclan].clan != clan)
    		{
    			send_to_char("Subclan does not match the clan.\n\r",ch);
    			return;
    		}
    	} else
    		subclan = 0;
    		
    	
    }

    sprintf(buf,"Player's new clan is: %s",
	capitalize(clan_table[clan].name));

    sprintf(buf2,"You are now a part of %s",
        capitalize(clan_table[clan].name));

    if (clan_table[clan].subclan)
	sprintf(buf3, " %s.\n\r", capitalize(subclan_table[subclan].name));
    else
	sprintf(buf3, ".\n\r");
	
    strcat(buf, buf3);
    strcat(buf2, buf3);
	
    send_to_char(buf, ch);
    send_to_char(buf2, victim);
    

    victim->clan = clan;
    victim->subclan= subclan;
    if(victim->level >= IMMORTAL)
        victim->clanlevel = 11;
    else
        victim->clanlevel = 1;
    
    return;
}

/*
 * Coded by: Thale (Andrew Maslin)
 * Syntax: Rename <victim> <new_name>
 * Limitations: This header must be kept with this function.  In addition,
 * this file is subject to the ROM license.  The code in this file is
 * copywritten by Andrew Maslin, 1998.  If you have a "credits" help in your
 * mud, please add the name Thale to that as credit for this function.
 */
void do_rename(CHAR_DATA *ch, char *argument)
{
  CHAR_DATA *victim;
  FILE *fp;
  char strsave[MAX_INPUT_LENGTH];
  char *name;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  char playerfile[MAX_INPUT_LENGTH];


  if (!IS_IMMORTAL(ch))
  {
    send_to_char("You don't have the power to do that.\n\r",ch);
    return;
  }

  argument = one_argument(argument,arg1);
  argument = one_argument(argument,arg2);

  if (arg1[0] == '\0')
  {
    send_to_char("Rename who?\n\r",ch);
    return;
  }
  if (arg2[0] == '\0')
  {
    send_to_char("What should their new name be?\n\r",ch);
    return;
  }

  arg2[0] = UPPER(arg2[0]);

  if ((victim = get_char_world(ch,arg1)) == NULL)
  {
    send_to_char("They aren't connected.\n\r",ch);
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("Use string for NPC's.\n\r",ch);
    return;
  }

  if (!check_parse_name(arg2))
  {
    sprintf(buf,"The name {c%s{x is {Rnot allowed{x.\n\r",arg2);
    send_to_char(buf,ch);
    return;
  }

  sprintf(playerfile, "%s%s", PLAYER_DIR, capitalize(arg2));
  if ((fp = fopen(playerfile, "r")) != NULL)
  {
    sprintf(buf,"You can't have two people named %s dummy.\n\r",capitalize(arg2));
    send_to_char(buf,ch);
    fclose(fp);
    return;
  }

  if ((victim->level >= ch->level) && (victim->level >= ch->trust)
    &&((ch->level != IMPLEMENTOR) || (ch->trust != IMPLEMENTOR))
    &&(ch != victim))
  {
    send_to_char("I don't think that's a good idea.\n\r",ch);
    return;
  }

  if (victim->position == POS_FIGHTING)
  {
    send_to_char("They are fighting right now.\n\r",ch);
    return;
  }

  name = str_dup(victim->name);
  sprintf( strsave, "%s%s", PLAYER_DIR, capitalize(victim->name));
  arg2[0] = UPPER(arg2[0]);
  free_string(victim->name);
  victim->name = str_dup(arg2);
  save_char_obj(victim);
  unlink(strsave);
#if defined(unix)
  if (IS_IMMORTAL(victim))
  {
    sprintf(strsave,"%s%s", GOD_DIR, capitalize(name));
    unlink(strsave);
  }
#endif
  if (victim != ch)
  {
    sprintf(buf,"{YNOTICE: {xYou have been renamed to {c%s{x.\n\r",arg2);
    send_to_char(buf,victim);
  }
  send_to_char("Done.\n\r",ch);

return;
}

void do_bonus( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char       buf  [ MAX_STRING_LENGTH ];
    char       arg1 [ MAX_INPUT_LENGTH ];
    char       arg2 [ MAX_INPUT_LENGTH ];
    int      value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: bonus <char> <Exp>.\n\r", ch );
	return;
    }
    
    if (( victim = get_char_world ( ch, arg1 ) ) == NULL )
    {
      send_to_char("That player is not here.\n\r", ch);
      return;
    }
       
    if ( IS_NPC( victim ) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "You may not award yourself experience points.\n\r", ch );
	return;
    }

    if (IS_IMMORTAL(victim) || victim->level >= LEVEL_IMMORTAL)
    {
    send_to_char("You can't award experience to immortals silly!\n\r", ch);
    return;
    }

    value = atoi( arg2 );

    if ( value < -100000 || value > 100000 )
    {
	send_to_char( "Value range is -100000 to 100000.\n\r", ch );
	return;
    }

    if ( value == 0 )
    {
	send_to_char( "The value must not be equal to 0.\n\r", ch );
	return;
    }
     
    if (value > 0)
    {
    gain_exp(victim, value);
   
    sprintf( buf,"{YNOTICE:{c You have awarded {B%s{c a whopping {D%d{c experience points!{x\n\r",
    		victim->name, value);
    		send_to_char(buf, ch);
    }            
    else
    {
    gain_exp(victim, value);
  
    sprintf( buf,"{YNOTICE:{R You have penalized {B%s{R a whopping {D%d{R experience points!{x\n\r",
                victim->name, value);
                send_to_char(buf, ch);
    }

    if ( value > 0 )
    { 
      sprintf( buf,"{c%s likes you! {BYou have been awarded %d experience points!{x\n\r", ch->name, value );
      send_to_char( buf, victim );
    }
    else
    {
      sprintf( buf,"{c%s doesn't like you! {RYou have been penalized %d experience points!{x\n\r", ch->name, value );
      send_to_char( buf, victim );
    }
 
    return;
}  

/*
 * Copywrite 1996 by Amadeus of AnonyMUD, AVATAR, Horizon MUD, and Despair
 *			( amadeus@myself.com )
 *
 * Public use authorized with this header intact.
 */		
void do_wpeace(CHAR_DATA *ch, char *argument )
{
     CHAR_DATA *rch;
     char buf[MAX_STRING_LENGTH];

     rch = get_char( ch );

     for ( rch = char_list; rch; rch = rch->next )
      {
	if ( ch->desc == NULL || ch->desc->connected != CON_PLAYING )
	   continue;

	if ( rch->fighting )
	  {
	   sprintf( buf, "%s has declared World Peace.\n\r", ch->name );
	   send_to_char( buf, rch );
	   stop_fighting( rch, TRUE );
          }
      }

    send_to_char( "You have declared World Peace.\n\r", ch );
    return;
}

void do_olevel(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];
    char level[MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 50;

    buffer = new_buf();

    argument = one_argument(argument, level);
    if (level[0] == '\0')
    {
        send_to_char("Syntax: olevel <level>\n\r",ch);
        send_to_char("        olevel <level> <name>\n\r",ch);
        return;
    }
 
    argument = one_argument(argument, name);
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( obj->level != atoi(level) )
            continue;

	if ( name[0] != '\0' && !is_name(name, obj->name) )
	    continue;

        found = TRUE;
        number++;
 
        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );
 
        if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
        &&   in_obj->carried_by->in_room != NULL)
            sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
                number, obj->short_descr,PERS(in_obj->carried_by, ch),
                in_obj->carried_by->in_room->vnum );
        else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
            sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
                number, obj->short_descr,in_obj->in_room->name, 
                in_obj->in_room->vnum);
        else
            sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr);
 
        buf[0] = UPPER(buf[0]);
        add_buf(buffer,buf);
 
        if (number >= max_found)
            break;
    }
 
    if ( !found )
        send_to_char( "There are no objects that level.\n\r", ch );
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
}

void do_mlevel( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	send_to_char("Syntax: mlevel <level>\n\r",ch);
	return;
    }

    found = FALSE;
    buffer = new_buf();
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
        if ( victim->in_room != NULL
        &&   atoi(argument) == victim->level )
        {
            found = TRUE;
            count++;
            sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
                IS_NPC(victim) ? victim->pIndexData->vnum : 0,
                IS_NPC(victim) ? victim->short_descr : victim->name,
                victim->in_room->vnum,
                victim->in_room->name );
            add_buf(buffer,buf);
        }
    }

    if ( !found )
        act( "You didn't find any mob of level $T.", ch, NULL, argument, TO_CHAR );
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);

    return;
}

void do_addlag(CHAR_DATA *ch, char *argument)
{

	CHAR_DATA *victim;
	char arg1[MAX_STRING_LENGTH];
	int x;

	argument = one_argument(argument, arg1);

	if (arg1[0] == '\0')
	{
		send_to_char("Who do you wish to torture endlessly?\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL)
	{
		send_to_char("They're not here.\n\r", ch);
		return;
	}

	if ((x = atoi(argument)) <= 0)
	{
		send_to_char("What's the point in that moron?\n\r", ch);
		return;
	}

	if (x > 300)
	{
		send_to_char("There's a limit to cruel and unusual punishment.\n\r", ch);
		return;
	}

	WAIT_STATE(victim, x);
	send_to_char("Adding lag now...", ch);
	return;
}

void do_nuke( CHAR_DATA *ch, char *argument)
{
   char strsave[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];

   one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: nuke  <character name> \n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They are not logged on.\n\r", ch );
	return;
    }

   if (IS_NPC(victim))
    {
     send_to_char( "What's wrong with you? How do you delete a mobile?\n\r", ch);
      return;
    }

    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( victim->name ) );
    stop_fighting(victim,TRUE);
    sprintf( buf,"You have successfully removed {c%s{x from the game {Dpermanently!{x\n\r", victim->name);
    send_to_char( buf, ch);
    sprintf( buf,"{YWARNING:{c You have been {RNUKED{c by %s{Y!!!{x (aka - deleted)\n\r", ch->name);
    send_to_char( buf, victim);
    do_quit(victim,"");
    unlink(strsave);
    return;
}

/* Voltecs being mean to characters section... Voltec -98 */

void do_notitle(CHAR_DATA *ch, char *argument)
{
   char      arg[MAX_INPUT_LENGTH];
   char      buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   argument = one_argument(argument, arg);
   victim   = get_char_world(ch, arg);

   if (arg[0] == '\0')
      {
      send_to_char("Whose Title Priviliges do you wish to change?\n\r", ch);
      return;
      }

	if ((victim = get_char_world( ch, arg ))==NULL )
      {
      send_to_char("They aren't of this world!\n\r", ch);
      return;
      }
   else
      {
      if (IS_NPC(victim))
         {
         send_to_char("Not on NPC's!\n\r", ch);
         return;
         }
		else if (IS_SET(victim->act, PLR_NOTITLE))
			{
			REMOVE_BIT(victim->act, PLR_NOTITLE);
         sprintf(buf, "You restore %s's Title priviliges!\n\r", victim->name);
         send_to_char(buf, ch);
         return;
         }
      else
         {
			SET_BIT(victim->act, PLR_NOTITLE);
         sprintf(buf, "You remove %s's Title priviliges!\n\r", victim->name);
         send_to_char(buf, ch);
         return;
         }
      }
}

void do_noexp(CHAR_DATA *ch, char *argument)
{
   char      arg[MAX_INPUT_LENGTH];
   char      buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
      {
      send_to_char("Whose EXP Priviliges do you wish to change?\n\r", ch);
      return;
      }

	if ((victim = get_char_world( ch, arg ) )==NULL )
      {
      send_to_char("They aren't of this world!\n\r", ch);
      return;
      }
   else
      {
      if (IS_NPC(victim))
         {
         send_to_char("Not on NPC's!\n\r", ch);
         return;
         }
      else if (IS_SET(victim->act, PLR_NOEXP))
         {
         REMOVE_BIT(victim->act, PLR_NOEXP);
         sprintf(buf, "You restore %s's Exp priviliges!\n\r", victim->name);
         send_to_char(buf, ch);
         return;
         }
      else
         {
         SET_BIT(victim->act, PLR_NOEXP);
         sprintf(buf, "You remove %s's Exp priviliges!\n\r", victim->name);
         send_to_char(buf, ch);
         return;
         }
      }
}

void do_noqp(CHAR_DATA *ch, char *argument)
{
   char      arg[MAX_INPUT_LENGTH];
   char      buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
      {
      send_to_char("Whose QP Priviliges do you wish to change?\n\r", ch);
      return;
      }

	if ((victim = get_char_world( ch, arg ) )==NULL )
      {
      send_to_char("They aren't of this world!\n\r", ch);
      return;
      }
   else
      {
      if (IS_NPC(victim))
         {
         send_to_char("Not on NPC's!\n\r", ch);
         return;
         }
      else if (IS_SET(victim->act, PLR_NOQP))
         {
         REMOVE_BIT(victim->act, PLR_NOQP);
         sprintf(buf, "You restore %s's QP priviliges!\n\r", victim->name);
         send_to_char(buf, ch);
         return;
         }
      else
         {
         SET_BIT(victim->act, PLR_NOQP);
         sprintf(buf, "You remove %s's QP priviliges!\n\r", victim->name);
         send_to_char(buf, ch);
         return;
         }
      }
}

/** Function: do_pload
  * Descr   : Loads a player object into the mud, bringing them (and their
  *           pet) to you for easy modification.  Player must not be connected.
  *           Note: be sure to send them back when your done with them.
  * Returns : (void)
  * Syntax  : pload (who)
  * Written : v1.0 12/97
  * Author  : Gary McNickle <gary@dharvest.com>
  */
void do_pload( CHAR_DATA *ch, char *argument )
{
  DESCRIPTOR_DATA d;
  bool isChar = FALSE;
  char name[MAX_INPUT_LENGTH];

  if (argument[0] == '\0')
  {
    send_to_char("Load who?\n\r", ch);
    return;
  }

  argument[0] = UPPER(argument[0]);
  argument = one_argument(argument, name);

  /* Dont want to load a second copy of a player who's allready online! */
  if ( get_char_world( ch, name ) != NULL )
  {
    send_to_char( "That person is already connected!\n\r", ch );
    return;
  }

  isChar = load_char_obj(&d, name); /* char pfile exists? */

  if (!isChar)
  {
    send_to_char("Load Who? Are you sure? I cant seem to find them.\n\r", ch);
    return;
  }

  d.character->desc     = NULL;
  d.host		= str_dup("");
  d.character->next     = char_list;
  char_list             = d.character;
  d.connected           = CON_PLAYING;
  reset_char(d.character);

  /* bring player to imm */
  if ( d.character->in_room != NULL )
  {
    char_to_room( d.character, ch->in_room); /* put in room imm is in */
  }

  act( "$n has pulled $N from the pattern!",
        ch, NULL, d.character, TO_ROOM );

  if (d.character->pet != NULL)
   {
     char_to_room(d.character->pet,d.character->in_room);
     act("$n has entered the game.",d.character->pet,NULL,NULL,TO_ROOM);
   }

}

/** Function: do_punload
  * Descr   : Returns a player, previously 'ploaded' back to the void from
  *           whence they came.  This does not work if the player is actually 
  *           connected.
  * Returns : (void)
  * Syntax  : punload (who)
  * Written : v1.0 12/97
  * Author  : Gary McNickle <gary@dharvest.com>
  */
void do_punload( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char who[MAX_INPUT_LENGTH];

  argument = one_argument(argument, who);

  if ( ( victim = get_char_world( ch, who ) ) == NULL )
  {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  /** Person is legitametly logged on... was not ploaded.
   */
  if (victim->desc != NULL)
  {
    send_to_char("I dont think that would be a good idea...\n\r", ch);
    return;
  }

  if (victim->was_in_room != NULL) /* return player and pet to orig room */
  {
    char_to_room(victim, victim->was_in_room);
    if (victim->pet != NULL)
      char_to_room(victim->pet, victim->was_in_room);
  }

  save_char_obj(victim);
  do_quit(victim,"");

  act("$n has released $N back to the Pattern.",
       ch, NULL, victim, TO_ROOM);
}

/*Outfit command*/
void do_outfit ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *wl, *wr, *obj;
    int i,sn,vnum;

    if (ch->level > 5 || IS_NPC(ch))
    {
	send_to_char("Find it yourself!\n\r",ch);
	return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL )
    {
        obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_LIGHT );
    }
 
    if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL )
    {
	obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );
	obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_BODY );
    }

    /* do the weapon thing */
    wl = get_eq_char(ch, WEAR_LEFT);
    wr = get_eq_char(ch, WEAR_RIGHT);
    if ((wl == NULL || (wl != NULL && wl->item_type != ITEM_WEAPON))
       && (wr == NULL || (wr != NULL && wr->item_type != ITEM_WEAPON)))
    {
    	sn = 0; 
    	vnum = OBJ_VNUM_SCHOOL_SWORD; /* just in case! */

    	for (i = 0; weapon_table[i].name != NULL; i++)
    	{
	    if (ch->pcdata->learned[sn] < 
		ch->pcdata->learned[*weapon_table[i].gsn])
	    {
	    	sn = *weapon_table[i].gsn;
	    	vnum = weapon_table[i].vnum;
	    }
    	}

    	obj = create_object(get_obj_index(vnum),0);
     	obj_to_char(obj,ch);
     	if(ch->pcdata->learned[gsn_right_hand] < ch->pcdata->learned[gsn_left_hand])
     	{
     	    equip_char(ch, obj, WEAR_LEFT);
     	}
     	else
     	{
    	    equip_char(ch,obj, WEAR_RIGHT);
        }
    }

    send_to_char("You have been equipped by the gods.\n\r",ch);
}

     
/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
 
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    {
        send_to_char( "Nochannel whom?\n\r", ch );
        return;
    }
 
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
 
    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }
 
    if ( IS_SET(victim->comm, COMM_NOCHANNELS) )
    {
        REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "The gods have restored your channel priviliges.\n\r", 
		      victim );
        send_to_char( "NOCHANNELS removed.\n\r", ch );
	sprintf(buf,"$N restores channels to %s",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
        SET_BIT(victim->comm, COMM_NOCHANNELS);
        send_to_char( "The gods have revoked your channel priviliges.\n\r", 
		       victim );
        send_to_char( "NOCHANNELS set.\n\r", ch );
	sprintf(buf,"$N revokes %s's channels.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
 
    return;
}


void do_smote(CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char *letter,*name;
    char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
    int matches = 0;
 
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) )
    {
        send_to_char( "You can't show your emotions.\n\r", ch );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }
    
    if (strstr(argument,ch->name) == NULL)
    {
	send_to_char("You must include your name in an smote.\n\r",ch);
	return;
    }
   
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
 
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
        if (vch->desc == NULL || vch == ch)
            continue;
 
        if ((letter = strstr(argument,vch->name)) == NULL)
        {
            if(!IS_IMMORTAL(ch))
            	send_to_char("*", vch);
	    send_to_char(argument,vch);
	    send_to_char("\n\r",vch);
            continue;
        }
 
        strcpy(temp,argument);
        temp[strlen(argument) - strlen(letter)] = '\0';
        last[0] = '\0';
        name = vch->name;
 
 
 	if(!IS_IMMORTAL(ch))
 	    send_to_char("*", vch);
 	
        for (; *letter != '\0'; letter++)
        {
            if (*letter == '\'' && matches == strlen(vch->name))
            {
                strcat(temp,"r");
                continue;
            }
 
            if (*letter == 's' && matches == strlen(vch->name))
            {
                matches = 0;
                continue;
            }
 
            if (matches == strlen(vch->name))
            {
                matches = 0;
            }
 
            if (*letter == *name)
            {
                matches++;
                name++;
                if (matches == strlen(vch->name))
                {
                    strcat(temp,"you");
                    last[0] = '\0';
                    name = vch->name;
                    continue;
                }
                strncat(last,letter,1);
                continue;
            }
 
            matches = 0;
            strcat(temp,last);
            strncat(temp,letter,1);
            last[0] = '\0';
            name = vch->name;
        }
 
	send_to_char(temp,vch);
	send_to_char("\n\r",vch);
    }
 
    return;
}

void do_transin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    if(IS_NPC(ch))
    	return;
    
    smash_tilde( argument );
    
    if(argument[0] == '\0')
    {
    	sprintf(buf, "Your transin is %s\n\r", ch->pcdata->transin);
    	send_to_char(buf, ch);
    	return;
    }
    
    if( strstr(argument, "%s") == NULL)
    {
    	send_to_char("You must include a %s in the line.\n\r", ch);
    	return;
    }
    
    free_string(ch->pcdata->transin);
    ch->pcdata->transin = str_dup( argument );
    
    sprintf(buf, "Your transin is now: %s\n\r", ch->pcdata->transin);
    send_to_char(buf, ch);
}

void do_transout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    if(IS_NPC(ch))
    	return;
    
    smash_tilde( argument );
    
    if(argument[0] == '\0')
    {
    	sprintf(buf, "Your transout is %s\n\r", ch->pcdata->transout);
    	send_to_char(buf, ch);
    	return;
    }
    
    if( strstr(argument, "%s") == NULL)
    {
    	send_to_char("You must include a %s in the line.\n\r", ch);
    	return;
    }
    
    free_string(ch->pcdata->transout);
    ch->pcdata->transout = str_dup( argument );
    
    sprintf(buf, "Your transout is now: %s\n\r", ch->pcdata->transout);
    send_to_char(buf, ch);
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( !IS_NPC(ch) )
    {
	smash_tilde( argument );

	if (argument[0] == '\0')
	{
	    sprintf(buf,"Your poofin is %s\n\r",ch->pcdata->bamfin);
	    send_to_char(buf,ch);
	    return;
	}

	if ( strstr(argument,ch->name) == NULL)
	{
	    send_to_char("You must include your name.\n\r",ch);
	    return;
	}
	     
	free_string( ch->pcdata->bamfin );
	ch->pcdata->bamfin = str_dup( argument );

        sprintf(buf,"Your poofin is now %s\n\r",ch->pcdata->bamfin);
        send_to_char(buf,ch);
    }
    return;
}

void do_bamfout( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
 
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
 
        if (argument[0] == '\0')
        {
            sprintf(buf,"Your poofout is %s\n\r",ch->pcdata->bamfout);
            send_to_char(buf,ch);
            return;
        }
 
        if ( strstr(argument,ch->name) == NULL)
        {
            send_to_char("You must include your name.\n\r",ch);
            return;
        }
 
        free_string( ch->pcdata->bamfout );
        ch->pcdata->bamfout = str_dup( argument );
 
        sprintf(buf,"Your poofout is now %s\n\r",ch->pcdata->bamfout);
        send_to_char(buf,ch);
    }
    return;
}



void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim = NULL;

    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Deny whom?\n\r", ch );
	return;
    }
    
    for( d = descriptor_list; d; d = d->next )
    {
    	if( d->character && !str_cmp( d->character->name, arg1 ) )
    	{
    		victim = d->character;
    		break;
    	}
    }
    
    if( !victim )
    {
    	send_to_char( "They aren't here.\n\r", ch );
    	return;
        
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
	if (arg2 != NULL)
	{
		sprintf(buf, "%s\n\r", arg2);
		send_to_char( buf, victim );
	}
	else {
    send_to_char( "{CCharacter name not approved, please try another! If you need a good name check out http://spitfire.ausys.se/johan/names/default.htm{x\n\r", victim);
    }
    sprintf(buf,"$N denies access to %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    send_to_char( "OK.\n\r", ch );
    save_char_obj(victim);
    stop_fighting(victim,TRUE);
    do_function(victim, &do_quit, "" );

    return;
}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Disconnect whom?\n\r", ch );
	return;
    }

    if (is_number(arg))
    {
	int desc;

	desc = atoi(arg);
    	for ( d = descriptor_list; d != NULL; d = d->next )
    	{
            if ( d->descriptor == desc )
            {
            	close_socket( d );
            	send_to_char( "Ok.\n\r", ch );
            	return;
            }
	}
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d == victim->desc )
	{
	    close_socket( d );
	    send_to_char( "Ok.\n\r", ch );
	    return;
	}
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}

void do_pardon( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: pardon <character> <violent|killer|thief|slain|pk>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "killer" ) )
    {
	if ( IS_SET(victim->act, PLR_KILLER) )
	{
	    REMOVE_BIT( victim->act, PLR_KILLER );
	    send_to_char( "Killer flag removed.\n\r", ch );
	    send_to_char( "You are no longer a KILLER.\n\r", victim );
	}
	return;
    }

    if ( !str_cmp( arg2, "thief" ) )
    {
	if ( IS_SET(victim->act, PLR_THIEF) )
	{
	    REMOVE_BIT( victim->act, PLR_THIEF );
	    send_to_char( "Thief flag removed.\n\r", ch );
	    send_to_char( "You are no longer a THIEF.\n\r", victim );
	}
	return;
    }
    
    if( !str_cmp( arg2, "violent" ) )
    {
    	if( IS_SET(victim->act, PLR_VIOLENT) )
    	{
    	    REMOVE_BIT( victim->act, PLR_VIOLENT );
    	    victim->violent = 0;
    	    send_to_char( "Violent flag removed.\n\r", ch );
    	    send_to_char( "You are no longer violent.\n\r", victim );
    	}
    	return;
    }
    
    if( !str_cmp( arg2, "slain" ) )
    {
    	if( IS_SET(victim->act, PLR_SLAIN) )
    	{
    	    REMOVE_BIT( victim->act, PLR_SLAIN );
    	    victim->slain = 0;
    	    send_to_char( "Slain flag removed.\n\r", ch );
    	    send_to_char( "You are vulnerable to attack again.\n\r", victim );
    	}
    	return;
    }
    if( !str_cmp( arg2, "pk" ) )
	{
	   if( IS_SET(victim->act, PLR_PROKILLER) )
		{
	    	   REMOVE_BIT( victim->act, PLR_PROKILLER );
		   send_to_char( "PKiller flag removed.\n\r", ch);
		   send_to_char( "You are lucky, you are no longer a PKer.\n\r", victim);
		}
	   return;
	}
    send_to_char( "Syntax: pardon <character> <violent|killer|thief|slain|pk>.\n\r", ch );
    return;
}



void do_echo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Global echo what?\n\r", ch );
	return;
    }
    
    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING )
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char( "global> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}



void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Local echo what?\n\r", ch );

	return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
	if ( d->connected == CON_PLAYING
	&&   d->character->in_room == ch->in_room )
	{
            if (get_trust(d->character) >= get_trust(ch))
                send_to_char( "local> ",d->character);
	    send_to_char( argument, d->character );
	    send_to_char( "\n\r",   d->character );
	}
    }

    return;
}

void do_zecho(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	send_to_char("Zone echo what?\n\r",ch);
	return;
    }

    for (d = descriptor_list; d; d = d->next)
    {
	if (d->connected == CON_PLAYING
	&&  d->character->in_room != NULL && ch->in_room != NULL
	&&  d->character->in_room->area == ch->in_room->area)
	{
	    if (get_trust(d->character) >= get_trust(ch))
		send_to_char("zone> ",d->character);
	    send_to_char(argument,d->character);
	    send_to_char("\n\r",d->character);
	}
    }
}

void do_pecho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);
 
    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
	send_to_char("Personal echo what?\n\r", ch); 
	return;
    }
   
    if  ( (victim = get_char_world(ch, arg) ) == NULL )
    {
	send_to_char("Target not found.\n\r",ch);
	return;
    }

    if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
        send_to_char( "personal> ",victim);

    send_to_char(argument,victim);
    send_to_char("\n\r",victim);
    send_to_char( "personal> ",ch);
    send_to_char(argument,ch);
    send_to_char("\n\r",ch);
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
	return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
	return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
	return obj->in_room;

    return NULL;
}

void do_slip( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( !is_room_owner(ch,location) && room_is_private( location ) 
	&&  get_trust(ch) < MAX_LEVEL)
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );

    char_from_room( victim );
    char_to_room( victim, location);
    send_to_char("Ok.\n\r", ch);
}

void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Transfer whom (and where)?\n\r", ch );
	return;
    }

    if(!str_cmp(arg1, "allmortal"))
    {
        for(d = descriptor_list; d != NULL; d = d->next)
        if(d->connected == CON_PLAYING
        && !IS_IMMORTAL(d->character)
        && d->character != ch
        && d->character->in_room != NULL)
        {
            char buf[MAX_INPUT_LENGTH];
            sprintf(buf, "%s %s", d->character->name, arg2);
            do_function(ch, &do_transfer, buf);
        }
        return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    &&   d->character != ch
	    &&   d->character->in_room != NULL
	    &&   can_see( ch, d->character ) )
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "%s %s", d->character->name, arg2 );
		do_function(ch, &do_transfer, buf );
	    }
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
	location = ch->in_room;
    }
    else
    {
	if ( ( location = find_location( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No such location.\n\r", ch );
	    return;
	}

	if ( !is_room_owner(ch,location) && room_is_private( location ) 
	&&  get_trust(ch) < MAX_LEVEL)
	{
	    send_to_char( "That room is private right now.\n\r", ch );
	    return;
	}
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->in_room == NULL )
    {
	send_to_char( "They are in limbo.\n\r", ch );
	return;
    }

    if ( victim->fighting != NULL )
	stop_fighting( victim, TRUE );

    if( ch->pcdata != NULL && ch->pcdata->transout[0] != '\0')
    {
    	if(!IS_NPC(victim))
    	    sprintf(buf, ch->pcdata->transout, victim->name);
    	else
    	    sprintf(buf, ch->pcdata->transout, victim->short_descr);
        act("$t", victim, buf, NULL, TO_ROOM);
    }
    else
	act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
    
    char_from_room( victim );
    char_to_room( victim, location );
    
    if( ch->pcdata != NULL && ch->pcdata->transin[0] != '\0')
    {
    	if(!IS_NPC(victim))
    	    sprintf(buf, ch->pcdata->transin, victim->name);
    	else
    	    sprintf(buf, ch->pcdata->transin, victim->short_descr);
	act("$t", victim, buf, NULL, TO_ROOM);
    }
    else
	act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
	act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_function(victim, &do_look, "auto" );
    send_to_char( "Ok.\n\r", ch );
    pet_follow(victim, victim->pet);
}



void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA *on;
    CHAR_DATA *wch;
    
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "At where what?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && room_is_private( location ) 
    &&  get_trust(ch) < MAX_LEVEL)
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
	if ( wch == ch )
	{
	    char_from_room( ch );
	    char_to_room( ch, original );
	    ch->on = on;
	    break;
	}
    }

    return;
}



void do_goto( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Goto where?\n\r", ch );
	return;
    }

    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    count = 0;
    for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
        count++;

    if (!is_room_owner(ch,location) && room_is_private(location) 
    &&  (count > 1 || get_trust(ch) < MAX_LEVEL))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    if ( ch->fighting != NULL )
	stop_fighting( ch, TRUE );

    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
	if (get_trust(rch) >= ch->invis_level)
	{
	    if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
		act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
	    else
		act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
	}
    }

    char_from_room( ch );
    char_to_room( ch, location );


    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }

    do_function(ch, &do_look, "auto" );
    pet_follow(ch, ch->pet);
    return;
}

void do_violate( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;
 
    if ( argument[0] == '\0' )
    {
        send_to_char( "Goto where?\n\r", ch );
        return;
    }
 
    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    if (!room_is_private( location ))
    {
        send_to_char( "That room isn't private, use goto.\n\r", ch );
        return;
    }
 
    if ( ch->fighting != NULL )
        stop_fighting( ch, TRUE );
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
                act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
            else
                act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    char_from_room( ch );
    char_to_room( ch, location );
 
 
    for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if (get_trust(rch) >= ch->invis_level)
        {
            if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
                act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
            else
                act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
        }
    }
 
    do_function(ch, &do_look, "auto" );
    return;
}

/* RT to replace the 3 stat commands */

void do_stat ( CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  stat <name>\n\r",ch);
	send_to_char("  stat obj <name>\n\r",ch);
	send_to_char("  stat mob <name>\n\r",ch);
 	send_to_char("  stat room <number>\n\r",ch);
	return;
   }

   if (!str_cmp(arg,"room"))
   {
	do_function(ch, &do_rstat, string);
	return;
   }
  
   if (!str_cmp(arg,"obj"))
   {
	do_function(ch, &do_ostat, string);
	return;
   }

   if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob"))
   {
	do_function(ch, &do_mstat, string);
	return;
   }
   
   /* do it the old way */

  victim = get_char_world(ch,argument);
  if (victim != NULL)
  {
    do_function(ch, &do_mstat, argument);
    return;
  }
  
   obj = get_obj_world(ch,argument);
   if (obj != NULL)
   {
     do_function(ch, &do_ostat, argument);
     return;
   }

  location = find_location(ch,argument);
  if (location != NULL)
  {
    do_function(ch, &do_rstat, argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}

void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private( location ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char( "That room is private right now.\n\r", ch );
	return;
    }

    sprintf( buf, "Name: '%s'\n\rArea: '%s'\n\r",
	location->name,
	location->area->name );
    send_to_char( buf, ch );

    sprintf( buf,
	"Vnum: %d  Sector: %s  Light: %d  Healing: %d  Mana: %d\n\r",
	location->vnum,
	flag_string(sector_flags, location->sector_type),
	location->light,
	location->heal_rate,
	location->mana_rate );
    send_to_char( buf, ch );

    sprintf( buf,
	"Room flags: %s.\n\rDescription 1:\n\r%s\n\rDescription 2:\n\r%s\n\r",
	flag_string(room_flags, location->room_flags),
	location->description1,
	location->description2 );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );
	for ( ed = location->extra_descr; ed; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}
	send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
	if (can_see(ch,rch))
        {
	    send_to_char( " ", ch );
	    one_argument( rch->name, buf );
	    send_to_char( buf, ch );
	}
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
	send_to_char( " ", ch );
	one_argument( obj->name, buf );
	send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = location->exit[door] ) != NULL )
	{
	    sprintf( buf,
		"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

		door,
		(pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
	    	pexit->key,
	    	pexit->exit_info,
	    	pexit->keyword,
	    	pexit->description[0] != '\0'
		    ? pexit->description : "(none).\n\r" );
	    send_to_char( buf, ch );
	}
    }

    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat what?\n\r", ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
    {
	send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
	return;
    }

    sprintf( buf, "Name(s): %s\n\r",
	obj->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
	obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
	item_name(obj->item_type), obj->pIndexData->reset_num );
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
	obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\r",
	wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n\r",
	1,           get_obj_number( obj ),
	obj->weight, get_obj_weight( obj ),get_true_weight(obj) );
    send_to_char( buf, ch );

    sprintf( buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
	obj->level, obj->cost, obj->condition, obj->timer );
    send_to_char( buf, ch );

    sprintf( buf,
	"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
	obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	obj->carried_by == NULL    ? "(none)" : 
	    can_see(ch,obj->carried_by) ? obj->carried_by->name
				 	: "someone",
	obj->wear_loc );
    send_to_char( buf, ch );
    
    sprintf( buf, "Values: %d %d %d %d %d\n\r",
	obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	obj->value[4] );
    send_to_char( buf, ch );
    
    /* now give out vital statistics as per identify */
    
    switch ( obj->item_type )
    {
    	case ITEM_SCROLL: 
    	case ITEM_POTION:
    	case ITEM_PILL:
	    sprintf( buf, "Level %d spells of:", obj->value[0] );
	    send_to_char( buf, ch );

	    if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[1]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[2]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	    {
		send_to_char(" '",ch);
		send_to_char(skill_table[obj->value[4]].name,ch);
		send_to_char("'",ch);
	    }

	    send_to_char( ".\n\r", ch );
	break;

    	case ITEM_WAND: 
    	case ITEM_STAFF: 
	    sprintf( buf, "Has %d(%d) charges of level %d",
	    	obj->value[1], obj->value[2], obj->value[0] );
	    send_to_char( buf, ch );
      
	    if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
	    {
	    	send_to_char( " '", ch );
	    	send_to_char( skill_table[obj->value[3]].name, ch );
	    	send_to_char( "'", ch );
	    }

	    send_to_char( ".\n\r", ch );
	break;

	case ITEM_DRINK_CON:
	    sprintf(buf,"It holds %s-colored %s.\n\r",
		liq_table[obj->value[2]].liq_color,
		liq_table[obj->value[2]].liq_name);
	    send_to_char(buf,ch);
	    break;
		
      
    	case ITEM_WEAPON:
 	    send_to_char("Weapon type is ",ch);
	    switch (obj->value[0])
	    {
	    	case(WEAPON_EXOTIC): 
		    send_to_char("exotic\n\r",ch);
		    break;
	    	case(WEAPON_SWORD): 
		    send_to_char("sword\n\r",ch);
		    break;	
	    	case(WEAPON_DAGGER): 
		    send_to_char("dagger\n\r",ch);
		    break;
	    	case(WEAPON_SPEAR):
		    send_to_char("spear/staff\n\r",ch);
		    break;
	    	case(WEAPON_MACE): 
		    send_to_char("mace/club\n\r",ch);	
		    break;
	   	case(WEAPON_AXE): 
		    send_to_char("axe\n\r",ch);	
		    break;
	    	case(WEAPON_FLAIL): 
		    send_to_char("flail\n\r",ch);
		    break;
	    	case(WEAPON_WHIP): 
		    send_to_char("whip\n\r",ch);
		    break;
	    	case(WEAPON_POLEARM): 
		    send_to_char("polearm\n\r",ch);
		    break;
		case(WEAPON_LANCE):
		    send_to_char("lance\n\r",ch);
		    break;
	    	default: 
		    send_to_char("unknown\n\r",ch);
		    break;
 	    }
	    if (obj->pIndexData->new_format)
	    	sprintf(buf,"Damage is %dd%d (average %d)\n\r",
		    obj->value[1],obj->value[2],
		    (1 + obj->value[2]) * obj->value[1] / 2);
	    else
	    	sprintf( buf, "Damage is %d to %d (average %d)\n\r",
	    	    obj->value[1], obj->value[2],
	    	    ( obj->value[1] + obj->value[2] ) / 2 );
	    send_to_char( buf, ch );

	    sprintf(buf,"Damage noun is %s.\n\r",
		(obj->value[3] > 0 && obj->value[3] < MAX_DAMAGE_MESSAGE) ?
		    attack_table[obj->value[3]].noun : "undefined");
	    send_to_char(buf,ch);
	    
	    if (obj->value[4])  /* weapon flags */
	    {
	        sprintf(buf,"Weapons flags: %s\n\r",
		    weapon_bit_name(obj->value[4]));
	        send_to_char(buf,ch);
            }
	break;

    	case ITEM_ARMOR:
	    sprintf( buf, 
	    "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
	        obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	    send_to_char( buf, ch );
	break;

        case ITEM_CONTAINER:
            sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
                obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
            send_to_char(buf,ch);
            if (obj->value[4] != 100)
            {
                sprintf(buf,"Weight multiplier: %d%%\n\r",
		    obj->value[4]);
                send_to_char(buf,ch);
            }
        break;
    }


    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
	EXTRA_DESCR_DATA *ed;

	send_to_char( "Extra description keywords: '", ch );

	for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
	    	send_to_char( " ", ch );
	}

	for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
	{
	    send_to_char( ed->keyword, ch );
	    if ( ed->next != NULL )
		send_to_char( " ", ch );
	}

	send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char(buf,ch);
	if ( paf->duration > -1)
	    sprintf(buf,", %d hours.\n\r",paf->duration);
	else
	    sprintf(buf,".\n\r");
	send_to_char( buf, ch );
	if (paf->bitvector)
	{
	    switch(paf->where)
	    {
		case TO_AFFECTS: case TO_AFFECTS2:
		    sprintf(buf,"Adds %s affect.\n",
			affect_bit_name(paf->bitvector));
		    sprintf(buf,"Adds %s affect.\n",
			affect2_bit_name(paf->bitvector));
		    break;
                case TO_WEAPON:
                    sprintf(buf,"Adds %s weapon flags.\n",
                        weapon_bit_name(paf->bitvector));
		    break;
		case TO_OBJECT:
		    sprintf(buf,"Adds %s object flag.\n",
			extra_bit_name(paf->bitvector));
		    break;
		case TO_IMMUNE:
		    sprintf(buf,"Adds immunity to %s.\n",
			imm_bit_name(paf->bitvector));
		    break;
		case TO_RESIST:
		    sprintf(buf,"Adds resistance to %s.\n\r",
			imm_bit_name(paf->bitvector));
		    break;
		case TO_VULN:
		    sprintf(buf,"Adds vulnerability to %s.\n\r",
			imm_bit_name(paf->bitvector));
		    break;
		default:
		    sprintf(buf,"Unknown bit %d: %d\n\r",
			paf->where,paf->bitvector);
		    break;
	    }
	    send_to_char(buf,ch);
	}
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf, "Affects %s by %d, level %d.\n\r",
	    affect_loc_name( paf->location ), paf->modifier,paf->level );
	send_to_char( buf, ch );
        if (paf->bitvector)
        {
            switch(paf->where)
            {
                case TO_AFFECTS:
                    sprintf(buf,"Adds %s affect.\n",
                        affect_bit_name(paf->bitvector));
		    sprintf(buf,"Adds %s affect.\n",
			affect2_bit_name(paf->bitvector));
                    break;
                case TO_OBJECT:
                    sprintf(buf,"Adds %s object flag.\n",
                        extra_bit_name(paf->bitvector));
                    break;
                case TO_IMMUNE:
                    sprintf(buf,"Adds immunity to %s.\n",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_RESIST:
                    sprintf(buf,"Adds resistance to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                case TO_VULN:
                    sprintf(buf,"Adds vulnerability to %s.\n\r",
                        imm_bit_name(paf->bitvector));
                    break;
                default:
                    sprintf(buf,"Unknown bit %d: %d\n\r",
                        paf->where,paf->bitvector);
                    break;
            }
            send_to_char(buf,ch);
        }
    }

    return;
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Stat whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    send_to_char("{w----------------------------------------------------------------------{x\n\r", ch);
    sprintf( buf, "{yName: {C%-14s ", victim->name);
    send_to_char( buf, ch );
    if(!IS_NPC(victim))
    {
        sprintf(buf, "{yAge: {c%-3d {yPlayed: {c%-4d {yLastLevel: {c%-4d {yTimer: %-3d{x",
           get_age(victim),
           (int) (victim->played + current_time - victim->logon) / 3600,
           victim->pcdata->last_level,
           victim->timer);
        send_to_char(buf, ch);
    }
    
    send_to_char("\n\r", ch);

    sprintf( buf, 
	"{mVnum: {W%d  {mFormat: {D%s  {mRace: {D%s  {mGroup: {W%d  {mSex: {D%s  {mRoom: {W%d{x\n\r",
	IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
	race_table[victim->race].name,
	IS_NPC(victim) ? victim->group : 0, sex_table[victim->sex].name,
	victim->in_room == NULL    ?        0 : victim->in_room->vnum
	);
    send_to_char( buf, ch );
    
    

    if (IS_NPC(victim))
    {
	sprintf(buf,"{mCount: {W%d  {mKilled: {W%d\n\r",
	    victim->pIndexData->count,victim->pIndexData->killed);
	send_to_char(buf,ch);
    }
    else
    {
    	if( is_clan( victim ) )
    	{
    		sprintf( buf, "{mClan: {x%s%s {mRank: {D%s {x({W%d{x)\n\r",
    			clan_table[victim->clan].who_name,
    			(victim->subclan) ? subclan_table[victim->subclan].who_name : "",
    			clanrank_name( victim ),
    			victim->clanlevel );
    		send_to_char( buf, ch );
    	}
    		
    	sprintf( buf, "{mGod: {D%s  {mRecall: {W%d  {mQP: {W%d{x\n\r",
    		god_table[victim->god].name,
    		(victim->recall) ? victim->recall : is_clan( victim ) ? clan_table[victim->clan].recall : god_table[victim->god].recall,
    		victim->questpoints );
    	send_to_char( buf, ch );
    }
    		

    sprintf( buf, 
   	"{mStr: {r%d{x({R%d{x)  {mInt: {y%d{x({Y%d{x)  {mWis: {g%d{x({G%d{x)  {mDex: {b%d{x({B%d{x)  {mCon: %d{x({M%d{x)\n\r",
	victim->perm_stat[STAT_STR],
	get_curr_stat(victim,STAT_STR),
	victim->perm_stat[STAT_INT],
	get_curr_stat(victim,STAT_INT),
	victim->perm_stat[STAT_WIS],
	get_curr_stat(victim,STAT_WIS),
	victim->perm_stat[STAT_DEX],
	get_curr_stat(victim,STAT_DEX),
	victim->perm_stat[STAT_CON],
	get_curr_stat(victim,STAT_CON) );
    send_to_char( buf, ch );

    sprintf( buf, "{mHp: {r%d{x/{R%d  {mMana: {b%d{x/{B%d  {mMove: {g%d{x/{G%d{x  {mPractices: {W%d{x\n\r",
	victim->hit,         victim->max_hit,
	victim->mana,        victim->max_mana,
	victim->move,        victim->max_move,
	IS_NPC(ch) ? 0 : victim->practice );
    send_to_char( buf, ch );
	
    sprintf( buf,
	"{mLvl: {W%d  {mClass: {D%s  {mAlign: {W%d  {mSteel: {W%ld  {mGold: {W%ld  {mExp: {W%d{x\n\r",
	victim->level,       
	IS_NPC(victim) ? "mobile" : class_table[victim->class].name,            
	victim->alignment,
	victim->steel, victim->gold, victim->exp );
    send_to_char( buf, ch );

    if(!IS_NPC(victim))
    {
        sprintf(buf,"{mBank: {cSteel: {W%ld {cGold: {W%ld{x", victim->pcdata->banksteel, victim->pcdata->bankgold);
        send_to_char( buf, ch );
        sprintf(buf, "  {mSafe Boxes: {cUsed: {W%d {cAvailable: {W%d{x\n\r", count_box(victim), victim->pcdata->safeboxes);
        send_to_char( buf, ch );
    }

    sprintf(buf,"{mArmor: {cpierce: {W%d  {cbash: {W%d  {cslash: {W%d  {cmagic: {W%d{x\n\r",
	    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
	    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));
    send_to_char(buf,ch);

    sprintf( buf, 
	"{mHit: {W%d  {mDam: {W%d  {mSaves: {W%d  {mSize: {D%s  {mPosition: {D%s  {mWimpy: {W%d{x\n\r",
	GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
	size_table[victim->size].name, position_table[victim->position].name,
	victim->wimpy );
    send_to_char( buf, ch );

    if (IS_NPC(victim) && victim->pIndexData->new_format)
    {
	sprintf(buf, "{mDamage: {W%dd%d  {mMessage:  {D%s{x\n\r",
	    victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE],
	    attack_table[victim->dam_type].noun);
	send_to_char( buf, ch );
    }
    sprintf( buf, "{mFighting: {D%s{x\n\r",
	victim->fighting ? victim->fighting->name : "(none)" );
    send_to_char( buf, ch );

    if ( !IS_NPC(victim) )
    {
	sprintf( buf,
	    "{mThirst: {W%d  {mHunger: {W%d  {mFull: {W%d  {mDrunk: {W%d{x\n\r",
	    victim->pcdata->condition[COND_THIRST],
	    victim->pcdata->condition[COND_HUNGER],
	    victim->pcdata->condition[COND_FULL],
	    victim->pcdata->condition[COND_DRUNK] );
	send_to_char( buf, ch );
    }

    sprintf( buf, "{mCarry number: {W%d  {mCarry weight: {W%ld\n\r",
	victim->carry_number, get_carry_weight(victim) / 10 );
    send_to_char( buf, ch );


    if (!IS_NPC(victim))
    {
    	sprintf( buf, 
	    "{mAge: {W%d  {mPlayed: {W%d  {mLast Level: {W%d  {mTimer: {W%d{x\n\r",
	    get_age(victim), 
	    (int) (victim->played + current_time - victim->logon) / 3600, 
	    victim->pcdata->last_level, 
	    victim->timer );
    	send_to_char( buf, ch );
    }

    sprintf(buf, "{mAct: {D%s{x\n\r",act_bit_name(victim->act));
    send_to_char(buf,ch);
    
    sprintf( buf, "{mAct2: {D%s{x\n\r", act2_bit_name( victim->act2 ) );
    send_to_char( buf, ch );
    
    if(!IS_NPC(victim) && IS_IMMORTAL(victim))
    {
        sprintf(buf, "{mImmflag: {D%s{x\n\r", immf_bit_name (victim->pcdata->immflag) );
        send_to_char( buf, ch );
    }
    
    if (victim->comm)
    {
    	sprintf(buf,"{mComm: {D%s{x\n\r",comm_bit_name(victim->comm));
    	send_to_char(buf,ch);
    }

    if (IS_NPC(victim) && victim->off_flags)
    {
    	sprintf(buf, "{mOffense: {D%s{x\n\r",off_bit_name(victim->off_flags));
	send_to_char(buf,ch);
    }

    if (victim->imm_flags)
    {
	sprintf(buf, "{mImmune: {D%s{x\n\r",imm_bit_name(victim->imm_flags));
	send_to_char(buf,ch);
    }
 
    if (victim->res_flags)
    {
	sprintf(buf, "{mResist: {D%s{x\n\r", imm_bit_name(victim->res_flags));
	send_to_char(buf,ch);
    }

    if (victim->vuln_flags)
    {
	sprintf(buf, "{mVulnerable: {D%s{x\n\r", imm_bit_name(victim->vuln_flags));
	send_to_char(buf,ch);
    }

    sprintf(buf, "{mForm: {D%s{x\n\r{mParts: {D%s{x\n\r", 
	form_bit_name(victim->form), part_bit_name(victim->parts));
    send_to_char(buf,ch);

    if (victim->affected_by)
    {
	sprintf(buf, "{mAffected by {D%s{x\n\r", 
	    affect_bit_name(victim->affected_by));
	send_to_char(buf,ch);
    }

    if (victim->affected2_by)
    {
	sprintf(buf, "{mAffected by {D%s{x\n\r",
	    affect2_bit_name(victim->affected2_by));
	send_to_char(buf,ch);
    }

    sprintf( buf, "{mMaster: {D%s{x  {mLeader: {D%s{x  {mPet: {D%s{x\n\r",
	victim->master      ? victim->master->name   : "(none)",
	victim->leader      ? victim->leader->name   : "(none)",
	victim->pet 	    ? victim->pet->name	     : "(none)");
    send_to_char( buf, ch );

    if (!IS_NPC(victim))
    {
	sprintf( buf, "{mSecurity: {W%d.{x {mTrust: {W%d.{x\n\r", victim->pcdata->security, victim->trust );	/* OLC */
	send_to_char( buf, ch );					/* OLC */
    }
    
    if(!IS_NPC(victim))
    {
    	sprintf(buf, "{mHost: {W%s{x\n\r", (victim->pcdata->orig_host[0] != '0') ? victim->pcdata->orig_host : victim->desc->host);
    	send_to_char(buf, ch);
    }

    sprintf( buf, "{mShort description: {x%s\n\r{mLong  description: {x%s",
	victim->short_descr,
	victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
    send_to_char( buf, ch );

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
    {
	sprintf(buf,"{mMobile has special procedure {D%s.{x\n\r",
		spec_name(victim->spec_fun));
	send_to_char(buf,ch);
    }

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
	sprintf( buf,"{mSpell: '{D%s{m' modifies {D%s {mby {W%d {mfor {W%d {mhours with bits {D%s{m & {D%s{m, level {W%d.{x\n\r",skill_table[(int) paf->type].name,affect_loc_name( paf->location ),paf->modifier,paf->duration, affect_bit_name( paf->bitvector ),affect2_bit_name( paf->bitvector ),paf->level);
	send_to_char( buf, ch );
    }

    return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument,arg);
 
    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  vnum obj <name>\n\r",ch);
	send_to_char("  vnum mob <name>\n\r",ch);
	send_to_char("  vnum skill <skill or spell>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_function(ch, &do_ofind, string);
 	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    { 
	do_function(ch, &do_mfind, string);
	return;
    }

    if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell"))
    {
	do_function (ch, &do_slookup, string);
	return;
    }
    /* do both */
    do_function(ch, &do_mfind, argument);
    do_function(ch, &do_ofind, argument);
}


void do_mfind( CHAR_DATA *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find whom?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pMobIndex->player_name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pMobIndex->vnum, pMobIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No mobiles by that name.\n\r", ch );

    return;
}



void do_ofind( CHAR_DATA *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Find what?\n\r", ch );
	return;
    }

    fAll	= FALSE; /* !str_cmp( arg, "all" ); */
    found	= FALSE;
    nMatch	= 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
	if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	{
	    nMatch++;
	    if ( fAll || is_name( argument, pObjIndex->name ) )
	    {
		found = TRUE;
		sprintf( buf, "[%5d] %s\n\r",
		    pObjIndex->vnum, pObjIndex->short_descr );
		send_to_char( buf, ch );
	    }
	}
    }

    if ( !found )
	send_to_char( "No objects by that name.\n\r", ch );

    return;
}


void do_owhere(CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf();

    if (argument[0] == '\0')
    {
	send_to_char("Find what?\n\r",ch);
	return;
    }
 
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || !is_name( argument, obj->name )
        ||   ch->level < obj->level)
            continue;
 
        found = TRUE;
        number++;
 
        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
            ;
 
        if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
	&&   in_obj->carried_by->in_room != NULL)
            sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
                number, obj->short_descr,PERS(in_obj->carried_by, ch),
		in_obj->carried_by->in_room->vnum );
        else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
            sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
                number, obj->short_descr,in_obj->in_room->name, 
	   	in_obj->in_room->vnum);
	else
            sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr);
 
        buf[0] = UPPER(buf[0]);
        add_buf(buffer,buf);
 
        if (number >= max_found)
            break;
    }
 
    if ( !found )
        send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
        page_to_char(buf_string(buffer),ch);

    free_buf(buffer);
}

/*
 * do_mwhere changed by Zanthras of Mystical Realities MUD
 * so you can do "mwhere <mob> area" to look for mobs only
 * in the current area.
 * Also added ability to do "mwhere all area" to list
 * all mobs in the current area.
 */
void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    found = FALSE;
    buffer = new_buf();

    if ( arg1[0] == '\0' )
    {
	DESCRIPTOR_DATA *d;

	/* show characters logged */

	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->character != NULL && d->connected == CON_PLAYING
	    &&  d->character->in_room != NULL && can_see(ch,d->character)
	    &&  can_see_room(ch,d->character->in_room))
	    {
		victim = d->character;
		count++;
		if (d->original != NULL)
		    sprintf(buf,"%3d) %s (in the body of %s) is in %s [%d]\n\r",
			count, d->original->name,victim->short_descr,
			victim->in_room->name,victim->in_room->vnum);
		else
		    sprintf(buf,"%3d) %s is in %s [%d]\n\r",
			count, victim->name,victim->in_room->name,
			victim->in_room->vnum);
		add_buf(buffer,buf);
	    }
	}

        page_to_char(buf_string(buffer),ch);
	free_buf(buffer);
	return;
    }
    else if(arg2[0] == '\0' )
    {
      for ( victim = char_list; victim != NULL; victim = victim->next )
      {
	if ( victim->in_room != NULL
	&&   is_name( arg1, victim->name ) )
	{
	    found = TRUE;
	    count++;
	    sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		IS_NPC(victim) ? victim->short_descr : victim->name,
		victim->in_room->vnum,
		victim->in_room->name );
	    add_buf(buffer,buf);
	}
      }
    }
    else if(!str_cmp(arg1,"all") && !str_cmp(arg2,"area"))
    {
      found = FALSE;
      buffer = new_buf();
      for ( victim = char_list; victim != NULL; victim = victim->next )
      {
	if ( victim->in_room != NULL
        &&   victim->in_room->area == ch->in_room->area
	&&   IS_NPC(victim) )
	{
	    found = TRUE;
	    count++;
	    sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		IS_NPC(victim) ? victim->short_descr : victim->name,
		victim->in_room->vnum,
		victim->in_room->name );
	    add_buf(buffer,buf);
	}
      }
    }
    else if(!str_cmp(arg2,"area"))
    {
      found = FALSE;
      buffer = new_buf();
      for ( victim = char_list; victim != NULL; victim = victim->next )
      {
	if ( victim->in_room != NULL
        &&   victim->in_room->area == ch->in_room->area
	&&   is_name( arg1, victim->name ) )
	{
	    found = TRUE;
	    count++;
	    sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		IS_NPC(victim) ? victim->short_descr : victim->name,
		victim->in_room->vnum,
		victim->in_room->name );
	    add_buf(buffer,buf);
	}
      }
    }



    if ( !found )
    {     
      if ( !str_cmp(arg2,"area") )
         act("You didn't find any $T in this area.", ch, NULL, arg1, TO_CHAR );
      else
	 act("You didn't find any $T.", ch, NULL, arg1, TO_CHAR );
    }
    else
    {
    	page_to_char(buf_string(buffer),ch);
    }
    free_buf(buffer);

    return;
}

void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}



void do_reboot( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;

    if (ch->invis_level < LEVEL_HERO)
    {
    	sprintf( buf, "Reboot by %s.", ch->name );
    	do_function(ch, &do_echo, buf );
    }

    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;
	if (vch != NULL)
	    save_char_obj(vch);
    	close_socket(d);
    }
    
    return;
}

void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}

void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;
    CHAR_DATA *vch;

    if (ch->invis_level < LEVEL_HERO)
    sprintf( buf, "Shutdown by %s.", ch->name );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    if (ch->invis_level < LEVEL_HERO)
    {
    	do_function(ch, &do_echo, buf );
    }
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next)
    {
	d_next = d->next;
	vch = d->original ? d->original : d->character;
	if (vch != NULL)
	    save_char_obj(vch);
	close_socket(d);
    }
    return;
}

void do_protect( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;

    if (argument[0] == '\0')
    {
	send_to_char("Protect whom from snooping?\n\r",ch);
	return;
    }

    if ((victim = get_char_world(ch,argument)) == NULL)
    {
	send_to_char("You can't find them.\n\r",ch);
	return;
    }

    if (IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
	act_new("$N is no longer snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
	send_to_char("Your snoop-proofing was just removed.\n\r",victim);
	REMOVE_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
    else
    {
	act_new("$N is now snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
	send_to_char("You are now immune to snooping.\n\r",victim);
	SET_BIT(victim->comm,COMM_SNOOP_PROOF);
    }
}
  


void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Snoop whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL )
    {
	send_to_char( "No descriptor to snoop.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Cancelling all snoops.\n\r", ch );
	wiznet("$N stops being such a snoop.",
		ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == ch->desc )
		d->snoop_by = NULL;
	}
	return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
	send_to_char( "Busy already.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        send_to_char("That character is in a private room.\n\r",ch);
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) 
    ||   IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( ch->desc != NULL )
    {
	for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
	{
	    if ( d->character == victim || d->original == victim )
	    {
		send_to_char( "No snoop loops.\n\r", ch );
		return;
	    }
	}
    }

    victim->desc->snoop_by = ch->desc;
    sprintf(buf,"$N starts snooping on %s",
	(IS_NPC(ch) ? victim->short_descr : victim->name));
    wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
	send_to_char( "Switch into whom?\n\r", ch );
	return;
    }

    if ( ch->desc == NULL )
	return;
    
    if ( ch->desc->original != NULL )
    {
	send_to_char( "You are already switched.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can only switch into mobiles.\n\r",ch);
	return;
    }

    if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
    &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
	send_to_char("That character is in a private room.\n\r",ch);
	return;
    }

    if ( victim->desc != NULL )
    {
	send_to_char( "Character in use.\n\r", ch );
	return;
    }

    sprintf(buf,"$N switches into %s",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    /* change communications to match */
    if (ch->prompt != NULL)
        victim->prompt = str_dup(ch->prompt);
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    send_to_char( "Ok.\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->desc == NULL )
	return;

    if ( ch->desc->original == NULL )
    {
	send_to_char( "You aren't switched.\n\r", ch );
	return;
    }

    send_to_char( 
"You return to your original body. Type replay to see any missed tells.\n\r", 
	ch );
    if (ch->prompt != NULL)
    {
	free_string(ch->prompt);
	ch->prompt = NULL;
    }

    sprintf(buf,"$N returns from %s.",ch->short_descr);
    wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc; 
    ch->desc                  = NULL;
    return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (IS_TRUSTED(ch,GOD)
	|| (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
	|| (IS_TRUSTED(ch,DEMIGOD)  && obj->level <= 10 && obj->cost <= 500)
	|| (IS_TRUSTED(ch,BUILDER)  && obj->level <=  5 && obj->cost <= 250)
	|| (IS_TRUSTED(ch,AVATAR)   && obj->level ==  0 && obj->cost <= 100))
	return TRUE;
    else
	return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
    OBJ_DATA *c_obj, *t_obj;


    for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
	if (obj_check(ch,c_obj))
	{
	    t_obj = create_object(c_obj->pIndexData,0);
	    clone_object(c_obj,t_obj);
	    obj_to_obj(t_obj,clone);
	    recursive_clone(ch,c_obj,t_obj);
	}
    }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA  *obj;

    rest = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Clone what?\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	mob = NULL;
	obj = get_obj_here(ch,rest);
	if (obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	obj = NULL;
	mob = get_char_room(ch,rest);
	if (mob == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }
    else /* find both */
    {
	mob = get_char_room(ch,argument);
	obj = get_obj_here(ch,argument);
	if (mob == NULL && obj == NULL)
	{
	    send_to_char("You don't see that here.\n\r",ch);
	    return;
	}
    }

    /* clone an object */
    if (obj != NULL)
    {
	OBJ_DATA *clone;

	if (!obj_check(ch,obj))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_object(obj->pIndexData,0); 
	clone_object(obj,clone);
	if (obj->carried_by != NULL)
	    obj_to_char(clone,ch);
	else
	    obj_to_room(clone,ch->in_room);
 	recursive_clone(ch,obj,clone);

	act("$n has created $p.",ch,clone,NULL,TO_ROOM);
	act("You clone $p.",ch,clone,NULL,TO_CHAR);
	wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
	return;
    }
    else if (mob != NULL)
    {
	CHAR_DATA *clone;
	OBJ_DATA *new_obj;
	char buf[MAX_STRING_LENGTH];

	if (!IS_NPC(mob))
	{
	    send_to_char("You can only clone mobiles.\n\r",ch);
	    return;
	}

	if ((mob->level > 20 && !IS_TRUSTED(ch,GOD))
	||  (mob->level > 10 && !IS_TRUSTED(ch,IMMORTAL))
	||  (mob->level >  5 && !IS_TRUSTED(ch,DEMIGOD))
	||  (mob->level >  0 && !IS_TRUSTED(ch,BUILDER))
	||  !IS_TRUSTED(ch,AVATAR))
	{
	    send_to_char(
		"Your powers are not great enough for such a task.\n\r",ch);
	    return;
	}

	clone = create_mobile(mob->pIndexData);
	clone_mobile(mob,clone); 
	
	for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	    if (obj_check(ch,obj))
	    {
		new_obj = create_object(obj->pIndexData,0);
		clone_object(obj,new_obj);
		recursive_clone(ch,obj,new_obj);
		obj_to_char(new_obj,clone);
		new_obj->wear_loc = obj->wear_loc;
	    }
	}
	char_to_room(clone,ch->in_room);
        act("$n has created $N.",ch,NULL,clone,TO_ROOM);
        act("You clone $N.",ch,NULL,clone,TO_CHAR);
	sprintf(buf,"$N clones %s.",clone->short_descr);
	wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
        return;
    }
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  load mob <vnum>\n\r",ch);
	send_to_char("  load obj <vnum> <level>\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
	do_function(ch, &do_mload, argument);
	return;
    }

    if (!str_cmp(arg,"obj"))
    {
	do_function(ch, &do_oload, argument);
	return;
    }
    /* echo syntax */
    do_function(ch, &do_load, "");
}


void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    
    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
	send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
	return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
	send_to_char( "No mob has that vnum.\n\r", ch );
	return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    sprintf(buf,"$N loads %s.",victim->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;
    
    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
	send_to_char( "Syntax: load obj <vnum> <level>.\n\r", ch );
	return;
    }
    
    level = get_trust(ch); /* default */
  
    if ( arg2[0] != '\0')  /* load with a level */
    {
	if (!is_number(arg2))
        {
	  send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
	  return;
	}
        level = atoi(arg2);
        if (level < 0 || level > get_trust(ch))
	{
	  send_to_char( "Level must be be between 0 and your level.\n\r",ch);
  	  return;
	}
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
	send_to_char( "No object has that vnum.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
	obj_to_char( obj, ch );
    else
	obj_to_room( obj, ch->in_room );
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	/* 'purge' */
	CHAR_DATA *vnext;
	OBJ_DATA  *obj_next;

	for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
	{
	    vnext = victim->next_in_room;
	    if ( IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE) 
	    &&   victim != ch /* safety precaution */ )
		extract_char( victim, TRUE );
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
	      extract_obj( obj );
	}

	act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) )
    {

	if (ch == victim)
	{
	  send_to_char("Ho ho ho.\n\r",ch);
	  return;
	}

	if (get_trust(ch) <= get_trust(victim))
	{
	  send_to_char("Maybe that wasn't a good idea...\n\r",ch);
	  sprintf(buf,"%s tried to purge you!\n\r",ch->name);
	  send_to_char(buf,victim);
	  return;
	}

	act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);

    	if (victim->level > 1)
	    save_char_obj( victim );
    	d = victim->desc;
    	extract_char( victim, TRUE );
    	if ( d != NULL )
          close_socket( d );

	return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}



void do_advance( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL )
    {
	sprintf(buf,"Level must be 1 to %d.\n\r", MAX_LEVEL);
	send_to_char(buf, ch);
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust level.\n\r", ch );
	return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= victim->level )
    {
        int temp_prac;

	send_to_char( "Lowering a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim );
	temp_prac = victim->practice;
	victim->level    = 1;
	victim->exp      = exp_per_level(victim,victim->pcdata->points);
	victim->max_hit  = 10;
	victim->max_mana = 100;
	victim->max_move = 100;
	victim->practice = 0;
	victim->hit      = victim->max_hit;
	victim->mana     = victim->max_mana;
	victim->move     = victim->max_move;
	advance_level( victim, TRUE );
	victim->practice = temp_prac;
    }
    else
    {
	send_to_char( "Raising a player's level!\n\r", ch );
	send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
    }

    for ( iLevel = victim->level ; iLevel < level; iLevel++ )
    {
	victim->level += 1;
	advance_level( victim,TRUE);
    }
    sprintf(buf,"You are now level %d.\n\r",victim->level);
    send_to_char(buf,victim);
    victim->exp   = exp_per_level(victim,victim->pcdata->points) 
		  * UMAX( 1, victim->level );
    victim->trust = 0;
    save_char_obj(victim);
    return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
	send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "That player is not here.\n\r", ch);
	return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
    {
	sprintf(buf, "Level must be 0 (reset) or 1 to %d.\n\r",MAX_LEVEL);
	send_to_char(buf, ch);
	return;
    }

    if ( level > get_trust( ch ) )
    {
	send_to_char( "Limited to your trust.\n\r", ch );
	return;
    }

    victim->trust = level;
    return;
}



void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */
    	
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
        {
            affect_strip(vch,gsn_plague);
            affect_strip(vch,gsn_poison);
            affect_strip(vch,gsn_blindness);
            affect_strip(vch,gsn_sleep);
            affect_strip(vch,gsn_curse);
            
            vch->hit 	= vch->max_hit;
            vch->mana	= vch->max_mana;
            vch->move	= vch->max_move;
            
            if(!IS_NPC(vch))
            {
            	if(vch->pcdata->condition[COND_HUNGER] < 50)
            	    vch->pcdata->condition[COND_HUNGER] = 50;
            	if(vch->pcdata->condition[COND_THIRST] < 50)
            	    vch->pcdata->condition[COND_THIRST] = 50;
            }
            update_pos( vch);
            act("$n has restored you.",ch,NULL,vch,TO_VICT);
        }

        sprintf(buf,"$N restored room %d.",ch->in_room->vnum);
        wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
        
        send_to_char("Room restored.\n\r",ch);
        return;

    }
    
    if ( get_trust(ch) >=  MAX_LEVEL - 1 && !str_cmp(arg,"all"))
    {
    /* cure all */
    	
        for (d = descriptor_list; d != NULL; d = d->next)
        {
	    victim = d->character;

	    if (victim == NULL || IS_NPC(victim))
		continue;
                
            affect_strip(victim,gsn_plague);
            affect_strip(victim,gsn_poison);
            affect_strip(victim,gsn_blindness);
            affect_strip(victim,gsn_sleep);
            affect_strip(victim,gsn_curse);
            
            victim->hit 	= victim->max_hit;
            victim->mana	= victim->max_mana;
            victim->move	= victim->max_move;
            if(!IS_NPC(victim))
            {
            	if(victim->pcdata->condition[COND_HUNGER] < 50)
            	    victim->pcdata->condition[COND_HUNGER] = 50;
            	if(victim->pcdata->condition[COND_THIRST] < 50)
            	    victim->pcdata->condition[COND_THIRST] = 50;
            }
            update_pos( victim);
	    if (victim->in_room != NULL)
                act("$n has restored you.",ch,NULL,victim,TO_VICT);
        }
	send_to_char("All active players restored.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    affect_strip(victim,gsn_plague);
    affect_strip(victim,gsn_poison);
    affect_strip(victim,gsn_blindness);
    affect_strip(victim,gsn_sleep);
    affect_strip(victim,gsn_curse);
    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    if(!IS_NPC(victim))
    {
    	if(victim->pcdata->condition[COND_HUNGER] < 50)
            victim->pcdata->condition[COND_HUNGER] = 50;
        if(victim->pcdata->condition[COND_THIRST] < 50)
            victim->pcdata->condition[COND_THIRST] = 50;
    }
    update_pos( victim );
    act( "$n has restored you.", ch, NULL, victim, TO_VICT );
    sprintf(buf,"$N restored %s",
	IS_NPC(victim) ? victim->short_descr : victim->name);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
    send_to_char( "Ok.\n\r", ch );
    return;
}

 	
void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Freeze whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) > get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
	REMOVE_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can play again.\n\r", victim );
	send_to_char( "FREEZE removed.\n\r", ch );
	sprintf(buf,"$N thaws %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->act, PLR_FREEZE);
	send_to_char( "You can't do ANYthing!\n\r", victim );
	send_to_char( "FREEZE set.\n\r", ch );
	sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    save_char_obj( victim );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Log whom?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	if ( fLogAll )
	{
	    fLogAll = FALSE;
	    send_to_char( "Log ALL off.\n\r", ch );
	}
	else
	{
	    fLogAll = TRUE;
	    send_to_char( "Log ALL on.\n\r", ch );
	}
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
	REMOVE_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
	SET_BIT(victim->act, PLR_LOG);
	send_to_char( "LOG set.\n\r", ch );
    }

    return;
}

void do_nonote(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    
    one_argument(argument, arg);
    
    if(arg[0] == '\0')
    {
        send_to_char("Nonote whom?\n\r", ch);
        return;
    }
    
    if((victim = get_char_world(ch, arg)) == NULL)
    {
        send_to_char("That person is not here.\n\r", ch);
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NONOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NONOTE);
	send_to_char( "You can send notes again.\n\r", victim );
	send_to_char( "NONOTE removed.\n\r", ch );
	sprintf(buf,"$N restores notes to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NONOTE);
	send_to_char( "Your note writing privileges have been revoked!\n\r", victim );
	send_to_char( "NONOTE set.\n\r", ch );
	sprintf(buf,"$N revokes %s's notes.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
}

void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noemote whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }


    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
	REMOVE_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can emote again.\n\r", victim );
	send_to_char( "NOEMOTE removed.\n\r", ch );
	sprintf(buf,"$N restores emotes to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOEMOTE);
	send_to_char( "You can't emote!\n\r", victim );
	send_to_char( "NOEMOTE set.\n\r", ch );
	sprintf(buf,"$N revokes %s's emotes.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_noshout( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Noshout whom?\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOSHOUT) )
    {
	REMOVE_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can shout again.\n\r", victim );
	send_to_char( "NOSHOUT removed.\n\r", ch );
	sprintf(buf,"$N restores shouts to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOSHOUT);
	send_to_char( "You can't shout!\n\r", victim );
	send_to_char( "NOSHOUT set.\n\r", ch );
	sprintf(buf,"$N revokes %s's shouts.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Notell whom?", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
	REMOVE_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can tell again.\n\r", victim );
	send_to_char( "NOTELL removed.\n\r", ch );
	sprintf(buf,"$N restores tells to %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }
    else
    {
	SET_BIT(victim->comm, COMM_NOTELL);
	send_to_char( "You can't tell!\n\r", victim );
	send_to_char( "NOTELL set.\n\r", ch );
	sprintf(buf,"$N revokes %s's tells.",victim->name);
	wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
    }

    return;
}



void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch->fighting != NULL )
	    stop_fighting( rch, TRUE );
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	    REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern char wlreason[MAX_INPUT_LENGTH];
    extern bool wizlock;
    char buf[MAX_INPUT_LENGTH];
    wizlock = !wizlock;
    wlreason[0] = '\0';

    if ( wizlock )
    {
    	if(argument[0] != '\0')
    	{
    	    strcpy(wlreason, argument);
    	    strcat(wlreason, "\n\r");
    	}
	wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
	if(argument[0] != '\0')
	{
	    sprintf(buf, "Reason: %s", argument);
	    wiznet(buf,ch,NULL,0,0,0);
	}
	send_to_char( "Game wizlocked.\n\r", ch );
    }
    else
    {
	wiznet("$N removes wizlock.",ch,NULL,0,0,0);
	send_to_char( "Game un-wizlocked.\n\r", ch );
    }

    return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA *ch, char *argument )
{
    extern bool newlock;
    extern char nlreason[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    newlock = !newlock;
    nlreason[0] = '\0';
 
    if ( newlock )
    {
    	if(argument[0] != '\0')
    	{
    	    strcpy(nlreason, argument);
    	    strcat(nlreason, "\n\r");
    	}
	wiznet("$N locks out new characters.",ch,NULL,0,0,0);
	if(argument[0] != '\0')
	{
	    sprintf(buf, "Reason: %s", argument);
	    wiznet(buf,ch,NULL,0,0,0);
        }
        send_to_char( "New characters have been locked out.\n\r", ch );
    }
    else
    {
	wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
        send_to_char( "Newlock removed.\n\r", ch );
    }
 
    return;
}


void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Lookup which skill or spell?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		sn, skill_table[sn].slot, skill_table[sn].name );
	    send_to_char( buf, ch );
	}
    }
    else
    {
	if ( ( sn = skill_lookup( arg ) ) < 0 )
	{
	    send_to_char( "No such skill or spell.\n\r", ch );
	    return;
	}

	sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
	    sn, skill_table[sn].slot, skill_table[sn].name );
	send_to_char( buf, ch );
    }

    return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set mob   <name> <field> <value>\n\r",ch);
	send_to_char("  set obj   <name> <field> <value>\n\r",ch);
	send_to_char("  set room  <room> <field> <value>\n\r",ch);
        send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
    {
	do_function(ch, &do_mset, argument);
	return;
    }

    if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell"))
    {
	do_function(ch, &do_sset, argument);
	return;
    }

    if (!str_prefix(arg,"object"))
    {
	do_function(ch, &do_oset, argument);
	return;
    }

    if (!str_prefix(arg,"room"))
    {
	do_function(ch, &do_rset, argument);
	return;
    }
    /* echo syntax */
    do_function(ch, &do_set, "");
}


void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch);
	send_to_char( "  set skill <name> all <value>\n\r",ch);  
	send_to_char("   (use the name of the skill, not the number)\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
	send_to_char( "No such skill or spell.\n\r", ch );
	return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
	send_to_char( "Value range is 0 to 100.\n\r", ch );
	return;
    }

    if ( fAll )
    {
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name != NULL )
		victim->pcdata->learned[sn]	= value;
	}
    }
    else
    {
	victim->pcdata->learned[sn] = value;
    }

    return;
}


void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set char <name> <field> <value>\n\r",ch); 
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    str int wis dex con sex class level god\n\r",ch );
	send_to_char( "    race group steel gold hp mana move prac\n\r",ch);
	send_to_char( "    align train thirst hunger drunk full\n\r",	ch );
	send_to_char( "    security hitroll damroll clanlvl subclan\n\r",ch );
	send_to_char( "    questp recall clanrank safeleft hours\n\r", ch );
	send_to_char( "    bsteel bgold\n\r", ch);
	return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    /* clear zones for mobs */
    victim->zone = NULL;

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */
    if(!str_prefix(arg2,"bounty"))
    {
   	if(value < 0)
    	{
    	    send_to_char("Value must be numeric.\n\r", ch);
    	    return;
    	}
    	
    	if(IS_NPC(victim))
    	{
    	    send_to_char("Not on NPCs.\n\r", ch);
    	    return;
    	}
    	
    	victim->pcdata->bounty = value;
    	sprintf(buf, "%s's bounty is now, %d steel.\n\r", victim->name,
    	value);
    	return;
     }
    
    if(!str_prefix(arg2,"bsteel"))
    {
   	if(value < 0)
    	{
    	    send_to_char("Value must be numeric.\n\r", ch);
    	    return;
    	}
    	
    	if(IS_NPC(victim))
    	{
    	    send_to_char("Not on NPCs.\n\r", ch);
    	    return;
    	}
    	
    	victim->pcdata->banksteel = value;
    	sprintf(buf, "%s's steel count in the back set to %d.\n\r", victim->name,
    	value);
    	return;
     }
     
    if(!str_prefix(arg2,"bgold"))
    {
   	if(value < 0)
    	{
    	    send_to_char("Value must be numeric.\n\r", ch);
    	    return;
    	}
    	
    	if(IS_NPC(victim))
    	{
    	    send_to_char("Not on NPCs.\n\r", ch);
    	    return;
    	}
    	
    	victim->pcdata->bankgold = value;
    	sprintf(buf, "%s's gold count in the back set to %d.\n\r", victim->name,
    	value);
    	return;
     }
     
    if (!str_prefix(arg2,"hours"))
    {
    	if(value < 0)
    	{
    	    send_to_char("Value must be numeric.\n\r", ch);
    	    return;
    	}
    	
    	if(IS_NPC(victim))
    	{
    	    send_to_char("Not on NPCs.\n\r", ch);
    	    return;
    	}
    	
    	victim->played = value * 3600 - (current_time - ch->logon);
    	send_to_char("Hours set.\n\r", ch);
    	return;
    } 
 
    if (!str_prefix(arg2,"safeleft"))
    {
        /* maxsafe = safe boxes availble + safes used = count_box(victim) */
        /* safe available = victim->pcdata->safeboxes */
        /* safe used = count_box(victim) - victim->pcdata->safeboxes */
        
        if(value < 0)
        {
            send_to_char("Must be a number and at least 0.\n\r", ch);
            return;
        }
        
        if(IS_NPC(victim))
        {
            send_to_char("Not on NPCs.\n\r", ch);
            return;
        }
        
        if(value < count_box(victim) - victim->pcdata->safeboxes)
        {
            send_to_char("You can't do that.\n\r", ch);
            return;
        }
        
        victim->pcdata->safeboxes = value;
        send_to_char("Value successfully set.\n\r", ch);
        return;
    }
    if (!str_prefix(arg2,"recall"))
    {
        int vnum;
        vnum = atoi(arg3);
        /* Make sure the recall room exists! */
        if(get_room_index(vnum) == NULL && vnum != 0)
        {
            send_to_char("That room does not exist!\n\r", ch);
            return;
        }
        
        if(!IS_NPC(victim))
            victim->recall = vnum;
        else
            send_to_char("Not on NPCs.\n\r", ch);
        
        return;
    }
    
    if (!str_prefix(arg2,"god"))
    {
        int god;

        god = god_lookup(arg3);

        if( god == 0 )
        {
            send_to_char("That is not a valid god for that character.\n\r",ch);
            return;
        }

        victim->god = god;
        
        return;
    }

    if( !str_prefix( arg2, "clanlvl" ) )
    {
	char str[MAX_STRING_LENGTH];
	
	if( value == -1 ) 
	{
		send_to_char( "Not a valid number.\n\r", ch );
	}
	else if( (value > 0) && (value < 12) )
	{
		sprintf( str, "Player's clan level changed to %d.\n\r", value );
		send_to_char( str, ch );
		victim->clanlevel = value;
	}
	else
	{
		send_to_char( "Clanlevel must be between 1 and 11 (11 for gods only).\n\r", ch );
	}
	return;
    }

    if( !str_prefix( arg2, "subclan" ) )
    {

	int subclan;

	subclan = subclan_lookup( arg3 );

	if( subclan == 0 || str_prefix( arg3, "none" ) )
	{
		send_to_char( "Not a valid subclan.\n\r", ch );
		return;
	}
		
    	
	victim->subclan = subclan;
	
	return;
    }
    	
    if ( !str_cmp( arg2, "str" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_STR) )
	{
	    sprintf(buf,
		"Strength range is 3 to %d\n\r.",
		get_max_train(victim,STAT_STR));
	    send_to_char(buf,ch);
	    return;
	}

	victim->perm_stat[STAT_STR] = value;
	return;
    }

    if ( !str_cmp( arg2, "security" ) )	/* OLC */
    {
	if ( IS_NPC(ch) )
	{
		send_to_char( "Si, claro.\n\r", ch );
		return;
	}

        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

	if ( value > ch->pcdata->security || value < 0 )
	{
	    if ( ch->pcdata->security != 0 )
	    {
		sprintf( buf, "Valid security is 0-%d.\n\r",
		    ch->pcdata->security );
		send_to_char( buf, ch );
	    }
	    else
	    {
		send_to_char( "Valid security is 0 only.\n\r", ch );
	    }
	    return;
	}
	victim->pcdata->security = value;
	return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
        if ( value < 3 || value > get_max_train(victim,STAT_INT) )
        {
            sprintf(buf,
		"Intelligence range is 3 to %d.\n\r",
		get_max_train(victim,STAT_INT));
            send_to_char(buf,ch);
            return;
        }
 
        victim->perm_stat[STAT_INT] = value;
        return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_WIS) )
	{
	    sprintf(buf,
		"Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_WIS] = value;
	return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_DEX) )
	{
	    sprintf(buf,
		"Dexterity range is 3 to %d.\n\r",
		get_max_train(victim,STAT_DEX));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_DEX] = value;
	return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
	if ( value < 3 || value > get_max_train(victim,STAT_CON) )
	{
	    sprintf(buf,
		"Constitution range is 3 to %d.\n\r",
		get_max_train(victim,STAT_CON));
	    send_to_char( buf, ch );
	    return;
	}

	victim->perm_stat[STAT_CON] = value;
	return;
    }

    if ( !str_prefix( arg2, "sex" ) )
    {
	if ( value < 0 || value > 2 )
	{
	    send_to_char( "Sex range is 0 to 2.\n\r", ch );
	    return;
	}
	victim->sex = value;
	if (!IS_NPC(victim))
	    victim->pcdata->true_sex = value;
	return;
    }

    if ( !str_prefix( arg2, "class" ) )
    {
	int class;

	if (IS_NPC(victim))
	{
	    send_to_char("Mobiles have no class.\n\r",ch);
	    return;
	}

	class = class_lookup(arg3);
	if ( class == -1 )
	{
	    char buf[MAX_STRING_LENGTH];

        	strcpy( buf, "Possible classes are: " );
        	for ( class = 0; class < MAX_CLASS; class++ )
        	{
            	    if ( class > 0 )
                    	strcat( buf, " " );
            	    strcat( buf, class_table[class].name );
        	}
            strcat( buf, ".\n\r" );

	    send_to_char(buf,ch);
	    return;
	}

	victim->class = class;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {

	if ( value < 0 || value > MAX_LEVEL )
	{
	    sprintf(buf, "Level range is 0 to %d.\n\r", MAX_LEVEL);
	    send_to_char(buf, ch);
	    return;
	}
	victim->level = value;
	return;
    }

    if ( !str_prefix( arg2, "steel" ) )
    {
	victim->steel = value;
	return;
    }

    if ( !str_prefix(arg2, "gold" ) )
    {
	victim->gold = value;
	return;
    }

    if ( !str_prefix( arg2, "hp" ) )
    {
	if ( value < -10 || value > 30000 )
	{
	    send_to_char( "Hp range is -10 to 30,000 hit points.\n\r", ch );
	    return;
	}
	victim->max_hit = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_hit = value;
	return;
    }

    if ( !str_prefix( arg2, "mana" ) )
    {
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "Mana range is 0 to 30,000 mana points.\n\r", ch );
	    return;
	}
	victim->max_mana = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_mana = value;
	return;
    }

    if ( !str_prefix( arg2, "move" ) )
    {
	if ( value < 0 || value > 30000 )
	{
	    send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
	    return;
	}
	victim->max_move = value;
        if (!IS_NPC(victim))
            victim->pcdata->perm_move = value;
	return;
    }

    if ( !str_prefix( arg2, "practice" ) )
    {
	if ( value < 0 || value > 250 )
	{
	    send_to_char( "Practice range is 0 to 250 sessions.\n\r", ch );
	    return;
	}
	victim->practice = value;
	return;
    }

    if ( !str_prefix( arg2, "train" ))
    {
	if (value < 0 || value > 50 )
	{
	    send_to_char("Training session range is 0 to 50 sessions.\n\r",ch);
	    return;
	}
	victim->train = value;
	return;
    }

    if ( !str_prefix( arg2, "align" ) )
    {
	if ( value < -1000 || value > 1000 )
	{
	    send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
	    return;
	}
	victim->alignment = value;
	return;
    }

    if ( !str_prefix( arg2, "thirst" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Thirst range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_THIRST] = value;
	return;
    }

    if ( !str_prefix( arg2, "drunk" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -1 || value > 100 )
	{
	    send_to_char( "Drunk range is -1 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_DRUNK] = value;
	return;
    }

    if ( !str_prefix( arg2, "full" ) )
    {
	if ( IS_NPC(victim) )
	{
	    send_to_char( "Not on NPC's.\n\r", ch );
	    return;
	}

	if ( value < -11 || value > 100 )
	{
	    send_to_char( "Full range is -11 to 100.\n\r", ch );
	    return;
	}

	victim->pcdata->condition[COND_FULL] = value;
	return;
    }

    if ( !str_prefix( arg2, "hunger" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
 
        if ( value < -11 || value > 100 )
        {
            send_to_char( "Full range is -11 to 100.\n\r", ch );
            return;
        }
 
        victim->pcdata->condition[COND_HUNGER] = value;
        return;
    }

    if (!str_prefix( arg2, "race" ) )
    {
	int race;

	race = race_lookup(arg3);

	if ( race == 0)
	{
	    send_to_char("That is not a valid race.\n\r",ch);
	    return;
	}

	if (!IS_NPC(victim) && !race_table[race].pc_race)
	{
	    send_to_char("That is not a valid player race.\n\r",ch);
	    return;
	}

	victim->race = race;
	return;
    }
   
    if (!str_prefix(arg2,"group"))
    {
	if (!IS_NPC(victim))
	{
	    send_to_char("Only on NPCs.\n\r",ch);
	    return;
	}
	victim->group = value;
	return;
    }

    if ( !str_prefix(arg2, "damroll" ) )
    {
        victim->damroll = value;
        return;
    }

    if ( !str_prefix(arg2, "hitroll" ) )
    {
        victim->hitroll = value;
        return;
    }
    
    if( !str_prefix( arg2, "questp" ) )
    {
    	if( IS_NPC( victim ) )
    	{
    		send_to_char( "Not on NPC's.\n\r", ch );
    		return;
    	}
    	
    	victim->questpoints = value;
    	return;
    }
    
    if( !str_prefix( arg2, "clanrank" ) )
    {
    	if( IS_NPC( victim ) )
    	{
    		send_to_char( "Not on NPC's.\n\r", ch );
    		return;
    	}
    	
    	if( !str_prefix( arg3, "reset" ) )
    	{
    		send_to_char( "Player's rank reset.\n\r", ch );
    		victim->clanrank = NULL;
    		return;
    	}
    	
    	send_to_char( "Player's rank set.\n\r", ch );
    	victim->clanrank = str_dup( arg3 );
    	return;
    }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_mset, "" );
    return;
}

void do_string( CHAR_DATA *ch, char *argument )
{
    char type [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  string char <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long desc title spec\n\r",ch);
	send_to_char("  string obj  <name> <field> <string>\n\r",ch);
	send_to_char("    fields: name short long extended\n\r",ch);
	return;
    }
    
    if (!str_prefix(type,"character") || !str_prefix(type,"mobile"))
    {
    	if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
    	}

	/* clear zone for mobs */
	victim->zone = NULL;

	/* string something */

     	if ( !str_prefix( arg2, "name" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }
	    free_string( victim->name );
	    victim->name = str_dup( arg3 );
	    return;
    	}
    	
    	if ( !str_prefix( arg2, "description" ) )
    	{
    	    free_string(victim->description);
    	    victim->description = str_dup(arg3);
    	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( victim->short_descr );
	    victim->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( victim->long_descr );
	    strcat(arg3,"\n\r");
	    victim->long_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "title" ) )
    	{
	    if ( IS_NPC(victim) )
	    {
	    	send_to_char( "Not on NPC's.\n\r", ch );
	    	return;
	    }

	    set_title( victim, arg3 );
	    return;
    	}

      if ( !str_prefix( arg2, "spec" ) )
    	{
	    if ( !IS_NPC(victim) )
	    {
	    	send_to_char( "Not on PC's.\n\r", ch );
	    	return;
	    }

	    if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
	    {
	    	send_to_char( "No such spec fun.\n\r", ch );
	    	return;
	    }

	    return;
    	}
    }
    
    if (!str_prefix(type,"object"))
    {
    	/* string an obj */
    	
   	if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    	{
	    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	    return;
    	}
    	
        if ( !str_prefix( arg2, "name" ) )
    	{
	    free_string( obj->name );
	    obj->name = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "short" ) )
    	{
	    free_string( obj->short_descr );
	    obj->short_descr = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "long" ) )
    	{
	    free_string( obj->description );
	    obj->description = str_dup( arg3 );
	    return;
    	}

    	if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
    	{
	    EXTRA_DESCR_DATA *ed;

	    argument = one_argument( argument, arg3 );
	    if ( argument == NULL )
	    {
	    	send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		    ch );
	    	return;
	    }

 	    strcat(argument,"\n\r");

	    ed = new_extra_descr();

	    ed->keyword		= str_dup( arg3     );
	    ed->description	= str_dup( argument );
	    ed->next		= obj->extra_descr;
	    obj->extra_descr	= ed;
	    return;
    	}
    }
    
    	
    /* echo bad use message */
    do_function(ch, &do_string, "");
}



void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  set obj <object> <field> <value>\n\r",ch);
	send_to_char("  Field being one of:\n\r",				ch );
	send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",	ch );
	send_to_char("    extra wear level weight cost timer\n\r",		ch );
	return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
	return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
	obj->value[0] = UMIN(50,value);
	return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
	obj->value[1] = value;
	return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
	obj->value[2] = value;
	return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
	obj->value[3] = value;
	return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
	obj->value[4] = value;
	return;
    }

    if ( !str_prefix( arg2, "extra" ) )
    {
	obj->extra_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
	obj->wear_flags = value;
	return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
	obj->level = value;
	return;
    }
	
    if ( !str_prefix( arg2, "weight" ) )
    {
	obj->weight = value;
	return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
	obj->cost = value;
	return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
	obj->timer = value;
	return;
    }
	
    /*
     * Generate usage message.
     */
    do_function(ch, &do_oset, "" );
    return;
}



void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char( "Syntax:\n\r",ch);
	send_to_char( "  set room <location> <field> <value>\n\r",ch);
	send_to_char( "  Field being one of:\n\r",			ch );
	send_to_char( "    flags sector\n\r",				ch );
	return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No such location.\n\r", ch );
	return;
    }

    if (!is_room_owner(ch,location) && ch->in_room != location 
    &&  room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        send_to_char("That room is private right now.\n\r",ch);
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
	send_to_char( "Value must be numeric.\n\r", ch );
	return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "flags" ) )
    {
	location->room_flags	= value;
	return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
	location->sector_type	= value;
	return;
    }

    /*
     * Generate usage message.
     */
    do_function(ch, &do_rset, "");
    return;
}




/* Written by Stimpy, ported to rom2.4 by Goldhand 3/12
 *
 *	Added the other COMM_ stuff that wasn't defined before 4/16 -Silverhand
 */
void do_sockets( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA       *vch;
    DESCRIPTOR_DATA *d;
    char            buf  [ MAX_STRING_LENGTH ];
    char            buf2 [ MAX_STRING_LENGTH ];
    int             count;
    char *          st;
    char            s[100];
    char            idle[10];


    count       = 0;
    buf[0]      = '\0';
    buf2[0]     = '\0';

    strcat( buf2, "\n\r{B[{mNum Connected_State  Login  Idl{B] {CPlayer Name  {RHost{x\n\r" );
    strcat( buf2,
"{G--------------------------------------------------------------------------{x\n\r");  
    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->connected != CON_PLAYING || (d->character && can_see( ch, d->character ) ))
        {
           /* NB: You may need to edit the CON_ values */
           switch( d->connected )
           {
              case CON_PLAYING:              st = "    PLAYING    ";    break;
              case CON_GET_NAME:             st = "   Get Name    ";    break;
              case CON_GET_OLD_PASSWORD:     st = "Get Old Passwd ";    break;
              case CON_CONFIRM_NEW_NAME:     st = " Confirm Name  ";    break;
              case CON_GET_NEW_PASSWORD:     st = "Get New Passwd ";    break;
              case CON_CONFIRM_NEW_PASSWORD: st = "Confirm Passwd ";    break;
              case CON_GET_NEW_RACE:         st = "  Get New Race ";    break;
              case CON_GET_NEW_SEX:          st = "  Get New Sex  ";    break;
		  case CON_GET_PK:		   st = "     Get PK	";	break;
              case CON_GET_NEW_CLASS:        st = " Get New Class ";    break;
              case CON_GET_ALIGNMENT:        st = " Get New Align ";    break;
              case CON_DEFAULT_CHOICE:       st = " Choosing Cust ";    break;
              case CON_GEN_GROUPS:           st = " Customization ";    break;
              case CON_PICK_WEAPON:          st = " Picking Weapon";    break;
              case CON_GET_HAND:	     st = " Choosing Hand ";	break;
              case CON_READ_IMOTD:           st = " Reading IMOTD ";    break;
              case CON_BREAK_CONNECT:        st = "   LINKDEAD    ";    break;
              case CON_READ_MOTD:            st = " Reading MOTD  ";    break;
              case CON_ANSI:                 st = "  Connecting   ";    break;
              default:                       st = "   !UNKNOWN!   ";    break;
           }
           count++;
           
           /* Format "login" value... */
           vch = d->original ? d->original : d->character;
           
           if(vch)
               strftime( s, 100, "%I:%M%p", localtime( &vch->logon ) );
           else
               strcpy(s, " ");
           
           if ( vch && vch->timer > 0 )
              sprintf( idle, "%-2d", vch->timer );
           else
              strcpy( idle, " " );
           
           sprintf( buf, "{B[{m%3d %s %7s  %2s{B]{R%s{C%-12s {R%-32.32s{x\n\r",
              d->descriptor,
              st,
              s,
              idle,
              vch ? (vch->level >= IMMORTAL ? "*" : " ") : " ",
              vch ? vch->name : "(None!)",
              d->host );
              
           strcat( buf2, buf );

        }
    }

    sprintf( buf, "\n\r%d user%s\n\r", count, count == 1 ? "" : "s" );
    strcat( buf2, buf );
    send_to_char( buf2, ch );
    return;
}







/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Force whom to do what?\n\r", ch );
	return;
    }

    one_argument(argument,arg2);
  
    if (!str_cmp(arg2, "quit") 
    || !str_cmp(arg2,"delete") 
    || !str_prefix(arg2,"mob"))
    {
	send_to_char("That will NOT be done.\n\r",ch);
	return;
    }

    sprintf( buf, "$n forces you to '%s'.", argument );

    if ( !str_cmp( arg, "all" ) )
    {
        DESCRIPTOR_DATA *d, *d_next;

	if (get_trust(ch) < MAX_LEVEL - 5)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( d->connected == CON_PLAYING
	     && get_trust( d->character ) < get_trust( ch ) )
	    {
		act( buf, ch, NULL, d->character, TO_VICT );
		interpret( d->character, argument );
	    }
	}
    }
    else if (!str_cmp(arg,"players"))
    {
        DESCRIPTOR_DATA *d, *d_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( d = descriptor_list; d != NULL; d = d_next )
        {
            d_next = d->next;
 
            if ( d->connected == CON_PLAYING
            && get_trust( d->character ) < get_trust( ch ) 
	    &&	 d->character->level < LEVEL_HERO)
            {
                act( buf, ch, NULL, d->character, TO_VICT );
                interpret( d->character, argument );
            }
        }
    }
    else if (!str_cmp(arg,"gods"))
    {
        DESCRIPTOR_DATA *d, *d_next;
 
        if (get_trust(ch) < MAX_LEVEL - 2)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
 
        for ( d = descriptor_list; d != NULL; d = d_next )
        {
            d_next = d->next;
 
            if ( d->connected == CON_PLAYING
            && get_trust( d->character ) < get_trust( ch )
            &&   d->character->level >= LEVEL_HERO)
            {
                act( buf, ch, NULL, d->character, TO_VICT );
                interpret( d->character, argument );
            }
        }
    }
    else
    {
	CHAR_DATA *victim;

	if ( ( victim = get_char_world( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

    	if (!is_room_owner(ch,victim->in_room) 
	&&  ch->in_room != victim->in_room 
        &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
    	{
            send_to_char("That character is in a private room.\n\r",ch);
            return;
        }

	if ( get_trust( victim ) >= get_trust( ch ) )
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}

	if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -5)
	{
	    send_to_char("Not at your level!\n\r",ch);
	    return;
	}

	act( buf, ch, NULL, victim, TO_VICT );
	interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' ) 
    /* take the default path */

      if ( ch->invis_level)
      {
	  ch->invis_level = 0;
	  act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly fade back into existence.\n\r", ch );
      }
      else
      {
	  ch->invis_level = 102;
	  act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
	send_to_char("Invis level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
	  ch->reply = NULL;
          ch->invis_level = level;
          act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You slowly vanish into thin air.\n\r", ch );
      }
    }

    return;
}


void do_incognito( CHAR_DATA *ch, char *argument )
{
    int level;
    char arg[MAX_STRING_LENGTH];
 
    /* RT code for taking a level argument */
    one_argument( argument, arg );
 
    if ( arg[0] == '\0' )
    /* take the default path */
 
      if ( ch->incog_level)
      {
          ch->incog_level = 0;
          act( "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You are no longer cloaked.\n\r", ch );
      }
      else
      {
          ch->incog_level = get_trust(ch);
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > get_trust(ch))
      {
        send_to_char("Incog level must be between 2 and your level.\n\r",ch);
        return;
      }
      else
      {
          ch->reply = NULL;
          ch->incog_level = level;
          act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
          send_to_char( "You cloak your presence.\n\r", ch );
      }
    }
 
    return;
}



void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
	REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
	SET_BIT(ch->act, PLR_HOLYLIGHT);
	send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi (CHAR_DATA *ch, char *argument)
{
    send_to_char("You cannot abbreviate the prefix command.\r\n",ch);
    return;
}

void do_prefix (CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if (argument[0] == '\0')
    {
	if (ch->prefix[0] == '\0')
	{
	    send_to_char("You have no prefix to clear.\r\n",ch);
	    return;
	}

	send_to_char("Prefix removed.\r\n",ch);
	free_string(ch->prefix);
	ch->prefix = str_dup("");
	return;
    }

    if (ch->prefix[0] != '\0')
    {
	sprintf(buf,"Prefix changed to %s.\r\n",argument);
	free_string(ch->prefix);
    }
    else
    {
	sprintf(buf,"Prefix set to %s.\r\n",argument);
    }

    ch->prefix = str_dup(argument);
}

void do_copyove(CHAR_DATA *ch, char *argument)
{
    send_to_char("If you want to copyover, type the whole command.\n\r", ch);
}

#define CH(descriptor)  ((descriptor)->original ? \
(descriptor)->original : (descriptor)->character)

/* This file holds the copyover data */
#define COPYOVER_FILE "copyover.data"

/* This is the executable file */
#define EXE_FILE	  "../src/rom"


/*  Copyover - Original idea: Fusion of MUD++
 *  Adapted to Diku by Erwin S. Andreasen, <erwin@pip.dknet.dk>
 *  http://pip.dknet.dk/~pip1773
 *  Changed into a ROM patch after seeing the 100th request for it :)
 */
void do_copyover (CHAR_DATA *ch, char * argument)
{
	FILE *fp;
	DESCRIPTOR_DATA *d, *d_next;
	char buf [150], buf2[150];
	extern int port,control; /* db.c */
	
	fp = fopen (COPYOVER_FILE, "w");
	
	if (!fp)
	{
		send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
		logf ("Could not write to copyover file: %s", COPYOVER_FILE);
		perror ("do_copyover:fopen");
		return;
	}
	
	/* Consider changing all saved areas here, if you use OLC */
	
	/* do_asave (NULL, "");  - autosave changed areas */
	
	
	sprintf (buf2, "\n\r  {cThe world around you suddenly disappears!!\n\r  %s must be up to no good...{x\n\r", 
	ch->invis_level < IMMORTAL && ch->incog_level < IMMORTAL ? ch->name : "Someone");

	/* For each playing descriptor, save its state */
	for (d = descriptor_list; d ; d = d_next)
	{
		CHAR_DATA * och = CH (d);
		d_next = d->next; /* We delete from the list , so need to save this */
		
		if (!d->character || d->connected > CON_PLAYING) /* drop those logging on */
		{
			write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r", 0);
			close_socket (d); /* throw'em out */
		}
		else
		{
			fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);

#if 0			 /* This is not necessary for ROM */
			if (och->level == 1)
			{
				write_to_descriptor (d->descriptor, "Since you are level one, and level one characters do not save, you gain a free level!\n\r", 0);
				advance_level (och);
				och->level++; /* Advance_level doesn't do that */
			}
#endif			
			save_char_obj (och);
			
			colourconv(buf, buf2, och);
			write_to_descriptor (d->descriptor, buf, 0);
		}
	}
	
	fprintf (fp, "-1\n");
	fclose (fp);
	
	/* Close reserve and other always-open files and release other resources */
	
	fclose (fpReserve);
	
	/* exec - descriptors are inherited */
	
	sprintf (buf, "%d", port);
	sprintf (buf2, "%d", control);
	execl (EXE_FILE, "rom", buf, "copyover", buf2, (char *) NULL);

	/* Failed - sucessful exec will not return */
	
	perror ("do_copyover: execl");
	send_to_char ("Copyover FAILED!\n\r",ch);
	
	/* Here you might want to reopen fpReserve */
	fpReserve = fopen (NULL_FILE, "r");
}

/* Recover from a copyover - load players */
void copyover_recover ()
{
	DESCRIPTOR_DATA *d;
	FILE *fp;
	char name [100];
	char host[MSL];
	char buf[100], buf2[100];
	int desc;
	bool fOld;
	
	logf ("Copyover recovery initiated");
	
	fp = fopen (COPYOVER_FILE, "r");
	
	if (!fp) /* there are some descriptors open which will hang forever then ? */
	{
		perror ("copyover_recover:fopen");
		logf ("Copyover file not found. Exitting.\n\r");
		exit (1);
	}

	unlink (COPYOVER_FILE); /* In case something crashes - doesn't prevent reading	*/
	
	for (;;)
	{
		fscanf (fp, "%d %s %s\n", &desc, name, host);
		if (desc == -1)
			break;

		/* Write something, and check if it goes error-free */		
		if (!write_to_descriptor (desc, "\n\r      The world begins to reform...\n\r",0))
		{
			close (desc); /* nope */
			continue;
		}
		
		d = new_descriptor();
		d->descriptor = desc;
		
		d->host = str_dup (host);
		d->next = descriptor_list;
		descriptor_list = d;
		d->connected = CON_COPYOVER_RECOVER; /* -15, so close_socket frees the char */
		
	
		/* Now, find the pfile */
		
		fOld = load_char_obj (d, name);
		
		if (!fOld) /* Player file not found?! */
		{
			write_to_descriptor (desc, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
			close_socket (d);			
		}
		else /* ok! */
		{
			strcpy(buf2, "\n\r  {cKrynn {Gsighs in relief{c as life continues undisturbed.{x\n\r");
			colourconv(buf, buf2, d->character);
			write_to_descriptor (desc, buf, 0);
	
			/* Just In Case */
			if (!d->character->in_room)
				d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);

			/* Insert in the char_list */
			d->character->next = char_list;
			char_list = d->character;

			char_to_room (d->character, d->character->in_room);
			do_look (d->character, "auto");
			act ("$n materializes!", d->character, NULL, NULL, TO_ROOM);
			d->connected = CON_PLAYING;

			if (d->character->pet != NULL)
			{
			    char_to_room(d->character->pet,d->character->in_room);
			    act("$n materializes!.",d->character->pet,NULL,NULL,TO_ROOM);
			}
		}
		
	}
   fclose (fp);
	
	
}



void do_grab (CHAR_DATA *ch, char *argument)
{
    CHAR_DATA  *victim;
    OBJ_DATA   *obj;
    char 	arg1 [ MAX_INPUT_LENGTH ];
    char 	arg2 [ MAX_INPUT_LENGTH ];
    char 	arg3 [ MAX_INPUT_LENGTH ];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( ( arg1[0] == '\0' ) || ( arg2[0] == '\0' ) )
    {
        send_to_char( "Syntax : grab <object> <player>\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world( ch, arg2 ) ) )
    {
        send_to_char( "They are not here!\n\r", ch );
        return;
    }

    if ( !( obj = get_obj_list( ch, arg1, victim->carrying ) ) )
    {
        send_to_char( "They do not have that item.\n\r", ch );
        return;
    }

    if ( victim->level >= ch->level )
    {
        send_to_char( "You Failed.\r\n", ch );
        return;
    }

    if ( obj->wear_loc != WEAR_NONE )
        unequip_char( victim, obj );

    obj_from_char( obj );
    obj_to_char( obj, ch );

    act( "You grab $p from $N.", ch, obj, victim, TO_CHAR );
    if ( arg3[0] == '\0' 
    	|| !str_cmp( arg3, "yes" ) || !str_cmp( arg3, "true" ) )		
           act( "You no longer own $p.", ch, obj, victim, TO_VICT );

    return;
}




int colorstrlen(char *argument)
{
  char *str;
  int strlength;
  int value;

  if (argument == NULL || argument[0] == '\0')
    return 0;

  str = argument;
  strlength = 0;
  value = 0;

  while (str[value] != '\0')
  {
    if ( str[value] != '{' ) 
    {	
      value++;
      strlength++;
      continue;
    }

    if (str[value] == '{')
      value++;

    value++;
  }
  return strlength;
}


void do_whotext (CHAR_DATA *ch, char *argument)
{

	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

      argument = one_argument( argument, arg1 );
      strcpy( arg2, argument );


    if( arg1[0] == '\0' && arg2[0] == '\0')
    {
	send_to_char( "Syntax: Whotext <char> <string>.\n\r", ch );
	return;
    }

    if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "There is no player of that name, try again.\n\r", ch );
	return;
    }

    if( IS_NPC( victim ) )
    {
	send_to_char( "Try a player, not a mob.\n\r", ch );
	return;
    }

    if( colorstrlen( arg2 ) > 14)
    {
	send_to_char( "String length must be less than 14.\n\r", ch );
	return;
    }

    if( arg1[0] != '\0' && arg2[0] == '\0' )
    {
	victim->whotext = NULL;
	send_to_char( "Whotext reset.\n\r", ch );
	return;
    }


    victim->whotext = str_dup( arg2 );
    sprintf( buf, "Whotext set to %s.\n\r", arg2 );
    send_to_char( buf, ch );
    return;

}


void do_retribution( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
        return;

    if ( argument[0] != '\0' )
    {
	buf[0] = '\0';
	smash_tilde( argument );

    	if (argument[0] == '-')
    	{
            int len;
            bool found = FALSE;
 
            if (ch->pcdata->smite == NULL || ch->pcdata->smite[0] == '\0')
            {
                send_to_char("No lines left to remove.\n\r",ch);
                return;
            }
	
            strcpy(buf,ch->pcdata->smite);
 
            for (len = strlen(buf); len > 0; len--)
            {
                if (buf[len] == '\r')
                {
                    if (!found)  /* back it up */
                    {
                        if (len > 0)
                            len--;
                        found = TRUE;
                    }
                    else /* found the second one */
                    {
                        buf[len + 1] = '\0';
                        free_string(ch->pcdata->smite);
                        ch->pcdata->smite = str_dup(buf);
                        send_to_char( "Your divine fury takes the form of:\n\r", ch );
                        send_to_char( ch->pcdata->smite ? ch->pcdata->smite : 
			    "(None).\n\r", ch );
                        return;
                    }
                }
            }
            buf[0] = '\0';
            free_string(ch->pcdata->smite);
            ch->pcdata->smite = str_dup(buf);
            send_to_char("Mortals no longer need fear your mark upon them.\n\r",ch);
	    return;
        }
	if ( argument[0] == '+' )
	{
            if ( ch->pcdata->smite != NULL )
                strcat( buf, ch->pcdata->smite );
	    argument++;
	    while ( isspace(*argument) )
		argument++;
	}

        if ( strlen(buf) >= 1024)
	{
            send_to_char( "Your fury is great indeed, too great.  Use less lines.\n\r", ch );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\n\r" );
        free_string( ch->pcdata->smite );
        ch->pcdata->smite = str_dup( buf );
    }

    send_to_char( "Your divine fury takes the form of:\n\r", ch );
    send_to_char( ch->pcdata->smite ? ch->pcdata->smite : "(None).\n\r", ch );
    return;
}


void do_smite( CHAR_DATA *ch, char *argument )
{

    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;

    if ( IS_NPC(ch) )  /* NPCs may get to be smited, but switch immortals can't */
        return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: Smite <victim>.\n\r",ch);
	return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
        send_to_char( "They are saved only through their abscence.\n\r",ch);
	return;
    }

    if (!IS_NPC(victim) && victim->level >= get_trust(ch))
    {
        send_to_char("Your reach exceeds your grasp.\n\r",ch);
        return;
    }

    
    /* Customize stuff by alignment */

    if (ch->alignment > 300)
    {                                                                                                                                   
        act_new("Your actions have brought the holy power of $n upon you!",ch,NULL,victim,TO_VICT,POS_DEAD);
        act_new("$N has brought the holy power of $n upon themselves!",ch,NULL,victim,TO_NOTVICT,POS_DEAD);                                              
    }
    if (ch->alignment > -301 && ch->alignment < 301)
    {
        act_new("Your actions have called the divine fury of $n upon you!",ch,NULL,victim,TO_VICT,POS_DEAD);
        act_new("$N has called the divine fury of $n upon themselves!",ch,NULL,victim,TO_NOTVICT,POS_DEAD);
    }
    if (ch->alignment < -300)
    {
        act_new("You are struck down by the infernal power of $n!!",ch,NULL,victim,TO_VICT,POS_DEAD);
        act_new("The hellspawned, infernal power of $n has struck down $N!!",ch,NULL,victim,TO_NOTVICT,POS_DEAD);
    }

    /* This is where the thing we did in retribution is used */

    if ( ch->pcdata->smite[0] != '\0' )
    {
        send_to_char( ch->pcdata->smite, victim );
    }

	victim->position = POS_SITTING;

	victim->hit = 1;

	victim->mana = 1;

	victim->move = 1;

	location = get_room_index( (victim->recall ? victim->recall : (is_clan( victim ) ? clan_table[victim->clan].recall : god_table[victim->god].recall )) );

	char_from_room( victim );
	char_to_room( victim, location);

    send_to_char("Your will is done, your power felt.\n\r",ch);
    return;
}



void do_omni( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char editact[MAX_STRING_LENGTH];
    char editvnum[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int immmatch;
    int mortmatch;
    int hptemp;
    MOB_INDEX_DATA *pMob;
    OBJ_INDEX_DATA *pObj;
    ROOM_INDEX_DATA *pRoom; 
    AREA_DATA *pArea;


 /*
 * Initalize Variables.
 */

    immmatch = 0;
    mortmatch = 0;
    buf[0] = '\0';
    output = new_buf();


/*
 * Crashes if used while switched - disable it
 */
 
if( IS_NPC( ch ) )
    return;


 /*
 * Count and output the IMMs.
 */

	sprintf( buf, " {W----{CImmortals{W----{x\n\r");
	add_buf(output,buf);
	sprintf( buf, "{cName          Level   Wiz   Incog    [ Vnum]     Editing{x\n\r");
	add_buf(output,buf);
	sprintf( buf, "----------------------------------------------------------------\n\r");
	add_buf(output,buf);

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;

        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;
 
        wch   = ( d->original != NULL ) ? d->original : d->character;

	if (!can_see(ch,wch)  || wch->level < IMMORTAL)
	    continue;
 
        immmatch++;


	if( wch->desc == NULL )
	{
	    sprintf( editact, " Switch" );
	    editvnum[0] = '\0';
	}
	else if( wch->desc->editor == 1)
	{
	    EDIT_AREA(wch, pArea);
	    sprintf( editact, "   Area" );
	    sprintf( editvnum, "(%d-%d)", pArea->min_vnum, pArea->max_vnum ); 
	}
        else if( wch->desc->editor == 2)
	{
	    EDIT_ROOM(wch, pRoom);
            sprintf( editact, "   Room" );
	    sprintf( editvnum, "(%d)", pRoom->vnum );
	}
        else if( wch->desc->editor == 3)
	{
	    EDIT_OBJ(wch, pObj);
            sprintf( editact, " Object" );
	    sprintf( editvnum, "(%d)", pObj->vnum );
	}
        else if( wch->desc->editor == 4)
	{
	    EDIT_MOB(wch, pMob);
            sprintf( editact, " Mobile" );
	    sprintf( editvnum, "(%d)", pMob->vnum );
	}
	else   
	{
	    sprintf( editact, "Nothing" );
	    editvnum[0] = '\0';
	}


	sprintf( buf, "{g%-14s %d    %-3d    %-3d     [%5d]     %s %s{x\n\r",
			wch->name,
			wch->level,
			wch->invis_level,
			wch->incog_level,
			wch->in_room->vnum,
			editact,
			editvnum);
			add_buf(output,buf);
    }
    
    
 /*
 * Count and output the Morts.
 */
	sprintf( buf, " \n\r {W-----{CMortals{W-----{x\n\r");
	add_buf(output,buf);
	sprintf( buf, "{cName           Race       Class   Position    Lev  %%hps  [ Vnum]{x\n\r");
	add_buf(output,buf);
	sprintf( buf, "----------------------------------------------------------------\n\r");
	add_buf(output,buf);
	hptemp = 0;
	
   for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;
        char race[MAX_STRING_LENGTH];
        char class[MAX_STRING_LENGTH];
        
        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;
 
        wch   = ( d->original != NULL ) ? d->original : d->character;

	if (!can_see(ch,wch) || wch->level > get_trust(ch) || wch->level >= IMMORTAL)
	    continue;
 
        mortmatch++;
 	if ((wch->max_hit != wch->hit) && (wch->hit > 0))
 		hptemp = (wch->hit*100)/wch->max_hit;
 	else if (wch->max_hit == wch->hit)
 		hptemp = 100;
 	else if (wch->hit < 0)
 		hptemp = 0;
 		
	sprintf( class, "%s", capitalize( class_table[wch->class].name ) );
	
	sprintf( race, "%s", capitalize( pc_race_table[wch->race].name ) );

	sprintf( buf, "{g%-14s %-10s %-7s %-11s %3d  %3d%%  [%5d]{x\n\r",
		wch->name,
		race,
	    	class,
		capitalize( position_table[wch->position].name) , 
		wch->level,
		hptemp,
		wch->in_room->vnum);
	add_buf(output,buf);
    }

/*
 * Tally the counts and send the whole list out.
 */
   sprintf( buf2, "\n\r{cIMMs found:  {r%d{x\n\r", immmatch );
    add_buf(output,buf2);
    sprintf( buf2, "{cMorts found: {r%d{x\n\r", mortmatch );
    add_buf(output,buf2);
    page_to_char( buf_string(output), ch );
    free_buf(output);
    return;
}





void do_castall( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char *target_name;
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d; 
    void *vo;
    void *victname;
    int sn;
    int target;
    int i;

    argument = one_argument( argument, arg1 );
    target_name = one_argument( argument, arg2 );

    if( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: castall <spell> -<char>.\n\r", ch );
        return;
    }

    if( (sn = find_spell(ch,arg1)) < 1)
    {
	send_to_char( "That is not a spell.\n\r", ch);
	return;
    }


    if( arg2[0] == '-' )
    {
	for( i = 0; i < strlen( arg2 ); i++ )
	{
	    buf[i] = arg2[i+1];
	}

	if( ( victim = get_char_world( ch, buf ) ) == NULL )
	{
	    send_to_char( "That player is not here.\n\r", ch );
	    return;
	}
	
	victname = (void *) victim;
    }
	
    victname = (void *) ch;

    for( d = descriptor_list; d != NULL; d = d->next )
    {
 
        if( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;
    
        victim   = ( d->original != NULL ) ? d->original : d->character;
	
        vo = (void *) victim;
	target = TARGET_CHAR;

	if( victim->level <= 101 )
	{
	    if( str_cmp( vo, victname ) )
        	(*skill_table[sn].spell_fun) ( sn, ch->level, ch, vo,target);
	}
    }
    return;
}

void do_giveall( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;

    level = get_trust(ch);

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: giveall <vnum>.\n\r", ch );
	return; 
    }
    
    if ( ( pObjIndex = get_obj_index( atoi( argument ) ) ) == NULL )
    {
        send_to_char( "No object has that vnum.\n\r", ch );
        return;
    }

/*    obj = create_object( pObjIndex, level );*/
	     
    for ( d = descriptor_list; d != NULL; d = d->next )
    {     
 
/*        if ( d->connected != CON_PLAYING
          || !can_see( ch, d->character )
          || !IS_IMMORTAL( d->character )
          || d->character != ch )
            continue;
  */
  	if(d->connected != CON_PLAYING
  	|| !can_see(ch, d->character)
  	|| IS_IMMORTAL(d->character)
  	|| d->character == ch)
  	    continue;
  	      
/*        victim   = ( d->original != NULL ) ? d->original : d->character;*/
	victim = d->character;
	
   	obj = create_object( pObjIndex, level );


	obj_to_char( obj, victim );
	act( "$n gives $p to $N.", ch, obj, victim, TO_ROOM );
	act( "$n gives you $p.",   ch, obj, victim, TO_VICT );
        act( "You give $p to $N.", ch, obj, victim, TO_CHAR );
    
    }
    return;
}

/*
 * Add this somewhere in act_wiz.c
 */
void do_addapply(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA *obj;
  AFFECT_DATA *paf,*af_new;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  int affect_modify = 0, bit = 0, enchant_type, pos, i, loc = 0;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
  {
    send_to_char("Syntax for applies: addapply <object> <apply type> <value>\n\r",ch);
    send_to_char("Apply Types: hp str dex int wis con sex mana\n\r", ch);
    send_to_char("             ac move hitroll damroll saves\n\r\n\r", ch);
    send_to_char("Syntax for affects: addapply <object> affect <affect name>\n\r",ch);
    send_to_char("Affect Names: blind invisible detect_evil detect_invis detect_magic\n\r",ch);
    send_to_char("              detect_hidden detect_good sanctuary faerie_fire infrared\n\r",ch);
    send_to_char("              curse poison protect_evil protect_good sneak hide sleep charm\n\r", ch);
    send_to_char("              flying pass_door haste calm plague weaken dark_vision berserk\n\r", ch);
    send_to_char("              swim regeneration slow\n\r", ch);
    send_to_char("Affects availible include the ones you add too!\n\r", ch);
    return;
  }

  obj = get_obj_world(ch,arg1);

  if (obj == NULL)
  {
    send_to_char("No such object exists!\n\r",ch);
    return;
  }

       if (!str_prefix(arg2,"hp"))
	enchant_type=APPLY_HIT;
  else if (!str_prefix(arg2,"str"))
	enchant_type=APPLY_STR;
  else if (!str_prefix(arg2,"dex"))
	enchant_type=APPLY_DEX;
  else if (!str_prefix(arg2,"int"))
	enchant_type=APPLY_INT;
  else if (!str_prefix(arg2,"wis"))
	enchant_type=APPLY_WIS;
  else if (!str_prefix(arg2,"con"))
	enchant_type=APPLY_CON;
/*  else if (!str_prefix(arg2,"sex"))
	enchant_type=APPLY_SEX;
*/
  else if (!str_prefix(arg2,"mana"))
	enchant_type=APPLY_MANA;
  else if (!str_prefix(arg2,"move"))
	enchant_type=APPLY_MOVE;
  else if (!str_prefix(arg2,"ac"))
	enchant_type=APPLY_AC;
  else if (!str_prefix(arg2,"hitroll"))
	enchant_type=APPLY_HITROLL;
  else if (!str_prefix(arg2,"damroll"))
	enchant_type=APPLY_DAMROLL;
  else if (!str_prefix(arg2,"saves"))
	enchant_type=APPLY_SAVING_SPELL;
  else if (!str_prefix(arg2,"affect"))
	enchant_type=APPLY_SPELL_AFFECT;
  else
  {
    send_to_char("That apply is not possible!\n\r",ch);
    return;
  }

  if (enchant_type==APPLY_SPELL_AFFECT)
  {
    for (pos = 0; affect_flags[pos].name != NULL; pos++)
	if (!str_cmp(affect_flags[pos].name,arg3))
	{
	    bit = affect_flags[pos].bit;
	    loc = 1;
	}
	    
    for (pos = 0; affect2_flags[pos].name != NULL; pos++)
    	if (!strcmp(affect2_flags[pos].name, arg3))
    	{
    	    bit = affect2_flags[pos].bit;
    	    loc = 2;
    	}
  }
  else
  {
    if ( is_number(arg3) )
	affect_modify=atoi(arg3);	
    else
    {
	send_to_char("Applies require a value.\n\r", ch);
        return;
    }
  }

    if (!obj->enchanted)
    {
      obj->enchanted = TRUE;

      for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
      {
	if (affect_free == NULL)
	af_new = alloc_perm(sizeof(*af_new));
	else
        {
	  af_new = affect_free;
	  affect_free = affect_free->next;
        }

      af_new->next = obj->affected;
      obj->affected = af_new;
      af_new->type        = UMAX(0,paf->type);
      af_new->level       = paf->level;
      af_new->duration    = paf->duration;
      af_new->location    = paf->location;
      af_new->modifier    = paf->modifier;
      af_new->bitvector   = paf->bitvector;
      }
    }

  if (affect_free == NULL)
    paf = alloc_perm(sizeof(*paf));
  else
  {
    paf = affect_free;
    affect_free = affect_free->next;
  }

	paf->type 	= -1;
        paf->level      = ch->level;
        paf->duration   = -1;
        paf->location   = enchant_type;
	paf->modifier   = affect_modify;
        paf->bitvector  = bit;

	if (enchant_type==APPLY_SPELL_AFFECT)
	{
	    /* Quick hack to make table compatible with skill_lookup */

	    for ( i=0 ; arg3[i] != '\0'; i++ )
	    {
	        if ( arg3[i] == '_' )
	            arg3[i] = ' ';
	    }

	    paf->type 		= skill_lookup(arg3);
	    paf->where		= (loc == 1 ? TO_AFFECTS : TO_AFFECTS2);
	    paf->modifier	= 0;
	    paf->location 	= flag_value(apply_flags, "none");
	}

	paf->next       = obj->affected;
	obj->affected   = paf;

	send_to_char("Ok.\n\r", ch);
}

void do_pretitle (CHAR_DATA *ch, char *argument)
{

	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

      argument = one_argument( argument, arg1 );
      strcpy( arg2, argument );

    if( arg1[0] == '\0' && arg2[0] == '\0')
    {
	send_to_char( "Syntax: Pretitle <char> <string>.\n\r", ch );
	return;
    }

    if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
	send_to_char( "There is no player of that name, try again.\n\r", ch);
	return;     }

    if( IS_NPC( victim ) )
    {
	send_to_char( "Try a player, not a mob.\n\r", ch );
	return;
    }

    if( arg1[0] != '\0' && arg2[0] == '\0' )
    {
	victim->pretitle = NULL;
	send_to_char( "Pretitle reset.\n\r", ch );
	return;
    }

    victim->pretitle = str_dup( arg2 );
    sprintf( buf, "%s is now %s %s.\n\r", victim->name, victim->pretitle,
	victim->name );
    send_to_char( buf, ch );
    return;
}


void do_grant( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];    
    char arg2[MAX_STRING_LENGTH];
    char onegrant[MAX_STRING_LENGTH];
    char newgrant[MAX_STRING_LENGTH];
    char *allgrant;
    int cmd;
    int found=0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    
    if( arg1[0] == '\0' )
    {
    	send_to_char( "Syntax: grant <character> <command>\n\r", ch );
    	return;
    }

    if( (victim = get_char_world( ch, arg1 )) == NULL )
    {	
    	send_to_char( "There is no player with that name.\n\r", ch );
    	return;
    }
    
    if( arg2[0] == '\0' )
    {
    	send_to_char( victim->grant, ch );
    	return;
    }
    
    
    allgrant = victim->grant;
    
    for( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if( arg2[0] == cmd_table[cmd].name[0]
	&&  !str_prefix( arg2, cmd_table[cmd].name ) )
	{
		allgrant = victim->grant;
		while( allgrant != NULL )
		{
			allgrant = one_argument( allgrant, onegrant );
			if( onegrant[0] == '\0' )
				break;

			if( !str_prefix( cmd_table[cmd].name, onegrant ) )
			{
				found = 2;
				break;
			}
		}
		if( !found )
		{
			strcpy( onegrant, cmd_table[cmd].name );
			found = 1;	
		}
	
		break;
	}
    }
    
    if( !found )
    {
    	send_to_char( "Command not found.\n\r", ch );
    } else if( found == 1 )
    {
    	sprintf( buf, "Granted %s to %s.\n\r", onegrant, victim->name );
    	send_to_char( buf, ch );
    	sprintf( buf, "%s granted for your use.\n\r", onegrant );
    	send_to_char( buf, victim );
    	
    	sprintf( newgrant, "%s %s", victim->grant, onegrant );
    	victim->grant = str_dup( newgrant );
    } else
    {
    	sprintf( buf, "Took %s from %s.\n\r", onegrant, victim->name );
    	send_to_char( buf, ch );
    	sprintf( buf, "You can no longer use %s.\n\r", onegrant );
    	send_to_char( buf, ch );
    	
    	allgrant = victim->grant;
    	strcpy( buf, onegrant );
    	newgrant[0] = '\0';
    	while( allgrant != NULL )
    	{
    		allgrant = one_argument( allgrant, onegrant );
		if( onegrant[0] == '\0' )
			break;
		
		if( str_prefix( onegrant, buf ) )
		{
			sprintf( newgrant, "%s %s", newgrant, onegrant );
		}
	}
	victim->grant = str_dup( newgrant );
    }	
    
    return;

}

void do_undelete( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char cmd[MAX_STRING_LENGTH];
    char name[MAX_STRING_LENGTH];
    FILE *fp;
    
    argument = one_argument( argument, name );
    
    
    if( name[0] == '\0' )
    {
    	send_to_char( "Syntax: undelete <char>\n\r", ch );
    	return;
    }
    
    sprintf( buf, "%s%s", PLAYER_BAK, capitalize( name ) );
    
    if( !(fp = fopen( buf, "r")) )
    {
    	send_to_char( "%s's file does not exist.\n\r", ch );
    	fclose( fp );
    	return;
    }
    else
	fclose( fp );
	

    sprintf( cmd, "mv %s %s%s", buf, PLAYER_DIR, capitalize( name ) );
    system( cmd );
    
    send_to_char( "Player restored.\n\r", ch );
    
    return;
}

void do_mudstatus(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    extern bool wizlock;
    extern bool newlock;
    extern char wlreason[MAX_INPUT_LENGTH];
    extern char nlreason[MAX_INPUT_LENGTH];
    
    strcpy(buf, "\n\r     {YCurrent Mud Status\n\r");
    strcat(buf, "     ------------------{x\n\r");
    send_to_char(buf, ch);
    sprintf(buf, "     {wNewlock       %s{x\n\r", newlock ? "{RON" : "{GOFF");
    send_to_char(buf, ch);
    if(nlreason[0] != '\0')
    {
    	sprintf(buf, "     {wReason: {M%s{x", nlreason);
    	send_to_char(buf, ch);
    }
    sprintf(buf, "     {wWizlock       %s{x\n\r", wizlock ? "{RON" : "{GOFF");
    send_to_char(buf, ch);
    if(wlreason[0] != '\0')
    {
    	sprintf(buf, "     {wReason: {M%s{x", wlreason);
    	send_to_char(buf, ch);
    }
}

void do_clearquest(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *wch;
    char arg1[MAX_INPUT_LENGTH];
    
    argument = one_argument(argument, arg1);
    
    if(!str_cmp(arg1, "all"))
    {
    	DESCRIPTOR_DATA *d;
    	
    	for(d = descriptor_list; d != NULL; d = d->next)
    	{
    	    if(d->connected == CON_PLAYING)
    	    {
    	    	wch = d->character;
    	    	REMOVE_BIT(wch->act, PLR_QUESTOR);
    	    	wch->questgiver = NULL;
    	    	wch->countdown = 0;
    	    	wch->questmob = 0;
    	    	wch->questobj = 0;
    	    	wch->nextquest = 0;
    	    	send_to_char("You get the urge to stop questing.\n\r", wch);
    	    }
    	}
    	return;
    }
    
    if((wch = get_char_world(ch, arg1)) == NULL)
    {
    	send_to_char("They aren't here.\n\r", ch);
    	return;
    }
    
    if(IS_NPC(wch))
    {
    	send_to_char("Not on NPCs.\n\r", ch);
    	return;
    }
    
    REMOVE_BIT(wch->act, PLR_QUESTOR);
    wch->questgiver = NULL;
    wch->countdown = 0;
    wch->questmob = 0;
    wch->questobj = 0;
    wch->nextquest = 0;
    send_to_char("You get the urge to stop questing.\n\r", wch);
    send_to_char("Ok.\n\r", ch);
}

void do_multi(CHAR_DATA *ch, char *argument)
{
    BUFFER *output;
    CHAR_DATA *wch, *mch;
    DESCRIPTOR_DATA *d;
    bool found = FALSE;
    char nameList[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
    
    strcpy(nameList, " ");
    output = new_buf();
    add_buf(output, "The following players share the same host: \n\r");
    
    for(d = descriptor_list; d != NULL; d = d->next)
    {
        if(d->connected != CON_PLAYING)
            continue;
            
        wch = d->character ? d->character : d->original;
        
        if(is_name(wch->name, nameList))
            continue;
        
        if((mch = check_multi(wch)) != NULL)
        {
            strcat(nameList, mch->name);
            strcat(nameList, " ");
            if(!can_see(ch, wch) || !can_see(ch, mch))
                continue;
            sprintf(buf, "%s%-14s{x <-> %s%-14s{x --> %s\n\r",
            IS_IMMORTAL(wch) ? "{R" : "",
            wch->name,
            IS_IMMORTAL(mch) ? "{R" : "",
            mch->name,
            wch->desc->host);
            add_buf(output, buf);
            found = TRUE;
        }
    }
    
    if(!found)
        add_buf(output, "None.\n\r");
    
    page_to_char(buf_string(output), ch);
}

void do_rp (CHAR_DATA *ch, char *argument)
{
	char arg[MIL];

    	argument = one_argument(argument,arg);

	if (arg[0] == '\0' || (str_cmp(arg, "on") && str_cmp(arg, "off")))
        {
  		send_to_char ("{CDo you want RolePlaying on or off?{x\n\r", ch);
                return;
	}
        
        if (!str_cmp(arg, "on"))
	{
	    send_to_char ("{CRolePlaying is now on{x.\n\r", ch);
	    SET_BIT (ch->act, PLR_ROLEPL);
	}

	if (!str_cmp(arg, "off"))
	{
	    send_to_char ("{CRolePlaying is now off{x.\n\r", ch);
	    REMOVE_BIT (ch->act, PLR_ROLEPL);
        }
}

void do_roomwiz(CHAR_DATA *ch, char* argument)
{
    if(IS_SET(ch->pcdata->immflag, IMMF_ROOMWIZ))
    {
        REMOVE_BIT(ch->pcdata->immflag, IMMF_ROOMWIZ);
        send_to_char("RoomWIZI off.\n\r", ch);
    }
    else
    {
        SET_BIT(ch->pcdata->immflag, IMMF_ROOMWIZ);
        send_to_char("RoomWIZI on.\n\r", ch);
    }
}
void do_arealinks(CHAR_DATA *ch, char *argument)
{
    FILE *fp;
    BUFFER *buffer;
    AREA_DATA *parea;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;
    ROOM_INDEX_DATA *from_room;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int vnum = 0;
    int iHash, door;
    bool found = FALSE;

    /* To provide a convenient way to translate door numbers to words */
    static char * const dir_name[] = 
{"north","east","south","west","up","down"};

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    /* First, the 'all' option */
    if (!str_cmp(arg1,"all"))
    {
	/*
	 * If a filename was provided, try to open it for writing
	 * If that fails, just spit output to the screen.
	 */
	if (arg2[0] != '\0')
	{
	    fclose(fpReserve);
	    if( (fp = fopen(arg2, "w")) == NULL)
	    {
		send_to_char("Error opening file, printing to screen.\n\r",ch);
		fclose(fp);
		fpReserve = fopen(NULL_FILE, "r");
		fp = NULL;
	    }
	}
	else
	    fp = NULL;

	/* Open a buffer if it's to be output to the screen */
	if (!fp)
	    buffer = new_buf();

	/* Loop through all the areas */
	for (parea = area_first; parea != NULL; parea = parea->next)
	{
	    /* First things, add area name  and vnums to the buffer */
	    sprintf(buf, "*** %s (%d to %d) ***\n\r",
			 parea->name, parea->min_vnum, parea->max_vnum);
	    fp ? fprintf(fp, buf) : add_buf(buffer, buf);

	    /* Now let's start looping through all the rooms. */
	    found = FALSE;
	    for(iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	    {
		for(from_room = room_index_hash [iHash];
		    from_room != NULL;
		    from_room = from_room->next )
		{
		    /*
		     * If the room isn't in the current area,
		     * then skip it, not interested.
		     */
		    if ( from_room->vnum < parea->min_vnum
		    ||   from_room->vnum > parea->max_vnum )
			continue;

		    /* Aha, room is in the area, lets check all directions 
*/
		    for (door = 0; door < 6; door++)
		    {
			/* Does an exit exist in this direction? */
			if( (pexit = from_room->exit[door]) != NULL )
			{
			    to_room = pexit->u1.to_room;

			    /*
			     * If the exit links to a different area
			     * then add it to the buffer/file
			     */
			    if( to_room != NULL
			    &&  (to_room->vnum < parea->min_vnum
			    ||   to_room->vnum > parea->max_vnum) )
			    {
				found = TRUE;
				sprintf(buf, "    (%d) links %s to %s (%d)\n\r",
				    from_room->vnum, dir_name[door],
				    to_room->area->name, to_room->vnum);

				/* Add to either buffer or file */
				if(fp == NULL)
				    add_buf(buffer, buf);
				else
				    fprintf(fp, buf);
			    }
			}
		    }
		}
	    }

	    /* Informative message for areas with no external links */
	    if (!found)
		add_buf(buffer, "    No links to other areas found.\n\r");
	}

	/* Send the buffer to the player */
	if (!fp)
	{
	    page_to_char(buf_string(buffer), ch);
	    free_buf(buffer);
	}
	/* Or just clean up file stuff */
	else
	{
	    fclose(fp);
	    fpReserve = fopen(NULL_FILE, "r");
	}

	return;
    }

    /* No argument, let's grab the char's current area */
    if(arg1[0] == '\0')
    {
	parea = ch->in_room ? ch->in_room->area : NULL;

	/* In case something wierd is going on, bail */
	if (parea == NULL)
	{
	    send_to_char("You aren't in an area right now, funky.\n\r",ch);
	    return;
	}
    }
    /* Room vnum provided, so lets go find the area it belongs to */
    else if(is_number(arg1))
    {
	vnum = atoi(arg1);

	/* Hah! No funny vnums! I saw you trying to break it... */
	if (vnum <= 0 || vnum > 65536)
	{
	    send_to_char("The vnum must be between 1 and 65536.\n\r",ch);
	    return;
	}

	/* Search the areas for the appropriate vnum range */
	for (parea = area_first; parea != NULL; parea = parea->next)
	{
	    if(vnum >= parea->min_vnum && vnum <= parea->max_vnum)
		break;
	}

	/* Whoops, vnum not contained in any area */
	if (parea == NULL)
	{
	    send_to_char("There is no area containing that vnum.\n\r",ch);
	    return;
	}
    }
    /* Non-number argument, must be trying for an area name */
    else
    {
	/* Loop the areas, compare the name to argument */
	for(parea = area_first; parea != NULL; parea = parea->next)
	{
	    if(!str_prefix(arg1, parea->name))
		break;
	}

	/* Sorry chum, you picked a goofy name */
	if (parea == NULL)
	{
	    send_to_char("There is no such area.\n\r",ch);
	    return;
	}
    }

    /* Just like in all, trying to fix up the file if provided */
    if (arg2[0] != '\0')
    {
	fclose(fpReserve);
	if( (fp = fopen(arg2, "w")) == NULL)
	{
	    send_to_char("Error opening file, printing to screen.\n\r",ch);
	    fclose(fp);
	    fpReserve = fopen(NULL_FILE, "r");
	    fp = NULL;
	}
    }
    else
	fp = NULL;

    /* And we loop the rooms */
    for(iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
	for(from_room = room_index_hash [iHash];
	     from_room != NULL;
	     from_room = from_room->next )
	{
	    /* Gotta make sure the room belongs to the desired area */
	    if ( from_room->vnum < parea->min_vnum
	    ||   from_room->vnum > parea->max_vnum )
		continue;

	    /* Room's good, let's check all the directions for exits */
	    for (door = 0; door < 6; door++)
	    {
		if( (pexit = from_room->exit[door]) != NULL )
		{
		    to_room = pexit->u1.to_room;

		    /* Found an exit, does it lead to a different area? */
		    if( to_room != NULL
		    &&  (to_room->vnum < parea->min_vnum
		    ||   to_room->vnum > parea->max_vnum) )
		    {
			found = TRUE;
			sprintf(buf, "%s (%d) links %s to %s (%d)\n\r",
				    parea->name, from_room->vnum, 
dir_name[door],
				    to_room->area->name, to_room->vnum);

			/* File or buffer output? */
			if(fp == NULL)
			    send_to_char(buf, ch);
			else
			    fprintf(fp, buf);
		    }
		}
	    }
	}
    }

    /* Informative message telling you it's not externally linked */
    if(!found)
    {
	send_to_char("No links to other areas found.\n\r",ch);
	/* Let's just delete the file if no links found */
	if (fp)
	    unlink(arg2);
	return;
    }

    /* Close up and clean up file stuff */
    if(fp)
    {
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
    }

}

 
int days_since_last_file_mod( char *filename )
{
    int days;
    struct stat buf;
    extern time_t current_time;

    if (!stat(filename, &buf))
    {
	days = (current_time - buf.st_mtime)/86400;
    }
    else
	days = 0;

    return days;
}

void do_plist( CHAR_DATA *ch, char *argument )
{
    DIR *dp;
    struct dirent *ep;
    char buf[80];
    char buffer[MAX_STRING_LENGTH*4];
    int days;
    bool fAll = FALSE, fImmortal = FALSE, fBak = FALSE;

    buffer[0] = '\0';

    if ( argument[0] == '\0' || !str_cmp( argument, "all" ) )
    {
	fAll = TRUE;
    }
    else
    if ( !str_prefix( argument, "immortal" ) )
    {
	fImmortal = TRUE;
    }
    else
    if (!str_prefix( argument, "bak"))
    {
	fBak = TRUE;
    }
    else
    {
	send_to_char( "Syntax: PLIST [ALL/IMMORTAL/BAK]\n\r", ch );
	return;
    }

    if ( fImmortal )
    {
	/*
	 * change the string literals to GODS_DIR or where
	 * you keep your gods folder
	 */
	dp = opendir ("../gods");
    }
    else if ( fBak )
    {
	dp = opendir ("../player/bak");
    }
    else
    {
	/*
	 * change the string literals to PLAYER_DIR or where
	 * you keep your gods folder
	 */
	dp = opendir ("../player");
    }

    if (dp != NULL)
    {
 	while ( (ep = readdir (dp)) )
      	{
	    if ( ep->d_name[0] == '.' )
	    	continue;

	/*
	 * change the string literals to fit your needs like above
	 */
	    sprintf( buf, "%s%s", fImmortal ? "../gods/" : "../player/", ep->d_name );

	    days = days_since_last_file_mod( buf );

	/*
	 * color coding using Lopes comaptable color to highlight
	 * inactivity. green = active, red = innactive
	 * Just remove the colors if they cause problems.
	 */
	    sprintf( buf, "%-15s %s%-3d{x days\n\r",
		ep->d_name, days > 30 ? "{r" : days > 20 ? "{Y" : days > 10 ? "{g" : "{G", days );
	    strcat( buffer, buf );
      	}
      	closedir (dp);
    }
    else
    	perror ("Couldn't open the directory");

    page_to_char( buffer, ch );

  return;
}

#define ILLEGAL_NAME_FILE	"illegalnames.txt"

void do_illegalname( CHAR_DATA *ch, char *argument)
{
    char strsave[MIL], namelist[MSL], nameread[MSL], name[MSL];
    FILE *fp;
    if ( argument[0] == '\0' )
    {
	send_to_char( "syntax: badname <name>.\n\r", ch );
	return;
    }
    fclose( fpReserve );
    sprintf( strsave, "%s", ILLEGAL_NAME_FILE );
    sprintf(name, "%s\n", argument);
    sprintf(namelist,"%s","");
    if ( (fp = fopen( strsave, "r" ) ) != NULL )
    {
	for ( ; ; )
	{
            fscanf (fp, "%s", nameread);
	    if ( !str_cmp( nameread, "END" ) )
           	break;
	    else
	    {
		strcat(namelist, nameread);
		strcat(namelist,"\n");
	    }
	}
    }
    else
	fp = fopen( NULL_FILE, "r" );
    fclose( fp );
    fp = fopen( strsave, "w" );
    strcat( namelist, name );
    fprintf( fp, "%s", namelist );
    fprintf( fp, "END" );
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    send_to_char( "All set, that name is now illegal.\n\r",ch);
}


void do_wiziname( CHAR_DATA *ch, char *argument)
{
	if (!IS_NPC (ch)) {
		smash_tilde (argument);

		if (ch->pcdata->storedwiziname[0] == '\0') {
			send_to_char( "Error, you have no custom wiziname set for this character!\n\r",ch);
			free_string (ch->pcdata->wiziname);
			ch->pcdata->wiziname = str_dup (DEFAULT_WIZINAME);
			free_string (ch->pcdata->storedwiziname);
			ch->pcdata->storedwiziname = str_dup (DEFAULT_WIZINAME);
			send_to_char("Default name set\n\r",ch);
			ch->pcdata->wizinametoggle = 0;
			return;
		}

		if (argument[0] == '\0') {
			send_to_char("Wiziname toggled.\n\r",ch);
			switch (ch->pcdata->wizinametoggle) {
				case 0:
					ch->pcdata->wizinametoggle = 1;
					send_to_char("Wiziname Set\n\r",ch);
					free_string (ch->pcdata->wiziname);
					ch->pcdata->wiziname = str_dup (DEFAULT_WIZINAME);
					break;

				default:
					ch->pcdata->wizinametoggle = 0;
					send_to_char("Wiziname now set to mud default\n\r", ch);
					free_string (ch->pcdata->wiziname);
					ch->pcdata->wiziname = str_dup (ch->pcdata->storedwiziname);
					send_to_char("Your stored wiziname has not been lost.",ch);
			}
		} else {
			free_string (ch->pcdata->wiziname);
			free_string (ch->pcdata->storedwiziname);
			ch->pcdata->wiziname = str_dup (argument);
			ch->pcdata->storedwiziname = str_dup (argument);

			ch->pcdata->wizinametoggle = 1;
			send_to_char("Wiziname now set to your custom wiziname\n\r", ch);
		}
	}
	return;
}

void do_unlink( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *linkdead;
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    bool found = FALSE;
    int i = 0;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Unlink <all> or Unlink <player>\n\r", ch );
        send_to_char( "\n\r{C================================================{x\n\r\n\r",ch );
     
        for ( wch = char_list; wch != NULL; wch = wch_next )
        {
             wch_next = wch->next;
            
             if (IS_NPC( wch ) )
                 continue;        

             if ( wch->desc == NULL )
		{
		 printf_to_char( ch, "{R%s, Is LinkDead{x\n\r", 
wch->name);
                 i++;
		}
	     else
		{
		 printf_to_char( ch, "{Y%s{x, Is {C NOT LinkDead{x\n\r", 
wch->name);
		}
        }

        send_to_char( "\n\r{C================================================{x\n\r\n\r",ch );
        printf_to_char( ch, "The number of linkdead people on the mud is %d.\n\r", i );
        return;    
    }

    if  ( !str_cmp( argument, "all" ) )
    {
        for ( wch = char_list;  wch != NULL; wch = wch_next )
        {
              wch_next = wch->next;

              if ( IS_NPC( wch ) )
                  continue;

              if ( wch->desc == NULL )
              {
                  found = TRUE;
                  do_function( wch, &do_quit, "" );
              }
        } 

        if ( found )
            send_to_char( "Extracting all linkdead players.\n\r", ch );

        if ( !found )
        send_to_char( "No linkdead players were found!\n\r", ch );
        return;
    }

    if ((linkdead = get_char_world(ch,argument)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }
    
    if ( linkdead->desc != NULL )
    {
        send_to_char( "They aren't linkdead just use disconnect instead.\n\r", ch );
        return;
    }

    else
    {
        do_function( linkdead, &do_quit, "" );
        send_to_char("They have been booted.\n\r", ch);
        return;
    }

}

void do_otype(CHAR_DATA *ch, char *argument)
{
    int  		type;
    int  		type2;
    int  		vnum = 1;
    char 		buf[MAX_STRING_LENGTH];
    char 		buffer[12 * MAX_STRING_LENGTH];
    char 		arg1[MAX_INPUT_LENGTH];
    char 		arg2[MAX_INPUT_LENGTH];
    char 		*item;
    OBJ_INDEX_DATA 	*obj;
    bool		found;

    item = one_argument(argument, arg1);
    one_argument( item, arg2 );

    found = FALSE;
    buffer[0] = '\0';

    if(arg1[0] == '\0')
    {
	send_to_char("Incorrect Ussage\n\r",ch);
	return;
    }
    else if(!str_cmp(arg1,"armor"))
	{
	   type = flag_value(type_flags, arg1);
	   if((type2 = flag_value(wear_flags,arg2)) == NO_FLAG)
	   {
		send_to_char("No such armor type.\n\r",ch);
		return;
	   }
	}
    else if(!str_cmp(arg1,"weapon"))
	{
	   type = flag_value(type_flags,arg1);
	   if((type2=flag_value(weapon_class,arg2)) == NO_FLAG)
	   {
		send_to_char("No such weapon type.\n\r",ch);
		return;
	   }
	}
    else
	{
	   if((type=flag_value(type_flags,arg1)) == NO_FLAG)
	   {
		send_to_char("Unkown Type.\n\r",ch);
		return;
	   }
	}

    for(;vnum <= top_vnum_obj; vnum++)
	{
	   if((obj=get_obj_index(vnum)) != NULL)
	   {
	    if((obj->item_type == type && type2 == 0
		&& str_cmp(arg1,"weapon") && str_cmp(arg1,"armor"))
		|| (obj->item_type == type && obj->value[0] == type2
		&& str_cmp(arg1,"armor"))
		|| (obj->item_type == type && IS_SET(obj->wear_flags,type2)
		&& str_cmp(arg1,"weapon")))
	    {
		sprintf(buf, "%35s - %5d - Area [%d]\n\r",obj->short_descr,vnum,obj->area->vnum);
	        found = TRUE;
		strcat(buffer,buf);
	    }
	   }
	}
    if(!found)
	send_to_char("No objects of that type exist\n\r",ch);
    else
	if(ch->lines)
	   page_to_char(buffer,ch);
	else
	   send_to_char(buffer,ch);
}
