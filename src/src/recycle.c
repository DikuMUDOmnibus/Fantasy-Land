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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"

#define MAX_BUF_LIST	13
#define BASE_BUF	16
#define BUFFER_SAFE	0
#define BUFFER_OVERFLOW	1
#define BUFFER_FREED	2
#define IS_VALID( data )	( ( data ) != NULL && ( data )->valid )
#define VALIDATE( data )	( ( data )->valid = TRUE )
#define INVALIDATE( data )	( ( data )->valid = FALSE )

AFFECT_DATA	*affect_free;
AREA_DATA	*area_free;
AUCTION_DATA	*auction_free;
BAN_DATA	*ban_free;
BUFFER		*buf_free;
CENSOR_DATA	*censor_free;
CHAR_DATA	*char_free;
DESC_CHECK_DATA	*desc_check_free;
DESCRIPTOR_DATA	*descriptor_free;
EXIT_DATA	*exit_free;
EXTRA_DESCR_DATA*extra_descr_free;
GEN_DATA	*gen_data_free, *gen_data_list;
GRANT_DATA	*grant_free;
HELP_DATA	*help_free;
MEM_DATA	*mem_data_free;
MOB_INDEX_DATA	*mob_index_free;
MULTIPLAY_DATA	*allow_free;
NOTE_DATA	*note_free;
OBJ_DATA	*obj_free;
OBJ_INDEX_DATA	*obj_index_free;
OBJ_MULTI_DATA	*obj_multi_free;
PC_DATA		*pcdata_free, *pcdata_list;
PKILL_DATA	*pkill_free;
PKILL_RECORD	*pk_record_free;
POLL_DATA	*poll_free;
PROG_CODE	*pcode_free;
PROG_LIST	*prog_free;
RESET_DATA	*reset_free;
ROOM_DAMAGE_DATA*room_damage_free;
ROOM_INDEX_DATA	*room_index_free;
SHOP_DATA	*shop_free;
VOTE_DATA	*vote_free;
WIZ_DATA	*wiz_free;
extern	int	top_area, top_ed, top_exit, top_room;
long		last_mob_id, last_pc_id;

const int buf_size[MAX_BUF_LIST] =
{
    16,32,64,128,256,1024,2048,4096,8192,16384,32768,65536,131072
};

sh_int *new_short( sh_int size, sh_int value )
{
    sh_int i, *new = malloc( size * sizeof( sh_int ) );

    for ( i = 0; i < size; i++ )
	new[i] = value;

    return new;
}

sh_int *redo_short( sh_int *data, sh_int size, sh_int pos, sh_int value )
{
    sh_int *new_data;
    int i, lvl;

    if ( pos == -1 )
    {
	new_data = realloc( data, sizeof( sh_int ) * size );
	new_data[size-1] = value;
	return new_data;
    } else {
	new_data = malloc( size * sizeof( sh_int ) );

	for ( i = 0, lvl = 0; i <= size; i++ )
	{
	    if ( i != pos )
	    {
		new_data[lvl] = data[i];
		lvl++;
	    }
	}

	free( data );
	return new_data;
    }
}

void free_short( sh_int *data )
{
    free( data );
    data = NULL;
}

bool *new_bool( sh_int size, bool value )
{
    bool *new = malloc( size * sizeof( bool ) );
    sh_int i;

    for ( i = 0; i < size; i++ )
	new[i] = value;

    return new;
}

bool *redo_bool( bool *data, sh_int size, sh_int pos, bool value )
{
    bool *new_data;
    int i, lvl;

    if ( pos == -1 )
    {
	new_data = realloc( data, sizeof( bool ) * size );
	new_data[size-1] = value;
	return new_data;
    } else {
	new_data = malloc( size * sizeof( bool ) );

	for ( i = 0, lvl = 0; i <= size; i++ )
	{
	    if ( i != pos )
	    {
		new_data[lvl] = data != NULL ? data[i] : value;
		lvl++;
	    }
	}

	free( data );
	return new_data;
    }
}

void free_bool( bool *data )
{
    free( data );
    data = NULL;
}

NOTE_DATA *new_note( )
{
    NOTE_DATA *note;

    if ( note_free == NULL )
	note = alloc_perm( sizeof( *note ) );
    else
    {
	note = note_free;
	note_free = note_free->next;
    }
    return note;
}

void free_note( NOTE_DATA *note )
{
    free_string( note->text    );
    free_string( note->subject );
    free_string( note->to_list );
    free_string( note->date    );
    free_string( note->sender  );

    note->next = note_free;
    note_free   = note;
}

BAN_DATA *new_ban( void )
{
    static BAN_DATA ban_zero;
    BAN_DATA *ban;

    if ( ban_free == NULL )
	ban = alloc_perm( sizeof( *ban ) );
    else
    {
	ban = ban_free;
	ban_free = ban_free->next;
    }

    *ban = ban_zero;

    ban->name = &str_empty[0];
    return ban;
}

void free_ban( BAN_DATA *ban )
{
    free_string( ban->name );

    ban->next = ban_free;
    ban_free = ban;
}

CENSOR_DATA *new_censor( void )
{
    static CENSOR_DATA censor_zero;
    CENSOR_DATA *censor;

    if ( censor_free == NULL )
	censor = alloc_perm( sizeof( *censor ) );
    else
    {
	censor = censor_free;
	censor_free = censor_free->next;
    }

    *censor = censor_zero;

    censor->word = &str_empty[0];
    censor->replace = &str_empty[0];
    return censor;
}

void free_censor( CENSOR_DATA *censor )
{
    free_string( censor->word );
    free_string( censor->replace );

    censor->next = censor_free;
    censor_free = censor;
}

