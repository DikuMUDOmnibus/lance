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
 **************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1998 Russ Taylor			   
*
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@hypercube.org)				   *
*	    Gabrielle Taylor (gtaylor@hypercube.org)			   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"

char* ooc1;
char* ooc2;
char* ooc3;
char* ooc4;
char* ooc5;
char* ooc6;
char* ooc7;
char* ooc8;
char* ooc9;


void channel_stuff(CHAR_DATA *ch, char *argument, int type);

char *to_gnome(CHAR_DATA* ch, char* argument)
{
    char buf[MAX_INPUT_LENGTH];

    if(!IS_NPC(ch) && !str_cmp(pc_race_table[ch->race].name, "gnome"))
    {
    	int count;

    	for(count = 0; *argument != '\0'; argument++)
    	{
    	    if(isalnum(*argument) || *argument == '-' || *argument == '*'
    	    || *argument == '!' || *argument == '?')
    	    {
    	    	buf[count++] = *argument;
    	    }
    	}
    	buf[count] = '\0';
    	strcpy(argument, buf);
    }
    return argument;
}

/* RT code to delete yourself */
void do_delet( CHAR_DATA *ch, char *argument)
{
    send_to_char("You must type the full command to delete yourself.\n\r",ch);
}

void do_delete( CHAR_DATA *ch, char *argument)
{
   char strsave[MAX_INPUT_LENGTH];

   if (IS_NPC(ch))
	return;
  
   if (ch->pcdata->confirm_delete)
   {
	if (argument[0] != '\0')
	{
	    send_to_char("Delete status removed.\n\r",ch);
	    ch->pcdata->confirm_delete = FALSE;
	    return;
	}
	else
	{
    	    sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
	    wiznet("$N turns $Mself into line noise.",ch,NULL,0,0,0);
	    stop_fighting(ch,TRUE);
	    do_function(ch, &do_quit, "");
	    
	    save_char_obj( ch );	    
	    
	    unlink(strsave);
	    return;
 	}
    }

    if (argument[0] != '\0')
    {
	send_to_char("Just type delete. No argument.\n\r",ch);
	return;
    }

    send_to_char("Type delete again to confirm this command.\n\r",ch);
    send_to_char("WARNING: this command is irreversible.\n\r",ch);
    send_to_char("Typing delete with an argument will undo delete status.\n\r",
	ch);
    ch->pcdata->confirm_delete = TRUE;
    wiznet("$N is contemplating deletion.",ch,NULL,0,0,get_trust(ch));
}
	    

void show_channels(CHAR_DATA *me)
{
    DESCRIPTOR_DATA *d;
    BUFFER *buf;
    char line[MAX_STRING_LENGTH];
    buf = new_buf();
    sprintf(line, "%-14s Channels Off\n\r", "Name");
    add_buf(buf, line);
    add_buf(buf,"---------------------------------------------------------------------------\n\r");
    
    for(d = descriptor_list; d != NULL; d = d->next)
    {
        CHAR_DATA *ch;
        if(d->connected != CON_PLAYING)
            continue;
            
        ch = d->character ? d->character : d->original;
        if(!can_see(me, ch))
            continue;
        
        // Check channel information for this character.
        sprintf(line, "%-14s %s%s%s%s%s%s%s%s%s%s\n\r",
            ch->name,
            IS_SET(ch->comm, COMM_NOOOC) ? "[{MOOC{x]" : "",
            IS_SET(ch->comm, COMM_NOOSAY) ? "[{Wosay{x]" : "",
            IS_SET(ch->comm, COMM_NOIC) ? "[{GIC{x]" : "",
            IS_SET(ch->comm, COMM_NOAUCTION) ? "[{YAuc{x]" : "",
            IS_SET(ch->comm, COMM_NOMUSIC) ? "[{mMus{x]" : "",
            IS_SET(ch->comm, COMM_NOGRATS) ? "[{RGRAT{x]" : "",
            IS_SET(ch->comm, COMM_DEAF) ? "[{yTell{x]" : "",
            IS_SET(ch->comm, COMM_QUIET) ? "[{DQuiet{x]" : "",
            IS_IMMORTAL(me) ? (IS_SET(ch->comm, COMM_NOWIZ) ? "[{CImm{x]" : "") : "",
            (is_clan(ch) || !clan_table[ch->clan].independent) ? 
                (IS_SET(ch->comm, COMM_NOCLAN) ? "[{BCLAN{x]" : "") : ""
            );
        add_buf(buf, line);
    }
    
    page_to_char(buf_string(buf), me);
    send_to_char("\n\rFor your old channel information, type 'channels self'.\n\r", me);
}

/* RT code to display channel status */

void do_channels( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if(str_cmp(argument, "self"))
    {
        show_channels(ch);
        return;
    }
    /* lists all channels and their status */
    send_to_char(" {cchannel        {Wstatus{x\n\r",ch);


send_to_char("{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{x\n\r",ch);
 
    send_to_char(" {Mooc{x            ",ch);
    if (!IS_SET(ch->comm,COMM_NOOOC))
      send_to_char("{YON{x\n\r",ch);
    else
      send_to_char("{DOFF{x\n\r",ch);
      
    send_to_char(" {Wosay{x           ",ch);
    if (!IS_SET(ch->comm,COMM_NOOSAY))
      send_to_char("{YON{x\n\r",ch);
    else
      send_to_char("{DOFF{x\n\r",ch);

    send_to_char(" {Gic{x             ",ch);
    if (!IS_SET(ch->comm,COMM_NOIC))
      send_to_char("{YON{x\n\r",ch);
    else
      send_to_char("{DOFF{x\n\r",ch);
    send_to_char(" {Yauction{x        ",ch);
    if (!IS_SET(ch->comm,COMM_NOAUCTION))
      send_to_char("{YON{x\n\r",ch);
    else
      send_to_char("{DOFF{x\n\r",ch);

    send_to_char(" {cmusic{x          ",ch);
    if (!IS_SET(ch->comm,COMM_NOMUSIC))
      send_to_char("{YON{x\n\r",ch);
    else
      send_to_char("{DOFF{x\n\r",ch);

    send_to_char(" {Rgrats{x          ",ch);
    if (!IS_SET(ch->comm,COMM_NOGRATS))
      send_to_char("{YON{x\n\r",ch);
    else
      send_to_char("{DOFF{x\n\r",ch);

    if (!is_clan(ch) || clan_table[ch->clan].independent)
    ;
    else
    {
      send_to_char(" {Cclantalk{x       ",ch);
      if(!IS_SET(ch->comm,COMM_NOCLAN))
        send_to_char("{YON{x\n\r",ch);
      else
        send_to_char("{DOFF{x\n\r",ch);
    }

    if (IS_IMMORTAL(ch))
    {
      send_to_char(" {Cimmtalk{x        ",ch);
      if(!IS_SET(ch->comm,COMM_NOWIZ))
        send_to_char("{YON{x\n\r",ch);
      else
        send_to_char("{DOFF{x\n\r",ch);
    }

/*
    send_to_char(" {rshouts{x         ",ch);
    if (!IS_SET(ch->comm,COMM_SHOUTSOFF))
      send_to_char("{YON{x\n\r",ch);
    else
      send_to_char("{DOFF{x\n\r",ch);
*/

    send_to_char(" {ytells{x          ",ch);
    if (!IS_SET(ch->comm,COMM_DEAF))
	send_to_char("{YON{x\n\r",ch);
    else
	send_to_char("{DOFF{x\n\r",ch);

    send_to_char(" quiet mode     ",ch);
    if (IS_SET(ch->comm,COMM_QUIET))
      send_to_char("{YON{x\n\r",ch);
    else
      send_to_char("{DOFF{x\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOSHOUT))
      send_to_char(" You cannot {rshout{x.\n\r",ch);
        
    if (IS_SET(ch->comm,COMM_NOTELL))
      send_to_char(" You cannot use {ytell{x.\n\r",ch);

    if (IS_SET(ch->comm,COMM_NOEMOTE))
      send_to_char(" You cannot use emote.\n\r",ch);
        
    if (IS_SET(ch->comm,COMM_NOCHANNELS))
      send_to_char(" {RYou cannot use any channels!{x\n\r",ch);

    if (IS_SET(ch->comm,COMM_IAW))
      send_to_char(" You are {yIAW{x. (in another window)\n\r",ch);

    if (IS_SET(ch->comm,COMM_AFK))
	send_to_char(" You are {RAFK{x.\n\r",ch);

    if (IS_SET(ch->comm,COMM_SNOOP_PROOF))
	send_to_char(" You are immune to snooping.\n\r",ch);
   
    if (ch->lines != PAGELEN)
    {
	if (ch->lines)
	{
	    sprintf(buf," {WYou display {m%d{W lines of scroll.{x\n\r",ch->lines+2);
	    send_to_char(buf,ch);
 	}
	else
	    send_to_char(" Scroll buffering is off.\n\r",ch);
    }

    if (ch->prompt != NULL)
    {
	sprintf(buf," Your prompt is: %s\n\r",ch->prompt);
	send_to_char(buf,ch);
    }

send_to_char("{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{x\n\r",ch);
}

/* RT deaf blocks out all shouts */

void do_deaf( CHAR_DATA *ch, char *argument)
{
    
   if (IS_SET(ch->comm,COMM_DEAF))
   {
     send_to_char("You can now hear tells again.\n\r",ch);
     REMOVE_BIT(ch->comm,COMM_DEAF);
   }
   else 
   {
     send_to_char("From now on, you won't hear tells.\n\r",ch);
     SET_BIT(ch->comm,COMM_DEAF);
   }
}

/* RT quiet blocks out all communication */

void do_quiet ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_QUIET))
    {
      send_to_char("{YQuiet {bmode {mremoved{x.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_QUIET);
    }
   else
   {
     send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
     SET_BIT(ch->comm,COMM_QUIET);
   }
}

