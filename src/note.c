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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"

#define NOTE_NOTE	0
#define NOTE_IDEA	1
#define NOTE_PENALTY	2
#define NOTE_NEWS	3
#define NOTE_CHANGES	4
#define NOTE_SIGN	5

extern FILE *                  fpArea;
extern char                    strArea[MAX_INPUT_LENGTH];

NOTE_DATA *note_list;
NOTE_DATA *idea_list;
NOTE_DATA *penalty_list;
NOTE_DATA *news_list;
NOTE_DATA *changes_list;

bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote )
{
    if ( !str_cmp( ch->name, pnote->sender )
    ||   is_name( "all", pnote->to_list )
    ||   ( IS_IMMORTAL(ch) && is_name( "immortal", pnote->to_list ) )
    ||   ( ch->clan && is_name( clan_table[ch->clan].name, pnote->to_list ) )
    ||   is_exact_name( ch->name, pnote->to_list ) )
	return TRUE;

    return FALSE;
}

bool hide_note (CHAR_DATA *ch, NOTE_DATA *pnote)
{
    time_t last_read;

    if (IS_NPC(ch))
	return TRUE;

    switch (pnote->type)
    {
	default:
	    return TRUE;
	case NOTE_NOTE:
	    last_read = ch->pcdata->last_note;
	    break;
	case NOTE_IDEA:
	    last_read = ch->pcdata->last_idea;
	    break;
	case NOTE_PENALTY:
	    last_read = ch->pcdata->last_penalty;
	    break;
	case NOTE_NEWS:
	    last_read = ch->pcdata->last_news;
	    break;
	case NOTE_CHANGES:
	    last_read = ch->pcdata->last_changes;
	    break;
    }
    
    if (pnote->date_stamp <= last_read)
	return TRUE;

    if (!str_cmp(ch->name,pnote->sender))
	return TRUE;

    if (!is_note_to(ch,pnote))
	return TRUE;

    return FALSE;
}

void count_total( CHAR_DATA *ch, NOTE_DATA *spool, int count[2] )
{
    NOTE_DATA *pnote;

    count[0] = 0;
    count[1] = 0;

    for ( pnote = spool; pnote != NULL; pnote = pnote->next )
    {
	if ( is_note_to( ch, pnote ) )
	    count[1]++;

	if ( !hide_note( ch, pnote ) )
	    count[0]++;
    }
}

void do_unread( CHAR_DATA *ch )
{
    POLL_DATA *poll;
    VOTE_DATA *vote;
    char buf[MAX_STRING_LENGTH];
    int value[2];

    if ( IS_NPC( ch ) )
	return; 

    send_to_char( "{s ===================================== \n\r", ch );
    send_to_char( "| {qBoard                 {s| {tNew {s| {tTotal {s|\n\r", ch );
    send_to_char( "{s ===================================== \n\r", ch );

    count_total( ch, news_list, value );
    sprintf( buf, "| {qNews Articles         {s| {%c%3d {s| {t%5d {s|\n\r",
	value[0] == 0 ? 't' : 'q', value[0], value[1] );
    send_to_char( buf, ch );

    if ( IS_IMMORTAL( ch ) )
    {
	count_total( ch, penalty_list, value );
	sprintf( buf, "| {qPenalties             {s| {%c%3d {s| {t%5d {s|\n\r",
	    value[0] == 0 ? 't' : 'q', value[0], value[1] );
	send_to_char( buf, ch );
    }

    count_total( ch, changes_list, value );
    sprintf( buf, "| {qChanges               {s| {%c%3d {s| {t%5d {s|\n\r",
	value[0] == 0 ? 't' : 'q', value[0], value[1] );
    send_to_char( buf, ch );

    count_total( ch, note_list, value );
    sprintf( buf, "| {qNotes                 {s| {%c%3d {s| {t%5d {s|\n\r",
	value[0] == 0 ? 't' : 'q', value[0], value[1] );
    send_to_char( buf, ch );

    count_total( ch, idea_list, value );
    sprintf( buf, "| {qIdeas                 {s| {%c%3d {s| {t%5d {s|\n\r",
	value[0] == 0 ? 't' : 'q', value[0], value[1] );
    send_to_char( buf, ch );

    value[0] = 0;
    value[1] = 0;
    for ( poll = first_poll; poll != NULL; poll = poll->next )
    {
	for ( vote = poll->vote; vote != NULL; vote = vote->next_vote )
	{
	    if ( !str_cmp( vote->voter_name, ch->name )
	    ||   !str_cmp( vote->voter_ip, ch->pcdata->socket ) )
	    {
		value[0]--;
		break;
	    }
	}

	value[0]++;
	value[1]++;
    }

    sprintf( buf, "| {qVoting Polls          {s| {%c%3d {s| {t%5d {s|\n\r",
	value[0] == 0 ? 't' : 'q', value[0], value[1] );
    send_to_char( buf, ch );

    send_to_char( " ====================================={x \n\r", ch );
}

