/*
 *  Clan loader and clan functions 
 *  Justin Guerber, 2000
 */

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "olc.h"


/*
 * Function to announce to clan certain happenings.
 */

void clan_announce( int clan, char *argument, CHAR_DATA *victim, CHAR_DATA *ch )
{
    DESCRIPTOR_DATA *d;
    
    for( d = descriptor_list; d; d = d->next )
    {
    CHAR_DATA *wch;
    	if( d->connected != CON_PLAYING )
    		return;
   	
   	wch = ( d->original != NULL ) ? d->original : d->character;
   	
    	if( wch != ch &&
	    wch != victim &&
   	    wch->clan == clan )
    	{
    			send_to_char( argument, wch );
	}
    }
    return;
}    		



/*
 * Sets the subclan
 *   Not very efficient
 */
 

void set_subclan( CHAR_DATA *ch )
{
/*
    if( ch->clan == 2 )
    {
    	if( ch->class == 2 || ch->class == 3 || ch->class == 4 || ch->class == 5 )
    		ch->subclan = 4;
    	else if( ch->class == 1 || ch->class == 6 )
    		ch->subclan = 5;
    	else if( ch->class == 0 || ch->class == 7 )
    		ch->subclan = 6;
    }
*/
    if( ch->clan == 3 )
    {
	if( ch->alignment >= 333 )
		ch->subclan = 7;
	else if( ch->alignment >= -333 )
		ch->subclan = 8;
	else
		ch->subclan = 9;
    }
    
    if( ch->clan == 4 )
    {
    	if( ch->alignment >= 333 )
    		ch->subclan = 10;
    	else if( ch->alignment >=-333 )
    		ch->subclan = 11;
    	else
    		ch->subclan = 12;
    }
    
    return;
}
    

/*
 * Subfunction for petition
 *    Lists people petitioning your clan  
 */
                        
void do_petition_list(int clan, CHAR_DATA *ch)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found=FALSE;
           
    for (d = descriptor_list; d; d = d->next)
    {
        CHAR_DATA *victim;
        victim = d->original ? d->original : d->character;
        if (d->connected == CON_PLAYING)
		if( victim->petition == clan )
		{
                	sprintf( buf, "{c--> {x%s%s\n\r",
                	        victim->name,
                	        victim->pcdata->title );
                	send_to_char( buf, ch );
                	
                	found = TRUE;
		}
    }
         
    if( !found )
        send_to_char( "{cNone.{x\n\r", ch );
}




/*
 * petition
 * petition <clan>
 * petition retract
 */

void do_petition( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];

    int clan;
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );


    if(ch->clan == clan_lookup("loner") || !ch->clan
    || ch->clan == clan_lookup("renegade")
    || ch->clan == clan_lookup("heathen") ) {

    if( arg1[0] == '\0' )
    {
    	send_to_char( "Syntax: petition <clan>.\n\r", ch );
    	send_to_char( "Syntax: petition retract.\n\r", ch );
    	return;
    }

    if( !str_prefix(arg1, "retract") )
    {
         send_to_char("You withdraw your petition.\n\r", ch);
         
         sprintf( buf1, "%s withdraws their petition.\n\r", ch->name );
         clan_announce( ch->petition, buf1, ch, ch ); 
         
	 ch->petition = 0;
         return;
         
    }
    
    if ((clan = clan_lookup(arg1)) == 0)
    {
    	send_to_char("Clan does not exist.\n\r", ch);
    	return;
    }

    if( ch->petition )
    {
    	send_to_char( "You have already petitioned a clan. Try retracting that one first.\n\r", ch);
    	return;
    }


    sprintf( buf, "You have petitioned %s for membership.\n\r",
    	capitalize( clan_table[clan].name ) );
    send_to_char( buf, ch );

    ch->petition = clan;

    sprintf( buf1, "%s has petitioned for acceptance.\n\r", ch->name );
    clan_announce( ch->petition, buf1, ch, ch );

    return;    

    }

        
    if( ch->clanlevel <= 6 )
    {
    	send_to_char( "You can't petition while in a clan.\n\r", ch );
    	return;
    }

    if( ch->clanlevel >= 7 )
    {
    	if( arg1[0] == '\0' )
    	{
    		send_to_char( "{yThe following people have petitioned your clan:{x\n\r", ch );
		do_petition_list( ch->clan, ch );
    		return;
    	}
    }
	
    return;
}



