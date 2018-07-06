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
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"

char *	const	where_name	[] =
{
    "{w<{cused as light{x{w>{x     ",
    "{w<{cworn on finger{x{w>{x    ",
    "{w<{cworn on finger{x{w>{x    ",
    "{w<{cworn around neck{x{w>{x  ",
    "{w<{cworn around neck{x{w>{x  ",
    "{w<{cworn on torso{x{w>{x     ",
    "{w<{cworn on head{x{w>{x      ",
    "{w<{cworn on legs{x{w>{x      ",
    "{w<{cworn on feet{x{w>{x      ",
    "{w<{cworn on hands{x{w>{x     ",
    "{w<{cworn on arms{x{w>{x      ",
    "{w<{cworn as shield{x{w>{x    ",
    "{w<{cworn about body{x{w>{x   ",
    "{w<{cworn about waist{x{w>{x  ",
    "{w<{cworn around wrist{x{w>{x ",
    "{w<{cworn around wrist{x{w>{x ",
    "{w<{cright hand{x{w>{x        ",
    "{w<{cleft hand{x{w>{x         ",
    "{w<{cfloating nearby{x{w>{x   ",
    "{w<{cworn on ankle{x{w>{x     ",
    "{w<{cworn on ear{x{w>{x       ",
    "{w<{cworn on back{x{w>{x      ",
    "{w<{cleft arm tattoo{x{w>{x   ",
    "{w<{cright arm tattoo{x{w>{x  ",
    "{w<{cworn on face{x{w>{x      ",

};


/* for  keeping track of the player count */
int max_on = 0;

/*
 * Local functions.
 */
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );
bool	bank_deposit		args( ( CHAR_DATA *ch, int amount, const char *type ) );
bool	bank_withdraw		args( ( CHAR_DATA *ch, int amount, const char *type ) );
void	bank_balance		args( ( CHAR_DATA *ch ) );
void	mob_desc		args( ( CHAR_DATA *ch, char *argument ) );

char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
    ||  (obj->description == NULL || obj->description[0] == '\0'))
	return buf;

    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )   strcat( buf, "{W({DInvis{W){x "     );
    if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
         && IS_OBJ_STAT(obj, ITEM_EVIL)   )   strcat( buf, "{W({rRed Aura{W){x "  );
    if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
    &&  IS_OBJ_STAT(obj,ITEM_BLESS))	      strcat( buf, "{W({BBlue Aura{W){x "	);
    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
         && IS_OBJ_STAT(obj, ITEM_MAGIC)  )   strcat( buf, "{W({CMagical{W){x "   );
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "{W({YGlowing{W){x "   );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "{W({YHumming{W){x "   );

    if ( fShort )
    {
	if ( obj->short_descr != NULL )
	    strcat( buf, obj->short_descr );
    }
    else
    {
	if ( obj->description != NULL)
	    strcat( buf, obj->description );
    }

    return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
	return;

    /*
     * Alloc space for output lines.
     */
    output = new_buf();

    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
	count++;
    prgpstrShow	= alloc_mem( count * sizeof(char *) );
    prgnShow    = alloc_mem( count * sizeof(int)    );
    nShow	= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    { 
	if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) ) 
	{
	    pstrShow = format_obj_to_char( obj, ch, fShort );

	    fCombine = FALSE;

	    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    {
		/*
		 * Look for duplicates, case sensitive.
		 * Matches tend to be near end so run loop backwords.
		 */
		for ( iShow = nShow - 1; iShow >= 0; iShow-- )
		{
		    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
		    {
			prgnShow[iShow]++;
			fCombine = TRUE;
			break;
		    }
		}
	    }

	    /*
	     * Couldn't combine, or didn't want to.
	     */
	    if ( !fCombine )
	    {
		prgpstrShow [nShow] = str_dup( pstrShow );
		prgnShow    [nShow] = 1;
		nShow++;
	    }
	}
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
	if (prgpstrShow[iShow][0] == '\0')
	{
	    free_string(prgpstrShow[iShow]);
	    continue;
	}

	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	{
	    if ( prgnShow[iShow] != 1 )
	    {
		sprintf( buf, "(%2d) ", prgnShow[iShow] );
		add_buf(output,buf);
	    }
	    else
	    {
		add_buf(output,"     ");
	    }
	}
	add_buf(output,prgpstrShow[iShow]);
	add_buf(output,"\n\r");
	free_string( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
	if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
	    send_to_char( "     ", ch );
	send_to_char( "Nothing.\n\r", ch );
    }
    page_to_char(buf_string(output),ch);

    /*
     * Clean up.
     */
    free_buf(output);
    free_mem( prgpstrShow, count * sizeof(char *) );
    free_mem( prgnShow,    count * sizeof(int)    );

    return;
}



