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
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "tables.h"

/*
 * Local functions.
 */
void	check_assist	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_violent	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_parry	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_phase     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_shield_block     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    dam_message 	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
                            int dt, bool immune ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim, 
			    int total_levels ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	make_corpse	args( ( CHAR_DATA *ch ) );
void	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, int hand ) );
void    mob_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	raw_kill	args( ( CHAR_DATA *victim ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	real_attack	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void    crowd_brawl	args( ( CHAR_DATA *ch ) );

/* Engage skill implementation */
void do_engage(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    char arg1[MAX_INPUT_LENGTH];
/*
    CHAR_DATA *wch;

    if(IS_SET(wch->act, PLR_ROLEPL))
    {
        send_to_char("Sorry, you can't attack a player in RP Mode!\n\r", ch);
        return;
    }

    if(IS_SET(wch->act, PLR_ROLEP2))
    {
        send_to_char("Sorry, you can't attack a player in RP Mode!\n\r", ch);
        return;
    }
*/
    one_argument(argument, arg1);

    if(get_skill(ch, gsn_engage) < 1)
    {
    	send_to_char("You hopelessly try to engage someone.\n\r", ch);
    	return;
    }
        
    if(ch->fighting == NULL)
    {
    	send_to_char("You're not even fighting!\n\r", ch);
    	return;
    }
    
    if((rch = get_char_room(ch, arg1)) == NULL)
    {
    	send_to_char("You attempt to fight someone that's not here.\n\r", ch);
    	return;
    }

    if(rch == ch)
    {
        send_to_char("You attempt to engage yourself in battle.\n\r", ch);
        return;
    }
    
    if(rch == ch->fighting)
    {
    	act("You're already fighting $n!", ch, NULL, rch, TO_CHAR);
    	return;
    }
    
    if(number_percent() < get_skill(ch, gsn_engage))
    {
    	act("You turn your attention to $N!", ch, NULL, rch, TO_CHAR);
    	act("$N turns $S attention to you!", rch, NULL, ch, TO_CHAR);
    	ch->fighting = rch;
    	if(rch->fighting == NULL)
    	    rch->fighting = ch;
    	check_improve(ch, gsn_engage, TRUE, 5);
    }
    else
    {
    	act("You try to refocus your attention but fail.",ch, NULL,NULL,TO_CHAR);
    	check_improve(ch, gsn_engage, FALSE, 5);
    }
    WAIT_STATE(ch, skill_table[gsn_engage].beats);
}


/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;

    for ( ch = char_list; ch != NULL; ch = ch_next )
    {
	ch_next	= ch->next;

	if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
	    continue;

	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
         {
           crowd_brawl(ch);
	   multi_hit( ch, victim, TYPE_UNDEFINED );        
        }
 	else
	    stop_fighting( ch, FALSE );

	if ( ( victim = ch->fighting ) == NULL )
	    continue;

	/*
	 * Fun for the whole family!
	 */
	check_assist(ch,victim);

	if ( IS_NPC( ch ) )
	{
	    if ( HAS_TRIGGER( ch, TRIG_FIGHT ) )
		mp_percent_trigger( ch, victim, NULL, NULL, TRIG_FIGHT );
	    if ( HAS_TRIGGER( ch, TRIG_HPCNT ) )
		mp_hprct_trigger( ch, victim );
	}
    }

    return;
}

void crowd_brawl(CHAR_DATA *ch)
{
    CHAR_DATA *rch, *rch_next, *vch, *vch_next;
    CHAR_DATA *is_fighting[45];
    SHOP_DATA *pshop;
    int chance;
    int counter;
    int to_fight;

 for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;

       chance=number_range(1,300);

	if ((rch->fighting == NULL)
	 &&((!IS_IMMORTAL(rch) && !IS_NPC(rch))
	 ||(IS_NPC(rch)
	 &&!(IS_SET(rch->act,ACT_TRAIN)
	 ||  IS_SET(rch->act,ACT_PRACTICE)
	 ||  IS_SET(rch->act,ACT_IS_HEALER)
	 ||  IS_SET(rch->act,ACT_IS_CHANGER)
	 || ((pshop = rch->pIndexData->pShop) != NULL))))
	 &&IS_AWAKE(rch)
	 &&(chance <= 2))
	{
	  counter = 0;
	  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	  {
	    vch_next = vch->next_in_room;
	    if ((vch->fighting != NULL) && (counter <= 44))
	    {
 	is_fighting[counter] = vch;
	      ++counter;
	      /* if it's an NPC, thrice as likely as a PC to be attacked */
	      /* max of 45 fighting PC/NPCs in array, can't exceed */
	      if ((IS_NPC(vch)) && (counter <= 44))
	      {
	        is_fighting[counter] = vch;
	        ++counter;
	      }
	      if ((IS_NPC(vch)) && (counter <= 44))
	      {
	        is_fighting[counter] = vch;
	        ++counter;

	      }
  	    }
	  }
	  /* random number of mob in array to fight */
	  to_fight = number_range(1,counter);
	  /* (to_fight -1) because arrays start at 0 not at 1 */
	  to_fight -= 1;
  /* checks needed for PCs only */
	  if (!IS_NPC(rch) && !IS_NPC(is_fighting[to_fight]))
	  {
	    if (is_same_clan(rch,is_fighting[to_fight]))
	      continue;

	    if (((UMAX(rch->level,is_fighting[to_fight]->level) -
		  UMIN(rch->level,is_fighting[to_fight]->level)) > 7)
	       ||(is_fighting[to_fight]->level < 15))
	      continue;
	  }
	  else if (IS_NPC(rch))
	  {

	  if (IS_IMMORTAL(is_fighting[to_fight]))
	    continue;

	  if ((rch->position <= POS_RESTING) && (chance > 1))
	    continue;

	  if (!can_see(rch,is_fighting[to_fight]))
	    continue;

	  if (is_same_group(rch,is_fighting[to_fight]))
	    continue;

	  if (IS_AFFECTED(rch,AFF_CHARM)
	   &&((rch->master == is_fighting[to_fight])
	   ||(is_fighting[to_fight]->master == rch)
	   ||(is_fighting[to_fight]->master == rch->master)))
	    continue;

	  if (IS_SET(rch->act,ACT_PET)
	   &&((rch->master == is_fighting[to_fight])
	   ||(is_fighting[to_fight]->master == rch)
	   ||(is_fighting[to_fight]->master == rch->master)))
	    continue;

	  if (is_fighting[to_fight] != NULL)
	  {
	    rch->fighting = is_fighting[to_fight];
	    rch->position = POS_FIGHTING;
	    act("You find yourself caught up in the brawl!",rch,NULL,NULL,TO_CHAR);
	    act("$n finds $mself involved in the brawl.",rch,NULL,NULL,TO_ROOM);
	    return;
	  }
	    bug("Crowd_brawl - person to fight is NULL.", 0);
	return;
	}
    }
  }
}
/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
    CHAR_DATA *rch, *rch_next;

    for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
	rch_next = rch->next_in_room;
	
	if (IS_AWAKE(rch) && rch->fighting == NULL)
	{

	    /* quick check for ASSIST_PLAYER */
	    if (!IS_NPC(ch) && IS_NPC(rch) 
	    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
	    &&  rch->level + 6 > victim->level)
	    {
		do_function(rch, &do_emote, "screams and attacks!");
		multi_hit(rch,victim,TYPE_UNDEFINED);
		continue;
	    }

	    /* PCs next */
	    if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM))
	    {
		if ( ( (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
		||     IS_AFFECTED(rch,AFF_CHARM)) 
		&&   is_same_group(ch,rch) 
		&&   !is_safe(rch, victim))
		    multi_hit (rch,victim,TYPE_UNDEFINED);
		
		continue;
	    }
  	
	    /* now check the NPC cases */
	    
 	    if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
	
	    {
		if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

		||   (IS_NPC(rch) && rch->group && rch->group == ch->group)

		||   (IS_NPC(rch) && rch->race == ch->race 
		   && IS_SET(rch->off_flags,ASSIST_RACE))

		||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
		   &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
		     ||  (IS_EVIL(rch)    && IS_EVIL(ch))
		     ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch)))) 

		||   (rch->pIndexData == ch->pIndexData 
		   && IS_SET(rch->off_flags,ASSIST_VNUM)))

	   	{
		    CHAR_DATA *vch;
		    CHAR_DATA *target;
		    int number;

		    if (number_bits(1) == 0)
			continue;
		
		    target = NULL;
		    number = 0;
		    for (vch = ch->in_room->people; vch; vch = vch->next)
		    {
			if (can_see(rch,vch)
			&&  is_same_group(vch,victim)
			&&  number_range(0,number) == 0)
			{
			    target = vch;
			    number++;
			}
		    }

		    if (target != NULL)
		    {
			do_function(rch, &do_emote, "screams and attacks!");
			multi_hit(rch,target,TYPE_UNDEFINED);
		    }
		}	
	    }
	}
    }
}

void real_attack(CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    OBJ_DATA *wieldR, *wieldL;
    wieldR = get_eq_char(ch, WEAR_RIGHT);
    wieldL = get_eq_char(ch, WEAR_LEFT);
    
    // No weapon (hand to hand)
    if((wieldR == NULL || (wieldR != NULL && wieldR->item_type != ITEM_WEAPON))
    && (wieldL == NULL || (wieldL != NULL && wieldL->item_type != ITEM_WEAPON)))
    {
        if(wieldR == NULL)
            one_hit( ch, victim, dt, WEAR_RIGHT );
        else if(wieldL == NULL)
            one_hit( ch, victim, dt, WEAR_LEFT );
        else
            send_to_char("You have nothing to attack with!\n\r", ch);
        return;
    }
    
    if(wieldR != NULL && wieldR->item_type == ITEM_WEAPON)
        one_hit( ch, victim, dt, WEAR_RIGHT);
     
    if(wieldL != NULL && wieldL->item_type == ITEM_WEAPON)
        one_hit( ch, victim, dt, WEAR_LEFT);
}

/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int     chance;

    /* decrement the wait */
    if (ch->desc == NULL)
	ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);

    if (ch->desc == NULL)
	ch->daze = UMAX(0,ch->daze - PULSE_VIOLENCE); 


    /* no attacks for stunnies -- just a check */
    if (ch->position < POS_RESTING)
	return;

    if (IS_NPC(ch))
    {
	mob_hit(ch,victim,dt);
	return;
    }

    real_attack(ch, victim, dt);

    if (ch->fighting != victim)
	return;

    if (IS_AFFECTED(ch,AFF_HASTE))
	real_attack(ch,victim,dt);

    if ( ch->fighting != victim || dt == gsn_backstab )
	return;

    chance = get_skill(ch,gsn_second_attack)/2;

    if(IS_AFFECTED2(ch,AFF_BLUR))
	chance =+ 75;	
	if (IS_AFFECTED(ch,AFF_SLOW))
	chance /= 2;

    if ( number_percent( ) < chance )
    {
	real_attack( ch, victim, dt );
	check_improve(ch,gsn_second_attack,TRUE,5);
	if ( ch->fighting != victim )
	    return;
    }

    chance = get_skill(ch,gsn_third_attack)/4;

    if (IS_AFFECTED(ch,AFF_SLOW))
	chance = 0;
    
	if(IS_AFFECTED2(ch,AFF_BLUR))
	chance =+ 65;	

    if ( number_percent( ) < chance )
    {
	real_attack( ch, victim, dt );
	check_improve(ch,gsn_third_attack,TRUE,6);
	if ( ch->fighting != victim )
	    return;
    }

    chance = get_skill(ch,gsn_fourth_attack)/6;

    if (IS_AFFECTED(ch,AFF_SLOW))
        chance = 0;

	if(IS_AFFECTED2(ch,AFF_BLUR))
	chance =+ 50;	

    if ( number_percent( ) < chance )
    {
        real_attack( ch, victim, dt );
        check_improve(ch,gsn_fourth_attack,TRUE,7);
        if ( ch->fighting != victim )
            return;
    }

    chance = get_skill(ch,gsn_fifth_attack)/6;

    if (IS_AFFECTED(ch,AFF_SLOW))
        chance = 0;

        if(IS_AFFECTED2(ch,AFF_BLUR))
        chance =+ 65;

    if ( number_percent( ) < chance )
    {
        real_attack( ch, victim, dt );
        check_improve(ch,gsn_fifth_attack,TRUE,6);
        if ( ch->fighting != victim )
            return;
    }

    chance = get_skill(ch,gsn_sixth_attack)/4;

    if (IS_AFFECTED(ch,AFF_SLOW))
        chance = 0;

        if(IS_AFFECTED2(ch,AFF_BLUR))
        chance =+ 65;

    if ( number_percent( ) < chance )
    {
        real_attack( ch, victim, dt );
        check_improve(ch,gsn_sixth_attack,TRUE,6);
        if ( ch->fighting != victim )
            return;
    }


    return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
    int chance,number;
    CHAR_DATA *vch, *vch_next;

    one_hit(ch,victim,dt,WEAR_RIGHT);

    if (ch->fighting != victim)
	return;

    /* Area attack -- BALLS nasty! */
 
    if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
    {
	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	    vch_next = vch->next;
	    if ((vch != victim && vch->fighting == ch))
		one_hit(ch,vch,dt,WEAR_RIGHT);
	}
    }

    if (IS_AFFECTED(ch,AFF_HASTE) 
    ||  (IS_SET(ch->off_flags,OFF_FAST) && !IS_AFFECTED(ch,AFF_SLOW)))
	one_hit(ch,victim,dt,WEAR_RIGHT);

    if (ch->fighting != victim || dt == gsn_backstab)
	return;

    chance = get_skill(ch,gsn_second_attack)/2;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
	chance /= 2;

    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt,WEAR_RIGHT);
	if (ch->fighting != victim)
	    return;
    }

    chance = get_skill(ch,gsn_third_attack)/4;

    if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
	chance = 0;

    if (number_percent() < chance)
    {
	one_hit(ch,victim,dt,WEAR_RIGHT);
	if (ch->fighting != victim)
	    return;
    } 

    /* oh boy!  Fun stuff! */

    if (ch->wait > 0)
	return;

    number = number_range(0,2);

    if (number == 1 && IS_SET(ch->act,ACT_MAGE))
    {
	/*  { mob_cast_mage(ch,victim); return; } */ ;
    }

    if (number == 2 && IS_SET(ch->act,ACT_CLERIC))
    {	
	/* { mob_cast_cleric(ch,victim); return; } */ ;
    }

    /* now for the skills */

    number = number_range(0,8);

    switch(number) 
    {
    case (0) :
	if (IS_SET(ch->off_flags,OFF_BASH))
	    do_function(ch, &do_bash, "");
	break;

    case (1) :
	if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
	    do_function(ch, &do_berserk, "");
	break;


    case (2) :
	if (IS_SET(ch->off_flags,OFF_DISARM) 
	|| (get_weapon_sn(ch, WEAR_RIGHT) != gsn_hand_to_hand 
	&& (IS_SET(ch->act,ACT_WARRIOR)
   	||  IS_SET(ch->act,ACT_THIEF))))
	    do_function(ch, &do_disarm, "");
	break;

    case (3) :
	if (IS_SET(ch->off_flags,OFF_KICK))
	    do_function(ch, &do_kick, "");
	break;

    case (4) :
	if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
	    do_function(ch, &do_dirt, "");
	break;

    case (5) :
	if (IS_SET(ch->off_flags,OFF_TAIL))
	{
	    /* do_function(ch, &do_tail, "") */ ;
	}
	break; 

    case (6) :
	if (IS_SET(ch->off_flags,OFF_TRIP))
	    do_function(ch, &do_trip, "");
	break;

    case (7) :
	if (IS_SET(ch->off_flags,OFF_CRUSH))
	{
	    /* do_function(ch, &do_crush, "") */ ;
	}
	break;
    case (8) :
	if (IS_SET(ch->off_flags,OFF_BACKSTAB))
	{
	    do_function(ch, &do_backstab, "");
	}
    }
}
	

