/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Starfeldt, Tom Madsen, and Katja Nyboe.   *
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
/******************************************************
 * ..........   SandStorm:Mages Sanctuary             *
 *  ........    By David Simmerson                    *
 * .......      2001-2002(c) David Simmerson          *
 *  ....        email:ldevil@hotmail.com              *
 *   ..                                               *
 *    .           Can you survive the sandstorm?      *
 ******************************************************/
/********************************************************************
 * The Dark forgotten is a ROM Derivative. Rom was made with Merc2.2*
 * By Russ Taylor.  In order for you to use this code, you must     *
 * agree to the Licence Merc, Rom, and Diku Licences, Along with the*
 * TDF License file.                                                *
 * The Dark Forgotten was written by David Simmerson(aka SoEnSo)    *
 * Contact him at.  ldevil@hotmail.com for questions or comments    *
 * about the TDF source.. He may even help you.                     *
 ********************************************************************/
/* Online Social Editting Module, 
 * (c) 1996,97 Erwin S. Andreasen <erwin@andreasen.org>
 * See the file "License" for important licensing information
 */

/* This version contains minor modifications to support ROM 2.4b4. */

#if defined(WIN32)
#include <sys/types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "db.h"
#include "tables.h"
#include "olc.h"

#define XSOCIAL_FILE "../area/xsocials.txt"

int maxXSocial;

struct xsocial_type *xsocial_table;	/* and social table */

#define XSEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
void
load_xsocial (FILE * fp, struct xsocial_type *social)
{
  social->name = fread_string (fp);
  social->char_no_arg = fread_string (fp);
  social->others_no_arg = fread_string (fp);
  social->char_found = fread_string (fp);
  social->others_found = fread_string (fp);
  social->vict_found = fread_string (fp);
  social->char_auto = fread_string (fp);
  social->others_auto = fread_string (fp);
  social->gender = fread_number(fp);
  social->stage = fread_number(fp);
  social->position = fread_number(fp);
  social->self = fread_number(fp);
  social->other = fread_number(fp);
  social->extra = fread_number(fp);
  social->chance = fread_number(fp);
}

void
load_xsocial_table ()
{
  FILE *fp;
  int i;

  fp = file_open (XSOCIAL_FILE, "r");

  if (!fp)
    {
      bug ("Could not open " XSOCIAL_FILE " for reading.", 0);
      exit (1);
    }

  fscanf (fp, "%d\n", &maxXSocial);

  /* IMPORTANT to use malloc so we can realloc later on */
  xsocial_table = malloc (sizeof (struct xsocial_type) * (maxXSocial + 1));

  for (i = 0; i < maxXSocial; i++)
    load_xsocial (fp, &xsocial_table[i]);

  /* For backwards compatibility */

  xsocial_table[maxXSocial].name = str_dup ("");	/* empty! */
  file_close (fp);
  return;
}

void
save_xsocial (const struct xsocial_type *s, FILE * fp)
{
  /* get rid of (null) */
  fprintf (fp, "%s~\n",  s->name ? s->name : "");
  fprintf (fp, "%s~\n",  s->char_no_arg ? s->char_no_arg : "");
  fprintf (fp, "%s~\n",  s->others_no_arg ? s->others_no_arg : "");
  fprintf (fp, "%s~\n",  s->char_found ? s->char_found : "");
  fprintf (fp, "%s~\n",  s->others_found ? s->others_found : "");
  fprintf (fp, "%s~\n",  s->vict_found ? s->vict_found : "");
  fprintf (fp,"%s~\n%s~\n%d %d %d %d %d %d %d\n\n",
	   s->char_auto ? s->char_auto : "",
	   s->others_auto ? s->others_auto : "",
       	   s->gender,	   s->stage,	   s->position,
	   s->self,	   s->other,	   s->extra,
	   s->chance);

}

void
save_xsocial_table ()
{
  FILE *fp =NULL;
  int i;

  fp = file_open (XSOCIAL_FILE, "w");

  if (!fp)
    {
      bug ("Could not open " XSOCIAL_FILE " for writing.", 0);
      return;
    }

  fprintf (fp, "%d\n", maxXSocial);

  for (i = 0; i < maxXSocial; i++)
    save_xsocial (&xsocial_table[i], fp);

  file_close (fp);
}

