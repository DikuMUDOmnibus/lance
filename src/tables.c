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
#include <time.h>
#include "merc.h"
#include "tables.h"

/* for clans  */
const struct clan_type clan_table[MAX_CLAN] =
{
    /*  name,		who entry,
    	death-transfer room,	recall room,	independent	subclan*/
    
    /* independent should be FALSE if is a real clan */
    /* subclan should be TRUE for clan with subclans */
    {	"",		"",
    	11500,			0,		TRUE,	FALSE	},

    {	"solamnic",	"[{C Solamnic {x] ",
    	14072,			14072,		FALSE,	TRUE	},
    {	"kot",		"[{D   KoT    {x] ",
    	100,			100,		FALSE,	TRUE	},
    {	"conclave",	"[{W Conclave {x] ",
    	11701,			11701,		FALSE,	TRUE	},
    {	"holyorder",	"[{GHoly Order{x] ",
    	5600,			5600,		FALSE,	TRUE	},
    {	"wildrunner",	"[{GWildrunner{x] ",
    	14851,			14851,		FALSE,	FALSE	},
    {	"mercenary",	"[{BMercenary {x] ",
    	16053,			16053,		FALSE,	FALSE	},
    {	"thehorde",	"[ {DTheHorde{x ] ",
    	6000,			6000,		FALSE,	FALSE	},
    {	"loner",	"[{Y  Loner   {x] ",
    	11500,			0,		TRUE,	FALSE	},
    {	"renegade",	"[{m Renegade {x] ",
    	11500,			0,		TRUE,	FALSE	},
    {	"heathen",	"[{g Heathen  {x] ",
    	11500,			0,		TRUE,	FALSE	},
    {	"finder",	"[{R  Finder  {x] ",
    	18202,			18202,		FALSE,	FALSE	},
    {	"rfos",	"[{r   RFoS   {x] ",
    	20086,			20050,		FALSE,	FALSE	},
    {	"undead",	"[{r  Undead  {x] ",
    	903,			903,		FALSE,	FALSE	},
    {	"teyr",	    	"[{R   Teyr   {x] ",
    	1,			1,		FALSE,	FALSE	},
    {	"thieves",	"[{y Thieves  {x] ",
    	13856,			13856,		FALSE, 	FALSE	}

};

const struct subclan_type subclan_table[MAX_SUBC] = 
{
    {   "",             "",                     0       },
    {	"crown",	"[{Y Crown {x] ",	1	},
    {	"sword",	"[{W Sword {x] ",	1	},
    {	"rose",		"[{r Rose  {x] ",	1	},
    {	"reserve",	"[{yReserve{x] ",	1	},

    {	"lily",		"[{B Lily  {x] ",	2	},
    {	"skull",	"[{W Skull {x] ",	2	},
    {	"thorn",	"[{r Thorn {x] ",	2	},
    {   "storm",        "[{c Storm {x] ",       2     },

    {	"white",	"[{W White {x] ",	3	},
    {	"red",		"[{r  Red  {x] ",	3	},
    {	"black",	"[{D Black {x] ",	3	},
    
    {	"light",	"[{W Light {x] ",	4	},
    {	"balance",	"[{rBalance{x] ",	4	},
    {	"dark",		"[{D Dark  {x] ",	4	}
   
};
const struct ctalk_type ctalk_table[MAX_CLAN] = 
{
    {	"",		""			},
    {	"solamnic",	"a solamnic squire"	},
    {	"kot",		"a squire of Takhisis"	},
    {	"conclave",	"a conclave apprentice"	},
    {	"holy order",	"an adept"		},
    {	"wildrunner",	"a green-clad runner"	},
    {	"mercenary",	"an errand-boy"		},
    {	"thehorde",	"a skulking goblin"	},

    {	"loner",	""			},
    {	"renegade",	""			},
    {	"heathen",	""			},
    {	"finder",	"a small kender"	},
    {	"rfos",	"a minotaur"		},
    {	"undead",       "a rotten dove"        	},
    {	"teyr",		"a baaz servant"	},
    {	"thieves",	"a pick-pocket"		}

};

const struct rank_type handler_table[]=
{
    {	""			},
    {	"Jr. Borrower"		},
    {	"Borrower"		},
    {	"Wanderer"		},
    {	"Pouch Keeper"		},
    {	"Taunter"		},
    {	"Handler"		},
    {	"Explorer"		},
    {	"Mapmaker"		},
    {	"Master of Pouches"	},
    {	"Uncle"			},
    {	"Master Handler"	}
};

const struct rank_type queshu_table[] =
{
    {	""			},
    {	"Warrior of Darkness"		},
    {	"Gladiator"			},
    {	"Fist of Darkness"		},
    {	"Mariner"		},
    {	"Avenger of the Blood Sea"		},
    {	"Captain"		},
    {	"Commander"		},
    {	"Champion of the Circus"			},
    {	"Advisor"		},
    {	"Champion of the Blood Sea"		},
    {	"Wrathful"	}
};

const struct rank_type sol_crown_table[] =
{
    {	""			},
    {	"Page of the Crown"	},
    {	"Squire of the Crown"	},
    {	"Knight of the Crown"	},
    {	"Shield Knight"		},
    {	"Shield of the Crown"	},
    {	"Warrior of the Crown"	},
    {	"Lord of Shield"	},
    {	"Lord of Crown"		},
    {	"Master Warrior"	},
    {	"High Warrior"		},
    {   "Lord Warrior"		}
};

const struct rank_type sol_sword_table[] =
{
    {	""			},
    {	"Page to the Sword"	},
    {	"Squire of the Sword"	},
    {	"Knight of the Sword"	},
    {	"Captain of the Sword"	},
    {	"Sword Warrior"		},
    {	"Vetran of the Sword"	},
    {	"Master of the Sword"	},
    {	"Lord of the Sword"	},
    {	"Master Clerist"	},
    {	"High Clerist"		}
};   

const struct rank_type sol_rose_table[] =
{
    {	""			}, 
    {	"Novice of Roses"	}, 
    {	"Knight of Tears"	}, 
    {	"Knight of Mind"	}, 
    {	"Knight of Heart"	}, 
    {	"Knight of Roses"	}, 
    {	"Master of Roses"	}, 
    {	"Archknight"		}, 
    {	"Lord of Roses"		}, 
    {	"Master of Justice"	}, 
    {	"High Justice"  	}
};
      
const struct rank_type solamnic_table[] =
{
    {   ""			},
    {   "Squire of the Crown"	},
    {	"Knight of the Crown"	},
    {	"Novice of the Sword"	},
    {	"Knight of the Sword"	},
    {	"Knight of the Rose"	},
    {	"Archknight"		},
    {	"High Warrior"		},
    {	"High Clerist"		},
    {	"High Justice"		},
    {	"Grand Master"		},
    {	"Grand Master"		}
};