/*
 * clancmd promote <player>
 * clancmd demote <player>
 * clancmd induct <player>
 * clancmd discharge <player>
 * clancmd subclan <player> <subclan>
 */

void do_clancmd( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char arg3[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    
    
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );


    if( !ch->clan || ch->clanlevel <= 6 )
    {
    	send_to_char( "Huh?\n\r", ch );
    	return;
    }
    
    if( arg1[0] == '\0'  )
    {
    	send_to_char( "Syntax:\n\r", ch );
    	send_to_char( "        clancmd induct <player> [subclan].\n\r", ch );
	send_to_char( "        clancmd promote <player>.\n\r", ch );
    	send_to_char( "        clancmd demote <player>.\n\r", ch );
    if( ch->clanlevel >= 10 )
    {
    	send_to_char( "        clancmd discharge <player>.\n\r", ch );
    	send_to_char( "        clancmd subclan <player> <subclan>.\n\r", ch );
    }
        return;
    }
    
    if( (victim = get_char_room( ch, arg2 )) == NULL )
    {
    	send_to_char( "They're not here.\n\r", ch );
    	return;
    }
    
    
    if( !str_prefix( arg1, "induct" ) )
    {
    	if( victim->petition != ch->clan )
    	{
    		send_to_char( "They haven't petitioned your clan. Type petition.\n\r", ch );
    		return;
    	}


    	victim->clan = ch->clan;
    	victim->clanlevel = 1;
    	
    	if( victim->clan == 1 )
    		victim->subclan = 1;

	if(arg3[0] == '\0')
    	    set_subclan( victim );
    	else
    	{
    	    int subclan = subclan_lookup( arg3 );
    	    if(subclan_table[subclan].clan == victim->clan)
    	    {
    	        victim->subclan = subclan;
    	    }
    	    else
    	    {
    	        send_to_char("That subclan is not part of your clan.\n\r", ch);
    	        send_to_char("Default subclan will be set.\n\r", ch);
    	        set_subclan(victim);
    	    }
    	}
    	victim->petition = 0;
    	

    	sprintf( buf1, "You have admitted %s into the clan.\n\r", victim->name );
    	send_to_char( buf1, ch );
    	
    	sprintf( buf1, "%s has admitted you into %s.\n\r",
    		ch->name,
    		capitalize( clan_table[ch->clan].name ) );
    	send_to_char( buf1, victim );
    	
    	sprintf( buf1, "%s has admitted %s into the clan.\n\r", ch->name, victim->name );
    	clan_announce( ch->clan, buf1, victim, ch );
    	
    	return;
    }
    
    if( victim->clan != ch->clan )
    {
    	send_to_char( "They're not in your clan.\n\r", ch );
    	return;
    }

    if( !str_prefix( arg1, "promote" ) )
    {
	if( (ch->level <= 101) && (victim->clanlevel >= 9) )
	{
		send_to_char( "Only gods can make leaders.\n\r", ch );
		return;
	}

    	if( victim->clanlevel >= ch->clanlevel )
    	{
    		send_to_char( "You can't promote someone who outranks you.\n\r", ch );
    		return;
    	}


    	victim->clanlevel++;

    	
    	sprintf( buf1, "You have promoted %s to %s!\n\r",
    		victim->name, clanrank_name(victim) );
    	send_to_char( buf1, ch );
    	
    	sprintf( buf1, "%s has promoted you to %s!\n\r",
    		ch->name, clanrank_name(victim) );
    	send_to_char( buf1, victim );
    	
    	
    	sprintf( buf1, "%s has promoted %s to %s!\n\r",
    		ch->name, victim->name, clanrank_name(victim) );
    	clan_announce( ch->clan, buf1, victim, ch );
    	
    	return;
    }
    else if( !str_prefix( arg1, "demote" ) )
    {
	if( victim->clanlevel == 1 )
	{
		send_to_char( "They're as low as they can go.\n\r", ch );
		return;
	}

	if( (ch->level <= 101) && (victim->clanlevel >= 10) )
	{
		send_to_char( "Only gods can demote a leader.\n\r", ch );
		return;
	}

    	if( victim->clanlevel >= ch->clanlevel )
        {
        	send_to_char( "You can't demote someone who outranks you.\n\r", ch );
		return;
	}


    	victim->clanlevel--;
    	
	
	sprintf( buf1, "You have demoted %s to %s!\n\r",
		victim->name, clanrank_name(victim) );
	send_to_char( buf1, ch );
	
	sprintf( buf1, "%s has demoted you to %s!\n\r",
		ch->name, clanrank_name(victim) );
	send_to_char( buf1, victim );    	


    	sprintf( buf1, "%s has demoted %s to %s!\n\r",
    		ch->name, victim->name, clanrank_name(victim) );
    	clan_announce( ch->clan, buf1, victim, ch );
    	
    	return;
    }


    if( ch->clanlevel >= 10 ) {
    if( !str_prefix( arg1, "discharge" ) )
    {
	if( victim->clanlevel >= ch->clanlevel )
    	{
        	send_to_char( "You can't discharge someone who outranks you.\n\r", ch );
        	return;
    	}


	sprintf( buf1, "You have discharged %s from the clan!\n\r",
		victim->name );
	send_to_char( buf1, ch );
	
	sprintf( buf1, "%s has discharged you from the clan!\n\r",
		ch->name );
	send_to_char( buf1, victim );


    	sprintf( buf1, "%s has discharged %s from the clan!\n\r",
    		ch->name, victim->name );
    	clan_announce( ch->clan, buf1, victim, ch );


    	victim->clanlevel = 0;
    	victim->clan = 0;
	victim->subclan = 0;
	
	return;
    }
    else if( !str_prefix( arg1, "subclan" ) )
    {
    	int subclan;
    	
    	if(victim->clanlevel >= ch->clanlevel)
    	{
    	    send_to_char("You really can't do that.\n\r", ch);
    	    return;
    	}
    
    	if( arg3[0] == '\0' )
    	{
    		send_to_char( "Please enter a subclan.\n\r", ch );
    		return;
    	}
    	
    	subclan = subclan_lookup( arg3 );
    	
    	if( subclan_table[subclan].clan != ch->clan )
    	{
    		send_to_char( "That subclan is not a part of your clan.\n\r", ch);
    		return;
    	}
    	
    	
    	sprintf( buf1, "You have changed %s's subclan to %s.\n\r",
    		victim->name, capitalize(subclan_table[subclan].name) );
    	send_to_char( buf1, ch );
    	
    	sprintf( buf1, "%s has changed your subclan to %s.\n\r",
    		ch->name, capitalize(subclan_table[subclan].name) );
    	send_to_char( buf1, victim);
    	
    	
    	sprintf( buf1, "%s has changed %s's subclan to %s.\n\r",
    		ch->name, victim->name, capitalize( subclan_table[subclan].name) );
    	clan_announce( ch->clan, buf1, victim, ch );
    	
    	victim->subclan = subclan;
    	
    	return;
    }
    }


    	send_to_char( "Syntax:\n\r", ch );
    	send_to_char( "        clancmd induct <player>.\n\r", ch );
	send_to_char( "        clancmd promote <player>.\n\r", ch );
    	send_to_char( "        clancmd demote <player>.\n\r", ch );
    if( ch->clanlevel >= 10 )
    {
    	send_to_char( "        clancmd discharge <player>.\n\r", ch );
    	send_to_char( "        clancmd subclan <player> <subclan>.\n\r", ch );
    }

}
