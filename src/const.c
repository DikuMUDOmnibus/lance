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
*	By using this code, you have agreed to follow the terms of the	   
*
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"


/* item type list */
const struct item_type		item_table	[]	=
{
    {	ITEM_LIGHT,	"light"		},
    {	ITEM_SCROLL,	"scroll"	},
    {	ITEM_WAND,	"wand"		},
    {   ITEM_STAFF,	"staff"		},
    {   ITEM_WEAPON,	"weapon"	},
    {   ITEM_TREASURE,	"treasure"	},
    {   ITEM_ARMOR,	"armor"		},
    {	ITEM_POTION,	"potion"	},
    {	ITEM_CLOTHING,	"clothing"	},
    {   ITEM_FURNITURE,	"furniture"	},
    {	ITEM_TRASH,	"trash"		},
    {	ITEM_CONTAINER,	"container"	},
    {	ITEM_DRINK_CON, "drink"		},
    {	ITEM_KEY,	"key"		},
    {	ITEM_FOOD,	"food"		},
    {	ITEM_MONEY,	"money"		},
    {	ITEM_BOAT,	"boat"		},
    {	ITEM_CORPSE_NPC,"npc_corpse"	},
    {	ITEM_CORPSE_PC,	"pc_corpse"	},
    {   ITEM_FOUNTAIN,	"fountain"	},
    {	ITEM_PILL,	"pill"		},
    {	ITEM_PROTECT,	"protect"	},
    {	ITEM_MAP,	"map"		},
    {	ITEM_PORTAL,	"portal"	},
    {	ITEM_WARP_STONE,"warp_stone"	},
    {	ITEM_ROOM_KEY,	"room_key"	},
    {	ITEM_GEM,	"gem"		},
    {	ITEM_JEWELRY,	"jewelry"	},
    {   ITEM_JUKEBOX,	"jukebox"	},
    {	ITEM_PARCHMENT,	"parchment"	},
    {	ITEM_PEN,	"pen"		},
    {	ITEM_SPELL,	"spellcomp"	},
    {   0,		NULL		}
};


/* weapon selection table */
const	struct	weapon_type	weapon_table	[]	=
{
   { "sword",	OBJ_VNUM_SCHOOL_SWORD,	WEAPON_SWORD,	&gsn_sword	},
   { "mace",	OBJ_VNUM_SCHOOL_MACE,	WEAPON_MACE,	&gsn_mace 	},
   { "dagger",	OBJ_VNUM_SCHOOL_DAGGER,	WEAPON_DAGGER,	&gsn_dagger	},
   { "axe",	OBJ_VNUM_SCHOOL_AXE,	WEAPON_AXE,	&gsn_axe	},
   { "staff",	OBJ_VNUM_SCHOOL_STAFF,	WEAPON_SPEAR,	&gsn_spear	},
   { "flail",	OBJ_VNUM_SCHOOL_FLAIL,	WEAPON_FLAIL,	&gsn_flail	},
   { "whip",	OBJ_VNUM_SCHOOL_WHIP,	WEAPON_WHIP,	&gsn_whip	},
   { "polearm",	OBJ_VNUM_SCHOOL_POLEARM,WEAPON_POLEARM,	&gsn_polearm	},
   { "lance",	OBJ_VNUM_SCHOOL_LANCE,	WEAPON_LANCE,	&gsn_lance	},
   { NULL,	0,				0,	NULL		}
};

const struct weight_type weight_table [] =
{
{ "slender", WEIGHT_SLENDER, 1 },
{ "slim", WEIGHT_SLIM, 1 },
{ "average", WEIGHT_AVERAGE, 1 },
{ "stocky", WEIGHT_STOCKY, 1 },
{ "heavy", WEIGHT_HEAVY, 1 },
{ NULL, 0, 0 }
};

const struct eye_type eye_table [] =
{
{ "blue", EYE_BLUE, 1 },
{ "green", EYE_GREEN, 1 },
{ "brown", EYE_BROWN, 1 },
{ NULL, 0, 0 }
};

const struct height_type height_table [] =
{
{ "short", HEIGHT_SHORT, 1 },
{ "midget", HEIGHT_MIDGET, 1 },
{ "average",HEIGHT_AVERAGE, 1 },
{ "tall", HEIGHT_TALL, 1 },
{ "giant", HEIGHT_GIANT, 1 },
{ NULL, 0, 0 }
};

const struct hair_type hair_table [] =
{
{ "blond", HAIR_BLOND, 1 },
{ "red", HAIR_RED, 1 },
{ "black",HAIR_BLACK, 1 },
{ NULL, 0, 0 }
};
 
/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
   {    "on",           WIZ_ON,         IM },
   {    "prefix",	WIZ_PREFIX,	IM },
   {    "ticks",        WIZ_TICKS,      IM },
   {    "logins",       WIZ_LOGINS,     IM },
   {    "sites",        WIZ_SITES,      L4 },
   {    "links",        WIZ_LINKS,      L7 },
   {	"newbies",	WIZ_NEWBIE,	IM },
   {	"spam",		WIZ_SPAM,	L5 },
   {    "deaths",       WIZ_DEATHS,     IM },
   {    "resets",       WIZ_RESETS,     L4 },
   {    "mobdeaths",    WIZ_MOBDEATHS,  L4 },
   {    "flags",	WIZ_FLAGS,	L5 },
   {	"penalties",	WIZ_PENALTIES,	L5 },
   {	"saccing",	WIZ_SACCING,	L5 },
   {	"levels",	WIZ_LEVELS,	IM },
   {	"load",		WIZ_LOAD,	L2 },
   {	"restore",	WIZ_RESTORE,	L2 },
   {	"snoops",	WIZ_SNOOPS,	L2 },
   {	"switches",	WIZ_SWITCHES,	L2 },
   {	"secure",	WIZ_SECURE,	L1 },
   {	"posts",	WIZ_POSTS,	IM },
   {	"multi",	WIZ_MULTI,	L2 },
   {	NULL,		0,		0  }
};

/* attack table  -- not very organized :( */
const 	struct attack_type	attack_table	[MAX_DAMAGE_MESSAGE]	=
{
    { 	"none",		"hit",		-1		},  /*  0 */
    {	"slice",	"slice", 	DAM_SLASH	},	
    {   "stab",		"stab",		DAM_PIERCE	},
    {	"slash",	"slash",	DAM_SLASH	},
    {	"whip",		"whip",		DAM_SLASH	},
    {   "claw",		"claw",		DAM_SLASH	},  /*  5 */
    {	"blast",	"blast",	DAM_BASH	},
    {   "pound",	"pound",	DAM_BASH	},
    {	"crush",	"crush",	DAM_BASH	},
    {   "grep",		"grep",		DAM_SLASH	},
    {	"bite",		"bite",		DAM_PIERCE	},  /* 10 */
    {   "pierce",	"pierce",	DAM_PIERCE	},
    {   "suction",	"suction",	DAM_BASH	},
    {	"beating",	"beating",	DAM_BASH	},
    {   "digestion",	"digestion",	DAM_ACID	},
    {	"charge",	"charge",	DAM_BASH	},  /* 15 */
    { 	"slap",		"slap",		DAM_BASH	},
    {	"punch",	"punch",	DAM_BASH	},
    {	"wrath",	"wrath",	DAM_ENERGY	},
    {	"magic",	"magic",	DAM_ENERGY	},
    {   "divine",	"divine power",	DAM_HOLY	},  /* 20 */
    {	"cleave",	"cleave",	DAM_SLASH	},
    {	"scratch",	"scratch",	DAM_PIERCE	},
    {   "peck",		"peck",		DAM_PIERCE	},
    {   "peckb",	"peck",		DAM_BASH	},
    {   "chop",		"chop",		DAM_SLASH	},  /* 25 */
    {   "sting",	"sting",	DAM_PIERCE	},
    {   "smash",	 "smash",	DAM_BASH	},
    {   "shbite",	"shocking bite",DAM_LIGHTNING	},
    {	"flbite",	"flaming bite", DAM_FIRE	},
    {	"frbite",	"freezing bite", DAM_COLD	},  /* 30 */
    {	"acbite",	"acidic bite", 	DAM_ACID	},
    {	"chomp",	"chomp",	DAM_PIERCE	},
    {  	"drain",	"life drain",	DAM_NEGATIVE	},
    {   "thrust",	"thrust",	DAM_PIERCE	},
    {   "slime",	"slime",	DAM_ACID	},
    {	"shock",	"shock",	DAM_LIGHTNING	},
    {   "thwack",	"thwack",	DAM_BASH	},
    {   "flame",	"flame",	DAM_FIRE	},
    {   "chill",	"chill",	DAM_COLD	},
    {   "irslash",	"iron slash",   DAM_IRON	},
    {   "sislash",	"silver slash", DAM_SILVER	},
    {   NULL,		NULL,		0		}
};