const struct rank_type kot_lily_table[] =
{
    {   ""			},
    {   "Night Acolyte"		},
    {	"Night Warrior"		},
    {	"Dark Wanderer"		},
    {	"Dark Warrior"		},
    {	"Warrior of the Lily"	},
    {	"Master of the Lily"	},
    {	"Champion of the Lily"	},
    {	"Captain of the Lily"	},
    {	"Protector of the Lily"	},
    {	"Lord of the {bLily{x"	},
    {	"Lord of the {bLily{x"	}
};


const struct rank_type kot_skull_table[] =
{
    {   ""			},
    {   "Bone Warrior"		},
    {	"Dark Cleric"		},
    {	"Skull Bishop"		},
    {	"Skull Priest"		},
    {	"Skull Knight"		},
    {	"Master of the Skull"	},
    {	"Skull Champion"	},
    {	"Keeper of the Bones"	},
    {	"High Priest"		},
    {	"Lord of the {WSkull{x"	},
    {	"Lord of the {WSkull{x"	}
};


const struct rank_type kot_thorn_table[] =
{
    {   ""			},
    {   "Blood Acolyte"		},
    {	"Thorn Apprentice"	},
    {	"Thorn Sorcerer"	},
    {	"Thorn Wizard"		},
    {	"Thorn Seer"		},
    {	"Thorn Master"		},
    {	"Nightlord"		},
    {	"Master of the Thorn"	},
    {	"Archmage of the Thorn"	},
    {	"Lord of the {rThorn{x"	},
    {	"Lord of the {rThorn{x"	}
};

const struct rank_type kot_storm_table[] =
{
    {   ""                           },
    {   "{DDread Rider{x"            },
    {   "{DDread Knight{x"           },
    {   "{DDoom Knight{x"            },
    {   "{DStorm Knight{x"           },
    {   "{DStorm Champion{x"         },
    {   "{DStorm Forger{x"           },
    {   "{DTamer of the Storm{x"     },
    {   "{DKeeper of the Night{x"    },
    {   "{DCommander of the Night{x" },
    {   "{DHarbinger of Vengeance{x" },
    {   "{DBlah{x"                   }
};

const struct rank_type ho_light_table[] =
{
    {   ""				},
    {   "Initiate"			},
    {	"Neophyte"			},
    {	"Novitiate"			},
    {	"Acolyte"			},
    {	"Believer of the Faith"		},
    {	"Master of the Faith"		},
    {	"Upholder of the Faith"		},
    {	"Protector of the Faith"	},
    {	"Crusader of the Faith"		},
    {	"High Master Healer"		},
    {	"High Master"		}
};

const struct rank_type ho_balance_table[] =
{
    {   ""				},
    {   "Initiate"			},
    {	"Neophyte"			},
    {	"Novitiate"			},
    {	"Acolyte"			},
    {	"Believer of the Faith"		},
    {	"Master of the Faith"		},
    {	"Upholder of the Faith"		},
    {	"Protector of the Faith"	},
    {	"Crusader of the Faith"		},
    {	"High Master Healer"		},
    {	"High Master"		}
};

const struct rank_type ho_dark_table[] =
{
    {   ""				},
    {   "Initiate"			},
    {	"Neophyte"			},
    {	"Novitiate"			},
    {	"Acolyte"			},
    {	"Believer of the Faith"		},
    {	"Master of the Faith"		},
    {	"Upholder of the Faith"		},
    {	"Protector of the Faith"	},
    {	"Crusader of the Faith"		},
    {	"High Master Healer"		},
    {	"High Master"		}
};


const struct rank_type conclave_table[] =
{
    {   ""		},
    {   "Initiate"	},
    {	"Apprentice"    },
    {	"Adept"  	},
    {	"Magus"		},
    {	"Master"	},
    {	"Spellmaster"	},
    {	"Taskmage"	},
    {	"Highmaster"	},
    {	"Archmage"	},
    {	"High Archmage"	},
    {	"High ArchMoon" }
};
    
const struct rank_type wildrunner_table[] =
{
    {   ""			},
    {   "Runner"		},
    {	"Private" 		},
    {	"Scout"			},
    {	"Ranger"		},
    {	"Seargant"		},
    {	"Major"   	    	},
    {	"Lieuteant"         	},
    {	"Captin"  		},
    {	"Commander"		},
    {	"General"		},
    {	"Holy Leader"		}
};

const struct rank_type mercenary_table[] =
{
    {   ""			},
    {   "Merchant"	        },
    {	"Thug"		        },
    {	"Hit-Man"		},
    {	"Sell Sword"		},
    {	"Bodyguard"		},
    {	"Executioner"		},
    {	"Bounty Hunter"		},
    {	"Assassin"	        },
    {	"Enforcer"	        },
    {	"Blademaster"	        },
    {	"Money Bags"	        }
};

const struct rank_type mithril_table[] =
{
    {   ""		        	},
    {   "Peon"		        	},
    {   "Bully"				},
    {	"Jerk" 				},
    {	"Liar"				},
    {	"Cutthroat"			},
    {	"Cheater"			},
    {	"Pillager"			},
    {	"Raider"			},
    {	"Defiler"			},
    {	"Overlord"			},
    {	"The boss?"        	        }
};

const struct rank_type undead_table[] =
{
    {   ""			},
    {   "Slave"			},
    {	"Servant"		},
    {	"Master"		},
    {	"Minion"		},
    {	"Lord"   		},
    {	"Ravager"		},
    {	"Baron" 		},
    {	"Soul Taker"		},
    {	"Death Lord"		},
    {	"Avatar"		},
    {   "God of Death"          }
};

const struct rank_type teyr_table[] =
{
    {   ""			},
    {   "Citizen"		},
    {	"Merchant"		},
    {	"Guard"			},
    {	"Soldier"		},
    {	"Commander"		},
    {	"Group Leader"		},
    {	"Council Member"	},
    {	"Clan General"		},
    {	"Supreme General"	},
    {	"Governor"		},
    {	"Governer"		}
};

const struct rank_type thieves_table[] =
{
    {   ""                      },               
    {   "Street Urchin"     },
    {   "Rogue"                 },
    {   "Cutpurse"              },
    {   "Burglar"               },
    {   "Bandit"                },
    {   "Cutthroat"             },
    {   "Shadow Walker"         },
    {   "Master Thief"      },
    {   "Kingpin"            },
    {   "Prince of Thieves"     },
    {   "King Liar"             }
};