/*This is for when your making the fresh table from the stuff in the code.*/
#ifdef SAVEEX
void save_xsocials_to_table()
{
  FILE *fp = NULL;
  long int i;
  long int count_xsocials =0;
  fp = file_open(XSOCIAL_FILE, "w");

  if (!fp)
    {
      bug ("Could not open " XSOCIAL_FILE " for writing.", 0);
      return;
    }

  logstr(LOG_GAME,"Saving Xsocial File");

  for (i = 0; xsocial_table[i].name != '\0'; i++)
      count_xsocials++;

  fprintf (fp, "%ld\n", count_xsocials);

  /* get rid of (null) */
  for (i = 0;  xsocial_table[i].name !='\0'; i++)
  {
   fprintf (fp, "%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n",
	   xsocial_table[i].name ? xsocial_table[i].name : "",  
           xsocial_table[i].char_no_arg ? 	   xsocial_table[i].char_no_arg : "",
	   xsocial_table[i].others_no_arg ? 	   xsocial_table[i].others_no_arg : "",
	   xsocial_table[i].char_found ? 	   xsocial_table[i].char_found : "",
	   xsocial_table[i].others_found ? 	   xsocial_table[i].others_found : "",
	   xsocial_table[i].vict_found ? 	   xsocial_table[i].vict_found : "",
	   xsocial_table[i].char_auto ? 	   xsocial_table[i].char_auto : "",
	   xsocial_table[i].others_auto ? 	   xsocial_table[i].others_auto : "");
   fprintf(fp,"%d %d %d %d %d %d %d\n\r",
       	   xsocial_table[i].gender,	   xsocial_table[i].stage,	   xsocial_table[i].position,
	   xsocial_table[i].self,	   xsocial_table[i].other,	   xsocial_table[i].extra,
	   xsocial_table[i].chance);
   }
  file_close (fp);
  logstr(LOG_GAME,"Done saving xsocial file");
}
#endif

/* Find a social based on name */
int
xsocial_lookup (const char *name)
{
  int i;

  for (i = 0; i < maxXSocial; i++)
    if (!str_cmp (name, xsocial_table[i].name))
      return i;

  return -1;
}

/*
 * Social editting command
 */

XSOCIAL_DATA *
get_xsocial_data (char *name)
{
  int i;

  for (i = 0; i < maxXSocial; i++)
    if (!str_cmp (name, xsocial_table[i].name))
      return &xsocial_table[i];
  return NULL;
}

XSEDIT (xsedit_show)
{
  XSOCIAL_DATA *pSocial;
//    char buf[MSL];

  if (IS_NULLSTR (argument))
    EDIT_XSOCIAL (ch, pSocial);
  else
    pSocial = get_xsocial_data (argument);

  if (pSocial == NULL)
    {
      stc ("That xsocial does not exist.\n\r", ch);
      return FALSE;
    }

/*    xprintf(buf, "[ %s: %s ]", olc_ed_name (ch), olc_ed_vnum (ch));
    ptc (ch,"%s\n\r", buf);
*/
  ptc (ch, "Name    : %s\n\r"
		  "(cnoarg): %s\n\r" "(onoarg): %s\n\r"
		  "(cfound): %s\n\r" "(ofound): %s\n\r"
		  "(vfound): %s\n\r" "(cself) : %s\n\r"
		  "(oself) : %s\n\r",
		  (pSocial->name),
		  (pSocial->char_no_arg),
		  (pSocial->others_no_arg),
		  (pSocial->char_found),
		  (pSocial->others_found),
		  (pSocial->vict_found),
		  (pSocial->char_auto), (pSocial->others_auto));
   ptc(ch, "Stage   : %d\n\r",pSocial->stage);
   ptc(ch, "Sex     : %d\n\r",pSocial->gender);
   ptc(ch, "Position: %d\n\r",pSocial->position);
   ptc(ch, "Self    : %d\n\r",pSocial->self);
   ptc(ch, "Other   : %d\n\r",pSocial->other);
   ptc(ch, "Extra   : %d\n\r",pSocial->extra);
   ptc(ch, "chance  : %d\n\r",pSocial->chance);

  stc ("\n\r", ch);
  return TRUE;
}