/* race table */
const 	struct	race_type	race_table	[]		=
{
/*
    {
	name,		pc_race?,
	act bits,act2 bits,	aff_by bits,	off bits,
	imm,		res,		vuln,
	form,		parts 
    },
*/
    { "unique",		FALSE, 0, 0, 0, 0, 0, 0, 0, 0, 0 },

    { 
	"human",		TRUE, 
	0,	0,	0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
 	"silvanesti",		TRUE,
	0,	0,	AFF_INFRARED,	0,
	0,		0,		VULN_IRON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
 	"qualinesti",		TRUE,
	0,	0,	AFF_INFRARED,	0,
	0,		0,		VULN_IRON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
 	"kagonesti",		TRUE,
	0,	0,	AFF_INFRARED,	0,
	0,		RES_COLD,	VULN_IRON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    }, 

    {
 	"dimernesti",		TRUE,
	0,	0,	AFF_INFRARED,	0,
	0,		0,		VULN_IRON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
 	"dargonesti",		TRUE,
	0,	0,	AFF_INFRARED,	0,
	0,		0,		VULN_IRON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
 	"half-elf",		TRUE,
	0,	0,	AFF_INFRARED,	0,
	0,		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"hylar",		TRUE,
	0,	0,	AFF_INFRARED,	0,
	0,		RES_POISON|RES_DISEASE, VULN_COLD|VULN_DROWNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"neidar",		TRUE,
	0,	0,	AFF_INFRARED,	0,
	0,		RES_POISON|RES_DISEASE, VULN_COLD|VULN_DROWNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"aghar",		TRUE,
	0,	0,	AFF_INFRARED,	0,
	0,		RES_POISON|RES_DISEASE, VULN_COLD|VULN_DROWNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"daergar",		TRUE,
	0,	0,	AFF_INFRARED,	0,
	0,		RES_POISON|RES_DISEASE, VULN_COLD|VULN_DROWNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },
    
    {
	"gnome",		TRUE,
	0,	0,	0,		0,
	0,		RES_MAGIC,	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"kender",		TRUE,
	0,	0,	0,		0,
	0,		0,		VULN_POISON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
    	"minotaur",		TRUE,
    	0,	0,	0,		0,
    	0,		RES_COLD,	VULN_FIRE,
    	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
    	"Kapak",		TRUE,
    	0,	0,	0,		0,
    	0,		0,		0,
    	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
       "Baaz",			TRUE,
	0,	0,	0,		0,
	0,		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"Bozak",		TRUE,
	0,	0,	0,		0,
	0,		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"Sivak",		TRUE,
	0,	0,	0,		0,
	0,		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
        "Aurak",		TRUE,
	0,	0,	0,		0,
	0,		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },
    {
       "Goblin",               TRUE,
        0,      0,      0,              0,
	0,		AFF_DARK_VISION, 0,  
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },
   
    {
       "HobGoblin",        	TRUE,
	0,	0,	0,			0,
	0,		AFF_DARK_VISION,	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
       "Ogre",			TRUE,
	0,	0,	0,		0,
	0,	0,	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
       "thanoi",			TRUE,
	0,	0,	0,		0,
	0,	0,	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },


    {	"bat",			FALSE,
	0,	0,	AFF_FLYING|AFF_DARK_VISION,	OFF_DODGE|OFF_FAST,
	0,		0,		VULN_LIGHT,
	A|G|V,		A|C|D|E|F|H|J|K|P
    },

    {
	"bear",			FALSE,
	0,	0,	0,		OFF_CRUSH|OFF_DISARM|OFF_BERSERK,
	0,		RES_BASH|RES_COLD,	0,
	A|G|V,		A|B|C|D|E|F|H|J|K|U|V
    },

    {
	"cat",			FALSE,
	0,	0,	AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|U|V
    },

    {
	"centipede",		FALSE,
	0,		AFF_DARK_VISION,	0,
	0,		RES_PIERCE|RES_COLD,	VULN_BASH,
 	A|B|G|O,		A|C|K	
    },

    {
	"dog",			FALSE,
	0,	0,	0,		OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|U|V
    },

    {
	"doll",			FALSE,
	0,	0,	0,		0,
	IMM_COLD|IMM_POISON|IMM_HOLY|IMM_NEGATIVE|IMM_MENTAL|IMM_DISEASE
	|IMM_DROWNING,	RES_BASH|RES_LIGHT,
	VULN_SLASH|VULN_FIRE|VULN_ACID|VULN_LIGHTNING|VULN_ENERGY,
	E|J|M|cc,	A|B|C|G|H|K
    },

    { 	"dragon", 		FALSE, 
	0, 	0,		AFF_INFRARED|AFF_FLYING,	0,
	0,			RES_FIRE|RES_BASH|RES_CHARM, 
	VULN_PIERCE|VULN_COLD,
	A|H|Z,		A|C|D|E|F|G|H|I|J|K|P|Q|U|V|X
    },

    {
	"fido",			FALSE,
	0,	0,	0,		OFF_DODGE|ASSIST_RACE,
	0,		0,			VULN_MAGIC,
	A|B|G|V,	A|C|D|E|F|H|J|K|Q|V
    },		
   
    {
	"fox",			FALSE,
	0,	0,	AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|V
    },

    {
	"goblin",		FALSE,
	0,	0,	AFF_INFRARED,	0,
	0,		RES_DISEASE,	VULN_MAGIC,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"hobgoblin",		FALSE,
	0,	0,	AFF_INFRARED,	0,
	0,		RES_DISEASE|RES_POISON,	0,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K|Y
    },

    {
	"kobold",		FALSE,
	0,	0,	AFF_INFRARED,	0,
	0,		RES_POISON,	VULN_MAGIC,
	A|B|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|Q
    },

    {
	"lizard",		FALSE,
	0,	0,	0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|X|cc,	A|C|D|E|F|H|K|Q|V
    },

    {
	"modron",		FALSE,
	0,	0,	AFF_INFRARED,		ASSIST_RACE|ASSIST_ALIGN,
	IMM_CHARM|IMM_DISEASE|IMM_MENTAL|IMM_HOLY|IMM_NEGATIVE,
			RES_FIRE|RES_COLD|RES_ACID,	0,
	H,		A|B|C|G|H|J|K
    },

    {
	"orc",			FALSE,
	0,	0,	AFF_INFRARED,	0,
	0,		RES_DISEASE,	VULN_LIGHT,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {
	"pig",			FALSE,
	0,	0,	0,		0,
	0,		0,		0,
	A|G|V,	 	A|C|D|E|F|H|J|K
    },	

    {
	"rabbit",		FALSE,
	0,	0,	0,		OFF_DODGE|OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K
    },
    
    {
	"school monster",	FALSE,
	ACT_NOALIGN,	0,	0,		0,
	IMM_CHARM|IMM_SUMMON,	0,		VULN_MAGIC,
	A|M|V,		A|B|C|D|E|F|H|J|K|Q|U
    },	

    {
	"snake",		FALSE,
	0,	0,	0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|X|Y|cc,	A|D|E|F|K|L|Q|V|X
    },
 
    {
	"song bird",		FALSE,
	0,	0,	AFF_FLYING,		OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|W,		A|C|D|E|F|H|K|P
    },

    {
	"troll",		FALSE,
	0,	0,	AFF_REGENERATION|AFF_INFRARED|AFF_DETECT_HIDDEN,
	OFF_BERSERK,
 	0,	RES_CHARM|RES_BASH,	VULN_FIRE|VULN_ACID,
	A|B|H|M|V,		A|B|C|D|E|F|G|H|I|J|K|U|V
    },

    {
	"water fowl",		FALSE,
	0,	0,	AFF_SWIM|AFF_FLYING,	0,
	0,		RES_DROWNING,		0,
	A|G|W,		A|C|D|E|F|H|K|P
    },		
  
    {
	"wolf",			FALSE,
	0,	0,	AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,	
	A|G|V,		A|C|D|E|F|J|K|Q|V
    },

    {
	"wyvern",		FALSE,
	0,	0,	AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN,
	OFF_BASH|OFF_FAST|OFF_DODGE,
	IMM_POISON,	0,	VULN_LIGHT,
	A|B|G|Z,		A|C|D|E|F|H|J|K|Q|V|X
    },

    {
	"unique",		FALSE,
	0,	0,	0,		0,
	0,		0,		0,		
	0,		0
    },


    {
	NULL, 0, 0, 0, 0, 0, 0, 0
    }
};

const	struct	pc_race_type	pc_race_table	[]	=
{
    { "null race", 0, { 100, 100, 100, 100 }, "",
      { "" }, { 13, 13, 13, 13, 13 }, { 14, 14, 14, 14, 14 },	{ 18, 18, 18, 18, 18 }, 0 },
 
/*
    {
	"race name", 	points,	{ class multipliers }, "Race Description",
	{ bonus skills },
	{ Minimum starting stats },		{ Maximum starting stats },		{ max stats },		size 
    },
*/
    {
	"human",	3,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
	"Jack of all trades, Master of none",
	{ "human" },
	{ 13, 13, 13, 13, 13 },	{ 15, 15, 15, 15, 15 },	{ 22, 22, 22, 22, 22 },	SIZE_MEDIUM, 0, A
    },

    { 	
	"silvanesti",	6,	{ 100, 100, 100, 100, 100, 100, 100, 100 }, 
	"Elves with many magic users",
	{ "meditation, silvanesti" },
	{ 11, 15, 15, 13, 11 },	{ 13, 17, 17, 15, 13 },	{ 20, 25, 25, 21, 21 }, SIZE_MEDIUM, 110, B 
    },

    { 	
	"qualinesti",	3,	{ 100, 100, 100, 100, 100, 100, 100, 100 }, 
	"More rounded elves",
	{ "qualinesti" },
	{ 12, 14, 14, 13, 12 },	{ 14, 16, 16, 15, 14 },	{ 21, 23, 24, 22, 21 }, SIZE_MEDIUM, 110, C
    },

    { 	
	"kagonesti",	5,	{ 100, 100, 100, 100, 100, 100, 100, 100 }, 
	"Wild Elves, dislikes industry",
	{ "sneak, kagonesti" },
	{ 14, 12, 12, 14, 13 },	{ 16, 14, 14, 16, 15 },	{ 23, 21, 21, 24, 22 }, SIZE_MEDIUM, 110, D
    },

    { 	
	"dimernesti",	3,	{ 100, 100, 100, 100, 100, 100, 100, 100 }, 
	"Shoal Elves, Hgh Sea Elves",
	{ "dimernesti" },
	{ 12, 13, 13, 15, 12 },	{ 14, 15, 15, 17, 14 },	{ 21, 22, 23, 24, 21 }, SIZE_MEDIUM, 110, E
    },

    { 	
	"dargonesti",	3,	{ 100, 100, 100, 100, 100, 100, 100, 100 }, 
	"Deep Elves, Grey Sea Elves",
	{ "dargonesti" },
	{ 12, 13, 13, 15, 12 },	{ 14, 15, 15, 27, 14 },	{ 21, 22, 23, 24, 21 }, SIZE_MEDIUM, 110, F
    },

    { 	
	"half-elf",	3,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
	"Half elf, half human",
	{ "" },
	{ 12, 14, 14, 13, 12 },	{ 14, 16, 16, 15, 14 },	{ 21, 23, 23, 23, 21 }, SIZE_MEDIUM, 98, G
    },

    {
	"hylar",	5,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
	"Mountain dwarf",
	{ "berserk, hylar" },
	{ 15, 11, 13, 10, 16 },	{ 17, 13, 15, 12, 18 },	{ 24, 20, 22, 20, 25 }, SIZE_SMALL, 50, H
    },

    {
	"neidar",	5,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
	"Hill dwarf",
	{ "fast healing, neidar" },
	{ 14, 12, 14, 10, 15 },	{ 16, 14, 16, 12, 17 },	{ 23, 21, 23, 20, 24 }, SIZE_SMALL, 50, I
    },

    {
	"aghar",	0,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
	"Gully dwarf, no redeemable qualities",
	{ "sneak, hide, aghar" },
	{ 14, 12, 12, 14, 13 },	{ 16, 14, 14, 16, 15},	{ 23, 21, 20, 23, 22 }, SIZE_SMALL, 50, J
    },
    
    {   "daergar",   3,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
	"Dark Dwarves, Still afraid of water",
	{ "human" },
	{ 14, 13, 13, 12, 14 },	{ 16, 15, 15, 13, 16 },	{24, 22, 22, 23, 24 }, SIZE_SMALL, 10, V
    },

    {
	"gnome",	3,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
	"Mechanics through and through",
	{ "gnome" },
	{ 13, 14, 12, 13, 13 },	{ 15, 16, 14, 15, 15 },	{ 20, 24, 24, 20, 24 }, SIZE_SMALL, 30, K
    },

    {
	"kender",	6,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
	"Child-like in size and nature",
	{ "steal, kender" },
	{ 12, 13, 12, 16, 12 },	{ 14, 15, 14, 18, 14 },	{ 21, 22, 22, 25, 21 }, SIZE_SMALL, 30, L
    },
    
    {
	"minotaur",	7,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
	"Natural warriors, with a sense of honor",
	{ "fast healing, minotaur" },
	{ 16, 11, 13, 10, 15 },	{ 18, 13, 15, 12, 17 },	{ 25, 20, 22, 20, 24 }, SIZE_LARGE, 20, M
    },
    
    {
    	"kapak",	5,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
    	"Scaled Magical Construct, Assassins",
    	{ "dracon" },
    	{ 15, 11, 11, 13, 15 },	{ 17, 13, 13, 15, 17}, {25, 20, 20, 20, 25 }, SIZE_LARGE, 10, N
    },
    
    {
	"baaz",		5,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
        "Scaled Magical Construct, Common drone",
        { "dracon" },
        { 13, 11, 11, 13, 14 },	{ 15, 13, 13, 15, 16 }, {24, 20, 20, 21, 25 }, SIZE_LARGE, 10, O
    },

    {   "bozak",	5,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
        "Scaled Magical Construct, magic users",
	{ "dracon, meditation" },
	{ 11, 15, 15, 12, 12 },	{ 13, 17, 17, 14, 14 }, {22, 25, 24, 21, 22 }, SIZE_LARGE, 10, P
    },

    {
        "sivak",	5,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
        "Scaled Magical Construct, Elite troops",
	{ "dracon" },
	{ 15, 11, 11, 13, 15 },	{ 17, 13, 13, 15, 17 }, {25, 21, 21, 22, 25 }, SIZE_LARGE, 10, Q
    },
   
    {
	"aurak",	5,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
	"Scaled Magical Construct, wingless type",
	{ "dracon" },
	{ 15, 15, 11, 11, 13 },	{ 17, 17, 13, 13, 15 }, {25, 23, 23, 20, 25 }, SIZE_LARGE, 10, R
    },

    {
        "goblin",       6,      { 100, 100, 100, 100, 100, 100, 100, 100 },
	"Sly thieves and warrior, wee bit dumb",
	{ "sneak" },
	{ 14, 13, 11, 13, 14 },	{ 16, 15, 13, 15, 16 }, {23, 22, 21, 25, 22 }, SIZE_SMALL, 10, S
    },   
   
    {
        "hobgoblin",	6,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
	"cousins of goblins, larger and smarter",
	{ "sneak, hide" },
	{ 15, 11, 13, 13, 13 },	{ 17, 13, 15, 15, 15 }, {25, 22, 22, 21, 23 }, SIZE_LARGE, 10, T
    },

    {
	"Ogre",       7,	{ 100, 100, 100, 100, 100, 100, 100, 100 },
	"Ugly and dumb, but makes a mean warrior",
	{ "bash" },
	{ 17, 11, 11, 14, 12 },	{ 19, 13, 13, 16, 14 }, {25, 20, 20, 22, 25 }, SIZE_LARGE, 10, U
    },

    {
	"thanoi",     7,     { 100, 100, 100, 100, 100, 100, 100, 100 },
        "It's the Walrus Man from the arctic!",
	{ "human" },
	{ 15, 11, 11, 12, 16 },	{ 17, 13, 13, 14, 18 },  {25, 19, 20, 21, 25 }, SIZE_LARGE, 10, R
    }
   
};

	
      	

/*
 * Class table.
 */
const	struct	class_type	class_table	[MAX_CLASS]	=
{
    {
	"mage", "Mag",  STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 0 },  75,  20, 6,  6,  8, TRUE,
	"Trained in the magical arts",
	"mage basics", "mage default",
	NO_KENDER|NO_MINO|NO_GOBLIN|NO_BAAZ|NO_KAPAK|NO_SIVAK|NO_OGRE,
	0, A, 0
    },

    {
	"cleric", "Cle",  STAT_WIS,  OBJ_VNUM_SCHOOL_MACE,
	{ 0 },  75,  20, 2,  7, 10, TRUE,
	"Healers, priests and priestesses",
	"cleric basics", "cleric default",
	NO_OGRE|NO_GOBLIN|NO_BAAZ|NO_KAPAK|NO_SIVAK,
	0, B, 0
    },

    {
	"thief", "Thi",  STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 0 },  75,  20,  -4,  11, 17, FALSE,
	"Trained to steal and be silent",
	"thief basics", "thief default",
        NO_SILV|NO_QUAL|NO_KAGO|NO_DARGO|NO_DIMER|NO_MINO|NO_GNOME|NO_OGRE|NO_BOZAK|NO_AURAK,
        0, C, 0
    },

    {
	"warrior", "War",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
	{ 0 },  75,  20,  -10,  15, 23, FALSE,
	"Trained in the martial arts",
	"warrior basics", "warrior default",
	NO_GNOME|NO_BOZAK|NO_AURAK,
	0, D, 0
    },

    {
	"paladin", "Pal",  STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
	{ 0 }, 	 75,  20,  -8,   10,  17, TRUE,
	"Warriors that draw power from their god",
	"paladin basics", "paladin default",
	NO_AGHAR|NO_KENDER|NO_GNOME|NO_GOBLIN|NO_HOBGOBLIN|NO_OGRE|NO_BAAZ|NO_BOZAK|NO_KAPAK|NO_AURAK|NO_SIVAK|NO_MINO,
	0, E, 0
    },

    {
    	"ranger", "Ran",   STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
    	{ 0, 0 }, 	 75,  20,  -7,   9,  14, TRUE,
    	"Users of nature",
    	"ranger basics", "ranger default",
    	NO_KENDER|NO_AGHAR|NO_GNOME|NO_OGRE|NO_GOBLIN|NO_BAAZ|NO_KAPAK|NO_BOZAK|NO_AURAK|NO_MINO,
    	0, F, 0
    },

    {
    	"templar", "Tem",  STAT_WIS,  OBJ_VNUM_SCHOOL_MACE,
    	{ 0 },  75,  20,  -1,   9,  12, TRUE,
    	"Clerics trained to do battle for their god",
    	"cleric basics", "cleric default",
    	NO_AGHAR|NO_KENDER|NO_OGRE|NO_GOBLIN|NO_AURAK|NO_HOBGOBLIN|NO_BAAZ|NO_KAPAK|NO_BOZAK|NO_SIVAK|NO_MINO,
    	0, G, 0
    },

    {
    	"bmage", "Bmg",   STAT_INT,  OBJ_VNUM_SCHOOL_DAGGER,
    	{ 0 },  75,  20,  3,   8,  10, TRUE,
    	"Mages trained in the arts of battle",
    	"mage basics", "mage default",
    	NO_AGHAR|NO_KENDER|NO_MINO|NO_GOBLIN|NO_OGRE|NO_HOBGOBLIN|NO_BAAZ|NO_SIVAK|NO_KAPAK,
    	0, H, 0
    },

   {
       "archmg", "Amg",	 STAT_INT,   OBJ_VNUM_SCHOOL_DAGGER,
	{ 0 },  75,  20,  6,  6,  8, TRUE,
	"Master Mages",
	"mage basics", "mage default",
	0,
	NO_THIEF|NO_CLERIC|NO_WARRIOR|NO_TEMPLAR|NO_BMAGE|NO_RANGER|NO_PALADIN,
	I, 1
   },

   {
       "warwiz", "Wwz", STAT_INT,   OBJ_VNUM_SCHOOL_DAGGER,
	{ 0 },  75,  20,  3,  8,  10, TRUE,
	"Advanced Battle Mages",
	"mage basics", "mage default",
	0,
	NO_MAGE|NO_THIEF|NO_CLERIC|NO_WARRIOR|NO_TEMPLAR|NO_RANGER|NO_PALADIN,
	J, 1
   },

   {
	"avenger", "Hav", STAT_STR,   OBJ_VNUM_SCHOOL_SWORD,
	{ 0 },  75,  20,  -8,  10,  17, TRUE,
	"Master Paladins",
	"paladin basics", "paladin default",
	0,
	NO_MAGE|NO_THIEF|NO_CLERIC|NO_WARRIOR|NO_TEMPLAR|NO_RANGER|NO_BMAGE,
	K, 1
   },

   {
         "druid", "Dru", STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 0 },  75,  20,  -7,  9,  14,  TRUE,
	"Master of Nature",
	"ranger basics",  "ranger default",
	0,
	NO_MAGE|NO_THIEF|NO_CLERIC|NO_WARRIOR|NO_TEMPLAR|NO_PALADIN|NO_BMAGE,
	L, 1
   },

   {
	"assassin",  "Asn", STAT_DEX,  OBJ_VNUM_SCHOOL_DAGGER,
	{ 0 },  75,  20,  -4,  11,  17,  FALSE,
	"Master cutthroats and thieves",
	"thief basics",  "thief default",
	0,
	NO_MAGE|NO_RANGER|NO_CLERIC|NO_WARRIOR|NO_TEMPLAR|NO_PALADIN|NO_BMAGE,
	M, 1
   },

   {
	"bishop", "Bis", STAT_WIS,  OBJ_VNUM_SCHOOL_MACE,
	{ 0 },  75,  20,  2,  7,  10,  TRUE,
	"Highest order of Preist/Preistess",
	"cleric basics", "cleric default",
	0,
	NO_MAGE|NO_RANGER|NO_THIEF|NO_WARRIOR|NO_TEMPLAR|NO_PALADIN|NO_BMAGE,
	N, 1
   },

   {
        "zealot", "Zea", STAT_STR,  OBJ_VNUM_SCHOOL_MACE,
	{ 0 },  75,  20,  -1,  9,  12,  TRUE,
	"Fanatical Templars and Clerics",
	"cleric basics", "cleric default",
	0,
	NO_MAGE|NO_RANGER|NO_THIEF|NO_WARRIOR|NO_CLERIC|NO_PALADIN|NO_BMAGE,
	O, 1
   },

   {
	"cavalier", "Cav", STAT_STR,  OBJ_VNUM_SCHOOL_SWORD,
	{ 0 }, 75,  20,  -10,  15,  23,  FALSE,
	"Noble, Heroic warriors",
	"warrior basics", "warrior default",
	0,
	NO_MAGE|NO_RANGER|NO_THIEF|NO_TEMPLAR|NO_CLERIC|NO_PALADIN|NO_BMAGE,
	P, 1
   }

};