void update_read(CHAR_DATA *ch, NOTE_DATA *pnote)
{
    time_t stamp;

    if (IS_NPC(ch))
	return;

    stamp = pnote->date_stamp;

    switch (pnote->type)
    {
        default:
            return;
        case NOTE_NOTE:
	    ch->pcdata->last_note = UMAX(ch->pcdata->last_note,stamp);
            break;
        case NOTE_IDEA:
	    ch->pcdata->last_idea = UMAX(ch->pcdata->last_idea,stamp);
            break;
        case NOTE_PENALTY:
	    ch->pcdata->last_penalty = UMAX(ch->pcdata->last_penalty,stamp);
            break;
        case NOTE_NEWS:
	    ch->pcdata->last_news = UMAX(ch->pcdata->last_news,stamp);
            break;
        case NOTE_CHANGES:
	    ch->pcdata->last_changes = UMAX(ch->pcdata->last_changes,stamp);
            break;
    }
}

void save_notes(int type)
{
    FILE *fp;
    char *name;
    NOTE_DATA *pnote;
    extern int port;

    log_string( "Saving notes." );

    if ( port != MAIN_GAME_PORT )
	return;

    switch (type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    name = NOTE_FILE;
	    pnote = note_list;
	    break;
	case NOTE_IDEA:
	    name = IDEA_FILE;
	    pnote = idea_list;
	    break;
	case NOTE_PENALTY:
	    name = PENALTY_FILE;
	    pnote = penalty_list;
	    break;
	case NOTE_NEWS:
	    name = NEWS_FILE;
	    pnote = news_list;
	    break;
	case NOTE_CHANGES:
	    name = CHANGES_FILE;
	    pnote = changes_list;
	    break;
    }

    if ( ( fp = fopen( name, "w" ) ) == NULL )
    {
	perror( name );
    }
    else
    {
	for ( ; pnote != NULL; pnote = pnote->next )
	{
	    fprintf( fp, "Sender  %s~\n", pnote->sender);
	    fprintf( fp, "Date    %s~\n", pnote->date);
	    fprintf( fp, "Stamp   %ld\n", pnote->date_stamp);
	    fprintf( fp, "To      %s~\n", pnote->to_list);
	    fprintf( fp, "Subject %s~\n", pnote->subject);
	    fprintf( fp, "Text\n%s~\n",   pnote->text);
	}
	fclose( fp );
   	return;
    }
}

void note_remove( CHAR_DATA *ch, NOTE_DATA *pnote, bool delete)
{
    char to_new[MAX_INPUT_LENGTH];
    char to_one[MAX_INPUT_LENGTH];
    NOTE_DATA *prev;
    NOTE_DATA **list;
    char *to_list;

    if (!delete && ch->level < MAX_LEVEL)
    {
	/* make a new list */
        to_new[0]	= '\0';
        to_list	= pnote->to_list;
        while ( *to_list != '\0' )
        {
    	    to_list	= one_argument( to_list, to_one );
    	    if ( to_one[0] != '\0' && str_cmp( ch->name, to_one ) )
	    {
	        strcat( to_new, " " );
	        strcat( to_new, to_one );
	    }
        }
        /* Just a simple recipient removal? */
       if ( str_cmp( ch->name, pnote->sender ) && to_new[0] != '\0' )
       {
	   free_string( pnote->to_list );
	   pnote->to_list = str_dup( to_new + 1 );
	   return;
       }
    }
    /* nuke the whole note */

    switch(pnote->type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    list = &note_list;
	    break;
	case NOTE_IDEA:
	    list = &idea_list;
	    break;
	case NOTE_PENALTY:
	    list = &penalty_list;
	    break;
	case NOTE_NEWS:
	    list = &news_list;
	    break;
	case NOTE_CHANGES:
	    list = &changes_list;
	    break;
    }

    /*
     * Remove note from linked list.
     */
    if ( pnote == *list )
    {
	*list = pnote->next;
    }
    else
    {
	for ( prev = *list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == pnote )
		break;
	}

	if ( prev == NULL )
	{
	    bug( "Note_remove: pnote not found.", 0 );
	    return;
	}

	prev->next = pnote->next;
    }

    save_notes(pnote->type);
    free_note(pnote);
    return;
}

void note_attach( CHAR_DATA *ch, int type )
{
    NOTE_DATA *pnote;

    if ( ch->pcdata->pnote != NULL )
	return;

    pnote = new_note();

    pnote->next		= NULL;
    pnote->sender	= str_dup( ch->name );
    pnote->date		= str_dup( "" );
    pnote->to_list	= str_dup( "" );
    pnote->subject	= str_dup( "" );
    pnote->text		= str_dup( "" );
    pnote->type		= type;
    ch->pcdata->pnote	= pnote;
    return;
}