/* for position */
const struct position_type position_table[] =
{
    {	"dead",			"dead"	},
    {	"mortally wounded",	"mort"	},
    {	"incapacitated",	"incap"	},
    {	"stunned",		"stun"	},
    {	"sleeping",		"sleep"	},
    {	"resting",		"rest"	},
    {   "sitting",		"sit"   },
    {	"fighting",		"fight"	},
    {	"standing",		"stand"	},
    {	"riding",		"ride"	},
    {	NULL,			NULL	}
};

/* for sex */
const struct sex_type sex_table[] =
{
   {	"none"		},
   {	"male"		},	
   {	"female"	},
   {	"either"	},
   {	NULL		}
};

/* for sizes */
const struct size_type size_table[] =
{ 
    {	"tiny"		},
    {	"small" 	},
    {	"medium"	},
    {	"large"		},
    {	"huge", 	},
    {	"giant" 	},
    {	NULL		}
};

/* various flag tables */
const struct flag_type act_flags[] =
{
    {	"npc",			A,	FALSE	},
    {	"sentinel",		B,	TRUE	},
    {	"scavenger",		C,	TRUE	},
    {	"aggressive",		F,	TRUE	},
    {	"stay_area",		G,	TRUE	},
    {	"wimpy",		H,	TRUE	},
    {	"pet",			I,	TRUE	},
    {	"train",		J,	TRUE	},
    {	"practice",		K,	TRUE	},
    {	"undead",		O,	TRUE	},
    {	"cleric",		Q,	TRUE	},
    {	"mage",			R,	TRUE	},
    {	"thief",		S,	TRUE	},
    {	"warrior",		T,	TRUE	},
    {	"noalign",		U,	TRUE	},
    {	"nopurge",		V,	TRUE	},
    {	"outdoors",		W,	TRUE	},
    {	"indoors",		Y,	TRUE	},
    {	"healer",		aa,	TRUE	},
    {	"gain",			bb,	TRUE	},
    {	"update_always",	cc,	TRUE	},
    {	"changer",		dd,	TRUE	},
    {	"mountable",		ee,	TRUE	},
    {	NULL,			0,	FALSE	}
};

const struct flag_type act2_flags[] =
{
    {	"banker",		A,	TRUE	},
    {	"gambler",		B,	TRUE	},
    {	"no_attack",		C,	TRUE	},
    {	"no_counter",		D,	TRUE	},
    {	NULL,			0,	FALSE	}
};

const struct flag_type plr_flags[] =
{
    {	"npc",			A,	FALSE	},
    {	"autoassist",		C,	FALSE	},
    {   "autodamage",           D,      FALSE   },
    {	"autoexit",		E,	FALSE	},
    {	"autoloot",		F,	FALSE	},
    {	"autosac",		G,	FALSE	},
    {	"autogold",		H,	FALSE	},
    {	"autosplit",		I,	FALSE	},
    {	"mounted",		J,	TRUE	},
    {	"holylight",		N,	FALSE	},
    {	"can_loot",		P,	FALSE	},
    {	"nosummon",		Q,	FALSE	},
    {	"nofollow",		R,	FALSE	},
    {	"colour",		T,	FALSE	},
    {	"permit",		U,	TRUE	},
    {	"log",			W,	FALSE	},
    {	"deny",			X,	FALSE	},
    {	"freeze",		Y,	FALSE	},
    {	"thief",		Z,	FALSE	},
    {	"killer",		aa,	FALSE	},
    {	NULL,			0,	0	}
};

const struct flag_type affect_flags[] =
{
    {	"blind",		A,	TRUE	},
    {	"invisible",		B,	TRUE	},
    {	"detect_evil",		C,	TRUE	},
    {	"detect_invis",		D,	TRUE	},
    {	"detect_magic",		E,	TRUE	},
    {	"detect_hidden",	F,	TRUE	},
    {	"detect_good",		G,	TRUE	},
    {	"sanctuary",		H,	TRUE	},
    {	"faerie_fire",		I,	TRUE	},
    {	"infrared",		J,	TRUE	},
    {	"curse",		K,	TRUE	},
    {	"poison",		M,	TRUE	},
    {	"protect_evil",		N,	TRUE	},
    {	"protect_good",		O,	TRUE	},
    {	"sneak",		P,	TRUE	},
    {	"hide",			Q,	TRUE	},
    {	"sleep",		R,	TRUE	},
    {	"charm",		S,	TRUE	},
    {	"flying",		T,	TRUE	},
    {	"pass_door",		U,	TRUE	},
    {	"haste",		V,	TRUE	},
    {	"calm",			W,	TRUE	},
    {	"plague",		X,	TRUE	},
    {	"weaken",		Y,	TRUE	},
    {	"dark_vision",		Z,	TRUE	},
    {	"berserk",		aa,	TRUE	},
    {	"swim",			bb,	TRUE	},
    {	"regeneration",		cc,	TRUE	},
    {	"slow",			dd,	TRUE	},
    {	"true_sight",		ee,	TRUE	},
    {	NULL,			0,	0	}
};

const struct flag_type affect2_flags[] =
{
    {	"enhanced_mind",	A,	TRUE	},
    {	"advanced_mind",	B,	TRUE	},
    {	"flameshield",		C,	TRUE	},
    {	"silence",			D,	TRUE	},
    {	"hallucinating",	E,	TRUE	},
    {	"blur",				F,	TRUE	},
    {   "lightningshield",	G,	TRUE	},
    {   "layhands",		H,	TRUE	},
    {   "flaming_eyes",		I,	TRUE	},
    {	NULL,			0,	0	}
};

const struct flag_type off_flags[] =
{
    {	"area_attack",		A,	TRUE	},
    {	"backstab",		B,	TRUE	},
    {	"bash",			C,	TRUE	},
    {	"berserk",		D,	TRUE	},
    {	"disarm",		E,	TRUE	},
    {	"dodge",		F,	TRUE	},
    {	"fade",			G,	TRUE	},
    {	"fast",			H,	TRUE	},
    {	"kick",			I,	TRUE	},
    {	"dirt_kick",		J,	TRUE	},
    {	"parry",		K,	TRUE	},
    {	"rescue",		L,	TRUE	},
    {	"tail",			M,	TRUE	},
    {	"trip",			N,	TRUE	},
    {	"crush",		O,	TRUE	},
    {	"assist_all",		P,	TRUE	},
    {	"assist_align",		Q,	TRUE	},
    {	"assist_race",		R,	TRUE	},
    {	"assist_players",	S,	TRUE	},
    {	"assist_guard",		T,	TRUE	},
    {	"assist_vnum",		U,	TRUE	},
    {	NULL,			0,	0	}
};