/*
 * Titles.
 */

char *	const			title_table	[MAX_CLASS][MAX_LEVEL+1][2] =
{
    {
	{ "Man",			"Woman"				},
	{ "Apprentice of Magic",	"Apprentice of Magic"		},
    },

    {
	{ "Man",			"Woman"				},
	{ "Believer",			"Believer"			},
    },

    {
	{ "Man",			"Woman"				},
	{ "Pilferer",			"Pilferess"			},
    },

    {
	{ "Man",			"Woman"				},
	{ "Swordpupil",			"Swordpupil"			},
    },

    {
        { "Man",                        "Woman"                         },
        { "Champion",                   "Lady Champion"			},
    },
    
    {
    	{ "Man",			"Woman"				},
    	{ "Woodsman",			"Woodswoman"			},
    },
    
    {
    	{ "Man",			"Woman"				},
    	{ "Believer",			"Believer"			},
    },
    
    {
    	{ "Man",			"Woman"				},
    	{ "Apprentice of Magic",	"Apprentice of Magic"		},
    },

    {
	{ "Man",			"Woman"				},
	{ "Magius",			"Magius"			},
    },
   
    {
	{ "Man",			"Woman"				},
	{ "War Wizard",			"War Wizard"			},
    },
    
    {
	{ "Man",			"Woman"				},
	{ "Hero",			"Heroine"			},
    },

    {
	{ "Man",			"Woman"				},
	{ "Master of the Forests",	"Mistress of the Forests"	},
    },

    {
	{ "Man",			"Woman"				},
	{ "Assasin",			"Assasin"			},
    },

    {
	{ "Man",			"Woman"				},
	{ "Blessed",			"Blessed"			},
    },

    {
	{ "Man",			"Woman"				},
	{ "Fanatic",			"Fanatic"			},
    },

    {
        { "Man",		        "Woman"				},
        { "Noble",			"Noblewoman"			},
    }, 

};



/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[26]		=
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },  /* 1  */
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },  /* 3  */
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0, 100,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 115, 11 },
    {  0,  0, 130, 12 },
    {  0,  0, 130, 13 }, /* 13  */
    {  0,  1, 140, 14 },
    {  1,  1, 150, 15 }, /* 15  */
    {  1,  2, 165, 16 },
    {  2,  3, 180, 22 },
    {  2,  3, 200, 25 }, /* 18  */
    {  3,  4, 225, 30 },
    {  3,  5, 250, 35 }, /* 20  */
    {  4,  6, 300, 40 },
    {  4,  6, 350, 45 },
    {  5,  7, 400, 50 },
    {  5,  8, 450, 55 },
    {  6,  9, 500, 60 }  /* 25   */
};



const	struct	int_app_type	int_app		[26]		=
{
    {  3 },	/*  0 */
    {  5 },	/*  1 */
    {  7 },
    {  8 },	/*  3 */
    {  9 },
    { 10 },	/*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },	/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },	/* 15 */
    { 34 },
    { 37 },
    { 40 },	/* 18 */
    { 44 },
    { 49 },	/* 20 */
    { 55 },
    { 60 },
    { 70 },
    { 80 },
    { 85 }	/* 25 */
};



const	struct	wis_app_type	wis_app		[26]		=
{
    { 0 },	/*  0 */
    { 0 },	/*  1 */
    { 0 },
    { 0 },	/*  3 */
    { 0 },
    { 1 },	/*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 1 },	/* 10 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 2 },	/* 15 */
    { 2 },
    { 2 },
    { 3 },	/* 18 */
    { 3 },
    { 3 },	/* 20 */
    { 3 },
    { 4 },
    { 4 },
    { 4 },
    { 5 }	/* 25 */
};



const	struct	dex_app_type	dex_app		[26]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 }    /* 25 */
};


const	struct	con_app_type	con_app		[26]		=
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -2, 30 },
    { -2, 35 },	  /*  3 */
    { -1, 40 },
    { -1, 45 },   /*  5 */
    { -1, 50 },
    {  0, 55 },
    {  0, 60 },
    {  0, 65 },
    {  0, 70 },   /* 10 */
    {  0, 75 },
    {  0, 80 },
    {  0, 85 },
    {  0, 88 },
    {  1, 90 },   /* 15 */
    {  2, 95 },
    {  2, 97 },
    {  3, 99 },   /* 18 */
    {  3, 99 },
    {  4, 99 },   /* 20 */
    {  4, 99 },
    {  5, 99 },
    {  6, 99 },
    {  7, 99 },
    {  8, 99 }    /* 25 */
};



/*
 * Liquid properties.
 * Used in world.obj.
 */