void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH],message[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if ( IS_SET(victim->comm,COMM_IAW     )   ) strcat( buf, "{W({yIAW{W){x "        );
    if (!IS_NPC(victim) && !victim->desc      ) strcat( buf, "{B*{rLD{B*{x " );
    if ( IS_SET(victim->comm,COMM_AFK	  )   ) strcat( buf, "{W[{RAFK{W]{x "	     );
    if ( victim->incog_level >= LEVEL_HERO    ) strcat( buf, "{W({cI{W){x "         );
    if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "{W({BInvis{W){x "      );
    if ( victim->invis_level >= LEVEL_HERO    ) strcat( buf, "{W({gW{W){x "	     );
    if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "{W({DHide{W){x "       );
    if ( IS_AFFECTED(victim, AFF_CHARM)       ) strcat( buf, "{W({GCharmed{W){x "    );
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "{W({cTranslucent{W){x ");
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "{W({mPink Aura{W){x "  );
    if ( IS_EVIL(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)     ) strcat( buf, "{W({rRed Aura{W){x "   );
    if ( IS_GOOD(victim)
    &&   IS_AFFECTED(ch, AFF_DETECT_GOOD)     ) strcat( buf, "{W({YSteelen Aura{W){x ");
    if ( IS_AFFECTED(victim, AFF_SANCTUARY)   ) strcat( buf, "{W(White Aura){x " );
    if ( IS_AFFECTED2(victim, AFF_FLAMESHIELD)) strcat( buf, "{r(Fiery Aura){x " );
    if ( IS_AFFECTED2(victim, AFF_LIGHTNING))   strcat( buf, "{c(Crackeling){X " );
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER ) )
						strcat( buf, "({RKILLER{x) "     );
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF  ) )
						strcat( buf, "({yTHIEF{x) "      );
    if ( IS_NPC(victim) && (ch->questmob > 0) && (victim->pIndexData->vnum == ch->questmob) )
    						strcat( buf, "[{RTARGET{x] " );
    if(IS_NPC(victim) && victim->master != NULL
     && victim == victim->master->pet
     && victim->master->position == POS_RIDING)
    {
    	sprintf(message, "%s is here, ridden by %s.\n\r", victim->short_descr, victim->master->name);
    	strcat(buf, message);
    	send_to_char(buf, ch);
    	return;
    }
    if ( victim->position == victim->start_pos && victim->long_descr[0] != '\0' )
    {
	strcat( buf, victim->long_descr );
	send_to_char( buf, ch );
	return;
    }

    strcat( buf, PERS( victim, ch ) );
    if ( !IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF) 
    &&   victim->position == POS_STANDING && ch->on == NULL )
	strcat( buf, victim->pcdata->title );

    switch ( victim->position )
    {
    case POS_DEAD:     strcat( buf, " is DEAD!!" );              break;
    case POS_MORTAL:   strcat( buf, " is mortally wounded." );   break;
    case POS_INCAP:    strcat( buf, " is incapacitated." );      break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned." ); break;
    case POS_SLEEPING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],SLEEP_AT))
  	    {
		sprintf(message," is sleeping at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],SLEEP_ON))
	    {
		sprintf(message," is sleeping on %s.",
		    victim->on->short_descr); 
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message, " is sleeping in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
	else 
	    strcat(buf," is sleeping here.");
	break;
    case POS_RESTING:  
        if (victim->on != NULL)
	{
            if (IS_SET(victim->on->value[2],REST_AT))
            {
                sprintf(message," is resting at %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else if (IS_SET(victim->on->value[2],REST_ON))
            {
                sprintf(message," is resting on %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else 
            {
                sprintf(message, " is resting in %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
	}
        else
	    strcat( buf, " is resting here." );       
	break;
    case POS_SITTING:  
        if (victim->on != NULL)
        {
            if (IS_SET(victim->on->value[2],SIT_AT))
            {
                sprintf(message," is sitting at %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else if (IS_SET(victim->on->value[2],SIT_ON))
            {
                sprintf(message," is sitting on %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
            else
            {
                sprintf(message, " is sitting in %s.",
                    victim->on->short_descr);
                strcat(buf,message);
            }
        }
        else
	    strcat(buf, " is sitting here.");
	break;
    case POS_STANDING: 
	if (victim->on != NULL)
	{
	    if (IS_SET(victim->on->value[2],STAND_AT))
	    {
		sprintf(message," is standing at %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	    else if (IS_SET(victim->on->value[2],STAND_ON))
	    {
		sprintf(message," is standing on %s.",
		   victim->on->short_descr);
		strcat(buf,message);
	    }
	    else
	    {
		sprintf(message," is standing in %s.",
		    victim->on->short_descr);
		strcat(buf,message);
	    }
	}
    	else
    	    strcat( buf, " is here." );               
	break;
    case POS_FIGHTING:
	strcat( buf, " is here, fighting " );
	if ( victim->fighting == NULL )
	    strcat( buf, "thin air??" );
	else if ( victim->fighting == ch )
	    strcat( buf, "YOU!" );
	else if ( victim->in_room == victim->fighting->in_room )
	{
	    strcat( buf, PERS( victim->fighting, ch ) );
	    strcat( buf, "." );
	}
	else
	    strcat( buf, "someone who left??" );
	break;
    case POS_RIDING:
    	sprintf(message, " is riding on %s.", victim->pet->short_descr);
    	strcat( buf, message );
    	break;
    }

    strcat( buf, "\n\r" );
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;

    if ( can_see( victim, ch ) )
    {
	if (ch == victim)
	    act( "$n looks at $mself.",ch,NULL,NULL,TO_ROOM);
	else
	{
	    act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
	    act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
	}
    }

    if ( victim->description[0] != '\0' )
    {
	send_to_char( victim->description, ch );
    }
    else
    {
	act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    if ( victim->max_hit > 0 )
	percent = ( 100 * victim->hit ) / victim->max_hit;
    else
	percent = -1;

    strcpy( buf, PERS(victim, ch) );

    if (percent >= 100) 
	strcat( buf, " is in excellent condition.\n\r");
    else if (percent >= 90) 
	strcat( buf, " has a few scratches.\n\r");
    else if (percent >= 75) 
	strcat( buf," has some small wounds and bruises.\n\r");
    else if (percent >=  50) 
	strcat( buf, " has quite a few wounds.\n\r");
    else if (percent >= 30)
	strcat( buf, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
	strcat ( buf, " looks pretty hurt.\n\r");
    else if (percent >= 0 )
	strcat (buf, " is in awful condition.\n\r");
    else
	strcat(buf, " is bleeding to death.\n\r");

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
	&&   can_see_obj( ch, obj ) )
	{
	    if ( !found )
	    {
		send_to_char( "\n\r", ch );
		act( "$N is using:", ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    send_to_char( where_name[iWear], ch );
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
    }

    if ( victim != ch
    &&   !IS_NPC(ch)
    &&   number_percent( ) < get_skill(ch,gsn_peek))
    {
	send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
	check_improve(ch,gsn_peek,TRUE,4);
	show_list_to_char( victim->carrying, ch, TRUE, TRUE );
        send_to_char( "\n\r", ch);
        sprintf( buf, "{C%d Steel{y %d Gold{x", victim->steel, victim->gold);
        send_to_char( buf, ch);
    }

    return;
}



void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
	if ( rch == ch )
	    continue;

	if ( get_trust(ch) < rch->invis_level)
	    continue;
	
	if (!IS_NPC(rch) && get_trust(ch) < rch->level
	    && IS_SET(rch->pcdata->immflag, IMMF_ROOMWIZ))
	    continue;

	if ( can_see( ch, rch ) )
	{
	    show_char_to_char_0( rch, ch );
	}
	else if ( room_is_dark( ch->in_room )
	&&        IS_AFFECTED(rch, AFF_INFRARED ) )
	{
	    send_to_char( "You see glowing red eyes watching YOU!\n\r", ch );
	}
    }

    return;
} 

bool check_blind( CHAR_DATA *ch )
{

    if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
	return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    { 
	send_to_char( "You can't see a thing!\n\r", ch ); 
	return FALSE; 
    }

    return TRUE;
}

void do_dice(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int dice;
    int number, number2;
    int roll;   
    
    argument = one_argument(argument, arg1);
    one_argument(argument, arg2);
    
    if (arg1[0] != '\0' && arg2[0] != '\0')
    {
    	if(!is_number(arg1) || !is_number(arg2))
	{
    	    send_to_char("You must enter two numbers only.\n\r", ch);
    	    return;
	}
    	number = atoi(arg1);
    	dice = atoi(arg2);
    }
    else if(arg1[0] == '\0')
    {
    	number = 2;
    	dice = 6;
    }
    else
    {
    	send_to_char("Syntax: dice <number of dice> <maximum value>\n\r", ch);
    	send_to_char("        dice\n\r", ch);
    	return;
    }
    
    if(number > 20 || number < 1
      || dice > 100 || dice < 1)
    {
    	send_to_char("Get real.\n\r", ch);
    	return;
    }

    number2 = number;
    for(roll = 0; number2 > 0; number2--)
        roll += number_range(1, dice);

    sprintf(buf, "You rolled %d from %dd%d.\n\r", roll, number, dice);
    send_to_char(buf, ch);
    sprintf(buf, "With extreme precision, %s rolls %d from %dd%d.", !IS_NPC(ch) ? ch->name : ch->short_descr, roll, number, dice);
    act("$t", ch, buf, NULL, TO_ROOM);
    return;
}  

void do_finger(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *pch = NULL;
    bool bLoaded = FALSE;
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    
    if(arg1[0] == '\0')
    /* Show the stuff */
    {
    	send_to_char("Syntax: finger <player>\n\r", ch);
    	send_to_char("        finger set <option> <string>\n\r", ch);
    	return;
    }
    
    if(!str_cmp(arg1, "set"))
    {
    	argument = one_argument(argument, arg2);
    	smash_tilde(argument);
    	
    	if(!str_cmp(arg2, "name"))
    	{
    	    free_string(ch->pcdata->fngr_name);
    	    ch->pcdata->fngr_name = str_dup(argument);
    	    sprintf(buf, "Real name set to %s.\n\r", argument);
    	    send_to_char(buf, ch);
    	    return;
    	}
    	
    	if(!str_cmp(arg2, "email"))
    	{
    	    free_string(ch->pcdata->fngr_email);
    	    ch->pcdata->fngr_email = str_dup(argument);
    	    sprintf(buf, "Email set to %s.\n\r", argument);
    	    send_to_char(buf, ch);
    	    return;
    	}
    	
    	if(!str_cmp(arg2, "icq"))
    	{
    	    free_string(ch->pcdata->fngr_icq);
    	    ch->pcdata->fngr_icq = str_dup(argument);
    	    sprintf(buf, "ICQ set to %s.\n\r", argument);
    	    send_to_char(buf, ch);
    	    return;
    	}
    	
    	if(!str_cmp(arg2, "aim"))
    	{
    	    free_string(ch->pcdata->fngr_aim);
    	    ch->pcdata->fngr_aim = str_dup(argument);
    	    sprintf(buf, "AIM set to %s.\n\r", argument);
    	    send_to_char(buf, ch);
    	    return;
    	}
    	send_to_char("Invalid set option.\n\r", ch);
    	return;
    }
    
    /* Check for a match online first */
    if(!str_cmp(arg1, "self"))
    	pch = ch;
    else
    {
    	DESCRIPTOR_DATA *des;
    	for(des = descriptor_list; des != NULL; des = des->next)
    	{
    	    if(des->connected == CON_PLAYING
    	    && !str_prefix(des->character->name, arg1))
    	    {
    	    	pch = des->character;
    		break;
    	    }
    	}
    }

    if(pch == NULL)
    {
    	bool bIsChar;
    	DESCRIPTOR_DATA d;
    	/* Check the pfiles now */
    	
    	bIsChar = load_char_obj(&d, arg1);
    	if(!bIsChar)
    	{
    	    sprintf(buf, "%s does not exist.\n\r", arg1);
    	    send_to_char(buf, ch);
    	    return;
    	}
    	
    	d.character->desc = NULL;
  	d.character->next = char_list;
  	char_list = d.character;
  	d.connected = CON_PLAYING;
  	reset_char(d.character);
  	
  	pch = d.character;
  	char_to_room(pch, pch->in_room);
  	bLoaded = TRUE;
    }
    
    if(IS_NPC(pch))
    {
    	send_to_char("Not on NPCs. Please type the full name.\n\r", ch);
    	return;
    }
    
    if(pch->level >= LEVEL_IMMORTAL && (!IS_IMMORTAL(ch) || get_trust(ch) <= LEVEL_IMMORTAL)
    && pch != ch)
    {
    	    sprintf(buf, "%s does not exist.\n\r", arg1);
    	    send_to_char(buf, ch);
    if(bLoaded)
    {
    	extract_char(pch, TRUE);
    }
    	return;
    }
    strcpy(buf,"{y****************************************************************{x\n\r");
    sprintf(buf1, "{wName: %-15s", pch->name);
    strcat(buf, buf1);
    if(IS_IMMORTAL(ch))
    	sprintf(buf1, "  {BLast Logon:{W %-25s{x\r",
           pch->pcdata->lastlogon != 0 ? (char *) ctime(&pch->pcdata->lastlogon) : (char *) ctime(&pch->logon));
    else
    	strcpy(buf1, "{x\n\r");
    strcat(buf, buf1);
    if(!bLoaded && can_see(ch, pch))
    {
    	strcat(buf, "({GCurrently playing{x)     ");
    	if(IS_IMMORTAL(ch))
    	    sprintf(buf1, "Connected: {W%s{x\r", (char *) ctime(&pch->logon));
    	else
    	    strcpy(buf1, "\n\r");
    	strcat(buf, buf1);
    }
    strcat(buf, "{y****************************************************************{x\n\r");
    sprintf(buf1, "{rReal Name: {R%s{x\n\r", pch->pcdata->fngr_name);
    strcat(buf, buf1);
    sprintf(buf1, "{yEmail: {Y%s\n\r", pch->pcdata->fngr_email);
    strcat(buf, buf1);
    sprintf(buf1, "{gICQ: {G%s\n\r", pch->pcdata->fngr_icq);
    strcat(buf, buf1);
    sprintf(buf1, "{mAIM: {M%s\n\r", pch->pcdata->fngr_aim);
    strcat(buf, buf1);
    
    if(IS_IMMORTAL(ch) && get_trust(ch) >= get_trust(pch))
    {
        sprintf(buf1, "\n\r{mLast known host: {M%s\n\r", pch->pcdata->fngr_lasthost);
        strcat(buf, buf1);
    	strcat(buf, "{y****************************************************************{x\n\r");
    	sprintf(buf1, "{mLevel: {W%d{x\n\r", pch->level);
    	strcat(buf, buf1);
    }
    
    send_to_char(buf, ch);
    send_to_char("{x", ch);
    if(bLoaded)
    {
    	extract_char(pch, TRUE);
    }
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    one_argument(argument,arg);
    
    if (arg[0] == '\0')
    {
	if (ch->lines == 0)
	    send_to_char("You do not page long messages.\n\r",ch);
	else
	{
	    sprintf(buf,"You currently display %d lines per page.\n\r",
		    ch->lines + 2);
	    send_to_char(buf,ch);
	}
	return;
    }

    if (!is_number(arg))
    {
	send_to_char("You must provide a number.\n\r",ch);
	return;
    }

    lines = atoi(arg);

    if (lines == 0)
    {
        send_to_char("Paging disabled.\n\r",ch);
        ch->lines = 0;
        return;
    }

    if (lines < 10 || lines > 100)
    {
	send_to_char("You must provide a reasonable number.\n\r",ch);
	return;
    }

    sprintf(buf,"Scroll set to %d lines.\n\r",lines);
    send_to_char(buf,ch);
    ch->lines = lines - 2;
}

/* Craps game for gambler flagged mobs */
void do_craps(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int bet, roll, point;

    for(rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
        if(IS_NPC(rch) && IS_SET(rch->act2, ACT_GAMBLER))
            break;
    }
    
    if(rch == NULL)
    {
    	send_to_char("You can't do that here.\n\r", ch);
    	return;
    }
    
    /* Grab the player's bet */
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    one_argument(argument, arg3);
    
    if(arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
    	send_to_char("Syntax: CRAPS <amount> <steel|gold> <pass|nopass>\n\r", ch);
    	return;
    }
    
    if(!is_number(arg1) || (bet = atoi(arg1)) < 0)
    {
        send_to_char("Bet is invalid. Please try again.\n\r", ch);
        return;
    }
    
    if(str_cmp(arg2, "steel") && str_cmp(arg2, "gold"))
    {
    	send_to_char("What are you betting, steel or gold?\n\r", ch);
    	return;
    }
    
    if(str_cmp(arg3, "pass") && str_cmp(arg3, "nopass"))
    {
    	send_to_char("Valid bets are 'pass' or 'nopass'.\n\r", ch);
    	return;
    }
    
    if(bet>2000)
	{
	   send_to_char("Wow... to much money for me to handle.\n\r", ch);
	   return;
	}
    if(((ch->steel - bet < 0) && !str_cmp(arg2, "steel")) 
    || ((ch->gold - bet < 0) && !str_cmp(arg2, "gold")))
    {
    	send_to_char("You don't have that much to bet.\n\r", ch);
    	return;
    }
    
    if(!str_cmp(arg2, "steel"))
    	ch->steel -= bet;
    else
    	ch->gold -= bet;
    
    /* Come out roll */
    roll = number_range(1,6);
    roll += number_range(1,6);
    
    sprintf(buf, "Come out roll is %d.", roll);
    do_function(rch, &do_say, buf);
    
    /* Check for winners */
    switch(roll)
    {
    case 7:
    case 11:
    	sprintf(buf, "%d is a winner. Pass wins.", roll);
    	do_function(rch, &do_say, buf);
    	if(!str_cmp(arg3, "pass"))
    	{
    	    bet *= 2;
    	    sprintf(buf, "You win %d %s.\n\r", bet,
    	    !str_cmp(arg2, "steel") ? "steel" : "gold");
    	    send_to_char(buf, ch);
    	    if(!str_cmp(arg2, "steel"))
    	    	ch->steel += bet;
    	    else
    	    	ch->gold += bet;
    	    return;
    	}
    	else /* Bet must have been nopass */
    	{
    	    send_to_char("You lost. The dealer takes your bet.\n\r", ch);
    	    return;
    	}
    	break;
    case 2:
    case 3:
    case 12:
    	sprintf(buf, "%d is craps. Pass loses.", roll);
    	do_function(rch, &do_say, buf);
    	if(!str_cmp(arg3, "pass"))
    	{
    	    send_to_char("You lost. The dealer removes your bet.\n\r", ch);
    	    return;
    	}
    	else /* Bet must have been nopass */
    	{
    	    bet *= 2;
    	    sprintf(buf, "You win %d %s.\n\r", bet,
    	    !str_cmp(arg2, "steel") ? "steel" : "gold");
    	    send_to_char(buf, ch);
    	    if(!str_cmp(arg2, "steel"))
    	    	ch->steel += bet;
    	    else
    	    	ch->gold += bet;
    	    return;
    	}
    	break;
    }
    
    /* Point established */
    point = roll;
    sprintf(buf, "Mark the %d.", point);
    do_function(rch, &do_say, buf);
    
    for(;;)
    {
    	roll = number_range(1,6);
    	roll += number_range(1,6);
    	sprintf(buf, "Roll was %d.", roll);
    	do_function(rch, &do_recho, buf);
	if(roll == point || roll == 7)
    	    break;
    }
    
    if(roll == point)
    {
    	if(!str_cmp(arg3, "pass"))
    	{
    	    bet *= 2;
    	    sprintf(buf, "You win %d %s.\n\r", bet,
    	    !str_cmp(arg2, "steel") ? "steel" : "gold");
    	    send_to_char(buf, ch);
    	    if(!str_cmp(arg2, "steel"))
    	    	ch->steel += bet;
    	    else
    	    	ch->gold += bet;
    	    return;
    	}
    	else
    	{
    	    send_to_char("You lost your bet.\n\r", ch);
    	    return;
    	}
    }
    else /* Rolled a 7 */
    {
    	if(!str_cmp(arg3, "pass"))
    	{
    	    send_to_char("You lost your bet.\n\r", ch);
    	    return;
    	}
    	else
    	{
    	    bet *= 2;
    	    sprintf(buf, "You win %d %s.\n\r", bet,
    	    !str_cmp(arg2, "steel") ? "steel" : "gold");
    	    send_to_char(buf, ch);
    	    if(!str_cmp(arg2, "steel"))
    	    	ch->steel += bet;
    	    else
    	    	ch->gold += bet;
    	    return;
    	}
    }
}

/* Banking code - JN */

void bank_balance(CHAR_DATA *ch)
{
    char buf[MAX_STRING_LENGTH];
    
    sprintf(buf, "Your bank balance is: %ld steel, %ld gold.\n\r", ch->pcdata->banksteel, ch->pcdata->bankgold);
    send_to_char(buf, ch);
    send_to_char("Saving...", ch);
    save_char_obj(ch);
    send_to_char(" done.\n\r", ch);
}

bool bank_deposit(CHAR_DATA *ch, int amount, const char* type)
{
    char buf[MAX_STRING_LENGTH];
    
    if(!str_cmp(type, "all"))
    {
        /* bank deposit all */
        ch->pcdata->banksteel += ch->steel;
        ch->pcdata->bankgold += ch->gold;
        sprintf(buf, "You have deposited all your money: %ld steel, %ld gold.\n\r", ch->steel, ch->gold);
        send_to_char(buf, ch);
        ch->steel = ch->gold = 0;
        return TRUE;
    }
    /* How much do they want to bank? */
    if(!str_prefix(type, "steel"))
    {
        if(ch->steel - amount < 0)
        {
            send_to_char("You can't deposit more steel than you have on hand!\n\r", ch);
            return FALSE;
        }
            
        ch->steel -= amount;
        if(ch->pcdata->banksteel < 0)
            ch->pcdata->banksteel = 0;
            
        ch->pcdata->banksteel += amount;
        sprintf(buf, "%d steel has been deposited into your account.\n\r", amount);
    }
    else if(!str_cmp(type, "gold"))
    {
        if(ch->gold - amount < 0)
        {
            send_to_char("You can't deposit more gold than you have on hand!\n\r", ch);
            return FALSE;
        }
            
        ch->gold -= amount;
        if(ch->pcdata->bankgold < 0)
            ch->pcdata->bankgold = 0;
            
        ch->pcdata->bankgold += amount;
        sprintf(buf, "%d gold has been deposited into your account.\n\r", amount);
    }
    else
    {
        send_to_char("What do you want to deposit?\n\r", ch);
        return FALSE;
    }

    send_to_char(buf, ch);
    return TRUE;
}

bool bank_withdraw(CHAR_DATA *ch, int amount, const char* type)
{
    char buf[MAX_STRING_LENGTH];

    if(!str_prefix(type, "steel"))
    {
        if(ch->pcdata->banksteel < 0)
            ch->pcdata->banksteel = 0;
            
        if(ch->pcdata->banksteel - amount < 0)
        {
            send_to_char("You can't withdraw more than you have in the bank.\n\r", ch);
            return FALSE;
        }
            
        ch->pcdata->banksteel -= amount;
        ch->steel += amount;
        sprintf(buf, "%d steel has been withdrawn from your bank.\n\r", amount);
    }
    else if(!str_cmp(type, "gold"))
    {
        if(ch->pcdata->bankgold < 0)
            ch->pcdata->bankgold = 0;
            
        if(ch->pcdata->bankgold - amount < 0)
        {
            send_to_char("You can't withdraw more than you have in the bank.\n\r", ch);
            return FALSE;
        }
            
        ch->pcdata->bankgold -= amount;
        ch->gold += amount;
        sprintf(buf, "%d gold has been withdrawn from your bank.\n\r", amount);
    }
    else
    {
        send_to_char("What do you want to withdraw?\n\r", ch);
        return FALSE;
    }
        
    send_to_char(buf, ch);
    return TRUE;
}

void do_bank(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *bch; /* Bank Mob */
    bool bIsBank = FALSE;
    char arg1[MAX_INPUT_LENGTH]; /* Teller command */
    char arg2[MAX_INPUT_LENGTH]; /* Amount to bank */
    char arg3[MAX_INPUT_LENGTH]; /* steel/gold */
    char arg4[MAX_INPUT_LENGTH]; /* Optional: transfer target */
    int amount = 0;
    
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    argument = one_argument(argument, arg4);
    
    for(bch = ch->in_room->people; bch != NULL; bch = bch->next_in_room)
    {
        if(IS_NPC(bch) && IS_SET(bch->act2, ACT_BANKER))
        {
            bIsBank = TRUE;
            break;
        }
    }

    if(IS_IMMORTAL(ch))
        bIsBank = TRUE;
            
    if(!bIsBank)
    {
        send_to_char("You can't do that here.\n\r", ch);
        return;
    }
    if(arg1[0] == '\0')
    {
        send_to_char("Usage: bank <command> <amount> [target]\n\r"
                     "command: deposit, withdraw, balance, transfer\n\r", ch);
        return;
    }
    
    if(arg2[0] != '\0')
    {
        /* Idiot check */
        amount = atoi(arg2);
        if(!is_number(arg2) || amount < 0)
        {
            send_to_char("Invalid amount entered. Please try again.\n\r", ch);
            return;
        }
    }

    if(!str_prefix(arg1, "deposit"))
    {
        if(!str_cmp(arg2, "all"))
        {
            bank_deposit(ch, 0, "all");
            bank_balance(ch);
            return;
        }
        if(arg3[0] != '\0')
        {
            if(bank_deposit(ch, amount, arg3))
                bank_balance(ch);
            return;
        }
        else
        {
            if(bank_deposit(ch, amount, "gold"))
            	bank_balance(ch);
            return;
        }
    }
    
    if(!str_prefix(arg1, "transfer"))
    {
        char buf[MAX_STRING_LENGTH];
        CHAR_DATA *vch;
        if(arg4[0] == '\0')
        {
            send_to_char("Transfer to whom?\n\r", ch);
            return;
        }
        if((vch = get_char_world(ch, arg4)) == NULL)
        {
            send_to_char("They aren't here.\n\r", ch);
            return;
        }
        
        if(IS_NPC(vch))
        {
            send_to_char("Cannot transfer to NPCs.\n\r", ch);
            return;
        }
        
        if(bank_withdraw(ch, amount, arg3))
        {
            /* Give the victim some money to bank */
            if(!str_prefix(arg3, "steel"))
            {
                vch->steel += amount;
                ch->steel -= amount;
            }
            else
            {
                vch->gold += amount;
                ch->gold -= amount;
            }
            
            sprintf(buf, "{g[{GBANK{g]{x Money transfer to {W%s{x.\n\r", vch->name);
            send_to_char(buf, ch);
            act("{g[{GBANK{g]{x Incoming money transfer from {W$N{x.", vch, NULL, ch, TO_CHAR);
            bank_deposit(vch, amount, arg3);
            bank_balance(ch);
            bank_balance(vch);
            strcpy(buf, "{g[{GBANK{g] {xTransfer completed!\n\r");
            send_to_char(buf, ch);
            send_to_char(buf, vch);
            sprintf(buf, "%s has transferred money to %s: %d %s.\n\r", ch->name, vch->name, amount, arg3);
            wiznet(buf, NULL, NULL, WIZ_BTRANS, 0, 0);
        }
        else
            send_to_char("Transfer Failed.\n\r", ch);
        
        return;
    }
    
    if(!str_prefix(arg1, "withdraw"))
    {
    	if(arg3[0] == '\0')
    	{
    	    if(bank_withdraw(ch, amount, "gold"))
    	        bank_balance(ch);
    	}
    	else
            if(bank_withdraw(ch, amount, arg3))
            	bank_balance(ch);
        return;
    }
    
    if(!str_prefix(arg1, "balance"))
    {
        bank_balance(ch);
        return;
    }
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int iSocial;
    int col;
     
    col = 0;
   
    for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
	sprintf(buf,"%-12s",social_table[iSocial].name);
	send_to_char(buf,ch);
	if (++col % 6 == 0)
	    send_to_char("\n\r",ch);
    }

    if ( col % 6 != 0)
	send_to_char("\n\r",ch);
    return;
}


 
/* RT Commands to replace news, motd, imotd, etc from ROM */

void do_motd(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "motd");
}

void do_imotd(CHAR_DATA *ch, char *argument)
{  
    do_function(ch, &do_help, "imotd");
}

void do_rules(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "rules");
}

/*
void do_story(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "story");
}

*/
/*
void do_war(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "war");
}
*/

void do_wizlist(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void do_autolist(CHAR_DATA *ch, char *argument)
{
    /* lists most player flags */
    if (IS_NPC(ch))
      return;

    send_to_char("  {caction        {Wstatus{x\n\r",ch);

send_to_char("{c+{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W+{x\n\r"
,ch);
 
    send_to_char("{W|                                    |{x\n\r",ch);

    send_to_char("{c| {Wautoassist{x     ",ch);
    if (IS_SET(ch->act,PLR_AUTOASSIST))
        send_to_char("{YON                  {c|{x\n\r",ch);
    else
        send_to_char("{DOFF                 {c|{x\n\r",ch); 

    send_to_char("{c| {Yautodamage{x     ",ch);
    if (IS_SET(ch->act,PLR_AUTODAMAGE))
	send_to_char("{YON		    {c|{x\n\r",ch);
    else
        send_to_char("{DOFF		    {c|{x\n\r",ch);
    send_to_char("{W| {cautoexit{x       ",ch);
    if (IS_SET(ch->act,PLR_AUTOEXIT))
        send_to_char("{YON                  {W|{x\n\r",ch);
    else
        send_to_char("{DOFF                 {W|{x\n\r",ch);

    send_to_char("{c| {Wautogold{x       ",ch);
    if (IS_SET(ch->act,PLR_AUTOGOLD))
        send_to_char("{YON                  {c|{x\n\r",ch);
    else  
        send_to_char("{DOFF                 {c|{x\n\r",ch);

    send_to_char("{W| {cautoloot{x       ",ch);
    if (IS_SET(ch->act,PLR_AUTOLOOT))
        send_to_char("{YON                  {W|{x\n\r",ch);
    else
        send_to_char("{DOFF                 {W|{x\n\r",ch);

    send_to_char("{c| {Wautosac{x        ",ch);
    if (IS_SET(ch->act,PLR_AUTOSAC))
        send_to_char("{YON                  {c|{x\n\r",ch);
    else
        send_to_char("{DOFF                 {c|{x\n\r",ch);

    send_to_char("{W| {cautosplit{x      ",ch);
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
        send_to_char("{YON                  {W|{x\n\r",ch);
    else
        send_to_char("{DOFF                 {W|{x\n\r",ch);

    send_to_char("{c| {Wcompact mode{x   ",ch);
    if (IS_SET(ch->comm,COMM_COMPACT))
        send_to_char("{YON		     {c|{x\n\r",ch);
    else
        send_to_char("{DOFF		     {c|{x\n\r",ch);

    send_to_char("{W| {cprompt{x         ",ch);
    if (IS_SET(ch->comm,COMM_PROMPT))
	send_to_char("{YON		     {W|{x\n\r",ch);
    else
	send_to_char("{DOFF		     {W|{x\n\r",ch);

    send_to_char("{c| {Wbrief mode{x     ",ch);
    if (IS_SET(ch->comm,COMM_BRIEF))
        send_to_char("{YON                  {c|{x\n\r",ch);
    else
        send_to_char("{DOFF		     {c|{x\n\r",ch);

    send_to_char("{W| {ccombine items{x  ",ch);
    if (IS_SET(ch->comm,COMM_COMBINE))
	send_to_char("{YON      	     {W|{x\n\r",ch);
    else
	send_to_char("{DOFF                 {W|{x\n\r",ch);

    if (!IS_SET(ch->act,PLR_CANLOOT))
	send_to_char("{c| {gYour corpse is safe from looters.  {c|{x\n\r",ch);
    else 
        send_to_char("{c| {gYour corpse may be looted.         {c|{x\n\r",ch);

    if (IS_SET(ch->act,PLR_NOSUMMON))
	send_to_char("{W| {mYou cannot be summoned.            {W|{x\n\r",ch);
    else
	send_to_char("{W| {mYou can be summoned.	             {W|{x\n\r",ch);
   
    if (IS_SET(ch->act,PLR_NOFOLLOW))
	send_to_char("{c| {rYou are wary of unwanted followers.{c|{x\n\r",ch);
    else
	send_to_char("{c| {rYou allow followers.               {c|{x\n\r",ch);

    send_to_char("{W|                                    |{x \n\r",ch);

send_to_char("{c+{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W*{c*{W+{x\n\r"
,ch);
}

void do_autoassist(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
    
    if (IS_SET(ch->act,PLR_AUTOASSIST))
    {
      send_to_char("{DYes! Let others fight for themselves.{x\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOASSIST);
    }
    else
    {
      send_to_char("{DYou will now assist when your services are needed.{x\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOASSIST);
    }
}

void do_whoclan(CHAR_DATA *ch, char *argument)
{
   if (IS_NPC(ch))
      return;
        if (IS_SET(ch->act,PLR_CLANWHO))
        {
	   send_to_char("{bYour clan is well hidden.{x\n\r",ch);
	   REMOVE_BIT(ch->act,PLR_CLANWHO);
        }
	else
	{
	   send_to_char("{BEveryone Will see your clan{x\n\r",ch);
	   SET_BIT(ch->act,PLR_CLANWHO);
	}
}
/*
Joe's Autodamage Code
*/

void do_autodamage(CHAR_DATA *ch, char *argument)
{
   if (IS_NPC(ch))
      return;
        if (IS_SET(ch->act,PLR_AUTODAMAGE))
        {
	   send_to_char("{bYou will no longer see damage.{x\n\r",ch);
	   REMOVE_BIT(ch->act,PLR_AUTODAMAGE);
        }
	else
	{
	   send_to_char("{BYou will now see the damage you deal.{x\n\r",ch);
	   SET_BIT(ch->act,PLR_AUTODAMAGE);
	}
}
void do_pk(CHAR_DATA *ch, char *argument)
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;

   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Syntax: pk <char>.\n\r", ch );
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
	   if( !(IS_SET(victim->act, PLR_PROKILLER)) )
		{
	    	   SET_BIT( victim->act, PLR_PROKILLER );
		   send_to_char( "PKiller flag added.\n\r", ch);
		   send_to_char( "Run for your life, {RYou are now PK{x.\n{YGood Luck, You will need it.{x\n\r", victim);
		}
	   return;
	


}
void do_autoexit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOEXIT))
    {
      send_to_char("{rYou will no longer see exits.{x\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOEXIT);
    }
    else
    {
      send_to_char("{rExits will now be displayed.{x\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOEXIT);
    }
}

void do_autogold(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOGOLD))
    {
      send_to_char("{yYou refuse to loot the dead of their steel.{x\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOGOLD);
    }
    else
    {
      send_to_char("{YYou will now divest the dead of all their steel.{x\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOGOLD);
    }
}

void do_autoloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOLOOT))
    {
      send_to_char("{cYou refuse to loot the dead of all they have.{x\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOLOOT);
    }
    else
    {
      send_to_char("{CYou will now divest the dead of everything they own.{x\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOLOOT);
    }
}

void do_autosac(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOSAC))
    {
      send_to_char("{DAutosacrificing removed.{x\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSAC);
    }
    else
    {
      send_to_char("{DAutomatic corpse sacrificing set.{x\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSAC);
    }
}

void do_autosplit(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_AUTOSPLIT))
    {
      send_to_char("{gAutosplitting removed. {GGreedy bastard!{x\n\r",ch);
      REMOVE_BIT(ch->act,PLR_AUTOSPLIT);
    }
    else
    {
      send_to_char("{gAutomatic steel splitting set.{x\n\r",ch);
      SET_BIT(ch->act,PLR_AUTOSPLIT);
    }
}

void do_allauto(CHAR_DATA *ch, char *argument)
{
        if (IS_NPC(ch))
          return;

        SET_BIT(ch->act,PLR_AUTOASSIST);
        SET_BIT(ch->act,PLR_AUTOEXIT);
        SET_BIT(ch->act,PLR_AUTOGOLD);
        SET_BIT(ch->act,PLR_AUTOLOOT);
        SET_BIT(ch->act,PLR_AUTOSAC);
        SET_BIT(ch->act,PLR_AUTOSPLIT);
	SET_BIT(ch->act,PLR_AUTODAMAGE);
        send_to_char("{RYou have turn all your autos on. You stud you!{x\n\r",ch);
}

void do_brief(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_BRIEF))
    {
      send_to_char("Full descriptions activated.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_BRIEF);
    }
    else
    {
      send_to_char("Short descriptions activated.\n\r",ch);
      SET_BIT(ch->comm,COMM_BRIEF);
    }
}

void do_compact(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMPACT))
    {
      send_to_char("Compact mode removed.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMPACT);
    }
    else
    {
      send_to_char("Compact mode set.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMPACT);
    }
}

void do_show(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
    {
      send_to_char("Affects will no longer be shown in score.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_SHOW_AFFECTS);
    }
    else
    {
      send_to_char("Affects will now be shown in score.\n\r",ch);
      SET_BIT(ch->comm,COMM_SHOW_AFFECTS);
    }
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];
 
   if ( argument[0] == '\0' )
   {
	if (IS_SET(ch->comm,COMM_PROMPT))
   	{
      	    send_to_char("You will no longer see prompts.\n\r",ch);
      	    REMOVE_BIT(ch->comm,COMM_PROMPT);
    	}
    	else
    	{
      	    send_to_char("You will now see prompts.\n\r",ch);
      	    SET_BIT(ch->comm,COMM_PROMPT);
    	}
       return;
   }
 
   if( !strcmp( argument, "all" ) )
      strcpy( buf, "<%hhp %mm %vmv> ");
   else
   {
      if ( strlen(argument) > 50 )
         argument[50] = '\0';
      strcpy( buf, argument );
      smash_tilde( buf );
      if (str_suffix("%c",buf))
	strcat(buf," ");
	
   }
 
   free_string( ch->prompt );
   ch->prompt = str_dup( buf );
   sprintf(buf,"Prompt set to %s\n\r",ch->prompt );
   send_to_char(buf,ch);
   return;
}

void do_combine(CHAR_DATA *ch, char *argument)
{
    if (IS_SET(ch->comm,COMM_COMBINE))
    {
      send_to_char("Long inventory selected.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_COMBINE);
    }
    else
    {
      send_to_char("Combined inventory selected.\n\r",ch);
      SET_BIT(ch->comm,COMM_COMBINE);
    }
}

void do_noloot(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
      return;
 
    if (IS_SET(ch->act,PLR_CANLOOT))
    {
      send_to_char("Your corpse is now safe from thieves.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_CANLOOT);
    }
    else
    {
      send_to_char("Your corpse may now be looted.\n\r",ch);
      SET_BIT(ch->act,PLR_CANLOOT);
    }
}

void do_nofollow(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM))
      return;
 
    if (IS_SET(ch->act,PLR_NOFOLLOW))
    {
      send_to_char("You now accept followers.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    }
    else
    {
      send_to_char("You no longer accept followers.\n\r",ch);
      SET_BIT(ch->act,PLR_NOFOLLOW);
      die_follower( ch );
    }
}

void do_nosummon(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch))
    {
      if (IS_SET(ch->imm_flags,IMM_SUMMON))
      {
	send_to_char("You are no longer immune to summon.\n\r",ch);
	REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
      }
      else
      {
	send_to_char("You are now immune to summoning.\n\r",ch);
	SET_BIT(ch->imm_flags,IMM_SUMMON);
      }
    }
    else
    {
      if (IS_SET(ch->act,PLR_NOSUMMON))
      {
        send_to_char("You are no longer immune to summon.\n\r",ch);
        REMOVE_BIT(ch->act,PLR_NOSUMMON);
      }
      else
      {
        send_to_char("You are now immune to summoning.\n\r",ch);
        SET_BIT(ch->act,PLR_NOSUMMON);
      }
    }
}

void do_look( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    int door;
    int number,count;

    if ( ch->desc == NULL )
	return;

    if ( ch->position < POS_SLEEPING )
    {
	send_to_char( "You can't see anything but stars!\n\r", ch );
	return;
    }

    if ( ch->position == POS_SLEEPING )
    {
	send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
	return;
    }

    if ( !check_blind( ch ) )
	return;

    if ( !IS_NPC(ch)
    &&   !IS_SET(ch->act, PLR_HOLYLIGHT)
    &&   room_is_dark( ch->in_room )
    &&   !IS_AFFECTED(ch, AFF_INFRARED) )
    {
	send_to_char( "It is pitch black ... \n\r", ch );
	show_char_to_char( ch->in_room->people, ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    number = number_argument(arg1,arg3);
    count = 0;

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
	/* 'look' or 'look auto' */
	send_to_char( ch->in_room->name, ch );

	if(IS_IMMORTAL(ch) && (IS_NPC(ch) || IS_SET(ch->act,PLR_HOLYLIGHT)))
	{
	    sprintf(buf,"{r [{RRoom %d{r]{x",ch->in_room->vnum);
	    send_to_char(buf,ch);
	}

	send_to_char( "\n\r", ch );

	if ( arg1[0] == '\0'
	|| ( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) ) )
	{
	    send_to_char( "  ",ch);
	    
	    if( (weather_info.sunlight == SUN_DARK) && (ch->in_room->description2[0] != '\0') )
	    {
	    send_to_char( ch->in_room->description2, ch );
	    } else {
	    send_to_char( ch->in_room->description1, ch );
	    }
	    
	}

        if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) )
	{
	    send_to_char("\n\r",ch);
            do_function(ch, &do_exits, "auto" );
	}

	show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
	show_char_to_char( ch->in_room->people,   ch );
	return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp(arg1, "in")  || !str_cmp(arg1,"on"))
    {
	/* 'look in' */
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Look in what?\n\r", ch );
	    return;
	}

	if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You do not see that here.\n\r", ch );
	    return;
	}

	switch ( obj->item_type )
	{
	default:
	    send_to_char( "That is not a container.\n\r", ch );
	    break;

	case ITEM_DRINK_CON:
	    if ( obj->value[1] <= 0 )
	    {
		send_to_char( "It is empty.\n\r", ch );
		break;
	    }

	    sprintf( buf, "It's %sfilled with  a %s liquid.\n\r",
		obj->value[1] <     obj->value[0] / 4
		    ? "less than half-" :
		obj->value[1] < 3 * obj->value[0] / 4
		    ? "about half-"     : "more than half-",
		liq_table[obj->value[2]].liq_color
		);

	    send_to_char( buf, ch );
	    break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    if ( IS_SET(obj->value[1], CONT_CLOSED) )
	    {
		send_to_char( "It is closed.\n\r", ch );
		break;
	    }

	    act( "$p holds:", ch, obj, NULL, TO_CHAR );
	    show_list_to_char( obj->contains, ch, TRUE, TRUE );
	    break;
	}
	return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
	show_char_to_char_1( victim, ch );
	return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{  /* player can see object */
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    {
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}
	    	else continue;
	    }

 	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
 	    if ( pdesc != NULL )
	    {
 	    	if (++count == number)
 	    	{	
		    send_to_char( pdesc, ch );
		    return;
	     	}
		else continue;
	     }

	    if ( is_name( arg3, obj->name ) )
	    	if (++count == number)
	    	{
	    	    send_to_char( obj->description, ch );
	    	    send_to_char( "\n\r",ch);
		    return;
		  }
	  }
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
	if ( can_see_obj( ch, obj ) )
	{
	    pdesc = get_extra_descr( arg3, obj->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}

	    pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
	    if ( pdesc != NULL )
	    	if (++count == number)
	    	{
		    send_to_char( pdesc, ch );
		    return;
	    	}

	    if ( is_name( arg3, obj->name ) )
		if (++count == number)
		{
		    send_to_char( obj->description, ch );
		    send_to_char("\n\r",ch);
		    return;
		}
	}
    }

    pdesc = get_extra_descr(arg3,ch->in_room->extra_descr);
    if (pdesc != NULL)
    {
	if (++count == number)
	{
	    send_to_char(pdesc,ch);
	    return;
	}
    }
    
    if (count > 0 && count != number)
    {
    	if (count == 1)
    	    sprintf(buf,"You only see one %s here.\n\r",arg3);
    	else
    	    sprintf(buf,"You only see %d of those here.\n\r",count);
    	
    	send_to_char(buf,ch);
    	return;
    }

         if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door = 0;
    else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door = 1;
    else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door = 2;
    else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door = 3;
    else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door = 4;
    else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door = 5;
    else
    {
	send_to_char( "You do not see that here.\n\r", ch );
	return;
    }

    /* 'look direction' */
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
	send_to_char( "Nothing special there.\n\r", ch );
	return;
    }

    if ( pexit->description != NULL && pexit->description[0] != '\0' )
	send_to_char( pexit->description, ch );
    else
	send_to_char( "Nothing special there.\n\r", ch );

    if ( pexit->keyword    != NULL
    &&   pexit->keyword[0] != '\0'
    &&   pexit->keyword[0] != ' ' )
    {
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	{
	    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
	}
	else if ( IS_SET(pexit->exit_info, EX_ISDOOR) )
	{
	    act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
	}
    }

    return;
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA *ch, char *argument )
{
    do_function(ch, &do_look, argument);
}

void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int chance;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Examine what?\n\r", ch );
	return;
    }

    do_function(ch, &do_look, arg );

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	switch ( obj->item_type )
	{
	default:
	    break;
	
	case ITEM_JUKEBOX:
	    do_function(ch, &do_play, "list");
	    break;

	case ITEM_MONEY:
	    if (obj->value[0] == 0)
	    {
	        if (obj->value[1] == 0)
		    sprintf(buf,"Odd...there's no coins in the pile.\n\r");
		else if (obj->value[1] == 1)
		    sprintf(buf,"Wow. One steel coin.\n\r");
		else
		    sprintf(buf,"There are %d steel coins in the pile.\n\r",
			obj->value[1]);
	    }
	    else if (obj->value[1] == 0)
	    {
		if (obj->value[0] == 1)
		    sprintf(buf,"Wow. One gold coin.\n\r");
		else
		    sprintf(buf,"There are %d gold coins in the pile.\n\r",
			obj->value[0]);
	    }
	    else
		sprintf(buf,
		    "There are %d steel and %d gold coins in the pile.\n\r",
		    obj->value[1],obj->value[0]);
	    send_to_char(buf,ch);
	    break;

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	    sprintf(buf,"in %s",argument);
	    do_function(ch, &do_look, buf );
	}
    /* Lore may kick in */
    	chance = get_skill(ch, gsn_lore);
    	if(number_percent() < chance)
    	{
    	    send_to_char("You remember something from a story you once heard.\n\r", ch);
    	    check_improve(ch,gsn_lore,TRUE, 5);
    	    spell_identify(0,ch->level, ch, obj, 0);
    	}
    }

    return;
}



