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
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/*************************************************************************** 
*       ROT 1.4 is copyright 1996-1997 by Russ Walsh                       * 
*       By using this code, you have agreed to follow the terms of the     * 
*       ROT license, in the file doc/rot.license                           * 
***************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"

BAN_DATA *ban_list;
CENSOR_DATA *censor_list;

void save_bans(void)
{
    BAN_DATA *pban;
    CENSOR_DATA *censor;
    MULTIPLAY_DATA *allow;
    FILE *fp;
    bool found = FALSE;

    if ( ( fp = fopen( BAN_FILE, "w" ) ) == NULL )
    {
        perror( BAN_FILE );
    }

    for (pban = ban_list; pban != NULL; pban = pban->next)
    {
	if (IS_SET(pban->ban_flags,BAN_PERMANENT))
	{
	    found = TRUE;
	    fprintf(fp,"BAN %s %d %s\n",pban->name,pban->level,
		print_flags(pban->ban_flags));
	}
    }

    for (censor = censor_list; censor != NULL; censor = censor->next)
    {
	found = TRUE;
	fprintf(fp,"CEN %s~ %s~\n", censor->word, censor->replace);
    }

    for (allow = multiplay_list; allow != NULL; allow = allow->next)
    {
	found = TRUE;
	fprintf(fp,"MUL %s %s %s\n", allow->name, allow->host,
	    print_flags(allow->allow_flags));
    }

    fclose(fp);

    if (!found)
	unlink(BAN_FILE);

    return;
}

void load_bans(void)
{
    FILE *fp;
    MULTIPLAY_DATA *allow, *allow_last;
    BAN_DATA *pban, *ban_last;
    CENSOR_DATA *censor, *censor_last;
 
    if ( ( fp = fopen( BAN_FILE, "r" ) ) == NULL )
        return;
 
    ban_last = NULL;
    allow_last = NULL;
    censor_last = NULL;

    for ( ; ; )
    {
	char *word;

        if ( feof(fp) )
        {
            fclose( fp );
            return;
        }
 
	word = fread_word(fp);

	if (!str_cmp(word,"BAN"))
	{
	    pban = new_ban();
 
	    pban->name		= str_dup(fread_word(fp));
	    pban->level		= fread_number(fp);
	    pban->ban_flags	= fread_flag(fp);
	    fread_to_eol(fp);

	    if (ban_list == NULL)
		ban_list = pban;
	    else
		ban_last->next = pban;
	    ban_last = pban;
	}

	else if (!str_cmp(word,"CEN"))
	{
	    censor		= new_censor();

	    censor->word	= fread_string(fp);
	    censor->replace	= fread_string(fp);
 	    fread_to_eol(fp);

	    if ( censor_list == NULL )
		censor_list = censor;
	    else
		censor_last->next = censor;
	    censor_last = censor;
	}

	else if (!str_cmp(word,"MUL"))
	{
	    allow		= new_allow();

	    allow->name		= str_dup(fread_word(fp));
	    allow->host		= str_dup(fread_word(fp));
	    allow->allow_flags	= fread_flag(fp);
 	    fread_to_eol(fp);

	    if (multiplay_list == NULL)
		multiplay_list = allow;
	    else
		allow_last->next = allow;
	    allow_last = allow;
	}

	else
	{
	    bug("Load_bans: Wrong Word.",0);
	    return;
	}
    }
    return;
}

bool check_allow(char *site, int type)
{
    MULTIPLAY_DATA *allow;
    char host[MAX_STRING_LENGTH];

    strcpy(host,capitalize(site));
    host[0] = LOWER(host[0]);

    if (host[0] == '\0')
    {
	bug("Check_allow: NULL site.",0);
	return TRUE;
    }

    for ( allow = multiplay_list; allow != NULL; allow = allow->next ) 
    {
	if(!IS_SET(allow->allow_flags,type))
	    continue;

	if (IS_SET(allow->allow_flags,ALLOW_PREFIX) 
	&&  IS_SET(allow->allow_flags,ALLOW_SUFFIX)  
	&&  strstr(allow->host,host) != NULL)
	    return TRUE;

	if (IS_SET(allow->allow_flags,ALLOW_PREFIX)
	&&  !str_suffix(allow->host,host))
	    return TRUE;

	if (IS_SET(allow->allow_flags,ALLOW_SUFFIX)
	&&  !str_prefix(allow->host,host))
	    return TRUE;

	if ( (strstr(allow->host,host) != NULL)
	||  !str_suffix(allow->host,host)
	||  !str_prefix(allow->host,host) )
	    return TRUE;

	if ( (strstr(allow->host,host) != NULL)
	||  !str_suffix(allow->host,host)
	||  !str_prefix(allow->host,host) )
            return TRUE;

    }

    return FALSE;
}

bool check_ban(char *site,int type)
{
    BAN_DATA *pban;
    char host[MAX_STRING_LENGTH];

    strcpy(host,capitalize(site));
    host[0] = LOWER(host[0]);

    for ( pban = ban_list; pban != NULL; pban = pban->next ) 
    {
	if(!IS_SET(pban->ban_flags,type))
	    continue;

	if (IS_SET(pban->ban_flags,BAN_PREFIX) 
	&&  IS_SET(pban->ban_flags,BAN_SUFFIX)  
	&&  strstr(pban->name,host) != NULL)
	    return TRUE;

	if (IS_SET(pban->ban_flags,BAN_PREFIX)
	&&  !str_suffix(pban->name,host))
	    return TRUE;

	if (IS_SET(pban->ban_flags,BAN_SUFFIX)
	&&  !str_prefix(pban->name,host))
	    return TRUE;

	if ( (strstr(pban->name,host) != NULL)
	||  !str_suffix(pban->name,host)
	||  !str_prefix(pban->name,host) )
	    return TRUE;

	if ( (strstr(pban->name,host) != NULL)
	||  !str_suffix(pban->name,host)
	||  !str_prefix(pban->name,host) )
            return TRUE;

    }

    return FALSE;
}

void do_allow( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
    char *name;
    MULTIPLAY_DATA *allow, *prev;
    bool prefix = FALSE, suffix = FALSE;
    int type;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' )
    {
	if (multiplay_list == NULL)
	{
	    send_to_char("No multiplayers allowed at this time.\n\r",ch);
	    return;
  	}

        send_to_char("Allowed Multiplayers                           Name           Allowance\n\r",ch);

        for (allow = multiplay_list; allow != NULL; allow = allow->next)
        {
	    sprintf(buf2,"%s%s%s",
		IS_SET(allow->allow_flags,ALLOW_PREFIX) ? "*" : "",
		allow->host,
		IS_SET(allow->allow_flags,ALLOW_SUFFIX) ? "*" : "");
	    sprintf(buf,"%-46s %-14s %s\n\r",
		buf2, allow->name, IS_SET(allow->allow_flags,ALLOW_ITEMS) ? 
		"Items" : IS_SET(allow->allow_flags,ALLOW_CONNECTS) ?
		"Connects" : "" );
	    send_to_char(buf,ch);
        }
        return;
    }

    /* find out what type of allow */
    if (arg2[0] == '\0' || !str_prefix(arg2,"connections"))
	type = ALLOW_CONNECTS;
    else if (!str_prefix(arg2,"items"))
	type = ALLOW_ITEMS;
    else
    {
	send_to_char("Acceptable multiplay allow types are connections, and items.\n\r",
	    ch); 
	return;
    }

    name = arg1;

    if (name[0] == '*')
    {
	prefix = TRUE;
	name++;
    }

    if (name[strlen(name) - 1] == '*')
    {
	suffix = TRUE;
	name[strlen(name) - 1] = '\0';
    }

    if (strlen(name) == 0)
    {
	send_to_char("You have to allow_multiplayer SOMETHING.\n\r",ch);
	return;
    }

    prev = NULL;
    for ( allow = multiplay_list; allow != NULL; prev = allow, allow = allow->next )
    {
        if (!str_cmp(name,allow->host))
        {
	    if (prev == NULL)
		multiplay_list = allow->next;
	    else
		prev->next = allow->next;
	    free_allow(allow);
        }
    }

    allow	= new_allow();
    allow->host	= str_dup(name);
    allow->name	= str_dup(ch->name);

    allow->allow_flags = type;

    if (prefix)
	SET_BIT(allow->allow_flags,ALLOW_PREFIX);
    if (suffix)
	SET_BIT(allow->allow_flags,ALLOW_SUFFIX);

    allow->next		= multiplay_list;
    multiplay_list	= allow;
    save_bans();

    sprintf(buf,"%s has been permitted to multiplay.\n\r",allow->host);
    send_to_char( buf, ch );
    return;
}