const	struct	liq_type	liq_table	[]	=
{
/*    name			color	proof, full, thirst, food, ssize */
    { "water",			"clear",	{   0, 1, 10, 0, 16 }	},
    { "beer",			"amber",	{  12, 1,  8, 1, 12 }	},
    { "red wine",		"burgundy",	{  30, 1,  8, 1,  5 }	},
    { "ale",			"brown",	{  15, 1,  8, 1, 12 }	},
    { "dark ale",		"dark",		{  16, 1,  8, 1, 12 }	},

    { "whisky",			"golden",	{ 120, 1,  5, 0,  2 }	},
    { "lemonade",		"pink",		{   0, 1,  9, 2, 12 }	},
    { "firebreather",		"boiling",	{ 190, 0,  4, 0,  2 }	},
    { "local specialty",	"clear",	{ 151, 1,  3, 0,  2 }	},
    { "slime mold juice",	"green",	{   0, 2, -8, 1,  2 }	},

    { "milk",			"white",	{   0, 2,  9, 3, 12 }	},
    { "tea",			"tan",		{   0, 1,  8, 0,  6 }	},
    { "coffee",			"black",	{   0, 1,  8, 0,  6 }	},
    { "blood",			"red",		{   0, 2, -1, 2,  6 }	},
    { "salt water",		"clear",	{   0, 1, -2, 0,  1 }	},

    { "coke",			"brown",	{   0, 2,  9, 2, 12 }	}, 
    { "root beer",		"brown",	{   0, 2,  9, 2, 12 }   },
    { "elvish wine",		"green",	{  35, 2,  8, 1,  5 }   },
    { "white wine",		"golden",	{  28, 1,  8, 1,  5 }   },
    { "champagne",		"golden",	{  32, 1,  8, 1,  5 }   },

    { "mead",			"honey-colored",{  34, 2,  8, 2, 12 }   },
    { "rose wine",		"pink",		{  26, 1,  8, 1,  5 }	},
    { "benedictine wine",	"burgundy",	{  40, 1,  8, 1,  5 }   },
    { "vodka",			"clear",	{ 130, 1,  5, 0,  2 }   },
    { "cranberry juice",	"red",		{   0, 1,  9, 2, 12 }	},

    { "orange juice",		"orange",	{   0, 2,  9, 3, 12 }   }, 
    { "absinthe",		"green",	{ 200, 1,  4, 0,  2 }	},
    { "brandy",			"golden",	{  80, 1,  5, 0,  4 }	},
    { "aquavit",		"clear",	{ 140, 1,  5, 0,  2 }	},
    { "schnapps",		"clear",	{  90, 1,  5, 0,  2 }   },

    { "icewine",		"purple",	{  50, 2,  6, 1,  5 }	},
    { "amontillado",		"burgundy",	{  35, 2,  8, 1,  5 }	},
    { "sherry",			"red",		{  38, 2,  7, 1,  5 }   },	
    { "framboise",		"red",		{  50, 1,  7, 1,  5 }   },
    { "rum",			"amber",	{ 151, 1,  4, 0,  2 }	},

    { "cordial",		"clear",	{ 100, 1,  5, 0,  2 }   },
    { NULL,			NULL,		{   0, 0,  0, 0,  0 }	}
};



/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n

	struct	skill_type	skill_table	[MAX_SKILL]	=
{

/*
 * Magic spells.
 */
/*
    {
        name,			, levels,
        rating,
        spell fun,		targets,		position,
        sn,		slot, minimum mana, beats,
        damage, wear off, wear off obj
    },    
*/
    {
	"reserved",		{ 110,110,110,110,110,110,110,110 },	
	{ 99, 99, 99, 99, 99, 99, 99, 99 },
	0,			TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT( 0),	 0,	 0,
	"",			"",		""
    },
     
   {
      "find familiar",	{ 15, 42, 0, 40 },	{3, 5, 0, 8 },
      spell_find_familiar,	TAR_IGNORE,		POS_RESTING,
      NULL,	SLOT(617),	50,	12,
      "",		"!Find Familiar",	""
   },


    {
	"acid blast",		{ 38,102,102,102,102,102,102,30 }, 	
	{ 1,  1,  2,  2,  2,  2,  2,  2},
	spell_acid_blast,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(70),	20,	12,
	"acid blast",		"!Acid Blast!", ""
    },
    
    {
    	"acid rain",		{ 0 },
    	{ 0 },
    	spell_acid_rain,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
    	NULL,			SLOT(618),	35,	12,
    	"acid rain",		"!Acid Rain!", 	""
    },

    {
	"armor",		{  5,  5,102,102, 15, 102,102,102 },     
	{ 1,  1,  2,  2, 1,  2,  2,  2},
	spell_armor,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT( 1),	 5,	12,
	"",			"You feel less armored.",	""
    },

    {
        "advanced mind",	{ 15, 12,102,102,102,102,102,102 },	
	{ 1,  1,  2,  2,  2,  2,  2,  2},
	spell_advanced_mind,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(600),	10,	12,
	"",			"Your mind is no longer as clear.",	""
    },

    {
        "aquastrike",           { 25, 35,102,102,102,102,102,102 },      
	{ 1,  1,  2, 2, 2,  2,  2,  2},
        spell_aquastrike,       TAR_CHAR_OFFENSIVE,      POS_FIGHTING,
        NULL,                   SLOT(605),       15,     12,
        "aquastrike",           "!Aquastrike!",          ""
    },

    {
	"bless",		{ 102, 10,102,102, 12,102,102,102 },    
	{ 1,  1,  2, 2, 2, 2, 2, 2 },
	spell_bless,		TAR_OBJ_CHAR_DEF,	POS_STANDING,
	NULL,			SLOT( 3),	 5,	12,
	"",			"You feel less righteous.", 
	"$p's holy aura fades."
    },

    {
	"blindness",		{ 15, 30,102,102,102,102,102,102 },     
	{ 1,  1,  2,  2, 2, 2, 2, 2},
	spell_blindness,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_blindness,		SLOT( 4),	 5,	12,
	"",			"You can see again.",	""
    },

