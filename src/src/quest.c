/****************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com    *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this   *
*  code is allowed provided you add a credit line to the effect of:         *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest     *
*  of the standard diku/rom credits. If you use this or a modified version  *
*  of this code, let me know via email: moongate@moongate.ams.com. Further  *
*  updates will be posted to the rom mailing list. If you'd like to get     *
*  the latest version of quest.c, please send a request to the above add-   *
*  ress. Quest Code v2.03. Please do not remove this notice from this file. *
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "recycle.h"

#define QUEST_TOKEN_LOW		1200
#define QUEST_TOKEN_HIGH	1205

DECLARE_DO_FUN( do_say		);

void complete_quest( CHAR_DATA *ch, CHAR_DATA *questman )
{
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    int bonus, gold, points, pos, random = number_percent( );

    gold   = number_range( mud_stat.quest_gold[0],   mud_stat.quest_gold[1] );
    points = number_range( mud_stat.quest_points[0], mud_stat.quest_points[1] );

    sprintf( buf, "Congratulations on completing your quest, %s!", ch->name );
    do_say( questman, buf );

    sprintf( buf, "As a reward, I am giving you %d quest point%s, and %d gold.",
	points, points != 1 ? "s" : "", gold );
    do_say( questman, buf );

    if ( random <= 4 )
    {
	do_say( questman, "Since you were so hasty I'm giving you a bonus!" );
	send_to_char( "{YYou gain a {wD{Devian{wt {rP{Roin{rt{Y!{x\n\r", ch );
	ch->pcdata->deviant_points[0]++;
	ch->pcdata->deviant_points[1]++;
    }

    else if ( random <= 10 )
    {
	bonus = number_range( mud_stat.quest_object[0], mud_stat.quest_object[1] );

	sprintf( buf, "Since you were so hasty I'm giving you %d bonus%s!",
	    bonus, bonus == 1 ? "" : "es" );
	do_say( questman, buf );

	for ( pos = 0; pos < bonus; pos++ )
	{
	    obj = create_object( get_obj_index( number_range( mud_stat.quest_obj_vnum[0], mud_stat.quest_obj_vnum[1] ) ) );
	    if ( obj == NULL )
	    {
		do_say( questman, "I can't seem to find the object vnum, could you tell an Immortal to contact me?" );
		continue;
	    }

	    act( "$N gives you $p.",
		ch, obj, questman, TO_CHAR, POS_RESTING );
	    act( "$N gives $p to $n.",
		ch, obj, questman, TO_ROOM, POS_RESTING );
	    obj_to_char( obj, ch );
	}
    }

    else if ( random > 96 )
    {
	bonus = number_range( mud_stat.quest_pracs[0], mud_stat.quest_pracs[1] );

	sprintf( buf, "I'm granting you %d practice points!", bonus );
	do_say( questman, buf );

	ch->pcdata->practice += bonus;
    }

    else if ( random > 85 )
    {
	bonus = number_range( mud_stat.quest_exp[0], mud_stat.quest_exp[1] );

	sprintf( buf, "I'm granting you %d experience points!", bonus );
	do_say( questman, buf );
	gain_exp( ch, bonus );
    }

    REMOVE_BIT( ch->act, PLR_QUESTOR );
    ch->pcdata->questgiver	= 0;
    ch->pcdata->countdown	= 0;
    ch->pcdata->questmob	= 0;
    ch->pcdata->questobj	= 0;
    ch->pcdata->questroom	= NULL;
    ch->pcdata->questarea	= NULL;
    ch->pcdata->nextquest	= IS_IMMORTAL( ch ) ? 0 : 10;

    ch->pcdata->total_questcomplete++;
    ch->pcdata->questpoints += points;
    ch->pcdata->total_questpoints+= points;
    add_cost( ch, gold, VALUE_GOLD );
}

bool quest_level_diff( int clevel, int mlevel )
{
    if ( ( clevel < 6 && mlevel < 10 )
    ||   ( clevel > 5 && clevel < 20 && mlevel < 22 )
    ||   ( clevel > 15 && clevel < 30 && mlevel > 20 && mlevel < 35 )
    ||   ( clevel > 25 && clevel < 45 && mlevel > 30 && mlevel < 50 )
    ||   ( clevel > 40 && clevel < 65 && mlevel > 45 && mlevel < 70 )
    ||   ( clevel > 55 && clevel < 75 && mlevel > 60 && mlevel < 75 )
    ||   ( clevel > 65 && clevel < 80 && mlevel > 70 && mlevel < 80 )
    ||   ( clevel > 78 && clevel < 92 && mlevel > 75 && mlevel < 96 )
    ||   ( clevel > 85 && clevel < 100 && mlevel > 85 && mlevel < 108 )
    ||   ( clevel > 95 && mlevel > 90 ) )
	return TRUE;

    return FALSE;
}
                
void generate_quest( CHAR_DATA *ch, CHAR_DATA *questman )
{
    MOB_INDEX_DATA *pMob;
    CHAR_DATA *victim = NULL;
    OBJ_DATA *questitem;
    char buf[MAX_STRING_LENGTH];
    int mob_vnum, mcounter;

    sprintf( buf, "Thank you, brave %s!", ch->name );
    do_say( questman, buf );

    for ( mcounter = 0; mcounter < 5000; mcounter++ )
    {
	mob_vnum = number_range( 100, 55200 );

	if ( ( pMob = get_mob_index( mob_vnum ) ) != NULL )
	{
	    if ( pMob->bank_branch != 0
	    ||   pMob->pShop != NULL
	    ||   IS_SET( pMob->act, ACT_NOQUEST )
	    ||   IS_SET( pMob->act, ACT_PET )
	    ||   IS_SET( pMob->act, ACT_PRACTICE )
	    ||   IS_SET( pMob->act, ACT_TRAIN )
	    ||   IS_SET( pMob->act, ACT_IS_HEALER )
	    ||   IS_SET( pMob->act, ACT_IS_PRIEST )
	    ||   IS_SET( pMob->act, ACT_IS_SATAN )
	    ||   IS_SET( pMob->area->area_flags, AREA_UNLINKED )
	    ||   IS_SET( pMob->area->area_flags, AREA_NO_QUEST )
	    ||   IS_SET( pMob->area->area_flags, AREA_SPECIAL )
	    ||   !quest_level_diff( ch->level, pMob->level ) )
		continue;

	    for ( victim = char_list; victim != NULL; victim = victim->next )
	    {
		if ( victim->pIndexData != NULL
		&&   victim->pIndexData->vnum == mob_vnum )
		    break;
	    }

	    if ( victim != NULL
	    &&   victim->in_room != NULL
	    &&   victim->fighting == NULL
	    &&   victim->pIndexData != NULL
	    &&   !IS_SET( victim->affected_by, AFF_CHARM )
	    &&   !IS_SET( victim->in_room->room_flags, ROOM_PET_SHOP )
	    &&   !IS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
		break;
	}
    }

    if ( victim == NULL  )
    {
	do_say( questman, "I'm sorry, but I don't have any quests for you at this time." );
	do_say( questman, "Try again later." );
	ch->pcdata->nextquest = 3;
	return;
    }

    if ( victim->carrying != NULL
    &&   number_percent( ) <= 20 )
    {
	questitem		= victim->carrying;
	ch->pcdata->questobj	= questitem->pIndexData->vnum;
	ch->pcdata->questmob	= victim->pIndexData->vnum;
	ch->pcdata->questarea	= victim->in_room->area->name;
	ch->pcdata->questroom	= victim->in_room->name;

	sprintf( buf, "$N says '{S%s{S has been stolen from the royal treasury by %s{S!{x'",
	    questitem->short_descr, PERS( victim, ch ) );
	act( buf, ch, NULL, questman, TO_CHAR, POS_RESTING );

	act( "$N says '{SMy court wizardess, with her magic mirror, has pinpointed its location.{x'",
	    ch, NULL, questman, TO_CHAR, POS_RESTING );

	sprintf( buf, "$N says '{SLook in the general area of %s {Sfor %s{S!{x'",
	    victim->in_room->area->name, victim->in_room->name );
	act( buf, ch, NULL, questman, TO_CHAR, POS_RESTING );
    }

    else if ( number_percent( ) <= 35 )
    {
	questitem		= create_object( get_obj_index(
	    number_range( QUEST_TOKEN_LOW, QUEST_TOKEN_HIGH ) ) );

	if ( questitem == NULL )
	{
	    send_to_char( "Fatal error, missing quest token!\n\r", ch );
	    return;
	}

	ch->pcdata->questobj	= questitem->pIndexData->vnum;
	ch->pcdata->questmob	= 0;
	ch->pcdata->questarea	= victim->in_room->area->name;
	ch->pcdata->questroom	= victim->in_room->name;
	obj_to_room( questitem, victim->in_room );

	sprintf( buf, "$N says '{SVile pilferers have stolen %s{S from the royal treasury!{x'",
	    questitem->short_descr );
	act( buf, ch, NULL, questman, TO_CHAR, POS_RESTING );

	act( "$N says '{SMy court wizardess, with her magic mirror, has pinpointed its location.{x'",
	    ch, NULL, questman, TO_CHAR, POS_RESTING );

	sprintf( buf, "$N says '{SLook in the general area of %s {Sfor %s{S!{x'",
	    victim->in_room->area->name, victim->in_room->name );
	act( buf, ch, NULL, questman, TO_CHAR, POS_RESTING );
    }

    else
    {
	switch( number_range( 0, 1 ) )
	{
	    case 0:
		sprintf( buf, "$N says '{SAn enemy of mine, %s{S, is making vile threats against the crown.{x'",
		    victim->short_descr );
		act( buf, ch, NULL, questman, TO_CHAR, POS_RESTING );

		act( "$N says '{SThis threat must be eliminated!{x'",
		    ch, NULL, questman, TO_CHAR, POS_RESTING );
		break;

	    case 1:
		sprintf( buf, "$N says '{SRune's most heinous criminal, %s{S, has escaped from the dungeon!{x'",
		    victim->short_descr );
		act( buf, ch, NULL, questman, TO_CHAR, POS_RESTING );

		sprintf( buf, "$N says '{SSince the escape, %s{S has murdered %d civillians!{x'",
		    victim->short_descr, number_range( 2, 35 ) );
		act( buf, ch, NULL, questman, TO_CHAR, POS_RESTING );

		act( "$N says '{SThe penalty for this crime is death, and you are to deliver the sentence!{x'",
		    ch, NULL, questman, TO_CHAR, POS_RESTING );
		break;
	}

	if ( victim->in_room->name != NULL )
	{
	    sprintf( buf, "$N says '{SSeek %s{S out somewhere in the vicinity of %s{S!{x'",
		victim->short_descr, victim->in_room->name );
	    act( buf, ch, NULL, questman, TO_CHAR, POS_RESTING );

	    sprintf( buf, "$N says '{SThat location is in the general area of %s.{x'",
		victim->in_room->area->name );
	    act( buf, ch, NULL, questman, TO_CHAR, POS_RESTING );

	    ch->pcdata->questroom = victim->in_room->name;
	    ch->pcdata->questarea = victim->in_room->area->name;
	}

	ch->pcdata->questmob = victim->pIndexData->vnum;
	ch->pcdata->questobj = 0;
    }
}

void do_quest( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *questman;
    OBJ_DATA *obj = NULL;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    if ( IS_NPC( ch ) )
	return;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char("{GQUEST {gcommands{w:\n\r"
		     "  {GQUEST INFO{w:            {gdisplay your quest history\n\r"
		     "  {GQUEST REQUEST{w:         {gwish for a quest\n\r"
		     "  {GQUEST COMPLETE{w:        {ginform questmaster you are done\n\r"
		     "  {GQUEST BUY <item>{w:      {gpurchase an item\n\r"
		     "  {GQUEST SELL <item>{w:     {greturn an item for half price\n\r"
		     "  {GQUEST IDENTIFY <item>{w: {gevaluate items for sale\n\r"
		     "  {GQUEST QUIT/FAIL{w:       {gcancel current quest\n\r"
		     "  {GQUEST LIST <weapons|armor|other|all> <lvl> <lvl>{w: {gdisplay current items\n\r"
		     "{gFor more information, type '{GHELP QUEST{g'.{x\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "info" ) )
    {
	BUFFER *final = new_buf();
	OBJ_INDEX_DATA *questinfoobj;
	MOB_INDEX_DATA *questinfomob;
	char buf2[MAX_STRING_LENGTH];
	char *questgiver;

	add_buf( final, "{C -{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={r( {RQuest Info {r){c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C- \n\r"
			"{C| {c************************************************************************ {C|\n\r" );
	sprintf( buf,	"{C| {c*     {wQuest Points{c:   {r({w%6d{r)            {wTotal Points{c:    {r({w%6d{r)    {c* {C|\n\r",
	    ch->pcdata->questpoints, ch->pcdata->total_questpoints );
	add_buf( final, buf );

	sprintf( buf,	"{C| {c*     {wQuest Attempts{c: {r({w%6d{r)            {wQuests Complete{c: {r({w%6d{r)    {c* {C|\n\r",
	    ch->pcdata->total_questattempt, ch->pcdata->total_questcomplete );
	add_buf( final, buf );

	sprintf( buf,	"{C| {c*     {wQuests Expired{c: {r({w%6d{r)            {wQuests Failed{c:   {r({w%6d{r)    {c* {C|\n\r",
	    ch->pcdata->total_questexpire, ch->pcdata->total_questfail );
	add_buf( final, buf );

	if ( !IS_SET( ch->act, PLR_QUESTOR ) )
	{
	    sprintf( buf, "{C| {c*     {wQuest Time{c:     {r(    {w%2d{r)                                         {c* {C|\n\r",
		ch->pcdata->nextquest );
	    add_buf( final, buf );
	} else {
	    questinfomob = get_mob_index( ch->pcdata->questgiver );
	    questgiver = questinfomob ? questinfomob->short_descr : "the questmaster";

	    add_buf( final, "{C| {c************************************************************************ {C|\n\r" );

	    if ( ch->pcdata->questmob == -1 )
	    {
		sprintf( buf, "         {wYour quest is complete, inform %s {wimmediately!",
		    questgiver );
		sprintf( buf2, "{C| {c* %s {c* {C|\n\r", end_string( buf, 68 ) );
		add_buf( final, buf2 );
	    } else {
		questinfoobj = get_obj_index( ch->pcdata->questobj );
		questinfomob = get_mob_index( ch->pcdata->questmob );

		if ( questinfoobj == NULL && questinfomob == NULL )
		{
		    sprintf( buf, "          You don't apprear to be on a quest." );
		    sprintf( buf2, "{C| {c* {w%s {c* {C|\n\r", end_string( buf, 69 ) );
		    add_buf( final, buf2 );
		} else {
		    if ( questinfoobj != NULL )
		    {
			sprintf( buf, "         %s {whas given you %d minutes to recover",
			    questgiver, ch->pcdata->countdown );
			sprintf( buf2,"{C| {c* {w%s {c* {C|\n\r", end_string( buf, 68 ) );
			add_buf( final, buf2 );

			sprintf( buf, "         {w%s {wfrom %s{w.",
			    questinfoobj->short_descr, ch->pcdata->questroom );
			sprintf( buf2,"{C| {c* %s {c* {C|\n\r", end_string( buf, 68 ) );
			add_buf( final, buf2 );
		    }

		    else if ( questinfomob != NULL )
		    {
			sprintf( buf, "         {w%s {whas given you %d minutes to eliminate",
			    questgiver, ch->pcdata->countdown );
			sprintf( buf2,"{C| {c* %s {c* {C|\n\r", end_string( buf, 68 ) );
			add_buf( final, buf2 );

			sprintf( buf, "         {w%s {wfrom %s{w.",
			    questinfomob->short_descr, ch->pcdata->questroom );
			sprintf( buf2,"{C| {c* %s {c* {C|\n\r", end_string( buf, 68 ) );
			add_buf( final, buf2 );
		    }

		    sprintf( buf, "         Look around somewhere in %s{w.",
			ch->pcdata->questarea );
		    sprintf( buf2, "{C| {c* {w%s {c* {C|\n\r", end_string( buf, 68 ) );
		    add_buf( final, buf2 );
		}
	    }
	}

	add_buf( final, "{C| {c************************************************************************ {C|\n\r"
			"{C -{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={r( {RQuest Info {r){c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C-{c={C- {x\n\r" );
	page_to_char( final->string, ch );
	free_buf( final );
	return;
    }

    for ( questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room )
    {
	if ( !IS_NPC( questman ) )
	    continue;

	if ( questman->spec_fun == spec_lookup( "spec_questmaster" ) )
	    break;
    }

    if ( questman == NULL
    ||   questman->spec_fun != spec_lookup( "spec_questmaster" ) )
    {
	send_to_char( "You can't do that here.\n\r", ch );
	return;
    }

    if ( questman->fighting != NULL )
    {
	send_to_char( "Wait until the fighting stops.\n\r", ch );
	return;
    }

    if ( !str_prefix( arg, "list" ) )
    {
	OBJ_INDEX_DATA *pObjIndex;
	BUFFER *final, *armor, *weapo, *other;
	char arga[MAX_INPUT_LENGTH], argb[MAX_INPUT_LENGTH];
	int lvl1 = 0, lvl2 = LEVEL_HERO, info, count = 0;

	argument = one_argument( argument, arga );
	argument = one_argument( argument, argb );

	if ( arga[0] != '\0'
	&&   str_prefix( arga, "weapons" )
	&&   str_prefix( arga, "armors" )
	&&   str_prefix( arga, "others" )
	&&   str_prefix( arga, "all" ) )
	{
	    send_to_char( "Quest list arguments: all/[none], weapons, armor, other.\n\r", ch );
	    return;
	}

	if ( argument[0] != '\0' && is_number( argument ) )
	{
	    lvl1 = atoi( argb );
	    lvl2 = atoi( argument );

	    if ( lvl1 < 1 || lvl1 > LEVEL_HERO
	    ||   lvl2 < 1 || lvl2 > LEVEL_HERO )
	    {
		send_to_char( "Levels must be between 1 and Hero.\n\r", ch );
		send_to_char( "Quest list <armor|weapons|other|all> <lvl1> <lvl2>.\n\r", ch );
		return;
	    }

	    if ( lvl1 > lvl2 )
	    {
		send_to_char( "Level 1 must be less than or equal to level 2.\n\r", ch );
		return;
	    }
	}

	armor = new_buf( );
	weapo = new_buf( );
	other = new_buf( );

	for ( info = 0; count < top_obj_index; info++ )
	{
	    if ( ( pObjIndex = get_obj_index( info ) ) != NULL )
	    {
		count++;

		if ( IS_OBJ_STAT( pObjIndex, ITEM_AQUEST )
		&&   pObjIndex->level >= lvl1
		&&   pObjIndex->level <= lvl2 )
		{
		    sprintf( buf, "{w[{y%4d{w] [{c%3d{w] %s\n\r",
			pObjIndex->quest_points, pObjIndex->level,
			pObjIndex->short_descr );

		    if ( pObjIndex->item_type == ITEM_WEAPON )
			add_buf( weapo, buf );
		    else if ( pObjIndex->item_type == ITEM_ARMOR )
			add_buf( armor, buf );
		    else
			add_buf( other, buf );
		}
	    }
	}

	add_buf( other, "{w[{y1 DP{w]       30 Quest Points\n\r" );

	act( "$n asks $N for a list of quest items.",
	    ch, NULL, questman, TO_ROOM, POS_RESTING );
	act( "You ask $N for a list of quest items.",
	    ch, NULL, questman, TO_CHAR, POS_RESTING );

	final = new_buf( );

	add_buf( final, "\n\r{w[{yCost{w] [{cLvl{w] Item\n\r"
			"---------------------------------------\n\r"  );

	if ( arga[0] == '\0'
	||   !str_prefix( arga, "weapons" )
	||   !str_prefix( arga, "all" ) )
	    add_buf( final, weapo->string );

	if ( arga[0] == '\0'
	||   !str_prefix( arga, "armors" )
	||   !str_prefix( arga, "all" ) )
	    add_buf( final, armor->string );

	if ( arga[0] == '\0'
	||   !str_prefix( arga, "other" )
	||   !str_prefix( arga, "all" ) )
	    add_buf( final, other->string );

	free_buf( armor );
	free_buf( weapo );
	free_buf( other );

	add_buf( final, "{x" );

	page_to_char( final->string, ch );
	free_buf( final );
        return;
    }

    else if ( !str_prefix( arg, "sell" ) )
    {
	sh_int points;

	if ( argument[0] == '\0' )
	{
	    act( "$N says '{STo sell an item, type '{RQUEST SELL <item>{S'.{x'",
		ch, NULL, questman, TO_CHAR, POS_RESTING );
	    return;
	}

	if ( ( obj = get_obj_carry( ch, argument ) ) == NULL )
	{
	    act( "$N says '{SYou don't appear to have any items named '$t'.{x'",
		ch, argument, questman, TO_CHAR, POS_RESTING );
	    return;
	}

	if ( !IS_OBJ_STAT( obj, ITEM_AQUEST )
	||   obj->pIndexData->quest_points <= 0 )
	{
	    act( "$N says '{SI only buy quest items, $p {Sdoes not appear to be one.{x'",
		ch, obj, questman, TO_CHAR, POS_RESTING );
	    return;
	}

	points = obj->pIndexData->quest_points / 2;

	act( "$n gives $p to $N.", ch, obj, questman, TO_ROOM, POS_RESTING );
	act( "You give $p to $N.", ch, obj, questman, TO_CHAR, POS_RESTING );

	sprintf( buf, "Thanks for returning %s{S.", obj->short_descr );
	do_say( questman, buf );

	sprintf( buf, "I am reimbursing you %d quest points.", points );
	do_say( questman, buf );

	obj_from_char( obj );
	extract_obj( obj );
	ch->pcdata->questpoints += points;

	return;
    }

    else if ( !str_prefix( arg, "identify" ) )
    {
	BUFFER *final;
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	int pos = 0, info;

	for ( info = 0; pos < top_obj_index; info++ )
	{
	    if ( ( pObjIndex = get_obj_index( info ) ) != NULL )
	    {
		pos++;

		if ( IS_OBJ_STAT( pObjIndex, ITEM_AQUEST )
		&&   is_name( argument, pObjIndex->name ) )
		{
		    obj = create_object( get_obj_index( info ) );
		    final = display_stats( obj, ch, TRUE );
		    page_to_char( final->string, ch );
		    free_buf( final );
		    extract_obj( obj );
		    return;
		}
	    }
	}

	act( "No info for '$t' could be located for identification.",
	    ch, argument, NULL, TO_CHAR, POS_DEAD );
	return;
    }

    else if ( !str_prefix( arg, "buy" ) )
    {
	OBJ_INDEX_DATA *pObjIndex;
	bool found = FALSE;
	int qobj, count = 0;

	if ( argument[0] == '\0' )
	{
	    send_to_char( "To buy an item, type 'QUEST BUY <item>'.\n\r", ch );
	    return;
        }

	if ( is_name( argument, "points" ) )
	{
	    if ( ch->pcdata->deviant_points[0] >= 1 )
	    {
		ch->pcdata->deviant_points[0] -= 1;
		ch->pcdata->questpoints += 30;
		act( "$N gives $n 30 quest points.",
		    ch, NULL, questman, TO_ROOM, POS_RESTING );
		act( "$N gives you 30 quest points.",
		    ch, NULL, questman, TO_CHAR, POS_RESTING );
		return;
	    } else {
		sprintf( buf, "Sorry, %s, but you don't have enough deviant points for that.", ch->name );
		do_say( questman, buf );
		return;
	    }
	}

	for ( qobj = 0; count < top_obj_index; qobj++ )
	{
	    if ( ( pObjIndex = get_obj_index( qobj ) ) != NULL )
	    {
		count++;

		if ( IS_OBJ_STAT( pObjIndex, ITEM_AQUEST )
		&&   pObjIndex->quest_points > 0
		&&   is_name( argument, pObjIndex->name ) )
		{
		    found = TRUE;
		    obj = create_object( get_obj_index( qobj ) );
		    break;
		}
	    }
	}

	if ( !found )
	{
	    act( "No quest items named '$t' are available for purchase.",
		ch, argument, NULL, TO_CHAR, POS_DEAD );
	    return;
	}

	if ( ch->pcdata->questpoints < obj->pIndexData->quest_points )
	{
	    sprintf( buf, "$N says, '{SSorry, $n, but $p {Scosts %d quest points, while you only have %d.{x'",
		obj->pIndexData->quest_points, ch->pcdata->questpoints );
	    act( buf, ch, obj, questman, TO_CHAR, POS_RESTING );
	    extract_obj( obj );
	    return;
	}

	if ( ch->level < obj->level )
	{
	    act( "$N says '{SSorry, $n, but $p {Sis too powerful you you to handle.{x'",
		ch, obj, questman, TO_CHAR, POS_DEAD );
	    extract_obj( obj );
	    return;
	}

	set_obj_loader( ch, obj, IS_IMMORTAL( ch ) ? "IQST" : "QUST" );

	ch->pcdata->questpoints -= obj->pIndexData->quest_points;

	act( "$N gives $p to $n.", ch, obj, questman, TO_ROOM, POS_RESTING );
	act( "$N gives you $p.",   ch, obj, questman, TO_CHAR, POS_RESTING );
	obj_to_char( obj, ch );

	return;
    }

    else if ( !strcmp( arg, "fail" ) || !strcmp( arg, "quit" ) )
    {
	send_to_char( "You inform the questmaster that you are a failure.\n\r", ch );

	if ( !IS_SET( ch->act, PLR_QUESTOR ) )
	{
	    sprintf( buf, "$N says '{SDamn %s, You must suck if you thought you were on a quest.{x'", ch->name );
	    act( buf, ch, NULL, questman, TO_CHAR, POS_RESTING );
	    return;
	} else {
	    sprintf( buf, "$N says '{SI am very disappointed in you %s.{x'", ch->name) ;
	    act( buf, ch, NULL, questman, TO_CHAR, POS_RESTING );

	    act( "$N says '{SI thought you could handle such a easy quest.{x'",
		ch, NULL, questman, TO_CHAR, POS_RESTING );

	    REMOVE_BIT( ch->act, PLR_QUESTOR );
	    ch->pcdata->questgiver	= 0;
	    ch->pcdata->countdown	= 0;
	    ch->pcdata->questmob	= 0;
	    ch->pcdata->questobj	= 0;
	    ch->pcdata->nextquest	= IS_IMMORTAL(ch) ? 0 : 10;
	    ch->pcdata->questroom	= NULL;
	    ch->pcdata->questarea	= NULL;
	    ch->pcdata->total_questfail++;
	    return;
	}
    }

    else if ( !str_prefix( arg, "request" ) )
    {
	act( "You ask $N for a quest.",
	    ch, NULL, questman, TO_CHAR, POS_RESTING );
	act( "$n asks $N for a quest.",
	    ch, NULL, questman, TO_ROOM, POS_RESTING );

	if ( IS_SET( ch->act, PLR_QUESTOR ) )
	{
	    do_say( questman, "You're already on a quest, you dumbass!" );
	    return;
	}

	if ( ch->pcdata->nextquest > 0 )
	{
	    sprintf( buf, "You're very brave, %s, but let someone else have a chance.", ch->name );
	    do_say( questman, buf );

	    do_say( questman, "Come back later." );
	    return;
	}

	generate_quest( ch, questman );

	if ( ch->pcdata->questmob > 0 || ch->pcdata->questobj > 0 )
	{
	    SET_BIT( ch->act, PLR_QUESTOR );
	    ch->pcdata->total_questattempt++;
	    ch->pcdata->questgiver	= questman->pIndexData->vnum;
	    ch->pcdata->countdown	= number_range( 10, 30 );

	    sprintf( buf, "You have %d minutes to complete this quest.", ch->pcdata->countdown );
	    do_say( questman, buf );
	    do_say( questman, "May the gods go with you!" );
	}

	return;
    }

    else if ( !str_prefix( arg, "complete" ) )
    {
	act( "You inform $N you have completed $S quest.",
	    ch, NULL, questman, TO_CHAR, POS_RESTING );
	act( "$n informs $N $e has completed $S quest.",
	    ch, NULL, questman, TO_ROOM, POS_RESTING );

	if ( ch->pcdata->questgiver != questman->pIndexData->vnum )
	{
	    do_say( questman, "I never sent you on a quest! Perhaps you're thinking of someone else." );
	    return;
	}

	if ( IS_SET( ch->act, PLR_QUESTOR ) )
	{
	    if ( ch->pcdata->questmob == -1 && ch->pcdata->countdown > 0 )
	    {
		complete_quest( ch, questman );
		return;
	    }

	    else if ( ch->pcdata->questobj > 0 && ch->pcdata->countdown > 0 )
	    {
		for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
		{
		    if ( obj->pIndexData->vnum == ch->pcdata->questobj )
		    {
			act( "You hand $p to $N.",
			    ch, obj, questman, TO_CHAR, POS_RESTING );
			act( "$n hands $p to $N.",
			    ch, obj, questman, TO_ROOM, POS_RESTING );
			extract_obj( obj );
			complete_quest( ch, questman );
			return;
		    }
		}

		do_say( questman, "You haven't completed the quest yet, but there is still time!" );
		return;
            }

	    else if ( ( ch->pcdata->questmob > 0 || ch->pcdata->questobj > 0 )
		 &&   ch->pcdata->countdown > 0 )
	    {
		do_say( questman, "You haven't completed the quest yet, but there is still time!" );
		return;
            }
	}

	if ( ch->pcdata->nextquest > 0 )
	    sprintf( buf, "But, you didn't complete your quest in time!" );
	else
	    sprintf( buf, "You have to REQUEST a quest first, %s.", ch->name );
	do_say( questman, buf );

	return;
    }

    do_quest( ch, "" );
    return;
}

void quest_update( void )
{
    CHAR_DATA *wch;

    for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
    {
	if ( wch->pcdata->nextquest > 0 )
	{
	    if ( --wch->pcdata->nextquest <= 0 )
		send_to_char( "You may now quest again.\n\r", wch );
	}

	else if ( IS_SET( wch->act, PLR_QUESTOR ) )
	{
	    if ( --wch->pcdata->countdown <= 0 )
	    {
		char buf [MAX_STRING_LENGTH];

		REMOVE_BIT( wch->act, PLR_QUESTOR );
		wch->pcdata->nextquest	= IS_IMMORTAL( wch ) ? 0 : 10;
		wch->pcdata->questgiver	= 0;
		wch->pcdata->countdown	= 0;
		wch->pcdata->questmob	= 0;
		wch->pcdata->questobj	= 0;
		wch->pcdata->questroom	= NULL;
		wch->pcdata->questarea	= NULL;
		wch->pcdata->total_questexpire++;

		sprintf( buf,	"You have run out of time for your quest!\n\r"
				"You may quest again in %d minutes.\n\r",
		    wch->pcdata->nextquest );
		send_to_char( buf, wch );
	    }

	    else if ( wch->pcdata->countdown > 0 && wch->pcdata->countdown < 6 )
		send_to_char( "Better hurry, you're almost out of time for your quest!\n\r", wch );
	}
    }
}