/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA *ch, char *argument )
{
    extern char * const dir_name[];
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door;

    fAuto  = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
	return;

    if (fAuto)
	sprintf(buf,"{W[{ME{mxits{W:{x ");
    else if (IS_IMMORTAL(ch))
	sprintf(buf,"Obvious exits from room %d:\n\r",ch->in_room->vnum);
    else
	sprintf(buf,"Obvious exits:\n\r");

    found = FALSE;
    for ( door = 0; door <= 5; door++ )
    {
	if ( ( pexit = ch->in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   can_see_room(ch,pexit->u1.to_room) )
	{
	    if ( fAuto )
	    {
		if( IS_SET(pexit->exit_info, EX_CLOSED) )
		{
			if( !IS_SET(pexit->exit_info, EX_HIDDEN) )
			{
				found = TRUE;
				sprintf( buf1, "<%s> ", dir_name[door] );
				strcat( buf, buf1 );
			} else if( IS_IMMORTAL( ch ) )
			{
				found = TRUE;
				sprintf( buf1, "[%s] ", dir_name[door] );
				strcat( buf, buf1 );
			}
		}
		else if( IS_SET(pexit->exit_info, EX_ISDOOR) )
		{
			found = TRUE;
			sprintf( buf1, "(%s) ", dir_name[door] );
			strcat( buf, buf1 );
		} else
		{
			found = TRUE;
			sprintf( buf1, "%s ", dir_name[door] );
			strcat( buf, buf1 );
		}
		
	    }
	    else
	    {
	    	if( IS_SET(pexit->exit_info, EX_CLOSED) &&
	    	   !IS_SET(pexit->exit_info, EX_HIDDEN) )
	    	{
	    		found = TRUE;
	    		sprintf( buf + strlen(buf), "%-5s - Closed",
	    			dir_name[door] );
	    	}
	    	else if( IS_SET(pexit->exit_info, EX_HIDDEN)
	    	&& 	 IS_IMMORTAL( ch ) )
	    	{
	    		found = TRUE;
	    		sprintf( buf +strlen(buf), "%-5s - Closed & Hidden",
	    			dir_name[door] );
	    	}
	    	else if( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    	{
	    		found = TRUE;
	    		sprintf( buf + strlen(buf), "%-5s - %s",
	    			dir_name[door],
	    			room_is_dark( pexit->u1.to_room )
	    			?  "Too dark to tell"
	    			: pexit->u1.to_room->name );
	    	}
	    		
		if (IS_IMMORTAL(ch))
		    sprintf(buf + strlen(buf), 
			" (room %d)\n\r",pexit->u1.to_room->vnum);
		else
		    sprintf(buf + strlen(buf), "\n\r");
	    }
	}
    }

    if ( !found )
	strcat( buf, fAuto ? "none " : "None.\n\r " );

    if ( fAuto )
	strcat( buf, "{W]{x\n\r" );

    send_to_char( buf, ch );
    return;
}

void do_worth( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    {
	sprintf(buf,"{RYou have {C%ld {Rsteel and {Y%ld {Rgold{x.\n\r",
	    ch->steel,ch->gold);
	send_to_char(buf,ch);
	return;
    }

    sprintf(buf, 
    "{RYou have {C%ld {Rsteel, {Y%ld {Rgold, and {W%d {Rexperience {x({R%d {Rexp to level{x).\n\r",
	ch->steel, ch->gold,ch->exp,
	(ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);

    send_to_char(buf,ch);

    return;
}



void do_score( CHAR_DATA *ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int i;

  if (IS_NPC(ch))
    {
      send_to_char("Use the OLD score function\n\r",ch);
      return;
    }

	printf_to_char( ch, "%s%s\n\r",
			ch->name, ch->pcdata->title);
  if( is_clan(ch) )			
  {
    printf_to_char( ch, "Clan: %s%s\n\r",
			clan_table[ch->clan].who_name,
			subclan_table[ch->subclan].who_name );
  
    sprintf( buf, "Rank: {c%s{x ({C%d{x)\n\r",
    			clanrank_name( ch ),
  			ch->clanlevel );
    send_to_char( buf, ch );
  }
	
send_to_char("{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{x\n\r", ch);
	printf_to_char( ch, "{BName{x : %-11s    {RHp{x  : %5d/%-5d    {GPrac{x  : %d\n\r",
			ch->name,
			ch->hit, ch->max_hit,
			ch->practice );
	printf_to_char( ch, "{BRace{x : %-11s    {RMana{x: %5d/%-5d    {GTrain{x : %d\n\r",
  			IS_NPC(ch) ? "Mobile " : capitalize( pc_race_table[ch->race].name ),
			ch->mana, ch->max_mana,
			ch->train );
	printf_to_char( ch, "{BClass{x: %-11s    {RMove{x: %5d/%-5d    {GWeight{x: %d/%d\n\r",
  			IS_NPC(ch) ? "Mobile " : capitalize( class_table[ch->class].name ),
  			ch->move, ch->max_move,
			get_carry_weight(ch) / 10, can_carry_w(ch) / 10 );
	printf_to_char( ch, "{BSex{x  : %-11s    {RWimp{x: %11d    {GItems{x : %d/%d\n\r",
			ch->sex == 0 ? "Sexless" : ch->sex == 1 ? "Male   " : "Female ",
			ch->wimpy,
  			ch->carry_number, can_carry_n(ch) );
	printf_to_char( ch, "{BLevel{x: %-11d   {R Hour{x: %11d    {GGod{x   : %s\n\r",
			ch->level, 
			(ch->played + (int) (current_time - ch->logon)) / 3600,
			god_table[ch->god].name);
	printf_to_char( ch, "{BAge{x  : %-11d\n\r", get_age(ch));
	send_to_char("{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{x\n\r",ch);
	printf_to_char( ch, "{BStr{x:   {R%d{x ({C%d{x)   Steel: {C%5d{x Bank Steel: {C%5d{x\n\r",
			ch->perm_stat[STAT_STR],
			get_curr_stat(ch,STAT_STR),
			ch->steel, ch->pcdata->banksteel );
	printf_to_char( ch, "{BInt{x:   {R%d{x ({C%d{x)   Gold : {y%5d{x Bank Gold : {y%5d{x\n\r",
			ch->perm_stat[STAT_INT],
			get_curr_stat(ch,STAT_INT),
			ch->gold, ch->pcdata->bankgold );
	printf_to_char( ch, "{BWis{x:   {R%d{x ({C%d{x)   XTNL : {Y%5d{x Quest Poi.: %5d\n\r",
			ch->perm_stat[STAT_WIS],
			get_curr_stat(ch,STAT_WIS),
  			IS_HERO(ch) ? (1) : (ch->level + 1) *
			  exp_per_level(ch,ch->pcdata->points) - ch->exp,
			ch->questpoints);
	printf_to_char(	ch, "{BDex{x:   {R%d{x ({C%d{x)\n\r",
			ch->perm_stat[STAT_DEX],
			get_curr_stat(ch,STAT_DEX));
	printf_to_char(	ch, "{BCon{x:   {R%d{x ({C%d{x)\n\r",
			ch->perm_stat[STAT_CON],
			get_curr_stat(ch,STAT_CON));
	
	send_to_char("{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{x\n\r",ch);
  for (i = 0; i < 4; i++)
  {
    char * temp;
 
    switch(i)
    {
      case(AC_PIERCE):	temp = "{Bpiercing.{x";	break;
      case(AC_BASH):	temp = "{Rbashing. {x";	break;
      case(AC_SLASH):	temp = "{Gslashing.{x";	break;
      case(AC_EXOTIC):	temp = "{Ymagic.   {x";	break;
      default:		temp = "error.   ";	break;
    }

    if      (GET_AC(ch,i) >=  101 ) 
      sprintf(buf,"{DHopelessly vulnerable{x to %s ",temp);
    else if (GET_AC(ch,i) >= 80) 
      sprintf(buf,"{dDefenseless{x against %s      ",temp);
    else if (GET_AC(ch,i) >= 60)
      sprintf(buf,"{yBarely protected{x from %s    ",temp);
    else if (GET_AC(ch,i) >= 40)
      sprintf(buf,"Slightly armored against %s ",temp);
    else if (GET_AC(ch,i) >= 20)
      sprintf(buf,"Somewhat armored against %s ",temp);
    else if (GET_AC(ch,i) >= 0)
      sprintf(buf,"Armored against %s          ",temp);
    else if (GET_AC(ch,i) >= -20)
      sprintf(buf,"Well-armored against %s     ",temp);
    else if (GET_AC(ch,i) >= -40)
      sprintf(buf,"Very well-armored against %s",temp);
    else if (GET_AC(ch,i) >= -60)
      sprintf(buf,"Heavily armored against %s  ",temp);
    else if (GET_AC(ch,i) >= -80)
      sprintf(buf,"Superbly armored against %s ",temp);
    else if (GET_AC(ch,i) >= -100)
      sprintf(buf,"Almost invulnerable to %s   ",temp);
    else
      sprintf(buf,"{WDivinely{x armored against %s ",temp);
  	
    if     ( i == 0 )
      printf_to_char( ch, "{BPierce{x : (%3d) %s  Hitroll: %d\n\r",
			GET_AC(ch,i),
			buf,
			GET_HITROLL(ch) );
    else if( i == 1 )
      printf_to_char( ch, "{RBash{x   : (%3d) %s  Damroll: %d\n\r",
			GET_AC(ch,i),
			buf,
			GET_DAMROLL(ch) );
    else if( i == 2 )
    {
      if( ch->level > 19 )
	sprintf( buf2, "Saves  : %d", ch->saving_throw );
      else
	sprintf( buf2, "  " );
      printf_to_char( ch, "{GSlash{x  : (%3d) %s  %s\n\r",
			GET_AC(ch,i),
			buf,
			buf2 );
    }
    else
    {
      printf_to_char( ch, "{YMagic{x  : (%3d) %s\n\r",
			GET_AC(ch,i),
			buf );
    }
  }
	
send_to_char("{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{x\n\r",ch);

  if (!IS_NPC(ch))
    {
	sprintf(buf, "{rMob Kills:{W    %4d.  {rMob Deaths:{W    %4d.\n\r",
	ch->pcdata->mkill, ch->pcdata->mdeath);
	send_to_char(buf, ch);
	sprintf(buf, "{RPlayer Kills:{D %4d.  {RPlayer Deaths:{D %4d.\n\r",
	ch->pcdata->pkill, ch->pcdata->pdeath);
	send_to_char(buf, ch);
    }
    if (ch->pcdata->spouse != NULL)
   {
    sprintf(buf, "{cYou are happily married to{m %s{x\n\r", ch->pcdata->spouse);
	send_to_char(buf, ch);
   }
    if(ch->alignment >= 350)
    sprintf(buf, "({W%5d{x) ", ch->alignment);
    else if(ch->alignment >= -350)
    sprintf(buf, "({G%5d{x) ", ch->alignment);
    else
    sprintf(buf, "({R%5d{x) ", ch->alignment);
    send_to_char( buf, ch);
    send_to_char( "You are ", ch );
  if      ( ch->alignment >  900 ) send_to_char( "{Wangelic.{x\n\r", ch );
  else if ( ch->alignment >  700 ) send_to_char( "{Ysaintly.{x\n\r", ch );
  else if ( ch->alignment >  350 ) send_to_char( "{ygood.{x\n\r",    ch );
  else if ( ch->alignment >  100 ) send_to_char( "{wkind.{x\n\r",    ch );
  else if ( ch->alignment > -100 ) send_to_char( "{rneutral.{x\n\r", ch );
  else if ( ch->alignment > -350 ) send_to_char( "{bmean.{x\n\r",    ch );
  else if ( ch->alignment > -700 ) send_to_char( "{Bevil.{x\n\r",    ch );
  else if ( ch->alignment > -900 ) send_to_char( "{Ddemonic.{x\n\r", ch );
  else                             send_to_char( "{Rsatanic.{x\n\r", ch );
  if( IS_IMMORTAL(ch) )
  {
	
send_to_char("{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{x\n\r",ch);	
    printf_to_char( ch, "{gWizi: {W%d  {cIncog: {W%d  {YHoly Light: ",
			ch->invis_level,
			ch->incog_level );
    
    if (IS_SET(ch->act,PLR_HOLYLIGHT) )
      send_to_char( "{yOn{x  \n\r", ch );
    else
      send_to_char( "{DOff{x  \n\r", ch );
    printf_to_char( ch, "{RWiziname{x is %s\n\r", ch->pcdata->wiziname);    
    if (ch->pcdata->transin[0] != '\0')
      printf_to_char( ch, "{gTrans In:{x %s\n\r", ch->pcdata->transin);
    if (ch->pcdata->transout[0] != '\0')
      printf_to_char( ch, "{gTrans Out:{x %s\n\r", ch->pcdata->transout);
  }
    
  if ( get_trust( ch ) != ch->level )
    printf_to_char(ch, "{WTrust: {w%d\n\r", get_trust( ch ) );
  else
  send_to_char("",ch); /*Don't take this out, formatting thing*/

	
send_to_char("{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{x\n\r",ch);	

  if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
  {
    do_function(ch,&do_affects,"");
	
send_to_char("{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{x\n\r",ch);	
  }


}



void do_affects(CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf, *paf_last = NULL;
    char buf[MAX_STRING_LENGTH];
    
    if ( ch->affected != NULL )
    {
	send_to_char( "{DYou are affected by the following spells:{x\n\r", ch );
	for ( paf = ch->affected; paf != NULL; paf = paf->next )
	{
	    if (paf_last != NULL && paf->type == paf_last->type)
		if (ch->level >= 20)
		    sprintf( buf, "                      ");
		else
		    continue;
	    else
	    	sprintf( buf, "{cSpell: {G%-15s", skill_table[paf->type].name );

	    send_to_char( buf, ch );

	    if ( ch->level >= 20 )
	    {
		sprintf( buf,
		    "{c- modifies {M%s {cby {Y%d{c ",
		    affect_loc_name( paf->location ),
		    paf->modifier);
		send_to_char( buf, ch );
		if ( paf->duration == -1 )
		    sprintf( buf, "permanently" );
		else
		    sprintf( buf, "for {G%d {chours", paf->duration );
		send_to_char( buf, ch );
	    }

	    send_to_char( "{x\n\r", ch );
	    paf_last = paf;
	}
    }
    else 
	send_to_char("{DYou are not affected by any spells.{x\n\r",ch);

    return;
}



char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    extern char str_boot_time[];
    char buf[MAX_STRING_LENGTH];
    char *suf;
    int day;

    day     = time_info.day + 1;

         if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    sprintf( buf,
	"It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r",
	(time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
	time_info.hour >= 12 ? "pm" : "am",
	day_name[day % 7],
	day, suf,
	month_name[time_info.month]);
    send_to_char(buf,ch);
    sprintf(buf,"Lance started up at %s\n\rThe system time is %s",
	str_boot_time,
	(char *) ctime( &current_time )
	);

    send_to_char( buf, ch );
    return;
}



void do_weather( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    static char * const sky_look[4] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE(ch) )
    {
	send_to_char( "You can't see the weather indoors.\n\r", ch );
	return;
    }

    sprintf( buf, "The sky is %s and %s.\n\r",
	sky_look[weather_info.sky],
	weather_info.change >= 0
	? "a warm southerly breeze blows"
	: "a cold northern gust blows"
	);
    send_to_char( buf, ch );
    return;
}

void do_help( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    HELP_DATA *pHelp;
    BUFFER *output;
    bool found = FALSE;
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
    int level;
    int count = 0;

    output = new_buf();

    if ( argument[0] == '\0' )
	argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0' )
    {
	argument = one_argument(argument,argone);
	if (argall[0] != '\0')
	    strcat(argall," ");
	strcat(argall,argone);
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
    	level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

	if (level > get_trust( ch ) )
	    continue;

	if ( is_name( argall, pHelp->keyword ) )
	{
	    /* add seperator if found */
	    if (found)
	    {
		send_to_char("\n\r", ch);
		send_to_char("More then one help file found for what you where looking for\n\r", ch);
		send_to_char("Here is the list of them all\n\r", ch);
		
send_to_char("{r=========={R=========={b=========={B=========={g=========={G=========={x\n\r\n\r",ch);
		for ( pHelp = help_first; pHelp != NULL; pHelp = 
pHelp->next )
		{
			level = (pHelp->level < 0) ? -1 * pHelp->level - 1 
: pHelp->level;
			
			if (level > get_trust( ch ) )
				continue;

			if ( is_name( argall, pHelp->keyword ) )
			{
				if ( pHelp->level >= 0 && str_cmp( argall, 
"imotd" ) )
	    			{
					count++;
					sprintf( buf,
						"{B<{G%-2d{R:{x ", count);
   					send_to_char( buf, ch );

					send_to_char(pHelp->keyword, ch);
					send_to_char(" {B>{x\n\r", ch);
	    			}	
			}
		}
		sprintf( buf,
			"\n\rThere was {R%d{x keywords found for what you where looking for", count);
   		send_to_char( buf, ch );
	        return;
	    }
	    	if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
	    	{
			add_buf(output,pHelp->keyword);
	    	}

	    /* 
	     * Strip leading '.' to allow initial blanks. 
	     */ 
	if ( strlen( argall ) == 1 ) 
	; // DJR HACK - display only keywords 
	else if ( pHelp->text[0] == '.' ) 
	add_buf(output,pHelp->text+1); 
	else 
	add_buf(output,pHelp->text); 
	found = TRUE; 
	    /* small hack :) */
	    if (ch->desc != NULL && ch->desc->connected != CON_PLAYING 
	    &&  		    ch->desc->connected != CON_GEN_GROUPS)
		break;
	}
    }

    if (!found)
    {
    	send_to_char( "We are always working on new help files\n\r", ch );
	send_to_char( "So always check back for the help file you are looking for", ch );
	sprintf( log_buf, "NO HELP: %s could not find a help file for %s", ch->name, argone );
        log_string( log_buf );
    }
    else
	page_to_char(buf_string(output),ch);
    free_buf(output);
}



/* whois command */
void do_whois (CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char buf4[MAX_STRING_LENGTH];
    char buf5[MAX_STRING_LENGTH];
    char buf6[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found = FALSE;

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
	send_to_char("You must provide a name.\n\r",ch);
	return;
    }

    output = new_buf();

    for (d = descriptor_list; d != NULL; d = d->next)
    {
	CHAR_DATA *wch;

 	if (d->connected != CON_PLAYING || !can_see(ch,d->character))
	    continue;
	
	wch = ( d->original != NULL ) ? d->original : d->character;

 	if (!can_see(ch,wch))
	    continue;

	if (!str_prefix(arg,wch->name))
	{
	    found = TRUE;
	    

	    /* a little formatting */

    if( wch->whotext != NULL )
    {
        if( colorstrlen( wch->whotext ) == 14 )
        {
            sprintf( buf3, "[%s]", wch->whotext );
        }

        else if( colorstrlen( wch->whotext ) == 13 )
        {
            sprintf( buf3, "[%s ]", wch->whotext );
        }

        else
	    sprintf( buf3, "[%s]", center(wch->whotext, 14) );
    }
    else
    	sprintf( buf3, "[{g%s{x]", center(capitalize(pc_race_table[wch->race].name), 14) );

    if( wch->desc != NULL && wch->desc->editor != 0 )
	sprintf( buf4, "{r({gB{r){x " );
    else
	buf4[0] = '\0';

    if( wch->clanlevel > 6 )
        sprintf( buf5, "{R<{BR{G>{x " );
    else
	buf5[0] = '\0';
    
    if( wch->clanlevel >=10 )
        sprintf( buf5, "{g<L>{x " );
  
    
    if( wch->pretitle == NULL )
	buf6[0] = '\0';
    else
	sprintf( buf6, "%s ", wch->pretitle );
            
         
    sprintf( buf, "%s %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n\r", buf3,
    clan_table[wch->clan].who_name,
    subclan_table[wch->subclan].who_name,
    buf5,
    wch->incog_level >= LEVEL_HERO ? "{W({cI{W){x " : "",
    wch->invis_level >= LEVEL_HERO ? "{W({gW{W){x " : "",
    buf4,
    IS_SET(wch->act, PLR_SLAIN) ? "({rSLAIN{x) " : "",
    IS_SET(wch->act, PLR_VIOLENT) ? "({RVIOLENT{x) " : "",
    IS_SET(wch->act, PLR_KILLER) ? "({rKILLER{x) " : "",
    IS_SET(wch->act, PLR_THIEF)  ? "({yTHIEF{x) "  : "",
    IS_SET(wch->comm, COMM_AFK) ? "{W[{RAFK{W]{x " : "",
    IS_SET(wch->comm, COMM_IAW) ? "{W({yIAW{W){x "   : "",
    IS_SET(wch->act,  PLR_ROLEPL) ? "({GRP{x)     "   : "",
    buf6,
    wch->name,
    IS_NPC(wch) ? "" : wch->pcdata->title );
    
    add_buf(output,buf);
        
    
     	}
    }

    if (!found)
    {
	send_to_char("No one of that name is playing.\n\r",ch);
	return;
    }

    page_to_char(buf_string(output),ch);
    free_buf(output);
}

/* Another new 'who' command. I don't find restrictions very useful
 * so I took it out and made it simpler. Also restrung whotexts don't
 * work well with race restrictions.
 *
 * Perhaps it could be sorted by clans in the future.
 */
void do_who(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;
    BUFFER *mort, *immort;
    BUFFER *output;
    char buf[MAX_STRING_LENGTH];
    char whotext[MAX_STRING_LENGTH];
    char plr[MAX_STRING_LENGTH];
    char pretitle[MAX_STRING_LENGTH];
    bool bImm = FALSE, bMort = FALSE;
    int immcount, mortcount;
    int tempcount;   
    mort = new_buf();
    immort = new_buf();
    buf[0] = '\0';
    immcount = 0;
    mortcount = 0;
    for(tempcount=0;tempcount<10;tempcount++)
	{
	   add_buf(immort, "{C-{B-{x");
	}
    add_buf( immort, "{C-{B-{R IMMORTALS {C-{B-");
    for(tempcount=0;tempcount<10;tempcount++)
	{
	   add_buf(immort, "{C-{B-{x");
	}
    add_buf( immort, "\n\r");
    add_buf( immort, "\n\r");
    for(tempcount=0;tempcount<10;tempcount++)
	{
	   add_buf(mort, "{C-{B-{x");
	}
    add_buf( mort, "{C-{B-{C-{R MORTALS {B-{C-{B-");
    for(tempcount=0;tempcount<10;tempcount++)
	{
	   add_buf(mort, "{C-{B-{x");
	}
    add_buf( mort, "\n\r");
    add_buf( mort, "\n\r");

    for(d = descriptor_list; d != NULL; d = d->next)
    {
    	CHAR_DATA *wch;
    	
    	if(d->connected != CON_PLAYING)
    	    continue;
    	
    	wch = (d->character != NULL) ? d->character : d->original;
    	
    	if(!IS_NPC(wch) && can_see(ch, wch))
    	{
    	    if(wch->whotext != NULL)
    	    {
		if( colorstrlen( wch->whotext ) == 14 )
	    	    sprintf( whotext, "[%s]", wch->whotext );
		else if( colorstrlen( wch->whotext ) == 13 )
	    	    sprintf( whotext, "[%s ]", wch->whotext );
		else
	    	    sprintf( whotext, "[%s]", center(wch->whotext, 14) );
    	    }
    	    else
	    	sprintf( whotext, "[{g%s{x]", center(capitalize(pc_race_table[wch->race].name), 14) );
    	    	
    	    plr[0] = '\0';
	    if(IS_SET(wch->act, PLR_PROKILLER)) strcat(plr, "{R+{x ");
	    if(!(IS_SET(wch->act, PLR_PROKILLER))) strcat(plr, "{B {x ");
    	    if(IS_SET(wch->comm, COMM_AFK)) strcat(plr, "{W[{RAFK{W]{x ");
    	    if(IS_SET(wch->comm, COMM_IAW)) strcat(plr, "{W({yIAW{W){x ");
    	    if(wch->incog_level >= LEVEL_IMMORTAL) strcat(plr, "{W({cI{W){x ");
    	    if(wch->invis_level >= LEVEL_IMMORTAL) strcat(plr, "{W({gW{W){x ");
    	    if(IS_SET(wch->act, PLR_SLAIN)) strcat(plr, "({rSLAIN{x) ");
    	    if(IS_SET(wch->act, PLR_VIOLENT)) strcat(plr, "({RVIOLENT{x) ");
    	    if(IS_SET(wch->act, PLR_THIEF)) strcat(plr, "({yTHIEF{x) ");
    	    if(IS_SET(wch->act, PLR_KILLER)) strcat(plr, "({rKILLER{x) ");
    	    if(IS_SET(wch->act, PLR_ROLEPL)) strcat(plr, "({GRP{x) ");
    	    if(!IS_SET(wch->act, PLR_CLANWHO)&&!IS_IMMORTAL(ch)
		&& !is_same_clan(ch, wch))
		strcat(plr, "");
	    else
		{
		strcat(plr, clan_table[wch->clan].who_name);
		strcat(plr, subclan_table[wch->subclan].who_name);
    	        if(wch->clanlevel > 6 && wch->clanlevel < 10)
		   strcat(plr, " {C<{gR{C> {x");
                if(wch->clanlevel >= 10) 
		   strcat(plr, "{C<{WL{C>{x ");
		}
    	    if(wch->pretitle != NULL)
    	    	sprintf(pretitle, "%s ", wch->pretitle);
    	    else
    	    	strcpy(pretitle, "");
    	    
    	/* race, vio/thief/afk/etc., clan, subclan, <L><R>, pretitle, name, title */
	/* For goto in pico use WHOWHO */
     	    sprintf(buf, "%s %s%s%s%s{x\n\r",
    	    whotext,
    	    plr,
    	    pretitle,
    	    wch->name,
    	    wch->pcdata->title);

    	    if(wch->level >= IMMORTAL)
    	    {
    	    	immcount++;
    	    	add_buf(immort, buf);
    	    	bImm = TRUE;
    	    }
    	    else
    	    {
    	    	mortcount++;
    	    	add_buf(mort, buf);
    	    	bMort = TRUE;
    	    }
    	}
    }
    
    output = new_buf();
    if(bImm)
    	add_buf(output, buf_string(immort));
    if(bImm && bMort)
    	add_buf(output, "\n\r");
    if(bMort)
    	add_buf(output, buf_string(mort));
    	
    sprintf(buf, "\n\r{RImmortals visible: {r%d\n\r{cMortals visible: {r%d{x\n\r",
    immcount, mortcount);
    add_buf(output, buf);

    page_to_char(buf_string(output), ch);
    
    free_buf(immort);
    free_buf(mort);
    free_buf(output);
}

/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
#if 0
void do_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char buf4[MAX_STRING_LENGTH];
    char buf5[MAX_STRING_LENGTH];
    char buf6[MAX_STRING_LENGTH];
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int iClass;
    int iRace;
    int iClan;
    int nNumber;
    int nMatch;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_PC_RACE];
    bool rgfClan[MAX_CLAN];
    bool fClassRestrict = FALSE;
    bool fClanRestrict = FALSE;
    bool fClan = FALSE;
    bool fRaceRestrict = FALSE;
    bool fImmortalOnly = FALSE;
 
    /*
     * Set default arguments.
     */
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace <= MAX_PC_RACE; iRace++ )
        rgfRace[iRace] = FALSE;
    for (iClan = 0; iClan < MAX_CLAN; iClan++)
	rgfClan[iClan] = FALSE;
 
    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
        char arg[MAX_STRING_LENGTH];
 
        argument = one_argument( argument, arg );
        if ( arg[0] == '\0' )
            break;
 
        /*
         * Look for classes to turn on.
         */
        if (!str_prefix(arg,"immortals"))
        {
            fImmortalOnly = TRUE;
        }
        else
        {
            iClass = class_lookup(arg);
            if (iClass == -1)
            {
                iRace = race_lookup(arg);

                if (iRace == 0 || iRace > MAX_PC_RACE)
		{
			if (!str_prefix(arg,"clan"))
			    fClan = TRUE;
			else
		        {
			    iClan = clan_lookup(arg);
			    if (iClan)
			    {
				fClanRestrict = TRUE;
			   	rgfClan[iClan] = TRUE;
			    }
			    else
			    {
                        	send_to_char(
                            	"That's not a valid race, class, or clan.\n\r",
				   ch);
                            	return;
			    }
                        }
	        }
                else
                {
                	fRaceRestrict = TRUE;
                        rgfRace[iRace] = TRUE;
                }
            }
            else
            {
                fClassRestrict = TRUE;
                rgfClass[iClass] = TRUE;
            }
        }
        
    }
 
    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output = new_buf();
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;
        char const *class;
 
        /*
         * Check for match against restrictions.
         * Don't use trust as that exposes trusted mortals.
         */
        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;
 
        wch   = ( d->original != NULL ) ? d->original : d->character;

