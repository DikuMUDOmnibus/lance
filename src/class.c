/* Online setting of skill/spell levels, 
 * (c) 1996 Erwin S. Andreasen <erwin@andreasen.org>
 *
 */

#include "include.h" /* This includes all the usual include files */
#include "magic.h"

/* 

  Class table levels loading/saving
  
*/

/* Save this class */
void save_class (int num)
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	int lev, i;
	
	sprintf (buf, "%s%s", CLASS_DIR, class_table[num].name);
	
	if (!(fp = fopen (buf, "w")))
	{
		bugf ("Could not open file %s in order to save class %s.", buf, class_table[num].name);
		return;
	}
	
	
	for (lev = 0; lev < LEVEL_IMMORTAL; lev++)
		for (i = 0; i < MAX_SKILL; i++)
		{
			if (!skill_table[i].name || !skill_table[i].name[0])
				continue;
				
			if (skill_table[i].skill_level[num] == lev)
				fprintf (fp, "%d %d %s\n", lev, skill_table[i].rating[num], skill_table[i].name);
		}
	
	fprintf (fp, "-1"); /* EOF -1 */
	fclose (fp);
}



void save_classes()
{
	int i;
	
	for (i = 0; i < MAX_CLASS; i++)
		save_class (i);
}


/* Load a class */
void load_class (int num)
{
	char buf[MAX_STRING_LENGTH];
	int level,rating,n;
	FILE *fp;
	
	sprintf (buf, "%s%s", CLASS_DIR, class_table[num].name);
	
	if (!(fp = fopen (buf, "r")))
	{
		bugf ("Could not open file %s in order to load class %s.", buf, class_table[num].name);
		return;
	}
	
	fscanf (fp, "%d", &level);
	fscanf (fp, "%d", &rating);
	
	while (level != -1)
	{
		fscanf (fp, " %[^\n]\n", buf); /* read name of skill into buf */
		
		n = skill_lookup (buf); /* find index */
		
		if (n == -1)
		{
			char buf2[200];
			sprintf (buf2, "Class %s: unknown spell %s", class_table[num].name, buf);
			bug (buf2, 0);
		}
		else
		{
			skill_table[n].rating[num]	= rating;
			skill_table[n].skill_level[num] = level;
		}

		fscanf (fp, "%d", &level);
		fscanf (fp, "%d", &rating);
	}
	
	fclose (fp);
}
	
void load_classes ()
{
	int i,j;

	for (i = 0; i < MAX_CLASS; i++)
	{
		for (j = 0; j < MAX_SKILL; j++)
		{
			skill_table[j].skill_level[i] 	= LEVEL_IMMORTAL;
			skill_table[j].rating[i] 	= 0;
		}
		
		load_class (i);
	}
}



void do_skillset (CHAR_DATA *ch ,char * argument)
{
	char class_name[MIL], skill_name[MIL], arg3[MIL];
	int sn, level, class_no;
	sh_int rating;
	
	argument = one_argument (argument, class_name);
	argument = one_argument (argument, skill_name);
	argument = one_argument (argument, arg3);
	
	if (!argument[0])
	{
		send_to_char ("Syntax is: skillset <class> <skill> <level> <rating>.\n\r",ch);
		return;
	}
	
	level 	= atoi (arg3);
	rating	= atoi (argument);
	
	if (!is_number(argument) || level < 0 || level > LEVEL_IMMORTAL)
	{
		printf_to_char (ch, "Level range is from 0 to %d.\n\r", LEVEL_IMMORTAL);
		return;
	}
	
	
	if ( (sn = skill_lookup (skill_name)) == -1)
	{
		printf_to_char (ch, "There is no such spell/skill as '%s'.\n\r", skill_name);
		return;
	}
	
	if( (class_no = class_lookup(class_name)) == -1 )
	{
		printf_to_char (ch, "%s does not exist.\n\r", class_name);
		return;
	}
	
	skill_table[sn].rating[class_no]	= rating;
	skill_table[sn].skill_level[class_no] 	= level;
	
	printf_to_char (ch, "OK, %ss will now gain %s at level %d%s with a difficulty of %d.\n\r", class_table[class_no].name,
								skill_table[sn].name, level, level == LEVEL_IMMORTAL ? " (i.e. never)" : "", skill_table[sn].rating[class_no]);
	
	
	save_classes();
}