void append_note(NOTE_DATA *pnote)
{
    FILE *fp;
    char *name;
    NOTE_DATA **list;
    NOTE_DATA *last;

    switch(pnote->type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    name = NOTE_FILE;
	    list = &note_list;
	    break;
	case NOTE_IDEA:
	    name = IDEA_FILE;
	    list = &idea_list;
	    break;
	case NOTE_PENALTY:
	    name = PENALTY_FILE;
	    list = &penalty_list;
	    break;
	case NOTE_NEWS:
	     name = NEWS_FILE;
	     list = &news_list;
	     break;
	case NOTE_CHANGES:
	     name = CHANGES_FILE;
	     list = &changes_list;
	     break;
    }

    if (*list == NULL)
	*list = pnote;
    else
    {
	for ( last = *list; last->next != NULL; last = last->next);
	last->next = pnote;
    }

    if ( ( fp = fopen(name, "a" ) ) == NULL )
    {
        perror(name);
    }
    else
    {
        fprintf( fp, "Sender  %s~\n", pnote->sender);
        fprintf( fp, "Date    %s~\n", pnote->date);
        fprintf( fp, "Stamp   %ld\n", pnote->date_stamp);
        fprintf( fp, "To      %s~\n", pnote->to_list);
        fprintf( fp, "Subject %s~\n", pnote->subject);
        fprintf( fp, "Text\n%s~\n", pnote->text);
        fclose( fp );
    }
}