/*	if (!can_see(ch,wch))
	    continue;
*/
        if ( ( fImmortalOnly  && wch->level < LEVEL_IMMORTAL )
        || ( fClassRestrict && !rgfClass[wch->class] )
        || ( fRaceRestrict && !rgfRace[wch->race])
 	|| ( fClan && !is_clan(wch))
	|| ( fClanRestrict && !rgfClan[wch->clan]))
            continue;
 
        nMatch++;
 
        /*
         * Figure out what to print for class.
	 */
	class = class_table[wch->class].who_name;
	switch ( wch->level )
	{
	default: break;
            {
                case MAX_LEVEL - 0 : class = "IMP";     break;
                case MAX_LEVEL - 1 : class = "COD";     break;
                case MAX_LEVEL - 2 : class = "SUP";     break;
                case MAX_LEVEL - 3 : class = "DEM";     break;
                case MAX_LEVEL - 4 : class = "GOD";     break;
                case MAX_LEVEL - 5 : class = "BUI";     break;
                case MAX_LEVEL - 6 : class = "SOC";     break;
                case MAX_LEVEL - 7 : class = "AVA";     break;
                case MAX_LEVEL - 8 : class = "IMM";     break;
            }
	}


	/*
	 * Format it up.
	 *
	 * If statement insures the whotext to be the same size for two
	 *  digit levels and three digit levels
	 */


    if( wch->whotext != NULL )
    {
	if( colorstrlen( wch->whotext ) == 14 )
	{
	    sprintf( buf3, "[%s]", wch->whotext );
	}

	else if( colorstrlen( wch->whotext ) == 13 )
	{
	    sprintf( buf3, "[%s ]", wch->whotext );
	}

	else
	    sprintf( buf3, "[%s]", center(wch->whotext, 14) );
    }
    else
	sprintf( buf3, "[{g%s{x]", center(capitalize(pc_race_table[wch->race].name), 14) );
	
    
    if( wch->desc != NULL && wch->desc->editor != '\0')
	sprintf( buf4, "{r({gB{r){x ");
    else
	buf4[0] = '\0';


    if( wch->clanlevel > 6 )
    	sprintf( buf5, "{g<R>{x ");
    else
	buf5[0] = '\0';
    
    if( wch->clanlevel >= 10 )
    	sprintf( buf5, "{g<L>{x ");


    if( wch->pretitle == NULL )
	buf6[0] = '\0';
    else
	sprintf( buf6, "%s ", wch->pretitle );

   
    sprintf( buf, "%s %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n\r", buf3,
    clan_table[wch->clan].who_name,
    subclan_table[wch->subclan].who_name,
    buf5,
    wch->incog_level >= LEVEL_HERO ? "{W({cI{W){x " : "",
    wch->invis_level >= LEVEL_HERO ? "{W({gW{W){x " : "",
    buf4,
    IS_SET(wch->act, PLR_SLAIN) ? "({rANALLY VIOLATED{x) " : "",
    IS_SET(wch->act, PLR_VIOLENT) ? "({RVIOLENT{x) " : "",
    IS_SET(wch->act, PLR_KILLER) ? "({rKILLER{x) " : "",
    IS_SET(wch->act, PLR_THIEF)  ? "({yTHIEF{x) "  : "",
    IS_SET(wch->comm, COMM_AFK) ? "{W[{RAFK{W]{x " : "",
    IS_SET(wch->comm, COMM_IAW) ? "{W({yIAW{W){x "  : "",
    buf6,
    wch->name,
    IS_NPC(wch) ? "" : wch->pcdata->title );

    add_buf(output,buf);

    }

    sprintf( buf2, "\n\r{cPlayers found: {r%d{x\n\r", nMatch );
    add_buf(output,buf2);
    page_to_char( buf_string(output), ch );
    free_buf(output);
    return;

}
#endif