void do_censor( CHAR_DATA *ch, char *argument )
{
    CENSOR_DATA *censor;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    bool found = FALSE;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
	if ( censor_list == NULL )
	    send_to_char("No words are censored right now.\n\r",ch);
	else
	{
	    send_to_char("Censored word        Replacement         \n\r",ch);
	    for ( censor = censor_list; censor != NULL; censor = censor->next )
	    {
		sprintf(buf,"%-20s %-20s\n\r",censor->word,censor->replace);
		send_to_char(buf,ch);
	    }
	}
	return;
    }

    if ( (victim = get_pc_world(ch,arg1)) != NULL )
    {
	if ( !can_over_ride(ch,victim,FALSE) )
	{
	    send_to_char( "I don't think so...\n\r", ch );
	    return;
	}

	if ( !IS_SET(victim->act, PLR_CENSORED) )
	{
	    SET_BIT(victim->act, PLR_CENSORED);
	    act( "$N has had $s channels censored.",
		ch, NULL, victim, TO_CHAR, POS_DEAD );
	    send_to_char( "Your channels have been censored.\n\r", victim );
	} else {
	    REMOVE_BIT(victim->act, PLR_CENSORED);
	    act( "$N's channels are no longer censored.",
		ch, NULL, victim, TO_CHAR, POS_DEAD );
	    send_to_char( "Your channels are no longer censored.\n\r", victim );
	}
	return;
    }

    for ( censor = censor_list; censor != NULL; censor = censor->next )
    {
	if ( !str_cmp(censor->word, arg1) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( found )
    {
	if ( argument[0] != '\0' )
	{
	    free_string(censor->replace);
	    censor->replace = str_dup(argument);
	    send_to_char("Censor updated.\n\r",ch);
	    save_bans();
	    return;
	}

	if ( censor == censor_list )
	    censor_list = censor_list->next;
	else
	{
	    CENSOR_DATA *prev;

	    for ( prev = censor_list; prev != NULL; prev = prev->next )
	    {
		if ( prev->next == censor )
		{
		    prev->next = censor->next;
		    break;
		}
	    }
	}

	free_censor(censor);
	save_bans();
	send_to_char("Censor removed.\n\r",ch);
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char("Syntax: censor <word> <replacement>.\n\r",ch);
	return;
    }

    censor = new_censor();

    censor->word = str_dup(arg1);
    censor->replace = str_dup(argument);

    censor->next = censor_list;
    censor_list  = censor;

    send_to_char("New censor created.\n\r",ch);
    save_bans();

    return;
}

void ban_site(CHAR_DATA *ch, char *argument, bool fPerm)
{
    char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char *name;
    BAN_DATA *pban, *prev;
    bool prefix = FALSE,suffix = FALSE;
    int type;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' )
    {
	if (ban_list == NULL)
	{
	    send_to_char("No sites banned at this time.\n\r",ch);
	    return;
  	}

        send_to_char("Banned sites                                  level  type  status\n\r",ch);
        for (pban = ban_list;pban != NULL;pban = pban->next)
        {
	    sprintf(buf2,"%s%s%s",
		IS_SET(pban->ban_flags,BAN_PREFIX) ? "*" : "",
		pban->name,
		IS_SET(pban->ban_flags,BAN_SUFFIX) ? "*" : "");
	    sprintf(buf,"%-43s    %-3d  %-7s  %s\n\r",
		buf2, pban->level,
		IS_SET(pban->ban_flags,BAN_NEWBIES) ? "newbies" :
		IS_SET(pban->ban_flags,BAN_PERMIT)  ? "permit"  :
		IS_SET(pban->ban_flags,BAN_ALL)     ? "all"	: "",
	    	IS_SET(pban->ban_flags,BAN_PERMANENT) ? "perm" : "temp");
	    send_to_char(buf,ch);
        }
        return;
    }

    /* find out what type of ban */
    if (arg2[0] == '\0' || !str_prefix(arg2,"all"))
	type = BAN_ALL;
    else if (!str_prefix(arg2,"newbies"))
	type = BAN_NEWBIES;
    else if (!str_prefix(arg2,"permit"))
	type = BAN_PERMIT;
    else
    {
	send_to_char("Acceptable ban types are all, newbies, and permit.\n\r",
	    ch); 
	return;
    }

    name = arg1;

    if (name[0] == '*')
    {
	prefix = TRUE;
	name++;
    }

    if (name[strlen(name) - 1] == '*')
    {
	suffix = TRUE;
	name[strlen(name) - 1] = '\0';
    }

    if (strlen(name) == 0)
    {
	send_to_char("You have to ban SOMETHING.\n\r",ch);
	return;
    }

    prev = NULL;
    for ( pban = ban_list; pban != NULL; prev = pban, pban = pban->next )
    {
        if (!str_cmp(name,pban->name))
        {
	    if (pban->level > get_trust(ch))
	    {
            	send_to_char( "That ban was set by a higher power.\n\r", ch );
            	return;
	    }
	    else
	    {
		if (prev == NULL)
		    ban_list = pban->next;
		else
		    prev->next = pban->next;
		free_ban(pban);
	    }
        }
    }

    pban = new_ban();
    pban->name = str_dup(name);
    pban->level = get_trust(ch);

    /* set ban type */
    pban->ban_flags = type;

    if (prefix)
	SET_BIT(pban->ban_flags,BAN_PREFIX);
    if (suffix)
	SET_BIT(pban->ban_flags,BAN_SUFFIX);
    if (fPerm)
	SET_BIT(pban->ban_flags,BAN_PERMANENT);

    pban->next  = ban_list;
    ban_list    = pban;
    save_bans();
    sprintf(buf,"%s has been banned.\n\r",pban->name);
    send_to_char( buf, ch );
    return;
}

void do_ban(CHAR_DATA *ch, char *argument)
{
    ban_site(ch,argument,FALSE);
}

void do_permban(CHAR_DATA *ch, char *argument)
{
    ban_site(ch,argument,TRUE);
}

void do_unban( CHAR_DATA *ch, char *argument )                        
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    BAN_DATA *prev;
    BAN_DATA *curr;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Remove which site from the ban list?\n\r", ch );
        return;
    }

    prev = NULL;
    for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next )
    {
        if ( !str_cmp( arg, curr->name ) )
        {
	    if (curr->level > get_trust(ch))
	    {
		send_to_char(
		   "You are not powerful enough to lift that ban.\n\r",ch);
		return;
	    }
            if ( prev == NULL )
                ban_list   = ban_list->next;
            else
                prev->next = curr->next;

            free_ban(curr);
	    sprintf(buf,"Ban on %s lifted.\n\r",arg);
            send_to_char( buf, ch );
	    save_bans();
            return;
        }
    }

    send_to_char( "Site is not banned.\n\r", ch );
    return;
}

void do_unallow( CHAR_DATA *ch, char *argument )                        
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    MULTIPLAY_DATA *prev;
    MULTIPLAY_DATA *curr;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Remove which site from the allowed multiplay list?\n\r", ch );
        return;
    }

    prev = NULL;

    for ( curr = multiplay_list; curr != NULL; prev = curr, curr = curr->next )
    {
        if ( !str_cmp( arg, curr->host ) )
        {
            if ( prev == NULL )
                multiplay_list = multiplay_list->next;
            else
                prev->next = curr->next;

            free_allow(curr);
	    sprintf(buf,"Multiplay allowence on %s lifted.\n\r",arg);
            send_to_char( buf, ch );
	    save_bans();
            return;
        }
    }

    send_to_char( "Site is not permitted to multiplay.\n\r", ch );
    return;
}