const struct flag_type imm_flags[] =
{
    {	"summon",		A,	TRUE	},
    {	"charm",		B,	TRUE	},
    {	"magic",		C,	TRUE	},
    {	"weapon",		D,	TRUE	},
    {	"bash",			E,	TRUE	},
    {	"pierce",		F,	TRUE	},
    {	"slash",		G,	TRUE	},
    {	"fire",			H,	TRUE	},
    {	"cold",			I,	TRUE	},
    {	"lightning",		J,	TRUE	},
    {	"acid",			K,	TRUE	},
    {	"poison",		L,	TRUE	},
    {	"negative",		M,	TRUE	},
    {	"holy",			N,	TRUE	},
    {	"energy",		O,	TRUE	},
    {	"mental",		P,	TRUE	},
    {	"disease",		Q,	TRUE	},
    {	"drowning",		R,	TRUE	},
    {	"light",		S,	TRUE	},
    {	"sound",		T,	TRUE	},
    {	"wood",			X,	TRUE	},
    {	"silver",		Y,	TRUE	},
    {	"iron",			Z,	TRUE	},
    {	NULL,			0,	0	}
};

const struct flag_type form_flags[] =
{
    {	"edible",		FORM_EDIBLE,		TRUE	},
    {	"poison",		FORM_POISON,		TRUE	},
    {	"magical",		FORM_MAGICAL,		TRUE	},
    {	"instant_decay",	FORM_INSTANT_DECAY,	TRUE	},
    {	"other",		FORM_OTHER,		TRUE	},
    {	"animal",		FORM_ANIMAL,		TRUE	},
    {	"sentient",		FORM_SENTIENT,		TRUE	},
    {	"undead",		FORM_UNDEAD,		TRUE	},
    {	"construct",		FORM_CONSTRUCT,		TRUE	},
    {	"mist",			FORM_MIST,		TRUE	},
    {	"intangible",		FORM_INTANGIBLE,	TRUE	},
    {	"biped",		FORM_BIPED,		TRUE	},
    {	"centaur",		FORM_CENTAUR,		TRUE	},
    {	"insect",		FORM_INSECT,		TRUE	},
    {	"spider",		FORM_SPIDER,		TRUE	},
    {	"crustacean",		FORM_CRUSTACEAN,	TRUE	},
    {	"worm",			FORM_WORM,		TRUE	},
    {	"blob",			FORM_BLOB,		TRUE	},
    {	"mammal",		FORM_MAMMAL,		TRUE	},
    {	"bird",			FORM_BIRD,		TRUE	},
    {	"reptile",		FORM_REPTILE,		TRUE	},
    {	"snake",		FORM_SNAKE,		TRUE	},
    {	"dragon",		FORM_DRAGON,		TRUE	},
    {	"amphibian",		FORM_AMPHIBIAN,		TRUE	},
    {	"fish",			FORM_FISH ,		TRUE	},
    {	"cold_blood",		FORM_COLD_BLOOD,	TRUE	},
    {	NULL,			0,			0	}
};

const struct flag_type part_flags[] =
{
    {	"head",			PART_HEAD,		TRUE	},
    {	"arms",			PART_ARMS,		TRUE	},
    {	"legs",			PART_LEGS,		TRUE	},
    {	"heart",		PART_HEART,		TRUE	},
    {	"brains",		PART_BRAINS,		TRUE	},
    {	"guts",			PART_GUTS,		TRUE	},
    {	"hands",		PART_HANDS,		TRUE	},
    {	"feet",			PART_FEET,		TRUE	},
    {	"fingers",		PART_FINGERS,		TRUE	},
    {	"ear",			PART_EAR,		TRUE	},
    {	"eye",			PART_EYE,		TRUE	},
    {	"long_tongue",		PART_LONG_TONGUE,	TRUE	},
    {	"eyestalks",		PART_EYESTALKS,		TRUE	},
    {	"tentacles",		PART_TENTACLES,		TRUE	},
    {	"fins",			PART_FINS,		TRUE	},
    {	"wings",		PART_WINGS,		TRUE	},
    {	"tail",			PART_TAIL,		TRUE	},
    {	"claws",		PART_CLAWS,		TRUE	},
    {	"fangs",		PART_FANGS,		TRUE	},
    {	"horns",		PART_HORNS,		TRUE	},
    {	"scales",		PART_SCALES,		TRUE	},
    {	"tusks",		PART_TUSKS,		TRUE	},
    {	NULL,			0,			0	}
};

const struct flag_type comm_flags[] =
{
    {	"quiet",		COMM_QUIET,		TRUE	},
    {   "deaf",			COMM_DEAF,		TRUE	},
    {   "nowiz",		COMM_NOWIZ,		TRUE	},
    {   "noic",			COMM_NOIC,		TRUE	},
    {   "nomusic",		COMM_NOMUSIC,		TRUE	},
    {   "noclan",		COMM_NOCLAN,		TRUE	},
    {   "noosay",		COMM_NOOSAY,		TRUE	},
    {   "compact",		COMM_COMPACT,		TRUE	},
    {   "brief",		COMM_BRIEF,		TRUE	},
    {   "prompt",		COMM_PROMPT,		TRUE	},
    {   "combine",		COMM_COMBINE,		TRUE	},
    {   "telnet_ga",		COMM_TELNET_GA,		TRUE	},
    {   "show_affects",		COMM_SHOW_AFFECTS,	TRUE	},
    {   "nograts",		COMM_NOGRATS,		TRUE	},
    {   "noemote",		COMM_NOEMOTE,		FALSE	},
    {   "noosay",		COMM_NOOSAY,		FALSE	},
    {   "notell",		COMM_NOTELL,		FALSE	},
    {   "nochannels",		COMM_NOCHANNELS,	FALSE	},
    {   "nopray",               COMM_NOPRAY,            TRUE    },
    {   "snoop_proof",		COMM_SNOOP_PROOF,	FALSE	},
    {   "afk",			COMM_AFK,		TRUE	},
    {   "iaw",			COMM_IAW,		TRUE	},
    {	NULL,			0,			0	}
};

