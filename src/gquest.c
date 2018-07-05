/****************************************************************************
 *  Automated Global Quest Code						    *
 *  Telnet : <akbearsden.com:3778>					    *
 *  E-Mail : <dlmud@akbearsden.com>					    *
 *  Website: <http://dlmud.akbearsden.com>				    *
 *									    *
 *  Provides automated, saveable global quests for a level range.	    *
 *									    *
 *  This version has been redone and attempted to fit stock rom.	    *
 ***************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"

#define GQUEST_FILE     "../data/info/gquest.dat"
#define GQUEST_OFF	0
#define GQUEST_WAITING	1
#define GQUEST_RUNNING	2

typedef struct gquest_hist GQUEST_HIST;
typedef struct gquest_data GQUEST;

GQUEST_HIST	*gqhist_first = NULL;
GQUEST		gquest_info;

#define ON_GQUEST( ch )	( !IS_NPC( ch ) && IS_SET( ( ch )->act, PLR_GQUEST ) \
			&& gquest_info.running != GQUEST_OFF )

struct gquest_data
{
    char        *who;
    sh_int      mob_count;
    sh_int      timer;
    sh_int      involved;	// number of players still in the gquest
    int         qpoints;	// Deviant points
    int         qps;		// Quest Points
    sh_int      gold;
    sh_int      minlevel;
    sh_int      maxlevel;
    sh_int      running;
    sh_int      next;
    int         mobs[MAX_GQUEST_MOB];
};

struct gquest_hist
{
    GQUEST_HIST *	next;
    char *		short_descr;
    char *		text;
};

void gquest_channel( CHAR_DATA *ch, char *message )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf, "{GGQUEST{W: {x%s", message );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
  	if ( d->connected == CON_PLAYING
	&&   d->character != ch 
	&&   !IS_SET( d->character->comm, COMM_QUIET ) )
	    act( buf, d->character, NULL, ch, TO_CHAR, POS_DEAD );
    }
}

bool save_gquest_data( void )
{
    FILE *fp;
    char buf[MAX_INPUT_LENGTH];
    int i;

    if ( (fp = fopen(GQUEST_FILE, "w")) == NULL )
    {
	sprintf( log_buf, "save_gquest_data: Could not open file %s in order to save mud data.", buf );
	bug( log_buf, 0 );
	return FALSE;
    }

    fprintf( fp, "#GQUESTDATA\n\n" );

    fprintf( fp, "Mobs    %d ", gquest_info.mob_count	);
    for ( i = 0; i < gquest_info.mob_count; i++ )
	fprintf( fp, "%d ", gquest_info.mobs[i]	);
    fprintf( fp, "\n" );

    fprintf( fp, "Who     %s~\n", gquest_info.who	);
    fprintf( fp, "Timer   %d\n", (gquest_info.timer > 0) ? (gquest_info.timer + 1) : gquest_info.timer	);
    fprintf( fp, "Involv  %d\n", gquest_info.involved	);
    fprintf( fp, "Qpoints %d\n", gquest_info.qpoints	);
    fprintf( fp, "Qps     %d\n", gquest_info.qps	);
    fprintf( fp, "Gold    %d\n", gquest_info.gold	);
    fprintf( fp, "MinLev  %d\n", gquest_info.minlevel	);
    fprintf( fp, "MaxLev  %d\n", gquest_info.maxlevel	);
    fprintf( fp, "Running %d\n", gquest_info.running	);
    fprintf( fp, "Next    %d\n", (gquest_info.next > 0) ? (gquest_info.next + 1) : gquest_info.next	);
    fprintf( fp, "#0\n"					);
    fprintf( fp, "\nEnd\n"				);
    fclose(fp);

    return TRUE;
}

void reset_gqmob( CHAR_DATA * ch, int value )
{
    int i;

    for ( i = 0; i < MAX_GQUEST_MOB; i++ )
    {
	if ( ch && !IS_NPC(ch) )
	    ch->pcdata->gq_mobs[i] = value;
	else
	    gquest_info.mobs[i] = value;
    }
}

void end_gquest(void)
{
    CHAR_DATA *wch;

    free_string(gquest_info.who);
    gquest_info.who		= str_dup("");
    gquest_info.running		= GQUEST_OFF;
    gquest_info.mob_count	= 0;
    gquest_info.timer		= 0;
    gquest_info.involved	= 0;
    gquest_info.qpoints		= 0;
    gquest_info.qps		= 0;
    gquest_info.gold		= 0;
    gquest_info.minlevel	= 0;
    gquest_info.maxlevel	= 0;
    gquest_info.next		= number_range( 120, 360 );
    reset_gqmob(NULL, 0);

    while ( gquest_info.next % 10 != 0 )
	--gquest_info.next;

    for ( wch = player_list; wch != NULL; wch = wch->next )
    {
	if ( IS_SET(wch->act, PLR_GQUEST) )
	{
	    REMOVE_BIT(wch->act, PLR_GQUEST);
	    reset_gqmob(wch, 0);
	}
    }
}

bool load_gquest_data( void )
{
    FILE *fp;
    char *word;
    bool fMatch = FALSE;

    end_gquest( );

    if ( (fp = fopen(GQUEST_FILE, "r")) == NULL )
    {
	sprintf( log_buf, "load_gquest_data: Could not open file %s in order to read gquest data. Creating.", GQUEST_FILE );
	bug( log_buf, 0 );
	return save_gquest_data( );
    }

    if ( str_cmp(fread_word(fp), "#GQUESTDATA") )
    {
	sprintf( log_buf, "load_gquest_data: Invalid gquest data file (%s).\n\r", GQUEST_FILE );
	bug( log_buf, 0 );
	return FALSE;
    }

    for ( ; ; )
    {
	word = feof(fp) ? "End" : fread_word(fp);

	if ( !str_cmp(word, "End") )
	{
	    fMatch = TRUE;
	    break;
	}

	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	    case '#':
		fMatch = TRUE;
		fread_to_eol(fp);
		break;

	    case 'G':
		KEY( "Gold",	gquest_info.gold,	fread_number(fp) );
		break;

	    case 'I':
		KEY( "Involv",	gquest_info.involved,	fread_number(fp) );
		break;

	    case 'M':
		KEY( "MinLev",	gquest_info.minlevel,	fread_number(fp) );
		KEY( "MaxLev",	gquest_info.maxlevel,	fread_number(fp) );

		if ( !str_cmp(word, "Mobs") )
		{
		    int i;

		    gquest_info.mob_count = fread_number(fp);

		    for ( i = 0; i < gquest_info.mob_count; i++ )
			gquest_info.mobs[i] = fread_number(fp);

		    fMatch = TRUE;
		    break;
		}
		break;

	    case 'N':
		KEY( "Next",	gquest_info.next,	fread_number(fp) );
		break;

	    case 'Q':
		KEY( "Qpoints",	gquest_info.qpoints,	fread_number(fp) );
		KEY( "Qps",	gquest_info.qps,	fread_number(fp) );
		break;

	    case 'R':
		KEY( "Running",	gquest_info.running,	fread_number(fp) );
		break;

	    case 'T':
		KEY( "Timer",	gquest_info.timer,	fread_number(fp) );
		break;

	    case 'W':
		SKEY("Who",	gquest_info.who				 );
		break;
	}

	if ( !fMatch )
	{
	    sprintf( log_buf, "load_gquest_data: Invalid Key: %s", word );
	    bug( log_buf, 0 );
	    fread_to_eol(fp);
	}
    }

    fclose(fp);
    return TRUE;
}

bool is_random_gqmob( int vnum )
{
    int i;

    if ( get_mob_index(vnum) == NULL )
	return FALSE;

    for ( i = 0; i < gquest_info.mob_count; i++ )
    {
	if (gquest_info.mobs[i] == vnum)
	    return FALSE;
    }

    return TRUE;
}

bool generate_gquest( CHAR_DATA* who )
{
    CHAR_DATA *victim = NULL;
    char buf[MAX_STRING_LENGTH];
    size_t mob_count, randm;
    int vnums[top_mob_index];
    int i;

    reset_gqmob(NULL, 0);

    mob_count = 0;

    for ( victim = char_list; victim; victim = victim->next )
    {
	if ( !IS_NPC(victim) )
	{
	    REMOVE_BIT(victim->act, PLR_GQUEST);
	    reset_gqmob(victim, 0);
	    continue;
	}

	if ( victim->level > (gquest_info.maxlevel + 50)
	||   victim->level < gquest_info.minlevel
	||   victim->pIndexData == NULL
	||   victim->in_room == NULL
	||   victim->in_room->area->clan > 0
	||   victim->pIndexData->pShop != NULL 
	||   victim->spec_fun == spec_lookup("spec_questmaster")
	||   victim->spec_fun == spec_lookup("spec_stringer")
	||   IS_SET(victim->in_room->room_flags, ROOM_PET_SHOP)
	||   IS_SET(victim->act, ACT_PET	| ACT_TRAIN	| ACT_PRACTICE |
				 ACT_NOQUEST	| ACT_IS_PRIEST|
				 ACT_IS_HEALER	| ACT_IS_SATAN  | ACT_GAIN)
	||   IS_SET(victim->affected_by, AFF_CHARM)
	||   IS_SET(victim->in_room->room_flags, ROOM_PET_SHOP | ROOM_SAFE)
	||   IS_SET(victim->in_room->area->area_flags,AREA_UNLINKED)
	||   ( victim->pIndexData->vnum >= 18600 && victim->pIndexData->vnum <= 18799 ))
	    continue;

	vnums[mob_count] = victim->pIndexData->vnum;
	mob_count++;

	if ( mob_count >= top_mob_index )
	    break;
    }

    if ( mob_count < 5 )
    {
	end_gquest();
	return FALSE;
    }
    else if ( mob_count < gquest_info.mob_count )
    {
	gquest_info.mob_count = mob_count;
    }

    for ( i = 0; i < gquest_info.mob_count; i++ )
    {
	randm = number_range(0, mob_count - 1);

	while (!is_random_gqmob(vnums[randm]))
	    randm = number_range(0, mob_count - 1);

	gquest_info.mobs[i] = vnums[randm];
    }

    gquest_info.qpoints		= (gquest_info.mob_count / 20) + number_range(0,1);
    gquest_info.qps		= number_range(15, 40) + gquest_info.mob_count;
    gquest_info.gold		= number_range(100, 150) * gquest_info.mob_count;
    gquest_info.timer		= 3;
    gquest_info.next		= 0;

    if ( gquest_info.maxlevel <= 70 )
    {
      	if ( gquest_info.qpoints > 1 ) 
	    gquest_info.qpoints = 1;
    }
    else if ( gquest_info.maxlevel <= 150 )
    {
	if ( gquest_info.qpoints > 2 )
	    gquest_info.qpoints = 2;

	gquest_info.qps += number_range(5,10);
    }
    else
    {
	if ( gquest_info.qpoints > 3 )
	    gquest_info.qpoints = 3;

	gquest_info.qps += number_range(10,20);
    }

    if ( gquest_info.qpoints >= 2 && gquest_info.mob_count < 20 )	/// Adjustments
 	gquest_info.qpoints = 1;

    if ( gquest_info.qpoints < 0 )		/// Adjustments
	gquest_info.qpoints  = 1;

    sprintf(buf, "{g%s Global Quest for levels {W%d {gto {W%d%s{g.  Type '{WGQuest INFO{g' to see the quest.{x",
	     !who ? "A" : "$N{g has declared a", gquest_info.minlevel,
	     gquest_info.maxlevel, !who ? " has started" : "");
    gquest_channel(who, buf);
    sprintf(buf, "{gYou announce a Global Quest for levels {W%d {gto {W%d {gwith {W%d{g Quest Targets.{x\n\r", gquest_info.minlevel, gquest_info.maxlevel, gquest_info.mob_count);
    send_to_char(buf, who);
    sprintf(buf, "{gGlobal Quest for levels {W%d {gto {W%d{g.{x",gquest_info.minlevel,gquest_info.maxlevel);
//    news(buf,NULL,NULL,NEWS_GQUEST,0,0);
    return TRUE;
}

bool start_gquest( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int mobs, blevel, elevel, cost;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
	send_to_char("Syntax: gquest start <min level> <max level> <#mobs>\n\r", ch);
	return FALSE;
    }

    blevel	= atoi(arg1);
    elevel	= atoi(arg2);
    mobs	= atoi(arg3);

    if ( blevel <= 0 || blevel > MAX_LEVEL
    ||   elevel <= 0 || elevel > MAX_LEVEL )
    {
	sprintf(buf, "Level must be between 1 and %d.\n\r", MAX_LEVEL);
	send_to_char(buf, ch);
	return FALSE;
    }

    if ( elevel < blevel )
    {
	send_to_char( "Min level can not be more than max level.\n\r", ch );
	return FALSE;
    }

    if ( mobs < 5 || mobs >= MAX_GQUEST_MOB )
    {
	sprintf(buf, "Number of mobs must be between 5 and %d.\n\r", MAX_GQUEST_MOB - 1);
	send_to_char(buf, ch);
	return FALSE;
    }

    if ( gquest_info.running != GQUEST_OFF )
    {
	send_to_char("There is already a global quest running!\n\r", ch);
	return FALSE;
    }

    cost = 1 + (mobs / 20);

    gquest_info.running		= GQUEST_WAITING;
    gquest_info.minlevel	= blevel;
    gquest_info.maxlevel	= elevel;
    gquest_info.mob_count	= mobs;
    free_string(gquest_info.who);
    gquest_info.who		= str_dup( ch->name );

    if ( !generate_gquest(ch) )
    {
	send_to_char("Failed to start a gquest, not enough mobs found.\n\r", ch);
	return FALSE;
    }
    return TRUE;
}

void auto_gquest( void )
{
    CHAR_DATA *wch, *registar;
    SPEC_FUN *spec = spec_lookup( "spec_questmaster" );
    int maxlvl = 0, minlvl = MAX_LEVEL;

    if ( gquest_info.running != GQUEST_OFF )
	return;

    for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
    {
	if ( !IS_IMMORTAL( wch ) )
	{
	    maxlvl = UMAX( maxlvl, wch->level );
	    minlvl = UMIN( minlvl, wch->level );
	}
    }

    if ( maxlvl == 0 )
    {
	end_gquest( );
	return;
    }

    switch( number_range( 1, 5 ) )
    {
	default:
	case 1:
	    break;

	case 2:
	    minlvl = maxlvl - number_range( 10, 15 );
	    break;

	case 3:
	    maxlvl = minlvl + number_range( 10, 15 );
	    break;

	case 4:
	    minlvl = maxlvl - number_range( 1, 5 );
	    break;

	case 5:
	    maxlvl = minlvl + number_range( 1, 5 );
	    break;
    }

    for ( registar = char_list; registar != NULL; registar = registar->next )
    {
	if ( !IS_NPC( registar ) )
	    continue;

	if ( registar->spec_fun == spec )
	    break;
    }

    gquest_info.running		= GQUEST_WAITING;
    gquest_info.mob_count	= number_range( 5, MAX_GQUEST_MOB );
    gquest_info.minlevel	= UMAX( 1, minlvl );
    gquest_info.maxlevel	= UMIN( LEVEL_HERO, maxlvl );
    free_string( gquest_info.who );
    gquest_info.who = str_dup( registar->short_descr );
    generate_gquest( registar );
    return;
}

int count_gqmobs(CHAR_DATA * ch)
{
    int i, count = 0;

    if (IS_NPC(ch))
	return 0;

    for ( i = 0; i < gquest_info.mob_count; i++ )
    {
	if ( ch->pcdata->gq_mobs[i] == -1 )
	    count++;
    }

    return count;
}

void post_gquest( CHAR_DATA *ch )
{
    BUFFER *output;
    CHAR_DATA *wch;
    MOB_INDEX_DATA *mob;
    GQUEST_HIST *hist;
    char *strtime;
    char shortd[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int i;
    
    if ( gquest_info.running == GQUEST_OFF || gquest_info.involved == 0 )
	return;

    hist = alloc_mem(sizeof(hist));

    strtime = ctime(&current_time);
    strtime[strlen(strtime) - 1] = '\0';

    sprintf(shortd, "%24s %3d %3d %4d %12s\n\r", strtime,
	    gquest_info.minlevel, gquest_info.maxlevel, gquest_info.mob_count,
	    ch->name);
    hist->short_descr = str_dup(shortd);

    output = new_buf();

    sprintf(buf, "GLOBAL QUEST INFO\n\r-----------------\n\r");
    add_buf(output, buf);

    sprintf(buf, "Started by  : %s\n\r", gquest_info.who[0] == '\0' ? "Unknown" : gquest_info.who);
    add_buf(output, buf);

    sprintf(buf, "Levels      : %d - %d\n\r", gquest_info.minlevel, gquest_info.maxlevel);
    add_buf(output, buf);

    sprintf(buf, "Those Playing\n\r-------------\n\r");
    add_buf(output, buf);

    for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
    {
	if ( ON_GQUEST(wch) && count_gqmobs(wch) != gquest_info.mob_count )
	{
	    sprintf(buf, "%s [%d mobs left]\n\r", wch->name,
	    gquest_info.mob_count - count_gqmobs(wch));
	    add_buf(output, buf);
	}
    }

    sprintf(buf, "%s won the GQuest.\n\r", ch->name);
    add_buf(output, buf);

    sprintf(buf, "Quest Rewards\n\r-------------\n\r");
    add_buf(output, buf);

    sprintf(buf, "Deviant Reward: %d\n\r", gquest_info.qpoints);
    add_buf(output, buf);

    sprintf(buf, "Quest Points: %d\n\r", gquest_info.qps);
    add_buf(output, buf);

    sprintf(buf, "Gold Reward : %d\n\r", gquest_info.gold);
    add_buf(output, buf);

    sprintf(buf, "Quest Targets\n\r-------------\n\r");
    add_buf(output, buf);

    for ( i = 0; i < gquest_info.mob_count; i++ )
    {
	if ( (mob = get_mob_index(gquest_info.mobs[i])) != NULL )
	{
	    sprintf(buf, "%2d) [%-20.20s] %-30s\n\r",
		i + 1, mob->area->name, mob->short_descr);
	    add_buf(output, buf);
	}
    }

    hist->text		= str_dup( output->string );
    hist->next		= gqhist_first;
    gqhist_first	= hist;
    free_buf(output);
}

void do_gquest( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *wch;
    MOB_INDEX_DATA *mob;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    int i = 0;

    if ( IS_NPC(ch) )
    {
	send_to_char("Your the victim not the player.\n\r", ch);
	return;
    }

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
	send_to_char(	"Syntax: {Ggquest join        {y- {gjoin a global quest\n\r"
                    "        {Ggquest quit        {y- {gquit the global quest\n\r"
                    "        {Ggquest info        {y- {gshow global quest info\n\r"
                    "        {Ggquest time        {y- {gshow global quest time\n\r"
                    "        {Ggquest check       {y- {gshow what targets you have left\n\r"
                    "        {Ggquest progress    {y- {gshow progress of other players\n\r"
                    "        {Ggquest complete    {y- {gcompletes the current quest\n\r"
                    "        {Ggquest hist        {y- {gshows gquest history since last reboot\n\r", ch);			
	if ( IS_IMMORTAL(ch) )
	{
	    send_to_char("        {Ggquest start       {y- {gstarts a gquest{x\n\r", ch);
		send_to_char("        {Ggquest end         {y- {gends the gquest\n\r", ch);
	    send_to_char("        {Ggquest next        {y- {gsets time to next gquest{x\n\r", ch);
	}

	return;
    }

    else if ( !str_prefix(arg1, "start") && IS_IMMORTAL(ch) )
    {
	start_gquest(ch, argument);
	return;
    }

    else if ( !str_prefix(arg1, "next") && IS_IMMORTAL(ch) )
    {
	if ( gquest_info.running != GQUEST_OFF )
	{
	    send_to_char("Not while a gquest is running.\n\r", ch);
	    return;
	}

	i = is_number(argument) ? atoi(argument) : number_range(30, 100);
	gquest_info.next = i;
	sprintf(buf, "The next gquest will start in %d minutes.\n\r",
	    gquest_info.next);
	send_to_char(buf, ch);
	return;
    }

    else if ( !str_prefix(arg1, "history") )
    {
	GQUEST_HIST *hist;
	int count = 0;

	if ( !gqhist_first )
	{
	    send_to_char("No global quests completed yet.\n\r", ch);
	    return;
	}

	if ( argument[0] == '\0' )
	{
	    BUFFER *output = new_buf();

	    add_buf(output,
		    "Num Finished Time            Levels  Mobs Completed by\n\r"
		    "--- ------------------------ ------- ---- ------------\n\r");

	    for (hist = gqhist_first; hist != NULL; hist = hist->next)
	    {
		sprintf(buf, "%2d) ", ++count);
		add_buf(output, buf);
		add_buf(output, hist->short_descr);
	    }

	    add_buf(output, "Type 'gquest hist #' to view details.\n\r");
	    page_to_char( output->string, ch);
	    free_buf(output);
	} else {
	    bool found = FALSE;

	    if ( !is_number(argument) )
	    {
		send_to_char("Syntax: gquest hist #\n\r", ch);
		return;
	    }

	    for ( hist = gqhist_first; hist != NULL; hist = hist->next )
	    {
		if ( ++count == atoi(argument) )
		{
		    send_to_char(hist->text, ch);
		    found = TRUE;
		}
	    }

	    if ( !found )
		send_to_char("History data not found.\n\r", ch);
	}
	return;
    }

    else if ( gquest_info.running == GQUEST_OFF )
    {
	sprintf(buf, "There is no global quest running.  The next Gquest will start in %d minutes.\n\r", gquest_info.next);
	send_to_char(buf, ch);
	return;
    }

    else if ( !str_prefix(arg1, "end") && IS_IMMORTAL(ch) )
    {
	end_gquest();
	sprintf(buf, "{gYou end the global quest. Next global quest in {W%d{g minutes.{x\n\r", gquest_info.next);
	send_to_char(buf, ch);
	sprintf(buf, "{g$n{g has ended the global quest. Next gquest in {W%d{g minutes.{x", gquest_info.next);
	return;
    }

    else if ( !str_prefix(arg1, "join") )
    {
	if ( ON_GQUEST(ch) )
	{
	    send_to_char("Your already in the global quest.\n\r", ch);
	    return;
	}

	if ( gquest_info.minlevel > ch->level
	||   gquest_info.maxlevel < ch->level )
	{
	    send_to_char("This gquest is not in your level range.\n\r", ch);
	    return;
	}

	if ( count_gqmobs(ch) == gquest_info.mob_count )
	{
	    send_to_char("You have already quit this gquest.\n\r", ch);
	    return;
	}

	for ( i = 0; i < gquest_info.mob_count; i++ )
	    ch->pcdata->gq_mobs[i] = gquest_info.mobs[i];

	SET_BIT(ch->act, PLR_GQUEST);
	gquest_info.involved++;
     	gquest_info.qps += 1;
	send_to_char("{gYou have joined the Global Quest!  Use '{WGQuest INFO{g' for your task.{x\n\r", ch);
	gquest_channel(ch, "{g$N{g has joined the global quest.{x");
	return;
    }

    else if ( !str_prefix(arg1, "quit") )
    {
	if ( !ON_GQUEST(ch) )
	{
	    send_to_char("Your not in a global quest.\n\r", ch);
	    return;
	}
	/* hack to prevent coming back */
	reset_gqmob(ch, -1);
	REMOVE_BIT(ch->act, PLR_GQUEST);
	gquest_info.involved--;
	send_to_char("{gYour global quest flag is now off. Sorry you couldn't complete it.{x\n\r",ch);
	gquest_channel(ch, "{g$N{g has quit the global quest.{x");
	return;
    }

    else if ( !str_prefix(arg1, "info") )
    {
	BUFFER *final = new_buf();
	char buf2[MAX_STRING_LENGTH];
	char mob_short[MAX_INPUT_LENGTH];

	add_buf(final, "{B -{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={8( {WGLOBAL QUEST INFO{8 ){b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B- \n\r"
		       "{B| {W*************************************************************************** {B|\n\r");

	sprintf(buf, "{wStarted by  {W: {w%s",
	    gquest_info.who[0] == '\0' ? "Unknown" : gquest_info.who);
	sprintf(buf2, "{B| {W* %s {W* {B|\n\r", end_string(buf, 71));
	add_buf(final, buf2);

	sprintf(buf, "{wPlaying     {W: {w%d player%s.",
	    gquest_info.involved, gquest_info.involved == 1 ? "" : "s");
	sprintf(buf2, "{B| {W* %s {W* {B|\n\r", end_string(buf, 71));
	add_buf(final, buf2);

	sprintf(buf, "{wLevels      {W: {w%d - %d",
	    gquest_info.minlevel, gquest_info.maxlevel);
	sprintf(buf2, "{B| {W* %s {W* {B|\n\r", end_string(buf, 71));
	add_buf(final, buf2);

	sprintf(buf, "{wDeviant Reward{W: {w%d", gquest_info.qpoints);
	sprintf(buf2, "{B| {W* %s {W* {B|\n\r", end_string(buf, 71));
	add_buf(final, buf2);

	sprintf(buf, "{wQuest Points{W: {w%d", gquest_info.qps);
	sprintf(buf2, "{B| {W* %s {W* {B|\n\r", end_string(buf, 71));
	add_buf(final, buf2);

	sprintf(buf, "{wGold Reward {W: {w%d", gquest_info.gold);
	sprintf(buf2, "{B| {W* %s {W* {B|\n\r", end_string(buf, 71));
	add_buf(final, buf2);

	sprintf(buf, "{wStatus      {W: {w%s for %d minute%s.",
	    gquest_info.running == GQUEST_WAITING ? "Waiting" : "Running", gquest_info.timer, gquest_info.timer == 1 ? "" : "s");
	sprintf(buf2, "{B| {W* %s {W* {B|\n\r", end_string(buf, 71));
	add_buf(final, buf2);

	add_buf(final, "{B| {b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={8( {WQuest Targets{8 ){b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B |\n\r");

        if ( gquest_info.running == GQUEST_WAITING && !IS_IMMORTAL(ch) )
        {
	    sprintf(buf, "Waiting on Participants...");
	    sprintf(buf2, "{B| {W* {w%s {W* {B|\n\r", end_string(buf, 71));
	    add_buf(final, buf2);
	}
	else
	{
	    for ( i = 0; i < gquest_info.mob_count; i++ )
	    {
		if ( (mob = get_mob_index(gquest_info.mobs[i])) != NULL )
		{
		    sprintf(mob_short, "%s", mob->short_descr);

		    sprintf(buf, "%2d{W) {8[{w%-20.20s{8]  {w%s",
			i + 1, mob->area->name,end_string(mob_short,50));
		    sprintf(buf2, "{B| {W* {w%s {W* {B|\n\r",
			end_string(buf, 71));
		    add_buf(final, buf2);
		}
	    }
	}

	add_buf(final, "{B| {W*************************************************************************** {B|\n\r"
		       "{B -{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={8( {WGLOBAL QUEST INFO{8 ){b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{b={B-{x \n\r");

	page_to_char(final->string, ch);
	free_buf(final);

	return;
    }

    else if ( !str_prefix(arg1, "time") )
    {
	if ( gquest_info.next > 0 )
	    sprintf(buf, "{GThe next Global Quest will start in {W%d {Gminute%s.{x\n\r",
		gquest_info.next, gquest_info.next == 1 ? "" : "s");
	else
	    sprintf(buf, "{GThe Global Quest is {W%s {Gfor {W%d {Gminute%s.{x\n\r",
		gquest_info.running == GQUEST_WAITING ? "Waiting" :
		"Running", gquest_info.timer, gquest_info.timer == 1 ? ""
		: "s");
	send_to_char(buf, ch);
	return;
    }

    else if ( !str_prefix(arg1, "progress") )
    {
	if ( gquest_info.running == GQUEST_WAITING )
	{
	    send_to_char("The global quest hasn't started yet.\n\r", ch);
	    return;
	}

	for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
	{
	    if ( ON_GQUEST(wch) )
	    {
		sprintf(buf, "{C[{c%-12s{C] {whas {R({c%2d{R) {wof {R({c%2d{R) {wmobs left.{x\n\r",
		    wch->name,
		    gquest_info.mob_count - count_gqmobs(wch),
		    gquest_info.mob_count);
        	send_to_char(buf,ch);
	    }
	}
	return;
    }

    else if ( !str_prefix(arg1, "check") )
    {
	BUFFER *final;
	char mob_short[MAX_INPUT_LENGTH];

	if ( !ON_GQUEST(ch) )
	{
	    send_to_char("You aren't on a global quest.\n\r", ch);
	    return;
	}

	final = new_buf();

	sprintf(buf, "{C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={r( {R({w%d{R) of ({w%d{R) mobs left {r){c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-\n\r",
	    gquest_info.mob_count - count_gqmobs(ch),
	    gquest_info.mob_count);
	add_buf(final, buf);

        if ( gquest_info.running == GQUEST_WAITING && !IS_IMMORTAL(ch) )
        {
	    sprintf(buf, "{wWaiting on Participants...{x\n\r");
	    add_buf(final, buf);
	}
	else
	{
	    for ( i = 0; i < gquest_info.mob_count; i++ )
	    {
		if ( (mob = get_mob_index(ch->pcdata->gq_mobs[i])) != NULL )
		{
		    sprintf(mob_short, "%s", mob->short_descr);

		    sprintf(buf, "{w%2d{C) {c[{w%-20.20s{c]  {w%s\n\r",
			i + 1, mob->area->name, end_string(mob_short,52));
		    add_buf(final, buf);
		}
	    }
	}

	add_buf(final, "{x");
	page_to_char(final->string, ch);
	free_buf(final);

	return;
    }

    else if ( !str_prefix(arg1, "complete") )
    {
	if ( !ON_GQUEST(ch) )
	{
	    send_to_char("Your not in a global quest.\n\r", ch);
	    return;
	}

	if ( count_gqmobs(ch) != gquest_info.mob_count )
	{
	    sprintf(buf, "You haven't finished just yet, there is still %d mobs to kill.\n\r",gquest_info.mob_count - count_gqmobs(ch));
	    send_to_char(buf,ch);
	    return;
	}

	send_to_char("{RCongratulations!! You have completed the global quest.{x\n\r", ch);
	ch->pcdata->deviant_points[0] += gquest_info.qpoints;
	ch->pcdata->deviant_points[1] += gquest_info.qpoints;
 	ch->pcdata->questpoints += gquest_info.qps;
	add_cost(ch,gquest_info.gold,VALUE_GOLD);
	post_gquest(ch);
	sprintf(buf, "{GReceived {Y%d {GGold, {Y%d {GDeviant Point%s and {Y%d{G Quest Points.{x\n\r",
	    gquest_info.gold, gquest_info.qpoints, gquest_info.qpoints == 1 ? "" : "s", gquest_info.qps);
	send_to_char(buf,ch);
	end_gquest();
	sprintf(buf, "{g%s{g has completed the Global Quest!{x", ch->name);
	gquest_channel(ch, buf);
//	news(buf,ch,NULL,NEWS_GQUEST,0,0);
	return;
    }
    else
	do_gquest(ch, "");
    return;
}