    {
	"burning hands",	{ 8,102,102,102,102,102,102,102 },     
	{ 1,  1,  2,  2, 2, 2, 2, 2},
	spell_burning_hands,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 5),	15,	12,
	"burning hands",	"!Burning Hands!", 	""
    },

    {
	"call lightning",	{ 35, 25,102,102,102,102,102,102 },     
	{ 1,  1,  2,  2,  2,  2,  2,  2},
	spell_call_lightning,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT( 6),	15,	12,
	"lightning bolt",	"!Call Lightning!",	""
    },

    {   "calm",			{ 75, 10,102,102, 50,102,102,102 },     
	{ 1,  1,  2,  2, 2, 2, 2, 2},
	spell_calm,		TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(509),	30,	12,
	"",			"You have lost your peace of mind.",	""
    },

    {
	"cancellation",		{ 15, 30,102,102,102,102,102,102 },     
	{ 1,  1,  2,  2, 2, 2, 2, 2},
	spell_cancellation,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(507),	20,	12,
	""			"!cancellation!",	""
    },

    {
	"cause critical",	{ 102, 15,102,102,35,102,102,102 },    
	{ 1,  1,  2, 2, 2, 2, 2, 2},
	spell_cause_critical,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(63),	20,	12,
	"spell",		"!Cause Critical!",	""
    },

    {
	"cause light",		{ 102, 5,102,102, 15,102,102,102 },     
	{ 1,  1,  2,  2, 2, 2, 2, 2},
	spell_cause_light,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(62),	15,	12,
	"spell",		"!Cause Light!",	""
    },

    {
	"cause serious",	{ 102, 10,102,102, 25,102,102,102 },     
	{ 1,  1,  2,  2,  2,  2,  2,  2},
	spell_cause_serious,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(64),	17,	12,
	"spell",		"!Cause Serious!",	""
    },

    {   
	"chain lightning",	{ 50,102,102,102,102,102,102,102 },     
	{ 1,  1,  2,  2,  2,  2,  2,  2},
	spell_chain_lightning,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(500),	25,	12,
	"lightning",		"!Chain Lightning!",	""
    }, 

    {
	"change sex",		{ 45, 80,102,102,102,102,102,102 },    
	{ 1,  1,  2,  2,  2,  2,  2,  2},
	spell_change_sex,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(82),	15,	12,
	"",			"Your body feels familiar again.",	""
    },

    {
	"chaos flare",		{ 102, 102,102,102, 102,102,102,102 },    
	{ 1,  1,  2, 2, 2, 2, 2, 2 },
	spell_chaos_flare,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(663),	 5,	12,
	"",			"", 
	"Your flare of chaos subsides!"
    },

    {
	"charm person",		{ 32,102,102,102,102,102,102,102 },     
	{ 1,  1,  2,  2,  2,  2,  2,  2},
	spell_charm_person,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_charm_person,	SLOT( 7),	 5,	12,
	"",			"You feel more self-confident.",	""
    },

    {
	"chill touch",		{ 5,102,102,102,102,102,102,102 },
        { 1,  1,  2, 2, 2, 2, 2, 2},
	spell_chill_touch,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT( 8),	15,	12,
	"chilling touch",	"You feel less cold.",	""
    },

    {
	"colour spray",		{ 1, 45,102,102,102,102,102,102 },    
	{ 1,  1,  2,  2,  2,  2,  2,  2},
	spell_colour_spray,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(10),	15,	12,
	"colour spray",		"!Colour Spray!",	""
    },

    {
	"continual light",	{ 1, 1,102,102, 25,102,102,102 },  
        { 1,  1,  2,  2,  2,  2,  2,  2},
	spell_continual_light,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(57),	 7,	12,
	"",			"!Continual Light!",	""
    },

    {
	"control weather",	{ 30, 30,102,102,102,102,102,102 },
        { 1,  1,  2,  2,  2,  2,  2,  2},
	spell_control_weather,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(11),	25,	12,
	"",			"!Control Weather!",	""
    },

    {
	"create food",		{ 15, 4,102,102,102,102,102,102 },
        { 1,  1,  2,  2,  2,  2,  2,  2},
	spell_create_food,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(12),	 5,	12,
	"",			"!Create Food!",	""
    },

    {
	"create rose",		{ 15, 4,102,102,102,102,102,102 }, 	
	{ 1, 1, 2, 2, 2, 2, 2, 2 },
	spell_create_rose,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(511),	30, 	12,
	"",			"!Create Rose!",	""
    },  

    {
	"create spring",	{ 15, 4,102,102,102,102,102,102 },
        { 1,  1,  2,  2,  2,  2,  2,  2},
	spell_create_spring,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(80),	20,	12,
	"",			"!Create Spring!",	""
    },

    {
	"create water",		{ 15, 4,102,102,102,102,102,102 },     
	{ 1,  1,  2,  2,  2,  2,  2,  2},
	spell_create_water,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(13),	 5,	12,
	"",			"!Create Water!",	""
    },

    {
	"cure blindness",	{ 102, 20,102,102,102,102,102,102 },
        { 1,  1,  2,  2, 2, 2, 2, 2},
	spell_cure_blindness,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(14),	 5,	12,
	"",			"!Cure Blindness!",	""
    },

    {
	"cure critical",	{ 102, 20,102,102, 35,102,102,102 },
        { 1,  1,  2, 2,  2,  2,  2,  2},
	spell_cure_critical,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(15),	20,	12,
	"",			"!Cure Critical!",	""
    },

    {
	"cure disease",		{ 102, 12,102,102,102,102,102,102 },    
	{ 1,  1,  2,  2, 2, 2, 2, 2},
	spell_cure_disease,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(501),	20,	12,
	"",			"!Cure Disease!",	""
    },

    {
	"cure light",		{ 102, 2,102,102, 15,102,102,102 },
        { 1,  1,  2,  2,  2,  2,  2,  2},
	spell_cure_light,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(16),	10,	12,
	"",			"!Cure Light!",		""
    },

    {
	"cure poison",		{ 102, 13,102,102,102,102,102,102 },     
	{ 1,  1,  2, 2, 2,  2,  2,  2},
	spell_cure_poison,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(43),	 5,	12,
	"",			"!Cure Poison!",	""
    },

    {
	"cure serious",		{ 102, 10,102,102,25,102,102,102 },    
	{ 1,  1,  2,  2,2, 2, 2, 2},
	spell_cure_serious,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(61),	15,	12,
	"",			"!Cure Serious!",	""
    },

    {
	"curse",		{ 55, 30,102,102,102,102,102,102 },     
	{ 1,  1,  2,  2, 2, 2, 2, 2},
	spell_curse,		TAR_OBJ_CHAR_OFF,	POS_FIGHTING,
	&gsn_curse,		SLOT(17),	20,	12,
	"curse",		"The curse wears off.", 
	"$p is no longer impure."
    },

    {
	"demonfire",		{ 102, 35,102,102,102,102,102,102 },
        { 1,  1,  2,  2,  2,  2,  2,  2},
	spell_demonfire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(505),	20,	12,
	"torments",		"!Demonfire!",		""
    },	

    {
	"detect evil",		{ 35, 15,102,102,5,102,102,102 },     
	{ 1,  1,  2,  2, 2, 2, 2, 2},
	spell_detect_evil,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(18),	 5,	12,
	"",			"The red in your vision disappears.",	""
    },

    {
        "detect good",          { 35, 15,102,102, 5,102,102,102 },
        { 1,  1,  2,  2,  2,  2,  2,  2},
        spell_detect_good,      TAR_CHAR_SELF,          POS_STANDING,
        NULL,                   SLOT(513),        5,     12,
        "",                     "The gold in your vision disappears.",	""
    },

    {
	"detect hidden",	{ 20, 15,102,102,102,102,102,102 },
        { 1,  1,  2,  2, 2, 2, 2, 2},
	spell_detect_hidden,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(44),	 5,	12,
	"",			"You feel less aware of your surroundings.",	
	""
    },

    {
	"detect invis",		{ 5, 15,102,102,102,102,102,102 },     
	{ 1,  1,  2,  2, 2, 2, 2, 2},
	spell_detect_invis,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(19),	 5,	12,
	"",			"You no longer see invisible objects.",
	""
    },

    {
	"detect magic",		{ 10, 10,102,102,102,102,102,102 },     
	{ 1,  1,  2,  2,  2, 2, 2, 2},
	spell_detect_magic,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(20),	 5,	12,
	"",			"The detect magic wears off.",	""
    },

    {
	"detect poison",	{ 1, 1,102,102,102,102,102,102 },
        { 1,  1,  2,  2,  2,  2,  2,  2},
	spell_detect_poison,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(21),	 5,	12,
	"",			"!Detect Poison!",	""
    },

    {
	"dispel evil",		{ 102, 30,102,102, 40,102,102,102 },
        { 1,  1,  2,  2, 2, 2, 2, 2},
	spell_dispel_evil,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(22),	15,	12,
	"dispel evil",		"!Dispel Evil!",	""
    },

    {
        "dispel good",          { 102, 30,102,102, 40,102,102,102 },
        { 1,  1,  2,  2,  2,  2,  2,  2},
        spell_dispel_good,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(512),      15,     12,
        "dispel good",          "!Dispel Good!",	""
    },

    {
	"dispel magic",		{ 20, 40,102,102,102,102,102,102 },
        { 1,  1,  2,  2,  2,  2,  2,  2},
	spell_dispel_magic,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(59),	15,	12,
	"",			"!Dispel Magic!",	""
    },

    {
	"earthquake",		{ 102, 15,102,102,102 },
        { 1,  1,  2, 2, 2},
	spell_earthquake,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(23),	15,	12,
	"earthquake",		"!Earthquake!",		""
    },

    {
	"enchant armor",	{ 30,102,102,102,102 },
	{ 2,  2,  4,  4, 6 },
	spell_enchant_armor,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(510),	100,	24,
	"",			"!Enchant Armor!",	""
    },

    {
	"enchant weapon",	{ 32,102,102,102,102 },     
	{ 2,  2, 4,  4,  4},
	spell_enchant_weapon,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(24),	100,	24,
	"",			"!Enchant Weapon!",	""
    },

    {
	"energy drain",		{ 30, 40,102,102,102 },
        { 1,  1,  2,  2,2},
	spell_energy_drain,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(25),	35,	12,
	"energy drain",		"!Energy Drain!",	""
    },

    {
        "enhanced mind",	{ 30, 15,102,102,102 },	
	{ 1,  1, 2,  2,2},
	spell_enhanced_mind,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(601),	25,	12,
	"",			"The enlightenment of the ages is no longer with you!."
    },


    {
	"faerie fire",		{  5, 5,102,102,102 },
        { 1,  1,  2,  2,2},
	spell_faerie_fire,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(72),	 5,	12,
	"faerie fire",		"The pink aura around you fades away.",
	""
    },

    {
	"faerie fog",		{ 20, 20,102,102,102 },     
	{ 1,  1,  2,  2,2},
	spell_faerie_fog,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(73),	12,	12,
	"faerie fog",		"!Faerie Fog!",		""
    },

    {
	"farsight",		{ 20, 20,102,102,102 },
	{ 1, 1, 2, 2, 2},
	spell_farsight,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(521),	36,	20,
	"farsight",		"!Farsight!",		""
    },	
    
    {
    	"feast",		{ 0 },
    	{ 1 },
    	spell_feast,		TAR_IGNORE,		POS_STANDING,
    	NULL,			SLOT(611),	20,	12,
    	"feast",		"!Feast!",		""
    },

    {
	"fireball",		{ 5,102,102,102,102 },     
	{ 1,  1,  2,  2, 2},
	spell_fireball,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(26),	15,	12,
	"fireball",		"!Fireball!",		""
    },
  
    {
	"fireproof",		{ 13, 12,102,102,102 },	
	{ 1, 1, 2, 2, 2},
	spell_fireproof,	TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(523),	10,	12,
	"",			"",	"$p's protective aura fades."
    },

    {
    	"flameshield",		{ 102 },
    	{ 1 },
    	spell_flameshield,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
    	NULL,			SLOT(610),	75,	12,
    	"flameshield",		"Your flameshield fades away.",	""
    },
    {
    	"lightningshield",		{ 102 },
    	{ 1 },
    	spell_lightningshield,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
    	NULL,			SLOT(658),	75,	12,
    	"lightning shield",	"Your shield of lightning fades away.",	""
    },

    {
	"acid darts",	{ 102,102,102,102,102 },
        { 1 },
	spell_acid_darts,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(659),	25,	24,
	"acid darts",	"!Acid Darts!",	""
    },
    {
	"holy wrath",	{ 102,102,102,102,102 },
        { 1 },
	spell_holy_wrath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(660),	25,	24,
	"holy wrath",		"!Holy Wrath!",	""
    },

    {
	"flamestrike",		{ 102, 35,102,102,102 },     
	{ 1,  1,  2,  2, 2},
	spell_flamestrike,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(65),	20,	12,
	"flamestrike",		"!Flamestrike!",		""
    },

   {
        "flamepillar",          { 60, 55,102,102,102 },
        { 1,  1,  2,  2, 2},
        spell_flamepillar,      TAR_CHAR_OFFENSIVE,      POS_FIGHTING,
        NULL,                   SLOT(606),       75,     12,
        "flamepillar",          "!Flamepillar!",                ""
    },

    {
	"fly",			{ 5, 15,102,102,102 },
        { 1,  1,  2,  2,  2},
	spell_fly,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(56),	10,	18,
	"",			"You slowly float to the ground.",	""
    },

    {
	"floating disc",	{ 5, 30,102,102,102 },
	{ 1, 1, 2, 2, 2},
	spell_floating_disc,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(522),	40,	24,
	"",			"!Floating disc!",	""
    },
    {
	"ethreal blade",	{ 102 },
	{ 1, 1, 2, 2, 2},
	spell_ethreal_blade,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(661),	200,	24,
	"",			"!Ethreal Blade!",	""
    },
    {
	"holy weapon",	{ 102 },
	{ 1, 1, 2, 2, 2},
	spell_holy_weapon,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(662),	200,	24,
	"",			"!Holy Weapon!",	""
    },

    {
        "frenzy",               { 80, 30,102,102,102 },
     { 1,  1,  2, 2, 2},
        spell_frenzy,           TAR_CHAR_DEFENSIVE,     POS_STANDING,
        NULL,                   SLOT(504),      30,     24,
        "",                     "Your rage ebbs.",	""
    },

    {
	"gate",			{ 42,102,102,102,102 },     
	{ 1,  1,  2,  2, 2},
	spell_gate,		TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(83),	80,	12,
	"",			"!Gate!",		""
    },

    {
	"giant strength",	{ 15, 12,102,102, 102 },     
	{ 1,  1,  2,  2, 2},
	spell_giant_strength,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(39),	20,	12,
	"",			"You feel weaker.",	""
    },
   

    {
	"harm",			{ 102, 25,102,102, 50 },     
	{ 1,  1,  2,  2, 2},
	spell_harm,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(27),	35,	12,
	"harm spell",		"!Harm!,		"""
    },
  
    {
	"haste",		{ 20, 35,102,102,102 },
        { 1,  1,  2,  2,  2},
	spell_haste,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(502),	30,	12,
	"",			"You feel yourself slow down.",	""
    },

    {
	"heal",			{ 102, 28,102,102,50 },
        { 1,  1,  2,  2,  2},
	spell_heal,		TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(28),	50,	12,
	"",			"!Heal!",		""
    },
  
    {
	"heat metal",		{ 102, 28,102,102,102 },	
	{ 1, 1, 2, 2, 2 },
	spell_heat_metal,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(516), 	25,	18,
	"spell",		"!Heat Metal!",		""
    },

    {
	"holy word",		{ 102, 42,102,102,102 },
	{ 2,  2,  4,  4,  4},
	spell_holy_word,	TAR_IGNORE,	POS_FIGHTING,
	NULL,			SLOT(506), 	200,	24,
	"divine wrath",		"!Holy Word!",		""
    },

    {
	"identify",		{ 5, 22,102,102,102 },     
	{ 1,  1,  2,  2,  2},
	spell_identify,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(53),	12,	24,
	"",			"!Identify!",		""
    },

    {
	"infravision",		{ 5, 12,102,102,102 },
        { 1,  1,  2,  2,  2},
	spell_infravision,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(77),	 5,	18,
	"",			"You no longer see in the dark.",	""
    },

    {
	"invisibility",		{ 3, 12,102,102,102 },     
	{ 1,  1,  2,  2,  2},
	spell_invis,		TAR_OBJ_CHAR_DEF,	POS_STANDING,
	&gsn_invis,		SLOT(29),	 5,	12,
	"",			"You are no longer invisible.",		
	"$p fades into view."
    },

    {
        "iron shards",          { 32, 32,102,102,102 },
        { 1,  1,  2,  2, 2},
        spell_iron_shards,      TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(603),       25,     12,
        "iron shards",          "!Iron Shards!"
    },

    {
	"know alignment",	{ 42, 15,102,102,1 },     
	{ 1,  1,  2,  2, 2},
	spell_know_alignment,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(58),	 9,	12,
	"",			"!Know Alignment!",	""
    },

    {
	"lightning bolt",	{ 25, 25,102,102,102 },
        { 1,  1,  2,  2,  2},
	spell_lightning_bolt,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(30),	15,	12,
	"lightning bolt",	"!Lightning Bolt!",	""
    },

    {
	"locate object",	{ 32, 25,102,102,102 },     
	{ 1,  1,  2,  2, 2},
	spell_locate_object,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(31),	20,	18,
	"",			"!Locate Object!",	""
    },
    
    {
    	"mana charge",		{ 102 },
    	{ 1 },
    	spell_mana_charge,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
    	NULL,			SLOT(615),	0,	0,
    	"",			"!Mana Charge!",	""
    },
    
    {
	"mass healing",		{ 102, 40,102,102,102 },
	{ 2,  2,  4,  4,  2},
	spell_mass_healing,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(508),	100,	36,
	"",			"!Mass Healing!",	""
    },

    {
	"mass invis",		{ 25, 32,102,102,102 },
        { 1,  1,  2,  2,  2},
	spell_mass_invis,	TAR_IGNORE,		POS_STANDING,
	&gsn_mass_invis,	SLOT(69),	20,	24,
	"",			"You are no longer invisible.",		""
    },
    
    {
        "mini meteor",		{ 0 },
        { 0 },
        spell_mini_meteor,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
        NULL,			SLOT(617),	30,	12,
        "mini meteor storm",	"!Mini Meteor!",	""
    },

    {
        "nexus",                { 50,102,102,102,102 },    
	{ 2,  2,  4,  4,  4},
        spell_nexus,            TAR_IGNORE,             POS_STANDING,
        NULL,                   SLOT(520),       150,   36,
        "",                     "!Nexus!",		""
    },

    {
	"pass door",		{ 32, 40,102,102,102 },    
	{ 1,  1,  2,  2, 2},
	spell_pass_door,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(74),	20,	12,
	"",			"You feel solid again.",	""
    },

    {
	"plague",		{ 32, 25,102,102,102 },
        { 1,  1,  2,  2,  2},
	spell_plague,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	&gsn_plague,		SLOT(503),	20,	12,
	"sickness",		"Your sores vanish.",	""
    },