const struct flag_type mprog_flags[] =
{
    {	"act",			TRIG_ACT,		TRUE	},
    {	"bribe",		TRIG_BRIBE,		TRUE 	},
    {	"death",		TRIG_DEATH,		TRUE    },
    {	"entry",		TRIG_ENTRY,		TRUE	},
    {	"fight",		TRIG_FIGHT,		TRUE	},
    {	"give",			TRIG_GIVE,		TRUE	},
    {	"greet",		TRIG_GREET,		TRUE    },
    {	"grall",		TRIG_GRALL,		TRUE	},
    {	"kill",			TRIG_KILL,		TRUE	},
    {	"hpcnt",		TRIG_HPCNT,		TRUE    },
    {	"random",		TRIG_RANDOM,		TRUE	},
    {	"speech",		TRIG_SPEECH,		TRUE	},
    {	"exit",			TRIG_EXIT,		TRUE    },
    {	"exall",		TRIG_EXALL,		TRUE    },
    {	"delay",		TRIG_DELAY,		TRUE    },
    {	"surr",			TRIG_SURR,		TRUE    },
    {	NULL,			0,			TRUE	}
};

const struct flag_type area_flags[] =
{
    {	"none",			AREA_NONE,		FALSE	},
    {	"changed",		AREA_CHANGED,		TRUE	},
    {	"added",		AREA_ADDED,		TRUE	},
    {	"loading",		AREA_LOADING,		FALSE	},
    {	NULL,			0,			0	}
};



const struct flag_type sex_flags[] =
{
    {	"male",			SEX_MALE,		TRUE	},
    {	"female",		SEX_FEMALE,		TRUE	},
    {	"neutral",		SEX_NEUTRAL,		TRUE	},
    {   "random",               3,                      TRUE    },   /* ROM */
    {	"none",			SEX_NEUTRAL,		TRUE	},
    {	NULL,			0,			0	}
};



const struct flag_type exit_flags[] =
{
    {   "door",			EX_ISDOOR,		TRUE    },
    {	"closed",		EX_CLOSED,		TRUE	},
    {	"locked",		EX_LOCKED,		TRUE	},
    {	"pickproof",		EX_PICKPROOF,		TRUE	},
    {   "nopass",		EX_NOPASS,		TRUE	},
    {   "easy",			EX_EASY,		TRUE	},
    {   "hard",			EX_HARD,		TRUE	},
    {	"infuriating",		EX_INFURIATING,		TRUE	},
    {	"noclose",		EX_NOCLOSE,		TRUE	},
    {	"nolock",		EX_NOLOCK,		TRUE	},
    {	"hidden",		EX_HIDDEN,		TRUE	},
    {	NULL,			0,			0	}
};



const struct flag_type door_resets[] =
{
    {	"open and unlocked",	0,		TRUE	},
    {	"closed and unlocked",	1,		TRUE	},
    {	"closed and locked",	2,		TRUE	},
    {	NULL,			0,		0	}
};



const struct flag_type room_flags[] =
{
    {	"dark",			ROOM_DARK,		TRUE	},
    {	"no_mob",		ROOM_NO_MOB,		TRUE	},
    {	"indoors",		ROOM_INDOORS,		TRUE	},
    {	"private",		ROOM_PRIVATE,		TRUE    },
    {	"safe",			ROOM_SAFE,		TRUE	},
    {	"solitary",		ROOM_SOLITARY,		TRUE	},
    {	"pet_shop",		ROOM_PET_SHOP,		TRUE	},
    {	"imp_only",		ROOM_IMP_ONLY,		TRUE    },
    {	"gods_only",	        ROOM_GODS_ONLY,		TRUE    },
    {	"heroes_only",		ROOM_HEROES_ONLY,	TRUE	},
    {	"newbies_only",		ROOM_NEWBIES_ONLY,	TRUE	},
    {	"law",			ROOM_LAW,		TRUE	},
    {   "nowhere",		ROOM_NOWHERE,		TRUE	},
    {	"no_recall",		ROOM_NO_RECALL,		TRUE	},
    {	"no_trans",		ROOM_NO_TRANS,		TRUE	},
    {	"no_summon",		ROOM_NO_SUMMON,		TRUE	},
    {	"no_gate",		ROOM_NO_GATE,		TRUE	},
    {	"no_scrye",		ROOM_NO_SCRYE,		TRUE	},
    {	"nomagic",		ROOM_NOMAGIC,		TRUE	},
    {	NULL,			0,			0	}
};



const struct flag_type sector_flags[] =
{
    {	"inside",	SECT_INSIDE,		TRUE	},
    {	"city",		SECT_CITY,		TRUE	},
    {	"field",	SECT_FIELD,		TRUE	},
    {	"forest",	SECT_FOREST,		TRUE	},
    {	"hills",	SECT_HILLS,		TRUE	},
    {	"mountain",	SECT_MOUNTAIN,		TRUE	},
    {	"swim",		SECT_WATER_SWIM,	TRUE	},
    {	"noswim",	SECT_WATER_NOSWIM,	TRUE	},
    {   "unused",	SECT_UNUSED,		TRUE	},
    {	"air",		SECT_AIR,		TRUE	},
    {	"desert",	SECT_DESERT,		TRUE	},
    {   "underground",  SECT_UNDERGROUND,       TRUE    },   
    {	"arena",	SECT_PKARENA,		TRUE	},
    {	NULL,		0,			0	}
};



const struct flag_type type_flags[] =
{
    {	"light",		ITEM_LIGHT,		TRUE	},
    {	"scroll",		ITEM_SCROLL,		TRUE	},
    {	"wand",			ITEM_WAND,		TRUE	},
    {	"staff",		ITEM_STAFF,		TRUE	},
    {	"weapon",		ITEM_WEAPON,		TRUE	},
    {	"treasure",		ITEM_TREASURE,		TRUE	},
    {	"armor",		ITEM_ARMOR,		TRUE	},
    {	"potion",		ITEM_POTION,		TRUE	},
    {	"furniture",		ITEM_FURNITURE,		TRUE	},
    {	"trash",		ITEM_TRASH,		TRUE	},
    {	"container",		ITEM_CONTAINER,		TRUE	},
    {	"drinkcontainer",	ITEM_DRINK_CON,		TRUE	},
    {	"key",			ITEM_KEY,		TRUE	},
    {	"food",			ITEM_FOOD,		TRUE	},
    {	"money",		ITEM_MONEY,		TRUE	},
    {	"boat",			ITEM_BOAT,		TRUE	},
    {	"npccorpse",		ITEM_CORPSE_NPC,	TRUE	},
    {	"pc corpse",		ITEM_CORPSE_PC,		FALSE	},
    {	"fountain",		ITEM_FOUNTAIN,		TRUE	},
    {	"pill",			ITEM_PILL,		TRUE	},
    {	"protect",		ITEM_PROTECT,		TRUE	},
    {	"map",			ITEM_MAP,		TRUE	},
    {   "portal",		ITEM_PORTAL,		TRUE	},
    {   "warpstone",		ITEM_WARP_STONE,	TRUE	},
    {	"roomkey",		ITEM_ROOM_KEY,		TRUE	},
    { 	"gem",			ITEM_GEM,		TRUE	},
    {	"jewelry",		ITEM_JEWELRY,		TRUE	},
    {	"jukebox",		ITEM_JUKEBOX,		TRUE	},
    {	"parchment",		ITEM_PARCHMENT,		TRUE	},
    {	"pen",			ITEM_PEN,		TRUE	},
    {	"spell",		ITEM_SPELL,		TRUE	},
    {	"all",			ITEM_SHOP_ALL,		FALSE	},
    {	NULL,			0,			0	}
};