void gquest_update ( void )
{
    char buf[MAX_STRING_LENGTH];

    if ( gquest_info.running == GQUEST_OFF )
    {
        if ( --gquest_info.next <= 0 )
            auto_gquest (  );
    }

    else if ( gquest_info.running == GQUEST_WAITING )
    {
        gquest_info.timer--;

        if ( gquest_info.timer > 0 )
        {
            sprintf ( buf, "{W%d {gminute%s left to join the global quest. {w({WLevels %d - %d{w){x",
                       gquest_info.timer, gquest_info.timer == 1 ? "" : "s",
                       gquest_info.minlevel, gquest_info.maxlevel );
                gquest_channel(NULL, buf);
        }
        else
        {
            if ( gquest_info.involved == 0 )
            {
                end_gquest ( );
                sprintf ( buf, "{gNot enough people for the global quest. The next quest will start in {W%d {gminutes.{x", gquest_info.next );
                gquest_channel(NULL, buf);
//		news(buf,NULL,NULL,NEWS_GQUEST,0,0);
            }
            else
            {
                gquest_info.timer =
                    number_range ( 2 * gquest_info.mob_count,
                                   4 * gquest_info.mob_count );

	    	if ( gquest_info.timer < 20 ) { gquest_info.timer = 20; }

		while ( gquest_info.timer % 5 != 0 )
			--gquest_info.timer;

                gquest_info.running = GQUEST_RUNNING; 
                sprintf ( buf, "{gThe Global Quest begins! You have {W%d {gminutes, Good Luck!{x",gquest_info.timer );
                gquest_channel(NULL, buf);
//		news(buf,NULL,NULL,NEWS_GQUEST,0,0);
            }
        }
    }

    else if ( gquest_info.running == GQUEST_RUNNING )
    {
        if ( gquest_info.involved == 0 )
        {
            end_gquest ( );
            sprintf ( buf, "{gQuest Incomplete, No participants left to participate.{x");
            gquest_channel(NULL, buf);
//	    news(buf,NULL,NULL,NEWS_GQUEST,0,0);
            return;
        }

        switch ( gquest_info.timer )
        {
            case 0:
                end_gquest ( );
                sprintf ( buf, "{gTime has run out on the Global Quest, next quest will start in {W%d {gminutes.{x",gquest_info.next );
	        gquest_channel(NULL, buf);
//    		news(buf,NULL,NULL,NEWS_GQUEST,0,0);
                return;
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 10:
                sprintf( buf, "{W%d {gminute%s remaining in the global quest.{x",
                           gquest_info.timer, gquest_info.timer > 1 ? "s" : "" );
                gquest_channel(NULL, buf);
            default:
                gquest_info.timer--;
                break;
        }
        return;
    }
}