void do_showskills (CHAR_DATA *ch, char *argument )
{
    FILE *fp;

    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    
    char sname[MAX_STRING_LENGTH];
    int rating;
    int lev;

    int class;
    int col;
    int sn;
    
    argument = one_argument( argument, arg1);
    
    if( !arg1[0] )
    {
    	send_to_char( "Syntax: showskills <class>.\n\r", ch );
    	return;
    }
    
    if( (class = class_lookup(arg1)) == -1 )
    {
    	send_to_char( "Class does not exist.\n\r", ch );
    	return;
    }

    sprintf( buf, "%s%s", CLASS_DIR, class_table[class].name );

    if( !(fp = fopen(buf, "r")) )
    {
        bugf( "Could not open %s for read.\n\r", buf );
        return;
    }
    
    fscanf( fp, "%d", &lev );
    fscanf( fp, "%d", &rating );

    col = 0;
    
    while( lev != -1)
    {
	fscanf( fp, " %[^\n]\n", sname );
	
	if( (sn = skill_lookup(sname)) == -1 )
	{
		char buf2[200];
		sprintf( buf2, "Class %s: unknown skill: %s", class_table[class].name, sname );
		bug( buf2, 0 );
	}
	

	if( skill_table[sn].spell_fun == spell_null )
	{
	    	sprintf( buf, "%3d %1d %-18s",
	    		lev,
	    		rating,
	    		sname );
	    		
		send_to_char( buf, ch );
	
		if( ++col % 3 == 0 )
			send_to_char( "\n\r", ch );
	}
		
		
	fscanf( fp, "%d", &lev );
	fscanf( fp, "%d", &rating );
    }

    fclose( fp );

    if( col % 3 != 0 )
    	send_to_char( "\n\r", ch );

}

void do_showspells (CHAR_DATA *ch, char *argument )
{
    FILE *fp;

    char arg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    
    char sname[MAX_STRING_LENGTH];
    int rating;
    int lev;

    int class;
    int col;
    int sn;
    
    argument = one_argument( argument, arg1);
    
    if( !arg1[0] )
    {
    	send_to_char( "Syntax: showspells <class>.\n\r", ch );
    	return;
    }
    
    if( (class = class_lookup(arg1)) == -1 )
    {
    	send_to_char( "Class does not exist.\n\r", ch );
    	return;
    }

    sprintf( buf, "%s%s", CLASS_DIR, class_table[class].name );

    if( !(fp = fopen(buf, "r")) )
    {
        bugf( "Could not open %s for read.\n\r", buf );
        return;
    }
    
    fscanf( fp, "%d", &lev );
    fscanf( fp, "%d", &rating );

    col = 0;
    
    while( lev != -1)
    {
	fscanf( fp, " %[^\n]\n", sname );
	
	if( (sn = skill_lookup(sname)) == -1 )
	{
		char buf2[200];
		sprintf( buf2, "Class %s: unknown skill: %s", class_table[class].name, sname );
		bug( buf2, 0 );
	}
	
	if( skill_table[sn].spell_fun != spell_null )
	{
	   	sprintf( buf, "%3d %1d %-18s",
    			lev,
    			rating,
    			sname );
    		
		send_to_char( buf, ch );
	
		if( ++col % 3 == 0 )
			send_to_char( "\n\r", ch );
	}
		
		
	fscanf( fp, "%d", &lev );
	fscanf( fp, "%d", &rating );
    }

    fclose( fp );

    if( col % 3 != 0 )
    	send_to_char( "\n\r", ch );

}