const struct flag_type extra_flags[] =
{
    {	"glow",			ITEM_GLOW,		TRUE	},
    {	"hum",			ITEM_HUM,		TRUE	},
    {	"dark",			ITEM_DARK,		TRUE	},
    {	"lock",			ITEM_LOCK,		TRUE	},
    {	"evil",			ITEM_EVIL,		TRUE	},
    {	"invis",		ITEM_INVIS,		TRUE	},
    {	"magic",		ITEM_MAGIC,		TRUE	},
    {	"nodrop",		ITEM_NODROP,		TRUE	},
    {	"bless",		ITEM_BLESS,		TRUE	},
    {	"antigood",		ITEM_ANTI_GOOD,		TRUE	},
    {	"antievil",		ITEM_ANTI_EVIL,		TRUE	},
    {	"antineutral",		ITEM_ANTI_NEUTRAL,	TRUE	},
    {	"noremove",		ITEM_NOREMOVE,		TRUE	},
    {	"inventory",		ITEM_INVENTORY,		TRUE	},
    {	"nopurge",		ITEM_NOPURGE,		TRUE	},
    {	"rotdeath",		ITEM_ROT_DEATH,		TRUE	},
    {	"visdeath",		ITEM_VIS_DEATH,		TRUE	},
    {   "nonmetal",		ITEM_NONMETAL,		TRUE	},
    {	"meltdrop",		ITEM_MELT_DROP,		TRUE	},
    {	"hadtimer",		ITEM_HAD_TIMER,		TRUE	},
    {	"sellextract",		ITEM_SELL_EXTRACT,	TRUE	},
    {	"burnproof",		ITEM_BURN_PROOF,	TRUE	},
    {	"nouncurse",		ITEM_NOUNCURSE,		TRUE	},
    {	"noiden",		ITEM_NOIDEN,		TRUE	},
    {	NULL,			0,			0	}
};



const struct flag_type wear_flags[] =
{
    {	"take",			ITEM_TAKE,		TRUE	},
    {	"finger",		ITEM_WEAR_FINGER,	TRUE	},
    {	"neck",			ITEM_WEAR_NECK,		TRUE	},
    {	"body",			ITEM_WEAR_BODY,		TRUE	},
    {	"head",			ITEM_WEAR_HEAD,		TRUE	},
    {	"legs",			ITEM_WEAR_LEGS,		TRUE	},
    {	"feet",			ITEM_WEAR_FEET,		TRUE	},
    {	"hands",		ITEM_WEAR_HANDS,	TRUE	},
    {	"arms",			ITEM_WEAR_ARMS,		TRUE	},
    {	"shield",		ITEM_WEAR_SHIELD,	TRUE	},
    {	"about",		ITEM_WEAR_ABOUT,	TRUE	},
    {	"waist",		ITEM_WEAR_WAIST,	TRUE	},
    {	"wrist",		ITEM_WEAR_WRIST,	TRUE	},
    {	"wield",		ITEM_WIELD,		TRUE	},
    {	"hold",			ITEM_HOLD,		TRUE	},
    {   "nosac",		ITEM_NO_SAC,		TRUE	},
    {	"wearfloat",		ITEM_WEAR_FLOAT,	TRUE	},
    {	"ankle",		ITEM_WEAR_ANKLE,	TRUE	},
    {	"ear",			ITEM_WEAR_EAR,		TRUE	},
    {   "back",                 ITEM_WEAR_BACK,         TRUE    },
    {   "tattoo",               ITEM_WEAR_TATTOO,       TRUE    },
    {   "face",                 ITEM_WEAR_FACE,         TRUE    },
/*    {   "twohands",            ITEM_TWO_HANDS,         TRUE    }, */
    {	NULL,			0,			0	}
};

const struct flag_type class_flags[] = 
{
    {	"mage",			ITEM_MAGE_ONLY,		TRUE	},
    {	"cleric",		ITEM_CLERIC_ONLY,	TRUE	},
    {	"thief",		ITEM_THIEF_ONLY,	TRUE	},
    {	"warrior",		ITEM_WARRIOR_ONLY,	TRUE	},
    {	"paladin",		ITEM_PALADIN_ONLY,	TRUE	},
    {	"ranger",		ITEM_RANGER_ONLY,	TRUE	},
    {	"templar",		ITEM_TEMPLAR_ONLY,	TRUE	},
    {	"bmage",		ITEM_BMAGE_ONLY,	TRUE	},
    {	NULL,			0,			0	}
};

/*
 * Used when adding an affect to tell where it goes.
 * See addaffect and delaffect in act_olc.c
 */
const struct flag_type apply_flags[] =
{
    {	"none",			APPLY_NONE,		TRUE	},
    {	"strength",		APPLY_STR,		TRUE	},
    {	"dexterity",		APPLY_DEX,		TRUE	},
    {	"intelligence",		APPLY_INT,		TRUE	},
    {	"wisdom",		APPLY_WIS,		TRUE	},
    {	"constitution",		APPLY_CON,		TRUE	},
    {	"sex",			APPLY_SEX,		TRUE	},
    {	"class",		APPLY_CLASS,		TRUE	},
    {	"level",		APPLY_LEVEL,		TRUE	},
    {	"age",			APPLY_AGE,		TRUE	},
    {	"height",		APPLY_HEIGHT,		TRUE	},
    {	"weight",		APPLY_WEIGHT,		TRUE	},
    {	"mana",			APPLY_MANA,		TRUE	},
    {	"hp",			APPLY_HIT,		TRUE	},
    {	"move",			APPLY_MOVE,		TRUE	},
    {	"steel",		APPLY_STEEL,		TRUE	},
    {	"experience",		APPLY_EXP,		TRUE	},
    {	"ac",			APPLY_AC,		TRUE	},
    {	"hitroll",		APPLY_HITROLL,		TRUE	},
    {	"damroll",		APPLY_DAMROLL,		TRUE	},
    {	"saves",		APPLY_SAVES,		TRUE	},
    {	"savingpara",		APPLY_SAVING_PARA,	TRUE	},
    {	"savingrod",		APPLY_SAVING_ROD,	TRUE	},
    {	"savingpetri",		APPLY_SAVING_PETRI,	TRUE	},
    {	"savingbreath",		APPLY_SAVING_BREATH,	TRUE	},
    {	"savingspell",		APPLY_SAVING_SPELL,	TRUE	},
    {	"spellaffect",		APPLY_SPELL_AFFECT,	FALSE	},
    {	NULL,			0,			0	}
};