XSEDIT (xsedit_delete)
{
  int i, j;
  XSOCIAL_DATA *pSocial;
  struct xsocial_type *new_table;

  new_table = (struct xsocial_type *) calloc (sizeof (*new_table), maxXSocial);

  if (!new_table)
    {
      stc ("Memory allocation failed. Brace for impact...\n\r", ch);
      return FALSE;
    }

  if (IS_NULLSTR (argument))
    EDIT_XSOCIAL (ch, pSocial);
  else
    pSocial = get_xsocial_data (argument);

  if (pSocial == NULL)
    {
      stc ("No such xsocial exists.\n\r", ch);
      return FALSE;
    }

  for (i = 0, j = 0; i < maxXSocial + 1; i++)
    {
      if (&xsocial_table[i] != pSocial)
	{
	  new_table[j] = xsocial_table[i];
	  j++;
	}
    }
  free (xsocial_table);
  xsocial_table = new_table;
  maxXSocial--;
  edit_done (ch);
  stc ("XSocial deleted.\n\r", ch);
  return TRUE;
}

XSEDIT (xsedit_create)
{
  int iSocial;
  XSOCIAL_DATA *pSocial;
  char arg[MIL];
  struct xsocial_type *new_table;

  argument = one_argument (argument, arg);

  if (IS_NULLSTR (arg))
    {
      stc ("Syntax: xsedit create [social]\n\r", ch);
      return FALSE;
    }
  if ((iSocial = xsocial_lookup (arg)) != -1)
    {
      stc ("A xsocial with that name already exists.\n\r", ch);
      return FALSE;
    }
  if (strlen(arg) > 14)
  {
      stc( "XSedit: Name too long, please chose another name.\n\r",ch);
      return FALSE;
  }
  
  maxXSocial++;
  new_table =
    realloc (xsocial_table, sizeof (struct xsocial_type) * (maxXSocial + 1));

  if (!new_table)
    {
      stc ("Memory allocation failed. Brace for impact...\n\r", ch);
      return FALSE;
    }
  xsocial_table = new_table;

  xsocial_table[maxXSocial - 1].name = str_dup (arg);

  xsocial_table[maxXSocial - 1].char_no_arg = str_dup ("");
  xsocial_table[maxXSocial - 1].others_no_arg = str_dup ("");
  xsocial_table[maxXSocial - 1].char_found = str_dup ("");
  xsocial_table[maxXSocial - 1].others_found = str_dup ("");
  xsocial_table[maxXSocial - 1].vict_found = str_dup ("");
  xsocial_table[maxXSocial - 1].char_auto = str_dup ("");
  xsocial_table[maxXSocial - 1].others_auto = str_dup ("");
  xsocial_table[maxXSocial].name = str_dup ("");
  xsocial_table[maxXSocial - 1].gender = 0;
  xsocial_table[maxXSocial - 1].stage = 0;
  xsocial_table[maxXSocial - 1].position = 0;
  xsocial_table[maxXSocial - 1].self = 0;
  xsocial_table[maxXSocial - 1].other = 0;
  xsocial_table[maxXSocial - 1].extra = 0;
  xsocial_table[maxXSocial - 1].chance = 0;
  pSocial = get_xsocial_data (arg);
  ch->desc->pEdit = (void *) pSocial;
  stc ("XSocial created.\n\r", ch);
  return TRUE;
}

XSEDIT (xsedit_name)
{
  XSOCIAL_DATA *pSocial;

  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No social to edit.\n\r", ch);
      return FALSE;
    }

  if(strlen(argument) > 14)
  {
     stc("Name too long, keep it under 14 characters in length.",ch);
     return FALSE;
  }

  strcpy (pSocial->name, argument);
  stc ("Social changed.\n\r", ch);
//    save_social_table();
  return TRUE;
}

XSEDIT (xsedit_cnoarg)
{
  XSOCIAL_DATA *pSocial;

  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No xsocial to edit.\n\r", ch);
      return FALSE;
    }

  replace_string (pSocial->char_no_arg, argument);
  stc ("xSocial changed.\n\r", ch);
//    save_social_table();
  return TRUE;
}

XSEDIT (xsedit_onoarg)
{
  XSOCIAL_DATA *pSocial;

  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No xsocial to edit.\n\r", ch);
      return FALSE;
    }

  replace_string (pSocial->others_no_arg, argument);
  stc ("XSocial changed.\n\r", ch);
  //  save_social_table();
  return TRUE;
}