void parse_note( CHAR_DATA *ch, char *argument, int type )
{
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];
    char final[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    NOTE_DATA *pnote;
    NOTE_DATA **list;
    char *list_name;
    int vnum;
    int anum;

    if ( IS_NPC(ch) )
	return;

    if ( ch->in_room->vnum == ROOM_VNUM_CORNER && !IS_IMMORTAL(ch) )
    {
	send_to_char("Just keep your nose in the corner like a good little player.\n\r",ch);
	return;
    }

    if ( ch->pcdata->penalty_time[PENALTY_NOCHANNEL] != 0 )
    {
	send_to_char("Your channel priviliges are revoked!\n\r",ch);
	return;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_WAR)
    || IS_SET(ch->in_room->room_flags, ROOM_ARENA) )
    {
	send_to_char("You can not see the messages from within the bloodshed.\n\r",ch);
	return;
    }

    switch(type)
    {
	default:
	    return;
        case NOTE_NOTE:
            list = &note_list;
	    list_name = "notes";
            break;
        case NOTE_IDEA:
            list = &idea_list;
	    list_name = "ideas";
            break;
        case NOTE_PENALTY:
            list = &penalty_list;
	    list_name = "penalties";
            break;
        case NOTE_NEWS:
            list = &news_list;
	    list_name = "news";
            break;
        case NOTE_CHANGES:
            list = &changes_list;
	    list_name = "changes";
            break;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_prefix( arg, "read" ) )
    {
	char time[100];
        bool fAll;
 
        if ( !str_cmp( argument, "all" ) )
        {
            fAll = TRUE;
            anum = 0;
        }
 
        else if ( argument[0] == '\0' || !str_prefix(argument, "next"))
        {
            vnum = 0;
            for ( pnote = *list; pnote != NULL; pnote = pnote->next)
            {
                if (!hide_note(ch,pnote))
                {
		    strftime(time,100,"%A, %B %d, %Y, at %I:%M:%S %p",localtime(&pnote->date_stamp));
                    sprintf( buf, "{B[{c%3d{B]{W %s:{c %s{x\n\r%s\n\rTo: {W%s{x\n\r",
                        vnum,
                        pnote->sender,
                        pnote->subject,
			time,
                        pnote->to_list);
                    send_to_char( buf, ch );
                    page_to_char( pnote->text, ch );
                    update_read(ch,pnote);
                    return;
                }
                else if (is_note_to(ch,pnote))
                    vnum++;
            }
	    sprintf(buf,"You have no unread %s.\n\r",list_name);
	    send_to_char(buf,ch);
            return;
        }
 
        else if ( is_number( argument ) )
        {
            fAll = FALSE;
            anum = atoi( argument );
        }
        else
        {
            send_to_char( "Read which number?\n\r", ch );
            return;
        }
 
        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && ( vnum++ == anum || fAll ) )
            {
		strftime(time,100,"%A, %B %d, %Y, at %I:%M:%S %p",localtime(&pnote->date_stamp));
                        sprintf( buf, "{B[{c%3d{B]{W %s:{c %s{x\n\r%s\n\rTo: {W%s{x\n\r",
                        vnum -1,
                        pnote->sender,
                        pnote->subject,
			time,
                        pnote->to_list);
                send_to_char( buf, ch );
                page_to_char( pnote->text, ch );
		update_read(ch,pnote);
                return;
            }
        }
 
	sprintf(buf,"There aren't that many %s.\n\r",list_name);
	send_to_char(buf,ch);
        return;
    }

    if ( !str_prefix( arg, "g-read" ) )
    {
        bool fAll;
 
        if ( !str_cmp( argument, "all" ) )
        {
            fAll = TRUE;
            anum = 0;
        }
 
        else if ( !str_prefix(argument, "g-next"))
        {
            vnum = 0;
            for ( pnote = *list; pnote != NULL; pnote = pnote->next)
            {
                if (!hide_note(ch,pnote))
                {
                        sprintf( buf, "{B[{c%3d{B]{W %s:{c %s{x\n\r%s\n\rTo: {W%s{x\n\r",
                        vnum,
                        pnote->sender,
                        pnote->subject,
                        pnote->date,
                        pnote->to_list);
                    send_to_char( buf, ch );
                    page_to_char( pnote->text, ch );
                    update_read(ch,pnote);
                    return;
                }
                vnum++;
            }
	    sprintf(buf,"You have no unread %s.\n\r",list_name);
	    send_to_char(buf,ch);
            return;
        }
 
        else if ( is_number( argument ) )
        {
            fAll = FALSE;
            anum = atoi( argument );
        }
        else
        {
            send_to_char( "Read which number?\n\r", ch );
            return;
        }
 
        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
	    if ( vnum++ == anum || fAll )
	    {
                    sprintf( buf, "{B[{c%3d{B]{W %s:{c %s{x\n\r%s\n\rTo: {W%s{x\n\r",
                    vnum - 1,
                    pnote->sender,
                    pnote->subject,
                    pnote->date,
                    pnote->to_list );
                send_to_char( buf, ch );
                page_to_char( pnote->text, ch );
                return;
	    }
        }
 
	sprintf(buf,"There aren't that many %s.\n\r",list_name);
	send_to_char(buf,ch);
        return;
    }

    if ( !str_prefix( arg, "list" ) )
    {
	buffer = new_buf( );

	vnum = 0;
	for ( pnote = *list; pnote != NULL; pnote = pnote->next )
	{
	    if ( is_note_to( ch, pnote ) )
	    {
		strftime(buf,100,"%m/%d/%Y",localtime(&pnote->date_stamp));
		sprintf(final, "{c%3d%s{B) {c%s {w%12s {D%s{x\n\r", 
		    vnum, hide_note(ch,pnote) ? " " : "N", 
		    end_string( pnote->subject, 48 ),
		    pnote->sender, buf );
		add_buf( buffer, final );
		vnum++;
	    }
	}

	if (!vnum)
	{
	    switch(type)
	    {
		case NOTE_NOTE:	
		    send_to_char("There are no notes for you.\n\r",ch);
		    break;
		case NOTE_IDEA:
		    send_to_char("There are no ideas for you.\n\r",ch);
		    break;
		case NOTE_PENALTY:
		    send_to_char("There are no penalties for you.\n\r",ch);
		    break;
		case NOTE_NEWS:
		    send_to_char("There are no news for you.\n\r",ch);
		    break;
		case NOTE_CHANGES:
		    send_to_char("There are no changes for you.\n\r",ch);
		    break;
	    }
	}

	else
	    page_to_char( buffer->string, ch );
	free_buf( buffer );

	return;
    }
    if ( ch->level == MAX_LEVEL && !str_prefix( arg, "g-list" ) )
    {
	vnum = 0;
	for ( pnote = *list; pnote != NULL; pnote = pnote->next )
	{
	    sprintf( buf, "[%3d%s] %s: %s\n\r",
		vnum, hide_note(ch,pnote) ? " " : "N", 
		pnote->sender, pnote->subject );
	    send_to_char(buf,ch);
	    vnum++;
	}
	if (!vnum)
	{
	    switch(type)
	    {
		case NOTE_NOTE:	
		    send_to_char("There are no notes for you.\n\r",ch);
		    break;
		case NOTE_IDEA:
		    send_to_char("There are no ideas for you.\n\r",ch);
		    break;
		case NOTE_PENALTY:
		    send_to_char("There are no penalties for you.\n\r",ch);
		    break;
		case NOTE_NEWS:
		    send_to_char("There are no news for you.\n\r",ch);
		    break;
		case NOTE_CHANGES:
		    send_to_char("There are no changes for you.\n\r",ch);
		    break;
	    }
	}
	return;
    }

    if ( !str_prefix( arg, "remove" ) )
    {
        if ( !is_number( argument ) )
        {
            send_to_char( "Note remove which number?\n\r", ch );
            return;
        }
 
        anum = atoi( argument );
        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && vnum++ == anum )
            {
                note_remove( ch, pnote, FALSE );
                send_to_char( "Ok.\n\r", ch );
                return;
            }
        }
 
	sprintf(buf,"There aren't that many %s.\n\r",list_name);
	send_to_char(buf,ch);
        return;
    }
 
    if ( !str_prefix( arg, "delete" ) && get_trust(ch) >= MAX_LEVEL - 1)
    {
        if ( !is_number( argument ) )
        {
            send_to_char( "Note delete which number?\n\r", ch );
            return;
        }
 
        anum = atoi( argument );
        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
            if ( is_note_to( ch, pnote ) && vnum++ == anum )
            {
                note_remove( ch, pnote,TRUE );
                send_to_char( "Ok.\n\r", ch );
                return;
            }
        }

 	sprintf(buf,"There aren't that many %s.\n\r",list_name);
	send_to_char(buf,ch);
        return;
    }

    if ( !str_prefix( arg, "g-delete" ) && get_trust(ch) >= MAX_LEVEL )
    {
        if ( !is_number( argument ) )
        {
            send_to_char( "Note delete which number?\n\r", ch );
            return;
        }
 
        anum = atoi( argument );
        vnum = 0;
        for ( pnote = *list; pnote != NULL; pnote = pnote->next )
        {
	    if (vnum++ == anum)
	    {
      		note_remove( ch, pnote,TRUE );
            	send_to_char( "Ok.\n\r", ch );
            	return;
	    }
        }

 	sprintf(buf,"There aren't that many %s.\n\r",list_name);
	send_to_char(buf,ch);
        return;
    }

    if (!str_prefix(arg,"catchup"))
    {
	switch(type)
	{
	    case NOTE_NOTE:	
		ch->pcdata->last_note = current_time;
		break;
	    case NOTE_IDEA:
		ch->pcdata->last_idea = current_time;
		break;
	    case NOTE_PENALTY:
		ch->pcdata->last_penalty = current_time;
		break;
	    case NOTE_NEWS:
		ch->pcdata->last_news = current_time;
		break;
	    case NOTE_CHANGES:
		ch->pcdata->last_changes = current_time;
		break;
	}
	return;
    }

    /* below this point only certain people can edit notes */
    if ((type == NOTE_NEWS && !IS_TRUSTED(ch,KNIGHT))
    ||  (type == NOTE_CHANGES && !IS_TRUSTED(ch,CREATOR)))
    {
	sprintf(buf,"You aren't authorized to write %s.",list_name);
	send_to_char(buf,ch);
	return;
    }

    if ( !str_cmp( arg, "+" ) )
    {
	if (ch->level < 20 && ch->pcdata->tier == 1)
	{
	    send_to_char("Your maturity in the realm is not great enough for such a task!\n\r",ch);
	    return;
	}

	note_attach( ch,type );

	if (ch->pcdata->pnote->type != type)
	{
	    send_to_char(
		"You already have a different note in progress.\n\r",ch);
	    return;
	}

	if (strlen(ch->pcdata->pnote->text)+strlen(argument) >= 4096)
	{
	    send_to_char( "Note too long.\n\r", ch );
	    return;
	}

 	buffer = new_buf();

	add_buf(buffer,ch->pcdata->pnote->text);
	add_buf(buffer,argument);
	add_buf(buffer,"\n\r");
	free_string( ch->pcdata->pnote->text );
	ch->pcdata->pnote->text = str_dup( buffer->string );
	free_buf(buffer);
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if (!str_cmp(arg,"-"))
    {
 	int len;
	bool found = FALSE;

	if (ch->level < 20 && ch->pcdata->tier == 1)
	{
	    send_to_char("Your maturity in the realm is not great enough for such a task!\n\r",ch);
	    return;
	}

	note_attach(ch,type);

        if (ch->pcdata->pnote->type != type)
        {
            send_to_char(
                "You already have a different note in progress.\n\r",ch);
            return;
        }

	if (ch->pcdata->pnote->text == NULL
	||  ch->pcdata->pnote->text[0] == '\0')
	{
	    send_to_char("No lines left to remove.\n\r",ch);
	    return;
	}

	strcpy(buf,ch->pcdata->pnote->text);

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
		    free_string(ch->pcdata->pnote->text);
		    ch->pcdata->pnote->text = str_dup(buf);
		    return;
		}
	    }
	}
	buf[0] = '\0';
	free_string(ch->pcdata->pnote->text);
	ch->pcdata->pnote->text = str_dup(buf);
	return;
    }

    if ( !str_cmp( arg, "format" ) )
    {
	note_attach( ch, type );
	ch->pcdata->pnote->text = format_string( ch->pcdata->pnote->text, atoi( argument ) );
	send_to_char( "Note formatted.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "desc" ) || !str_cmp( arg, "write") || !str_cmp( arg, "enter" ) )
    {
	if (ch->level < 20 && ch->pcdata->tier == 1)
	{
	    send_to_char("Your maturity in the realm is not great enough for such a task!\n\r",ch);
	    return;
	}

        note_attach( ch, type );

        if ( argument[0] == '\0' )
                string_append( ch, &ch->pcdata->pnote->text );
        return;
    }


    if ( !str_prefix( arg, "subject" ) )
    {
	if (ch->level < 20 && ch->pcdata->tier == 1)
	{
	    send_to_char("Your maturity in the realm is not great enough for such a task!\n\r",ch);
	    return;
	}

	note_attach( ch,type );

        if (ch->pcdata->pnote->type != type)
        {
            send_to_char(
                "You already have a different note in progress.\n\r",ch);
            return;
        }

	free_string( ch->pcdata->pnote->subject );
	ch->pcdata->pnote->subject = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "to" ) )
    {
	if (ch->level < 20 && ch->pcdata->tier == 1)
	{
	    send_to_char("Your maturity in the realm is not great enough for such a task!\n\r",ch);
	    return;
	}

	note_attach( ch,type );

        if (ch->pcdata->pnote->type != type)
        {
            send_to_char(
                "You already have a different note in progress.\n\r",ch);
            return;
        }

	free_string( ch->pcdata->pnote->to_list );
	ch->pcdata->pnote->to_list = str_dup( argument );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "clear" ) )
    {
	if (ch->level < 20 && ch->pcdata->tier == 1)
	{
	    send_to_char("Your maturity in the realm is not great enough for such a task!\n\r",ch);
	    return;
	}

	if ( ch->pcdata->pnote != NULL )
	{
	    free_note(ch->pcdata->pnote);
	    ch->pcdata->pnote = NULL;
	}

	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "show" ) )
    {
	if (ch->level < 20 && ch->pcdata->tier == 1)
	{
	    send_to_char("Your maturity in the realm is not great enough for such a task!\n\r",ch);
	    return;
	}

	if ( ch->pcdata->pnote == NULL )
	{
	    send_to_char( "You have no note in progress.\n\r", ch );
	    return;
	}

	if (ch->pcdata->pnote->type != type)
	{
	    send_to_char("You aren't working on that kind of note.\n\r",ch);
	    return;
	}

	sprintf( buf, "%s: %s\n\rTo: %s\n\r",
	    ch->pcdata->pnote->sender,
	    ch->pcdata->pnote->subject,
	    ch->pcdata->pnote->to_list
	    );
	send_to_char( buf, ch );
	send_to_char( ch->pcdata->pnote->text, ch );
	return;
    }

    if ( !str_prefix( arg, "post" ) || !str_prefix(arg, "send"))
    {
	char buf2 [MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	char *strtime;

	if (ch->level < 20 && ch->pcdata->tier == 1)
	{
	    send_to_char("Your maturity in the realm is not great enough for such a task!\n\r",ch);
	    return;
	}

	if ( ch->pcdata->pnote == NULL )
	{
	    send_to_char( "You have no note in progress.\n\r", ch );
	    return;
	}

        if (ch->pcdata->pnote->type != type)
        {
            send_to_char("You aren't working on that kind of note.\n\r",ch);
            return;
        }

	if (!str_cmp(ch->pcdata->pnote->to_list,""))
	{
	    send_to_char(
		"You need to provide a recipient (name, all, or immortal).\n\r",
		ch);
	    return;
	}

	if (!str_cmp(ch->pcdata->pnote->subject,""))
	{
	    send_to_char("You need to provide a subject.\n\r",ch);
	    return;
	}

	ch->pcdata->pnote->next		= NULL;
	strtime				= ctime( &current_time );
	strtime[strlen(strtime)-1]	= '\0';
	ch->pcdata->pnote->date		= str_dup( strtime );
	ch->pcdata->pnote->date_stamp	= current_time;

	switch(ch->pcdata->pnote->type)
	{
	    default:
		sprintf(buf2,"note");
		break;

	    case NOTE_NOTE:
		sprintf(buf2,"note");
		break;

	    case NOTE_IDEA:
		sprintf(buf2,"idea");
		break;

	    case NOTE_PENALTY:
		sprintf(buf2,"penalty");  
		break;

	    case NOTE_NEWS:
		sprintf(buf2,"news");
		break;

	    case NOTE_CHANGES:
		sprintf(buf2,"change");
		break;
	}

        sprintf(buf,"{8You remove a tack from the {&%s{8 board and attach your %s.{x\n\r",
                buf2,buf2);
        send_to_char(buf,ch);
        sprintf(buf,"{R*** {8You have posted a new {W%s{8 regarding: {Y%s {R***{x\n\r",
                buf2,ch->pcdata->pnote->subject);
        send_to_char( buf, ch );

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    CHAR_DATA *victim;
      
	    victim = d->original ? d->original : d->character;
       
	    if ( d->connected == CON_PLAYING
	    &&   !IS_SET(d->character->info, INFO_NOTES)
	    &&   d->character != ch
	    &&   is_note_to(d->character,ch->pcdata->pnote) )
	    {
		sprintf(buf,"{R*** {G%s{8 has posted a new {W%s{8 regarding: {Y%s {R***{x",
		    ch->name,buf2,ch->pcdata->pnote->subject);
		act("$t{x",ch,buf,d->character,TO_VICT,POS_DEAD);
	    }
	}

	append_note(ch->pcdata->pnote);
	ch->pcdata->pnote = NULL;
	return;
    }

    send_to_char( "You can't do that.\n\r", ch );
    return;
}