void do_count ( CHAR_DATA *ch, char *argument )
{
    int count;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    count = 0;

    for ( d = descriptor_list; d != NULL; d = d->next )
        if ( d->connected == CON_PLAYING && can_see( ch, d->character ) )
	    count++;

    max_on = UMAX(count,max_on);

    if (max_on == count)
        sprintf(buf,"There are %d characters on, the most so far today.\n\r",
	    count);
    else
	sprintf(buf,"There are %d characters on, the most on today was %d.\n\r",
	    count,max_on);

    send_to_char(buf,ch);
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    send_to_char( "{WYou are carrying:{x\n\r", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}



void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;

    send_to_char( "{WYou are using:{x\n\r", ch );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	    if(str_cmp(argument, "all"))
		continue;

	send_to_char( where_name[iWear], ch );
	if(obj == NULL)
	{
	    send_to_char( "{wEmpty{x\n\r", ch);
	    found = TRUE;
	    continue;
	}

	if ( can_see_obj( ch, obj ) )
	{
	    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
	    send_to_char( "\n\r", ch );
	}
	else
	{
	    send_to_char( "{Dsomething{x\n\r", ch );
	}
	found = TRUE;
    }

    if ( !found )
	send_to_char( "Nothing.\n\r", ch );

    return;
}