MULTIPLAY_DATA *new_allow( void )
{
    static MULTIPLAY_DATA allow_zero;
    MULTIPLAY_DATA *allow;

    if ( allow_free == NULL )
	allow = alloc_perm( sizeof( *allow ) );
    else
    {
	allow = allow_free;
	allow_free = allow_free->next;
    }

    *allow = allow_zero;

    allow->host = &str_empty[0];
    allow->name = &str_empty[0];

    return allow;
}

void free_allow( MULTIPLAY_DATA *allow )
{
    free_string( allow->name );
    free_string( allow->host );

    allow->next = allow_free;
    allow_free = allow;
}

DESC_CHECK_DATA *new_desc_check(void)
{
    static DESC_CHECK_DATA desc_check_zero;
    DESC_CHECK_DATA *desc_check;

    if ( desc_check_free == NULL )
	desc_check = alloc_perm( sizeof( *desc_check ) );
    else
    {
	desc_check = desc_check_free;
	desc_check_free = desc_check_free->next;
    }

    *desc_check = desc_check_zero;

    desc_check->host[0] = '\0';
    desc_check->name[0] = '\0';

    return desc_check;
}

void free_desc_check( DESC_CHECK_DATA *desc_check )
{
    desc_check->next = desc_check_free;
    desc_check_free = desc_check;
}

WIZ_DATA *new_wiz( void )
{
    static WIZ_DATA wiz_zero;
    WIZ_DATA *wiz;

    if ( wiz_free == NULL )
	wiz = alloc_perm( sizeof( *wiz ) );
    else
    {
	wiz = wiz_free;
	wiz_free = wiz_free->next;
    }

    *wiz	= wiz_zero;
    wiz->name	= &str_empty[0];


    VALIDATE( wiz );

    return wiz;
}

void free_wiz( WIZ_DATA *wiz )
{
    if ( !IS_VALID( wiz ) )
	return;

    free_string( wiz->name );

    wiz->name	= NULL;
    wiz->next	= wiz_free;
    wiz_free	= wiz;

    INVALIDATE( wiz );
}

GEN_DATA *new_gen_data( void )
{
    GEN_DATA *gen;

    if ( gen_data_free == NULL )
	gen = alloc_perm( sizeof( *gen ) );
    else
    {
	gen = gen_data_free;
	gen_data_free = gen_data_free->next;
    }

    gen->group_chosen = new_bool( maxGroup, FALSE );
    gen->skill_chosen = new_bool( maxSkill, FALSE );
    gen->points_chosen = 0;

    gen->next = gen_data_list;
    gen_data_list = gen;

    return gen;
}

void free_gen_data( GEN_DATA *gen )
{
    free_bool( gen->group_chosen );
    free_bool( gen->skill_chosen );

    gen->next = gen_data_free;
    gen_data_free = gen;
}

EXTRA_DESCR_DATA *new_extra_descr( void )
{
    EXTRA_DESCR_DATA *ed;

    if ( extra_descr_free == NULL )
	ed = alloc_perm( sizeof( *ed ) );
    else
    {
	ed = extra_descr_free;
	extra_descr_free = extra_descr_free->next;
    }

    ed->keyword = &str_empty[0];
    ed->description = &str_empty[0];
    return ed;
}

void free_extra_descr( EXTRA_DESCR_DATA *ed )
{
    free_string( ed->keyword );
    free_string( ed->description );

    ed->next = extra_descr_free;
    extra_descr_free = ed;
}

AFFECT_DATA *new_affect( void )
{
    static AFFECT_DATA affect_zero;
    AFFECT_DATA *af;

    if ( affect_free == NULL )
	af = alloc_perm( sizeof( *af ) );
    else
    {
	af = affect_free;
	affect_free = affect_free->next;
    }

    *af = affect_zero;

    af->dur_type = DUR_TICKS;

    return af;
}

void free_affect( AFFECT_DATA *af )
{
    af->next = affect_free;

    affect_free = af;
}

OBJ_DATA *new_obj( void )
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;

    if ( obj_free == NULL )
	obj = alloc_perm( sizeof( *obj ) );
    else
    {
	obj = obj_free;
	obj_free = obj_free->next;
    }

    *obj = obj_zero;
    return obj;
}

void free_obj( OBJ_DATA *obj )
{
    AFFECT_DATA *paf, *paf_next;
    EXTRA_DESCR_DATA *ed, *ed_next;
    OBJ_MULTI_DATA *mult, *mult_next;

    for ( paf = obj->affected; paf != NULL; paf = paf_next )
    {
	paf_next = paf->next;
	free_affect( paf );
    }
    obj->affected = NULL;

    for ( ed = obj->extra_descr; ed != NULL; ed = ed_next )
    {
	ed_next = ed->next;
	free_extra_descr( ed );
    }
    obj->extra_descr = NULL;

    for ( mult = obj->multi_data; mult != NULL; mult = mult_next )
    {
	mult_next = mult->next;
	free_obj_multi( obj, mult );
    }
    obj->multi_data = NULL;

    obj->dropped_by = NULL;

    free_string( obj->name		);
    free_string( obj->description	);
    free_string( obj->short_descr	);
    free_string( obj->owner		);
    free_string( obj->killer		);
    free_string( obj->loader		);

    obj->next   = obj_free;
    obj_free    = obj;
}

GRANT_DATA *new_grant( void )
{
    GRANT_DATA *grant;

    if ( grant_free == NULL )
	grant = alloc_perm( sizeof( *grant ) );
    else
    {
	grant = grant_free;
	grant_free = grant_free->next;
    }

    grant->command	= str_dup( "" );
    grant->granter	= str_dup( "" );

    return grant;
}