/*    {
	"poison",		{ 28, 15,102,102,102 },     
	{ 1,  1,  2,  2,  2},
	spell_poison,		TAR_OBJ_CHAR_OFF,	POS_FIGHTING,
	&gsn_poison,		SLOT(33),	10,	12,
	"poison",		"You feel less sick.",	
	"The poison on $p dries up."
    },*/

    {
        "portal",               { 35,102,102,102,102 },
        { 2,  2,  4,  4,  6}, 
        spell_portal,           TAR_IGNORE,             POS_STANDING,
        NULL,                   SLOT(519),       100,     24,
        "",                     "!Portal!",		""
    },

    {
	"protection evil",	{ 25, 15,102,102, 42 },     
	{ 1,  1,  2,  2,  2},
	spell_protection_evil,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(34), 	5,	12,
	"",			"You feel less protected.",	""
    },

    {
        "protection good",      { 25, 15,102,102, 42 },
        { 1,  1,  2,  2,  2},
        spell_protection_good,  TAR_CHAR_SELF,          POS_STANDING,
        NULL,                   SLOT(514),       5,     12,
        "",                     "You feel less protected.",	""
    },

    {
        "ray of truth",         { 102, 50,102,102,102 },
        { 1,  1,  2,  2,  2},
        spell_ray_of_truth,     TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(518),      20,     12,
        "ray of truth",         "!Ray of Truth!",	""
    },

    {
	"recharge",		{ 12,102,102,102,102 }, 	
	{ 1, 1, 2, 2, 2 },
	spell_recharge,		TAR_OBJ_INV,		POS_STANDING,
	NULL,			SLOT(517),	60,	24,
	"",			"!Recharge!",		""
    },

    {
	"refresh",		{  5, 2,102,102, 12 },
        { 1,  1,  2,  2,  2},
	spell_refresh,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(81),	12,	18,
	"refresh",		"!Refresh!",		""
    },

    {
	"remove curse",		{ 102, 25,102,102, 30 },    
	{ 1,  1,  2,  2, 2},
	spell_remove_curse,	TAR_OBJ_CHAR_DEF,	POS_STANDING,
	NULL,			SLOT(35),	 5,	12,
	"",			"!Remove Curse!",	""
    },

    {
	"sanctuary",		{ 40, 38,102,102, 65 },     
	{ 1,  1,  2,  2, 2},
	spell_sanctuary,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
	&gsn_sanctuary,		SLOT(36),	75,	12,
	"",			"The white aura around your body fades.",
	""
    },
    
    {
    	"scrye",		{ 1 },
    	{ 1 },
    	spell_scrye,		TAR_IGNORE,		POS_STANDING,
    	NULL,			SLOT(616),	90,	12,
    	"",			"!Scrye!",		""
    },

    {
	"shield",		{ 15, 20,102,102, 32 },
        { 1,  1,  2,  2,  2},
	spell_shield,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(67),	12,	18,
	"",			"Your force shield shimmers then fades away.",
	""
    },

    {
	"shocking grasp",	{ 2,102,102,102,102 },     
	{ 1,  1,  2,  2,  2},
	spell_shocking_grasp,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(53),	15,	12,
	"shocking grasp",	"!Shocking Grasp!",	""
    },

    {
    	"silence",		{ 1 },
    	{ 1 },
    	spell_silence,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
    	NULL,			SLOT(613),	30,	12,
    	"",			"You regain your voice.",	""
    },
    
    {
        "silver shards",        { 32, 32,102,102,102 },
        { 1,  1,  1, 1, 2},
        spell_silver_shards,    TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(604),       25,     12,
        "silver shards",        "!Silver Shards!"
    },

    {
	"sleep",		{ 25,102,102,102,102 },     
	{ 1,  1,  2,  2,  2},
	spell_sleep,		TAR_CHAR_OFFENSIVE,	POS_STANDING,
	&gsn_sleep,		SLOT(38),	15,	12,
	"",			"You feel less tired.",	""
    },

    {
        "slow",                 { 25, 50,102,102,102 },
        { 1,  1,  2,  2,  2},
        spell_slow,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(515),      30,     12,
        "",                     "You feel yourself speed up.",	""
    },

    {
	"stone skin",		{ 25, 40,102,102, 50 },     
	{ 1,  1,  2,  2, 2},
	spell_stone_skin,	TAR_CHAR_SELF,		POS_STANDING,
	NULL,			SLOT(66),	12,	18,
	"",			"Your skin feels soft again.",	""
    },

    {
	"summon",		{ 25, 50,102,102,102 },
        { 1,  1,  2,  2, 2},
	spell_summon,		TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(40),	50,	12,
	"",			"!Summon!",		""
    },

    {
	"teleport",		{ 60,102,102,102,102 },     
	{ 1,  1,  2,  2,2},
	spell_teleport,		TAR_CHAR_SELF,		POS_FIGHTING,
	NULL,	 		SLOT( 2),	35,	12,
	"",			"!Teleport!",		""
    },

    {
        "true sight",	        { 58, 60,102,102,102 },
        { 1,  1,  2,  2, 2},
        spell_true_sight,       TAR_CHAR_SELF,		POS_STANDING,
        NULL,                   SLOT(602),      35,     12,
        "",                     "Your eyes change back to normal.",     ""
    },
    {
        "flaming eyes",	        { 58, 60,102,102,102 },
        { 1,  1,  2,  2, 2},
        spell_flaming_eyes,       TAR_CHAR_SELF,	POS_STANDING,
        NULL,                   SLOT(659),      35,     12,
        "",                     "Your eyes change back to normal.",     ""
    },

    {
	"ventriloquate",	{  1, 2,102,102,102 },     
	{ 1,  1,  2,  2, 2},
	spell_ventriloquate,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(41),	 5,	12,
	"",			"!Ventriloquate!",	""
    },
    
    {
    	"voice",		{ 1 },
    	{ 1 },
    	spell_voice,		TAR_CHAR_DEFENSIVE,		POS_FIGHTING,
    	NULL,			SLOT(614),	10,	12,
    	"",			"!Voice!",		""
    },

    {
	"weaken",		{  25, 32,102,102,102 },
        { 1,  1,  2,  2,  2},
	spell_weaken,		TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(68),	20,	12,
	"spell",		"You feel stronger.",	""
    },

    {
	"word of recall",	{ 15, 14,102,102,102 },
        { 1,1, 2, 2, 2},
	spell_word_of_recall,	TAR_CHAR_SELF,		POS_RESTING,
	NULL,			SLOT(42),	 5,	12,
	"",			"!Word of Recall!",	""
    },

/*
 * Dragon breath
 */
    {
	"acid breath",		{ 58, 65,102,102,102 },    
        { 1,  1,  2,  2, 2},
	spell_acid_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(200),	100,	24,
	"blast of acid",	"!Acid Breath!",	""
    },

    {
	"fire breath",		{ 55, 63,102,102,102 },     
	{ 1,  1,  2,  2, 2},
	spell_fire_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(201),	200,	24,
	"blast of flame",	"The smoke leaves your eyes.",	""
    },

    {
	"frost breath",		{ 55, 60,102,102,102 },
        { 1,  1,  2,  2, 2},
	spell_frost_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(202),	125,	24,
	"blast of frost",	"!Frost Breath!",	""
    },

    {
	"gas breath",		{ 54, 60,102,102,102 },
        { 1,  1,  2,  2, 2},
	spell_gas_breath,	TAR_IGNORE,		POS_FIGHTING,
	NULL,			SLOT(203),	175,	24,
	"blast of gas",		"!Gas Breath!",		""
    },

    {
	"lightning breath",	{ 60, 68,102,102,102 },
        { 1,  1,  2,  2,  2},
	spell_lightning_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(204),	150,	24,
	"blast of lightning",	"!Lightning Breath!",	""
    },
    
/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
    {
        "general purpose",      { 102,102,102,102,102 },	
	{ 0, 0, 0, 0, 0 },
        spell_general_purpose,  TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(401),      0,      12,
        "general purpose ammo", "!General Purpose Ammo!",	""
    },
 
    {
        "high explosive",       { 102,102,102,102,102 },
	{ 0, 0, 0, 0, 0 },
        spell_high_explosive,   TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        NULL,                   SLOT(402),      0,      12,
        "high explosive ammo",  "!High Explosive Ammo!",	""
    },


