#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"

#define MAX_CHART	11

typedef struct chart_listings CHART_RANK;

CHART_RANK * chart_list[MAX_CHART], * rank_free;

struct chart_types
{
    char *	name;
    char *	show;
    bool	only_pk;
};

struct chart_listings
{
    CHART_RANK *next;
    char *	name;
    long	total;
};

const struct chart_types chart_table[MAX_CHART] =
{
    { "assists",	"Assists",		TRUE	},
    { "bounty",		"Bounty",		FALSE	},
    { "damage",		"Most Damage",		FALSE	},
    { "deviant",	"Deviant Pts",	        FALSE	},
    { "experience",	"Experience",		FALSE	},
    { "hours",		"Hours",		FALSE	},
    { "mobdeaths",	"Mobile Deaths",	FALSE	},
    { "mobkills",	"Mobile Kills",		FALSE	},
    { "pdeaths",	"Player Deaths",	TRUE	},
    { "pkills",		"Player Kills",		TRUE	},
    { "pkpoints",	"PK Points",		TRUE	}
};

void update_chart_ranks( CHAR_DATA *ch )
{
    rank_chart( ch, "pkills",		ch->pcdata->pkills		);
    rank_chart( ch, "pdeaths",		ch->pcdata->pdeath		);
    rank_chart( ch, "assists",		ch->pcdata->assist		);
    rank_chart( ch, "bounty",		ch->pcdata->bounty		);
    rank_chart( ch, "experience",	ch->exp				);
    rank_chart( ch, "pkpoints",		ch->pcdata->pkpoints		);
    rank_chart( ch, "mobkills",		ch->pcdata->mobkills		);
    rank_chart( ch, "mobdeaths",	ch->pcdata->mobdeath		);
    rank_chart( ch, "damage",		ch->pcdata->max_damage		);
    rank_chart( ch, "deviant",		ch->pcdata->deviant_points[1]	);
    rank_chart( ch, "hours",		(int) (ch->pcdata->played + current_time - ch->pcdata->logon) / 3600 );
}

CHART_RANK * new_rank( )
{
    CHART_RANK *rank;

    if ( rank_free == NULL )
	rank = alloc_perm( sizeof( *rank ) );
    else
    {
	rank = rank_free;
	rank_free = rank_free->next;
    }

    rank->name	= NULL;
    rank->total	= 0;

    return rank;
}

void free_rank( CHART_RANK *rank )
{
    free_string( rank->name );

    rank->name	= NULL;
    rank->next	= rank_free;
    rank_free	= rank;
}

int chart_lookup( const char *name )
{
   int chart;

   for ( chart = 0; chart < MAX_CHART; chart++ )
   {
	if ( !str_prefix( name, chart_table[chart].name ) )
	    return chart;
   }

   return -2;
}

void load_charts( bool clear )
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    sh_int chart;
    long stat;

    for ( chart = 0; chart < MAX_CHART; chart++ )
    {
	CHART_RANK *rank, *prev = NULL;

	sprintf( buf, "../data/charts/%s", chart_table[chart].name );

	if ( ( fp = fopen( buf, "r" ) ) == NULL )
	{
	    sprintf( log_buf, "Could not open file %s in order to read.", buf );
	    bug( log_buf, 0 );
	    continue;
	}

	for ( stat = fread_number( fp ); stat != -1; stat = fread_number( fp ) )
	{
	    rank		= new_rank( );
	    rank->name		= str_dup( fread_word( fp ) );
	    rank->total		= stat;

	    if ( prev == NULL )
		chart_list[chart] = rank;
	    else
		prev->next = rank;
	    prev = rank;
	}

	fclose ( fp );
    }

    return;
}

void save_chart_data( sh_int ichart )
{
    CHART_RANK *rank;
    FILE *fp;
    char buf[128];

    sprintf( buf, "../data/charts/%s", chart_table[ichart].name );

    if ( ( fp = fopen( buf, "w" ) ) == NULL )
    {
	sprintf( log_buf, "Save_charts: Could not open charts file %s.", buf );
	bug( log_buf, 0 );
	return;
    }

    for ( rank = chart_list[ichart]; rank != NULL; rank = rank->next )
	fprintf( fp, "%ld %s\n", rank->total, rank->name );

    fprintf( fp, "-1\n" );
    fclose( fp );
}