void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Compare what to what?\n\r", ch );
	return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1, ch ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }

    if (arg2[0] == '\0')
    {
	for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
	{
	    if (obj2->wear_loc != WEAR_NONE
	    &&  can_see_obj(ch,obj2)
	    &&  obj1->item_type == obj2->item_type
	    &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
		break;
	}

	if (obj2 == NULL)
	{
	    send_to_char("You aren't wearing anything comparable.\n\r",ch);
	    return;
	}
    } 

    else if ( (obj2 = get_obj_carry(ch,arg2,ch) ) == NULL )
    {
	send_to_char("You do not have that item.\n\r",ch);
	return;
    }

    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
	msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
	msg = "You can't compare $p and $P.";
    }
    else
    {
	switch ( obj1->item_type )
	{
	default:
	    msg = "You can't compare $p and $P.";
	    break;

	case ITEM_ARMOR:
	    value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
	    value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
	    break;

	case ITEM_WEAPON:
	    if (obj1->pIndexData->new_format)
		value1 = (1 + obj1->value[2]) * obj1->value[1];
	    else
	    	value1 = obj1->value[1] + obj1->value[2];

	    if (obj2->pIndexData->new_format)
		value2 = (1 + obj2->value[2]) * obj2->value[1];
	    else
	    	value2 = obj2->value[1] + obj2->value[2];
	    break;
	}
    }

    if ( msg == NULL )
    {
	     if ( value1 == value2 ) msg = "$p and $P look about the same.";
	else if ( value1  > value2 ) msg = "$p looks better than $P.";
	else                         msg = "$p looks worse than $P.";
    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    return;
}