XSEDIT (xsedit_cfound)
{
  XSOCIAL_DATA *pSocial;

  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No xsocial to edit.\n\r", ch);
      return FALSE;
    }

  replace_string (pSocial->char_found, argument);
  stc ("XSocial changed.\n\r", ch);
//    save_social_table();
  return TRUE;
}

XSEDIT (xsedit_ofound)
{
  XSOCIAL_DATA *pSocial;

  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No xsocial to edit.\n\r", ch);
      return FALSE;
    }

  replace_string (pSocial->others_found, argument);
  stc ("xSocial changed.\n\r", ch);
//    save_social_table();
  return TRUE;
}

XSEDIT (xsedit_vfound)
{
  XSOCIAL_DATA *pSocial;

  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No xsocial to edit.\n\r", ch);
      return FALSE;
    }

  replace_string (pSocial->vict_found, argument);
  stc ("XSocial changed.\n\r", ch);
//    save_social_table();
  return TRUE;
}

XSEDIT (xsedit_cself)
{
  XSOCIAL_DATA *pSocial;

  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No xsocial to edit.\n\r", ch);
      return FALSE;
    }

  replace_string (pSocial->char_auto, argument);
  stc ("XSocial changed.\n\r", ch);
//    save_social_table();
  return TRUE;
}

XSEDIT (xsedit_sex)
{
  XSOCIAL_DATA *pSocial;
  int checkk =0;
  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No xsocial to edit.\n\r", ch);
      return FALSE;
    }

  if(argument[0] =='\0')
  {
    stc("sex type\n\r",ch);
    stc("Types:0 - all\n\r1 - male\n\r2 - female\n\r3 - lesbian\n\r4 - gay\n\r",ch);
    return FALSE;
  }
  if(!is_number(argument))
  {
    stc("sex type\n\r",ch);
    stc("Types:0 - all\n\r1 - male\n\r2 - female\n\r3 - lesbian\n\r4 - gay\n\r",ch);
     return FALSE;
  }

  checkk = atoi(argument);
  
  if(checkk > 4)
  {
    stc("Types: 0 - all\n\r1 - male\n\r 2 - female\n\r3 - lesbian\n\r4 - gay\n\r",ch);
    return FALSE;
  }

  pSocial->gender = atoi(argument);
  stc ("XSocial changed.\n\r", ch);
//    save_social_table();
  return TRUE;
}

XSEDIT (xsedit_position)
{
  XSOCIAL_DATA *pSocial;

  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No xsocial to edit.\n\r", ch);
      return FALSE;
    }

  if(argument[0] =='\0')
  {
    stc("sex 0-7\n\r",ch);
    return FALSE;
  }
  if(!is_number(argument))
  {
    stc("sex 0-7\n\r",ch);
     return FALSE;
  }

  pSocial->position = atoi(argument);
  stc ("XSocial changed.\n\r", ch);
//    save_social_table();
  return TRUE;
}

XSEDIT (xsedit_selfpleasure)
{
  XSOCIAL_DATA *pSocial;

  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No xsocial to edit.\n\r", ch);
      return FALSE;
    }

  if(argument[0] =='\0')
  {
    stc("self 1-500\n\r",ch);
    return FALSE;
  }
  if(!is_number(argument))
  {
    stc("self 1-500\n\r",ch);
     return FALSE;
  }

  pSocial->self = atoi(argument);
  stc ("XSocial changed.\n\r", ch);
//    save_social_table();
  return TRUE;
}

XSEDIT (xsedit_otherpleasure)
{
  XSOCIAL_DATA *pSocial;

  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No xsocial to edit.\n\r", ch);
      return FALSE;
    }

  if(argument[0] =='\0')
  {
    stc("other 1-500\n\r",ch);
    return FALSE;
  }
  if(!is_number(argument))
  {
    stc("other 1-500\n\r",ch);
     return FALSE;
  }

  pSocial->other = atoi(argument);
  stc ("XSocial changed.\n\r", ch);
//    save_social_table();
  return TRUE;
}