void free_grant( GRANT_DATA *grant )
{
    free_string( grant->command );
    free_string( grant->granter );

    grant->command = NULL;
    grant->granter = NULL;

    grant->next	= grant_free;
    grant_free	= grant;
}

PKILL_DATA *new_pkill( void )
{
    PKILL_DATA *pkill;

    if ( pkill_free == NULL )
	pkill		= alloc_perm( sizeof( *pkill ) );
    else
    {
	pkill		= pkill_free;
	pkill_free	= pkill_free->next;
    }

    pkill->next		= NULL;
    pkill->player_name	= NULL;
    pkill->time		= 0;

    return pkill;
}

void free_pkill( PKILL_DATA *pkill )
{
    free_string( pkill->player_name	);

    pkill->player_name	= NULL;
    pkill->time		= 0;

    pkill->next		= pkill_free;
    pkill_free		= pkill;
}

PKILL_RECORD *new_pk_record( void )
{
    PKILL_RECORD *pkill;

    if ( pk_record_free == NULL )
	pkill		= alloc_perm( sizeof(*pkill) );
    else
    {
	pkill		= pk_record_free;
	pk_record_free	= pk_record_free->next;
    }

    pkill->next		= NULL;
    pkill->killer_name	= NULL;
    pkill->victim_name	= NULL;
    pkill->killer_clan	= NULL;
    pkill->victim_clan	= NULL;
    pkill->assist_string= NULL;
    pkill->level[0]	= 0;
    pkill->level[1]	= 0;
    pkill->pkill_points	= 0;
    pkill->bounty	= 0;

    return pkill;
}

void free_pk_record( PKILL_RECORD *pkill )
{
    free_string( pkill->killer_name	);
    free_string( pkill->victim_name	);
    free_string( pkill->killer_clan	);
    free_string( pkill->victim_clan	);
    free_string( pkill->assist_string	);

    pkill->killer_name	= NULL;
    pkill->victim_name	= NULL;
    pkill->killer_clan	= NULL;
    pkill->victim_clan	= NULL;
    pkill->assist_string= NULL;
    pkill->level[0]	= 0;
    pkill->level[1]	= 0;
    pkill->pkill_points	= 0;
    pkill->bounty	= 0;

    pkill->next		= pk_record_free;
    pk_record_free	= pkill;
}

AUCTION_DATA *new_auction( void )
{
    AUCTION_DATA *auction;

    if ( auction_free == NULL )
	auction		= alloc_perm( sizeof( *auction ) );
    else
    {
	auction		= auction_free;
	auction_free	= auction_free->next;
    }

    auction->next		= NULL;
    auction->item		= NULL;
    auction->owner		= NULL;
    auction->high_bidder	= NULL;
    auction->status		= 0;
    auction->slot		= 0;
    auction->bid_type		= VALUE_GOLD;
    auction->bid_amount		= 0;

    return auction;
}

void free_auction( AUCTION_DATA *auction )
{
    if ( auction == auction_list )
	auction_list = auction_list->next;
    else
    {
	AUCTION_DATA *auc;

	for ( auc = auction_list; auc != NULL; auc = auc->next )
	{
	    if ( auc->next == auction )
	    {
		auc->next = auction->next;
		break;
	    }
	}
    }

    auction->next	= auction_free;
    auction_free	= auction;

    if ( auction_list == NULL )
	auction_ticket = 0;
}

OBJ_MULTI_DATA *new_obj_multi( void )
{
    OBJ_MULTI_DATA *obj_multi;

    if ( obj_multi_free == NULL )
	obj_multi	= alloc_perm( sizeof(*obj_multi) );
    else
    {
	obj_multi	= obj_multi_free;
	obj_multi_free	= obj_multi_free->next;
    }

    obj_multi->next		= NULL;
    obj_multi->dropper		= NULL;
    obj_multi->socket		= NULL;
    obj_multi->drop_timer	= 0;

    return obj_multi;
}

void free_obj_multi( OBJ_DATA *obj, OBJ_MULTI_DATA *obj_multi )
{
    if ( obj_multi == obj->multi_data )
	obj->multi_data = obj_multi->next;
    else
    {
	OBJ_MULTI_DATA *mult;

	for ( mult = obj->multi_data; mult != NULL; mult = mult->next )
	{
	    if ( mult->next == obj_multi )
	    {
		mult->next = obj_multi->next;
		break;
	    }
	}
    }

    free_string( obj_multi->socket	);
    free_string( obj_multi->dropper	);

    obj_multi->socket	= NULL;
    obj_multi->dropper	= NULL;

    obj_multi->next	= obj_multi_free;
    obj_multi_free	= obj_multi;
}

long get_pc_id(void)
{
    int val;

    val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
    last_pc_id = val;
    return val;
}

long get_mob_id( void )
{
    last_mob_id++;
    return last_mob_id;
}

MEM_DATA *new_mem_data(void)
{
    MEM_DATA *memory;

    if (mem_data_free == NULL)
	memory = alloc_mem(sizeof(*memory));
    else
    {
	memory = mem_data_free;
	mem_data_free = mem_data_free->next;
    }

    memory->next = NULL;
    memory->id = 0;
    memory->reaction = 0;
    memory->when = 0;

    return memory;
}

void free_mem_data(MEM_DATA *memory)
{
    memory->next = mem_data_free;
    mem_data_free = memory;
}

int get_size (int val)
{
    int i;

    for (i = 0; i < MAX_BUF_LIST; i++)
    {
	if (buf_size[i] >= val)
	{
	    return buf_size[i];
	}
    }
    return -1;
}