/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt, int hand )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn,skill;
    int dam_type;
    int chance;
    bool result;

    sn = -1;

    /* just in case */
    if (victim == ch || ch == NULL || victim == NULL)
	return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return;

    /*
     * Figure out the type of damage message.
     */
    wield = get_eq_char( ch, hand );

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield != NULL && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
	else 
	    dt += ch->dam_type;
    }

    if (dt < TYPE_HIT)
    	if (wield != NULL)
    	    dam_type = attack_table[wield->value[3]].damage;
    	else
    	    dam_type = attack_table[ch->dam_type].damage;
    else
    	dam_type = attack_table[dt - TYPE_HIT].damage;

    if (dam_type == -1)
	dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_weapon_sn(ch, hand);
    skill = 20 + get_weapon_skill(ch,sn);
    if(!IS_NPC(ch))
    {
        if(hand == WEAR_RIGHT)
        {
            skill = skill * get_skill(ch, gsn_right_hand) / 100;
            check_improve(ch, gsn_right_hand, 5, TRUE);
        }
        else
        {
            skill = skill * get_skill(ch, gsn_left_hand) / 100;
            check_improve(ch, gsn_left_hand, 5, TRUE);
        }
    }

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = 20;
	thac0_32 = -4;   /* as good as a thief */ 
	if (IS_SET(ch->act,ACT_WARRIOR))
	    thac0_32 = -10;
	else if (IS_SET(ch->act,ACT_THIEF))
	    thac0_32 = -4;
	else if (IS_SET(ch->act,ACT_CLERIC))
	    thac0_32 = 2;
	else if (IS_SET(ch->act,ACT_MAGE))
	    thac0_32 = 6;
    }
    else
    {
	thac0_00 = class_table[ch->class].thac0_00;
	thac0_32 = class_table[ch->class].thac0_32;
    }
    thac0  = interpolate( ch->level, thac0_00, thac0_32 );

    if (thac0 < 0)
        thac0 = thac0/2;

    if (thac0 < -5)
        thac0 = -5 + (thac0 + 5) / 2;

    thac0 -= GET_HITROLL(ch) * skill/100;
    thac0 += 5 * (100 - skill) / 100;

    if (dt == gsn_backstab)
	thac0 -= 10 * (100 - get_skill(ch,gsn_backstab));

	if (dt == gsn_circle)
        thac0 -= 10 * (100 - get_skill(ch,gsn_circle));

    switch(dam_type)
    {
	case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;	break;
	case(DAM_BASH):	 victim_ac = GET_AC(victim,AC_BASH)/10;		break;
	case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
	default:	 victim_ac = GET_AC(victim,AC_EXOTIC)/10;	break;
    }; 
	
    if (victim_ac < -15)
	victim_ac = (victim_ac + 15) / 5 - 15;
     
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    if ( victim->position < POS_FIGHTING)
	victim_ac += 4;
 
    if (victim->position < POS_RESTING)
	victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	damage( ch, victim, 0, dt, dam_type, TRUE );
	tail_chain( );
	return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if ( IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
	if (!ch->pIndexData->new_format)
	{
	    dam = number_range( ch->level / 2, ch->level * 3 / 2 );
	    if ( wield != NULL )
	    	dam += dam / 2;
	}
	else
	    dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);
	
    else
    {
	if (sn != -1)
	    check_improve(ch,sn,TRUE,5);
	if ( wield != NULL )
	{
	    OBJ_DATA *wr, *wl;
	    if (wield->pIndexData->new_format)
		dam = dice(wield->value[1],wield->value[2]) * skill/100;
	    else
	    	dam = number_range( wield->value[1] * skill/100, 
				wield->value[2] * skill/100);

            wr = get_eq_char(ch,WEAR_RIGHT);
            wl = get_eq_char(ch,WEAR_LEFT);
	    if ((wr == NULL || (wr != NULL && wr->item_type != ITEM_WEAR_SHIELD))
	    && (wl == NULL || (wl != NULL && wl->item_type != ITEM_WEAR_SHIELD)))
	      /* no shield = more */
		dam = dam * 11/10;

            /* check ambidexterity */
            if(!IS_NPC(ch) && wl != NULL && wr != NULL
            && wr->item_type == ITEM_WEAPON && wl->item_type == ITEM_WEAPON)
            {
                int ambi_skill = get_skill(ch, gsn_ambidexterity);
                if(ambi_skill == 0)
                    dam = dam / 2;
                else if(ambi_skill < 25)
                    dam = dam * ambi_skill / 30;
                else if(ambi_skill < 50)
                    dam = dam * ambi_skill / 55;
                else if(ambi_skill > 74)
                    dam = dam * ambi_skill / (ambi_skill - 5);
                
                if(ambi_skill > 0)
                    check_improve(ch, gsn_ambidexterity, TRUE, 2);
            }
            
	    /* sharpness! */
	    if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
	    {
		int percent;

		if ((percent = number_percent()) <= (skill / 8))
		    dam = 2 * dam + (dam * 2 * percent / 100);
	    }
	}
	else
	    dam = number_range( 1 + 4 * skill/100, 2 * ch->level/3 * skill/100);
    }

    /*
     * Bonuses.
     */
    if ( get_skill(ch,gsn_enhanced_damage) > 0 )
    {
        diceroll = number_percent();
        if (diceroll <= get_skill(ch,gsn_enhanced_damage))
        {
            check_improve(ch,gsn_enhanced_damage,TRUE,6);
            dam += 2 * ( dam * diceroll/300);
        }
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;
     else if (victim->position < POS_FIGHTING)
	dam = dam * 3 / 2;

    if ( dt == gsn_backstab && wield != NULL) 
    {
    	if ( wield->value[0] != 2 )
	    {
	    dam *= 2 + (ch->level / 10);
		dam = dam/2;
	    } 
	else 
{
	    dam *= 2 + (ch->level / 8);
		dam = dam/2;
}	
	if ( dt == gsn_circle && wield != NULL)
	{
        if ( wield->value[0] != 2 )
           dam *= 2+ (ch->level / 10);
        else
           dam *=2 + (ch->level / 8);
		dam=dam/2;
        }
    }

    dam += GET_DAMROLL(ch) * UMIN(100,skill) /100;
    
    /* Finally, remort bonuses */
    if(!IS_NPC(ch))
        dam = dam * (ch->pcdata->remorts + 10) / 10;

    if ( dam <= 0 )
	dam = 1;

    if(IS_NPC(ch))
    	chance = !IS_SET(ch->act2, ACT_NO_COUNTER) ? number_range(0,5) + ch->level/2 : 0;
    else if(!IS_NPC(ch) && IS_NPC(victim) && IS_SET(victim->act2, ACT_NO_COUNTER))
    	chance = 0;
    else
    {
    	chance = get_skill(victim, gsn_counter)/4;
    	if(chance > 0)
    	  chance = chance + victim->level - ch->level;
    }

    chance /= 2; /* It is still happening too much */
    
    if(number_percent() < chance)
    {
    	act("{Y$N turns $n's attack and counters with $S own!{x",
    	 ch, NULL, victim, TO_NOTVICT);
    	act("{Y$N counters your attack!{x", ch, NULL, victim, TO_CHAR);
    	act("{YYou counter $n's attack!{x", ch, NULL, victim, TO_VICT);
    	real_attack(victim, ch, TYPE_UNDEFINED);
    	if(!IS_NPC(victim))
    	    check_improve(victim, gsn_counter, TRUE, 7);
    	return;
    }
    
    result = damage( ch, victim, dam, dt, dam_type, TRUE );

    if (IS_AFFECTED2(ch, AFF_FLAMESHIELD) && result)
    {
    	int dam;
    	dam = number_range(1, ch->level/4+1);
    	act("$n is burned by $N's flameshield.",victim,NULL,ch,TO_ROOM);
    	act("You are burned by $N's flameshield.",victim,NULL,ch,TO_CHAR);
	damage(ch,victim,dam,0,DAM_FIRE,FALSE);
    }    
    
    if(IS_AFFECTED2(victim, AFF_FLAMESHIELD))
    {
    	int dam;
    	dam = number_range(1, victim->level/4+1);
    	act("$n is burned by $N's flameshield.",ch,NULL,victim,TO_ROOM);
    	act("You are burned by $N's flameshield.",ch,NULL,victim,TO_CHAR);
    	fire_effect((void *) ch,victim->level/4,dam,TARGET_CHAR);
    	damage(victim,ch,dam,0,DAM_FIRE,FALSE);
    }
    if (IS_AFFECTED2(ch, AFF_LIGHTNING) && result)
    {
    	int dam;
    	dam = number_range(1, ch->level/4+1);
    	act("$n is shocked by $N's lightning.",victim,NULL,ch,TO_ROOM);
    	act("You are shocked by $N's lightning.",victim,NULL,ch,TO_CHAR);
	damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE);
    }    
    
    if(IS_AFFECTED2(victim, AFF_LIGHTNING))
    {
    	int dam;
    	dam = number_range(1, victim->level/4+1);
    	act("$n is shocked by $N's lightning.",ch,NULL,victim,TO_ROOM);
    	act("You are shocked by $N's lightning.",ch,NULL,victim,TO_CHAR);
    	fire_effect((void *) ch,victim->level/4,dam,TARGET_CHAR);
    	damage(victim,ch,dam,0,DAM_LIGHTNING,FALSE);
    }
    
    /* but do we have a funky weapon? */
    if (result && wield != NULL)
    { 
	int dam;

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_POISON))
	{
	    int level;
	    AFFECT_DATA *poison, af;

	    if ((poison = affect_find(wield->affected,gsn_poison)) == NULL)
		level = wield->level;
	    else
		level = poison->level;
	
	    if (!saves_spell(level / 2,victim,DAM_POISON)) 
	    {
		send_to_char("You feel poison coursing through your veins.\n\r",
		    victim);
		act("$n is poisoned by the venom on $p.",
		    victim,wield,NULL,TO_ROOM);

    		af.where     = TO_AFFECTS;
    		af.type      = gsn_poison;
    		af.level     = level * 3/4;
    		af.duration  = level / 2;
    		af.location  = APPLY_STR;
    		af.modifier  = -1;
    		af.bitvector = AFF_POISON;
    		affect_join( victim, &af );
	    }

	    /* weaken the poison if it's temporary */
	    if (poison != NULL)
	    {
	    	poison->level = UMAX(0,poison->level - 2);
	    	poison->duration = UMAX(0,poison->duration - 1);
	
	    	if (poison->level == 0 || poison->duration == 0)
		    act("The poison on $p has worn off.",ch,wield,NULL,TO_CHAR);
	    }
 	}


    	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
	{
	    dam = number_range(1, wield->level / 5 + 1);
	    act("$p draws life from $n.",victim,wield,NULL,TO_ROOM);
	    act("You feel $p drawing your life away.",
		victim,wield,NULL,TO_CHAR);
	    damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
            ch->hit=UMIN(ch->hit+ch->level,ch->max_hit);
	}
    	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VORPAL))
	{
	    dam = number_range(1, wield->level / 5 + 1);
	    act("$p draws energy from $n.",victim,wield,NULL,TO_ROOM);
	    act("You feel $p drawing your magical essesnce away.",
		victim,wield,NULL,TO_CHAR);
	    damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
	    victim->mana=UMAX(0,victim->mana-victim->level);
            ch->mana=UMIN(ch->mana+ch->level,ch->max_mana);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FLAMING))
	{
	    dam = number_range(1,wield->level / 4 + 1);
	    act("$n is burned by $p.",victim,wield,NULL,TO_ROOM);
	    act("$p sears your flesh.",victim,wield,NULL,TO_CHAR);
	    fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_FIRE,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FROST))
	{
	    dam = number_range(1,wield->level / 6 + 2);
	    act("$p freezes $n.",victim,wield,NULL,TO_ROOM);
	    act("The cold touch of $p freezes your flesh.",
		victim,wield,NULL,TO_CHAR);
	    cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_COLD,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
	{
	    dam = number_range(1,wield->level/5 + 2);
	    act("$n is struck by lightning from $p.",victim,wield,NULL,TO_ROOM);
	    act("You are shocked by $p.",victim,wield,NULL,TO_CHAR);
	    shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE);
	}

	if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_ACIDIC))
	{
	    dam = number_range(1,wield->level / 6 + 5);
	    act("$p eats at $n's flesh.",victim,wield,NULL,TO_ROOM);
	    act("The acid on $p eats violently at your skin!",
		victim,wield,NULL,TO_CHAR);
	    acid_effect(victim,wield->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_ACID,FALSE);
	}

    }
    tail_chain( );
    return;
}


/*
 * Inflict damage from a hit.
 */
bool damage(CHAR_DATA *ch,CHAR_DATA *victim,int dam,int dt,int dam_type,
	    bool show) 
{
    OBJ_DATA *corpse;
    bool immune;
    bool arena = FALSE;
    int chance;

    if ( victim->position == POS_DEAD )
	return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > 1200 && dt >= TYPE_HIT)
    {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf, "%s: Damage: %d: more than 1200 points!", ch->name, dam);
	/*bug( "Damage: %d: more than 1200 points!", dam );*/
	bug(buf, 0);
	dam = 1200;