XSEDIT (xsedit_extra)
{
  XSOCIAL_DATA *pSocial;

  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No xsocial to edit.\n\r", ch);
      return FALSE;
    }

  if(argument[0] =='\0')
  {
    stc("extra 1-10\n\r",ch);
    return FALSE;
  }
  if(!is_number(argument))
  {
    stc("extra 1-10\n\r",ch);
     return FALSE;
  }

  pSocial->extra = atoi(argument);
  stc ("XSocial changed.\n\r", ch);
//    save_social_table();
  return TRUE;
}

XSEDIT (xsedit_chance)
{
  XSOCIAL_DATA *pSocial;
  int b = 0;
  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No xsocial to edit.\n\r", ch);
      return FALSE;
    }

  if(argument[0] =='\0')
  {
    stc("chance 0-1\n\r",ch);
    return FALSE;
  }
  if(!is_number(argument))
  {
    stc("chance 0-1\n\r",ch);
     return FALSE;
  }

  b = atoi(argument);

  if(b >1)
   {
    stc("chance 0-1\n\r",ch);
     return FALSE;
   }
  

  pSocial->chance = atoi(argument);
  stc ("XSocial changed.\n\r", ch);
//    save_social_table();
  return TRUE;
}

XSEDIT (xsedit_stage)
{
  XSOCIAL_DATA *pSocial;
  int b = 0;
  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No xsocial to edit.\n\r", ch);
      return FALSE;
    }

  if(argument[0] =='\0')
  {
    stc("stage 0-3\n\r",ch);
    return FALSE;
  }
  if(!is_number(argument))
  {
    stc("stage 0-3\n\r",ch);
     return FALSE;
  }

  b = atoi(argument);

  if(b >3)
   {
    stc("stage 0-3\n\r",ch);
     return FALSE;
   }
  

  pSocial->stage = atoi(argument);
  stc ("XSocial changed.\n\r", ch);
//    save_social_table();
  return TRUE;
}

XSEDIT (xsedit_oself)
{
  XSOCIAL_DATA *pSocial;

  EDIT_XSOCIAL (ch, pSocial);

  if (pSocial == NULL)
    {
      stc ("No xsocial to edit.\n\r", ch);
      return FALSE;
    }

  replace_string (pSocial->others_auto, argument);
  stc ("XSocial changed.\n\r", ch);
//    save_social_table();
  return TRUE;
}


void
xsedit (CHAR_DATA * ch, char *argument)
{
  int cmd = 0;
  char arg[MIL], command[MIL];

  smash_tilde (argument);
  strcpy (arg, argument);
  argument = one_argument (argument, command);

  if (ch->pcdata->security < 5)
    {
      stc ("XSEdit: Insufficient security to modify socials.\n\r", ch);
      edit_done (ch);
    }

  if (!str_cmp (command, "done"))
    {
      save_xsocial_table ();
      edit_done (ch);
      return;
    }

  if (IS_NULLSTR (command))
    {
      xsedit_show (ch, argument);
      return;
    }

  for (cmd = 0; xsedit_table[cmd].name != NULL; cmd++)
    {
      if (!str_prefix (command, xsedit_table[cmd].name))
	{
	  (*xsedit_table[cmd].olc_fun) (ch, argument);
	  return;
	}
    }
  interpret (ch, arg);
  return;
}

ACMD (do_xsedit)
{
  XSOCIAL_DATA *pSocial;
  char arg1[MIL];

  argument = one_argument (argument, arg1);

  if (IS_NPC (ch))
    return;

  if (ch->pcdata->security < 5)
    {
      stc ("Insuficient security to edit xsocials.\n\r", ch);
      return;
    }

  if (!str_cmp (arg1, "show"))
    {
      if (IS_NULLSTR (argument))
	{
	  stc ("Syntax: xsedit show [social]\n\r", ch);
	  return;
	}
      xsedit_show (ch, argument);
      return;
    }
  if (!str_cmp (arg1, "create"))
    {
      if (IS_NULLSTR (argument))
	{
	  stc ("Syntax: xsedit create [social]\n\r", ch);
	  return;
	}
      if (xsedit_create (ch, argument))
	{
	  ch->desc->editor = ED_XSOCIAL;
	  act ("$n has entered the xsocial editor.", ch, NULL, NULL, TO_ROOM);
	}
      return;
    }
  if (!str_cmp (arg1, "delete"))
    {
      if (IS_NULLSTR (argument))
	{
	  stc ("Syntax: xsedit delete [social]\n\r", ch);
	  return;
	}
      if (xsedit_delete (ch, argument))
	{
	  return;
	}
    }

  if ((pSocial = get_xsocial_data (arg1)) != NULL)
    {
      ch->desc->pEdit = (void *) pSocial;
      ch->desc->editor = ED_XSOCIAL;
      act ("$n has entered the xsocial editor.", ch, NULL, NULL, TO_ROOM);
      return;
    }
  if (pSocial == NULL && !IS_NULLSTR (arg1))
    {
      stc ("That xsocial does not exist.\n\r", ch);
      return;
    }

  stc ("XSEdit: There is no default social to edit.\n\r", ch);
  return;
}