BUFFER *new_buf( )
{
    BUFFER *buffer;

    if (buf_free == NULL)
	buffer = alloc_perm(sizeof(*buffer));
    else
    {
	buffer = buf_free;
	buf_free = buf_free->next;
    }

    buffer->next	= NULL;
    buffer->state	= BUFFER_SAFE;
    buffer->size	= get_size(BASE_BUF);
    buffer->length	= 1;

    buffer->string	= alloc_mem(buffer->size);
    buffer->string[0]	= '\0';

    return buffer;
}

void free_buf( BUFFER *buffer )
{
    if ( buffer->string != NULL
    &&   buffer->string != &str_empty[0] )
	free_mem( buffer->string, buffer->size );

    buffer->string = NULL;
    buffer->size   = 0;
    buffer->length = 1;
    buffer->state  = BUFFER_FREED;

    buffer->next  = buf_free;
    buf_free      = buffer;
}

bool add_buf( BUFFER *buffer, char *string )
{
    char *oldstr;
    int oldsize;

    if ( buffer->state == BUFFER_OVERFLOW )
	return FALSE;

    oldstr = buffer->string;
    oldsize = buffer->size;
    buffer->length += actlen_color( string );

    while ( buffer->length >= buffer->size )
    {
	buffer->size = get_size(buffer->size + 1);
	{
	    if ( buffer->size == -1 )
	    {
		buffer->size = oldsize;
		buffer->state = BUFFER_OVERFLOW;
		bug( "buffer overflow past size %d", buffer->size );
		strcat( buffer->string, "{RBuffer terminated, string too long!{x\n\r" );
		return FALSE;
	    }
  	}
    }

    if ( buffer->size != oldsize )
    {
	buffer->string = alloc_mem( buffer->size );

	strcpy( buffer->string, oldstr );
	free_mem( oldstr, oldsize );
    }

    strcat( buffer->string, string );
    return TRUE;
}

PROG_LIST *new_prog(void)
{
    static PROG_LIST mp_zero;
    PROG_LIST *mp;

    if (prog_free == NULL)
	mp = alloc_perm(sizeof(*mp));
    else
    {
	mp = prog_free;
	prog_free=prog_free->next;
    }

    *mp			 = mp_zero;

    mp->vnum             = 0;
    mp->trig_type        = 0;
    mp->trig_phrase	 = str_dup("");
    mp->code             = str_dup("");
    return mp;
}

void free_prog( PROG_LIST *mp )
{
    free_string( mp->code		);
    free_string( mp->trig_phrase	);

    mp->next	= prog_free;
    prog_free	= mp;

    return;
}

HELP_DATA *new_help( void )
{
    HELP_DATA *NewHelp;

    if ( !help_free )
    {
	NewHelp		= alloc_perm( sizeof(*NewHelp) );
    } else {
	NewHelp		= help_free;
	help_free	= help_free->next;
    }

    NewHelp->level	= 0;
    NewHelp->keyword	= str_dup("");
    NewHelp->text	= str_dup("");
    NewHelp->name	= str_dup("");
    NewHelp->next	= NULL;

    top_help++;

    return NewHelp;
}

void free_help( HELP_DATA *pHelp )
{
    free_string( pHelp->name	);
    free_string( pHelp->keyword	);
    free_string( pHelp->text	);

    pHelp->next		= help_free;
    help_free		= pHelp;

    top_help--;

    return;
}

RESET_DATA *new_reset_data( void )
{
    RESET_DATA *pReset;

    if ( !reset_free )
    {
	pReset		= alloc_perm( sizeof(*pReset) );
    } else {
        pReset		= reset_free;
        reset_free	= reset_free->next;
    }

    pReset->next	= NULL;
    pReset->command	= 'X';
    pReset->arg1	= 0;
    pReset->arg2	= 0;
    pReset->arg3	= 0;
    pReset->arg4	= 0;

    top_reset++;

    return pReset;
}

void free_reset_data( RESET_DATA *pReset )
{
    pReset->next	= reset_free;
    reset_free		= pReset;

    top_reset--;

    return;
}

AREA_DATA *new_area( void )
{
    AREA_DATA *pArea;
    char buf[MAX_INPUT_LENGTH];

    if ( !area_free )
    {
	pArea		= alloc_perm( sizeof(*pArea) );
    } else {
	pArea		= area_free;
	area_free	= area_free->next;
    }

    pArea->vnum		= top_area;
    pArea->next		= NULL;
    pArea->name		= str_dup( "New area" );
    pArea->music	= NULL;
    pArea->security	= 1;
    pArea->min_vnum	= 0;
    pArea->max_vnum	= 0;
    pArea->age		= 0;
    pArea->nplayer	= 0;
    pArea->alignment	= '?';

    sprintf( buf, "area%d.are", pArea->vnum );
    pArea->file_name	= str_dup( buf );

    top_area++;

    return pArea;
}

void free_area( AREA_DATA *pArea )
{
    free_string( pArea->name		);
    free_string( pArea->file_name	);

    pArea->name		= NULL;
    pArea->file_name	= NULL;
    pArea->next         = area_free;
    area_free           = pArea;

    top_area--;

    return;
}

EXIT_DATA *new_exit( void )
{
    EXIT_DATA *pExit;

    if ( !exit_free )
    {
	pExit		= alloc_perm( sizeof(*pExit) );
	top_exit++;

    } else {
	pExit		= exit_free;
	exit_free	= exit_free->next;
    }

    pExit->u1.to_room	= NULL;
    pExit->next		= NULL;
    pExit->exit_info	= 0;
    pExit->key		= 0;
    pExit->keyword	= &str_empty[0];
    pExit->description	= &str_empty[0];
    pExit->rs_flags	= 0;

    return pExit;
}

void free_exit( EXIT_DATA *pExit )
{
    free_string( pExit->keyword		);
    free_string( pExit->description	);

    pExit->next		= exit_free;
    exit_free		= pExit;

    return;
}