void do_note(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_NOTE);
}

void do_idea(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_IDEA);
}

void do_penalty(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_PENALTY);
}

void do_news(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_NEWS);
}

void do_changes(CHAR_DATA *ch,char *argument)
{
    parse_note(ch,argument,NOTE_CHANGES);
}

void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time)
{
    FILE *fp;
    NOTE_DATA *pnotelast;
 
    if ( ( fp = fopen( name, "r" ) ) == NULL )
	return;
	 
    pnotelast = NULL;
    for ( ; ; )
    {
	NOTE_DATA *pnote;
	char letter;
	 
	do
	{
	    letter = getc( fp );
            if ( feof(fp) )
            {
                fclose( fp );
                return;
            }
        }
        while ( isspace(letter) );
        ungetc( letter, fp );
 
        pnote           = alloc_perm( sizeof(*pnote) );
 
        if ( str_cmp( fread_word( fp ), "sender" ) )
            break;
        pnote->sender   = fread_string( fp );
 
        if ( str_cmp( fread_word( fp ), "date" ) )
            break;
        pnote->date     = fread_string( fp );
 
        if ( str_cmp( fread_word( fp ), "stamp" ) )
            break;
        pnote->date_stamp = fread_number(fp);
 
        if ( str_cmp( fread_word( fp ), "to" ) )
            break;
        pnote->to_list  = fread_string( fp );
 
        if ( str_cmp( fread_word( fp ), "subject" ) )
            break;
        pnote->subject  = fread_string( fp );
 
        if ( str_cmp( fread_word( fp ), "text" ) )
            break;
        pnote->text     = fread_string( fp );
 
        if (free_time && pnote->date_stamp < current_time - free_time)
        {
	    free_note(pnote);
            continue;
        }

	pnote->type = type;
 
        if (*list == NULL)
            *list           = pnote;
        else
            pnotelast->next     = pnote;
 
        pnotelast       = pnote;
    }
 
    strcpy( strArea, NOTE_FILE );
    fpArea = fp;
    bug( "Load_notes: bad key word.", 0 );
    exit( 1 );
    return;
}