/* afk command */

void do_afk ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_AFK))
    {
      send_to_char("AFK mode removed. Type 'replay' to see tells.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_AFK);
    }
   else
   {
     send_to_char("You are now in AFK mode.\n\r",ch);
     SET_BIT(ch->comm,COMM_AFK);
   }
}

void do_iaw ( CHAR_DATA *ch, char * argument)
{
    if (IS_SET(ch->comm,COMM_IAW))
    {
	send_to_char("You aren't in another window anymore.\n\r",ch);
	REMOVE_BIT(ch->comm,COMM_IAW);
    }
    else
    {
	send_to_char("You are now in another window.\n\r",ch);
	SET_BIT(ch->comm,COMM_IAW);
    }
}



void do_replay (CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
	send_to_char("You can't replay.\n\r",ch);
	return;
    }

    if (buf_string(ch->pcdata->buffer)[0] == '\0')
    {
	send_to_char("You have no tells to replay.\n\r",ch);
	return;
    }

    page_to_char(buf_string(ch->pcdata->buffer),ch);
    clear_buf(ch->pcdata->buffer);
}

/* RT auction rewritten in ROM style */
void do_auction( CHAR_DATA *ch, char *argument )
{
	channel_stuff(ch, argument, CHAN_AUCTION);
}

void channel_stuff( CHAR_DATA *ch, char *argument, int type )
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
        if (IS_SET(ch->comm,COMM_QUIET))
        {
          send_to_char("You must turn off quiet mode first.\n\r",ch);
          return;
        }

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
          send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;

        }

    if (argument[0] == '\0' )
    {
	switch( type )
	{
		case CHAN_OOC:
			if (IS_SET(ch->comm,COMM_NOOOC))
      			{
      			  send_to_char("OOC channel is now ON.\n\r",ch);
     			  REMOVE_BIT(ch->comm,COMM_NOOOC);
			}
      			else
      			{
        		send_to_char("OOC channel is now OFF.\n\r",ch);
        		SET_BIT(ch->comm,COMM_NOOOC);
      			}
			break;
		case CHAN_IC:
		      if (IS_SET(ch->comm,COMM_NOIC))
		      {
		        send_to_char("IC channels are now ON.\n\r",ch);
		        REMOVE_BIT(ch->comm,COMM_NOIC);
		      }
		      else
		      {
		        send_to_char("IC channels are now OFF.\n\r",ch);
		        SET_BIT(ch->comm,COMM_NOIC);
		      }
			break;
		case CHAN_IC2:
		      if (IS_SET(ch->comm,COMM_NOIC))
		      {
		        send_to_char("IC channels are now ON.\n\r",ch);
		        REMOVE_BIT(ch->comm,COMM_NOIC);
		      }
		      else
		      {
		        send_to_char("IC channels are now OFF.\n\r",ch);
		        SET_BIT(ch->comm,COMM_NOIC);
		      }
			break;
		case CHAN_GRATS:
		      if (IS_SET(ch->comm,COMM_NOGRATS))
		      {
		        send_to_char("Grats channel is now ON.\n\r",ch);
		        REMOVE_BIT(ch->comm,COMM_NOGRATS);
		      }
		      else
		      {
		        send_to_char("Grats channel is now OFF.\n\r",ch);
		        SET_BIT(ch->comm,COMM_NOGRATS);
		      }
			break;
		case CHAN_MUSIC:
		      if (IS_SET(ch->comm,COMM_NOMUSIC))
		      {
		        send_to_char("Music channel is now ON.\n\r",ch);
		        REMOVE_BIT(ch->comm,COMM_NOMUSIC);
		      }
		      else
		      {
		        send_to_char("Music channel is now OFF.\n\r",ch);
		        SET_BIT(ch->comm,COMM_NOMUSIC);
		      }
			break;
		case CHAN_IMM:
		    if (ch->level < 102 )
		    {
		    send_to_char("Huh?",ch);
		    return;
		    }
		      if (IS_SET(ch->comm,COMM_NOWIZ))
		      {
		        send_to_char("IC channel is now ON.\n\r",ch);
		        REMOVE_BIT(ch->comm,COMM_NOWIZ);
		      }
		      else
		      {
		        send_to_char("IC channel is now OFF.\n\r",ch);
		        SET_BIT(ch->comm,COMM_NOWIZ);
		      }
			break;
		case CHAN_AUCTION:
		      if (IS_SET(ch->comm,COMM_NOAUCTION))
		      {
		        send_to_char("Auction channel is now ON.\n\r",ch);
		        REMOVE_BIT(ch->comm,COMM_NOAUCTION);
		      }
		      else
		      {
		        send_to_char("Auction channel is now OFF.\n\r",ch);
		        SET_BIT(ch->comm,COMM_NOAUCTION);
		      }
		}
	return;
	}
	else
	{
	switch(type)
	{
		case CHAN_AUCTION:
			sprintf( buf, "You auction '{Y%s{x'{x\n\r", argument );
		        send_to_char( buf, ch );
			break;
		case CHAN_OOC:
		      REMOVE_BIT(ch->comm,COMM_NOOOC);

		      sprintf( buf, "You OOC '{M%s{x'\n\r", argument );
		      send_to_char( buf, ch );
			break;
		case CHAN_IC:
		    REMOVE_BIT(ch->comm,COMM_NOIC);

		    argument = to_gnome(ch, argument);
		    sprintf( buf, "You IC '{G%s{x'{x\n\r", argument );
		    send_to_char( buf, ch );
			break;
		case CHAN_IC2:
		    REMOVE_BIT(ch->comm,COMM_NOIC);

		    argument = to_gnome(ch, argument);
		    sprintf( buf, "You IC '{C%s{x'{x\n\r", argument );
		    send_to_char( buf, ch );
			break;
		case CHAN_GRATS:
		      REMOVE_BIT(ch->comm,COMM_NOGRATS);

		      sprintf( buf, "You grats '{R%s{x'{x\n\r", argument );
		      send_to_char( buf, ch );
			break;
		case CHAN_MUSIC:
		        REMOVE_BIT(ch->comm,COMM_NOMUSIC);

		      sprintf( buf, "(You) {BMUSIC:{x '{c%s{x'{x\n\r", argument );
		      send_to_char( buf, ch );
			break;
		case CHAN_IMM:
		    REMOVE_BIT(ch->comm,COMM_NOWIZ);
	}
   for ( d = descriptor_list; d != NULL; d = d->next )
      {
        CHAR_DATA *victim;

        victim = d->character;

	switch(type)
	{
	  case CHAN_OOC:
		if ( d->connected == CON_PLAYING &&
        	     victim != ch &&
        	     !IS_SET(victim->comm,COMM_NOOOC) &&
        	     !IS_SET(victim->comm,COMM_QUIET)
        	     && victim->position >= POS_SLEEPING )
        	{
		if(!IS_IMMORTAL(ch))
		{
        	    sprintf(buf, "%s OOCs '{M%s{x'\n\r",
        	    can_see(victim, ch) ? (!IS_NPC(ch) ? ch->name : ch->short_descr) : "Someone"  , argument);
        	    send_to_char(buf, victim);
		}
		else
		{
        	    sprintf(buf, "%s OOCs '{M%s{x'\n\r",
        	    can_see(victim, ch) ? (!IS_NPC(ch) ? ch->name : ch->short_descr) : ch->pcdata->wiziname  , argument);
        	    send_to_char(buf, victim);
		}
              }	
			break;
	  case CHAN_IC:
		if ( d->connected == CON_PLAYING &&
        	     victim != ch &&
        	     !IS_SET(victim->comm,COMM_NOIC) &&
        	     !IS_SET(victim->comm,COMM_QUIET)
        	     && victim->position >= POS_SLEEPING )
        	{
        	if(!IS_IMMORTAL(ch))
		{
		    sprintf(buf, "%s ICs '{G%s{x'\n\r",
        	    can_see(victim, ch) ? (!IS_NPC(ch) ? ch->name : ch->short_descr) : "Someone", argument);
        	    send_to_char(buf, victim);
		}
		else
		{
        	    sprintf(buf, "%s ICs '{G%s{x'\n\r",
        	    can_see(victim, ch) ? (!IS_NPC(ch) ? ch->name : ch->short_descr) : ch->pcdata->wiziname, argument);
        	    send_to_char(buf, victim);
		}
        	}
			break;
	  case CHAN_IC2:
		if ( d->connected == CON_PLAYING &&
        	     victim != ch &&
        	     !IS_SET(victim->comm,COMM_NOIC) &&
        	     !IS_SET(victim->comm,COMM_QUIET)
        	     && victim->position >= POS_SLEEPING )
        	{
        	    sprintf(buf, "%s ICs '{C%s{x'\n\r",
        	    can_see(victim, ch) ? (!IS_NPC(ch) ? ch->name : ch->short_descr) : "Someone", argument);
        	    send_to_char(buf, victim);
        	}
			break;
	  case CHAN_MUSIC:
	        if ( d->connected == CON_PLAYING &&
	             victim != ch &&
	             !IS_SET(victim->comm,COMM_NOMUSIC) &&
	             !IS_SET(victim->comm,COMM_QUIET)
	             && victim->position >= POS_SLEEPING )
	        {
			if(!IS_IMMORTAL(ch))
			{
	            sprintf( buf, "{B%s MUSIC:{x '{c%s{x'\n\r",
	            can_see(victim, ch) ? ch->name : "Someone", argument );
	            send_to_char(buf, victim);
			}
			else
			{
	            sprintf( buf, "{B%s MUSIC:{x '{c%s{x'\n\r",
	            can_see(victim, ch) ? ch->name : ch->pcdata->wiziname , argument );
	            send_to_char(buf, victim);
			}
        	}
			break;
	  case CHAN_GRATS:
		if ( d->connected == CON_PLAYING &&
        	     victim != ch &&
        	     !IS_SET(victim->comm,COMM_NOGRATS) &&
        	     !IS_SET(victim->comm,COMM_QUIET)
        	     && victim->position >= POS_SLEEPING )
        	{
		if(!IS_IMMORTAL(ch))
		{
        	    sprintf(buf, "%s grats '{R%s{x'\n\r",
        	    can_see(victim, ch) ? (!IS_NPC(ch) ? ch->name : ch->short_descr) : "Someone", argument);
        	    send_to_char(buf, victim);
		}
		else
		{
        	    sprintf(buf, "%s grats '{R%s{x'\n\r",
        	    can_see(victim, ch) ? (!IS_NPC(ch) ? ch->name : ch->short_descr) : ch->pcdata->wiziname, argument);
        	    send_to_char(buf, victim);
		}
        	}
			break;
	  case CHAN_IMM:
	if ( d->connected == CON_PLAYING &&
	     IS_IMMORTAL(victim) &&
             !IS_SET(victim->comm,COMM_NOWIZ)
             && victim->position >= POS_DEAD )
	{
	    char buf2[MAX_STRING_LENGTH];
	    if(can_see(victim, ch) && (ch->invis_level > 0 || ch->incog_level > 0))
	        sprintf(buf2,"(%d)", UMAX(ch->invis_level, ch->incog_level));
	    else
	        sprintf(buf2, "%s", "");
            sprintf(buf, "%s{W[{C%s{W]: {C%s{x\n\r",
            buf2,
            can_see(victim, ch) ? ch->name : ch->pcdata->wiziname , argument);
	    send_to_char(buf, victim);
	}
			break;
	case CHAN_AUCTION:
	if ( d->connected == CON_PLAYING &&
	     victim != ch &&
	     !IS_SET(victim->comm,COMM_NOAUCTION) &&
	     !IS_SET(victim->comm,COMM_QUIET)
	     && victim->position >= POS_DEAD)
        {
		if(!IS_IMMORTAL(ch))
		{
            sprintf(buf, "%s auctions '{Y%s{x'\n\r",
            can_see(victim, ch) ? ch->name : "Someone", argument);
	    send_to_char(buf, victim);
		}
		else
		{
            sprintf(buf, "%s auctions '{Y%s{x'\n\r",
            can_see(victim, ch) ? ch->name : ch->pcdata->wiziname, argument);
	    send_to_char(buf, victim);
		}
		
	}
			break;
   }
}
}
}