/*
 * What is seen.
 */
const struct flag_type wear_loc_strings[] =
{
    {	"in the inventory",	WEAR_NONE,	TRUE	},
    {	"as a light",		WEAR_LIGHT,	TRUE	},
    {	"on the left finger",	WEAR_FINGER_L,	TRUE	},
    {	"on the right finger",	WEAR_FINGER_R,	TRUE	},
    {	"around the neck (1)",	WEAR_NECK_1,	TRUE	},
    {	"around the neck (2)",	WEAR_NECK_2,	TRUE	},
    {	"on the body",		WEAR_BODY,	TRUE	},
    {	"over the head",	WEAR_HEAD,	TRUE	},
    {	"on the legs",		WEAR_LEGS,	TRUE	},
    {	"on the feet",		WEAR_FEET,	TRUE	},
    {	"on the hands",		WEAR_HANDS,	TRUE	},
    {	"on the arms",		WEAR_ARMS,	TRUE	},
    {	"as a shield",		WEAR_SHIELD,	TRUE	},
    {	"about the shoulders",	WEAR_ABOUT,	TRUE	},
    {	"around the waist",	WEAR_WAIST,	TRUE	},
    {	"on the left wrist",	WEAR_WRIST_L,	TRUE	},
    {	"on the right wrist",	WEAR_WRIST_R,	TRUE	},
    {	"in the right hand",	WEAR_RIGHT,	TRUE	},
    {	"in the left hand",	WEAR_LEFT,	TRUE	},
    {	"floating nearby",	WEAR_FLOAT,	TRUE	},
    {	"on the ankle",		WEAR_ANKLE,	TRUE	},
    {	"on the ear",		WEAR_EAR,	TRUE	},
    {   "on the back",          WEAR_BACK,      TRUE    },
    {   "on the left arm",      WEAR_TATTOO_L,  TRUE    },
    {   "on the right arm",     WEAR_TATTOO_R,  TRUE    },
    {   "on the face",          WEAR_FACE,      TRUE    },
    {	NULL,			0	      , 0	}
};


const struct flag_type wear_loc_flags[] =
{
    {	"none",		WEAR_NONE,	TRUE	},
    {	"light",	WEAR_LIGHT,	TRUE	},
    {	"lfinger",	WEAR_FINGER_L,	TRUE	},
    {	"rfinger",	WEAR_FINGER_R,	TRUE	},
    {	"neck1",	WEAR_NECK_1,	TRUE	},
    {	"neck2",	WEAR_NECK_2,	TRUE	},
    {	"body",		WEAR_BODY,	TRUE	},
    {	"head",		WEAR_HEAD,	TRUE	},
    {	"legs",		WEAR_LEGS,	TRUE	},
    {	"feet",		WEAR_FEET,	TRUE	},
    {	"hands",	WEAR_HANDS,	TRUE	},
    {	"arms",		WEAR_ARMS,	TRUE	},
    {	"shield",	WEAR_SHIELD,	TRUE	},
    {	"about",	WEAR_ABOUT,	TRUE	},
    {	"waist",	WEAR_WAIST,	TRUE	},
    {	"lwrist",	WEAR_WRIST_L,	TRUE	},
    {	"rwrist",	WEAR_WRIST_R,	TRUE	},
    {	"rhand",	WEAR_RIGHT,	TRUE	},
    {	"lhand",	WEAR_LEFT,	TRUE	},
    {	"floating",	WEAR_FLOAT,	TRUE	},
    {   "ankle",	WEAR_ANKLE,	TRUE	},
    {   "ear",		WEAR_EAR,	TRUE	},
    {   "back",         WEAR_BACK,      TRUE    },
    {   "ltattoo",      WEAR_TATTOO_L,  TRUE    },
    {   "rtattoo",      WEAR_TATTOO_R,  TRUE    },
    {   "face",         WEAR_FACE,      TRUE    },
    {	NULL,		0,		0	}
};

const struct flag_type container_flags[] =
{
    {	"closeable",		1,		TRUE	},
    {	"pickproof",		2,		TRUE	},
    {	"closed",		4,		TRUE	},
    {	"locked",		8,		TRUE	},
    {	"puton",		16,		TRUE	},
    {	NULL,			0,		0	}
};

/*****************************************************************************
                      ROM - specific tables:
 ****************************************************************************/




const struct flag_type ac_type[] =
{
    {   "pierce",        AC_PIERCE,            TRUE    },
    {   "bash",          AC_BASH,              TRUE    },
    {   "slash",         AC_SLASH,             TRUE    },
    {   "exotic",        AC_EXOTIC,            TRUE    },
    {   NULL,              0,                    0       }
};


const struct flag_type size_flags[] =
{
    {   "tiny",          SIZE_TINY,            TRUE    },
    {   "small",         SIZE_SMALL,           TRUE    },
    {   "medium",        SIZE_MEDIUM,          TRUE    },
    {   "large",         SIZE_LARGE,           TRUE    },
    {   "huge",          SIZE_HUGE,            TRUE    },
    {   "giant",         SIZE_GIANT,           TRUE    },
    {   NULL,              0,                    0       },
};


const struct flag_type weapon_class[] =
{
    {   "exotic",	WEAPON_EXOTIC,		TRUE    },
    {   "sword",	WEAPON_SWORD,		TRUE    },
    {   "dagger",	WEAPON_DAGGER,		TRUE    },
    {   "spear",	WEAPON_SPEAR,		TRUE    },
    {   "mace",		WEAPON_MACE,		TRUE    },
    {   "axe",		WEAPON_AXE,		TRUE    },
    {   "flail",	WEAPON_FLAIL,		TRUE    },
    {   "whip",		WEAPON_WHIP,		TRUE    },
    {   "polearm",	WEAPON_POLEARM,		TRUE    },
    {	"lance",	WEAPON_LANCE,		TRUE	},
    {   NULL,		0,			0       }
};