void load_notes(void)
{
    extern int port;

    if ( port != MAIN_GAME_PORT )
	return;

    load_thread(NOTE_FILE,&note_list, NOTE_NOTE, 14*24*60*60);
    load_thread(IDEA_FILE,&idea_list, NOTE_IDEA, 28*24*60*60);
    load_thread(PENALTY_FILE,&penalty_list, NOTE_PENALTY, 0);
    load_thread(NEWS_FILE,&news_list, NOTE_NEWS, 0);
    load_thread(CHANGES_FILE,&changes_list,NOTE_CHANGES, 0);
}

void note_remove_quiet( NOTE_DATA *pnote)
{
    NOTE_DATA *prev;
    NOTE_DATA **list;


    switch(pnote->type)
    {
	default:
	    return;
	case NOTE_NOTE:
	    list = &note_list;
	    break;
	case NOTE_IDEA:
	    list = &idea_list;
	    break;
	case NOTE_PENALTY:
	    list = &penalty_list;
	    break;
	case NOTE_NEWS:
	    list = &news_list;
	    break;
	case NOTE_CHANGES:
	    list = &changes_list;
	    break;
    }

    /*
     * Remove note from linked list.
     */
    if ( pnote == *list )
    {
	*list = pnote->next;
    }
    else
    {
	for ( prev = *list; prev != NULL; prev = prev->next )
	{
	    if ( prev->next == pnote )
		break;
	}

	if ( prev == NULL )
	{
	    bug( "Note_remove: pnote not found.", 0 );
	    return;
	}

	prev->next = pnote->next;
    }

    save_notes(pnote->type);
    free_note(pnote);
    return;
}