void do_ooc( CHAR_DATA *ch, char *argument )
{
	channel_stuff(ch, argument, CHAN_OOC );
}



void do_ic( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

	sprintf(buf, "{g%s ICs '{G%s{x'", ch->name, argument);
	ooc9 = ooc8;
	ooc8 = ooc7;
	ooc7 = ooc6;
	ooc6 = ooc5;
	ooc5 = ooc4;
	ooc4 = ooc3;
	ooc3 = ooc2;
	ooc2 = ooc1;
	ooc1 = str_dup(buf);
	channel_stuff(ch, argument, CHAN_IC );

}

void do_icaction( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

	sprintf(buf1, "{g<{GIC{g> %s %s{x", ch->name, argument);
	ooc9 = ooc8;
	ooc8 = ooc7;
	ooc7 = ooc6;
	ooc6 = ooc5;
	ooc5 = ooc4;
	ooc4 = ooc3;
	ooc3 = ooc2;
	ooc2 = ooc1;
	ooc1 = str_dup(buf1);

    if( argument[0] == '\0' )
    {
    	send_to_char( "You must have an action to perform.\n\r", ch );
    	return;
    }

    if (IS_SET(ch->comm,COMM_QUIET))
    {
    	send_to_char("You must turn off quiet mode first.\n\r",ch);
        return;
    }

    if (IS_SET(ch->comm,COMM_NOCHANNELS))
    {
    	send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
        return;
    }

    REMOVE_BIT(ch->comm,COMM_NOIC);

    /* argument = to_gnome(ch, argument); */
    sprintf( buf, "<{GIC{X> {g%s %s{x\n\r", capitalize(ch->name), argument );
    send_to_char( buf, ch );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim;

        victim = d->character ? d->character : d->original;

        if ( d->connected == CON_PLAYING &&
             victim != ch &&
             !IS_SET(victim->comm,COMM_NOIC) &&
             !IS_SET(victim->comm,COMM_QUIET)
             && victim->position >= POS_SLEEPING )
        {
	if(!IS_IMMORTAL(ch))
	{
            sprintf(buf, "<{GIC{x> {g%s %s{x\n\r",
                can_see(victim, ch) ? (!IS_NPC(ch) ? ch->name : ch->short_descr) : "Someone",
                argument);
            send_to_char(buf, victim);
	}
	else
	{
            sprintf(buf, "<{GIC{x> {g%s %s{x\n\r",
                can_see(victim, ch) ? (!IS_NPC(ch) ? ch->name : ch->short_descr) : ch->pcdata->wiziname,
                argument);
            send_to_char(buf, victim);
	}
        }
    }
}

void do_ic2( CHAR_DATA *ch, char *argument )
{
	channel_stuff(ch, argument, CHAN_IC2 );
}