ROOM_DAMAGE_DATA *new_room_damage( int type )
{
    ROOM_DAMAGE_DATA *pRoom;

    if ( !room_damage_free )
	pRoom			= alloc_perm( sizeof(*pRoom) );
    else
    {
	pRoom			= room_damage_free;
	room_damage_free	= room_damage_free->next;
    }

    pRoom->next			= NULL;
    pRoom->msg_room		= str_dup( "" );
    pRoom->msg_victim		= str_dup( "" );
    pRoom->damage_min		= 0;
    pRoom->damage_max		= 0;
    pRoom->damage_type		= type;
    pRoom->success		= 100;

    return pRoom;
}

void free_room_damage( ROOM_DAMAGE_DATA *pRoom )
{
    free_string( pRoom->msg_room	);
    free_string( pRoom->msg_victim	);
    pRoom->msg_room	= NULL;
    pRoom->msg_victim	= NULL;
    pRoom->next		= room_damage_free;
    room_damage_free	= pRoom;
}

ROOM_INDEX_DATA *new_room_index( void )
{
    ROOM_INDEX_DATA *pRoom;
    int door;

    top_room++;
    if ( !room_index_free )
	pRoom			= alloc_perm( sizeof(*pRoom) );
    else
    {
	pRoom			= room_index_free;
	room_index_free		= room_index_free->next;
    }

    pRoom->room_damage		= NULL;
    pRoom->next			= NULL;
    pRoom->people		= NULL;
    pRoom->contents		= NULL;
    pRoom->extra_descr		= NULL;
    pRoom->area			= NULL;
    pRoom->music		= NULL;

    for ( door = 0; door < MAX_DIR; door++ )
        pRoom->exit[door]	= NULL;

    pRoom->name			= &str_empty[0];
    pRoom->description		= &str_empty[0];
    pRoom->vnum			= 0;
    pRoom->room_flags		= 0;
    pRoom->light		= 0;
    pRoom->sector_type		= 0;
    pRoom->heal_rate		= 100;
    pRoom->mana_rate		= 100;

    return pRoom;
}

void free_room_index( ROOM_INDEX_DATA *pRoom )
{
    EXTRA_DESCR_DATA *pExtra, *pExtra_next;
    PROG_LIST *pCode, *pCode_next;
    RESET_DATA *pReset, *pReset_next;
    ROOM_DAMAGE_DATA *dam, *dam_next;
    int door;

    free_string( pRoom->name		);
    free_string( pRoom->description	);

    for ( door = 0; door < MAX_DIR; door++ )
    {
	if ( pRoom->exit[door] )
	    free_exit( pRoom->exit[door] );
    }

    for ( pExtra = pRoom->extra_descr; pExtra; pExtra = pExtra_next )
    {
	pExtra_next = pExtra->next;
	free_extra_descr( pExtra );
    }
    pRoom->extra_descr = NULL;

    for ( pReset = pRoom->reset_first; pReset; pReset = pReset_next )
    {
	pReset_next = pReset->next;
	free_reset_data( pReset );
    }
    pRoom->reset_first = NULL;

    for ( dam = pRoom->room_damage; dam != NULL; dam = dam_next )
    {
	dam_next = dam->next;
	free_room_damage( dam );
    }
    pRoom->room_damage	= NULL;

    for ( pCode = pRoom->rprogs; pCode != NULL; pCode = pCode_next )
    {
	pCode_next = pCode->next;

	free_prog( pCode );
    }
    pRoom->rprogs = NULL;

    pRoom->next		= room_index_free;
    room_index_free	= pRoom;
    top_room--;

    return;
}

SHOP_DATA *new_shop( void )
{
    SHOP_DATA *pShop;
    int buy;

    if ( !shop_free )
    {
	pShop           =   alloc_perm( sizeof(*pShop) );
    } else {
        pShop           =   shop_free;
        shop_free       =   shop_free->next;
    }

    pShop->next         =   NULL;
    pShop->keeper       =   0;

    for ( buy=0; buy<MAX_TRADE; buy++ )
        pShop->buy_type[buy]    =   0;

    pShop->profit_buy   =   100;
    pShop->profit_sell  =   100;
    pShop->open_hour    =   0;
    pShop->close_hour   =   23;

    top_shop++;

    return pShop;
}

void free_shop( SHOP_DATA *pShop )
{
    pShop->next = shop_free;
    shop_free   = pShop;
    top_shop--;
    return;
}

OBJ_INDEX_DATA *new_obj_index( void )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    top_obj_index++;
    if ( !obj_index_free )
	pObj		= alloc_perm( sizeof( *pObj ) );
    else
    {
	pObj		= obj_index_free;
	obj_index_free	= obj_index_free->next;
    }

    pObj->next		= NULL;
    pObj->extra_descr	= NULL;
    pObj->affected	= NULL;
    pObj->area		= NULL;
    pObj->name		= NULL;
    pObj->short_descr	= NULL;
    pObj->description	= NULL;
    pObj->vnum		= 0;
    pObj->item_type	= ITEM_TRASH;
    pObj->extra_flags	= 0;
    pObj->wear_flags	= 0;
    pObj->count		= 0;
    pObj->weight	= 0;
    pObj->cost		= 0;
    pObj->size		= 6;
    pObj->class_can_use	= new_bool( maxClass, TRUE );
    pObj->success	= 100;

    for ( value = 0; value < 5; value++ )
	pObj->value[value] = 0;

    return pObj;
}