void expire_notes ( void )
{
    NOTE_DATA *pnote;
    NOTE_DATA **list;
    long diff;
    int note_num;

    note_num = 0;
    list = &note_list;
    for ( pnote = *list; pnote != NULL; pnote = pnote->next ) 
    {
        diff = (long)current_time - (long)pnote->date_stamp;
 	if (diff > 864000) {
	    note_num++;
	}
    }
    for ( ; note_num > 0; note_num--)
    {
	pnote = *list;
	if (pnote != NULL)
	{
	    note_remove_quiet(pnote);
	}
    }
    note_num = 0;
    list = &idea_list; 
    for ( pnote = *list; pnote != NULL; pnote = pnote->next ) 
    {
        diff = (long)current_time - (long)pnote->date_stamp;
 	if (diff > 864000) {
	    note_num++;
	}
    }
    for ( ; note_num > 0; note_num--)
    {
	pnote = *list;
	if (pnote != NULL)
	{
	    note_remove_quiet(pnote);
	}
    }

    return;
}

void sign_attach( CHAR_DATA *ch, int type )
{
    NOTE_DATA *pnote;

    if ( ch->pcdata->pnote != NULL )
	return;

    pnote = new_note();

    pnote->next		= NULL;
    pnote->sender	= str_dup( ch->name );
    pnote->date		= str_dup( "" );
    pnote->to_list	= str_dup( "" );
    pnote->subject	= str_dup( "" );
    pnote->text		= str_dup( "" );
    pnote->type		= type;
    ch->pcdata->pnote	= pnote;
    return;
}