void do_icaction2( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if( argument[0] == '\0' )
    {
    	send_to_char( "You must have an action to perform.\n\r", ch );
    	return;
    }

    if (IS_SET(ch->comm,COMM_QUIET))
    {
    	send_to_char("You must turn off quiet mode first.\n\r",ch);
        return;
    }

    if (IS_SET(ch->comm,COMM_NOCHANNELS))
    {
    	send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
        return;
    }

    REMOVE_BIT(ch->comm,COMM_NOIC);

    /* argument = to_gnome(ch, argument); */
    sprintf( buf, "<{cIC{X> {c%s %s{x\n\r", capitalize(ch->name), argument );
    send_to_char( buf, ch );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim;

        victim = d->character ? d->character : d->original;

        if ( d->connected == CON_PLAYING &&
             victim != ch &&
             !IS_SET(victim->comm,COMM_NOIC) &&
             !IS_SET(victim->comm,COMM_QUIET)
             && victim->position >= POS_SLEEPING )
        {
            sprintf(buf, "<{cIC{x> {c%s %s{x\n\r",
                can_see(victim, ch) ? (!IS_NPC(ch) ? ch->name : ch->short_descr) : "Someone",
                argument);
            send_to_char(buf, victim);
        }
    }
}

void do_grats( CHAR_DATA *ch, char *argument )
{
	channel_stuff(ch, argument, CHAN_GRATS );
}


/* RT music channel */
void do_music( CHAR_DATA *ch, char *argument )
{
	channel_stuff(ch, argument, CHAN_MUSIC );

}

/* clan channels */
void do_clantalk( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (!is_clan(ch) || clan_table[ch->clan].independent)
    {
	send_to_char("You aren't in a clan.\n\r",ch);
	return;
    }
    if ( argument[0] == '\0' )
    {
      if (IS_SET(ch->comm,COMM_NOCLAN))
      {
        send_to_char("Clan channel is now ON\n\r",ch);
        REMOVE_BIT(ch->comm,COMM_NOCLAN);
      }
      else
      {
        send_to_char("Clan channel is now OFF\n\r",ch);
        SET_BIT(ch->comm,COMM_NOCLAN);
      }
      return;
    }

        if (IS_SET(ch->comm,COMM_NOCHANNELS))
        {
         send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
          return;
        }

        REMOVE_BIT(ch->comm,COMM_NOCLAN);

      sprintf( buf2, "%s", ctalk_table[ch->clan].message );

      sprintf( buf, "You call %s to send a message:\n\r  {C%s{x.\n\r", buf2, argument );
      send_to_char( buf, ch );

      act( "$n calls $t to send a message.", ch, buf2, NULL, TO_ROOM );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING &&
             d->character != ch &&
	     is_same_clan(ch,d->character) &&
             !IS_SET(d->character->comm,COMM_NOCLAN) &&
	     !IS_SET(d->character->comm,COMM_QUIET) )
        {

            sprintf( buf3, "%s %s",	/* act not powerful enough */
            	clanrank_name( ch ),
            	can_see( d->character, ch ) ? capitalize( ch->name ) : "someone" );

	    act_new( "$t comes into the room to deliver a message to you from $T.", d->character, capitalize(buf2), buf3, TO_CHAR, POS_SLEEPING );

	    sprintf( buf, "  {c%s{x.\n\r", argument );
	    send_to_char( buf, d->character );


	    act( "$t comes into the room to deliver a message to $n.", d->character, capitalize(buf2), NULL, TO_ROOM );
        }

    }

    return;
}

void do_immtalk( CHAR_DATA *ch, char *argument )
{
	channel_stuff(ch, argument, CHAN_IMM);
}

void do_lsay( CHAR_DATA *ch, char *argument )
{
    char langarg[MAX_INPUT_LENGTH];
    int chance;
    int lang_gsn;
    char language[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    
    if(IS_NPC(ch))
    {
    	send_to_char("You don't speak any other languages.\n\r", ch);
    	return;
    }
    
    argument = one_argument(argument, langarg);
    
    if(langarg[0] == '\0')
    {
    	send_to_char("What do you want to say, and in which language?\n\r", ch);
    	return;
    }
    
    if(!str_prefix(langarg, "human"))
    {
    	strcpy(language, "Human");
    	lang_gsn = gsn_human;
    }
    else if(!str_prefix(langarg, "silvanesti"))
    {
    	strcpy(language, "Silvanesti");
    	lang_gsn = gsn_silvanesti;
    }
    else if(!str_prefix(langarg, "qualinesti"))
    {
    	strcpy(language, "Qualinesti");
    	lang_gsn = gsn_qualinesti;
    }
    else if(!str_prefix(langarg, "dargonesti"))
    {
    	strcpy(language, "Dargonesti");
    	lang_gsn = gsn_dargonesti;
    }
    else if(!str_prefix(langarg, "dimernesti"))
    {
    	strcpy(language, "Dimernesti");
    	lang_gsn = gsn_dimernesti;
    }
    else if(!str_prefix(langarg, "hylar"))
    {
    	strcpy(language, "Hylar");
    	lang_gsn = gsn_hylar;
    }
    else if(!str_prefix(langarg, "neidar"))
    {
    	strcpy(language, "Neidar");
    	lang_gsn = gsn_neidar;
    }
    else if(!str_prefix(langarg, "gnome"))
    {
    	strcpy(language, "Gnome");
    	lang_gsn = gsn_gnome;
    }
    else if(!str_prefix(langarg, "draconian"))
    {
    	strcpy(language, "Draconian");
    	lang_gsn = gsn_draconian;
    }
    else if(!str_prefix(langarg, "kagonesti"))
    {
    	strcpy(language, "Kagonesti");
    	lang_gsn = gsn_kagonesti;
    }
    else if(!str_prefix(langarg, "aghar"))
    {
    	strcpy(language, "Aghar");
    	lang_gsn = gsn_aghar;
    }
    else if(!str_prefix(langarg, "kender"))
    {
    	strcpy(language, "Kender");
    	lang_gsn = gsn_kender;
    }
    else if(!str_prefix(langarg, "minotaur"))
    {
    	strcpy(language, "Minotaur");
    	lang_gsn = gsn_minotaur;
    }
    else
    {
    	send_to_char("There is no such race.\n\r", ch);
    	return;
    }
    
    chance = get_skill(ch, lang_gsn);
    if(chance < number_percent())
    {
    	act("$n utters something in a language unknown to Krynn.", ch, NULL, NULL, TO_ROOM);
    	send_to_char("You say something you think is a language spoken on Krynn.\n\r", ch);
	check_improve(ch, lang_gsn, FALSE, 2);
    	return;
    }
	check_improve(ch, lang_gsn, TRUE, 2);
    	act("In $t, you say '$T'", ch, language, argument, TO_CHAR);
    
    for(rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
    	if(IS_NPC(rch) || rch == ch)
    	    continue;
    	    
    	chance = get_skill(rch, lang_gsn);
    	if(chance < 1)
    	{
    	    act("$n utters something in a language you've never heard before.", ch, NULL, rch, TO_VICT);
    	    continue;
    	}

    	if(chance < number_percent())
    	{
    	    sprintf(buf, "%s utters something in %s, but you can't make it out.\n\r",
    	      ch->name, language);
    	    send_to_char(buf, rch);
    	    continue;
    	}
    	
    	sprintf(buf, "In %s, %s says '%s'\n\r", language, ch->name, argument);
    	send_to_char(buf, rch);
    }
}


void do_say( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    char buf[MAX_STRING_LENGTH];
    
    if(IS_AFFECTED2(ch, AFF_SILENCE))
    {
    	send_to_char("Something's wrong with your voice.\n\r", ch);
    	return;
    }
    
    if ( argument[0] == '\0' )
    {
	send_to_char( "Say what?\n\r", ch );
	return;
    }

    argument = to_gnome(ch, argument);
    sprintf(buf, "You say '{g%s{x'\n\r", argument);
    send_to_char(buf, ch);
    for(rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if(rch != ch && rch->position >= POS_RESTING)
        {
	if(!IS_IMMORTAL(ch))
	{
            sprintf(buf, "%s says '{g%s{x'\n\r",
                can_see(rch, ch) ? (IS_NPC(ch) ? ch->short_descr : ch->name) : "Someone", argument);
            send_to_char(buf, rch);
	}
	else
	{
            sprintf(buf, "%s says '{g%s{x'\n\r",
                can_see(rch, ch) ? (IS_NPC(ch) ? ch->short_descr : ch->name) : ch->pcdata->wiziname , argument);
            send_to_char(buf, rch);
	}
        }
    }

    if ( !IS_NPC(ch) )
    {
	CHAR_DATA *mob, *mob_next;
	for ( mob = ch->in_room->people; mob != NULL; mob = mob_next )
	{
	    mob_next = mob->next_in_room;
	    if ( IS_NPC(mob) && HAS_TRIGGER( mob, TRIG_SPEECH )
	    &&   mob->position == mob->pIndexData->default_pos )
		mp_act_trigger( argument, mob, ch, NULL, NULL, TRIG_SPEECH );
	}
    }
    return;
}


void do_osay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    char buf[MAX_STRING_LENGTH];

    if (argument[0] == '\0')
    {
        if(!IS_SET(ch->comm, COMM_NOOSAY))
        {
            SET_BIT(ch->comm, COMM_NOOSAY);
            send_to_char("You will no longer hear ooc says.\n\r", ch);
            return;
        }
        else
        {
            REMOVE_BIT(ch->comm, COMM_NOOSAY);
            send_to_char("You will hear ooc says again.\n\r", ch);
            return;
        }
    }
    
//    act ("{6$n says (ooc) '{W$T{x'{x", ch, NULL, argument, TO_ROOM);
//  Using osay automatically removes osay channel mute.
    REMOVE_BIT(ch->comm, COMM_NOOSAY);

    for(rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if(rch == ch)
            continue;
        if(!IS_SET(rch->comm, COMM_NOOSAY)
           && rch->position >= POS_RESTING)
        {
     	if(!IS_IMMORTAL(ch))
	{
            sprintf(buf, "%s {Wsays (ooc) {x'{W%s{x'\n\r",
                can_see(rch, ch) ? (IS_NPC(ch) ? ch->short_descr : ch->name) : "Someone", argument);
            send_to_char(buf, rch);
	}
	else
	{
            sprintf(buf, "%s {Wsays (ooc) {x'{W%s{x'\n\r",
                can_see(rch, ch) ? (IS_NPC(ch) ? ch->short_descr : ch->name) : ch->pcdata->wiziname , argument);
            send_to_char(buf, rch);
	}
        }
    }
    act ("{6You say (ooc) '{W$T{x'{x", ch, NULL, argument, TO_CHAR);

if (!IS_NPC (ch))
    {
        CHAR_DATA *mob, *mob_next;
        for (mob = ch->in_room->people; mob != NULL; mob = mob_next)
        {
            mob_next = mob->next_in_room;
            if (IS_NPC (mob) && HAS_TRIGGER (mob, TRIG_SPEECH)
                && mob->position == mob->pIndexData->default_pos)
                mp_act_trigger (argument, mob, ch, NULL, NULL, TRIG_SPEECH);
        }
    }
    return;

}
#if 0
void do_shout( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0' )
    {
      	if (IS_SET(ch->comm,COMM_SHOUTSOFF))
      	{
            send_to_char("You can hear shouts again.\n\r",ch);
            REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);
      	}
      	else
      	{
            send_to_char("You will no longer hear shouts.\n\r",ch);
            SET_BIT(ch->comm,COMM_SHOUTSOFF);
      	}
      	return;
    }

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "You can't shout.\n\r", ch );
        return;
    }
 
    REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);

    WAIT_STATE( ch, 12 );
    
    argument = to_gnome(ch, argument);

    act( "You shout '{r$T{x'", ch, NULL, argument, TO_CHAR );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if ( d->connected == CON_PLAYING &&
	     d->character != ch &&
	     !IS_SET(victim->comm, COMM_SHOUTSOFF) &&
	     !IS_SET(victim->comm, COMM_QUIET) ) 
	{
	    act("$n shouts '{r$t{x'",ch,argument,d->character,TO_VICT);
	}
    }

    return;
}
#endif