/* combat and weapons skills */


 
    {
	"axe",			{ 102,102, 15, 10, 15 },	
	{ 0, 0, 4, 4, 4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_axe,            	SLOT( 0),       0,      0,
        "",                     "!Axe!",		""
    },

    {
        "dagger",               { 1 ,102, 1, 3, 10, 102, 102, 102 },
        { 4, 0, 2, 3, 3, 0, 0, 0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_dagger,            SLOT( 0),       0,      0,
        "",                     "!Dagger!",		""
    },
 
    {
	"flail",		{ 102, 3, 20, 5, 15 },	
	{ 0, 5, 8, 5, 5},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_flail,            	SLOT( 0),       0,      0,
        "",                     "!Flail!",		""
    },

    {
	"mace",			{ 102, 1, 20, 2, 8 },
	{ 0, 4, 5, 4, 5},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_mace,            	SLOT( 0),       0,      0,
        "",                     "!Mace!",		""
    },

    {
	"polearm",		{ 5,102,102, 15, 15 },
	{ 0, 0, 0, 4, 4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_polearm,           SLOT( 0),       0,      0,
        "",                     "!Polearm!",		""
    },

    {
	"lance",		{ 102,102,102, 20, 10 },	
	{ 0, 0, 0, 5, 3},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_lance,		SLOT( 0),	0,	0,
	"",			"!Lance!",		""
    },
    
    {
	"shield block",		{ 102,102, 3, 1, 1 },
	{ 0, 0, 2, 1, 2},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_shield_block,	SLOT(0),	0,	0,
	"",			"!Shield!",		""
    },
 
    {
	"spear",		{ 2,102, 5,  5, 102 },	
	{ 5, 0, 3, 3, 0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_spear,            	SLOT( 0),       0,      0,
        "",                     "!Spear!",		""
    },

    {
	"sword",		{ 102,102, 1, 1, 1},
	{ 0, 0, 3, 2, 3},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_sword,            	SLOT( 0),       0,      0,
        "",                     "!sword!",		""
    },

    {
	"whip",			{ 102,102, 5, 25, 102 },	
	{ 0, 0, 5, 6, 0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_whip,            	SLOT( 0),       0,      0,
        "",                     "!Whip!",	""
    },

    {
        "backstab",             { 102,102, 2,102,102 },
        { 0, 0, 5, 0, 0},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_backstab,          SLOT( 0),        0,     24,
        "backstab",             "!Backstab!",		""
    },

    {
	"bash",			{ 102,102,102, 1, 9 },
	{ 0, 0, 0, 2, 3},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_bash,            	SLOT( 0),       0,      24,
        "bash",                 "!Bash!",		""
    },

    {
	"berserk",		{ 102,102,102, 28, 32 },
	{ 20, 20, 102 },
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_berserk,        	SLOT( 0),       0,      24,
        "",                     "You feel your pulse slow down.",	""
    },

    {
	"blackjack",		{ 102, 102, 80, 102, 102 },
	{ 0, 0, 5, 0, 0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_blackjack,		SLOT( 0),	0,	10,
	"",			"You headache goes away!",	""
    },
    {
	"surge",		{ 102  },
	{ 0, 0, 5, 0, 0},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_surge,		SLOT( 0),	0,	10,
	"",			"!Surge!",	""
    },
    
    {
    	"counter",		{ 102 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_FIGHTING,
    	&gsn_counter,		SLOT( 0),	0,	0,
    	"",			"!Counter!",				""
    },
    
    {
    	"critical strike",	{ 102 },
    	{ 1 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_critical_strike,	SLOT( 0),	0,	0,
	"",			"!Crit Strike!",			""
    },
    
    {
    	"sword thrust",		{ 102 },
    	{ 1 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_thrust,	SLOT( 0),	0,	12,
	"sword thrust",		"!Sword Thrust!",			""
    },
    {
    	"lunge",		{ 102 },
    	{ 1 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_lunge,		SLOT( 0),	0,	12,
	"lunge",		"!Lunge!",			""
    },
    {
    	"track",		{ 102 },
    	{ 1 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_hunt,		SLOT( 0),	0,	12,
	"track",		"!Track!",			""
    },
    {
    	"finesse",		{ 102 },
    	{ 1 },
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_finesse,		SLOT( 0),	0,	12,
	"weapon finesse",	"!Finesse!",			""
    },
    {
    	"layhands",		{ 102,102,102,102,102 },
    	{ 1 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_layhands,		SLOT( 0),	0,	12,
	"layhands",		"Your divinegrace ebbs.",		""
    },

    {
	"dirt kicking",		{ 102,102, 5, 15, 102 },
	{ 0, 0, 3, 4, 0}, 
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_dirt,		SLOT( 0),	0,	24,
	"kicked dirt",		"You rub the dirt out of your eyes.",	""
    },

    {
        "disarm",               { 102,102, 40, 32, 38 },
        { 0, 0, 6, 4, 5},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_disarm,            SLOT( 0),        0,     24,
        "",                     "!Disarm!",		""
    },
 
    {
        "dodge",                { 102, 30, 5, 10, 10 },
        { 0, 8, 3, 3, 3},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_dodge,             SLOT( 0),        0,     0,
        "",                     "!Dodge!",		""
    },

    {
    	"ambidexterity",	{ 102, 102, 50, 50, 50, 50, 50, 102 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_ambidexterity,	SLOT( 0),	 0,	0,
    	"",			"!Ambidexterity!",		""
    },
    
    {
    	"engage",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_FIGHTING,
    	&gsn_engage,		SLOT( 0),	0,	12,
    	"",			"!Engage!",		""
    },
 
    {
        "enhanced damage",      { 102,102, 20, 6, 7 },     
	{ 0, 0, 4, 3, 4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_enhanced_damage,   SLOT( 0),        0,     0,
        "",                     "!Enhanced Damage!",	""
    },

    {
	"envenom",		{ 102,102, 15,102,102 },
	{ 0, 0, 2, 0, 0 },
	spell_null,		TAR_IGNORE,	  	POS_RESTING,
	&gsn_envenom,		SLOT(0),	0,	36,
	"",			"!Envenom!",		""
    },

    {
	"hand to hand",		{ 102,102, 30, 20, 22 },	
	{ 0, 0, 5, 4, 5},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_hand_to_hand,	SLOT( 0),	0,	0,
	"",			"!Hand to Hand!",	""
    },

    {
        "kick",                 { 102,102, 5, 5, 15 },     
	{ 0, 0, 4, 3, 3},
        spell_null,             TAR_CHAR_OFFENSIVE,     POS_FIGHTING,
        &gsn_kick,              SLOT( 0),        0,     12,
        "kick",                 "!Kick!",		""
    },

    {
        "parry",                { 102,102, 10, 2, 5 },     
	{ 0, 0, 4, 4, 4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_parry,             SLOT( 0),        0,     0,
        "",                     "!Parry!",		""
    },

     {
        "phase",                { 10, 15, 40, 53 },     { 1, 3, 6, 0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_phase,             SLOT( 0),       0,      0,
        "",                     "!Phase!",   ""
    },


    {
        "rescue",               { 102,102,102, 1, 1 },
        { 0, 0, 0, 3, 1},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_rescue,            SLOT( 0),        0,     12,
        "",                     "!Rescue!",		""
    },

    {
	"trip",			{ 102,102, 4, 10, 102 },	
	{ 0, 0, 4, 5, 0},
	spell_null,		TAR_IGNORE,		POS_FIGHTING,
	&gsn_trip,		SLOT( 0),	0,	24,
	"trip",			"!Trip!",		""
    },

    {
        "second attack",        { 102, 30, 16, 10, 14 },
        { 0, 8, 5, 3, 4},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_second_attack,     SLOT( 0),        0,     0,
        "",                     "!Second Attack!",	""
    },

    {
        "third attack",         { 102,102, 30, 24, 22 },
        { 0, 0, 8, 4, 5},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_third_attack,      SLOT( 0),        0,     0,
        "",                     "!Third Attack!",	""
    },

    {
        "fourth attack",        { 102,102,102, 45, 54 },    
	{ 0, 0, 0, 6, 7},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_fourth_attack,     SLOT( 0),        0,     0,
        "",                     "!Fourth Attack!",       ""
    },

    {
        "fifth attack",        { 65,102,102, 102, 102 },
        { 4, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_fifth_attack,     SLOT( 0),        0,     0,
        "",                     "!Fifth Attack!",       ""
    },

    {
        "sixth attack",        { 85,102,102, 102, 102 },
        { 5, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_sixth_attack,     SLOT( 0),        0,     0,
        "",                     "!Sixth Attack!",       ""
    },


/* non-combat skills */

    { 
	"fast healing",		{ 102,102, 20, 2, 15 },	
	{ 0, 0, 6, 4, 5},
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_fast_healing,	SLOT( 0),	0,	0,
	"",			"!Fast Healing!",	""
    },

    {
	"haggle",		{ 102,102, 1, 15, 102 },	
	{ 0, 0, 3, 5, 0},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_haggle,		SLOT( 0),	0,	0,
	"",			"!Haggle!",		""
    },

    {
	"hide",			{ 102,102, 15,102,102 },
	{ 0, 0, 4, 0, 0},
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_hide,		SLOT( 0),	 0,	12,
	"",			"!Hide!",		""
    },

    {
	"lore",			{ 102,102, 15, 15, 15 },
	{ 15, 15, 4 },
	spell_null,		TAR_IGNORE,		POS_RESTING,
	&gsn_lore,		SLOT( 0),	0,	36,
	"",			"!Lore!",		""
    },

    {
	"meditation",		{ 5, 10,102,102, 15 },
	{ 5, 5, 0, 0, 6},
	spell_null,		TAR_IGNORE,		POS_SLEEPING,
	&gsn_meditation,	SLOT( 0),	0,	0,
	"",			"Meditation",		""
    },

    {
	"peek",			{ 102,102, 2,102,102 },	
	{ 0, 0, 3, 0, 0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_peek,		SLOT( 0),	 0,	 0,
	"",			"!Peek!",		""
    },

    {
	"pick lock",		{ 102,102, 10,102,102 },
	{ 0, 0, 5, 0, 0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_pick_lock,		SLOT( 0),	 0,	12,
	"",			"!Pick!",		""
    },

    {
	"sneak",		{ 102,102, 2,102,102 },	
	{ 0, 0, 3, 0, 0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_sneak,		SLOT( 0),	 0,	12,
	"",			"You no longer feel stealthy.",	""
    },

    {
	"steal",		{ 102,102, 10,102,102 },
	{ 0, 0, 2, 0, 0},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_steal,		SLOT( 0),	 0,	24,
	"",			"!Steal!",		""
    },

    {
	"scrolls",		{  1,  1,102,102, 1 },	
	{ 2, 4, 0, 0, 5},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_scrolls,		SLOT( 0),	0,	12,
	"",			"!Scrolls!",		""
    },

    {
	"staves",		{  1,  1,102,102, 1 },
	{ 2, 3, 0, 0, 5},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_staves,		SLOT( 0),	0,	12,
	"",			"!Staves!",		""
    },
    
    {
	"wands",		{  1,  1,102,102, 1 },	
	{ 2, 3, 0, 0, 5},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_wands,		SLOT( 0),	0,	12,
	"",			"!Wands!",		""
    },
    {
	"magic missile",	{ 5,102,102,102,102 },
        { 1,  1,  2,  2,2},
	spell_magic_missile,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
	NULL,			SLOT(32),	15,	12,
	"magic missile",	"!Magic Missile!",	""
    },

    {
	"recall",		{  1,  1,  1,  1, 1 },
	{ 1, 1, 1, 1, 1},
	spell_null,		TAR_IGNORE,		POS_STANDING,
	&gsn_recall,		SLOT( 0),	0,	12,
	"",			"!Recall!",		""
    },
    
    /* Language skills*/
    
    {
    	"human",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_human,		SLOT( 0),	0,	0,
    	"",			"!Human!",		""
    },

    {
    	"aghar",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_aghar,		SLOT( 0),	0,	0,
    	"",			"!Aghar!",		""
    },

    {
    	"hylar",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_hylar,		SLOT( 0),	0,	0,
    	"",			"!Hylar!",		""
    },

    {
    	"neidar",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_neidar,		SLOT( 0),	0,	0,
    	"",			"!Neidar!",		""
    },

    {
    	"minotaur",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_minotaur,		SLOT( 0),	0,	0,
    	"",			"!Minotaur!",		""
    },

    {
    	"kagonesti",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_kagonesti,		SLOT( 0),	0,	0,
    	"",			"!Kagonesti!",		""
    },

    {
    	"dimernesti",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_dimernesti,		SLOT( 0),	0,	0,
    	"",			"!Dimernesti!",		""
    },

    {
    	"gnome",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_gnome,		SLOT( 0),	0,	0,
    	"",			"!Gnome!",		""
    },

    {
    	"kender",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_kender,		SLOT( 0),	0,	0,
    	"",			"!Kender!",		""
    },

    {
    	"dracon",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_draconian,		SLOT( 0),	0,	0,
    	"",			"!Draconian!",		""
    },

    {
    	"silvanesti",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_silvanesti,	SLOT( 0),	0,	0,
    	"",			"!Silvanesti!",		""
    },

    {
    	"qualinesti",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_qualinesti,	SLOT( 0),	0,	0,
    	"",			"!Qualinesti!",		""
    },
    
    {
    	"right hand",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_right_hand,	SLOT( 0),	0,	0,
    	"",			"!Right Hand!",		""
    },
    
    {
    	"left hand",		{ 1 },
    	{ 1 },
    	spell_null,		TAR_IGNORE,		POS_RESTING,
    	&gsn_left_hand,		SLOT( 0),	0,	0,
    	"",			"!Left Hand!",		""
    },

    {
        "circle",             { 102,102,102,102,102 },
        { 0, 0, 5, 0, 0},
        spell_null,             TAR_IGNORE,             POS_FIGHTING,
        &gsn_circle,          SLOT( 0),        0,     24,
        "backstab",             "!Backstab!",		""
    },
    {
        "brew",             { 102,102,102,102,102 },
        { 0, 0, 5, 0, 0},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_brew,          SLOT( 0),        0,     24,
        "",             "!Brew!",		""
    },

    {
        "sap",             { 101,102,102,102,102 },
        { 4, 0, 0, 0, 0},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_sap,          SLOT( 0),        0,     24,
        "",             "!Your headache has gone away!",               ""
    },


    {
        "scribe",             { 102,102,102,102,102 },
        { 0, 0, 5, 0, 0},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_scribe,          SLOT( 0),        0,     24,
        "",             "!Scribe!",		""
    },
    {
        "skin",             { 102,102,102,102,102 },
        { 0, 0, 5, 0, 0},
        spell_null,             TAR_IGNORE,             POS_STANDING,
        &gsn_skin,          SLOT( 0),        0,     24,
        "",             "!Skin!",		""
    },
    {
	"animate dead",		{ 102, 102, 102, 102 },	{ 1,  1,  2,  2},
	spell_resurrect,	TAR_IGNORE,		POS_STANDING,
	NULL,			SLOT(650),	100,	15,
	"",			"!Animate dead!",		"",
    },
    {
	"hallucination",	{ 102, 10,102,102, 25,102,102,102 },
	{ 102,  102,  102,  102,  102,  102,  102,  102},
	spell_hallucinate,	TAR_IGNORE,	POS_STANDING,
	NULL,			SLOT(655),	5,	2,
	"",		"{RYour {rhead {Bfeels {bnormal {Cagain{c...{x",	""
    },
    {
	"consume",		{ 102, 102, 102, 102 },	{ 1,  1,  2,  2},
	spell_consume,	TAR_CHAR_OFFENSIVE,		POS_FIGHTING,
	NULL,			SLOT(656),	100,	2,
	"spell",			"!Consume!",		"",
    },
    {
	"blur",	{ 102, 10,102,102, 25,102,102,102 },
	{ 102,  102,  102,  102,  102,  102,  102,  102},
	spell_null,	TAR_IGNORE,	POS_STANDING,
	&gsn_blur,			SLOT(0),	5,	2,
	"",		"You can see straight again.{x",	""
    },

    {
	"aid",			{ 53,  8, 53, 53 },	{ 0, 1, 0, 0 },
	spell_aid,		TAR_CHAR_DEFENSIVE,	POS_STANDING,
	NULL,			SLOT(657),	15,	12,
	"",			"You don't feel so aided anymore.",
	""
    }
};

const   struct  group_type      group_table     [MAX_GROUP]     =
{

    {
	"rom basics",		{ 0, 0, 0, 0, 0 },
	{ "scrolls", "staves", "wands", "recall", "right hand", "left hand" }
    },

    {
	"mage basics",		{ 0, -1, -1, -1, -1, -1, -1, -1 },
	{ "dagger" }
    },

    {
	"cleric basics",	{ -1, 0, -1, -1, -1, -1, -1, -1 },
	{ "mace" }
    },
   
    {
	"thief basics",		{ -1, -1, 0, -1, -1, -1 },
	{ "dagger", "steal" }
    },

    {
	"warrior basics",	{ -1, -1, -1, 0, -1, -1 },
	{ "sword", "second attack" }
    },

    {
	"paladin basics",	{ -1, -1, -1, -1, 0, -1 },
	{ "sword", "second attack", "layhands" }
    },
    
    {
    	"ranger basics",	{ -1, -1, -1, -1, -1, 0 },
    	{ "sword" }
    },
    
    {
    	"templar basics",	{ -1, -1, -1, -1, -1, -1, 0, -1 },
    	{ "mace", "second attack" }
    },
    
    {
    	"bmage basics",		{ -1, -1, -1, -1, -1, -1, -1, 0 },
    	{ "dagger" }
    },

    {
	"mage default",		{ 40, -1, -1, -1, -1 },
	{ "beguiling", "combat", "detection", "enhancement", "illusion",
	  "maladictions", "protective", "transportation", "weather" }
    },

    {
	"cleric default",	{ -1, 40, -1, -1, -1 },
	{ "flail", "attack", "curative",  "benedictions", "healing",
	  "harmful", "maladictions", "protective", "shield block" } 
    },
 
    {
	"thief default",	{ -1, -1, 40, -1, -1 },
	{ "mace", "sword", "backstab", "disarm", "dodge", "second attack",
	  "trip", "hide", "peek", "pick lock", "sneak" }
    },

    {
	"warrior default",	{ -1, -1, -1, 40, -1 },
	{ "weaponsmaster", "shield block", "bash", "disarm", "enhanced damage", 
	  "parry", "rescue", "third attack", }
    },

    {
	"paladin default",	{ -1, -1, -1, -1, 40 },
	{ "weaponsmaster", "shield block", "disarm", "enhanced damage",
	  "parry", "rescue", "dodge", "third attack", "fourth attack",
	  "protective" }
    },
    
    {
    	"ranger default",	{ -1, -1, -1, -1, -1, 40 },
    	{ "illusion", "detection", "beguiling", "ambidexterity", "dagger",
    	  "polearm", "sneak", "hide" }
    },
    
    {
    	"templar default",	{ -1, -1, -1, -1, -1, -1, 50 },
    	{ "flail", "attack", "curative",  "benedictions", "healing",
	  "harmful", "maladictions", "protective", "shield block",
	  "sword", "dagger"  }
    },
    
    {
    	"bmage default",	{ -1, -1, -1, -1, -1, -1, -1, 40 },
    	{ "beguiling", "combat", "detection", "enhancement", "illusion",
	  "maladictions", "protective", "transportation", "weather",
	  "sword" }
    },

    {
	"weaponsmaster",	{ -1, -1, 30, 20, 25, 35, 30, 30 },
	{ "axe", "dagger", "flail", "mace", "polearm", "spear", "sword", "whip",
	  "lance" }
    },

    {
	"attack",		{ -1, 6, -1, -1, 8, -1, 7, -1 },
	{ "demonfire", "dispel evil", "dispel good", "earthquake", 
	  "flamestrike", "heat metal", "ray of truth", "holy wrath" }
    },

    {
	"beguiling",		{ 5, 6, -1, -1, -1, 6, -1, 5 },
	{ "calm", "charm person", "sleep", "find familiar" }
    },

    {
	"benedictions",		{ -1, 4, -1, -1, 6, -1, 5, -1 },
	{ "bless", "calm", "frenzy", "holy word", "remove curse" }
    },

    {
	"combat",		{ 9, -1, -1, -1, -1, -1, -1, 8 },
	{ "acid blast", "burning hands", "chain lightning", "chill touch",
	  "colour spray", "fireball", "lightning bolt", "magic missile",
	  "shocking grasp", "acid darts"  }
    },

    {
	"creation",		{ 4, 6, -1, -1, -1, 3, 8, 4 },
	{ "continual light", "create food", "create spring", "create water",
	  "create rose", "floating disc", "feast", "ethreal blade", 
   	  "holy weapon" }
    },

    {
	"curative",		{ -1, 5, -1, -1, 6, -1, -1, -1 },
	{ "cure blindness", "cure disease", "cure poison", "voice" }
    }, 

    {
	"detection",		{ 6, 6, -1, -1, -1, -1, -1, 7 },
 	{ "detect evil", "detect good", "detect hidden", "detect invis", 
	  "detect magic", "detect poison", "identify", 
	  "know alignment", "locate object", "true sight", "scrye" } 
    },/* removed farsight */

    {
	"alignmnet",		{ -1, -1, -1, -1, 2, -1, -1, -1 },
 	{ "detect evil", "detect good" } 
    },


    {
	"dragon",		{ 8, -1, -1, -1, -1, -1, -1, 6 },
	{ "acid breath", "fire breath", "frost breath", "gas breath",
	  "lightning breath"  }
    },

    {
	"enchantment",		{ 6, -1, -1, -1, -1 },
	{ "enchant armor", "enchant weapon", "fireproof", "recharge" }
    },

    { 
	"enhancement",		{ 5, 6, -1, -1, -1, -1, 7, 7 },
	{ "giant strength", "haste", "infravision",
	  "enhanced mind", "advanced mind" }
    },

    {
	"harmful",		{ -1, 4, -1, -1, 6, -1, 4, -1 },
	{ "cause critical", "cause light", "cause serious", "harm" }
    },

    {   
	"healing",		{ -1, 4, -1, -1, 6, -1, 6 },
 	{ "cure critical", "cure light", "cure serious", "heal", 
	  "mass healing", "refresh" }
    },

    {
	"illusion",		{ 2, -1, -1, -1, -1, 4, -1, 5 },
	{ "invis", "mass invis", "ventriloquate" }
    },
  
    {
	"maladictions",		{ 6, 5, -1, -1, -1, -1, 5, 5 },
	{ "blindness", "change sex", "curse", "energy drain", "plague", 
	  "poison", "slow", "weaken", "silence" }
    },

    { 
	"protective",		{ 5, 4, -1, -1, 4, -1, 6, 6 },
	{ "armor", "cancellation", "dispel magic", "fireproof",
	  "protection evil", "protection good", "sanctuary", "shield", 
	  "stone skin", "flameshield", "lightningshield", "flaming eyes" }
    },

    {
	"transportation",	{ 4, 5, -1, -1, -1, -1, -1, 6 },
	{ "fly", "gate", "nexus", "pass door", "portal", "summon", "teleport", 
	  "word of recall" }
    },
   
    {
	"weather",		{ 4, 5, -1, -1, -1, -1, 6, 5 },
	{ "call lightning", "control weather", "faerie fire", "faerie fog",
	  "lightning bolt" }
    }
	
   

};




/* god table */
 const   struct  god_type       god_table      []           = 
 {
 /*
 god == 0
            || 
     {
         name,
         player_good?,	player_neutral?,player_evil?,
         player_human?,	player_silv?,	player_qual?,	player_kag?,
         player_dim?,	player_darg?,	player_helf?,	player_hylar?,
         player_neidar?,player_aghar?,	player_gnome?,	player_kender?,
         player_mino?,
         temple vnum
     },
 */
     { "",	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
     		FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, ROOM_VNUM_TEMPLE },
 
     { "Nobody",FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
     		FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, ROOM_VNUM_TEMPLE },
 
     { "Solinari",
       TRUE,	FALSE,	FALSE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,
       11548
     },
 
     { "Habbakuk",
       TRUE,	FALSE,	FALSE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,
       11547
     },
 
     { "Paladine",
       TRUE,	FALSE,	FALSE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,
       11545
     },
  
     { "Kiri-Jolith",
       TRUE,    FALSE,  FALSE,
       TRUE,    TRUE,   TRUE,   TRUE,	
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,
       11549
     },

     { "Branchala",
       TRUE,    FALSE,  FALSE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,
       11550
     },

     { "Majere",
       TRUE,    FALSE,  FALSE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,
       11551
     },

     { "Mishakal",
       TRUE,    FALSE,  FALSE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,
       11546
     },

/* Neutral Gods */

     { "Reorx",
       FALSE,   TRUE,   FALSE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,
       21948
     },

     { "Gilean",
       FALSE,   TRUE,   FALSE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,
       21920
     },

     { "Sirrion",
       FALSE,   TRUE,   FALSE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,
       21987
     },

     { "Lunitari",
       FALSE,   TRUE,   FALSE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,
       21954
     },

     { "Chislev",
       FALSE,   TRUE,   FALSE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,
       21922
     },

     { "Shinare",
       FALSE,   TRUE,   FALSE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,
       21992
     },

     { "Zivilyn",
       FALSE,   TRUE,   FALSE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,	TRUE,	TRUE,	TRUE,
       TRUE,
       21921
     },

/* Evil Gods */

     { "Sargonnas",
       FALSE,   FALSE,  TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,
       11409
     },

     { "Chemosh",
       FALSE,   FALSE,  TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,
       11415
     },

     { "Nuitari",
       FALSE,   FALSE,  TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,
       11419
     },

     { "Zeboim",
       FALSE,   FALSE,  TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,
       11428
     },

     { "Takhisis",
       FALSE,   FALSE,  TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,
       11401
     },

     { "Hiddukel",
       FALSE,   FALSE,  TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,
       11420
     },

     { "Morgion",
       FALSE,   FALSE,  TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,    TRUE,   TRUE,   TRUE,
       TRUE,
       11417
     },
     { "A",
       FALSE,   FALSE,  FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,
       11417
     },
     { "D",
       FALSE,   FALSE,  FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,
       11417
     },
     { "E",
       FALSE,   FALSE,  FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,
       11417
     },
     { "F",
       FALSE,   FALSE,  FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,
       11417
     },
     { "I",
       FALSE,   FALSE,  FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,
       11417
     },
     { "J",
       FALSE,   FALSE,  FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,
       11417
     },
     { "O",
       FALSE,   FALSE,  FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,
       11417
     },
     { "Q",
       FALSE,   FALSE,  FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,
       11417
     },
     { "U",
       FALSE,   FALSE,  FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,
       11417
     },
     { "V",
       FALSE,   FALSE,  FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,
       11417
     },
     { "W",
       FALSE,   FALSE,  FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,
       11417
     },
     { "X",
       FALSE,   FALSE,  FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,   FALSE,  FALSE, FALSE,
       FALSE,
       11417
     }
     
};