/*	if (!IS_IMMORTAL(ch))
	{
	    OBJ_DATA *obj;
	    obj = get_eq_char( ch, WEAR_RIGHT );
	    send_to_char("You really shouldn't cheat.\n\r",ch);
	    if (obj != NULL)
	    	extract_obj(obj);
	}
*/
    }

    
    /* damage reduction */
    if ( dam > 35)
	dam = (dam - 35)/2 + 35;
    if ( dam > 80)
	dam = (dam - 80)/2 + 80; 



   
    if ( victim != ch )
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if ( is_safe( ch, victim ) )
	    return FALSE;
	check_killer( ch, victim );
	check_violent( ch, victim );

	if ( victim->position > POS_STUNNED )
	{
	    if ( victim->fighting == NULL )
	    {
		set_fighting( victim, ch );
		if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_KILL ) )
		    mp_percent_trigger( victim, ch, NULL, NULL, TRIG_KILL );
	    }
	    if (victim->timer <= 4)
	    	victim->position = POS_FIGHTING;
	}

	if ( victim->position > POS_STUNNED )
	{
	    if ( ch->fighting == NULL )
		set_fighting( ch, victim );
	}

	/*
	 * More charm stuff.
	 */
	if ( victim->master == ch )
	    stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */
    if ( IS_AFFECTED(ch, AFF_HIDE))
	{
	   affect_strip( ch,gsn_hide);
	   REMOVE_BIT(ch->affected_by, AFF_HIDE);
	   act( "$n is no longer hidden.", ch, NULL, NULL, TO_ROOM );
	}
    if ( IS_AFFECTED(ch, AFF_SNEAK))
	{
	   affect_strip( ch,gsn_sneak);
	   REMOVE_BIT(ch->affected_by, AFF_SNEAK);
	   act( "$n is no longer sneaking.", ch, NULL, NULL, TO_ROOM );
	}
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
	affect_strip( ch, gsn_invis );
	affect_strip( ch, gsn_mass_invis );
	REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
	act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
    }

    /*
     * Damage modifiers.
     */

    if ( dam > 1 && !IS_NPC(victim) 
    &&   victim->pcdata->condition[COND_DRUNK]  > 10 )
	dam = 9 * dam / 10;

    if ( dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY) )
	dam /= 2;

    if ( dam > 1 && ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) )
    ||		     (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )))
	dam -= dam / 4;

    immune = FALSE;


    /*
     * Check for parry, and dodge.
     */
    if ( dt >= TYPE_HIT && ch != victim)
    {
        if ( check_parry( ch, victim ) )
	    return FALSE;
	if ( check_dodge( ch, victim ) )
	    return FALSE;
	if ( check_shield_block(ch,victim))
	    return FALSE;
	if ( check_shield_block(ch,victim))
	    return FALSE;
        if ( check_phase( ch, victim))
            return FALSE;      
   
   }

    switch(check_immune(victim,dam_type))
    {
	case(IS_IMMUNE):
	    immune = TRUE;
	    dam = 0;
	    break;
	case(IS_RESISTANT):	
	    dam -= dam/3;
	    break;
	case(IS_VULNERABLE):
	    dam += dam/2;
	    break;
    }

    if (show)
    {
    	if(dam > 0)
    	{
    	    chance = get_skill(ch, gsn_critical_strike)/3;
    	    if (IS_AFFECTED(ch, AFF_SLOW))
    	    	chance /= 2;
    	    if (IS_AFFECTED(ch, AFF_BLIND))
    	    	chance /= 2;
    	    if (IS_AFFECTED(ch, AFF_HASTE))
    	    	chance *= 2;
    	    if (IS_AFFECTED(victim, AFF_SLOW))
    	    	chance *= 2;
    	
    	    if(chance + number_range(5,10) > number_percent())
    	    {
    	    	act("{RCRITICAL!{x", ch, NULL, NULL, TO_ALL);
    	    	dam *= 2;
    	    	check_improve(ch, gsn_critical_strike, TRUE, 4);
    	    }
    	}
        if(dam>=2300)
	   dam=1900;
    	dam_message( ch, victim, dam, dt, immune );
    }

    if (dam == 0)
	return FALSE;

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_IMMORTAL
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( "$n is mortally wounded, and will die soon, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char( 
	    "You are mortally wounded, and will die soon, if not aided.\n\r",
	    victim );
	break;

    case POS_INCAP:
	act( "$n is incapacitated and will slowly die, if not aided.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char(
	    "You are incapacitated and will slowly die, if not aided.\n\r",
	    victim );
	break;

    case POS_STUNNED:
	act( "$n is stunned, but will probably recover.",
	    victim, NULL, NULL, TO_ROOM );
	send_to_char("You are stunned, but will probably recover.\n\r",
	    victim );
	break;

    case POS_DEAD:
	act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
	send_to_char( "You have been {RKILLED{x!!\n\r\n\r", victim );
	break;

    default:
	if ( dam > victim->max_hit / 4 )
	    send_to_char( "That really did {RHURT{x!\n\r", victim );
	if ( victim->hit < victim->max_hit / 4 )
	{
	    send_to_char( "You sure are{R BLEEDING{x!!\n\r", victim );
	}
	break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim) )
	stop_fighting( victim, FALSE );

    if(ch->in_room->sector_type == SECT_PKARENA && victim->in_room->sector_type == SECT_PKARENA)
        arena = TRUE;
    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
    	if(!arena)
	    group_gain( ch, victim );
	
	if ( !IS_NPC(victim) )
	{
	    sprintf( log_buf, "%s killed by %s at %d",
		victim->name,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		ch->in_room->vnum );
	    log_string( log_buf );
      if(!arena)
		{
		  if (IS_SET(victim->act,PLR_KILLER))
        REMOVE_BIT(victim->act,PLR_KILLER);
		}

	    /*
	     * Dying penalty:
	     * 2/3 way back to previous level.
	     */
	   /* No penalty for arena death */
	    if(!arena)
	    if ( victim->exp > exp_per_level(victim,victim->pcdata->points) 
			       * victim->level )
	gain_exp( victim, (2 * (exp_per_level(victim,victim->pcdata->points)
			         * victim->level - victim->exp)/3) + 50 );
	}
 
        sprintf( log_buf, "%s%s got toasted by %s at %s [room %d]",
            arena ? "[{rARENA{x] " : "",
            (IS_NPC(victim) ? victim->short_descr : victim->name),
            (IS_NPC(ch) ? ch->short_descr : ch->name),
            ch->in_room->name, ch->in_room->vnum);
 
        if (IS_NPC(victim))
            wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
        else
            wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);
	/*
	 *Bounty Stuff
	 *
	 */
	if (!IS_NPC(ch)
	&& !IS_NPC(victim)
	&& victim->pcdata->bounty > 0
	&& ch != victim)
	{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf,"You recieve a %d steel bounty for killing %s.\n\r", 
	victim->pcdata->bounty, victim->name);
	send_to_char(buf, ch);
	ch->steel += victim->pcdata->bounty;
	victim->pcdata->bounty =0;
        }

	/*
	 * Death trigger
	 */
	if ( IS_NPC( victim ) && HAS_TRIGGER( victim, TRIG_DEATH) )
	{
	    victim->position = POS_STANDING;
	    mp_percent_trigger( victim, ch, NULL, NULL, TRIG_DEATH );
	}

	raw_kill( victim );

    if (IS_NPC(victim) && !IS_NPC(ch)) ch->pcdata->mkill++;
    if (!IS_NPC(victim) && IS_NPC(ch)) victim->pcdata->mdeath++;
    if (!IS_NPC(victim) && !IS_NPC(ch) && ch != victim)
    {
	ch->pcdata->pkill++;
	victim->pcdata->pdeath++;
    }

	/* dump the flags */
        if (ch != victim && !IS_NPC(ch))
        {
                REMOVE_BIT(victim->act,PLR_THIEF);
                
            	REMOVE_BIT(victim->act,PLR_VIOLENT);
            	victim->violent = 0;
            
            if(!arena)
            {
            	SET_BIT(victim->act,PLR_SLAIN);
            	victim->slain = 10;
            }
        }

        /* RT new auto commands */

	if (!IS_NPC(ch)
	&&  (corpse = get_obj_list(ch,"corpse",ch->in_room->contents)) != NULL
	&&  corpse->item_type == ITEM_CORPSE_NPC && can_see_obj(ch,corpse))
	{
	    OBJ_DATA *coins;

	    corpse = get_obj_list( ch, "corpse", ch->in_room->contents ); 

	    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
		 corpse && corpse->contains) /* exists and not empty */
            {
		do_function(ch, &do_get, "all corpse");
	    }

 	    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
	        corpse && corpse->contains  && /* exists and not empty */
		!IS_SET(ch->act,PLR_AUTOLOOT))
	    {
		if ((coins = get_obj_list(ch,"gcash",corpse->contains))
		     != NULL)
		{
		    do_function(ch, &do_get, "all.gcash corpse");
	      	}
	    }
            
	    if (IS_SET(ch->act, PLR_AUTOSAC))
	    {
       	        if (IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
       	      	{
		    return TRUE;  /* leave if corpse has treasure */
	      	}
	        else
		{
		    do_function(ch, &do_sacrifice, "corpse");
		}
	    }
	}

	return TRUE;
    }

    if ( victim == ch )
	return TRUE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	if ( number_range( 0, victim->wait ) == 0 )
	{
	    do_function(victim, &do_recall, "" );
	    return TRUE;
	}
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
	if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 2 ) == 0
	&&   victim->hit < victim->max_hit / 5) 
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL
	&&     victim->master->in_room != victim->in_room ) )
	{
	    do_function(victim, &do_flee, "" );
	}
    }

    if ( !IS_NPC(victim)
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait < PULSE_VIOLENCE / 2 )
    {
	do_function (victim, &do_flee, "" );
    }

    tail_chain( );
    return TRUE;
}

bool check_phase( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if (IS_NPC(victim))
        return FALSE;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_phase) / 2;

    if (!can_see(victim,ch))
	chance /= 2;

   if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "Your body phases to avoid $n's attack.", ch, NULL, victim, 
    TO_VICT    );
    act( "$N's body phases to avoid your attack.", ch, NULL, victim, 
    TO_CHAR    );
    check_improve(victim,gsn_phase,TRUE,6);
    return TRUE;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (victim->in_room == NULL || ch->in_room == NULL)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL)
	return FALSE;
	

    if( IS_SET(victim->act, PLR_KILLER) )
	return FALSE;
    if( IS_SET(victim->act, PLR_VIOLENT) )
	return FALSE;
    if( IS_SET(victim->act, PLR_THIEF) )
	return FALSE;

    if( IS_SET(victim->act, PLR_SLAIN) )
    {
    	send_to_char( "Wait until they've had a chance to rest.\n\r", ch );
    	return TRUE;
    }

    /* killing mobiles */
    if (IS_NPC(victim))
    {

	/* safe room? */
	if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	{
	    send_to_char("Not in this room.\n\r",ch);
	    return TRUE;
	}
	
	if( IS_SET( victim->act2, ACT_NO_ATTACK ) )
	{
	    send_to_char( "An invisible force stops you.\n\r", ch );
	    return TRUE;
	}

	if (victim->pIndexData->pShop != NULL)
	{
	    send_to_char("The shopkeeper wouldn't like that.\n\r",ch);
	    return TRUE;
	}

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	||  IS_SET(victim->act,ACT_IS_CHANGER)
	||  IS_SET(victim->act2,ACT_BANKER))
	{
	    send_to_char("I don't think the gods would approve.\n\r",ch);
	    return TRUE;
	}

	if (!IS_NPC(ch))
	{
	    /* no pets */
	    if (IS_SET(victim->act,ACT_PET))
	    {
		act("But $N looks so cute and cuddly...",
		    ch,NULL,victim,TO_CHAR);
		return TRUE;
	    }

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)
	    {
		send_to_char("You don't own that monster.\n\r",ch);
		return TRUE;
	    }
	}
    }
    /* killing players */
    else
    {
	/* NPC doing the killing */
	if (IS_NPC(ch))
	{
	    /* safe room check */

	    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	    {
		send_to_char("Not in this room.\n\r",ch);
		return TRUE;
	    }

	}
	/* player doing the killing */
	else
	{
	    if( ch->in_room->sector_type == SECT_PKARENA
	    && victim->in_room->sector_type == SECT_PKARENA)
	    	return FALSE;
	if(!IS_NPC(ch)||!IS_NPC(victim))
	{
	    if( IS_SET(victim->act, PLR_VIOLENT))
		{
		   return FALSE;
		}
	    if( IS_SET(victim->act, PLR_KILLER))
		{
		   return FALSE;
		}
            if (!IS_SET(ch->act, PLR_PROKILLER))
            {
                send_to_char("You must be {RPK{x if you want to kill players.\n\r",ch);
                return TRUE;
            }
        
            if (!IS_SET(victim->act, PLR_PROKILLER) )
            {
                send_to_char("You can only attack other {RPK{x people.\n\r",ch);
                return TRUE;
            }
	}
	    
		if(IS_SET(victim->in_room->room_flags, ROOM_SAFE))
	        return TRUE;

		if (IS_SET(victim->act, PLR_KILLER) ||
	    	IS_SET(victim->act, PLR_THIEF)  ||
	    	(IS_SET(victim->act, PLR_VIOLENT) && ch->level < victim->level - 15 && ch->level > victim->level + 15))
		return FALSE;

	    if( ch->level < 15 )
	    {
	    	send_to_char( "You're not allowed to PK until level 15.\n\r", ch );
	    	return TRUE;
	    }
	    
	    if( victim->level < 15 )
	    {
	    	send_to_char( "Wait until they've reached level 15.\n\r", ch );
	    	return TRUE;
	    }

            if (!IS_SET(ch->act, PLR_PROKILLER))
            {
                send_to_char("You must be {RPK{x if you want to kill players.\n\r",ch);
                return TRUE;
            }
        
            if (!IS_SET(victim->act, PLR_PROKILLER) )
            {
                send_to_char("You can only attack other {RPK{x people.\n\r",ch);
                return TRUE;
            }

	    if( ch->level < victim->level - 15)
	    {
	    	send_to_char("Death would thank you for your gift.\n\r",ch);
	    	return TRUE;
	    }

	    if (ch->level > victim->level + 15)
	    {
		send_to_char("Pick on someone your own size.\n\r",ch);
		return TRUE;
	    }

	}
    }
    return FALSE;
}
 
bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area )
{

    return FALSE;

    if (victim->in_room == NULL || ch->in_room == NULL)
        return TRUE;

    if (victim == ch && area)
	return TRUE;

    if (victim->fighting == ch || victim == ch)
	return FALSE;

    if (IS_IMMORTAL(ch) && ch->level > LEVEL_IMMORTAL && !area)
	return FALSE;
	
    if( IS_SET(victim->act, PLR_SLAIN) )
    {
    	send_to_char( "Wait until they've had a chance to rest.\n\r", ch );
    	return TRUE;
    }

    /* killing mobiles */
    if (IS_NPC(victim))
    {
	/* safe room? */
	if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
	    return TRUE;

	if (victim->pIndexData->pShop != NULL)
	    return TRUE;

	/* no killing healers, trainers, etc */
	if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	||  IS_SET(victim->act,ACT_IS_CHANGER))
	    return TRUE;

	if (!IS_NPC(ch))
	{
	    /* no pets */
	    if (IS_SET(victim->act,ACT_PET))
	   	return TRUE;

	    /* no charmed creatures unless owner */
	    if (IS_AFFECTED(victim,AFF_CHARM) && (area || ch != victim->master))
		return TRUE;

	    /* legal kill? -- cannot hit mob fighting non-group member */
	    if (victim->fighting != NULL && !is_same_group(ch,victim->fighting))
		return TRUE;
	}
	else
	{
	    /* area effect spells do not hit other mobs */
	    if (area && !is_same_group(victim,ch->fighting))
		return TRUE;
	}
    }
    /* killing players */
    else
    {
	if (area && IS_IMMORTAL(victim))
	    return TRUE;

	/* NPC doing the killing */
	if (IS_NPC(ch))
	{
	    /* charmed mobs and pets cannot attack players while owned */
	    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
	    &&  ch->master->fighting != victim)
		return TRUE;
	
	    /* safe room? */
	    if (IS_SET(victim->in_room->room_flags,ROOM_SAFE))
		return TRUE;

	    /* legal kill? -- mobs only hit players grouped with opponent*/
	    if (ch->fighting != NULL && !is_same_group(ch->fighting,victim))
		return TRUE;
	}

	/* player doing the killing */
	else
	{
	    if (!is_clan(ch))
		return TRUE;

	    if (IS_SET(victim->act, PLR_KILLER) ||
	    	IS_SET(victim->act, PLR_THIEF)  ||
	    	IS_SET(victim->act, PLR_VIOLENT) )
		return FALSE;

	    if( (ch->level < 15) || (victim->level < 15) )
	    	return TRUE;

            if (!IS_SET(ch->act,PLR_PROKILLER))
                return TRUE;
            
            if (IS_SET(victim->act,PLR_PROKILLER))
                return FALSE;
        
            if (!IS_SET(victim->act,PLR_PROKILLER))
                return TRUE;

	    if( ch->level < victim->level - 8)
	    	return TRUE;

	    if (ch->level > victim->level + 8)
		return TRUE;
	}

    }
    return FALSE;
}
/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    /*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    while ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
	victim = victim->master;

    /*
     * NPC's are fair game.
     * So are killers and thieves.
     * ** NEW ** Pk arena
     */
    if ( IS_NPC(victim)
    ||	 (ch->in_room->sector_type == SECT_PKARENA
    	&& victim->in_room->sector_type == SECT_PKARENA)
    ||   IS_SET(victim->act, PLR_KILLER)
    ||   IS_SET(victim->act, PLR_THIEF)
    ||	 IS_SET(victim->act, PLR_VIOLENT))
	return;

    /*
     * Charm-o-rama.
     */
    if ( IS_SET(ch->affected_by, AFF_CHARM) )
    {
	if ( ch->master == NULL )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    REMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}
/*
	send_to_char( "{r*** You are now a {RKILLER{r!! ***{x\n\r", ch->master );
  	SET_BIT(ch->master->act, PLR_KILLER);
*/

	stop_follower( ch );
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch)
    ||   ch == victim
    ||   ch->level >= LEVEL_IMMORTAL
    ||   IS_SET(ch->act, PLR_KILLER) 
    ||	 ch->fighting  == victim
    ||	 !IS_SET(victim->in_room->room_flags, ROOM_LAW) )
	return;

    send_to_char( "{r*** You are now a {RKILLER{r!! ***{x\n\r", ch );
    SET_BIT(ch->act, PLR_KILLER);
    sprintf(buf,"$N is attempting to murder %s in a lawful room",victim->name);
    wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
    save_char_obj( ch );
    return;
}

/* sets both characters' violent flags */
void check_violent( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if( !IS_NPC(victim) && !IS_NPC(ch) )
    {
    	/* No violent flags for pkarena battles */
    	if(ch->in_room->sector_type == SECT_PKARENA
    	  && victim->in_room->sector_type == SECT_PKARENA)
    	    return;
    	    
    	if(IS_IMMORTAL(ch) || IS_IMMORTAL(victim))
    	    return;
    	
    	if( IS_SET(ch->act, PLR_SLAIN) )
    	{
    		REMOVE_BIT( ch->act, PLR_SLAIN );
    		ch->slain = 0;
    		send_to_char( "You have waived your safety rights.\n\r", ch );
    	}
    
	SET_BIT( ch->act, PLR_VIOLENT );
	ch->violent = 7;
    	save_char_obj( ch );
    
    	SET_BIT( victim->act, PLR_VIOLENT );
    	victim->violent = 8;
    	save_char_obj( victim );
    }
}    
    


/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_parry) / 2;

    if ( get_eq_char( victim, WEAR_RIGHT ) == NULL )
    {
	if (IS_NPC(victim))
	    chance /= 2;
	else
	    return FALSE;
    }

    if (!can_see(ch,victim))
	chance /= 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
	return FALSE;

    act( "You parry $n's attack.",  ch, NULL, victim, TO_VICT    );
    act( "$N parries your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_parry,TRUE,6);
    return TRUE;
}

/*
 * Check for shield block.
 */
bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;
    OBJ_DATA *wr, *wl;

    if ( !IS_AWAKE(victim) )
        return FALSE;


    chance = get_skill(victim,gsn_shield_block)-10;


    wr = get_eq_char(victim, WEAR_RIGHT);
    wl = get_eq_char(victim, WEAR_LEFT);
    if ((wr == NULL || (wr != NULL && wr->item_type != ITEM_WEAR_SHIELD))
    && (wl == NULL || (wl != NULL && wl->item_type != ITEM_WEAR_SHIELD)))
        return FALSE;

    check_improve(victim,gsn_shield_block,FALSE,6);

    if ( number_percent( ) >= chance )
        return FALSE;

    act( "You block $n's attack with your shield.",  ch, NULL, victim, 
TO_VICT    );
    act( "$N blocks your attack with a shield.", ch, NULL, victim, 
TO_CHAR    );
    check_improve(victim,gsn_shield_block,TRUE,6);
    return TRUE;
}


/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
	return FALSE;

    chance = get_skill(victim,gsn_dodge) / 2;

    if (!can_see(victim,ch))
	chance /= 2;

    if ( number_percent( ) >= chance + victim->level - ch->level )
        return FALSE;

    act( "You dodge $n's attack.", ch, NULL, victim, TO_VICT    );
    act( "$N dodges your attack.", ch, NULL, victim, TO_CHAR    );
    check_improve(victim,gsn_dodge,TRUE,6);
    return TRUE;
}



/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( victim->hit > 0 )
    {
    	if ( victim->position <= POS_STUNNED )
	    victim->position = POS_STANDING;
	return;
    }

    if ( IS_NPC(victim) && victim->hit < 1 )
    {
	victim->position = POS_DEAD;
	return;
    }

    if ( victim->hit <= -11 )
    {
	victim->position = POS_DEAD;
	return;
    }

         if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    return;
}