void save_charts( )
{
    sh_int ichart;

    for ( ichart = 0; ichart < MAX_CHART; ichart++ )
    {
	if ( chart_table[ichart].name == NULL )
	    return;

	save_chart_data( ichart );
    }
}

bool chart_remove( char *name, sh_int ichart )
{
    CHART_RANK *rank, *prev;

    for ( rank = chart_list[ichart]; rank != NULL; rank = rank->next )
    {
	if ( !str_cmp( name, rank->name ) )
	{
	    if ( rank == chart_list[ichart] )
		chart_list[ichart] = rank->next;
	    else
	    {
		for ( prev = chart_list[ichart]; prev != NULL; prev = prev->next )
		{
		    if ( prev->next == rank )
		    {
			prev->next = rank->next;
			break;
		    }	
		}
	    }

	    free_rank( rank );
	    return TRUE;
	}
    }

    return FALSE;
}

void unrank_charts( CHAR_DATA *ch )
{
    sh_int ichart;

    for ( ichart = 0; ichart < MAX_CHART; ichart++ )
    {
	if ( chart_remove( ch->name, ichart ) )
	    save_chart_data( ichart );
    }

    return;
}

void new_chart_ranking( char *name, sh_int ichart, int value )
{
    CHART_RANK *rank, *new;

    if ( chart_list[ichart] == NULL || chart_list[ichart]->total < value )
    {
	new			= new_rank( );
	new->name		= str_dup( name );
	new->total		= value;
	new->next		= chart_list[ichart];
	chart_list[ichart]	= new;
	return;
    }

    for ( rank = chart_list[ichart]; rank != NULL; rank = rank->next )
    {
	if ( !rank->next || rank->next->total < value )
	{
	    new		= new_rank( );
	    new->name	= str_dup( name );
	    new->total	= value;
	    new->next	= rank->next;
	    rank->next	= new;
	    return;
	}
    }
}

void rank_chart( CHAR_DATA *ch, char *chart, int value )
{
    sh_int ichart;

    if ( IS_NPC( ch ) || ch->level >= LEVEL_IMMORTAL || value == 0 )
	return;

    if ( ( ichart = chart_lookup( chart ) ) == -2 )
    {
	bug( "Chart %s not found", *chart );
	return;
    }

    if ( chart_table[ichart].only_pk && !is_pkill( ch ) )
	return;

    chart_remove( ch->name, ichart );
    new_chart_ranking( ch->name, ichart, value );

    save_chart_data( ichart );
}