void free_obj_index( OBJ_INDEX_DATA *pObj )
{
    EXTRA_DESCR_DATA *pExtra, *pExtra_next;
    PROG_LIST *pCode, *pCode_next;
    AFFECT_DATA *pAf, *pAf_next;

    free_string( pObj->name		);
    free_string( pObj->short_descr	);
    free_string( pObj->description	);

    for ( pAf = pObj->affected; pAf; pAf = pAf_next )
    {
	pAf_next = pAf->next;
        free_affect( pAf );
    }
    pObj->affected = NULL;

    for ( pExtra = pObj->extra_descr; pExtra; pExtra = pExtra_next )
    {
	pExtra_next = pExtra->next;
        free_extra_descr( pExtra );
    }
    pObj->extra_descr = NULL;

    for ( pCode = pObj->oprogs; pCode != NULL; pCode = pCode_next )
    {
	pCode_next = pCode->next;

	free_prog( pCode );
    }
    pObj->oprogs = NULL;

    pObj->next              = obj_index_free;
    obj_index_free          = pObj;
    top_obj_index--;
    return;
}

MOB_INDEX_DATA *new_mob_index( void )
{
    MOB_INDEX_DATA *pMob;
    sh_int pos;

    top_mob_index++;
    if ( !mob_index_free )
	pMob		= alloc_perm( sizeof( *pMob ) );
    else
    {
	pMob		= mob_index_free;
	mob_index_free	= mob_index_free->next;
    }

    pMob->next		= NULL;
    pMob->spec_fun	= NULL;
    pMob->pShop		= NULL;
    pMob->area		= NULL;
    pMob->player_name	= NULL;
    pMob->short_descr	= NULL;
    pMob->long_descr	= NULL;
    pMob->description	= NULL;
    pMob->die_descr	= &str_empty[0];
    pMob->say_descr	= &str_empty[0];
    pMob->vnum		= 0;
    pMob->count		= 0;
    pMob->sex		= 0;
    pMob->level		= 0;
    pMob->affected_by	= 0;
    pMob->alignment	= 0;
    pMob->hitroll	= 0;
    pMob->race		= race_lookup( "human" );
    pMob->parts		= 0;
    pMob->size		= SIZE_MEDIUM;
    pMob->ac[AC_PIERCE]	= 0;
    pMob->ac[AC_BASH]	= 0;
    pMob->ac[AC_SLASH]	= 0;
    pMob->ac[AC_EXOTIC]	= 0;
    pMob->hit[0]	= 0;
    pMob->hit[1]	= 0;
    pMob->mana[0]	= 0;
    pMob->mana[1]	= 0;
    pMob->start_pos	= POS_STANDING;
    pMob->default_pos	= POS_STANDING;
    pMob->wealth	= 0;
    pMob->exp_percent	= 100;
    pMob->learned	= new_short( maxSkill, 0 );
    pMob->damage[DICE_NUMBER]	= 0;
    pMob->damage[DICE_TYPE]	= 0;
    pMob->damage[DICE_NUMBER]	= 0;

    for ( pos = 0; pos < DAM_MAX; pos++ )
	pMob->damage_mod[pos] = 100;

    return pMob;
}

void free_mob_index( MOB_INDEX_DATA *pMob )
{
    PROG_LIST *pCode, *pCode_next;

    free_string( pMob->player_name );
    free_string( pMob->short_descr );
    free_string( pMob->long_descr );
    free_string( pMob->description );
    free_string( pMob->die_descr );
    free_string( pMob->say_descr );

    for ( pCode = pMob->mprogs; pCode != NULL; pCode = pCode_next )
    {
	pCode_next = pCode->next;

	free_prog( pCode );
    }
    pMob->mprogs = NULL;

    if ( pMob->pShop )
	free_shop( pMob->pShop );

    free_short( pMob->learned );

    pMob->next              = mob_index_free;
    mob_index_free          = pMob;
    top_mob_index--;
    return;
}

PROG_CODE *new_pcode(void)
{
     PROG_CODE *NewCode;

     if (!pcode_free)
     {
         NewCode = alloc_perm(sizeof(*NewCode) );
     }
     else
     {
         NewCode     = pcode_free;
         pcode_free = pcode_free->next;
     }

     NewCode->vnum    = 0;
     NewCode->code    = str_dup("");
     NewCode->name    = str_dup("");
     NewCode->author  = str_dup("");
     NewCode->next    = NULL;

     return NewCode;
}

void free_pcode(PROG_CODE *pMcode)
{
    free_string(pMcode->code);
    free_string(pMcode->name);
    free_string(pMcode->author);
    pMcode->next = pcode_free;
    pcode_free  = pMcode;
    return;
}

POLL_DATA *new_poll( void )
{
    POLL_DATA *poll;
    sh_int pos;

    if ( poll_free == NULL )
	poll = alloc_perm( sizeof(*poll) );
    else
    {
	poll		= poll_free;
	poll_free	= poll_free->next;
    }

    poll->next		= NULL;
    poll->vote		= NULL;
    poll->name		= NULL;
    poll->question	= NULL;

    for ( pos = 0; pos < MAX_POLL_RESPONSES; pos++ )
	poll->response[pos] = NULL;

    return poll;
}

void free_poll( POLL_DATA *poll )
{
    VOTE_DATA *vote, *vote_next;
    sh_int pos;

    free_string( poll->name	);
    free_string( poll->question	);

    for ( pos = 0; pos < MAX_POLL_RESPONSES; pos++ )
	free_string( poll->response[pos] );

    for ( vote = poll->vote; vote != NULL; vote = vote_next )
    {
	vote_next = vote->next_vote;

	free_vote( vote );
    }

    poll->next	= poll_free;
    poll_free	= poll;
}