/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fighting != NULL )
    {
	bug( "Set_fighting: already fighting", 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
	affect_strip( ch, gsn_sleep );

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    return;
}



/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
	if ( fch == ch || ( fBoth && fch->fighting == ch ) )
	{
	    fch->fighting	= NULL;
	    fch->position	= IS_NPC(fch) ? fch->default_pos
	    : (IS_SET(ch->act, PLR_MOUNTED) ? POS_RIDING : POS_STANDING);
	    update_pos( fch );
	}
	if( IS_NPC( fch ) && fch->pIndexData->vnum == 1 )
	{
	    extract_char(fch, TRUE);
	    act("A golem explodes into dust.\n\r", NULL, NULL, NULL, TO_ALL);
	}
    }
    
    return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj2;
    OBJ_DATA *cont = NULL;
    char *name;
    
    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
	corpse->timer	= number_range( 3, 6 );
	if ( ch->steel > 0 )
	{
	    obj_to_obj( create_money( ch->steel, ch->gold ), corpse );
	    ch->steel = 0;
	    ch->gold = 0;
	}
	corpse->cost = 0;
    }
    else
    {
	name		= ch->name;
	corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
	corpse->timer	= number_range( 25, 40 );
	REMOVE_BIT(ch->act,PLR_CANLOOT);
	if (!is_clan(ch))
	    corpse->owner = str_dup(ch->name);
	else
	{
	    corpse->owner = NULL;
	    if (ch->steel > 1 || ch->gold > 1)
	    {
		obj_to_obj(create_money(ch->steel / 2, ch->gold/2), corpse);
		ch->steel -= ch->steel/2;
		ch->gold -= ch->gold/2;
	    }
	}
		
	corpse->cost = 0;
    	cont = create_object(get_obj_index(OBJ_VNUM_SAFE_CONT), 0);
    	sprintf(buf, cont->name, name);
    	free_string( cont->name );
    	cont->name = str_dup( buf );
    }

    corpse->level = ch->level;
    

    sprintf( buf, corpse->short_descr, name );
    free_string( corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( corpse->description );
    corpse->description = str_dup( buf );


    for ( obj = ch->carrying; obj != NULL; obj = obj2)
    {
	bool floating = FALSE;

	obj2 = obj->next_content;
	
	if (!IS_NPC(ch) && obj->safe == 1)
	{
	    obj_from_char(obj);
	    obj_to_obj(obj, cont);
	    continue;
	}
	
	if (obj->wear_loc == WEAR_FLOAT)
	    floating = TRUE;

	obj_from_char( obj );
	    
	if (obj->item_type == ITEM_POTION)
	    obj->timer = number_range(500,1000);
	if (obj->item_type == ITEM_SCROLL)
	    obj->timer = number_range(1000,2500);
	if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH) && !floating)
	{
	    obj->timer = number_range(5,10);
	    REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);
	}
	REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);

	if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	{
	    extract_obj( obj );
	}
	else if (floating)
	{
	    if (IS_OBJ_STAT(obj,ITEM_ROT_DEATH)) 
	    { 
		if (obj->contains != NULL)
		{
		    OBJ_DATA *in, *in_next;

		    act("$p evaporates, scattering its contents.",
			ch,obj,NULL,TO_ROOM);
		    for (in = obj->contains; in != NULL; in = in_next)
		    {
			in_next = in->next_content;
			obj_from_obj(in);
			obj_to_room(in,ch->in_room);
		    }
		 }
		 else
		    act("$p evaporates.",
			ch,obj,NULL,TO_ROOM);
		 extract_obj(obj);
		 
	    }
	    else
	    {
		act("$p falls to the floor.",ch,obj,NULL,TO_ROOM);
		obj_to_room(obj,ch->in_room);
	    }
	}
	else
	{
	    obj_to_obj( obj, corpse );
	    
	}
    }

    if(!IS_NPC(ch) && ch->level <= 5)
    {
    	int vnum;
    	if(god_table[ch->god].pc_good)
    	    vnum = ROOM_VNUM_GOOD_TEMPLE;
    	else if(god_table[ch->god].pc_evil)
    	    vnum = ROOM_VNUM_EVIL_TEMPLE;
    	else
    	    vnum = ROOM_VNUM_NEUTRAL_TEMPLE;
    	obj_to_room(corpse, get_room_index(vnum));
    }
    else
    	obj_to_room( corpse, ch->in_room );
    
    if( !IS_NPC(ch) )
	obj_to_room( cont, get_room_index(3) );
	
    make_blood(ch, 1, FALSE);
    act("$n bleeds all over the floor.", ch, NULL, NULL, TO_ROOM);
    return;
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "You hear $n's death cry.";

    switch ( number_bits(4))
    {
    case  0: msg  = "$n hits the ground ... DEAD.";			break;
    case  1: 
	if (ch->material == 0)
	{
	    msg  = "$n splatters blood on your armor.";		
	    break;
	}
    case  2: 							
	if (IS_SET(ch->parts,PART_GUTS))
	{
	    msg = "$n spills $s guts all over the floor.";
	    vnum = OBJ_VNUM_GUTS;
	}
	break;
    case  3: 
	if (IS_SET(ch->parts,PART_HEAD))
	{
	    msg  = "$n's severed head plops on the ground.";
	    vnum = OBJ_VNUM_SEVERED_HEAD;				
	}
	break;
    case  4: 
	if (IS_SET(ch->parts,PART_HEART))
	{
	    msg  = "$n's heart is torn from $s chest.";
	    vnum = OBJ_VNUM_TORN_HEART;				
	}
	break;
    case  5: 
	if (IS_SET(ch->parts,PART_ARMS))
	{
	    msg  = "$n's arm is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_ARM;				
	}
	break;
    case  6: 
	if (IS_SET(ch->parts,PART_LEGS))
	{
	    msg  = "$n's leg is sliced from $s dead body.";
	    vnum = OBJ_VNUM_SLICED_LEG;				
	}
	break;
    case 7:
	if (IS_SET(ch->parts,PART_BRAINS))
	{
	    msg = "$n's skull is shattered, and $s brains splatter all over you.";
	    vnum = OBJ_VNUM_BRAINS;
	}
    }

    act( msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum != 0 )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );

	sprintf( buf, obj->short_descr, name );
	free_string( obj->short_descr );
	obj->short_descr = str_dup( buf );

	sprintf( buf, obj->description, name );
	free_string( obj->description );
	obj->description = str_dup( buf );

	if (obj->item_type == ITEM_FOOD)
	{
	    if (IS_SET(ch->form,FORM_POISON))
		obj->value[3] = 1;
	    else if (!IS_SET(ch->form,FORM_EDIBLE))
		obj->item_type = ITEM_TRASH;
	}

	obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "You hear something's death cry.";
    else
	msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
	EXIT_DATA *pexit;

	if ( ( pexit = was_in_room->exit[door] ) != NULL
	&&   pexit->u1.to_room != NULL
	&&   pexit->u1.to_room != was_in_room )
	{
	    ch->in_room = pexit->u1.to_room;
	    act( msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}



void raw_kill( CHAR_DATA *victim )
{
    OBJ_DATA* cont;
    ROOM_INDEX_DATA *pOldRoom;
    int i;

    stop_fighting( victim, TRUE );
    death_cry( victim );
    if(!IS_NPC(victim) && victim->in_room->sector_type == SECT_PKARENA)
    {
    	victim->hit = victim->max_hit;
    	victim->mana = victim->max_mana;
    	victim->move = victim->max_move;
    	victim->position = POS_STANDING;
    	return;
    }
    make_corpse( victim );

    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
	extract_char( victim, TRUE );
	return;
    }

    extract_char( victim, FALSE );
    while ( victim->affected )
	affect_remove( victim, victim->affected );
    victim->affected_by	= race_table[victim->race].aff;
    for (i = 0; i < 4; i++)
    	victim->armor[i]= 100;
    victim->position	= POS_RESTING;
    victim->hit		= UMAX( 1, victim->hit  );
    victim->mana	= UMAX( 1, victim->mana );
    victim->move	= UMAX( 1, victim->move );
    if(victim->pcdata->condition[COND_HUNGER] < 1)
    	victim->pcdata->condition[COND_HUNGER] = 48;
    if(victim->pcdata->condition[COND_THIRST] < 1)
    	victim->pcdata->condition[COND_THIRST] = 48;
    	
/*  save_char_obj( victim ); we're stable enough to not need this :) */
    /* Retrieve all safe objects from box */
    pOldRoom = victim->in_room;
    victim->in_room = get_room_index(3);
    for(cont = victim->in_room->contents; cont != NULL; cont = cont->next_content)
    {
        if(is_name(victim->name, cont->name))
        {
            OBJ_DATA *in_cont;
            for(in_cont = cont->contains; in_cont != NULL; in_cont = in_cont->next_content)
            {
                obj_from_obj(in_cont);
                obj_to_char(in_cont, victim);
            }
            break;
        }
    }
    extract_obj(cont);
    victim->in_room = pOldRoom;
    if(!IS_NPC(victim) && victim->level <=5)
    	act("The gods show mercy to you and place your corpse next to you.",
    	victim, NULL, NULL, TO_CHAR);
    return;
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp;
    int members;
    int group_levels;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( victim == ch )
	return;
    
    members = 0;
    group_levels = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) && !IS_NPC( gch ))
        {
	    members++;
	    group_levels += gch->level;
	}
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
	group_levels = ch->level ;
    }

    lch = (ch->leader != NULL) ? ch->leader : ch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !is_same_group( gch, ch ) || IS_NPC(gch))
	    continue;

	if ( gch->level - lch->level > 15 )
	{
	    send_to_char( "You are too high for this group.\n\r", gch );
	    continue;
	}

	if ( gch->level - lch->level < -15 )
	{
	    send_to_char( "You are too low for this group.\n\r", gch );
	    continue;
	}


	xp = xp_compute( gch, victim, group_levels );  
        if (!(IS_SET(ch->act, PLR_NOEXP)) )  
           {
	   sprintf( buf, "You receive %d experience points.\n\r", xp );
	   send_to_char( buf, gch );
	   gain_exp( gch, xp );
           }
        else
           send_to_char( "You receive 0 experience points.\n\r", ch);
           
        if ( IS_SET(ch->act,PLR_QUESTOR) && IS_NPC(victim) )
           {
           if ( ch->questmob == victim->pIndexData->vnum )
           	{
           	send_to_char( "You have almost completed your QUEST!\n\r", ch );
           	send_to_char( "Return to the questmaster before your time runs out!\n\r", ch );
           	ch->questmob = -1;
           	}
           }
           

	for ( obj = ch->carrying; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	    {
		act( "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
		act( "$n is zapped by $p.",   ch, obj, NULL, TO_ROOM );
		obj_from_char( obj );
		obj_to_room( obj, ch->in_room );
	    }
	}
    }

    return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels )
{
    int xp,base_exp;
    int align,level_range;
    int change;
    int time_per_level;

    level_range = victim->level - gch->level;

    /* compute the base exp */
    switch (level_range)
    {
 	default : 	base_exp =   0;		break;
	case -9 :	base_exp =   1;		break;
	case -8 :	base_exp =   2;		break;
	case -7 :	base_exp =   5;		break;
	case -6 : 	base_exp =   9;		break;
	case -5 :	base_exp =  11;		break;
	case -4 :	base_exp =  22;		break;
	case -3 :	base_exp =  33;		break;
	case -2 :	base_exp =  50;		break;
	case -1 :	base_exp =  70;		break;
	case  0 :	base_exp =  90;		break;
	case  1 :	base_exp = 115;		break;
	case  2 :	base_exp = 138;		break;
	case  3 :	base_exp = 153;		break;
	case  4 :	base_exp = 175;		break;
    }

    if (level_range > 4)
	base_exp = 180 + (20 * (level_range - 4));

    /* do alignment computations */

    align = victim->alignment - gch->alignment;

    if (IS_SET(victim->act,ACT_NOALIGN))
    {
    }

    else if (align > 500) /* monster is more good than slayer */ 
    {
/*	change = ( 20  );
	change = UMAX(1,change);
        gch->alignment = UMAX(-1000,gch->alignment - change); */
    }

    else if (align < -500) /*  monster is more evil than slayer */
    {
/*
	change =  (  -20 );
	change = UMAX(1,change);
	gch->alignment = UMIN(1000,gch->alignment + change);
*/
    }

    else  /* improve this someday */
    {
/*
	change =  gch->alignment * base_exp/500 * gch->level/total_levels;  
	gch->alignment -= URANGE(-1000, change, 1000);
*/
    }
   
    /* calculate exp multiplier */
    if (IS_SET(victim->act,ACT_NOALIGN))
	xp = base_exp;


    else if (gch->alignment > 500)  /* for goodie two shoes */
    {
	if (victim->alignment < -750)
	    xp = (base_exp *4)/3;
   
 	else if (victim->alignment < -500)
	    xp = (base_exp * 5)/4;

        else if (victim->alignment > 750)
	    xp = base_exp / 4;

   	else if (victim->alignment > 500)
	    xp = base_exp / 2;

        else if (victim->alignment > 250)
	    xp = (base_exp * 3)/4; 

	else
	    xp = base_exp;
    }

    else if (gch->alignment < -500) /* for baddies */
    {
	if (victim->alignment > 750)
	    xp = (base_exp * 5)/4;
	
  	else if (victim->alignment > 500)
	    xp = (base_exp * 11)/10; 

   	else if (victim->alignment < -750)
	    xp = base_exp/2;

	else if (victim->alignment < -500)
	    xp = (base_exp * 3)/4;

	else if (victim->alignment < -250)
	    xp = (base_exp * 9)/10;

	else
	    xp = base_exp;
    }

    else if (gch->alignment > 200)  /* a little good */
    {

	if (victim->alignment < -500)
	    xp = (base_exp * 6)/5;

 	else if (victim->alignment > 750)
	    xp = base_exp/2;

	else if (victim->alignment > 0)
	    xp = (base_exp * 3)/4; 
	
	else
	    xp = base_exp;
    }

    else if (gch->alignment < -200) /* a little bad */
    {
	if (victim->alignment > 500)
	    xp = (base_exp * 6)/5;
 
	else if (victim->alignment < -750)
	    xp = base_exp/2;

	else if (victim->alignment < 0)
	    xp = (base_exp * 3)/4;

	else
	    xp = base_exp;
    }

    else /* neutral */
    {

	if (victim->alignment > 500 || victim->alignment < -500)
	    xp = (base_exp * 4)/3;

	else if (victim->alignment < 200 && victim->alignment > -200)
	    xp = base_exp/2;

 	else
	    xp = base_exp;
    }

    /* more exp at the low levels */
    if (gch->level < 5)
        xp = 2 * xp;
    else if (gch->level < 10)
        xp = 14 * xp / (gch->level + 3);
    else if (gch->level < 15)
    	xp = 13 * xp / (gch->level - 5);

    /* less at high */
    if (gch->level > 55 )
	xp =  13 * xp / (gch->level - 8 );

    /* reduce for playing time */
    
    {
	/* compute quarter-hours per level */
	time_per_level = 4 *
			 (gch->played + (int) (current_time - gch->logon))/3600
			 / gch->level;

	time_per_level = URANGE(2,time_per_level,12);
	if (gch->level < 15)  /* make it a curve */
	    time_per_level = UMAX(time_per_level,(15 - gch->level));
	xp = xp * time_per_level / 5;
    }
   
    /* randomize the rewards */
    xp = number_range (xp * 3/4 , xp * 5/4);

    /* adjust for grouping */
//    xp = xp * gch->level/( UMAX(1,total_levels/2 -1) );

    return xp;
}


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune )
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;

    if (ch == NULL || victim == NULL)
	return;

	 if ( dam ==   0 ) { vs = "miss";	vp = "misses";	}
    else if ( dam <=   3 ) { vs = "{wscratch{x"; vp = "{wscratches{x";	}
    else if ( dam <=   6 ) { vs = "{ggraze{x";	vp = "{ggrazes{x";	}
    else if ( dam <=  10 ) { vs = "{ghit{x";	vp = "{ghits{x";	}
    else if ( dam <=  14 ) { vs = "{binjure{x";	vp = "{binjures{x";	}
    else if ( dam <=  18 ) { vs = "{bwound{x";	vp = "{bwounds{x";	}
    else if ( dam <=  22 ) { vs = "{ymaul{x";   vp = "{ymauls{x";	}
    else if ( dam <=  26 ) { vs = "{rdecimate{x"; vp = "{rdecimates{x";  }
    else if ( dam <=  32 ) { vs = "{rdevastate{x"; vp = "{rdevastates{x";}
    else if ( dam <=  34 ) { vs = "{Gmaim{x";	vp = "{Gmaims{x"; 	}
    else if ( dam <=  42 ) { vs = "{YMUTILATE{x"; vp= "{YMUTILATES{x";}
    else if ( dam <=  46 ) { vs = "{RDISEMBOWEL{x";  vp ="{RDISEMBOWELS{x";}
    else if ( dam <=  50 ) { vs = "{MMASSACRE{x"; vp="{MMASSACRES{x";	}
    else if ( dam <=  80 ) { vs = "{MMANGLE{x"; vp="{MMANGLES{x";	}
    else if ( dam <= 100 ) { vs = "{R@@@ {MDISMEMBER {R@@@{x";
    			     vp = "{R@@@ {MDISMEMBERS {R@@@{x"; }
    else if ( dam <= 110 ) { vs = "{r*** {bDEMOLISH {r***{x";
			     vp = "{r*** {bDEMOLISHES {r***{x"; }
    else if ( dam <= 130 ) { vs = "{Y*** {BDEVASTATE {Y***{x";
			     vp = "{Y*** {BDEVASTATES {Y***{x";	}
    else if ( dam <= 140 ) { vs = "{WABSOLUTELY HAMMER{x";
    			     vp = "{WABSOLUTELY HAMMERS{x"; }
    else if ( dam <= 150 ) { vs = "{r>>> {RANNIHILATE {r<<<{x";	
			     vp = "{r>>> {RANNIHILATES {r<<<{x"; }
    else if ( dam <= 175 ) { vs = "{b<<< {YERADICATE {b>>>{x"; 
			     vp = "{b<<< {YERADICATES {b>>>{x"; }
    else if ( dam <= 190 ) { vs = "{y=== {GLACERATE {y==={x";
    			     vp = "{y=== {GLACERATES {y==={x"; }
    else if ( dam <= 220 ) { vs = "{rBUTCHER{x";	
			     vp = "{rBUTCHERS{x";	}
    else if ( dam <= 240 ) { vs = "{rSLAUGHTER{x";
			     vp = "{rSLAUGHTERS{x";	}
    else if ( dam <= 270 ) { vs = "{RMURDER{x";
			     vp = "{RMURDERS{x";	}
    else if ( dam <= 300 ) { vs = "{WDESTROY{x";
    			     vp = "{WDESTROYS{x";	}
    else if ( dam <= 700 ) { vs = "{WLIQUIFY{x";
			     vp = "{WLIQUIFIES{x"; 	}
    else                   { vs = "{WL{RI{BQ{GU{YI{MF{CY{*{x";
			     vp = "{WL{RI{BQ{GU{YI{MF{CI{DE{GS{*{x"; 	}

    punct   = (dam <= 30) ? '.' : '!';

    if ( dt == TYPE_HIT )
    {
	if (ch  == victim)
	{
	    sprintf( buf1, "$n %s $melf%c{x",vp,punct);
	    sprintf( buf2, "You %s yourself%c{x",vs,punct);
	}
	else
	{
		if (IS_SET(ch->act,PLR_AUTODAMAGE)||IS_SET(victim->act,PLR_AUTODAMAGE)||IS_SET(victim->act,ACT_PET)||IS_SET(ch->act,ACT_PET))
		{
		if(dam<=100)
		{
	    sprintf( buf1, "$n %s $N%c [{B%d{x]{x",  vp, punct, dam );
	    sprintf( buf2, "You %s $N%c{x [{B%d{x]", vs, punct, dam );
	    sprintf( buf3, "$n %s you%c{x [{B%d{x]", vp, punct, dam );
		}
		else if(dam>=100&&dam<=200)
		{
	    sprintf( buf1, "$n %s $N%c [{R%d{x]{x",  vp, punct, dam );
	    sprintf( buf2, "You %s $N%c{x [{R%d{x]", vs, punct, dam );
	    sprintf( buf3, "$n %s you%c{x [{R%d{x]", vp, punct, dam );
		}
		else if(dam>=200&&dam<=300)
		{
	    sprintf( buf1, "$n %s $N%c [{G%d{x]{x",  vp, punct, dam );
	    sprintf( buf2, "You %s $N%c{x [{G%d{x]", vs, punct, dam );
	    sprintf( buf3, "$n %s you%c{x [{G%d{x]", vp, punct, dam );
		}
		else if(dam>=300&&dam<=500)
		{
	    sprintf( buf1, "$n %s $N%c [{Y%d{x]{x",  vp, punct, dam );
	    sprintf( buf2, "You %s $N%c{x [{Y%d{x]", vs, punct, dam );
	    sprintf( buf3, "$n %s you%c{x [{Y%d{x]", vp, punct, dam );
		}
		else if(dam>=500&&dam<=700)
		{
	    sprintf( buf1, "$n %s $N%c [{C%d{x]{x",  vp, punct, dam );
	    sprintf( buf2, "You %s $N%c{x [{C%d{x]", vs, punct, dam );
	    sprintf( buf3, "$n %s you%c{x [{C%d{x]", vp, punct, dam );
		}
		else if(dam>=700&&dam<=1000)
		{
	    sprintf( buf1, "$n %s $N%c [{W%d{x]{x",  vp, punct, dam );
	    sprintf( buf2, "You %s $N%c{x [{W%d{x]", vs, punct, dam );
	    sprintf( buf3, "$n %s you%c{x [{W%d{x]", vp, punct, dam );
		}
		else
		{
	    sprintf( buf1, "$n %s $N%c [{M%d{x]{x",  vp, punct, dam );
	    sprintf( buf2, "You %s $N%c{x [{M%d{x]", vs, punct, dam );
	    sprintf( buf3, "$n %s you%c{x [{M%d{x]", vp, punct, dam );
		}
		}
		else
		{
	    sprintf( buf1, "$n %s $N%c{x",  vp, punct );
	    sprintf( buf2, "You %s $N%c{x", vs, punct );
	    sprintf( buf3, "$n %s you%c{x", vp, punct );
		}
	   

	}
    }
    else
    {
	if ( dt >= 0 && dt < MAX_SKILL )
	    attack	= skill_table[dt].noun_damage;
	else if ( dt >= TYPE_HIT
	&& dt < TYPE_HIT + MAX_DAMAGE_MESSAGE) 
	    attack	= attack_table[dt - TYPE_HIT].noun;
	else
	{
	    bug( "Dam_message: bad dt %d.", dt );
	    dt  = TYPE_HIT;
	    attack  = attack_table[0].name;
	}

	if (immune)
	{
	    if (ch == victim)
	    {
		sprintf(buf1,"$n is unaffected by $s own %s.{x",attack);
		sprintf(buf2,"Luckily, you are immune to that.{x");
	    } 
	    else
	    {
	    	sprintf(buf1,"$N is unaffected by $n's %s!{x",attack);
	    	sprintf(buf2,"$N is unaffected by your %s!{x",attack);
	    	sprintf(buf3,"$n's %s is powerless against you.{x",attack);
	    }
	}
	else
	{
	    if (ch == victim)
	    {
		sprintf( buf1, "$n's %s %s $m%c{x",attack,vp,punct);
		sprintf( buf2, "Your %s %s you%c{x",attack,vp,punct);
	    }
	    else
	    {
		if (IS_SET(ch->act,PLR_AUTODAMAGE)||IS_SET(victim->act,PLR_AUTODAMAGE)||IS_SET(victim->act,ACT_PET)||IS_SET(ch->act,ACT_PET))
		{
		if(dam<=100)
		{
	    	sprintf( buf1, "$n's %s %s $N%c [{W%d{x]{x ",  attack, vp, punct, dam );
	    	sprintf( buf2, "Your %s %s $N%c [{W%d{x]{x ",  attack, vp, punct, dam );
	    	sprintf( buf3, "$n's %s %s you%c [{W%d{x]{x ", attack, vp, punct, dam );
		}
		else if(dam>=100&&dam<=200)
		{
	    	sprintf( buf1, "$n's %s %s $N%c [{B%d{x]{x ",  attack, vp, punct, dam );
	    	sprintf( buf2, "Your %s %s $N%c [{B%d{x]{x ",  attack, vp, punct, dam );
	    	sprintf( buf3, "$n's %s %s you%c [{B%d{x]{x ", attack, vp, punct, dam );
		}
		else if(dam>=200&&dam<=300)
		{
	    	sprintf( buf1, "$n's %s %s $N%c [{Y%d{x]{x ",  attack, vp, punct, dam );
	    	sprintf( buf2, "Your %s %s $N%c [{Y%d{x]{x ",  attack, vp, punct, dam );
	    	sprintf( buf3, "$n's %s %s you%c [{Y%d{x]{x ", attack, vp, punct, dam );
		}
		else if(dam>=300&&dam<=500)
		{
	    	sprintf( buf1, "$n's %s %s $N%c [{R%d{x]{x ",  attack, vp, punct, dam );
	    	sprintf( buf2, "Your %s %s $N%c [{R%d{x]{x ",  attack, vp, punct, dam );
	    	sprintf( buf3, "$n's %s %s you%c [{R%d{x]{x ", attack, vp, punct, dam );
		}
		else if(dam>=500&&dam<=700)
		{
	    	sprintf( buf1, "$n's %s %s $N%c [{G%d{x]{x ",  attack, vp, punct, dam );
	    	sprintf( buf2, "Your %s %s $N%c [{G%d{x]{x ",  attack, vp, punct, dam );
	    	sprintf( buf3, "$n's %s %s you%c [{G%d{x]{x ", attack, vp, punct, dam );
		}
		else if(dam>=700&&dam<=1000)
		{
	    	sprintf( buf1, "$n's %s %s $N%c [{M%d{x]{x ",  attack, vp, punct, dam );
	    	sprintf( buf2, "Your %s %s $N%c [{M%d{x]{x ",  attack, vp, punct, dam );
	    	sprintf( buf3, "$n's %s %s you%c [{M%d{x]{x ", attack, vp, punct, dam );
		}
		else
		{
	    	sprintf( buf1, "$n's %s %s $N%c [{y%d{x]{x ",  attack, vp, punct, dam );
	    	sprintf( buf2, "Your %s %s $N%c [{y%d{x]{x ",  attack, vp, punct, dam );
	    	sprintf( buf3, "$n's %s %s you%c [{y%d{x]{x ", attack, vp, punct, dam );
		}
		}
		else
		{
	    	sprintf( buf1, "$n's %s %s $N%c{x",  attack, vp, punct );
	    	sprintf( buf2, "Your %s %s $N%c{x",  attack, vp, punct );
	    	sprintf( buf3, "$n's %s %s you%c{x", attack, vp, punct );
		}
	    }
	}
    }

    if (ch == victim)
    {
	act(buf1,ch,NULL,NULL,TO_ROOM);
	act(buf2,ch,NULL,NULL,TO_CHAR);
    }
    else
    {
    	act( buf1, ch, NULL, victim, TO_NOTVICT );
    	act( buf2, ch, NULL, victim, TO_CHAR );
    	act( buf3, ch, NULL, victim, TO_VICT );
    }

    return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( victim, WEAR_RIGHT ) ) == NULL )
	return;

    if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
    {
	act("{x$S weapon won't budge!{x",ch,NULL,victim,TO_CHAR);
	act("{x$n tries to disarm you, but your weapon won't budge!{x",
	    ch,NULL,victim,TO_VICT);
	act("{x$n tries to disarm $N, but fails.{x",ch,NULL,victim,TO_NOTVICT);
	return;
    }

    act( "$n {GDISARMS {xyou and sends your weapon flying!{x", 
	 ch, NULL, victim, TO_VICT    );
    act( "You {Gdisarm {x$N!{x",  ch, NULL, victim, TO_CHAR    );
    act( "$n {Gdisarms {x$N!{x",  ch, NULL, victim, TO_NOTVICT );

    obj_from_char( obj );
    if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) )
	obj_to_char( obj, victim );
    else
    {
	obj_to_room( obj, victim->in_room );
	if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
	    get_obj(victim,obj,NULL);
    }

    return;
}