void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET(ch->comm, COMM_QUIET) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    if (IS_SET(ch->comm,COMM_DEAF))
    {
	send_to_char("You must turn off deaf mode first.\n\r",ch);
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Tell whom what?\n\r", ch );
	return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
    || ( IS_NPC(victim) && victim->in_room != ch->in_room ) )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
	act("$N seems to have misplaced $S link...try again later.",
	    ch,NULL,victim,TO_CHAR);
        sprintf(buf,"%s tells you '{y%s{x'{x\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
	return;
    }

    if ( !(IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }
  
    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
    && !IS_IMMORTAL(ch))
    {
	act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
  	return;
    }

    if (IS_SET(victim->comm,COMM_AFK))
    {
	if (IS_NPC(victim))
	{
	    act("$E is AFK, and not receiving tells.",ch,NULL,victim,TO_CHAR);
	    return;
	}

	act("$E is AFK, but your tell will go through when $E returns.",
	    ch,NULL,victim,TO_CHAR);
	sprintf(buf,"%s tells you '{y%s{x'{x\n\r",PERS(ch,victim),argument);
	buf[0] = UPPER(buf[0]);
	add_buf(victim->pcdata->buffer,buf);
	return;
    }

    act( "You tell $N '{y$t{x'{x", ch, argument, victim, TO_CHAR );
    act_new("$n tells you '{y$t{x'{x",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    if ( !IS_NPC(ch) && IS_NPC(victim) && HAS_TRIGGER(victim,TRIG_SPEECH) )
	mp_act_trigger( argument, victim, ch, NULL, NULL, TRIG_SPEECH );

    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->comm, COMM_NOTELL) )
    {
	send_to_char( "Your message didn't get through.\n\r", ch );
	return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim->desc == NULL && !IS_NPC(victim))
    {
        act("$N seems to have misplaced $S link...try again later.",
            ch,NULL,victim,TO_CHAR);
        sprintf(buf,"%s tells you '{y%s{x'{x\n\r",PERS(ch,victim),argument);
        buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
        return;
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
	act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
	return;
    }

    if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
    &&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim))
    {
        act_new( "$E is not receiving tells.", ch, 0, victim, TO_CHAR,POS_DEAD);
        return;
    }

    if (!IS_IMMORTAL(victim) && !IS_AWAKE(ch))
    {
	send_to_char( "In your dreams, or what?\n\r", ch );
	return;
    }

    if (IS_SET(victim->comm,COMM_AFK))
    {
        if (IS_NPC(victim))
        {
            act_new("$E is AFK, and not receiving tells.",
		ch,NULL,victim,TO_CHAR,POS_DEAD);
            return;
        }
 
        act_new("$E is AFK, but your tell will go through when $E returns.",
            ch,NULL,victim,TO_CHAR,POS_DEAD);
        sprintf(buf,"%s tells you '{y%s{x'{x\n\r",PERS(ch,victim),argument);
	buf[0] = UPPER(buf[0]);
        add_buf(victim->pcdata->buffer,buf);
        return;
    }

    act_new("You tell $N '{y$t{x'{x",ch,argument,victim,TO_CHAR,POS_DEAD);
    act_new("$n tells you '{y$t{x'{x",ch,argument,victim,TO_VICT,POS_DEAD);
    victim->reply	= ch;

    return;
}



void do_yell( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET(ch->comm, COMM_NOSHOUT) )
    {
        send_to_char( "You can't yell.\n\r", ch );
        return;
    }
 
    if ( argument[0] == '\0' )
    {
	send_to_char( "Yell what?\n\r", ch );
	return;
    }

    argument = to_gnome(ch, argument);

    sprintf(buf, "You yell '{y%s{x'\n\r", argument);
    send_to_char(buf, ch);
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim;
        victim = d->character ? d->character : d->original;
	if ( d->connected == CON_PLAYING
	&&   victim != ch
	&&   victim->in_room != NULL
	&&   victim->in_room->area == ch->in_room->area 
        &&   !IS_SET(victim->comm,COMM_QUIET) )
	{
	    sprintf(buf, "%s yells '{y%s{x'\n\r",
	        can_see(victim, ch) ? ch->name : "Someone",
	        argument);
	    send_to_char(buf, victim);
	}
    }

    return;
}


void do_emote( CHAR_DATA *ch, char *argument )
{
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
 
    MOBtrigger = FALSE;
    if(argument[0] == '\'')
    {
    	act("$n$T", ch, NULL, argument, TO_CHAR);
    	act("$n$T", ch, NULL, argument, TO_ROOM);
    }
    else
    {
    	act( "$n $T", ch, NULL, argument, TO_ROOM );
    	act( "$n $T", ch, NULL, argument, TO_CHAR );
    }
    MOBtrigger = TRUE;
    return;
}