void parse_sign( CHAR_DATA *ch, char *argument, int type )
{
    BUFFER *buffer;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;

    if ( IS_NPC(ch) )
	return;

    smash_tilde( argument );
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        return;
    }

    if ( !str_cmp( arg, "+" ) )
    {
	sign_attach( ch,type );
	if (ch->pcdata->pnote->type != type)
	{
	    send_to_char(
		"You already have a different note in progress.\n\r",ch);
	    return;
	}

	if (strlen(ch->pcdata->pnote->text)+strlen(argument) >= 4096)
	{
	    send_to_char( "Sign too long.\n\r", ch );
	    return;
	}

 	buffer = new_buf();

	add_buf(buffer,ch->pcdata->pnote->text);
	add_buf(buffer,argument);
	add_buf(buffer,"\n\r");
	free_string( ch->pcdata->pnote->text );
	ch->pcdata->pnote->text = str_dup( buffer->string );
	free_buf(buffer);
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "format" ) )
    {
	sign_attach( ch, type );
	ch->pcdata->pnote->text = format_string( ch->pcdata->pnote->text, atoi( argument ) );
	send_to_char( "Sign formatted.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "desc" ) || !str_cmp( arg, "write") )
    {
        sign_attach( ch, type );
        if ( argument[0] == '\0' )
                string_append( ch, &ch->pcdata->pnote->text );
        return;
    }

    if (!str_cmp(arg,"-"))
    {
 	int len;
	bool found = FALSE;

	sign_attach(ch,type);
        if (ch->pcdata->pnote->type != type)
        {
            send_to_char(
                "You already have a different note in progress.\n\r",ch);
            return;
        }

	if (ch->pcdata->pnote->text == NULL
	||  ch->pcdata->pnote->text[0] == '\0')
	{
	    send_to_char("No lines left to remove.\n\r",ch);
	    return;
	}

	strcpy(buf,ch->pcdata->pnote->text);

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
		    free_string(ch->pcdata->pnote->text);
		    ch->pcdata->pnote->text = str_dup(buf);
		    return;
		}
	    }
	}
	buf[0] = '\0';
	free_string(ch->pcdata->pnote->text);
	ch->pcdata->pnote->text = str_dup(buf);
	return;
    }

    if ( !str_prefix( arg, "make" ) )
    {
	sign_attach( ch,type );
        if (ch->pcdata->pnote->type != type)
        {
            send_to_char(
                "You already have a different note in progress.\n\r",ch);
            return;
        }
	free_string( ch->pcdata->pnote->to_list );
	free_string( ch->pcdata->pnote->subject );
	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "clear" ) )
    {
	if ( ch->pcdata->pnote != NULL )
	{
	    free_note(ch->pcdata->pnote);
	    ch->pcdata->pnote = NULL;
	}

	send_to_char( "Ok.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "show" ) )
    {
	if ( ch->pcdata->pnote == NULL )
	{
	    send_to_char( "You have no sign in progress.\n\r", ch );
	    return;
	}

	if (ch->pcdata->pnote->type != type)
	{
	    send_to_char("You aren't working on that kind of note.\n\r",ch);
	    return;
	}

	send_to_char( ch->pcdata->pnote->text, ch );
	return;
    }

    if ( !str_prefix( arg, "post" ) || !str_prefix(arg, "send")
    || !str_prefix(arg, "save"))
    {
	EXTRA_DESCR_DATA *ed;

	if ( ch->pcdata->pnote == NULL )
	{
	    send_to_char( "You have no sign in progress.\n\r", ch );
	    return;
	}

        if (ch->pcdata->pnote->type != type)
        {
            send_to_char("You aren't working on that kind of note.\n\r",ch);
            return;
        }

	pObjIndex = get_obj_index(OBJ_VNUM_QUEST_SIGN);
	obj = create_object( pObjIndex );
	obj_to_room( obj, ch->in_room );

	ed = new_extra_descr();

	ed->keyword = str_dup( "sign" );

	buffer = new_buf();
        add_buf(buffer,ch->pcdata->pnote->text);
	ed->description = str_dup(buffer->string);

	ed->next = NULL;
	obj->extra_descr = ed;
	ch->pcdata->pnote = NULL;

        send_to_char( "A sign now floats before you.\n\r", ch );
	return;
    }

    send_to_char( "You can't do that.\n\r", ch );
    return;
}

void do_sign(CHAR_DATA *ch,char *argument)
{
    parse_sign(ch,argument,NOTE_SIGN);
}