void do_berserk( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;

    if ((chance = get_skill(ch,gsn_berserk)) == 0
    ||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    ||  (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_berserk].skill_level[ch->class]))
    {
	send_to_char("You turn red in the face, but nothing happens.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,gsn_berserk)
    ||  is_affected(ch,skill_lookup("frenzy")))
    {
	send_to_char("You get a little madder.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	send_to_char("You're feeling to mellow to berserk.\n\r",ch);
	return;
    }

    if (ch->mana < 50)
    {
	send_to_char("You can't get up enough energy.\n\r",ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
	AFFECT_DATA af;

	WAIT_STATE(ch,PULSE_VIOLENCE);
	ch->mana -= 50;
	ch->move /= 2;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit,ch->max_hit);

	send_to_char("Your pulse races as you are consumed by rage!\n\r",ch);
	act("$n gets a wild look in $s eyes.",ch,NULL,NULL,TO_ROOM);
	check_improve(ch,gsn_berserk,TRUE,2);

	af.where	= TO_AFFECTS;
	af.type		= gsn_berserk;
	af.level	= ch->level;
	af.duration	= number_fuzzy(ch->level / 8);
	af.modifier	= UMAX(1,ch->level/5);
	af.bitvector 	= AFF_BERSERK;

	af.location	= APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	= APPLY_DAMROLL;
	affect_to_char(ch,&af);

	af.modifier	= UMAX(10,10 * (ch->level/5));
	af.location	= APPLY_AC;
	affect_to_char(ch,&af);
    }

    else
    {
	WAIT_STATE(ch,3 * PULSE_VIOLENCE);
	ch->mana -= 25;
	ch->move /= 2;

	send_to_char("Your pulse speeds up, but nothing happens.\n\r",ch);
	check_improve(ch,gsn_berserk,FALSE,2);
    }
}
void do_blur( CHAR_DATA *ch, char *argument)
{
    int chance, hp_percent;

    if ((chance = get_skill(ch,gsn_berserk)) == 0
    ||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BERSERK))
    ||  (!IS_NPC(ch)
    &&   ch->level < skill_table[gsn_berserk].skill_level[ch->class]))
    {
	send_to_char("What?.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(ch,AFF_BLUR) || is_affected(ch,gsn_blur)
    ||  is_affected(ch,skill_lookup("frenzy")))
    {
	send_to_char("You already did that.\n\r",ch);
	return;
    }


    if (ch->mana < 50)
    {
	send_to_char("You can't get up enough energy.\n\r",ch);
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit/ch->max_hit;
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
	AFFECT_DATA af;

	WAIT_STATE(ch,PULSE_VIOLENCE);
	ch->mana -= 50;
	ch->move /= 2;

	/* heal a little damage */
	ch->hit += ch->level * 2;
	ch->hit = UMIN(ch->hit,ch->max_hit);

	send_to_char("Your vision is blurred, as you get pissed as hell...\n\r",ch);
	act("$n gets a wild look in $s eyes.",ch,NULL,NULL,TO_ROOM);
	check_improve(ch,gsn_blur,TRUE,2);

	af.where	= TO_AFFECTS;
	af.type		= gsn_blur;
	af.level	= ch->level;
	af.duration	= number_fuzzy(ch->level / 8);
	af.modifier	= UMAX(1,ch->level/5);
	af.bitvector 	= AFF_BLUR;

	af.location	= APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	= APPLY_DAMROLL;
	affect_to_char(ch,&af);

	af.modifier	= -100;
	af.location	= APPLY_AC;
	affect_to_char(ch,&af);
    }

    else
    {
	WAIT_STATE(ch,3 * PULSE_VIOLENCE);
	ch->mana -= 25;

	send_to_char("You failed.\n\r",ch);
	check_improve(ch,gsn_blur,FALSE,2);
    }
}

void do_bash( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
/*
    CHAR_DATA *wch;

    if(IS_SET(wch->act, PLR_ROLEPL))
    {
        send_to_char("Sorry, you can't attack a player in RP Mode!\n\r", ch);
        return;
    }

    if(IS_SET(wch->act, PLR_ROLEP2))
    {
        send_to_char("Sorry, you can't attack a player in RP Mode!\n\r", ch);
        return;
    }
*/
    one_argument(argument,arg);
 
    if ( (chance = get_skill(ch,gsn_bash)) == 0
    ||	 (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))
    ||	 (!IS_NPC(ch)
    &&	  ch->level < skill_table[gsn_bash].skill_level[ch->class]))
    {	
	send_to_char("Bashing? What's that?\n\r",ch);
	return;
    }
 
    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
	return;
    } 

    if (victim == ch)
    {
	send_to_char("You try to bash your brains out, but fail.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if ( IS_NPC(victim) && 
	victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;

    if (ch->size < victim->size)
	chance += (ch->size - victim->size) * 15;
    else
	chance += (ch->size - victim->size) * 10; 


    /* stats */
    chance += get_curr_stat(ch,STAT_STR);
    chance -= (get_curr_stat(victim,STAT_DEX) * 4)/3;
    chance -= GET_AC(victim,AC_BASH) /25;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
        chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
        chance -= 30;

    /* level */
    chance += (ch->level - victim->level);

    if (!IS_NPC(victim) 
	&& chance < get_skill(victim,gsn_dodge) )
    {	/*
        act("$n tries to bash you, but you dodge it.{x",ch,NULL,victim,TO_VICT);
        act("$N dodges your bash, you fall flat on your face.{x",ch,NULL,victim,TO_CHAR);
        WAIT_STATE(ch,skill_table[gsn_bash].beats);
        return;*/
	chance -= 3 * (get_skill(victim,gsn_dodge) - chance);
    }

    /* now the attack */
    if (number_percent() < chance )
    {
    
	act("{x$n sends you sprawling with a powerful bash!{x",
		ch,NULL,victim,TO_VICT);
	act("{xYou slam into $N, and send $M flying!{x",ch,NULL,victim,TO_CHAR);
	act("{x$n sends $N sprawling with a powerful bash.{x",
		ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_bash,TRUE,1);

	DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2,2 + 2 * ch->size + chance/20),gsn_bash,
	    DAM_BASH,FALSE);
	
    }
    else
    {
	damage(ch,victim,0,gsn_bash,DAM_BASH,FALSE);
	act("{xYou fall flat on your face!{x",
	    ch,NULL,victim,TO_CHAR);
	act("{x$n falls flat on $s face.{x",
	    ch,NULL,victim,TO_NOTVICT);
	act("{xYou evade $n's bash, causing $m to fall flat on $s face.{x",
	    ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_bash,FALSE,1);
	ch->position = POS_RESTING;
	REMOVE_BIT(ch->act, PLR_MOUNTED);
	WAIT_STATE(ch,skill_table[gsn_bash].beats * 3/2); 
    }
	check_killer(ch,victim);
	check_violent(ch,victim);
}

/*
void do_blackjack( CHAR_DATA *ch, char *argument )
{

    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    AFFECT_DATA af;
    int chance;

    if ( (chance = get_skill(ch,gsn_blackjack)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BLACKJACK))
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_blackjack].skill_level[ch->class]))
    {
        send_to_char("Blackjack? Whats that?\n\r",ch);
        return;
    }

    one_argument( argument, arg );

    if (arg[0] == '\0')
    {
        send_to_char("Blackjack who?\n\r",ch);
        return;
    }

    if (is_safe(ch,victim))
        return;

    if ( IS_AFFECTED(victim, AFF_SLEEP) )
    {
        send_to_char("Sorry they are already blackjacked!\n\r",ch);
	return;
    }


    if (ch->fighting != NULL)
    {
        send_to_char("You can't blackjack while fighting!\n\r",ch);
        return;
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "How can you blackjack yourself?\n\r", ch );
        return;
    }

    chance += (ch->level - victim->level);
    check_killer(ch,victim);
    check_violent(ch,victim);
    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    multi_hit( ch, victim, TYPE_UNDEFINED );


    af.where     = TO_AFFECTS;
    af.type      = gsn_sleep;
    af.level     = 1;
    af.duration  = 1;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_BLACKJACK;
    affect_join( victim, &af );


    if ( IS_AWAKE(victim) )
    {
        send_to_char( "You are knocked out COLD!\n\r", victim );
        act( "You whacked $n out cold!", victim, NULL, NULL, TO_ROOM );
        victim->position = POS_SLEEPING;
    }
    return;

}

*/
void do_blackjack( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    AFFECT_DATA af;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_blackjack)) == 0)
    {
	send_to_char("You cant blackjack.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }
    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }


    if (IS_AFFECTED(victim,AFF_SLEEP))
    {
	act("$E's already been blackjacked.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_blackjack].beats );
    if ( number_percent( ) < get_skill(ch,gsn_blackjack)
    || ( get_skill(ch,gsn_blackjack) >= 2 && !IS_AWAKE(victim) ) )
    {
        send_to_char("You are knocked out cold!\n\r",victim);
        check_improve(ch,gsn_blackjack,TRUE,2);

        af.where        = TO_AFFECTS;
        af.type         = gsn_sleep;
        af.level        = 1;
        af.duration     = 1;
        af.location     = APPLY_NONE;
        af.modifier     = 0;
        af.bitvector    = AFF_SLEEP;

	ch->move -= 50;


        affect_to_char(victim,&af);
        victim->position = POS_SLEEPING;

       
        act("$n is knocked out cold!",victim,NULL,NULL,TO_ROOM);
        act("$n wacks you upside the head!",ch,NULL,victim,TO_VICT);	
        check_improve(ch,gsn_blackjack,TRUE,1);       
    }
    else
    {
        send_to_char("You failed horrbly!.\n\r",ch);
        
        check_improve(ch,gsn_blackjack,FALSE,1);
        multi_hit( ch, victim, TYPE_UNDEFINED );     
        WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
        check_killer( ch, victim );
        check_violent( ch, victim );
   
    }
 return;
}


void do_dirt( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_dirt)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK_DIRT))
    ||   (!IS_NPC(ch)
    &&    ch->level < skill_table[gsn_dirt].skill_level[ch->class]))
    {
	send_to_char("You get your feet dirty.\n\r",ch);
	return;
    }

    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't in combat!\n\r",ch);
	    return;
	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (IS_AFFECTED(victim,AFF_BLIND))
    {
	act("$E's already been blinded.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("Very funny.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_NPC(victim) &&
	 victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_DEX);

    /* speed  */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 25;

    /* level */
    chance += (ch->level - victim->level) * 2;

    /* sloppy hack to prevent false zeroes */
    if (chance % 5 == 0)
	chance += 1;

    /* terrain */

    switch(ch->in_room->sector_type)
    {
	case(SECT_INSIDE):		chance -= 20;	break;
	case(SECT_CITY):		chance -= 10;	break;
	case(SECT_FIELD):		chance +=  5;	break;
	case(SECT_FOREST):				break;
	case(SECT_HILLS):				break;
	case(SECT_MOUNTAIN):		chance -= 10;	break;
	case(SECT_WATER_SWIM):		chance  =  0;	break;
	case(SECT_WATER_NOSWIM):	chance  =  0;	break;
	case(SECT_AIR):			chance  =  0;  	break;
	case(SECT_DESERT):		chance += 10;   break;
	case(SECT_UNDERGROUND):		chance += 10;	break;
    }

    if (chance == 0)
    {
	send_to_char("There isn't any dirt to kick.\n\r",ch);
	return;
    }

    /* now the attack */
    if (number_percent() < chance)
    {
	AFFECT_DATA af;
	act("$n is blinded by the dirt in $s eyes!{x",victim,NULL,NULL,TO_ROOM);
	act("$n kicks dirt in your eyes!{x",ch,NULL,victim,TO_VICT);
        damage(ch,victim,number_range(2,5),gsn_dirt,DAM_NONE,FALSE);
	send_to_char("You can't see a thing!{x\n\r",victim);
	check_improve(ch,gsn_dirt,TRUE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);

	af.where	= TO_AFFECTS;
	af.type 	= gsn_dirt;
	af.level 	= ch->level;
	af.duration	= 0;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.bitvector 	= AFF_BLIND;

	affect_to_char(victim,&af);
    }
    else
    {
	damage(ch,victim,0,gsn_dirt,DAM_NONE,TRUE);
	check_improve(ch,gsn_dirt,FALSE,2);
	WAIT_STATE(ch,skill_table[gsn_dirt].beats);
    }
	check_killer(ch,victim);
	check_violent(ch,victim);
}