VOTE_DATA *new_vote( void )
{
    VOTE_DATA *vote;

    if ( vote_free == NULL )
	vote = alloc_perm( sizeof(*vote) );
    else
    {
	vote		= vote_free;
	vote_free	= vote_free->next_vote;
    }

    vote->next_vote	= NULL;
    vote->voter_name	= NULL;
    vote->voter_ip	= NULL;
    vote->voter_vote	= 0;

    return vote;
}

void free_vote( VOTE_DATA *vote )
{
    free_string( vote->voter_name	);
    free_string( vote->voter_ip		);

    vote->next_vote	= vote_free;
    vote_free		= vote;
}

DESCRIPTOR_DATA *new_descriptor(void)
{
    static DESCRIPTOR_DATA d_zero;
    DESCRIPTOR_DATA *d;

    if (descriptor_free == NULL)
	d = alloc_perm(sizeof(*d));
    else
    {
	d = descriptor_free;
	descriptor_free = descriptor_free->next;
    }
	
    *d = d_zero;
    VALIDATE(d);

    d->connected	= CON_GET_NAME;
    d->showstr_head	= NULL;
    d->showstr_point	= NULL;
    d->outsize		= 2000;
    d->outbuf		= alloc_mem( d->outsize );
    d->a_cur[0]		= '\0';

    return d;
}

void free_descriptor(DESCRIPTOR_DATA *d)
{
    if (!IS_VALID(d))
	return;

    free_string( d->host );
    free_string( d->hostip );
    free_string( d->run_buf );
    free_string( d->run_head );
    free_string( d->showstr_head );
    free_mem( d->outbuf, d->outsize );

    INVALIDATE(d);
    d->next = descriptor_free;
    descriptor_free = d;
}

CHAR_DATA *new_char (void)
{
    static CHAR_DATA ch_zero;
    CHAR_DATA *ch;
    int i;

    if (char_free == NULL)
	ch = alloc_perm(sizeof(*ch));
    else
    {
	ch = char_free;
	char_free = char_free->next;
    }

    *ch				= ch_zero;
    VALIDATE(ch);
    ch->name                    = &str_empty[0];
    ch->short_descr             = &str_empty[0];
    ch->long_descr              = &str_empty[0];
    ch->description             = &str_empty[0];
    ch->prompt                  = &str_empty[0];

    for (i = 0; i < 4; i++)
        ch->armor[i]            = 100;

    ch->position                = POS_STANDING;
    ch->hit                     = 100;
    ch->max_hit                 = 100;
    ch->mana                    = 100;
    ch->max_mana                = 100;
    ch->move                    = 100;
    ch->max_move                = 100;
    ch->learned			= new_short( maxSkill, 0 );

    for (i = 0; i < MAX_STATS; i ++)
    {
        ch->perm_stat[i] = 13;
        ch->mod_stat[i] = 0;
    }

    return ch;
}

void free_char (CHAR_DATA *ch)
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    if (!IS_VALID(ch))
	return;

    for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
	obj_next = obj->next_content;
	extract_obj(obj);
    }

    for (paf = ch->affected; paf != NULL; paf = paf_next)
    {
	paf_next = paf->next;
	affect_remove(ch,paf);
    }

    free_string(ch->name);
    free_string(ch->short_descr);
    free_string(ch->long_descr);
    free_string(ch->description);
    free_string(ch->prompt);

    if (ch->pcdata != NULL)
    	free_pcdata(ch->pcdata);

    free_short( ch->learned );

    ch->next = char_free;
    char_free  = ch;

    INVALIDATE(ch);
    return;
}

PC_DATA *new_pcdata( void )
{
    int alias;

    static PC_DATA pcdata_zero;
    PC_DATA *pcdata;

    if ( pcdata_free == NULL )
    {
	pcdata = alloc_perm( sizeof( *pcdata ) );
    } else {
	pcdata = pcdata_free;
	pcdata_free = pcdata_free->next;
    }

    *pcdata = pcdata_zero;

    for (alias = 0; alias < MAX_ALIAS; alias++)
    {
	pcdata->alias[alias] = NULL;
	pcdata->alias_sub[alias] = NULL;
    }

    for (alias = 0; alias < MAX_FORGET; alias++)
	pcdata->forget[alias] = NULL;

    pcdata->grants			= NULL;
    pcdata->confirm_delete		= FALSE;
    pcdata->buffer			= new_buf( );
    pcdata->pretitle			= str_dup("");
    pcdata->title			= str_dup("");
    pcdata->pwd		 		= str_dup("");
    pcdata->bamfin			= str_dup("");
    pcdata->bamfout			= str_dup("");
    pcdata->identity			= str_dup("");
    pcdata->who_descr			= str_dup("");
    pcdata->who_output			= str_dup("");
    pcdata->prefix			= &str_empty[0];
    pcdata->logon			= current_time;
    pcdata->lines			= PAGELEN;
    pcdata->bounty			= 0;
    pcdata->tier			= 1;
    pcdata->pkpoints			= 100;
    pcdata->condition[COND_THIRST]	= 48;
    pcdata->condition[COND_FULL]	= 48;
    pcdata->condition[COND_HUNGER]	= 48;
    pcdata->security			= 0;
    pcdata->group_known			= new_bool( maxGroup, FALSE );

    VALIDATE( pcdata );

    pcdata->next = pcdata_list;
    pcdata_list = pcdata;

    return pcdata;
}