const struct flag_type weapon_type2[] =
{
    {   "flaming",       WEAPON_FLAMING,       TRUE    },
    {   "frost",         WEAPON_FROST,         TRUE    },
    {   "vampiric",      WEAPON_VAMPIRIC,      TRUE    },
    {   "sharp",         WEAPON_SHARP,         TRUE    },
    {   "vorpal",        WEAPON_VORPAL,        TRUE    },
    {   "twohands",     WEAPON_TWO_HANDS,     TRUE    },
    {	"shocking",	 WEAPON_SHOCKING,      TRUE    },
    {	"poison",	WEAPON_POISON,		TRUE	},
    {   "acidic", 	 WEAPON_ACIDIC,		TRUE	},
    {   NULL,              0,                    0       }
};

const struct flag_type res_flags[] =
{
    {	"summon",	 RES_SUMMON,		TRUE	},
    {   "charm",         RES_CHARM,            TRUE    },
    {   "magic",         RES_MAGIC,            TRUE    },
    {   "weapon",        RES_WEAPON,           TRUE    },
    {   "bash",          RES_BASH,             TRUE    },
    {   "pierce",        RES_PIERCE,           TRUE    },
    {   "slash",         RES_SLASH,            TRUE    },
    {   "fire",          RES_FIRE,             TRUE    },
    {   "cold",          RES_COLD,             TRUE    },
    {   "lightning",     RES_LIGHTNING,        TRUE    },
    {   "acid",          RES_ACID,             TRUE    },
    {   "poison",        RES_POISON,           TRUE    },
    {   "negative",      RES_NEGATIVE,         TRUE    },
    {   "holy",          RES_HOLY,             TRUE    },
    {   "energy",        RES_ENERGY,           TRUE    },
    {   "mental",        RES_MENTAL,           TRUE    },
    {   "disease",       RES_DISEASE,          TRUE    },
    {   "drowning",      RES_DROWNING,         TRUE    },
    {   "light",         RES_LIGHT,            TRUE    },
    {	"sound",	RES_SOUND,		TRUE	},
    {	"wood",		RES_WOOD,		TRUE	},
    {	"silver",	RES_SILVER,		TRUE	},
    {	"iron",		RES_IRON,		TRUE	},
    {   NULL,          0,            0    }
};


const struct flag_type vuln_flags[] =
{
    {	"summon",	 VULN_SUMMON,		TRUE	},
    {	"charm",	VULN_CHARM,		TRUE	},
    {   "magic",         VULN_MAGIC,           TRUE    },
    {   "weapon",        VULN_WEAPON,          TRUE    },
    {   "bash",          VULN_BASH,            TRUE    },
    {   "pierce",        VULN_PIERCE,          TRUE    },
    {   "slash",         VULN_SLASH,           TRUE    },
    {   "fire",          VULN_FIRE,            TRUE    },
    {   "cold",          VULN_COLD,            TRUE    },
    {   "lightning",     VULN_LIGHTNING,       TRUE    },
    {   "acid",          VULN_ACID,            TRUE    },
    {   "poison",        VULN_POISON,          TRUE    },
    {   "negative",      VULN_NEGATIVE,        TRUE    },
    {   "holy",          VULN_HOLY,            TRUE    },
    {   "energy",        VULN_ENERGY,          TRUE    },
    {   "mental",        VULN_MENTAL,          TRUE    },
    {   "disease",       VULN_DISEASE,         TRUE    },
    {   "drowning",      VULN_DROWNING,        TRUE    },
    {   "light",         VULN_LIGHT,           TRUE    },
    {	"sound",	 VULN_SOUND,		TRUE	},
    {   "wood",          VULN_WOOD,            TRUE    },
    {   "silver",        VULN_SILVER,          TRUE    },
    {   "iron",          VULN_IRON,            TRUE    },
    {   NULL,              0,                    0       }
};

const struct flag_type position_flags[] =
{
    {   "dead",           POS_DEAD,            FALSE   },
    {   "mortal",         POS_MORTAL,          FALSE   },
    {   "incap",          POS_INCAP,           FALSE   },
    {   "stunned",        POS_STUNNED,         FALSE   },
    {   "sleeping",       POS_SLEEPING,        TRUE    },
    {   "resting",        POS_RESTING,         TRUE    },
    {   "sitting",        POS_SITTING,         TRUE    },
    {   "fighting",       POS_FIGHTING,        FALSE   },
    {   "standing",       POS_STANDING,        TRUE    },
    {   NULL,              0,                    0       }
};

const struct flag_type portal_flags[]=
{
    {   "normal_exit",	  GATE_NORMAL_EXIT,	TRUE	},
    {	"no_curse",	  GATE_NOCURSE,		TRUE	},
    {   "go_with",	  GATE_GOWITH,		TRUE	},
    {   "buggy",	  GATE_BUGGY,		TRUE	},
    {	"random",	  GATE_RANDOM,		TRUE	},
    {   NULL,		  0,			0	}
};

const struct flag_type furniture_flags[]=
{
    {   "stand_at",	  STAND_AT,		TRUE	},
    {	"stand_on",	  STAND_ON,		TRUE	},
    {	"stand_in",	  STAND_IN,		TRUE	},
    {	"sit_at",	  SIT_AT,		TRUE	},
    {	"sit_on",	  SIT_ON,		TRUE	},
    {	"sit_in",	  SIT_IN,		TRUE	},
    {	"rest_at",	  REST_AT,		TRUE	},
    {	"rest_on",	  REST_ON,		TRUE	},
    {	"rest_in",	  REST_IN,		TRUE	},
    {	"sleep_at",	  SLEEP_AT,		TRUE	},
    {	"sleep_on",	  SLEEP_ON,		TRUE	},
    {	"sleep_in",	  SLEEP_IN,		TRUE	},
    {	"put_at",	  PUT_AT,		TRUE	},
    {	"put_on",	  PUT_ON,		TRUE	},
    {	"put_in",	  PUT_IN,		TRUE	},
    {	"put_inside",	  PUT_INSIDE,		TRUE	},
    {	NULL,		  0,			0	}
};

const	struct	flag_type	apply_types	[]	=
{
	{	"affects",	TO_AFFECTS,	TRUE	},
	{	"affect2",	TO_AFFECTS2,	TRUE	},
	{	"object",	TO_OBJECT,	TRUE	},
	{	"immune",	TO_IMMUNE,	TRUE	},
	{	"resist",	TO_RESIST,	TRUE	},
	{	"vuln",		TO_VULN,	TRUE	},
	{	"weapon",	TO_WEAPON,	TRUE	},
	{	NULL,		0,		TRUE	}
};

const	struct	bit_type	bitvector_type	[]	=
{
	{	affect_flags,	"affect"	},
	{	affect2_flags,  "affect2"	},
	{	apply_flags,	"apply"		},
	{	imm_flags,	"imm"		},
	{	res_flags,	"res"		},
	{	vuln_flags,	"vuln"		},
	{	weapon_type2,	"weapon"	}
};