void do_trip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_trip)) == 0
    ||   (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_TRIP))
    ||   (!IS_NPC(ch) 
	  && ch->level < skill_table[gsn_trip].skill_level[ch->class]))
    {
	send_to_char("Tripping?  What's that?\n\r",ch);
	return;
    }


    if (arg[0] == '\0')
    {
	victim = ch->fighting;
	if (victim == NULL)
	{
	    send_to_char("But you aren't fighting anyone!\n\r",ch);
	    return;
 	}
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
	send_to_char("They aren't here.\n\r",ch);
	return;
    }

    if (is_safe(ch,victim))
	return;

    if (IS_NPC(victim) &&
	 victim->fighting != NULL && 
	!is_same_group(ch,victim->fighting))
    {
	send_to_char("Kill stealing is not permitted.\n\r",ch);
	return;
    }
    
    if (IS_AFFECTED(victim,AFF_FLYING))
    {
	act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim->position < POS_FIGHTING)
    {
	act("$N is already down.",ch,NULL,victim,TO_CHAR);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You fall flat on your face!{x\n\r",ch);
	WAIT_STATE(ch,2 * skill_table[gsn_trip].beats);
	act("$n trips over $s own feet!{x",ch,NULL,NULL,TO_ROOM);
	return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
	act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
	return;
    }

    /* modifiers */

    /* size */
    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 10;  /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= get_curr_stat(victim,STAT_DEX) * 3 / 2;

    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
	chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
	chance -= 20;

    /* level */
    chance -= (ch->level - victim->level * 2);


    /* now the attack */
    if (number_percent() < chance)
    {
	act("$n trips you and you go down!{x",ch,NULL,victim,TO_VICT);
	act("You trip $N and $N goes down!{x",ch,NULL,victim,TO_CHAR);
	act("$n trips $N, sending $M to the ground.{x",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_trip,TRUE,1);

	DAZE_STATE(victim,2 * PULSE_VIOLENCE);
        WAIT_STATE(ch,skill_table[gsn_trip].beats);
	victim->position = POS_RESTING;
	damage(ch,victim,number_range(2, 2 +  2 * victim->size),gsn_trip,
	    DAM_BASH,TRUE);
    }
    else
    {
	damage(ch,victim,0,gsn_trip,DAM_BASH,TRUE);
	WAIT_STATE(ch,skill_table[gsn_trip].beats*2/3);
	check_improve(ch,gsn_trip,FALSE,1);
    } 
	check_killer(ch,victim);
	check_violent(ch,victim);
}



void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
/*
    CHAR_DATA *wch;

    if(IS_SET(wch->act, PLR_ROLEPL))
    {
        send_to_char("Sorry, you can't attack a player in RP Mode!\n\r", ch);
        return;
    }

    if(IS_SET(wch->act, PLR_ROLEP2))
    {
        send_to_char("Sorry, you can't attack a player in RP Mode!\n\r", ch);
        return;
    }
*/
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Kill whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }
    if ( victim == ch )
    {
	send_to_char( "You hit yourself.  Ouch!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if ( is_safe( ch, victim ) )
	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
	act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING )
    {
	send_to_char( "You do the best you can!\n\r", ch );
	return;
    }
    

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    check_violent( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_backstab( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
/*
    CHAR_DATA *wch;

    if(IS_SET(wch->act, PLR_ROLEPL))
    {
        send_to_char("Sorry, you can't attack a player in RP Mode!\n\r", ch);
        return;
    }

    if(IS_SET(wch->act, PLR_ROLEP2))
    {
        send_to_char("Sorry, you can't attack a player in RP Mode!\n\r", ch);
        return;
    }
*/
    one_argument( argument, arg );


    if (ch->fighting != NULL)
    {
	send_to_char("You're facing the wrong end.\n\r",ch);
	return;
    }

    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "How can you sneak up on yourself?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( ( obj = get_eq_char( ch, WEAR_RIGHT ))  == NULL 
	&& ( get_eq_char(ch, WEAR_LEFT) == NULL ))
    {
	send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
	return;
    }

    if ( victim->hit < victim->max_hit / 3)
    {
	act( "{B$N is hurt and suspicious ... you can't sneak up{x.",
	    ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( IS_AFFECTED(victim, AFF_BLACKJACK) )
    {
	act ("{BSorry, {Y$N{B, is ready to be stole from, not slaughtered{x",
	     ch, NULL, victim, TO_CHAR );
	return;
    }    	
    	
    check_killer( ch, victim );
    check_violent( ch, victim );
    WAIT_STATE( ch, skill_table[gsn_backstab].beats );
    if ( number_percent( ) < get_skill(ch,gsn_backstab)
    || ( get_skill(ch,gsn_backstab) >= 2 && !IS_AWAKE(victim) ) )
    {
	check_improve(ch,gsn_backstab,TRUE,1);
	multi_hit( ch, victim, gsn_backstab );
    }
    else
    {
	check_improve(ch,gsn_backstab,FALSE,1);
	damage( ch, victim, 0, gsn_backstab,DAM_NONE,TRUE);
    }

    return;
}

void do_circle( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
 
    one_argument( argument, arg );
 
    if (arg[0] == '\0')
    {
        send_to_char("Circle whom?\n\r",ch);
        return;
    }
 
 
    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }
 
    if ( is_safe( ch, victim ) )
      return;
 
    if (IS_NPC(victim) &&
         victim->fighting != NULL &&
        !is_same_group(ch,victim->fighting))
 
    {
        send_to_char("Kill stealing is not permitted.\n\r",ch);
        return;
    }
 
    if ( ( obj = get_eq_char( ch, WEAR_RIGHT ) ) == NULL)
    {
        send_to_char( "You need to wield a weapon to circle.\n\r", ch );
        return;
    }
 
    if ( ( victim = ch->fighting ) == NULL )
    {
        send_to_char( "You must be fighting in order to circle.\n\r", ch );
        return;
    }
 
    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[gsn_circle].beats );
    if ( number_percent( ) < get_skill(ch,gsn_circle)
    || ( get_skill(ch,gsn_circle) >= 2 && !IS_AWAKE(victim) ) )
    {
        check_improve(ch,gsn_circle,TRUE,1);
        multi_hit( ch, victim, gsn_circle );
    }
    else
    {
        check_improve(ch,gsn_circle,FALSE,1);
        damage( ch, victim, 0, gsn_circle,DAM_NONE,TRUE);
    }
 
    return;
}
 


void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;

    if ( ( victim = ch->fighting ) == NULL )
    {
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
	EXIT_DATA *pexit;
	int door;

	door = number_door( );
	if ( ( pexit = was_in->exit[door] ) == 0
	||   pexit->u1.to_room == NULL
	||   IS_SET(pexit->exit_info, EX_CLOSED)
	||   number_range(0,ch->daze) != 0
	|| ( IS_NPC(ch)
	&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB
	&&   pexit->u1.to_room->sector_type == SECT_PKARENA) ) )
	    continue;

	move_char( ch, door, FALSE );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;

	ch->in_room = was_in;
	if(!IS_NPC(ch) && IS_SET(ch->act, PLR_MOUNTED))
	{
	    act("$n jumped off $s mount and fled!", ch, NULL, NULL, TO_ROOM);
	}
	else
	    act( "$n has fled!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;

	if ( !IS_NPC(ch) )
	{
	    send_to_char( "You flee from combat!\n\r", ch );
	    if( (ch->class == 2) 
	    && (number_percent() < 3*(ch->level/2) ) )
		send_to_char( "You snuck away safely.\n\r", ch);
	    else
	    {
	    	send_to_char( "You lost 10 exp.\n\r", ch); 
	    	gain_exp( ch, -10 );
	    }
    	    if(IS_SET(ch->act, PLR_MOUNTED))
	    {
	    	send_to_char("You left your mount behind.\n\r", ch);
	    	REMOVE_BIT(ch->act, PLR_MOUNTED);
	    	ch->position = POS_STANDING;
	    }
	}

	stop_fighting( ch, TRUE );
	return;
    }

    send_to_char( "PANIC! You couldn't escape!\n\r", ch );
    return;
}



void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Rescue whom?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "What about fleeing instead?\n\r", ch );
	return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if ( ch->fighting == victim )
    {
	send_to_char( "Too late.\n\r", ch );
	return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
	send_to_char( "That person is not fighting right now.\n\r", ch );
	return;
    }


    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( number_percent( ) > get_skill(ch,gsn_rescue))
    {
	send_to_char( "You fail the rescue.\n\r", ch );
	check_improve(ch,gsn_rescue,FALSE,1);
	return;
    }

    act( "{xYou rescue $N!{x",  ch, NULL, victim, TO_CHAR    );
    act( "{x$n rescues you!{x", ch, NULL, victim, TO_VICT    );
    act( "{x$n rescues $N!{x",  ch, NULL, victim, TO_NOTVICT );
    check_improve(ch,gsn_rescue,TRUE,1);

    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );

    check_killer( ch, fch );
    check_violent( ch, fch );
    if(ch->fighting == NULL)
        set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}



void do_kick( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_kick].skill_level[ch->class] )
    {
	send_to_char(
	    "You better leave the martial arts to fighters.\n\r", ch );
	return;
    }

    if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK))
	return;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_kick].beats );
    if ( get_skill(ch,gsn_kick) > number_percent())
    {
	damage(ch,victim,number_range( 1, ch->level ), gsn_kick,DAM_BASH,TRUE);
	check_improve(ch,gsn_kick,TRUE,1);
    }
    else
    {
	damage( ch, victim, 0, gsn_kick,DAM_BASH,TRUE);
	check_improve(ch,gsn_kick,FALSE,1);
    }
	check_killer(ch,victim);
	check_violent(ch,victim);
    return;
}




void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj, *obj2;
    int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;
    int vict_hand, ch_hand;

    hth = 0;

    if ((chance = get_skill(ch,gsn_disarm)) == 0)
    {
	send_to_char( "You don't know how to disarm opponents.\n\r", ch );
	return;
    }

    obj = get_eq_char(ch, WEAR_RIGHT);
    obj2 = get_eq_char(ch, WEAR_LEFT);

    if ( (obj == NULL || (obj != NULL && obj->item_type != ITEM_WEAPON))
    && (obj2 == NULL || (obj2 != NULL && obj2->item_type != ITEM_WEAPON))
    &&   ((hth = get_skill(ch,gsn_hand_to_hand)) == 0
    ||    (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_DISARM))))
    {
	send_to_char( "You must wield a weapon to disarm.\n\r", ch );
	return;
    }

    if(obj != NULL && obj2 != NULL
    && obj->item_type == ITEM_WEAPON && obj2->item_type == ITEM_WEAPON)
    {
        if(get_skill(ch, gsn_right_hand) < get_skill(ch, gsn_left_hand))
            ch_hand = WEAR_LEFT;
        else
            ch_hand = WEAR_RIGHT;
    }
    else if(obj != NULL && obj->item_type == ITEM_WEAPON)
        ch_hand = WEAR_RIGHT;
    else /* must be in left hand */
        ch_hand = WEAR_LEFT;

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    obj = get_eq_char(victim, WEAR_RIGHT);
    obj2 = get_eq_char(victim, WEAR_LEFT);
    if ( (obj == NULL || (obj != NULL && obj->item_type != ITEM_WEAPON))
    && (obj2 == NULL || (obj2 != NULL && obj2->item_type != ITEM_WEAPON)) )
    {
	send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
	return;
    }

    if(obj != NULL && obj->item_type == ITEM_WEAPON)
        vict_hand = WEAR_RIGHT;
    else
        vict_hand = WEAR_LEFT;

    
    /* find weapon skills */
    ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch, ch_hand));
    vict_weapon = get_weapon_skill(victim,get_weapon_sn(victim, vict_hand));
    ch_vict_weapon = get_weapon_skill(ch,get_weapon_sn(victim, vict_hand));

    /* modifiers */

    /* skill */
    if ( get_eq_char(ch,WEAR_RIGHT) == NULL)
	chance = chance * hth/150;
    else
	chance = chance * ch_weapon/100;

    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

    /* dex vs. strength */
    chance += get_curr_stat(ch,STAT_DEX);
    chance -= 2 * get_curr_stat(victim,STAT_STR);

    /* level */
    chance += (ch->level - victim->level) * 2;
 
    /* and now the attack */
    if (number_percent() < chance)
    {
    	WAIT_STATE( ch, skill_table[gsn_disarm].beats );
	disarm( ch, victim );
	check_improve(ch,gsn_disarm,TRUE,1);
    }
    else
    {
	WAIT_STATE(ch,skill_table[gsn_disarm].beats);
	act("{xYou fail to disarm $N.{x",ch,NULL,victim,TO_CHAR);
	act("{x$n tries to disarm you, but fails.{x",ch,NULL,victim,TO_VICT);
	act("{x$n tries to disarm $N, but fails.{x",ch,NULL,victim,TO_NOTVICT);
	check_improve(ch,gsn_disarm,FALSE,1);
    }
    check_killer(ch,victim);
    check_violent(ch,victim);
    return;
}