void do_charts( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    CHART_RANK *rank1, *rank2;
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    sh_int chart, pos, max_found;
    bool found = FALSE;

    if ( argument[0] == '\0' )
    {
	if ( IS_IMMORTAL( ch ) )
	{
	    send_to_char( "{qImmortal Options: {tchart save  {s({tsaves all lists{s)\n\r"
//			  "                  {tchart load  {s({treloads all lists{s)\n\r"
//			  "                  {tchart clean {s({tremoves all missing pfiles{s)\n\r"
			  "                  {tchart zero  {s({tresets all charts{s)\n\r\n\r", ch );
	}

	send_to_char( "{qSyntax: {tChart {s({tchart{s) ( {t# matches {s/ {t'all' {s)\n\r\n\r{qValid charts:\n\r{s---------------------\n\r  {s({t 0{s) {qAll\n\r", ch );

	for ( pos = 0; pos < MAX_CHART && chart_table[pos].name; pos++ )
	{
	    sprintf( buf, "  {s({t%2d{s) {q%s{x\n\r", pos+1, capitalize( chart_table[pos].name ) );
	    send_to_char( buf, ch );
	}
	return;
    }

    if ( IS_IMMORTAL( ch ) )
    {
	if ( !str_cmp( argument, "save" ) )
	{
	    save_charts( );
	    send_to_char( "Charts saved.\n\r", ch );
	    return;
	}
/*
	else if ( !str_cmp( argument, "load" ) )
	{
	    load_charts( FALSE );
	    send_to_char( "Charts loaded.\n\r", ch );
	    return;
	}

	else if ( !str_cmp( argument, "clean" ) )
	{
	    load_charts( TRUE );
	    save_charts( );
	    send_to_char( "Charts cleared of all invalid names.\n\r", ch );
	    return;
	}
*/
	else if ( !str_cmp( argument, "zero" ) )
	{
	    for ( chart = 0; chart < MAX_CHART; chart++ )
	    {
		for ( rank1 = chart_list[chart]; rank1 != NULL; rank1 = rank2 )
		{
		    rank2 = rank1->next;

		    free_rank( rank1 );
		}

		chart_list[chart] = NULL;
	    }

	    send_to_char( "Charts wiped clean.\n\r", ch );
	    return;
	}
    }

    argument = one_argument( argument, arg );

    if ( is_number( arg ) )
	chart = atoi( arg )-1;

    else if ( !str_cmp( arg, "all" ) )
	chart = -1;

    else
	chart = chart_lookup( arg );

    if ( chart < -1 || chart >= MAX_CHART )
    {
	send_to_char( "Invalid chart.\n\r\n\r", ch );
	do_charts( ch, "" );
	return;
    }

    if ( chart == -1 )
    {
	final = new_buf( );

	add_buf( final,	"{s -------------------------------------------------------\n\r" );

	for ( chart = 0; chart < MAX_CHART; chart++ )
	{
	    if ( chart_table[chart].name == NULL )
		break;

	    pos = 0;
	    for ( rank1 = chart_list[chart]; rank1 != NULL; rank1 = rank1->next )
	    {
		pos++;

		if ( !str_cmp( ch->name, rank1->name ) )
		{
		    sprintf( buf, "{s| {qPos: {s({t%3d{s)    {qChart: {t%-15s{s    {qTotal:{t%7ld{s |\n\r",
			pos, chart_table[chart].show, rank1->total );
		    add_buf( final, buf );
		    found = TRUE;
		    break;
		}
	    }
	}

	add_buf( final,	"{s -------------------------------------------------------{x\n\r" );

	if ( !found )
	    send_to_char( "You are not listed on any charts.\n\r", ch );
	else
	    page_to_char( final->string, ch );

	free_buf( final );

	return;
    }

    max_found = 0;
    for ( rank1 = chart_list[chart]; rank1 != NULL; rank1 = rank1->next )
	max_found++;

    if ( argument[0] == '\0' )
	max_found = UMIN( max_found, 60 );

    else if ( str_cmp( argument, "all" ) )
    {
	if ( !is_number( argument )
	||   ( max_found = UMIN( max_found, atoi( argument ) ) ) <= 0 )
	{
	    do_charts( ch, "" );
	    return;
	}
    }

    if ( max_found % 2 != 0 )
	max_found++;

    pos = 0;
    for ( rank2 = chart_list[chart]; rank2 != NULL && pos < max_found / 2; rank2 = rank2->next )
	pos++;

    final = new_buf( );

    add_buf( final, "{s ------------------------------         ------------------------------\n\r" );

    sprintf( buf, "{s| ({qPos{s) {tHolder {q%15s {s|       | ({qPos{s) {tHolder {q%15s {s|\n\r",
	chart_table[chart].show, chart_table[chart].show );
    add_buf( final, buf );

    add_buf( final, " ------------------------------         ------------------------------\n\r" );

    rank1 = chart_list[chart];
    for ( pos = 1; pos <= max_found/2; pos++ )
    {
	if ( rank1 )
	{
	    if ( !str_cmp( ch->name, rank1->name ) )	
		sprintf( buf, "| ({R%3d{s) {R%-13s {R%8ld {s|",
		    pos, rank1->name, rank1->total );

	    else
		sprintf( buf, "| ({q%3d{s) {t%-13s {q%8ld {s|",
		    pos, rank1->name, rank1->total );
	    add_buf( final, buf );
	    rank1 = rank1->next;
	} else {
	    add_buf( final, "|                              |" );
	}

	if ( rank2 )
	{
	    if ( !str_cmp( ch->name, rank2->name ) )	
		sprintf( buf, "       | ({R%3d{s) {R%-13s {R%8ld {s|\n\r",
		    max_found/2+pos, rank2->name, rank2->total );

	    else
		sprintf( buf, "       | ({q%3d{s) {t%-13s {q%8ld {s|\n\r",
		    max_found/2+pos, rank2->name, rank2->total );
	    add_buf( final, buf );
	    rank2 = rank2->next;
	} else {
	    add_buf( final, "       |                              |\n\r" );
	}
    }

    add_buf( final, " ------------------------------         ------------------------------{x\n\r" );

    page_to_char( final->string, ch );
    free_buf( final );
    return;
}