void do_pmote( CHAR_DATA *ch, char *argument )
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
 
    act( "$n $t", ch, argument, NULL, TO_CHAR );

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
	if (vch->desc == NULL || vch == ch)
	    continue;

	if ((letter = strstr(argument,vch->name)) == NULL)
	{
	    MOBtrigger = FALSE;
	    act("$N $t",vch,argument,ch,TO_CHAR);
	    MOBtrigger = TRUE;
	    continue;
	}

	strcpy(temp,argument);
	temp[strlen(argument) - strlen(letter)] = '\0';
   	last[0] = '\0';
 	name = vch->name;
	
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

	MOBtrigger = FALSE;
	act("$N $t",vch,temp,ch,TO_CHAR);
	MOBtrigger = TRUE;
    }
	
    return;
}


/*
 * All the posing stuff.
 */
struct	pose_table_type
{
    char *	message[2*MAX_CLASS];
};
struct quote_type
{
   char * text;
   char * by;
};

const struct quote_type quote_table [MAX_QUOTES] =
{
   { "The gods look after fools, children, and drunks. Kenders count for two out of theose three.", "Dwarven Saying" },
   { "Alas all good things must come to an end.","Random" },
   { "Do you feel me near, Dwarf?", "Dalamar" }
};

void do_quote(CHAR_DATA *ch)
{
   char buf[MAX_STRING_LENGTH];
   int number;

   number = number_range(0, MAX_QUOTES-1);

   sprintf( buf, "\n\r%s\n\r - %s \n\r",
     quote_table[number].text,
     quote_table[number].by);
   send_to_char( buf, ch);
   return;
}

const	struct	pose_table_type	pose_table	[]	=
{
    {
	{
	    "You sizzle with energy.",
	    "$n sizzles with energy.",
	    "You feel very holy.",
	    "$n looks very holy.",
	    "You perform a small card trick.",
	    "$n performs a small card trick.",
	    "You show your bulging muscles.",
	    "$n shows $s bulging muscles.",
	    "...",
	    "..."
	}
    },

    {
	{
	    "You turn into a butterfly, then return to your normal shape.",
	    "$n turns into a butterfly, then returns to $s normal shape.",
	    "You nonchalantly turn wine into water.",
	    "$n nonchalantly turns wine into water.",
	    "You wiggle your ears alternately.",
	    "$n wiggles $s ears alternately.",
	    "You crack nuts between your fingers.",
	    "$n cracks nuts between $s fingers."
	    "...",
	    "...",
	}
    },

    {
	{
	    "Blue sparks fly from your fingers.",
	    "Blue sparks fly from $n's fingers.",
	    "A halo appears over your head.",
	    "A halo appears over $n's head.",
	    "You nimbly tie yourself into a knot.",
	    "$n nimbly ties $mself into a knot.",
	    "You grizzle your teeth and look mean.",
	    "$n grizzles $s teeth and looks mean.",
	    "...",
	    "...",
	}
    },

    {
	{
	    "Little red lights dance in your eyes.",
	    "Little red lights dance in $n's eyes.",
	    "You recite words of wisdom.",
	    "$n recites words of wisdom.",
	    "You juggle with daggers, apples, and eyeballs.",
	    "$n juggles with daggers, apples, and eyeballs.",
	    "You hit your head, and your eyes roll.",
	    "$n hits $s head, and $s eyes roll.",
	    "...",
	    "...",
	}
    },

    {
	{
	    "A slimy green monster appears before you and bows.",
	    "A slimy green monster appears before $n and bows.",
	    "Deep in prayer, you levitate.",
	    "Deep in prayer, $n levitates.",
	    "You steal the underwear off every person in the room.",
	    "Your underwear is gone!  $n stole it!",
	    "Crunch, crunch -- you munch a bottle.",
	    "Crunch, crunch -- $n munches a bottle.",
	    "...",
	    "...",
	}
    },

    {
	{
	    "You turn everybody into a little pink elephant.",
	    "You are turned into a little pink elephant by $n.",
	    "An angel consults you.",
	    "An angel consults $n.",
	    "The dice roll ... and you win again.",
	    "The dice roll ... and $n wins again.",
	    "... 98, 99, 100 ... you do pushups.",
	    "... 98, 99, 100 ... $n does pushups.",
	    "...",
	    "..."
	}
    },

    {
	{
	    "A small ball of light dances on your fingertips.",
	    "A small ball of light dances on $n's fingertips.",
	    "Your body glows with an unearthly light.",
	    "$n's body glows with an unearthly light.",
	    "You count the money in everyone's pockets.",
	    "Check your money, $n is counting it.",
	    "Arnold Schwarzenegger admires your physique.",
	    "Arnold Schwarzenegger admires $n's physique.",
	    "...",
	    "..."
	}
    },

    {
	{
	    "Smoke and fumes leak from your nostrils.",
	    "Smoke and fumes leak from $n's nostrils.",
	    "A spot light hits you.",
	    "A spot light hits $n.",
	    "You balance a pocket knife on your tongue.",
	    "$n balances a pocket knife on your tongue.",
	    "Watch your feet, you are juggling granite boulders.",
	    "Watch your feet, $n is juggling granite boulders.",
	    "...",
	    "..."
	}
    },

    {
	{
	    "The light flickers as you rap in magical languages.",
	    "The light flickers as $n raps in magical languages.",
	    "Everyone levitates as you pray.",
	    "You levitate as $n prays.",
	    "You produce a coin from everyone's ear.",
	    "$n produces a coin from your ear.",
	    "Oomph!  You squeeze water out of a granite boulder.",
	    "Oomph!  $n squeezes water out of a granite boulder.",
	    "...",
	    "..."
	}
    },

    {
	{
	    "Your head disappears.",
	    "$n's head disappears.",
	    "A cool breeze refreshes you.",
	    "A cool breeze refreshes $n.",
	    "You step behind your shadow.",
	    "$n steps behind $s shadow.",
	    "You pick your teeth with a spear.",
	    "$n picks $s teeth with a spear.",
	    "...",
	    "..."
	}
    },

    {
	{
	    "A fire elemental singes your hair.",
	    "A fire elemental singes $n's hair.",
	    "The sun pierces through the clouds to illuminate you.",
	    "The sun pierces through the clouds to illuminate $n.",
	    "Your eyes dance with greed.",
	    "$n's eyes dance with greed.",
	    "Everyone is swept off their foot by your hug.",
	    "You are swept off your feet by $n's hug.",
	    "...",
	    "..."
	}
    },

    {
	{
	    "The sky changes color to match your eyes.",
	    "The sky changes color to match $n's eyes.",
	    "The ocean parts before you.",
	    "The ocean parts before $n.",
	    "You deftly steal everyone's weapon.",
	    "$n deftly steals your weapon.",
	    "Your karate chop splits a tree.",
	    "$n's karate chop splits a tree.",
	    "...",
	    "..."
	}
    },

    {
	{
	    "The stones dance to your command.",
	    "The stones dance to $n's command.",
	    "A thunder cloud kneels to you.",
	    "A thunder cloud kneels to $n.",
	    "The Grey Mouser buys you a beer.",
	    "The Grey Mouser buys $n a beer.",
	    "A strap of your armor breaks over your mighty thews.",
	    "A strap of $n's armor breaks over $s mighty thews.",
	    "...",
	    "..."
	}
    },

    {
	{
	    "The heavens and grass change colour as you smile.",
	    "The heavens and grass change colour as $n smiles.",
	    "The Burning Man speaks to you.",
	    "The Burning Man speaks to $n.",
	    "Everyone's pocket explodes with your fireworks.",
	    "Your pocket explodes with $n's fireworks.",
	    "A boulder cracks at your frown.",
	    "A boulder cracks at $n's frown.",
	    "...",
	    "..."
	}
    },

    {
	{
	    "Everyone's clothes are transparent, and you are laughing.",
	    "Your clothes are transparent, and $n is laughing.",
	    "An eye in a pyramid winks at you.",
	    "An eye in a pyramid winks at $n.",
	    "Everyone discovers your dagger a centimeter from their eye.",
	    "You discover $n's dagger a centimeter from your eye.",
	    "Mercenaries arrive to do your bidding.",
	    "Mercenaries arrive to do $n's bidding.",
	    "...",
	    "..."
	}
    },

    {
	{
	    "A black hole swallows you.",
	    "A black hole swallows $n.",
	    "Valentine Michael Smith offers you a glass of water.",
	    "Valentine Michael Smith offers $n a glass of water.",
	    "Where did you go?",
	    "Where did $n go?",
	    "Four matched Percherons bring in your chariot.",
	    "Four matched Percherons bring in $n's chariot.",
	    "...",
	    "..."
	}
    },

    {
	{
	    "The world shimmers in time with your whistling.",
	    "The world shimmers in time with $n's whistling.",
	    "The great god $g gives you a staff.",
	    "The great god $g gives $n a staff.",
	    "Click.",
	    "Click.",
	    "Atlas asks you to relieve him.",
	    "Atlas asks $n to relieve him.",
	    "...",
	    "..."
	}
    }
};