void do_sap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    AFFECT_DATA af;
    int chance;

    one_argument(argument,arg);

    if ( (chance = get_skill(ch,gsn_sap)) == 0)
    {
        send_to_char("You cant sap just yet.\n\r",ch);
        return;
    }

    if (arg[0] == '\0')
    {
        victim = ch->fighting;
        if (victim == NULL)
        {
            send_to_char("But you aren't in combat!\n\r",ch);
            return;
        }
    }
    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if (IS_AFFECTED(victim,AFF_SLEEP))
    {
        act("$E's already been sapped.",ch,NULL,victim,TO_CHAR);
        return;
    }

    if (victim == ch)
    {
        send_to_char("Very funny.\n\r",ch);
        return;
    }

    if (is_safe(ch,victim))
        return;

    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
    {
        act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
        return;
    }

    WAIT_STATE( ch, skill_table[gsn_sap].beats );
    if ( number_percent( ) < get_skill(ch,gsn_sap)
    || ( get_skill(ch,gsn_sap) >= 2 && !IS_AWAKE(victim) ) )
    {
        send_to_char("You were just sapped out cold!\n\r",victim);
        check_improve(ch,gsn_sap,TRUE,2);

        af.where        = TO_AFFECTS;
        af.type         = gsn_sleep;
        af.level        = 1;
        af.duration     = 1;
        af.location     = APPLY_NONE;
        af.modifier     = 0;
        af.bitvector    = AFF_SLEEP;

        ch->move -= 50;


        affect_to_char(victim,&af);
        victim->position = POS_SLEEPING;


        act("$n was just sapped!",victim,NULL,NULL,TO_ROOM);
        act("$n saps you upside the head!",ch,NULL,victim,TO_VICT);
        check_improve(ch,gsn_sap,TRUE,1);
    }
    else
    {
        send_to_char("You failed horrbly!.\n\r",ch);

        check_improve(ch,gsn_sap,FALSE,1);
        multi_hit( ch, victim, TYPE_UNDEFINED );
        WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
        check_killer( ch, victim );
        check_violent( ch, victim );

    }
 return;
}



void do_surrender( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    if ( (mob = ch->fighting) == NULL )
    {
	send_to_char( "But you're not fighting!\n\r", ch );
	return;
    }
    act( "You surrender to $N!", ch, NULL, mob, TO_CHAR );
    act( "$n surrenders to you!", ch, NULL, mob, TO_VICT );
    act( "$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT );
    stop_fighting( ch, TRUE );

    if ( !IS_NPC( ch ) && IS_NPC( mob ) 
    &&   ( !HAS_TRIGGER( mob, TRIG_SURR ) 
        || !mp_percent_trigger( mob, ch, NULL, NULL, TRIG_SURR ) ) )
    {
	act( "$N seems to ignore your cowardly act!", ch, NULL, mob, TO_CHAR );
	multi_hit( mob, ch, TYPE_UNDEFINED );
    }
}

void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}

void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax: [Char] [Type]\n\r", ch );
	send_to_char( "Types: Skin, Slit, Immolate, Demon, Shatter, Slit, Deheart, Pounce.\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "Suicide is a mortal sin.\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && get_trust( victim ) >= get_trust( ch ) )
    {
	send_to_char( "You failed.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "skin" ) )
    {
    act( "You rip the flesh from $N and send his soul to the fiery depths of hell.", ch, NULL, victim, TO_CHAR );
    act( "Your flesh has been torn from your bones and your bodyless soul now watches your bones incenerate in the fires of hell.", ch, NULL, victim, TO_VICT );
    act( "$n rips the flesh off of $N, releasing his soul into the fiery depths of hell.", ch, NULL, victim, TO_NOTVICT );
	}
/* This one is dedicated to my Head Builder */

	else if ( !str_cmp( arg2, "xwife" ) && get_trust(ch) == MAX_LEVEL )
	{
	act( "You whip out the rotting leg of Colin's x-wife and beat $N to death with it!", ch, NULL, victim, TO_CHAR );
	act( "$n pulls out the rotting leg of Colin's x-wife and beats you to death with it!", ch, NULL, victim, TO_VICT );
	act( "$n pulls out the rotting leg of Colin's x-wife and beats $N to death with it!", ch, NULL, victim, TO_NOTVICT );
	}

	else if ( !str_cmp( arg2, "deheart" ) )
	{
	act( "You rip through $N's chest and pull out $M beating heart in your hand.", ch, NULL, victim, TO_CHAR );
	act( "You feel a sharp pain as $n rips into your chest and pulls out your beating heart in $M hand.", ch, NULL, victim, TO_VICT );
	act( "Specks of blood hit your face as $n rips through $N's chest pulling out $M's beating heart.", ch, NULL, victim, TO_NOTVICT );
	}

    else if ( !str_cmp( arg2, "immolate" ) )
    {
      act( "Your fireball turns $N into a blazing inferno.",  ch, NULL,victim, TO_CHAR    );
      act( "$n releases a searing fireball in your direction.", ch, NULL,victim, TO_VICT    );
      act( "$n points at $N, who bursts into a flaming inferno.",  ch, NULL,victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "shatter" ) )
    {
      act( "You freeze $N with a glance and shatter the frozen corpse into tiny shards.",  ch, NULL, victim, TO_CHAR    );
      act( "$n freezes you with a glance and shatters your frozen body into tiny shards.", ch, NULL, victim, TO_VICT    );
      act( "$n freezes $N with a glance and shatters the frozen body into tiny shards.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "demon" ) )
    {
      act( "You gesture, and a slavering demon appears.  With a horrible grin, the foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_CHAR );
      act( "$n gestures, and a slavering demon appears.  The foul creature turns on you with a horrible grin.   You scream in panic before being eaten alive.",  ch, NULL, victim, TO_VICT );
      act( "$n gestures, and a slavering demon appears.  With a horrible grin, the foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "pounce" ) )
    {
      act( "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...",  ch, NULL, victim, TO_CHAR );
      act( "In a heartbeat, $n rips $s fangs through your throat!  Your blood sprays and pours to the ground as your life ends...", ch, NULL, victim, TO_VICT );
      act( "Leaping suddenly, $n sinks $s fangs into $N's throat.  As blood sprays and gushes to the ground, $n tosses $N's dying body away.",  ch, NULL, victim, TO_NOTVICT );
    }
 
    else if ( !str_cmp( arg2, "slit" ))
    {
      act( "You calmly slit $N's throat.", ch, NULL, victim, TO_CHAR );
      act( "$n reaches out with a clawed finger and calmly slits your throat.", ch, NULL, victim, TO_VICT );
      act( "A claw extends from $n's hand as $M calmly slits $N's throat.", ch, NULL, victim, TO_NOTVICT );
    }

    else
    {
      act( "You slay $N in cold blood!",  ch, NULL, victim, TO_CHAR    );
      act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
      act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    }

    raw_kill(victim);
    return;
}

void do_thrust( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int dam;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_thrust].skill_level[ch->class] )
    {
	send_to_char(
	    "You better leave the martial arts to monks.\n\r", ch );
	return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
	send_to_char( "You aren't fighting anyone.\n\r", ch );
	return;
    }

    if (ch->position == POS_STUNNED)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
        return;
    }

    if ( ( get_eq_char( ch, WEAR_RIGHT ))  == NULL 
	&& ( get_eq_char(ch, WEAR_LEFT) == NULL ))
    {
	send_to_char( "You need to wield a weapon to thrust it.\n\r", ch );
	return;
    }

    if ( !can_see( ch, victim ) )
    {
	send_to_char("You stumble blindly into a wall.\n\r",ch);
    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
	return;
    }

   if(ch->level>80)
	dam = number_range( 1, ch->level+1000 );
   else if(ch->level>50)
	dam = number_range( 1, ch->level+500 );
   else
	dam = number_range( 1, ch->level+250 );
   WAIT_STATE(ch, 1);
    if ( get_skill(ch,gsn_thrust) > number_percent())
    {
	damage(ch,victim,number_range( dam, (ch->level*3.7) ), gsn_thrust,DAM_PIERCE,TRUE);
	check_improve(ch,gsn_thrust,TRUE,1);
    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    WAIT_STATE(ch, 1);
    }
    else
    {
	damage( ch, victim, 0, gsn_thrust,DAM_PIERCE,TRUE);
	check_improve(ch,gsn_thrust,FALSE,1);
    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    }
    return;
}

void do_lunge( CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
/*
    CHAR_DATA *wch;

    if(IS_SET(wch->act, PLR_ROLEPL))
    {
        send_to_char("Sorry, you can't attack a player in RP Mode!\n\r", ch);
        return;
    }

    if(IS_SET(wch->act, PLR_ROLEP2))
    {
        send_to_char("Sorry, you can't attack a player in RP Mode!\n\r", ch);
        return;
    }
*/
    one_argument( argument, arg );

    if (arg[0] == '\0')
    {
	act( "{RLunge at whom?{x.",ch, NULL, victim, TO_CHAR );
        return;
    }


    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "That would be interesting?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if ( ( obj = get_eq_char( ch, WEAR_RIGHT ))  == NULL 
	&& ( get_eq_char(ch, WEAR_LEFT) == NULL ))
    {
	send_to_char( "You need to wield a weapon to lunge.\n\r", ch );
	return;
    }

    if ( victim->hit < 1000 && victim->max_hit > 1000)
    {
	act( "{B$N is {Bhurt and suspicious ... They have their guard up{x.",ch, NULL, victim, TO_CHAR );
	return;
    }

    check_killer( ch, victim );
    check_violent( ch, victim );
    if(chance(95))
    {
    WAIT_STATE( ch, skill_table[gsn_backstab].beats );
    if ( number_percent( ) < get_skill(ch,gsn_lunge)
    || ( get_skill(ch,gsn_lunge) >= 2 && !IS_AWAKE(victim) ) )
    {
	check_improve(ch,gsn_lunge,TRUE,1);
	multi_hit( ch, victim, gsn_lunge );
    }
    else
    {
	check_improve(ch,gsn_lunge,FALSE,1);
	damage( ch, victim, 0, gsn_lunge,DAM_NONE,TRUE);
    }
    }
    else
    {
    WAIT_STATE( ch, skill_table[gsn_backstab].beats/2 );
	act( "{RMiss, Miss,Miss, Miss... Your lunge is horible.{x.",ch, 
NULL, victim, TO_CHAR );
    }
    return;
}

void do_surge( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_surge].skill_level[ch->class] )
    {
	send_to_char(
	    "Your hair stands on end.\n\r", ch );
	return;
    }
    if(ch->hit<500)
	{
	   send_to_char("You head hurts and you feel as if you would pass out, if you continue this action\n\r", ch);
	   return;
	}

    WAIT_STATE( ch, skill_table[gsn_surge].beats );
    if ( get_skill(ch,gsn_surge) > number_percent())
    {
	ch->hit=ch->hit-500;
	ch->mana = UMIN(ch->mana+ch->level+500,ch->max_mana);
	send_to_char("Energy Flows through you.\n\r",ch);
	check_improve(ch,gsn_surge,TRUE,1);
    }
    else
    {
	ch->hit=ch->hit-250;
	send_to_char("Your hair stands on end.\n\r",ch);
	check_improve(ch,gsn_surge,FALSE,1);
    }

    return;
}

void do_layhands( CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    AFFECT_DATA af;
    one_argument( argument, arg );

    if (arg[0] == '\0')
    {
	victim = ch;
    }

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_layhands].skill_level[ch->class] )
    {
	send_to_char(
	    "You pray, but to no avail.\n\r", ch );
	return;
    }


    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if( ch->mana<=500 )
    {
	send_to_char("You are not feeling up to that right now.\n\r", ch);
	return;
    }
    if( IS_AFFECTED2( ch, AFF_LAYHANDS))
    {
	send_to_char("You are trying to call upon the aid of the heavens to much.\n\r", ch);
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_layhands].beats );
    if ( number_percent( ) < get_skill(ch,gsn_layhands))
    {
	send_to_char("Warmth flows through your hands.\n\r",ch);
	send_to_char("You feel refreshed.\n\r",victim);
	victim->hit=victim->max_hit;
	victim->mana=victim->max_mana;
	victim->move=victim->max_move;
	ch->mana=200;
	check_improve(ch,gsn_layhands,TRUE,1);
    }
    else
    {
	send_to_char("You place your hand down, and nothing happens\n\r",ch);
	check_improve(ch,gsn_layhands,FALSE,1);
    }
	af.where	=	TO_AFFECTS2;
	af.type		=	gsn_layhands;
	af.level	=	ch->level;
	af.duration	=	15;
	af.location	=	APPLY_HITROLL;
	af.modifier	=	15;
	af.bitvector	=	AFF_LAYHANDS;
	affect_to_char(ch, &af);
	af.location	=	APPLY_DAMROLL;
	af.modifier	=	15;
	affect_to_char(ch, &af);
	send_to_char("The gods bless you.\n\r",ch);
    return;
}

void do_finesse( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int dam;
    int c;
    int d;

    if ( !IS_NPC(ch)
    &&   ch->level < skill_table[gsn_finesse].skill_level[ch->class] )
    {
	send_to_char(
	    "Your skills with the weapons are not good enough.\n\r", ch );
	return;
    }
    one_argument( argument, arg );

    if (arg[0] == '\0')
    {
	act( "{YFinesse your weapon with whom?{x.",ch, NULL, victim, TO_CHAR );
        return;
    }


    else if ((victim = get_char_room(ch,arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r",ch);
        return;
    }

    if ( victim == ch )
    {
	send_to_char( "That would be interesting?\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim ) )
      return;

    if (ch->position == POS_STUNNED)
    {
        send_to_char("You're still a little woozy.\n\r",ch);
        return;
    }

    if ( ( get_eq_char( ch, WEAR_RIGHT ))  == NULL 
	&& ( get_eq_char(ch, WEAR_LEFT) == NULL ))
    {
	send_to_char( "To have finesse with a weapon, you need to wiled one.\n\r", ch );
	return;
    }

    if ( !can_see( ch, victim ) )
    {
	send_to_char("You flail your weapon around the air.\n\r",ch);
    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
	return;
    }

    WAIT_STATE( ch, skill_table[gsn_finesse].beats );

	dam = number_range( 1, ch->level+250 );
    if ( get_skill(ch,gsn_finesse) > number_percent())
    {
	c = number_range(1, 5);
	d = GET_DAMROLL(ch);
	for(;c>0;c--)
	   {
	      dam = (number_range(1+ch->damroll,ch->level+ch->damroll));
	      dam = (dam + (ch->damroll));
	      damage( ch, victim, dam, gsn_finesse, DAM_PIERCE,TRUE);
	      check_improve(ch,gsn_finesse,TRUE,1);
	   }
    }
    else
    {
	damage( ch, victim, 0, gsn_finesse,DAM_PIERCE,TRUE);
	check_improve(ch,gsn_finesse,FALSE,1);
        WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    }
    return;
}

