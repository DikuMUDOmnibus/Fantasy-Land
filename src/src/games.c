#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"

#define MAX_BARS	10
#define MAX_BAR_SLOT	3

const	struct	target_type	slot_table[MAX_BAR_SLOT] =
{
    { "{rS{Rhry{rp    ", 10 },
    { "{WZ{ca{Cr{ck{Wo    ",  9 },
    { "{YA{wl{Wt{wo{Yn    ",  8 }
};

void do_pull( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *slots;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int bar[MAX_BARS], count, pos, value, wager;
    bool winner = TRUE;

    if ( IS_NPC( ch ) )
	return;

    argument = one_argument( argument, arg );

    for ( slots = ch->in_room->contents; slots != NULL; slots = slots->next_content )
    {
	if ( slots->item_type == ITEM_SLOTS && can_see_obj( ch, slots ) )
	    break;
    }

    if ( slots == NULL )
    {
	send_to_char( "Does this look like a casino to you?\n\r", ch );
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Syntax: pull jackpots\n\r"
		      "        pull <wager> <money type>\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "jackpots" ) )
    {
	sprintf( buf, "The current slots jackpot is %d platinum, %d gold, and %d silver.\n\r",
	    slots->value[VALUE_PLATINUM], slots->value[VALUE_GOLD],
	    slots->value[VALUE_SILVER] );
	send_to_char( buf, ch );
	return;
    }

    if ( !is_number( arg )
    ||   ( wager = atoi( arg ) ) < 1
    ||   wager > 20 )
    {
	send_to_char( "Wager must be between 1 and 20 coins.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	do_pull( ch, "" );
	return;
    }

    if ( !str_prefix( argument, "platinum" ) )
    {
	value	= VALUE_PLATINUM;
	pos	= ch->platinum;
    }

    else if ( !str_prefix( argument, "gold" ) )
    {
	value	= VALUE_GOLD;
	pos	= ch->gold + ( 100 * ch->platinum );
    }

    else if ( !str_prefix( argument, "silver" ) )
    {
	value	= VALUE_SILVER;
	pos	= ch->silver + ( 100 * ch->gold ) + ( 10000 * ch->platinum );
    }

    else
    {
	do_pull( ch, "" );
	return;
    }


    if ( pos < wager )
    {
	send_to_char( "You can not cover that wager.\n\r", ch );
	return;
    }

    deduct_cost( ch, wager, value );
    slots->value[value] += wager;

    sprintf( buf, "You drop %d %s coin%s into %s and pull the handle!\n\n{C-=*",
	wager, value == VALUE_PLATINUM ? "platinum" :
	value == VALUE_GOLD ? "gold" : "silver", wager > 1 ? "s" : "",
	slots->short_descr );

    count	= URANGE( 2, slots->value[3], MAX_BARS		);

    for ( pos = 0; pos < count; pos++ )
    {
	bar[pos] = number_range( 0, MAX_BAR_SLOT-1 );
	strcat( buf, "{C| " );
	strcat( buf, slot_table[bar[pos]].name );

	if ( pos > 0 && bar[pos] != bar[pos-1] )
	    winner = FALSE;
    }

    strcat( buf, " {C|*=-{x\n\n" );

    if ( winner )
    {
	count = slots->value[value] * slot_table[bar[0]].bit / 10 * wager / 20;

	if ( bar[0] == 0 )
	    send_sound_char( ch, 75, 1, 100, "effects", "ST_WIN3.WAV", SOUND_NOMISC );
	else
	    send_sound_char( ch, 75, 1, 100, "effects", "ST_WIN1.WAV", SOUND_NOMISC );

	sprintf( arg, "{C{zWinner!{x\n{CYour payoff is %d %s!{x\n",
	    count,
	    value == VALUE_PLATINUM ? "platinum" :
	    value == VALUE_GOLD ? "gold" : "silver" );
	strcat( buf, arg );

	add_cost( ch, count, value );
	slots->value[value] =
	  UMAX( slots->pIndexData->value[value], slots->value[value] - count );
    }

    sprintf( arg, "The new jackpot is %d %s.\n\r",
	slots->value[value],
	value == VALUE_PLATINUM ? "platinum" :
	value == VALUE_GOLD ? "gold" : "silver" );
    strcat( buf, arg );

    send_to_char( buf, ch );
    WAIT_STATE( ch, PULSE_VIOLENCE );
}