/*
 * The X-social table.
 * Add new X-socials here.
 * Alphabetical order is not required.
 * As far as i know, this is originaly from KaViR of Godwars
 * i took it from the LOW4 godwars MudBase.. All credits are in
 * the godwars licence file.  Which is located in the DOC directory
 * You must adhere to it, before using this code.
 * From LOW4 Godwars Mud Base
 */

extern bool check_xsocial args ((CHAR_DATA * ch, char *command,
				 char *argument));
void make_preg args ((CHAR_DATA * mother, CHAR_DATA * father));
void stage_update( CHAR_DATA *ch, CHAR_DATA *victim, int stage,char *argument );

void
do_xsocials (CHAR_DATA * ch, char *argument)
{
  char buf[MSL];
  BUFFER *output = new_buf();
  int iSocial;
  int col;

  col = 0;

  add_buf(output,"{D============================================================================{x\n\r");
  add_buf(output,"{D                                 ALL                                    {x\n\r");
  for (iSocial = 0; iSocial < maxXSocial; iSocial++)
    {
     if(xsocial_table[iSocial].gender == 0)
      {
        xprintf(buf,"%-15s",xsocial_table[iSocial].name);
        add_buf(output,buf);
        col++;
        if(col ==5)
        {
          add_buf(output,"\n\r");
          col=0;
        } 
       }
    }
 if(col !=0)
  add_buf(output,"\n\r");

  add_buf(output,"{D                                MALE                                      {x\n\r");
  col = 0;
   for (iSocial = 0; iSocial < maxXSocial; iSocial++)
    {
    if(xsocial_table[iSocial].gender == 1)
       {
        xprintf(buf,"%-15s",xsocial_table[iSocial].name);
        add_buf(output,buf);
        col++;
        if(col == 5)
        {
          add_buf(output,"\n\r");
          col=0;
        }  
       }
     }
  if(col !=0)
   add_buf(output,"\n\r");

   add_buf(output,"{D                                FEMALE                                    {x\n\r");
    col = 0;
   for(iSocial =0; iSocial < maxXSocial; iSocial++)
   {
    if(xsocial_table[iSocial].gender ==2)
     {
        xprintf(buf,"%-15s",xsocial_table[iSocial].name);
        add_buf(output,buf);
        col++;
        if(col == 5)
        {
          add_buf(output,"\n\r");
          col = 0;
        }
     }
   }
  if(col !=0)
     add_buf(output,"\n\r");
     add_buf(output,"{D                                LESBIAN                                  {x\n\r");
     col = 0;
     for(iSocial = 0; iSocial < maxXSocial; iSocial++)
     {
       if(xsocial_table[iSocial].gender ==3)
       {
         xprintf(buf,"%-15s",xsocial_table[iSocial].name);
         add_buf(output,buf);
         col++;
         if(col ==5)
         {
           add_buf(output,"\n\r");
           col = 0;
         }
       }
     }
  if(col !=0)
     add_buf(output,"\n\r");

     add_buf(output,"{D                                  GAY                                     {x\n\r");
     col=0;
     for(iSocial = 0; iSocial < maxXSocial; iSocial++)
     {
       if(xsocial_table[iSocial].gender ==4)
       {
         xprintf(buf,"%-15s",xsocial_table[iSocial].name);
         add_buf(output,buf);
         col++;
         if(col ==5)
         {
           add_buf(output,"\n\r");
           col=0;
         }
       }
     }

  if (col % 6 != 0)
    add_buf (output,"\n\r");
 
  add_buf(output,"{D============================================================================{x\n\r");
  page_to_char(buf_string(output),ch);
  free_buf(output);  
  return;
}