void do_pose( CHAR_DATA *ch, char *argument )
{
    int level;
    int pose;

    if ( IS_NPC(ch) )
	return;

    level = UMIN( ch->level, sizeof(pose_table) / sizeof(pose_table[0]) - 1 );
    pose  = number_range(0, level);

    act( pose_table[pose].message[2*ch->class+0], ch, NULL, NULL, TO_CHAR );
    act( pose_table[pose].message[2*ch->class+1], ch, NULL, NULL, TO_ROOM );

    return;
}



void do_bug( CHAR_DATA *ch, char *argument )
{
    append_file( ch, BUG_FILE, argument );
    send_to_char( "Bug logged.\n\r", ch );
    return;
}

void do_typo( CHAR_DATA *ch, char *argument )
{
    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}

void do_rent( CHAR_DATA *ch, char *argument )
{
    send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
    return;
}


void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}



void do_quit( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d,*d_next;
    int id;

    if ( IS_NPC(ch) )
	return;

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "No way! You are fighting.\n\r", ch );
	return;
    }

    if( IS_SET(ch->act, PLR_VIOLENT) )
    {
    	send_to_char( "You're too excited to quit.\n\r", ch );
    	return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
	send_to_char( "You're not DEAD yet.\n\r", ch );
	return;
    }
    
    if( IS_SET(ch->act, PLR_SLAIN) )
    {
    	REMOVE_BIT( ch->act, PLR_SLAIN );
    	ch->slain = 0;
    }
    
        if (IS_SET(ch->act, PLR_QUESTOR))
        {
	  send_to_char( "Your quest was reset!\n\r", ch );
                REMOVE_BIT(ch->act, PLR_QUESTOR);
                ch->questgiver = NULL;
                ch->countdown = 0;
                ch->questmob = 0;
                ch->questobj = 0;
                ch->nextquest = 10;
		return;
	}
	if(IS_AFFECTED(ch, AFF_CHARM))
	{
	   send_to_char("You can not quit while charmed", ch);
	   return;
	}
//    send_to_char( 
//	"Alas, all good things must come to an end.\n\r",ch);
	do_quote(ch);
    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );
     wiznet("$N rejoins the real world.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));

    /*
     * After extract_char the ch is no longer valid!
     */
    
    if(str_cmp(argument, "finger"))
    	save_char_obj( ch );
    id = ch->id;
    d = ch->desc;
    extract_char( ch, TRUE );
    if ( d != NULL )
	close_socket( d );

    /* toast evil cheating bastards */
    for (d = descriptor_list; d != NULL; d = d_next)
    {
	CHAR_DATA *tch;

	d_next = d->next;
	tch = d->original ? d->original : d->character;
	if (tch && tch->id == id)
	{
	    extract_char(tch,TRUE);
	    close_socket(d);
	} 
    }

   // web_update(); //
    return;
}



void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    save_char_obj( ch );
    send_to_char("Saving. Remember that ROM has automatic saving now.\n\r", ch);
/*
    WAIT_STATE(ch,2 * PULSE_VIOLENCE);
*/
    return;
}



void do_follow( CHAR_DATA *ch, char *argument )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Follow whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL )
    {
	act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
	return;
    }

    if ( victim == ch )
    {
	if ( ch->master == NULL )
	{
	    send_to_char( "You already follow yourself.\n\r", ch );
	    return;
	}
	stop_follower(ch);
	return;
    }

    if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_IMMORTAL(ch))
    {
	act("$N doesn't seem to want any followers.",
             ch,NULL,victim, TO_CHAR);
        return;
    }

    REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    
    if ( ch->master != NULL )
	stop_follower( ch );

    add_follower( ch, victim );
    return;
}


void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master != NULL )
    {
	bug( "Add_follower: non-null master.", 0 );
	return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
	act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}



void stop_follower( CHAR_DATA *ch )
{
    if ( ch->master == NULL )
    {
	bug( "Stop_follower: null master.", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
    }

    if ( can_see( ch->master, ch ) && ch->in_room != NULL)
    {
	act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
    	act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
    }
    
    if(ch == ch->master->pet && IS_SET(ch->master->act, PLR_MOUNTED))
    {
    	REMOVE_BIT(ch->master->act, PLR_MOUNTED);
    	ch->master->position = POS_STANDING;
    }
    
    if (ch->master->pet == ch)
	ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch )
{    
    CHAR_DATA *pet;

    if ((pet = ch->pet) != NULL)
    {
    	stop_follower(pet);
    	if (pet->in_room != NULL)
    	    act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
    	extract_char(pet,TRUE);
    }
    ch->pet = NULL;

    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL )
    {
    	if (ch->master->pet == ch)
    	    ch->master->pet = NULL;
	stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch->master == ch )
	    stop_follower( fch );
	if ( fch->leader == ch )
	    fch->leader = fch;
    }

    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    argument = one_argument( argument, arg );
    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete") || !str_cmp(arg2,"mob"))
    {
        send_to_char("That will NOT be done.\n\r",ch);
        return;
    }

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Order whom to do what?\n\r", ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
	send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
	fAll   = TRUE;
	victim = NULL;
    }
    else
    {
	fAll   = FALSE;
	if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{
	    send_to_char( "They aren't here.\n\r", ch );
	    return;
	}

	if ( victim == ch )
	{
	    send_to_char( "Aye aye, right away!\n\r", ch );
	    return;
	}

	if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch 
	||  (IS_IMMORTAL(victim) && victim->trust >= ch->trust))
	{
	    send_to_char( "Do it yourself!\n\r", ch );
	    return;
	}
    }

    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
	och_next = och->next_in_room;

	if ( IS_AFFECTED(och, AFF_CHARM)
	&&   och->master == ch
	&& ( fAll || och == victim ) )
	{
	    found = TRUE;
	    sprintf( buf, "$n orders you to '%s'.", argument );
	    act( buf, ch, NULL, och, TO_VICT );
	    interpret( och, argument );
	}
    }

    if ( found )
    {
	WAIT_STATE(ch,PULSE_VIOLENCE);
	send_to_char( "Ok.\n\r", ch );
    }
    else
	send_to_char( "You have no followers here.\n\r", ch );
    return;
}