void do_acredits( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    
    sprintf(buf, "%s is built with care by: %s.\n\r", 
    ch->in_room->area->name[0] != '!' 
    ? ch->in_room->area->name : "This area",
    ch->in_room->area->credits);
    send_to_char(buf, ch);
    return;
}

void do_credits(CHAR_DATA *ch, char *argument)
{
    do_function(ch, &do_help, "diku");
}

void do_where( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "{yPlayers near you:{x\n\r", ch );
	found = FALSE;
	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->connected == CON_PLAYING
	    && ( victim = d->character ) != NULL
	    &&   !IS_NPC(victim)
	    &&   victim->in_room != NULL
	    &&   !IS_SET(victim->in_room->room_flags,ROOM_NOWHERE)
 	    &&   (is_room_owner(ch,victim->in_room) 
	    ||    !room_is_private(victim->in_room))
	    &&   victim->in_room->area == ch->in_room->area
	    &&   can_see( ch, victim ) )
	    {
		found = TRUE;
		sprintf( buf, "{r%-28s {g%s\n\r{x",
		    victim->name, victim->in_room->name );
		send_to_char( buf, ch );
	    }
	}
	if ( !found )
	    send_to_char( "None\n\r", ch );
    }
    else
    {
	found = FALSE;
	for ( victim = char_list; victim != NULL; victim = victim->next )
	{
	    if ( victim->in_room != NULL
	    &&   victim->in_room->area == ch->in_room->area
	    &&   !IS_AFFECTED(victim, AFF_HIDE)
	    &&   !IS_AFFECTED(victim, AFF_SNEAK)
	    &&   can_see( ch, victim )
	    &&   is_name( arg, victim->name ) )
	    {
		found = TRUE;
		sprintf( buf, "%-28s %s\n\r",
		    PERS(victim, ch), victim->in_room->name );
		send_to_char( buf, ch );
		break;
	    }
	}
	if ( !found )
	    act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}