int is_gqmob( CHAR_DATA *ch, int vnum )
{
    int i;

    if ( gquest_info.running == GQUEST_OFF
    ||   gquest_info.running == GQUEST_WAITING )
	return -1;

    for ( i = 0; i < gquest_info.mob_count && gquest_info.mob_count < MAX_GQUEST_MOB; i++ )
    {
	if ( ch && !IS_NPC(ch) )
	{
	    if ( ch->pcdata->gq_mobs[i] == vnum )
		return i;
	    else
		continue;
	} else {
	    if ( gquest_info.mobs[i] == vnum )
		return i;
	    else
		continue;
	}
    }

    return -1;
}

void check_gquest( CHAR_DATA *killer, CHAR_DATA *victim )
{
    int i;

    if ( ON_GQUEST( killer )
    &&   ( i = is_gqmob( killer, victim->pIndexData->vnum ) ) != -1 )
    {
	killer->pcdata->gq_mobs[i] = -1;
	send_to_char( "{GYou have elminated a global quest target!{x\n\r", killer );

	if ( count_gqmobs( killer ) == gquest_info.mob_count )
	    do_gquest( killer, "complete" );
    }
}

bool is_gq_target( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->pcdata != NULL
    &&   victim->pIndexData != NULL
    &&   is_gqmob( ch, victim->pIndexData->vnum ) != -1 )
	return TRUE;

    return FALSE;
}