void do_group( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	CHAR_DATA *gch;
	CHAR_DATA *leader;

	leader = (ch->leader != NULL) ? ch->leader : ch;
	sprintf( buf, "{Y%s's {Rgroup:{x\n\r", PERS(leader, ch) );
	send_to_char( buf, ch );

	for ( gch = char_list; gch != NULL; gch = gch->next )
	{
	    if ( is_same_group( gch, ch ) )
	    {
		sprintf( buf,
		"{r[{c%2d {g%s{r] {y%-14s {r%4d{x/{R%4d hp {g%4d{x/{G%4d mana {b%4d{x/{B%4d mv {W%5d xp{x\n\r",
		    gch->level,
		    IS_NPC(gch) ? "Mob" : class_table[gch->class].who_name,
		    capitalize( PERS(gch, ch) ),
		    gch->hit,   gch->max_hit,
		    gch->mana,  gch->max_mana,
		    gch->move,  gch->max_move,
		    gch->exp    );
		send_to_char( buf, ch );
	    }
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
	send_to_char( "But you are following someone else!\n\r", ch );
	return;
    }

    if ( victim->master != ch && ch != victim )
    {
	act_new("$N isn't following you.",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_CHARM))
    {
        send_to_char("You can't remove charmed mobs from your group.\n\r",ch);
        return;
    }
    
    if (IS_AFFECTED(ch,AFF_CHARM))
    {
    	act_new("You like your master too much to leave $m!",
	    ch,NULL,victim,TO_VICT,POS_SLEEPING);
    	return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
	victim->leader = NULL;
	act_new("$n removes $N from $s group.",
	    ch,NULL,victim,TO_NOTVICT,POS_RESTING);
	act_new("$n removes you from $s group.",
	    ch,NULL,victim,TO_VICT,POS_SLEEPING);
	act_new("You remove $N from your group.",
	    ch,NULL,victim,TO_CHAR,POS_SLEEPING);
	return;
    }
    
    if( ch->level > victim->level + 15 )
    {
    	act( "You're too powerful to group $N.", ch, NULL, victim, TO_CHAR );
    	return;
    }
    
    if( ch->level < victim->level - 15 )
    {
    	act( "$N is too powerful to group.", ch, NULL, victim, TO_CHAR );
    	return;
    }

    victim->leader = ch;
    act_new("$N joins $n's group.",ch,NULL,victim,TO_NOTVICT,POS_RESTING);
    act_new("You join $n's group.",ch,NULL,victim,TO_VICT,POS_SLEEPING);
    act_new("$N joins your group.",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount_steel = 0, amount_gold = 0;
    int share_steel, share_gold;
    int extra_steel, extra_gold;

    argument = one_argument( argument, arg1 );
	       one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount_gold = atoi( arg1 );

    if (arg2[0] != '\0')
	amount_steel = atoi(arg2);

    if ( amount_steel < 0 || amount_gold < 0)
    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount_steel == 0 && amount_gold == 0 )
    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->steel <  amount_steel || ch->gold < amount_gold)
    {
	send_to_char( "You don't have that much to split.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
	    members++;
    }

    if ( members < 2 )
    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }
	    
    share_gold = amount_gold / members;
    extra_gold = amount_gold % members;

    share_steel   = amount_steel / members;
    extra_steel   = amount_steel % members;

    if ( share_steel == 0 && share_gold == 0 )
    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->gold	-= amount_gold;
    ch->gold	+= share_gold + extra_gold;
    ch->steel 	-= amount_steel;
    ch->steel 	+= share_steel + extra_steel;

    if (share_gold > 0)
    {
	sprintf(buf,
	    "You split %d gold coins. Your share is %d gold.\n\r",
 	    amount_gold,share_gold + extra_gold);
	send_to_char(buf,ch);
    }

    if (share_steel > 0)
    {
	sprintf(buf,
	    "You split %d steel coins. Your share is %d steel.\n\r",
	     amount_steel,share_steel + extra_steel);
	send_to_char(buf,ch);
    }

    if (share_steel == 0)
    {
	sprintf(buf,"$n splits %d gold coins. Your share is %d gold.",
		amount_gold,share_gold);
    }
    else if (share_gold == 0)
    {
	sprintf(buf,"$n splits %d steel coins. Your share is %d steel.",
		amount_steel,share_steel);
    }
    else
    {
	sprintf(buf,
"$n splits %d gold and %d steel coins, giving you %d gold and %d steel.\n\r",
	 amount_gold,amount_steel,share_gold,share_steel);
    }

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
	{
	    act( buf, ch, NULL, gch, TO_VICT );
	    gch->steel += share_steel;
	    gch->gold += share_gold;
	}
    }

    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Tell your group what?\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "Your message didn't get through!\n\r", ch );
	return;
    }
    
    act_new( "You tell the group '{y$t{x'", ch, argument, NULL, TO_CHAR, POS_SLEEPING );

    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
	if ( is_same_group( gch, ch ) )
	    act_new("$n tells the group '{y$t{x'",
		ch,argument,gch,TO_VICT,POS_SLEEPING);
    }

    return;
}



/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach == NULL || bch == NULL)
	return FALSE;

    if ( ach->leader != NULL ) ach = ach->leader;
    if ( bch->leader != NULL ) bch = bch->leader;
    return ach == bch;
}

/*
 * ColoUr setting and unsetting, way cool, Ant Oct 94
 *        revised to include config colour, Ant Feb 95
 */
void do_colour( CHAR_DATA *ch, char *argument )
{
    if( IS_NPC( ch ) )
    {
	send_to_char_bw( "ColoUr is not ON, Way Moron!\n\r", ch );
	return;
    }

    if( !IS_SET( ch->act, PLR_COLOUR ) )
    {
   	SET_BIT( ch->act, PLR_COLOUR );
	send_to_char( "ColoUr is now ON, Way Cool!\n\r", ch );
	send_to_char( "ColoUr is brought to you by Lope, ant@solace.mh.se.\n\r", ch );
    }
    else
    {
    	send_to_char_bw( "ColoUr is now OFF, <sigh>\n\r", ch );
	REMOVE_BIT( ch->act, PLR_COLOUR );
    }

    return;

}




void do_beep( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    
    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
	send_to_char( "Your beep didn't get through.\n\r", ch );
	return;
    }

    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
	send_to_char( "You must turn off quiet mode first.\n\r", ch);
	return;
    }

    if ( argument[0]=='\0' )
    {
    	if ( IS_SET( ch->comm, COMM_NOBEEP ) )
    	{
    	    REMOVE_BIT( ch->comm, COMM_NOBEEP );
    	    send_to_char( "You now accept beeps.\n\r", ch );
    	}
    	else
    	{
    	    SET_BIT( ch->comm, COMM_NOBEEP );
    	    send_to_char( "You no longer accept beeps.\n\r", ch );
    	}
        return;
    }
    
    if ( IS_SET( ch->comm, COMM_NOBEEP ) )
    {
    	send_to_char( "You have to turn on the beep channel first.\n\r", ch);
    	return;
    }
    
    if ( ( victim=get_char_world( ch, argument ) )==NULL )
    {
        send_to_char( "Nobody like that.\n\r", ch );
        return;
    }
    
    if ( IS_SET( victim->comm, COMM_NOBEEP ) )
    {
    	act_new("$N is not receiving beeps.", ch, NULL, victim, TO_CHAR, POS_DEAD);
    	return;
    }
    
    act_new( "\aYou beep to $N.", ch, NULL, victim, TO_CHAR, POS_DEAD );
    act_new( "\a$n beeps you.", ch, NULL, victim, TO_VICT, POS_DEAD );

    return;
}

void do_pray( CHAR_DATA *ch, char *argument )
 {
     char buf[MAX_STRING_LENGTH];
     DESCRIPTOR_DATA *d;
 
     if ( argument[0] == '\0' )
     {
       if (IS_SET(ch->comm,COMM_NOPRAY))
       {
         send_to_char("Pray channel is now ON\n\r",ch);
         REMOVE_BIT(ch->comm,COMM_NOPRAY);
       }
       else
       {
         send_to_char("Pray channel is now OFF\n\r",ch);
         SET_BIT(ch->comm,COMM_NOPRAY);
       }
       return;
     }
 
     REMOVE_BIT(ch->comm,COMM_NOPRAY);
 
     sprintf( buf, "{y$n:{c %s{x", argument );
     act_new("$n prays {B$t{x",ch,argument,NULL,TO_CHAR,POS_DEAD);
     for ( d = descriptor_list; d != NULL; d = d->next )
     {
         if ( d->connected == CON_PLAYING
            && ( ch->god == d->character->god )
            && !IS_SET(d->character->comm,COMM_NOPRAY)
            && !IS_SET(d->character->comm,COMM_QUIET) )
         {
             act_new("$n prays '{B$t'{x",ch,argument,d->character,TO_VICT,POS_DEAD);
         }
     }
 
     return;
 }

void do_bounty( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
	
    
	if ( arg1[0] == '\0' || arg2[0] == '\0' )
        {
	     send_to_char( "Place a bounty on who's head?\n\rSyntax: Bounty<victim> <amount>\n\r", ch );
             return;
        }
	
        if ( ( victim = get_char_world( ch, arg1 ) ) == NULL)
	{
  	   send_to_char( "They are currently not logged in!", ch );
	   return;
        }
  
      if (IS_NPC(victim))
      {
	send_to_char( "You cannot put a bounty on NPCs!", ch );
	return;
      }

	if ( is_number( arg2 ) )
        {
	int amount;
	amount   = atoi(arg2);
        if (ch->steel < amount)
        {
		send_to_char( "You don't have that much steel!", ch );
		return;
        }
        if (amount < 0)
        {
		send_to_char( "Wow, you are special... I dont think so", ch );
		return;
        }
	ch->steel -= amount;
	victim->pcdata->bounty +=amount;
	sprintf( buf, "You have placed a %d gold bounty on %s{g.\n\r%s now has a bounty of %d steel.",
	amount,victim->name,victim->name,victim->pcdata->bounty );
	send_to_char(buf,ch);
	return;
	}
}