void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Consider killing whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They're not here.\n\r", ch );
	return;
    }

    if (is_safe(ch,victim))
    {
	send_to_char("Don't even think about it.\n\r",ch);
	return;
    }

    diff = victim->level - ch->level;

         if ( diff <= -10 ) msg = "You can kill $N naked and weaponless.";
    else if ( diff <=  -5 ) msg = "$N is no match for you.";
    else if ( diff <=  -2 ) msg = "$N looks like an easy kill.";
    else if ( diff <=   1 ) msg = "The perfect match!";
    else if ( diff <=   4 ) msg = "$N says 'Do you feel lucky, punk?'.";
    else if ( diff <=   9 ) msg = "$N laughs at you mercilessly.";
    else                    msg = "Death will thank you for your gift.";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
    {
	strcpy( buf, title );
    }

    free_string( ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Change your title to what?\n\r", ch );
	return;
    }
     
    if ( IS_SET(ch->act, PLR_NOTITLE ) )
		 {
		 send_to_char( "Ok.\n\r", ch);
		 return;
		 }

    if ( strlen(argument) > 60 )
	argument[60] = '\0';

    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Ok.\n\r", ch );
}

void do_description(CHAR_DATA *ch, char *argument)
{
    if(IS_NPC(ch))
    {
        mob_desc(ch, argument);
        return;
    }
    
    if(!str_cmp(argument, "view"))
    {
        send_to_char("Your description is:\n\r", ch);
        send_to_char(ch->description ? ch->description : "(None)\n\r", ch);
        return;
    }
    
    string_append(ch, &ch->description);
}

void mob_desc( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( argument[0] != '\0' )
    {
	buf[0] = '\0';
	smash_tilde( argument );

    	if (argument[0] == '-')
    	{
            int len;
            bool found = FALSE;
 
            if (ch->description == NULL || ch->description[0] == '\0')
            {
                send_to_char("No lines left to remove.\n\r",ch);
                return;
            }
	
  	    strcpy(buf,ch->description);
 
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
			free_string(ch->description);
			ch->description = str_dup(buf);
			send_to_char( "Your description is:\n\r", ch );
			send_to_char( ch->description ? ch->description : 
			    "(None).\n\r", ch );
                        return;
                    }
                }
            }
            buf[0] = '\0';
	    free_string(ch->description);
	    ch->description = str_dup(buf);
	    send_to_char("Description cleared.\n\r",ch);
	    return;
        }
	if ( argument[0] == '+' )
	{
	    if ( ch->description != NULL )
		strcat( buf, ch->description );
	    argument++;
	    while ( isspace(*argument) )
		argument++;
	}

        if ( strlen(buf) >= 1024)
	{
	    send_to_char( "Description too long.\n\r", ch );
	    return;
	}

	strcat( buf, argument );
	strcat( buf, "\n\r" );
	free_string( ch->description );
	ch->description = str_dup( buf );
    }

    send_to_char( "Your description is:\n\r", ch );
    send_to_char( ch->description ? ch->description : "(None).\n\r", ch );
    return;
}


void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf,
	"{D%s: {R%d{D/{M%d {Dhp {B%d{D/{C%d {Dmana {m%d{D/{r%d {Dmv {Y%d{D xp.{x'\n\r",
	ch->name,
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    send_to_char( buf, ch );

     sprintf( buf,
	"{D%s: {R%d{D/{M%d {Dhp {B%d{D/{C%d {Dmana {m%d{D/{r%d {Dmv {Y%d{D xp.{x'\n\r",
	ch->name,
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    act( buf, ch, NULL, NULL, TO_ROOM );

    return;
}



void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn;

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	int col;

	col    = 0;
	for ( sn = 0; sn < MAX_SKILL; sn++ )
	{
	    if ( skill_table[sn].name == NULL )
		break;
	    if ( ch->level < skill_table[sn].skill_level[ch->class] 
	      || ch->pcdata->learned[sn] < 1 /* skill is not known */)
		continue;

	    sprintf( buf, "{y%-18s %s%3d%%{x  ",
		skill_table[sn].name,
		ch->pcdata->learned[sn] < 16 ? "{r"
		: (ch->pcdata->learned[sn] < 30 ? "{R"
		: (ch->pcdata->learned[sn] < 50 ? "{y"
		: (ch->pcdata->learned[sn] < 76 ? "{Y"
		: (ch->pcdata->learned[sn] < 91 ? "{B"
		: (ch->pcdata->learned[sn] < 100 ? "{W"
		: "{G"))))),
		ch->pcdata->learned[sn] );
	    send_to_char( buf, ch );
	    if ( ++col % 3 == 0 )
		send_to_char( "\n\r", ch );
	}

	if ( col % 3 != 0 )
	    send_to_char( "\n\r", ch );

	sprintf( buf, "{cYou have {g%d {cpractice sessions left.{x\n\r",
	    ch->practice );
	send_to_char( buf, ch );
    }
    else
    {
	CHAR_DATA *mob;
	int adept;

	if ( !IS_AWAKE(ch) )
	{
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    return;
	}

	for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
	{
	    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
		break;
	}

	if ( mob == NULL )
	{
	    send_to_char( "You can't do that here.\n\r", ch );
	    return;
	}

	if ( ch->practice <= 0 )
	{
	    send_to_char( "You have no practice sessions left.\n\r", ch );
	    return;
	}

	if ( ( sn = find_spell( ch,argument ) ) < 0
	|| ( !IS_NPC(ch)
	&&   (ch->level < skill_table[sn].skill_level[ch->class] 
 	||    ch->pcdata->learned[sn] < 1 /* skill is not known */
	||    skill_table[sn].rating[ch->class] == 0)))
	{
	    send_to_char( "You can't practice that.\n\r", ch );
	    return;
	}

	adept = IS_NPC(ch) ? 100 : class_table[ch->class].skill_adept;

	if ( ch->pcdata->learned[sn] >= adept )
	{
	    sprintf( buf, "You are already learned at %s.\n\r",
		skill_table[sn].name );
	    send_to_char( buf, ch );
	}
	else
	{
	    ch->practice--;
	    ch->pcdata->learned[sn] += 
		int_app[get_curr_stat(ch,STAT_INT)].learn / 
	        skill_table[sn].rating[ch->class];
	    if ( ch->pcdata->learned[sn] < adept )
	    {
		act( "You practice $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act( "$n practices $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	    else
	    {
		ch->pcdata->learned[sn] = adept;
		act( "You are now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR );
		act( "$n is now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM );
	    }
	}
    }
    return;
}



/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
	wimpy = ch->max_hit / 5;
    else
	wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
	send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
	return;
    }

    if ( wimpy > ch->max_hit/2 )
    {
	send_to_char( "Such cowardice ill becomes you.\n\r", ch );
	return;
    }

    ch->wimpy	= wimpy;
    sprintf( buf, "Wimpy set to %d hit points.\n\r", wimpy );
    send_to_char( buf, ch );
    return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
	return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Syntax: password <old> <new>.\n\r", ch );
	return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }

    if ( strlen(arg2) < 5 )
    {
	send_to_char(
	    "New password must be at least five characters long.\n\r", ch );
	return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
	if ( *p == '~' )
	{
	    send_to_char(
		"New password not acceptable, try again.\n\r", ch );
	    return;
	}
    }

    free_string( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_bwho(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    BUFFER *output;

    buf[0] = '\0';
    output = new_buf();
    add_buf(output, "{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+ {RBOUNTY {r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+{r-{D+\n\r");

    for(d = descriptor_list; d != NULL; d = d->next)
    {
    	CHAR_DATA *wch;
    	
    	if(d->connected != CON_PLAYING)
    	    continue;
    	
    	wch = (d->character != NULL) ? d->character : d->original;
    	
    	if(!IS_NPC(wch) && can_see(ch, wch))
    	{
	   if(wch->pcdata->bounty>0)
		sprintf(buf, "{R%12s{x, {C%5d{x Steel\n\r\n\r",wch->name,wch->pcdata->bounty);
    	}
        add_buf(output, buf);
	buf[0]='\0';
    }
    page_to_char(buf_string(output), ch);

}

void do_ahours(CHAR_DATA *ch, char *argument)
{
	float hours, average;
	int level;
	
	if(IS_NPC(ch))
	{
	   send_to_char( "No average for you!" , ch);
	   return;
	}
	hours = (ch->played + (int) (current_time - ch->logon)) / 3600,
	level = ch->level;
	average = hours/level;
	printf_to_char(ch, "\n\r{YAverage Hours{x: {C%.4F{x\n\r", 
	average);
}

void do_history(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    extern char* ooc1;
    extern char* ooc2;
    extern char* ooc3;
    extern char* ooc4;
    extern char* ooc5;
    extern char* ooc6;
    extern char* ooc7;
    extern char* ooc8;
    extern char* ooc9;
    send_to_char("{g.{G=====[{WLast 8 IC/ICA{G]====={g.{x\n\r",ch);
    sprintf(buf, "  %s{x\n\r",ooc9);
	send_to_char(buf,ch);
    sprintf(buf, "  %s{x\n\r",ooc8);
	send_to_char(buf,ch);
    sprintf(buf, "  %s{x\n\r",ooc7);
	send_to_char(buf,ch);
    sprintf(buf, "  %s{x\n\r",ooc6);
	send_to_char(buf,ch);
    sprintf(buf, "  %s{x\n\r",ooc5);
	send_to_char(buf,ch);
    sprintf(buf, "  %s{x\n\r",ooc4);
	send_to_char(buf,ch);
    sprintf(buf, "  %s{x\n\r",ooc3);
	send_to_char(buf,ch);
    sprintf(buf, "  %s{x\n\r",ooc2);
	send_to_char(buf,ch);
    sprintf(buf, "  %s{x\n\r",ooc1);
	send_to_char(buf,ch);
    send_to_char("{g.{G=========================={g.{x\n\r",ch);
    return;
}
