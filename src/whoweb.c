/*
 *
 * Wholist for web
 * (C) 2001 by Jonathan Ng
 *
 */
 
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"

int colour_to_html args( (char type, char *string));
void colourize args( (const char *text, char *buffer) );

#define HTML_FILE "../../../public_html/who.html"

void web_update(void)
{
    FILE *fp;
    DESCRIPTOR_DATA *d;
    int online = 0;
    
    fclose(fpReserve);
    if((fp = fopen(HTML_FILE,"w")) == NULL)
    {
    	bug("who.html: fopen", 0);
    	perror("who.html");
    }
    
    fprintf(fp, "<html><head><title>\n");
    fprintf(fp, "Players on Legends of the Lance\n");
    fprintf(fp, "</title></head>\n");
    
    fprintf(fp, "<body bgcolor=black text=#228B22 link=blue vlink=purple alink=red>\n");
    fprintf(fp, "<font size=+3>Who's playing on Legends of the Lance?</font>\n");
    fprintf(fp, "<br>Last updated: %s", (char *) ctime(&current_time));
    fprintf(fp, "<table bordercolor=#228B22 bordercolordark=#003300 bordercolorlight=#009900 width=100%% border=1>\n");
    fprintf(fp, "<tr bgcolor=#005500 style='color: #FFFFFF;'><td width=200 align=center>Race</td><td width=100 align=center>Clan</td><td>Player name and title</td></tr>\n");
    
    for(d = descriptor_list; d != NULL; d = d->next)
    {
    	CHAR_DATA *ch;
    	char buf[3*MAX_STRING_LENGTH];
    	char c;
    	
    	if(d->connected != CON_PLAYING)
    	    continue;
    	
    	ch = (d->original != NULL) ? d->original : d->character;
    	
    	if(ch->invis_level > 101
    	|| ch->incog_level > 101)
			continue;/*
    	|| IS_AFFECTED(ch, AFF_INVISIBLE)
    	|| IS_AFFECTED(ch, AFF_SNEAK)
    	|| IS_AFFECTED(ch, AFF_HIDE))
    	    continue;*/

	/* race */
	fprintf(fp, "<tr><td width=200 align=center>");
	if(ch->whotext != NULL)
	{
	    colourize(ch->whotext, buf);
	    fprintf(fp, "%s", buf);
    	}
    	else
    	    fprintf(fp, "%s", capitalize(pc_race_table[ch->race].name));
    	fprintf(fp, "</td>");
    	
	/* The clan, if it exists */
    	fprintf(fp, "<td width=100 align=center>");
    	if(is_clan(ch))
    	{
    	    colourize(clan_table[ch->clan].who_name, buf);
    	    fprintf(fp, "%s", buf);
    	}
  	    fprintf(fp, "&nbsp;");
        fprintf(fp, "</td>\n");
        
        /* The name, (it better exist) */
        fprintf(fp, "<td>");
		if (ch->pretitle != NULL)
		{
		colourize(ch->pretitle, buf);
		sprintf( buf, "%s ", buf );
		fprintf(fp, buf);
		}
		fprintf(fp, ch->name);
        /* colour the title */
        buf[0] = '\0';
        colourize(ch->pcdata->title, buf);
        c = ch->pcdata->title[0];
        
        if(c != ','
        && c != '.'
        && c != '!'
        && c != '?')
            fprintf(fp, " ");
        fprintf(fp, buf);
        fprintf(fp, "</td></tr>\n\n");
        online++;
    }
    fprintf(fp, "</table>\n");
    fprintf(fp, "<br>Visible players online: %d\n", online);
    fprintf(fp, "</body>");
    fprintf(fp, "</html>");
    
    fclose(fp);
    fpReserve = fopen(NULL_FILE, "r");
}

void colourize(const char *text, char *buffer)
{
    const char *point;
    int skip = 0;
    
    for(point = text; *point; point++)
    {
    	if(*point == '{')
    	{
    	    point++;
    	    if(*point == '\0')
    	   	point--;
    	    else
    	    	skip = colour_to_html(*point, buffer);
    	    while(skip-- > 0)
    	    	++buffer;
    	    continue;
    	}
    	*buffer = *point;
    	*++buffer = '\0';
    }
    *buffer = '\0';
    return;   
}

int colour_to_html(char type, char *string)
{
    char code[25];
    char *p = '\0';
    
    switch(type)
    {
    default:
    case '\0':
    	code[0] = '\0';
    	break;
    case ' ':
    	strcpy(code, " ");
    case '{':
       	strcpy(code, "{");
       	break;
    case 'r':
       	strcpy(code, "<font color=#990000>");
       	break;
    case 'R':
       	strcpy(code, "<font color=#FF0000>");
       	break;
    case 'b':
       	strcpy(code, "<font color=#000099>");
       	break;
    case 'B':
       	strcpy(code, "<font color=#0000FF>");
       	break;
    case 'w':
       	strcpy(code, "<font color=#999999>");
       	break;
    case 'W':
       	strcpy(code, "<font color=#FFFFFF>");
       	break;
    case 'y':
       	strcpy(code, "<font color=#999900>");
       	break;
    case 'Y':
       	strcpy(code, "<font color=#FFFF00>");
       	break;
    case 'x':
    	strcpy(code, "</font>");
    	break;
    case 'g':
       	strcpy(code, "<font color=#009900>");
       	break;
    case 'G':
       	strcpy(code, "<font color=#00FF00>");
       	break;
    case 'm':
       	strcpy(code, "<font color=#990099>");
       	break;
    case 'M':
       	strcpy(code, "<font color=#FF00FF>");
       	break;
    case 'c':
       	strcpy(code, "<font color=#009999>");
       	break;
    case 'C':
       	strcpy(code, "<font color=#00FFFF>");
       	break;
    case 'D':
       	strcpy(code, "<font color=#666666>");
       	break;
    case '/':
       	strcpy(code, "<br>");
       	break;
    }
    
    p = code;
    while(*p != '\0')
    {
    	*string = *p++;
    	*++string = '\0';
    }
    
    return(strlen(code));
}