void free_pcdata( PC_DATA *pcdata )
{
    OBJ_DATA *obj, *obj_next;
    GRANT_DATA *grant, *grant_next;
    int alias;

    if ( !IS_VALID( pcdata ) )
	return;

    free_string( pcdata->pwd		);
    free_string( pcdata->bamfin		);
    free_string( pcdata->bamfout	);
    free_string( pcdata->who_descr	);
    free_string( pcdata->title		);
    free_string( pcdata->identity	);
    free_string( pcdata->pretitle	);
    free_string( pcdata->socket		);
    free_string( pcdata->who_output	);
    free_string( pcdata->prefix		);

    free_buf( pcdata->buffer		);
    
    for ( grant = pcdata->grants; grant != NULL; grant = grant_next )
    {
	grant_next = grant->next;
	free_grant( grant );
    }
    pcdata->grants = NULL;

    for ( alias = 0; alias < MAX_ALIAS; alias++ )
    {
	free_string( pcdata->alias[alias]	);
	free_string( pcdata->alias_sub[alias]	);
    }

    for ( alias = 0; alias < MAX_FORGET; alias++ )
	free_string( pcdata->forget[alias]	);

    for ( alias = 0; alias < MAX_BANK; alias++ )
    {
	for ( obj = pcdata->storage_list[alias]; obj != NULL; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    extract_obj( obj );
	}
    }

    if ( pcdata->pnote != NULL ) 
    {
        free_string( pcdata->pnote->text	);
        free_string( pcdata->pnote->subject	);
        free_string( pcdata->pnote->to_list	);
        free_string( pcdata->pnote->date	);
        free_string( pcdata->pnote->sender	);
        pcdata->pnote->text = NULL;
        pcdata->pnote->next = note_free;
        note_free           = pcdata->pnote;
        pcdata->pnote       = NULL;
    }

    if ( pcdata->recent_pkills != NULL )
    {
	PKILL_DATA *pkill, *pkill_next;

	for ( pkill = pcdata->recent_pkills; pkill != NULL; pkill = pkill_next )
	{
	    pkill_next = pkill->next;

	    free_pkill( pkill );
	}
    }

    if ( pcdata->kills_list != NULL )
    {
	PKILL_RECORD *pkill, *pkill_next;

	for ( pkill = pcdata->kills_list; pkill != NULL; pkill = pkill_next )
	{
	    pkill_next = pkill->next;

	    free_pk_record( pkill );
	}
    }

    if ( pcdata->death_list != NULL )
    {
	PKILL_RECORD *pkill, *pkill_next;

	for ( pkill = pcdata->death_list; pkill != NULL; pkill = pkill_next )
	{
	    pkill_next = pkill->next;

	    free_pk_record( pkill );
	}
    }

    free_bool( pcdata->group_known );

    INVALIDATE(pcdata);

    if ( pcdata_list == pcdata )
	pcdata_list = pcdata->next;
    else
    {
	PC_DATA *pcd;

	for ( pcd = pcdata_list; pcd != NULL; pcd = pcd->next )
	{
	    if ( pcd->next == pcdata )
	    {
		pcd->next = pcdata->next;
		break;
	    }
	}
    }
	
    pcdata->next = pcdata_free;
    pcdata_free = pcdata;

    return;
}

void update_group_data( int pos )
{
    PC_DATA *pcd;
    GEN_DATA *gen;

    for ( pcd = pcdata_list; pcd != NULL; pcd = pcd->next )
	pcd->group_known = redo_bool( pcd->group_known, maxGroup, pos, FALSE );

    for ( gen = gen_data_list; gen != NULL; gen = gen->next )
	gen->group_chosen = redo_bool( gen->group_chosen, maxGroup, pos, FALSE );
}

void update_skill_data( int pos )
{
    ARENA_DATA *match;
    CHAR_DATA *wch;
    DESCRIPTOR_DATA *d;
    GEN_DATA *gen;
    MOB_INDEX_DATA *pMob;
    int count, vnum;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->connected == CON_PLAYING || d->character == NULL )
	    continue;

	d->character->learned = redo_short( d->character->learned, maxSkill, pos, 0 );
    }

    for ( wch = char_list; wch != NULL; wch = wch->next )
	wch->learned = redo_short( wch->learned, maxSkill, pos, 0 );

    for ( match = arena_matches; match != NULL; match = match->next )
	match->disabled_skills = redo_bool( match->disabled_skills, maxSkill, pos, FALSE );

    for ( vnum = 0, count = 0; count < top_mob_index; vnum++ )
    {
	if ( ( pMob = get_mob_index( vnum ) ) != NULL )
	{
	    count++;
	    pMob->learned = redo_short( pMob->learned, maxSkill, pos, 0 );
	}
    }

    for ( gen = gen_data_list; gen != NULL; gen = gen->next )
	gen->skill_chosen = redo_bool( gen->skill_chosen, maxSkill, pos, FALSE );
}

void update_class_data( int pos )
{
    OBJ_INDEX_DATA *pObj;
    sh_int sn;
    int count, vnum;

    for ( sn = 0; sn < maxSkill; sn++ )
    {
	skill_table[sn].skill_level = redo_short( skill_table[sn].skill_level, maxClass, pos, LEVEL_IMMORTAL );
	skill_table[sn].rating = redo_short( skill_table[sn].rating, maxClass, pos, 0 );
    }

    for ( sn = 0; sn < maxGroup; sn++ )
	group_table[sn].rating = redo_short( group_table[sn].rating, maxClass, pos, -1 );

    for ( sn = 0; sn < maxRace; sn++ )
    {
	race_table[sn].class_mult = redo_short( race_table[sn].class_mult, maxClass, pos, 200 );
	race_table[sn].class_can_use = redo_bool( race_table[sn].class_can_use, maxClass, pos, TRUE );
    }

    for ( vnum = 0, count = 0; count < top_obj_index; vnum++ )
    {
	if ( ( pObj = get_obj_index( vnum ) ) != NULL )
	{
	    count++;
	    pObj->class_can_use = redo_bool( pObj->class_can_use, maxClass, pos, TRUE );
	}
    }

    return;
}
