/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku vMud improvments copyright (C) 1992, 1993 by Michael         *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
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

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include "merc.h"
#include "recycle.h"
#include "interp.h"

#if defined(unix)
#include <signal.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#define	TELOPT_COMPRESS2	86
#define SE			240
#define GA			249
#define SB			250
#define WILL			251
#define DO			253
#define DONT			254
#define IAC			255

	char	compress_start	[] = { IAC, SB, TELOPT_COMPRESS2, IAC, SE, '\0' };
const	char	compress_will	[] = { IAC, WILL, TELOPT_COMPRESS2, '\0' };
const	char	compress_do	[] = { IAC, DO, TELOPT_COMPRESS2, '\0' };
const	char	compress_dont	[] = { IAC, DONT, TELOPT_COMPRESS2, '\0' };
//const	char	go_ahead	[] = { IAC, GA, '\0' };

#if	defined(linux)
int	close		args( ( int fd ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#endif

DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
MULTIPLAY_DATA *    multiplay_list;	/* Host IPs that can Multiplay	*/
DESC_CHECK_DATA *   desc_check_list;	/* Recent connections		*/
bool		    merc_down;		/* Shutdown			*/
bool		    perm_affect;
char		    str_boot_time[128];
time_t		    current_time;	/* time of this pulse */	
char		    clcode[ MAX_INPUT_LENGTH ];
bool		    MOBtrigger = TRUE;  /* act() switch                 */
long                nAllocString;
long                sAllocString;
long                nAllocPerm;
long                sAllocPerm;
int		    port;
int		    control;
sh_int		    auction_ticket;
sh_int		    arena_match_count;
volatile 	    sig_atomic_t crashed = 0;
sh_int		    global_newbie;
sh_int		    global_quest;

#if defined(unix)
void	game_loop_unix		args( ( int control ) );
int	init_socket		args( ( int port ) );
void	init_descriptor		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( DESCRIPTOR_DATA *d, char *txt, int length ) );
#endif

#if !defined ( SA_NOMASK )
    #define SA_NOMASK 0
#endif

void	fread_char		args( ( CHAR_DATA *ch, FILE *fp, sh_int type ) );
int	channel_lookup		args( ( char *argument ) );
void	object_balance		args( ( OBJ_INDEX_DATA *pObj, sh_int level ) );
char	*get_random_editing	args( ( DESCRIPTOR_DATA *d ) );
void	update_chart_ranks	args( ( CHAR_DATA *ch ) );

/* Try to send any pending compressed-but-not-sent data in `desc' */
bool processCompressed( DESCRIPTOR_DATA *desc )
{
    int iStart, nBlock, nWrite, len;

    if ( !desc->out_compress )
        return TRUE;
    
    /* Try to write out some data.. */
    len = desc->out_compress->next_out - desc->out_compress_buf;
    if ( len > 0 )
    {
	struct timeval tv;

	fd_set write;
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	/* we have some data to write */
	for ( iStart = 0; iStart < len; iStart += nWrite )
        {
	    FD_ZERO( &write );
	    FD_SET( desc->descriptor, &write);

	    if ( select( desc->descriptor+1, NULL, &write, NULL, &tv ) < 0 )
		iStart = len+1;

	    nBlock = UMIN (len - iStart, 4096);
	    if ((nWrite = send (desc->descriptor, desc->out_compress_buf + iStart, nBlock, 0)) < 0)
            {
                if (errno == EAGAIN || errno == ENOSR)
                    break;

                return FALSE; /* write error */
            }

            if (nWrite <= 0)
                break;
        }

        if (iStart) {
//            We wrote "iStart" bytes
            if (iStart < len)
                memmove(desc->out_compress_buf, desc->out_compress_buf+iStart, len - iStart);

            desc->out_compress->next_out = desc->out_compress_buf + len - iStart;
        }
    }

    return TRUE;
}

/* write_to_descriptor, the compressed case */
bool writeCompressed(DESCRIPTOR_DATA *desc, char *txt, int length)
{
    z_stream *s = desc->out_compress;
    
    s->next_in = (unsigned char *)txt;
    s->avail_in = length;

    while (s->avail_in) {
        s->avail_out = COMPRESS_BUF_SIZE - (s->next_out - desc->out_compress_buf);
            
        if (s->avail_out) {
            int status = deflate(s, Z_SYNC_FLUSH);

            if (status != Z_OK) {
                /* Boom */
                return FALSE;
            }
        }

        /* Try to write out some data.. */
        if (!processCompressed(desc))
            return FALSE;

        /* loop */
    }
    
    /* Done. */
    return TRUE;
}

void *zlib_alloc( void *opaque, unsigned int items, unsigned int size )
{
    return calloc( items, size );
}

void zlib_free( void *opaque, void *address )
{
    free( address );
}

/*
 * Begin compressing data on `desc'
 */
bool compressStart( DESCRIPTOR_DATA *desc )
{
    z_stream *s;
    
    if ( desc->out_compress ) /* already compressing */
        return TRUE;

    /* allocate and init stream, buffer */
    s = ( z_stream * ) alloc_mem(sizeof( *s ) );
    desc->out_compress_buf = ( unsigned char * )alloc_mem( COMPRESS_BUF_SIZE );
    
    if ( !s )
    {
	log_string( "PANIC!  Couldn't find an s to which to compress!" );
	close_socket( desc );
	return FALSE;
    }

    s->next_in = NULL;
    s->avail_in = 0;

    s->next_out = desc->out_compress_buf;
    s->avail_out = COMPRESS_BUF_SIZE;

    s->zalloc = zlib_alloc;
    s->zfree  = zlib_free;
    s->opaque = NULL;

    if ( deflateInit( s, 9 ) != Z_OK )
    {
	/* problems with zlib, try to clean up */
	free_mem( desc->out_compress_buf, COMPRESS_BUF_SIZE );
	free_mem( s, sizeof( z_stream ) );
	return FALSE;
    }

    write_to_descriptor( desc, compress_start, strlen( compress_start ) );

    /* now we're compressing */
    desc->out_compress = s;
    return TRUE;
}

/* Cleanly shut down compression on `desc' */
bool compressEnd( DESCRIPTOR_DATA *desc )
{
    unsigned char dummy[1];
    
    if ( !desc->out_compress )
        return TRUE;

    desc->out_compress->avail_in = 0;
    desc->out_compress->next_in = dummy;

    /* No terminating signature is needed - receiver will get Z_STREAM_END */
    
    if ( deflate( desc->out_compress, Z_FINISH ) != Z_STREAM_END )
        return FALSE;

    if ( !processCompressed( desc ) ) /* try to send any residual data */
        return FALSE;

    deflateEnd( desc->out_compress );
    free_mem( desc->out_compress_buf, COMPRESS_BUF_SIZE );
    free_mem( desc->out_compress, sizeof( z_stream ) );
    desc->out_compress = NULL;
    desc->out_compress_buf = NULL;

    return TRUE;
}

/* User-level compression toggle */
void do_compress( CHAR_DATA *ch, char *argument )
{
    if ( !ch->desc )
    {
	send_to_char( "What descriptor?!\n", ch );
	return;
    }

    if ( !ch->desc->out_compress )
    {
	if ( !compressStart( ch->desc ) )
	{
	    send_to_char( "Failed.\n", ch );
	    return;
        }
	send_to_char("Ok, compression enabled.\n", ch );
    } else {
	if ( !compressEnd( ch->desc ) )
	{
	    send_to_char( "Failed.\n", ch );
	    return;
	}
	send_to_char( "Ok, compression disabled.\n", ch );
    }
}

void prepare_reboot( void )
{
    AUCTION_DATA *auc;
    OBJ_DATA *obj;

    arena_clear( NULL );
    do_asave( NULL, "changed" );
    save_special_items( );

    for ( auc = auction_list; auc != NULL; auc = auc->next )
    {
	if ( auc->item != NULL )
	    obj_to_char( auc->item, auc->owner );

	if ( auc->high_bidder != NULL )
	    add_cost( auc->high_bidder, auc->bid_type, auc->bid_amount );
    }

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
	if ( obj->in_room != NULL && obj->disarmed_from != NULL )
	{
	    act( "$p returns to you.",
		obj->disarmed_from, obj, NULL, TO_CHAR, POS_DEAD );
	    obj_to_char( obj, obj->disarmed_from );
	}
    }
}

bool write_to_desc( const char *txt, DESCRIPTOR_DATA *desc )
{
    const       char    *point;
                char    *point2;
                char    buf[ 262144 ];

    buf[0] = '\0';
    point2 = buf;
    if( txt && desc && desc->descriptor )
    {
	for( point = txt ; *point ; point++ )
	{
	    if( *point == '{' )
	    {
		point++;
		strcat( buf, colour2( *point ) );
		for( point2 = buf ; *point2 ; point2++ )
		    ;
		continue;
	    }
	    *point2 = *point;
	    *++point2 = '\0';
	}
	*point2 = '\0';
	write_to_descriptor( desc, buf, point2 - buf );
    } else {
	for( point = txt ; *point ; point++ )
	{
	    if( *point == '{' )
	    {
		point++;
		if( *point != '{' )
		    continue;
	    }
	    *point2 = *point;
	    *++point2 = '\0';
	}
	*point2 = '\0';
	write_to_descriptor( desc, buf, point2 - buf );
    }

    return TRUE;
}

void crash_recover( )
{
    FILE *fp;
    DESCRIPTOR_DATA *d, *d_next;
    char buf [100], buf2[100], name[MAX_INPUT_LENGTH];
    extern bool fBootDb;
    extern int port, control;

    if ( fBootDb )
    {
	log_string( "fBootDb detected: copyover attempt aborted." );
	return;
    }

    prepare_reboot( );

    if ( ( fp = fopen( LAST_COMMAND, "a" ) ) == NULL )
	bug( "Error in do_auto_save opening last_command.txt", 0 );
    else
    {
	fprintf( fp, "Last Command: %s\n", last_command );
	fclose( fp );
    }

    if ( ( fp = fopen( COPYOVER_FILE, "w" ) ) == NULL )
    {
	perror ( "crash_recover:fopen" );
	return;
    }

    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	CHAR_DATA * och = d->original ? d->original : d->character;
	d_next = d->next;

	if ( !d->character || d->connected > CON_PLAYING )
	{
	    sprintf( name, "\n\r%s is currently experiencing technically difficulties.\n\rPlease try back soon.\n\r", mud_stat.mud_name_string );
	    send_to_desc( name, d );
	    close_socket ( d );
	} else {
	    fprintf( fp, "%d %d %s %s %s\n", d->descriptor, d->out_compress ? 1 : 0, och->name, d->host, d->hostip );
	    save_char_obj( och, 0 );
	    write_to_desc( "\n\r{g*{G*{g* {RTECHNICAL DIFFICULTIES {g-{G-{g- {RATTEMPTING CRASH EVASION {g*{G*{g*\n\r", d );
	}

	compressEnd( d );
    }

    fprintf( fp, "-1\n" );
    fclose( fp );

    sprintf( buf, "%d", port );
    sprintf( buf2, "%d", control );

    if ( port == MAIN_GAME_PORT )
        {
        write_to_desc ("{!COMM.C WAYYNE", d);
	execl( EXE_FILE, "rot", buf, buf2, (char *) NULL );
        }
    else
	{
	write_to_desc ("{!COMM.C WAYYNE", d);
	execl( "../src/rot", "../src/rot", buf, buf2, (char *) NULL );
	}
}

void halt_mud( int sig )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;
    struct sigaction default_action;
    int i;
    pid_t forkpid;

    if ( sig == SIGTERM || sig == SIGKILL )
    {
	sprintf( log_buf, "\n\r\n\r{G**** {RSHUTDOWN BY SERVER HOST! {G****{x\n\r\n\r" );
	log_string( log_buf );

	prepare_reboot( );

	for ( ch = player_list; ch != NULL; ch = ch->pcdata->next_player )
	{
	    save_char_obj( ch, 0 );
	    write_to_desc( log_buf, ch->desc );
	}

	exit( 0 );
	return;
    }

    if( !crashed )
    {
	crashed++;
	sprintf( log_buf, "GAME CRASHED (SIGNAL %d).\nLast command: %s\n",
	    sig, last_command );
	log_string( log_buf );

	if ( ( forkpid = fork( ) ) > 0 )
	{
	    wait( NULL );
	    waitpid( forkpid, NULL, WNOHANG|WUNTRACED );
	    crash_recover( );
	    exit( 0 );
	}

	else if ( forkpid < 0 )
	    exit( 1 );

	for ( i = 255; i >= 0; i-- )
	    close( i );

	default_action.sa_handler = SIG_DFL;
	sigaction( sig,  &default_action,   NULL );

  	if( !fork( ) )
	    exit( 0 );
 	else
	    return;

 	raise( sig );
    }

    if ( crashed == 1 )
    {
	crashed++;

	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;
	    ch = d->original ? d->original : d->character;

	    if ( ch == NULL )
	    {
		close_socket( d );
		continue;
	    }

	    if ( IS_NPC( ch ) )
	    {
		close_socket( d );
		continue;
	    }

	    write_to_descriptor( d, "** Error saving character files; conducting full reboot. **\007\n\r", 0 );
	    close_socket( d );
	    continue;
	}

	sprintf( log_buf, "CHARACTERS NOT SAVED.\r" );
	log_string( log_buf );

	default_action.sa_handler = SIG_DFL;
	sigaction( sig,  &default_action,   NULL );

	if( !fork( ) )
	{
	    kill( getppid( ), sig );
	    exit( 1 );
 	}
	else
	    return;
 	raise( sig );
    }

    if ( crashed == 2 )
    {
	crashed++;
	log_string( "TOTAL GAME CRASH." );
	default_action.sa_handler = SIG_DFL;
	sigaction( sig,  &default_action,   NULL );

	if( !fork( ) )
	{
	    kill( getppid( ), sig );
	    exit( 1 );
	}
	else
	    return;
 	raise( sig );
    }

    if ( crashed == 3 )
    {
	default_action.sa_handler = SIG_DFL;
	sigaction( sig,  &default_action,   NULL );

	if( !fork( ) )
	{
	    kill( getppid( ), sig );
	    exit( 1 );
	}
	else
	    return;
 	raise( sig );
    }
}

int main( int argc, char **argv )
{
    struct timeval now_time;
    bool fCopyOver = FALSE;
    struct sigaction halt_action, ignore_action;

    halt_action.sa_handler = halt_mud;
    sigemptyset( &halt_action.sa_mask );
    halt_action.sa_flags = SA_NOMASK;

    ignore_action.sa_handler = SIG_IGN;
    sigemptyset( &ignore_action.sa_mask );
    ignore_action.sa_flags = 0;

    sigaction( SIGPIPE, &ignore_action, NULL );
    sigaction( SIGHUP,  &ignore_action, NULL );
    sigaction( SIGINT,  &halt_action,   NULL );
    sigaction( SIGQUIT, &halt_action,   NULL );
    sigaction( SIGILL,  &halt_action,   NULL );
    sigaction( SIGFPE,  &halt_action,   NULL );
    sigaction( SIGSEGV, &halt_action,   NULL );
    sigaction( SIGKILL,	&halt_action,	NULL );
    sigaction( SIGTERM, &halt_action,   NULL );
    sigaction( SIGBUS,  &halt_action,   NULL );

    gettimeofday( &now_time, NULL );
    current_time = (time_t) now_time.tv_sec;

    strftime( str_boot_time, 100, "{w%A{c, {w%B %d{c, {w%Y{c, at {w%I{c:{w%M{c:{w%S {c%p",
        localtime( &current_time ) );

    port = MAIN_GAME_PORT;

    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    exit( 1 );
	}

	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port numbers must be above 1024.\n" );
	    exit( 1 );
	}
    }

    if ( argv[2] != NULL && argv[2][0] != '\0' )
    {
	fCopyOver = TRUE;
	control = atoi( argv[2] );
    }

    else
        control = init_socket( port );

    boot_db( );

    if ( fCopyOver )
    {
        copyover_recover( );
	sprintf( log_buf, "Copy over working on port %d.", port );
    } else
	sprintf( log_buf, "RoT is ready to rock on port %d.", port );
    log_string( log_buf );

    game_loop_unix( control );
    close ( control );

    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}

bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;
    char buf[MAX_STRING_LENGTH];

    for ( ch = player_list; ch != NULL; ch = ch->pcdata->next_player )
    {
	if ((!fConn || ch->desc == NULL)
	&&   !str_cmp( d->character->name, ch->name ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		OBJ_DATA *obj;

		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
		d->timer	 = 0;
		if(ch->pcdata->tells)
		{
		    sprintf( buf, "Reconnecting.  You have {R%d{x tells waiting.\n\r",
			ch->pcdata->tells );
		    send_to_char( buf, ch );
		    send_to_char("Type 'replay' to see tells.\n\r",ch);
		}
		else
		{
			send_to_char("Reconnecting.  You have no tells waiting.\n\r",ch);
		}
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM, POS_RESTING );
		if ((obj = get_eq_char(ch,WEAR_LIGHT)) != NULL
		&&  obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
		    ++ch->in_room->light;

		sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
		log_string( log_buf );
		wiznet("$N groks the fullness of $S link.",
		    ch,NULL,WIZ_LINKS,0,get_trust(ch));
		d->connected = CON_PLAYING;
		if (ch->pcdata->pnote != NULL)
		    send_to_char("{YYou have a note in progress!{x\n\r",ch);

	    }
	    return TRUE;
	}
    }

    return FALSE;
}

bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
	         ? dold->original->name : dold->character->name ) )
	{
	    send_to_desc( "{3That character is already playing.\n\r",d);
	    send_to_desc( "{3Do you wish to connect anyway {R({WY{3/{wN{R){3?{x",d);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}

int weapon_lookup (const char *name)
{
    int type;

    for (type = 0; weapon_table[type].name != NULL; type++)
    {
	if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
	&&  !str_prefix(name,weapon_table[type].name))
	    return type;
    }

    return -1;
}

void list_classes( CHAR_DATA *ch, DESCRIPTOR_DATA *d )
{
    char buf[MAX_STRING_LENGTH];
    int i;

    send_to_desc( "\n\r{cThe following classes are available:\n\r\n\r", d );

    for ( i = 0; class_table[i].name[0] != '\0'; i++ )
    {
	if ( class_table[i].tier != ch->pcdata->tier
	||   !race_table[ch->race].class_can_use[i]
	||   class_table[i].disabled )
	    continue;

	sprintf( buf, "                 {%c%c{%c%s\n\r",
	    class_table[i].who_name[1], class_table[i].who_name[2],
	    class_table[i].who_name[4], class_table[i].name+1 );
	send_to_desc( buf, d );
    }

    send_to_desc( "\n\r{cWhat is your class?{x\n\r",d );
}

void list_races( CHAR_DATA *ch, DESCRIPTOR_DATA *d )
{
    char buf[MAX_STRING_LENGTH];
    int i, pos;

    send_to_desc( "\n{cThe following races are available:\n\n"
		  "  {wRace       {c(Possible Classes)", d );

    for ( i = 0; race_table[i].name[0] != '\0'; i++ )
    {
	if ( race_table[i].pc_race && !race_table[i].disabled )
	{
	    sh_int count = 0;

	    sprintf( buf, "\n  {w%-10s {c(", race_table[i].name );

	    for ( pos = 0; class_table[pos].name[0] != '\0'; pos++ )
	    {
		if ( class_table[pos].tier != ch->pcdata->tier
		||   class_table[pos].disabled )
		    continue;

		if ( count++ != 0 )
		    strcat( buf, " | " );

		if ( !race_table[i].class_can_use[pos] )
		{
		    sh_int space = strlen( class_table[pos].name );
		    while( space-- > 0 )
			strcat( buf, " " );
		}
		else
		    strcat( buf, class_table[pos].name );
	    }

	    strcat( buf, ")" );
	    send_to_desc( buf, d );
	}
    }

    send_to_desc( "\n\r\n\r{cWhat is your race {R({chelp for more information{R){c?{x\n\r", d );
}

void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    CHAR_DATA *ch, *wch;
    DESC_CHECK_DATA *dc;
    DESCRIPTOR_DATA *dt, *d_old, *d_next;
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char *pwdnew, *p;
    int iClass, race, i, weapon;
    bool fOld;

    while ( isspace(*argument) )
	argument++;

    ch = d->character;

    switch ( d->connected )
    {
	default:
	    bug( "Nanny: bad d->connected %d.", d->connected );
	    close_socket( d );
	    return;

	case CON_GET_NAME:
	    if ( argument[0] == '\0' )
	    {
		send_to_desc( "{W                       <{wenter your name to continue{W>{x\n\r", d );
//		close_socket( d );
		return;
	    }

	    argument[0] = UPPER(argument[0]);

	    if ( !check_parse_name( argument ) )
	    {
		send_to_desc( CLEAR_SCREEN,d);
		send_to_desc( "\n\n{RIllegal name, try another.\n\r"
			      "{c({RIf you've used this name here before, and are no\n\r"
			      " longer able to, it may be because we've added a\n\r"
			      " new mobile that uses the same name. Log in with\n\r"
			      " a new name, and let an IMM know, and we will fix it.{c)\n\r"
			      "\n\r{cWhich soul dares enter?{x ", d );
		return;
	    }

	    fOld = load_char_obj( d, argument, FALSE, FALSE );
	    ch   = d->character;

	    if ( mud_stat.multilock )
	    {
		for ( wch = player_list; wch != NULL; wch = wch->pcdata->next_player )
		{
		    if ( !IS_NPC(d->character)
		    &&   !IS_IMMORTAL(d->character)
		    &&   !IS_IMMORTAL(wch)
		    &&   str_cmp(d->character->name,"Tester")
		    &&   str_cmp(d->character->name,"Testguy")
		    &&   str_cmp(d->character->name,wch->name)
		    &&   d->host != NULL
		    &&   !check_allow(d->host,ALLOW_CONNECTS)
		    &&   !check_allow(d->host,ALLOW_ITEMS)
		    &&   wch->pcdata->socket != NULL
		    &&   !strcmp(d->host,wch->pcdata->socket) )
		    {
			sprintf(buf,"%s|%s@%s, ATTEMPTED MULTIPLAY!",
			    d->character->name, wch->name, d->host);
			log_string(buf);
			wiznet(buf,NULL,NULL,WIZ_OTHER,0,0);
			append_file( d->character, "../log/multiplay.txt", buf );

			sprintf( buf, "\n\r\n\r{RMultiplaying is Illegal!\n%s is already connected from your host!\nYour host: %s{x\n\r\n\r",
			    wch->name, d->host );
			send_to_desc( buf, d );

			sprintf( buf, "\n\r\n\r{RMultiplaying has been attempted!\n%s is attempting to connect from your host!\nYour host: %s{x\n\r\n\r",
			    d->character->name, d->host );
			send_to_char( buf, wch );

			close_socket(d);
			return;
		    }
		}
	    }

	    if (check_ban(d->host,BAN_PERMIT))
	    {
		send_to_desc( "Your site has been banned from here.\n\r",d);
		close_socket(d);
		return;
	    }

	    if (IS_SET(ch->comm, COMM_WIPED ) )
	    {
		sprintf(log_buf,"Denying access to %s@%s.",argument,d->host);
		log_string( log_buf );
		write_to_buffer( d, "You are denied access.\n\r", 0 );
		close_socket( d );
		return;
	    }

	    if ( !IS_IMMORTAL(ch) && mud_stat.multilock )
	    {
		for ( dc = desc_check_list; dc != NULL; dc = dc->next )
		{
		    if ( !str_cmp(dc->host,d->host)
		    &&    str_cmp(dc->name,ch->name) )
		    {
			char buf[MAX_STRING_LENGTH];

			sprintf(buf,"Due to abuse we have been forced to limit character switching.\n\r"
				    "Your site may not access any characters, besides %s, for %d minute%s.\n\r",
			    dc->name, dc->wait_time, dc->wait_time == 1 ? "" : "s");
			send_to_desc( buf, d );
			close_socket( d );
			return;
		    }
		}
	    }

	    if ( check_reconnect( d, argument, FALSE ) )
	    {
		fOld = TRUE;
	    } else {
		if ( mud_stat.wizlock && !IS_IMMORTAL(ch)) 
		{
		    write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		    close_socket( d );
		    return;
		}
	    }

	    if ( fOld )
	    {
		send_to_desc( "{cPassword:{x ", d );
		d->connected = CON_GET_OLD_PASSWORD;
		return;
	    } else {
		if (mud_stat.newlock)
		{
		    write_to_buffer( d, "The game is newlocked.\n\r", 0 );
		    close_socket( d );
		    return;
		}

		if (check_ban(d->host,BAN_NEWBIES))
		{
		    send_to_desc("New players are not allowed from your site.\n\r",d);
		    close_socket(d);
		    return;
		}

		for (dt = descriptor_list; dt != NULL; dt = dt->next)
		{
		    if ( d != dt && dt->character != NULL
		    &&   !str_cmp(dt->character->name,ch->name) )
		    {
			write_to_buffer(d,"Sorry, that name is being used.\n\r",0);
			write_to_buffer(d,"Please select another name: ",0);
			free_char( d->character );
			d->character = NULL;
			return;
		    }
		}

		send_to_desc( CLEAR_SCREEN, d );
		sprintf( buf, "\n\r{cThou wish to join the prey as {C%s {R({cY{R/{cN{R){c?{x ", argument );
		send_to_desc( buf, d );
		d->connected = CON_CONFIRM_NEW_NAME;
		return;
	    }
	    break;

	case CON_GET_OLD_PASSWORD:
	    #if defined(unix)
		write_to_buffer( d, "\n\r", 2 );
	    #endif

	    if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ))
	    {
		write_to_buffer( d, "Wrong password.\n\r", 0 );
		sprintf( log_buf, "%s. Bad Password. IP: %s", ch->name,d->host);
		log_string( log_buf );
		sprintf( log_buf, "%s@%s {RWRONG PASSWORD{V.", ch->name, d->host );
		wiznet(log_buf,NULL,NULL,WIZ_PASSWORDS,0,get_trust(ch));
		close_socket( d );
		return;
	    }

	    strftime( arg, 100, "%A{c, {w%B %d{c, {w%Y{c, at {w%I{c:{w%M{c:{w%S{c %p",
		localtime( &ch->pcdata->llogoff ) );

	    sprintf( buf, "\n\r{cPrevious login: {w%s {cfrom {w%s{c.{x\n\r",
		arg, ch->pcdata->socket );

	    if (check_playing(d,ch->name))
		return;

	    free_string(ch->pcdata->socket);
	    ch->pcdata->socket = str_dup( d->host );

	    if ( check_reconnect( d, ch->name, TRUE ) )
		return;

	    sprintf( log_buf, "Connection: %s@%s (%s).",
		ch->name, d->host, d->hostip );
	    log_string( log_buf );
	    wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

	    if (IS_SET(ch->act, PLR_REROLL) )
	    {
		send_to_desc("{w< {g!{G!{g!{wHit return to continue reroll procedures{g!{G!{g! {w>{x\n\r",d);
		d->connected = CON_REROLLING;
		send_to_char( buf, ch );
		break;
	    }

	    if ( IS_IMMORTAL(ch) )
	    {
		send_to_char(CLEAR_SCREEN, ch);
		do_help( ch, "imotd" );
		d->connected = CON_READ_IMOTD;
	    } else {
		send_to_char(CLEAR_SCREEN, ch);
		do_help( ch, "motd" );
		d->connected = CON_READ_MOTD;
	    }
	    send_to_char( buf, ch );
	    break;

	case CON_REROLLING:
	    die_follower( ch );

	    while( ch->affected )
		affect_remove( ch, ch->affected );

	    for ( i = 0; i < maxSkill; i++ )
		ch->learned[i] = 0;

	    for ( i = 0; group_table[i].name[0] != '\0'; i++ )
		ch->pcdata->group_known[i] = FALSE;

	    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
		obj->wear_loc = WEAR_NONE;

	    for ( i = 0; i < 4; i++ )
		ch->armor[i] = 100;

	    for ( i = 0; i < MAX_STATS; i++ )
		ch->mod_stat[i] = 0;

	    for ( i = 0; i < DEVOTE_CURRENT; i++ )
		ch->pcdata->devote[i] = 0;

	    weapon = 0;
	    for ( iClass = 0; class_table[iClass].name[0] != '\0'; iClass++ )
		weapon = UMAX( weapon, class_table[iClass].tier );

	    if ( IS_HERO( ch ) )
		ch->pcdata->tier = UMIN( class_table[ch->class].tier+1, weapon );
	    else
		ch->pcdata->tier = class_table[ch->class].tier;

	    ch->pcdata->confirm_reroll	= FALSE;
	    ch->pcdata->confirm_delete	= FALSE;
	    ch->level			= 0;
	    ch->magic_power		= 0;
	    ch->max_hit			= 100;
	    ch->pcdata->perm_hit	= 100;
	    ch->max_mana		= 100;
	    ch->pcdata->perm_mana	= 100;
	    ch->max_move		= 100;
	    ch->pcdata->perm_move	= 100;
	    ch->invis_level		= 0;
	    ch->incog_level		= 0;
	    ch->ghost_level		= 0;
	    ch->pcdata->dtimer		= 0;
	    ch->affected_by		= 0;
	    ch->shielded_by		= 0;
	    ch->parts			= 0;
	    ch->hitroll			= 0;
	    ch->damroll			= 0;
	    ch->saving_throw		= 0;

	    send_to_desc( CLEAR_SCREEN, d );

	    list_races( ch, d );

	    d->connected = CON_GET_NEW_RACE;
	    break;

	case CON_BREAK_CONNECT:
	    switch( UPPER(argument[0]) )
	    {
		case 'Y':
		    for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
		    {
			d_next = d_old->next;
			if (d_old == d || d_old->character == NULL)
			    continue;

			if (str_cmp(ch->name,d_old->original ?
			    d_old->original->name : d_old->character->name))
			    continue;

			close_socket(d_old);
		    }

		    free_string(ch->pcdata->socket);
		    ch->pcdata->socket = str_dup( d->host );
		    if (check_reconnect(d,ch->name,TRUE))
	    		return;
		    write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);

        	    if ( d->character != NULL )
		    {
			free_char( d->character );
			d->character = NULL;
		    }

		    d->connected = CON_GET_NAME;
		    break;

		case 'N':
		    sprintf( buf, "{cWhich soul dares enter %s{c?{x ",
			mud_stat.mud_name_string );
		    send_to_desc( buf, d );

	            if ( d->character != NULL )
		    {
			free_char( d->character );
			d->character = NULL;
		    }
		    d->connected = CON_GET_NAME;
		    break;

		default:
		    send_to_desc( "{RPlease type {cY {Ror {cN{R!{x ",d);
		    break;
	    }
	    break;

	case CON_CONFIRM_NEW_NAME:
	    switch ( UPPER(argument[0]) )
	    {
		case 'Y':
		    send_to_desc( CLEAR_SCREEN, d );
		    sprintf( buf, "\n\r{cGive me a password for {C%s{c:{x",
			ch->name );
		    free_string(ch->pcdata->socket);
		    ch->pcdata->socket = str_dup( d->host );
		    send_to_desc( buf, d );
		    d->connected = CON_GET_NEW_PASSWORD;
		    break;

		case 'N':
		    send_to_desc( CLEAR_SCREEN, d );
		    send_to_desc( "\n\n{cWhich name do you wish to use?{x ", d );
		    free_char( d->character );
		    d->character = NULL;
		    d->connected = CON_GET_NAME;
		    break;

		default:
		    send_to_desc( "{RPlease type {cYes {Ror {cNo{R!{x ", d );
		    break;
	    }
	    break;

	case CON_GET_NEW_PASSWORD:
	    #if defined(unix)
		write_to_buffer( d, "\n\r", 2 );
	    #endif

	    if ( strlen(argument) < 5 )
	    {
		send_to_desc("{cPassword must be at least five characters long.\n\rPassword:{x ", d );
		return;
	    }

	    pwdnew = crypt( argument, ch->name );

	    for ( p = pwdnew; *p != '\0'; p++ )
	    {
		if ( *p == '~' )
		{
		    send_to_desc("{cNew password not acceptable, try again.\n\rPassword:{x ", d );
		    return;
		}
	    }

	    free_string( ch->pcdata->pwd );
	    ch->pcdata->pwd	= str_dup( pwdnew );
	    send_to_desc( "{cPlease retype password:{x ", d );
	    d->connected = CON_CONFIRM_NEW_PASSWORD;
	    break;

	case CON_CONFIRM_NEW_PASSWORD:
	    #if defined(unix)
		write_to_buffer( d, "\n\r", 2 );
	    #endif

	    if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	    {
		send_to_desc( "{cForgot your password already?\n\rRetype password:{x ", d );
		d->connected = CON_GET_NEW_PASSWORD;
		return;
	    }

	    free_string(ch->pcdata->socket);
	    ch->pcdata->socket = str_dup( d->host );

	    send_to_desc( CLEAR_SCREEN, d );

	    list_races( ch, d );

	    d->connected = CON_GET_NEW_RACE;
	    break;

	case CON_GET_NEW_RACE:
	    one_argument( argument, arg );
	    send_to_desc( CLEAR_SCREEN, d );

	    if ( !strcmp( arg, "help" ) )
	    {
		argument = one_argument( argument, arg );

		send_to_desc( "\n\r", d );

		if ( argument[0] == '\0' )
		    do_help( ch, "race help" );
		else
		{
		    sprintf( buf, "race %s", argument );
		    do_checkstats( ch, buf );
		}

		send_to_desc( "\n\r{cWhat is your race {R({chelp for more information{R){c?{x ",d );
		break;
	    }

	    race = race_lookup( argument );

	    if ( race == -1 || !race_table[race].pc_race || race_table[race].disabled )
	    {
		send_to_desc( "\n\r{cThat is not a valid race.\n\r", d );
		list_races( ch, d );
		break;
	    }

	    ch->race = race;

	    for ( i = 0; i < MAX_STATS; i++ )
		ch->perm_stat[i] = race_table[race].stats[i];

	    for ( i = 0; i < DAM_MAX; i++ )
		ch->damage_mod[i] = race_table[race].damage_mod[i];

	    ch->parts = race_table[race].parts;

	    for ( i = 0; i < 5; i++ )
	    {
		if ( race_table[race].skills[i] == NULL )
		    break;

		group_add( ch, race_table[race].skills[i], FALSE );
	    }

	    ch->pcdata->points = race_table[race].points;
	    ch->size = race_table[race].size;
	    ch->dam_type = race_table[race].attack;

	    send_to_desc( CLEAR_SCREEN, d );
	    send_to_desc( "\n\r{cWhat is your sex {R({CM{R/{MF{R/{WN{R){c?{x ", d );
	    do_prompt (ch, "default");
	    d->connected = CON_GET_NEW_SEX;
	    break;

	case CON_GET_NEW_SEX:
	    switch ( UPPER(argument[0]) )
	    {
		case 'M':
		    ch->sex = SEX_MALE;    
		    ch->pcdata->true_sex = SEX_MALE;
		    break;

		case 'F':
		    ch->sex = SEX_FEMALE; 
		    ch->pcdata->true_sex = SEX_FEMALE;
		    break;

		case 'N':
		    ch->sex = SEX_NEUTRAL;
		    ch->pcdata->true_sex = SEX_NEUTRAL;
		    break;

		default:
		    send_to_desc( CLEAR_SCREEN, d );
		    send_to_desc( "\n\r{cWhat is your sex {R({CM{R/{MF{R/{WN{R){c?{x ", d );
		    return;
	    }

	    send_to_desc( CLEAR_SCREEN, d );

	    list_classes( ch, d );

	    d->connected = CON_GET_NEW_CLASS;

	    break;

	case CON_GET_NEW_CLASS:
	    if ( ( iClass = class_lookup( argument ) ) == -1 )
	    {
		send_to_desc( CLEAR_SCREEN, d );
		send_to_desc( "\n\r{cThat's not a class!{x\n\r", d );
		list_classes( ch, d );
		return;
	    }

	    if ( class_table[iClass].disabled )
	    {
		send_to_desc( CLEAR_SCREEN, d );
		send_to_desc( "\n\r{cThat class is currently disabled.{x\n\r", d );
		list_classes( ch, d );
		return;
	    }

	    if ( class_table[iClass].tier != ch->pcdata->tier )
	    {
		send_to_desc( CLEAR_SCREEN, d );
		send_to_desc("\n\r{cThat's not a valid class for your tier!\n\r", d);
		list_classes( ch, d );
		return;
	    }

	    if ( !race_table[ch->race].class_can_use[iClass] )
	    {
		send_to_desc( CLEAR_SCREEN, d );
		send_to_desc( "\n\r{cThat class was not designed to work with your race.{x\n\r", d );
		list_classes( ch, d );
		return;
	    }

	    ch->class = iClass;

	    sprintf( log_buf, "New Player: %s@%s (%s).",
		ch->name, d->host, d->hostip );
	    log_string( log_buf );
	    wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

	    send_to_desc( CLEAR_SCREEN, d );

	    send_to_desc( "\n\r{cYou may be {Bgood{c, {Wneutral{c, or {Revil{c.\n\r",d);
	    send_to_desc( "{cWhich alignment ({BG{c/{WN{c/{!E{c)?{x ",d);
	    d->connected = CON_GET_ALIGNMENT;
	    break;

	case CON_GET_ALIGNMENT:
	    switch( UPPER(argument[0]) )
	    {
		case 'G': ch->alignment = 750;	break;
		case 'N': ch->alignment = 0;	break;
		case 'E': ch->alignment = -750;	break;
		default:
		    send_to_desc( CLEAR_SCREEN, d );
		    send_to_desc( "\n\r{cThat's not a valid alignment.\n\r", d );
		    send_to_desc( "\n\r{cWhich alignment ({BG{c/{WN{c/{RE{c)?{x\n\r", d );
		    return;
	    }

	    group_add(ch,"rom basics",FALSE);
	    group_add(ch,class_table[ch->class].base_group,FALSE);
	    ch->learned[gsn_recall] = 50;

	    send_to_desc(CLEAR_SCREEN,d);
	    send_to_desc("\n\r{cDo you wish to customize this character?\n\r",d);
	    send_to_desc("{cCustomization takes time, but allows a wider range of skills and abilities.\n\r",d);
	    send_to_desc("{cCustomize {R({cY{R/{cN{R){c?{x ",d);
	    SET_BIT(ch->act,PLR_COLOUR);
	    d->connected = CON_DEFAULT_CHOICE;
	    break;

	case CON_DEFAULT_CHOICE:
	    send_to_desc(CLEAR_SCREEN,d);
	    switch ( UPPER(argument[0]) )
	    {
		case 'Y': 
		    ch->pcdata->gen_data = new_gen_data( );
		    ch->pcdata->gen_data->points_chosen = ch->pcdata->points;
		    send_to_desc("\n\r{cThe following skills and groups are available to your character:\n\r"
				 "(this list may be seen again by typing list){x\n\r",d);
		    list_group_costs(ch);
		    send_to_desc("{cYou already have the following skills:{x\n\r",d);
		    do_skills(ch,"");
		    send_to_desc("{CChoice {w({cadd{w, {cdrop{w, {clist{w, {chelp{w, {cclass{w, {clearned{w, {cdone{w){C?{x\n\r",d);
		    d->connected = CON_GEN_GROUPS;
		    break;

		case 'N': 
		    group_add(ch,class_table[ch->class].default_group,TRUE);
		    send_to_desc(CLEAR_SCREEN,d);
		    send_to_desc("\n\n{cPlease pick a weapon from the following choices:{w\n\r",d);
		    buf[0] = '\0';
		    for ( i = 0; weapon_table[i].name != NULL; i++)
		    {
			if (ch->learned[*weapon_table[i].gsn] > 0)
			{
			    strcat(buf,weapon_table[i].name);
			    strcat(buf," ");
			}
		    }
		    strcat(buf,"\n\r{cYour choice?{x ");
		    send_to_desc(buf,d);
		    d->connected = CON_PICK_WEAPON;
		    break;

		default:
		    send_to_desc(CLEAR_SCREEN,d);
		    send_to_desc("\n\r{cDo you wish to customize this character?\n\r",d);
		    send_to_desc("{cCustomization takes time, but allows a wider range of skills and abilities.\n\r",d);
		    send_to_desc("{cCustomize {R({cY{R/{cN{R){c?{x ",d);
		    return;
	    }
	    break;

	case CON_PICK_WEAPON:
	    write_to_buffer(d,"\n\r",2);
	    weapon = weapon_lookup( argument );
	    if (weapon == -1 || ch->learned[*weapon_table[weapon].gsn] <= 0)
	    {
		send_to_desc("{cThat's not a valid selection. Choices are:{w\n\r",d);
		buf[0] = '\0';
		for ( i = 0; weapon_table[i].name != NULL; i++)
		{
		    if (ch->learned[*weapon_table[i].gsn] > 0)
		    {
			strcat(buf,weapon_table[i].name);
			strcat(buf," ");
		    }
		}
		strcat(buf,"\n\r{cYour choice?{x ");
		send_to_desc(buf,d);
		return;
	    }

	    ch->learned[*weapon_table[weapon].gsn] = 40;
	    write_to_buffer(d,"\n\r",2);
	    send_to_char(CLEAR_SCREEN, ch);
	    do_help(d->character,"motd");
	    d->connected = CON_READ_MOTD;
	    break;

	case CON_GEN_GROUPS:
	    send_to_char("\n\r",ch);
	    if (!str_cmp(argument,"done"))
	    {
		send_to_desc(CLEAR_SCREEN,d);
		sprintf(buf,"\n\n{CCreation points{w: %d{x\n\r",ch->pcdata->points);
		send_to_char(buf,ch);
		sprintf(buf,"{CExperience per level{w: %ld{x\n\r",
	            exp_per_level(ch,ch->pcdata->gen_data->points_chosen));
		send_to_char(buf,ch);
		if (ch->pcdata->points < 40)
		    ch->pcdata->train = (40 - ch->pcdata->points + 1) / 2;
		free_gen_data( ch->pcdata->gen_data );
		ch->pcdata->gen_data = NULL;
		send_to_desc("\n\r{cPlease pick a weapon from the following choices:{w\n\r",d);
		buf[0] = '\0';
		for ( i = 0; weapon_table[i].name != NULL; i++)
		{
		    if (ch->learned[*weapon_table[i].gsn] > 0)
		    {
			strcat(buf,weapon_table[i].name);
			strcat(buf," ");
		    }
		}
		strcat(buf,"\n\r{cYour choice?{x ");
		send_to_desc(buf,d);
		d->connected = CON_PICK_WEAPON;
		break;
	    }

	    parse_gen_groups(ch,argument);
	    send_to_desc("{CChoice {w({cadd{w, {cdrop{w, {clist{w, {chelp{w, {cclass{w, {clearned{w, {cdone{w){C?{x\n\r",d);
	    break;

	case CON_READ_IMOTD:
	    send_to_char(CLEAR_SCREEN, ch);
	    do_help(d->character,"motd");
	    d->connected = CON_READ_MOTD;
	    break;

	case CON_READ_MOTD:
	    if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
	    {
		send_to_desc( "{RWarning! Null password!\n\r",d );
		send_to_desc( "Please report old password with bug.\n\r",d);
		send_to_desc( "Type 'password null <new password>' to fix.{x\n\r",d);
	    }

	    if ( mud_stat.multilock )
	    {
		for (wch = player_list; wch != NULL; wch = wch->pcdata->next_player)
		{
		    if ( !IS_NPC(d->character)
		    &&   !IS_IMMORTAL(d->character)
		    &&   !IS_IMMORTAL(wch)
		    &&   str_cmp(d->character->name,"Tester")
		    &&   str_cmp(d->character->name,"Testguy")
		    &&   str_cmp(d->character->name,wch->name)
		    &&   d->host != NULL
		    &&   !check_allow(d->host,ALLOW_CONNECTS)
		    &&   !check_allow(d->host,ALLOW_ITEMS)
		    &&   wch->pcdata->socket != NULL
		    &&   !strcmp(d->host,wch->pcdata->socket) )
		    {
			sprintf(buf,"%s|%s@%s, ATTEMPTED MULTIPLAY!",
			    d->character->name, wch->name, d->host);
			log_string(buf);
			wiznet(buf,NULL,NULL,WIZ_OTHER,0,0);
			append_file( d->character, "../log/multiplay.txt", buf );

			sprintf( buf, "\n\r\n\r{RMultiplaying Illegal!\n%s is already connected from your host!\nYour host: %s{x\n\r\n\r",
			    wch->name, d->host );
			send_to_desc( buf, d );

			sprintf( buf, "\n\r\n\r{RMultiplaying has been attempted!\n%s is attempting to connect from your host!\nYour host: %s{x\n\r\n\r",
			    d->character->name, d->host );
			send_to_char( buf, wch );

			return;
		    }
		}
	    }

	    ch->next		= char_list;
	    char_list		= ch;
	    d->connected	= CON_PLAYING;
	    ch->pcdata->next_player	= player_list;
	    player_list		= ch;
	    d->timer		= 0;
	    racial_spells(ch,TRUE);
	    do_devote_assign( ch );
	    
	    if ( ch->level == 0 )
	    {
		ch->perm_stat[class_table[ch->class].attr_prime] += 3;
		ch->level		= 1;
		ch->magic_power		= 1;
		ch->exp			= exp_per_level( ch, ch->pcdata->points );
		ch->hit			= ch->max_hit;
		ch->mana		= ch->max_mana;
		ch->move		= ch->max_move;
		ch->pcdata->train	= 25;
		ch->pcdata->practice	= 25;

		if ( ch->pcdata->tier == 1 )
		{
		    if ( ch->pcdata->title[0] == '\0' )
			sprintf (buf, "the %s that wanted to be a %s", class_table[ch->class].name,race_table[ch->race].name);
			set_title( ch, buf );

		    SET_BIT( ch->act, PLR_AUTOASSIST );
		    SET_BIT( ch->act, PLR_AUTOLOOT );
		    SET_BIT( ch->act, PLR_AUTOGOLD );
		    SET_BIT( ch->act, PLR_AUTOEXIT );
		    SET_BIT( ch->act, PLR_AUTOSAC );
		    SET_BIT( ch->act, PLR_AUTOPEEK );
		    SET_BIT( ch->act, PLR_AUTOSPLIT );
		    SET_BIT( ch->act, PLR_NOFOLLOW );
		    SET_BIT( ch->act, PLR_NOCANCEL );
		}

		REMOVE_BIT( ch->act, PLR_REROLL );
		obj_to_char( create_object( get_obj_index( OBJ_VNUM_MAP ) ), ch );
		obj_to_char( create_object( get_obj_index( OBJ_VNUM_WMAP ) ), ch );
		obj_to_char( create_object( get_obj_index( OBJ_VNUM_EMAP ) ), ch );

		char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
		send_to_char( "\n\r", ch );
		do_outfit( ch, "" );
	    }
	    else if ( ch->in_room != NULL )
	    {
		char_to_room( ch, ch->in_room );
	    }
	    else if ( IS_IMMORTAL(ch) )
	    {
		char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
	    }
	    else
	    {
		if ( ch->alignment <0 )
		    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLEB ) );
		else
		    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	    }

	    do_unread (ch, "");

	    if (ch->pcdata->pnote != NULL)
		send_to_char("{YYou have a note in progress!{x\n\r",ch);

	    if ( IS_IMMORTAL( ch ) )
	    {
		CHAR_DATA *rch;

		for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
		{
		    if ( IS_IMMORTAL( rch )
		    &&   can_see( ch, rch ) )
			act( "$n joins $t.",
			    ch, mud_stat.mud_name_string, rch, TO_VICT, POS_RESTING );
		}

		wiznet( "$N joins $t.", ch, mud_stat.mud_name_string, WIZ_LOGINS, WIZ_SITES, get_trust( ch ) );
		do_look( ch, "auto" );
	    } else {
		long tnl = ( ( ch->level + 1 ) * exp_per_level( ch, ch->pcdata->points ) - ch->exp );

		act( "$n joins $t.",
		    ch, mud_stat.mud_name_string, NULL, TO_ROOM, POS_RESTING );
		do_look( ch, "auto" );

		if ( tnl < 0 || tnl > exp_per_level( ch, ch->pcdata->points ) )
		{
		    ch->exp = exp_per_level( ch, ch->pcdata->points ) * ch->level;
		    send_to_char( "\n\r{REXP PROBLEM NOTED, YOUR EXP HAS BEEN RESET.{x\n\r", ch );
		    sprintf( log_buf, "EXP problem noted on %s, EXP reset.", ch->name );
		    bug( log_buf, 0 );
		}

		wiznet("$N has connected.",
		    ch,NULL,WIZ_LOGINS,WIZ_SITES,get_trust(ch));
		
		
	    }

	    if (ch->pet != NULL)
	    {
		char_to_room(ch->pet,ch->in_room);
		act("$n joins $t.",
		    ch->pet,mud_stat.mud_name_string,NULL,TO_ROOM,POS_RESTING);
	    }

	    if (ch->clan == clan_lookup("condemned")
	    &&  ch->pcdata->clan_rank > 0)
	    {
		ch->pcdata->clan_rank = 0;
		check_roster( ch, FALSE );
	    }
	    
	    mud_stat.connect_since_reboot++;
	    break;
    }

    return;
}

void bust_a_prompt( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    char doors[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    bool found = FALSE;
    const char *dir_name[] = { "N", "E", "S", "W", "U", "D" };
    char *door_name[] = { "north", "east", "south", "west", "up", "down" };
    int door, outlet, temp;
 
    sprintf( buf2, "%s", ch->prompt );
    if ( buf2 == NULL || buf2[0] == '\0' )
    {
	sprintf( buf, "<%dhp %dm %dmv> ",
	    ch->hit, ch->mana, ch->move );
	send_to_char( buf, ch );
	return;
    }

    if ( !IS_NPC( ch ) && IS_SET( ch->comm, COMM_AFK ) )
    {
	if ( ch->pcdata->tells > 0 )
	{
	    sprintf( buf, "[{BAFK{x] You have {R%d{x tell%s waiting.\n\r",
		ch->pcdata->tells, ch->pcdata->tells == 1 ? "" : "s" );
	    send_to_char( buf, ch );
	    return;
	} else {
	    send_to_char( "[{BAFK{x]", ch );
	    return;
	}
    }

    if ( ch->hit <= 0 || ch->max_hit <= 0 )
	sprintf( buf, "{R%d{x", ch->hit );
    else
    {
	temp = 100 * ch->hit / ch->max_hit;
	if ( temp >= 68 )
	    sprintf( buf, "{G%d{x", ch->hit );
	else if ( temp >= 34 )
	    sprintf( buf, "{Y%d{x", ch->hit );
	else
	    sprintf( buf, "{R%d{x", ch->hit );
    }
    str_replace_c( buf2, "%A", buf );

    if ( ch->mana <= 0 || ch->max_mana <= 0 )
	sprintf( buf, "{R%d{x", ch->mana );
    else
    {
	temp = ch->mana * 100 / ch->max_mana;
	if ( temp >= 68 )
	    sprintf( buf, "{G%d{x", ch->mana );
	else if ( temp >= 34 )
	    sprintf( buf, "{Y%d{x", ch->mana );
	else
	    sprintf( buf, "{R%d{x", ch->mana );
    }
    str_replace_c( buf2, "%B", buf );

    if ( ch->move <= 0 || ch->max_move <= 0 )
	sprintf(buf, "{R%d{x", ch->move );
    else
    {
	temp = 100 * ch->move / ch->max_move;
	if ( temp >= 68 )
	    sprintf( buf, "{G%d{x", ch->move );
	else if ( temp >= 34 )
	    sprintf( buf, "{Y%d{x", ch->move );
	else
	    sprintf( buf, "{R%d{x", ch->move );
    }
    str_replace_c( buf2, "%C", buf );

    sprintf( buf, "%d",ch->max_hit );
    str_replace_c( buf2, "%H", buf );

    sprintf( buf, "%d", ch->max_mana );
    str_replace_c( buf2, "%M", buf );

    sprintf( buf, "%d", ch->pcdata ? ch->pcdata->questpoints : 0 );
    str_replace_c( buf2, "%Q", buf );

    sprintf( buf, "%d", !ch->pcdata ? 0 : ch->pcdata->pktimer );
    str_replace_c( buf2, "%T", buf );

    sprintf( buf, "%d", ch->max_move );
    str_replace_c( buf2, "%V", buf );

    if ( IS_NPC( ch ) )
	strcat( buf, "0" );
    else if ( ch->level < LEVEL_HERO )
	sprintf( buf, "%ld",
	    ( ( ch->level + 1 ) * exp_per_level( ch, ch->pcdata->points ) - ch->exp ) );
    else
	sprintf( buf, "%ld",
	    ( ch->pcdata->devote_next[ch->pcdata->devote[DEVOTE_CURRENT]]
		- ch->pcdata->devote[ch->pcdata->devote[DEVOTE_CURRENT]] ) );
    str_replace_c( buf2, "%X", buf );

    sprintf( buf, "%d", ch->alignment );
    str_replace_c( buf2, "%a", buf );

    str_replace_c( buf2, "%c", "\n\r" );

    if ( ch->in_room != NULL )
    {
	doors[0] = '\0';
	for ( door = 0; door < 6; door++ )
	{
	    bool round = FALSE;
	    outlet = door;

	    if ( ( pexit = ch->in_room->exit[outlet] ) != NULL
	    &&   pexit ->u1.to_room != NULL
	    &&   ( can_see_room( ch, pexit->u1.to_room )
	    ||     ( IS_AFFECTED( ch, AFF_INFRARED )
	    &&       !IS_AFFECTED( ch, AFF_BLIND ) ) ) )
	    {
		found = TRUE;
		round = TRUE;

		if ( !IS_SET( pexit->exit_info, EX_CLOSED ) )
		    strcat( doors, dir_name[door] );

		else if ( !IS_SET( pexit->exit_info, EX_HIDDEN )
		     ||   IS_IMMORTAL( ch ) )
		{
		    strcat( doors, "(" );
		    strcat( doors, dir_name[door] );
		    strcat( doors, ")" );
		}
	    }

	    if ( !round )
	    {
		OBJ_DATA *portal;

		portal = get_obj_exit( door_name[door], ch->in_room->contents );

		if ( portal != NULL && !IS_AFFECTED( ch, AFF_BLIND ) )
		{
		    found = TRUE;
		    round = TRUE;
		}
	    }
	}
    }

    if ( !found )
	sprintf( buf, "none" );
    else
	sprintf( buf, "%s", doors );
    str_replace_c( buf2, "%e", buf );

    sprintf( buf, "%d", ch->gold );
    str_replace_c( buf2, "%g", buf );

    sprintf( buf, "%d", ch->hit );
    str_replace_c( buf2, "%h", buf );

    sprintf( buf, "%d", ch->mana );
    str_replace_c( buf2, "%m", buf );

    sprintf( buf, "%d", ch->platinum );
    str_replace_c( buf2, "%p", buf );

    sprintf( buf, "%d", IS_NPC( ch ) ? 0 : IS_SET( ch->act, PLR_QUESTOR ) ?
	ch->pcdata->countdown : ch->pcdata->nextquest );
    str_replace_c( buf2, "%q", buf );

    if ( ch->in_room != NULL )
	sprintf( buf, "%s",
	    ( ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) ) ||
	    ( !IS_AFFECTED( ch, AFF_BLIND ) && !room_is_dark( ch->in_room ) ) )
	    ? ch->in_room->name : "darkness" );
    else
	sprintf( buf, " " );
    str_replace_c( buf2, "%r", buf );

    sprintf( buf, "%d", ch->silver );
    str_replace_c( buf2, "%s", buf );

    sprintf( buf, "%s", ch->in_room && IS_AFFECTED( ch, AFF_DETECT_TERRAIN ) ?
	flag_string( sector_type, ch->in_room->sector_type ) : "unknown" );
    str_replace_c( buf2, "%t", buf );

    sprintf( buf, "%d", ch->move );
    str_replace_c( buf2, "%v", buf );

    if ( IS_NPC( ch ) || ch->level < LEVEL_HERO )
	sprintf( buf, "%ld", ch->exp );
    else
	sprintf( buf, "%ld", ch->pcdata->devote[ch->pcdata->devote[DEVOTE_CURRENT]] );
    str_replace_c( buf2, "%x", buf );

    if ( IS_IMMORTAL( ch ) && ch->in_room )
    {
	sprintf( buf, "%d", ch->in_room->vnum );
	str_replace_c( buf2, "%R", buf );

	sprintf( buf, "%s", ch->in_room->area->name );
	str_replace_c( buf2, "%z", buf );
    }

    send_to_char( buf2, ch );

    if ( ch->pcdata && ch->pcdata->prefix[0] != '\0' )
    {
	sprintf( buf, "%s ", ch->pcdata->prefix );
        write_to_buffer( ch->desc, buf, 0 );
    }
}

bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    extern bool merc_down;

    if ( !merc_down )
    {
	if ( d->showstr_point && d->connected == CON_PLAYING )
	    send_to_char("\r[Hit Return to Continue]",d->character);
	else if ( d->showstr_point && d->connected != CON_PLAYING )
	    send_to_desc("\r[Hit Return to Continue]",d);
	else if ( fPrompt && d->pString && d->connected == CON_PLAYING )
	    write_to_buffer( d, "> ", 2 );
    	else if ( fPrompt && d->connected == CON_PLAYING )
	{
	    CHAR_DATA *ch;
	    CHAR_DATA *victim;

	    ch = d->character;

	    if ((victim = ch->fighting) != NULL && can_see(ch,victim))
	    {
		send_to_char( show_condition( ch, victim, VALUE_HIT_POINT ), ch );

		if ( victim->stunned )
		    act( "{f$N {fis stunned.{x", ch, NULL, victim, TO_CHAR, POS_RESTING );

		if ( ch->stunned )
		    send_to_char( "{fYou are stunned.{x\n\r", ch );
	    }

	    ch = d->original ? d->original : d->character;

	    if ( !IS_SET( ch->configure, CONFIG_COMPACT ) )
		write_to_buffer( d, "\n\r", 2 );

	    switch ( d->editor )
	    {
		case ED_AREA:
		    printf_to_char( d->character, 
			"{s<{qEditing Area: {s[{t%d{s] {q%s{s>{x\n\r",
			((AREA_DATA *)d->pEdit)->vnum,
			((AREA_DATA *)d->pEdit)->name );
		    break;

		case ED_ROOM:
		    printf_to_char( d->character,
			"{s<{qEditing Room: {s[{t%d{s] {q%s{s>{x\n\r",
			((ROOM_INDEX_DATA *)d->pEdit)->vnum,
			((ROOM_INDEX_DATA *)d->pEdit)->name );
		    break;

		case ED_OBJECT:
		    printf_to_char( d->character,
			"{s<{qEditing Object: {s[{t%d{s] {q%s{s>{x\n\r",
			((OBJ_INDEX_DATA *)d->pEdit)->vnum,
			((OBJ_INDEX_DATA *)d->pEdit)->short_descr );
		    break;

		case ED_MOBILE:
		    printf_to_char( d->character,
			"{s<{qEditing Mobile: {s[{t%d{s] {q%s{s>{x\n\r",
			((MOB_INDEX_DATA *)d->pEdit)->vnum,
			((MOB_INDEX_DATA *)d->pEdit)->short_descr );
		    break;

		case ED_MPCODE:
		    printf_to_char( d->character,
			"{s<{qEditing Mob Program: {s[{t%d{s] {q%s{s>{x\n\r",
			((PROG_CODE *)d->pEdit)->vnum,
			((PROG_CODE *)d->pEdit)->name );
		    break;

		case ED_OPCODE:
		    printf_to_char( d->character,
			"{s<{qEditing Obj Program: {s[{t%d{s] {q%s{s>{x\n\r",
			((PROG_CODE *)d->pEdit)->vnum,
			((PROG_CODE *)d->pEdit)->name );
		    break;

		case ED_RPCODE:
		    printf_to_char( d->character,
			"{s<{qEditing Room Program: {s[{t%d{s] {q%s{s>{x\n\r",
			((PROG_CODE *)d->pEdit)->vnum,
			((PROG_CODE *)d->pEdit)->name );
		    break;

		case ED_HELP:
		    printf_to_char( d->character,
			"{s<{qEditing Help: {t%s{s>{x\n\r",
			((HELP_DATA *)d->pEdit)->name );
		    break;

		case ED_SHOP:
		    printf_to_char( d->character,
			"{s<{qEditing Shop: {s[{t%d{s] {q%s{s>{x\n\r",
			((SHOP_DATA *)d->pEdit)->keeper,
			get_mob_index( ((SHOP_DATA *)d->pEdit)->keeper )->short_descr );
		    break;

		case ED_SKILL:
		    printf_to_char( d->character,
			"{s<{qEditing Skill: {s[{t%d{s] {q%s{s>{x\n\r",
			((int)d->pEdit ),
			skill_table[((int)d->pEdit)].name );
		    break;

		case ED_GROUP:
		    printf_to_char( d->character,
			"{s<{qEditing Group: {s[{t%d{s] {q%s{s>{x\n\r",
			((int)d->pEdit ),
			group_table[((int)d->pEdit)].name );
		    break;

		case ED_CLASS:
		    printf_to_char( d->character,
			"{s<{qEditing Class: {s[{t%d{s] {q%s{s>{x\n\r",
			((int)d->pEdit ),
			class_table[((int)d->pEdit)].name );
		    break;

		case ED_CLAN:
		    printf_to_char( d->character,
			"{s<{qEditing Clan: {s[{t%d{s] {q%s{s>{x\n\r",
			((int)d->pEdit ),
			clan_table[((int)d->pEdit)].color );
		    break;

		case ED_RACE:
		    printf_to_char( d->character,
			"{s<{qEditing Race: {s[{t%d{s] {q%s{s>{x\n\r",
			((int)d->pEdit ),
			race_table[((int)d->pEdit)].name );
		    break;

		case ED_SOCIAL:
		    printf_to_char( d->character,
			"{s<{qEditing Social: {s[{t%d{s] {q%s{s>{x\n\r",
			((int)d->pEdit ),
			social_table[((int)d->pEdit)].name );
		    break;

		case ED_ROOM_DAM:
		    printf_to_char( d->character,
			"{s<{qEditing Room Damage: {s[{t%d{s] {q%s{s>{x\n\r",
			ch->in_room->vnum, ch->in_room->name );
		    break;

		case ED_CHANNEL:
		    printf_to_char( d->character,
			"{s<{qEditing Channel: {s[{t%d{s] {q%s{s>{x\n\r",
			((int)d->pEdit ), channel_table[((int)d->pEdit)].name );
		    break;

		case ED_COMMAND:
		    printf_to_char( d->character,
			"{s<{qEditing Command: {s[{t%d{s] {q%s{s>{x\n\r",
			((int)d->pEdit ), cmd_table[((int)d->pEdit)].name );
		    break;

		case ED_PREFIX:
		    printf_to_char( d->character,
			"{s<{qEditing Prefix: {s[{t%d{s] {q%s{s>{x\n\r",
			((int)d->pEdit ), get_random_editing( d ) );
		    break;

		case ED_SUFFIX:
		    printf_to_char( d->character,
			"{s<{qEditing Suffix: {s[{t%d{s] {q%s{s>{x\n\r",
			((int)d->pEdit ), get_random_editing( d ) );
		    break;

		case ED_GAME_STAT:
		    send_to_char( "{s<{qEditing Game Variables{s>{x\n\r", d->character );
		    break;

		default:
		    if ( IS_SET( ch->comm, COMM_PROMPT ) )
			bust_a_prompt( d->character );
		    break;
	    }
	}

//	write_to_buffer( d, go_ahead, 0 );
    }    
    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
	if (d->character != NULL)
	    write_to_buffer( d->snoop_by, d->character->name,0);
	write_to_buffer( d->snoop_by, "> ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    write_to_descriptor( d, d->outbuf, d->outtop );
    d->outtop = 0;
    return TRUE;
}

int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close(fd);
	exit( 1 );
    }

#if defined(SO_DONTLINGER) && !defined(SYSV)
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof(ld) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close(fd);
	    exit( 1 );
	}
    }
#endif

    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
	perror("Init socket: bind" );
	close(fd);
	exit(1);
    }

    if ( listen( fd, 3 ) < 0 )
    {
	perror("Init socket: listen");
	close(fd);
	exit(1);
    }

    return fd;
}

void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i, j, k;

    if ( d->incomm[0] != '\0' )
	return;

    if ( d->connected == CON_PLAYING )
    {
	if ( d->run_buf )
	{
	    while (isdigit(*d->run_head) && *d->run_head != '\0')
	    {
		char *s,*e;
 
		s = d->run_head;
		while( isdigit( *s ) )
		    s++;
		e = s;
		while( *(--s) == '0' && s != d->run_head );
		if ( isdigit( *s ) && *s != '0' && *e != 'o')
		{
		    d->incomm[0] = *e;
		    d->incomm[1] = '\0';
		    s[0]--;
		    while (isdigit(*(++s)))
			*s = '9';
		    return;
		}
		if (*e == 'o')
		    d->run_head = e;
		else
		    d->run_head = ++e;
	    }
	    if (*d->run_head != '\0')
	    {
		if (*d->run_head != 'o')
		{
		    d->incomm[0] = *d->run_head++;
		    d->incomm[1] = '\0';
		    return;
		} else {
		    char buf[MAX_INPUT_LENGTH];

		    d->run_head++;

		    sprintf( buf, "open " );
		    switch( *d->run_head )
		    {
			case 'n' : sprintf( buf+strlen(buf), "north" );	break;
			case 's' : sprintf( buf+strlen(buf), "south" );	break;
			case 'e' : sprintf( buf+strlen(buf), "east" );	break;
			case 'w' : sprintf( buf+strlen(buf), "west" );	break;
			case 'u' : sprintf( buf+strlen(buf), "up" );	break;
			case 'd' : sprintf( buf+strlen(buf), "down" );	break;
			default: return;
		    }

		    strcpy( d->incomm, buf );
		    d->run_head++;
		    return;
		}
	    }

	    free_runbuf(d);
	}
    }

    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
	else if (d->inbuf[i] == (signed char)IAC)
	{
	    if ( !memcmp( &d->inbuf[i], compress_do, strlen( compress_do ) ) )
	    {
		i += strlen(compress_do) - 1;
		compressStart(d);
	    }
	    else if ( !memcmp( &d->inbuf[i], compress_dont, strlen( compress_dont ) ) )
	    {
		i += strlen(compress_dont) - 1;
		compressEnd(d);
	    }
	}
    }

    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    if ( k > 1 || d->incomm[0] == '!' )
    {
	if (d->connected == CON_GET_NAME && d->incomm[0] == '!' )
	{
	    char buf[MAX_STRING_LENGTH];

	    send_to_desc("{RNice try {zasshole{x{R!{x\n\r", d );
	    sprintf(buf,"%s attempting !!! crash!",d->host);
	    log_string(buf);
	    close_socket( d );
	    return;
	}
    }

    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}

void game_loop_unix( int control )
{
    static struct timeval null_time;
    struct timeval last_time;

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    while ( !merc_down )
    {
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA *d;
	int maxdesc;

	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc	= control;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}

	if ( FD_ISSET( control, &in_set ) )
	    init_descriptor( control );

	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character && d->character->level > 1)
		    save_char_obj( d->character, 0 );
		d->outtop	= 0;
		close_socket( d );
	    }
	}

	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;

		if ( !read_from_descriptor( d ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character != NULL && d->character->level > 1)
			save_char_obj( d->character, 0 );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->connected == CON_PLAYING )
	    {
		if ( d->character != NULL )
		{
		    sh_int daze;
		    for ( daze = 0; daze < MAX_DAZE; daze++ )
		    {
			if ( d->character->daze[daze] > 0 )
			    --d->character->daze[daze];
		    }

		    if ( d->character->wait > 0 )
		    {
			--d->character->wait;
			continue;
		    }
		}

	    }

	    if ( d->incomm[0] == '\0' )
		read_from_buffer( d );

	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand = TRUE;
		d->timer = 0;

		if ( d->connected == CON_PLAYING
		&&   !IS_NPC(d->character)
		&&   d->character->pcdata->lag > 0 )
		    d->character->wait = d->character->pcdata->lag;

		if ( d->showstr_point )
		    show_string( d, d->incomm );
		else if ( d->pString )
		    string_add( d->character, d->incomm );
		else
		{
                    switch ( d->connected )
                    {
                        case CON_PLAYING:
                            if ( !run_olc_editor( d ) )
				substitute_alias( d, d->incomm );
                            break;

                        default:
                            nanny( d, d->incomm );
                            break;
                    }
		}
		if (d->connected != CON_PLAYING || d->a_cur[0] == '\0')
		    d->incomm[0] = '\0';
	    }
 	}

	update_handler( FALSE );

	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 || d->out_compress )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
		bool ok = TRUE;

		if ( d->fcommand || d->outtop > 0 )
		    ok = process_output( d, TRUE );

		if ( ok && d->out_compress )
		    ok = processCompressed( d );

		if ( !ok )
		{
		    if ( d->character != NULL && d->character->level > 1 )
			save_char_obj( d->character, 0 );
		    d->outtop	= 0;
		}
	    }
	}

	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "Game_loop: select: stall" );
		    exit( 1 );
		}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}

void init_descriptor( int control )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    struct sockaddr_in sock;
    struct hostent *from;
    int desc;
    unsigned int size;

    size = sizeof(sock);
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }

    dnew = new_descriptor();
    dnew->descriptor	= desc;
    dnew->pEdit		= NULL;
    dnew->pString	= NULL;
    dnew->editor	= 0;
    dnew->timer		= 0;

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );
    }
    else
    {
	int addr;

	addr = ntohl( sock.sin_addr.s_addr );
	sprintf( buf, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
	sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	log_string( log_buf );

	dnew->hostip = str_dup( buf );

	if ( (from = gethostbyaddr( (char *) &sock.sin_addr,
		sizeof(sock.sin_addr), AF_INET )) == NULL )
	    dnew->host = str_dup( buf );
	else
	    dnew->host = str_dup( from->h_name );
    }
	
    if ( check_ban(dnew->host,BAN_ALL))
    {
	write_to_descriptor( dnew,
	    "Your site has been banned from here.\n\r", 0 );
	close( desc );
	free_descriptor(dnew);
	return;
    }

    dnew->next			= descriptor_list;
    descriptor_list		= dnew;
    write_to_buffer( dnew, compress_will, 0 );

    {
	extern char * help_greeting1;
	extern char * help_greeting2;
	extern char * help_greeting3;
	extern char * help_greeting4;
	extern char * help_greeting5;
	extern char * help_greeting6;
	extern char * help_greeting7;
	extern char * help_greeting8;
	extern char * help_greeting9;
	extern char * help_greeting10;
	extern char * help_greeting11;
	extern char * help_greeting12;
	extern char * help_authors;
	        send_to_desc( help_authors, dnew );
	switch (number_range(0,0))
	{
	    default:
	    case 0:
		send_to_desc( help_greeting1, dnew );
		break;
	    case 1:
		send_to_desc( help_greeting2, dnew );
		break;
	    case 2:
		send_to_desc( help_greeting3, dnew );
		break;
	    case 3:
		send_to_desc( help_greeting4, dnew );
		break;
	    case 4:
		send_to_desc( help_greeting5, dnew );
		break;
            case 5:
		send_to_desc( help_greeting6, dnew );
                break;
            case 6:
		send_to_desc( help_greeting7, dnew );
                break;
            case 7:
		send_to_desc( help_greeting8, dnew );
                break;
            case 8:
		send_to_desc( help_greeting9, dnew );
                break;
            case 9:
		send_to_desc( help_greeting10, dnew );
                break;
            case 10:
		send_to_desc( help_greeting11, dnew );
                break;
            case 11:
		send_to_desc( help_greeting12, dnew );
                break;
	}

//	sprintf( buf, "%s", center_string( mud_stat.mud_name_string, 79 ) );
//	send_to_desc( buf, dnew );

	send_to_desc( "{D---------------{w---------------{W---------------{w---------------{D---------------\n\r", dnew );

//	send_to_desc( help_authors, dnew );
	send_to_desc( "{!             W{1ho dares to enter?{W <{wenter your name to continue{W>{x\n\r", dnew );
    }
    return;
}

void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
	if ( IS_SET( ch->sound, SOUND_ON ) )
	{
	    send_to_char( "\n\r!!SOUND(Off)\n\r", ch );
	    if ( !IS_SET( ch->sound, SOUND_NOMUSIC ) )
		send_to_char( "\n\r!!MUSIC(Off)\n\r", ch );
	}

	sprintf( log_buf, "Closing link to %s.", ch->name );
	log_string( log_buf );

 	if (dclose->connected == CON_PLAYING)
	{
	    if ( !IS_IMMORTAL(ch) && ch->name != NULL
	    &&   dclose->host != NULL && mud_stat.multilock 
	    &&   !check_allow(dclose->host,ALLOW_CONNECTS)
	    &&   !check_allow(dclose->host,ALLOW_ITEMS) )
	    {
		DESC_CHECK_DATA *dc, *dc_next;
		bool found = FALSE;

		for ( dc = desc_check_list; dc != NULL; dc = dc_next )
		{
		    dc_next = dc->next;

		    if ( !str_cmp(dc->name,ch->name)
		    ||   !str_cmp(dc->host,dclose->host) )
		    {
			strcpy(dc->name,ch->name);
			strcpy(dc->host,dclose->host);
			dc->wait_time = 5;
			found = TRUE;
			break;
		    }
		}

		if ( !found )
		{
		    dc = new_desc_check();

		    strcpy(dc->name,ch->name);
		    strcpy(dc->host,dclose->host);
		    dc->wait_time = 5;

		    dc->next		= desc_check_list;
		    desc_check_list	= dc;
		}
	    }

	    act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM, POS_RESTING );
	    wiznet("Net death has claimed $N.",ch,NULL,WIZ_LINKS,0,get_trust(ch));

	    if ( !IS_NPC(ch)
	    &&   ch->pcdata->match != NULL
	    &&   !IS_SET(ch->pcdata->match->specials, ARENA_PROGRESSING) )
		arena_clear( ch->pcdata->match );

	    ch->desc = NULL;
	}
	else
	{
	    free_char( dclose->original ? dclose->original : dclose->character );
	}
    }

    free_runbuf(dclose);
    dclose->a_cur[0] = '\0';

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    if ( dclose->out_compress )
    {
	deflateEnd( dclose->out_compress );
	free_mem( dclose->out_compress_buf, COMPRESS_BUF_SIZE );
	free_mem( dclose->out_compress, sizeof( z_stream ) );
	if ( !compressEnd( dclose ) )
	    write_to_descriptor( dclose, "Failed to stop compression.\n\r", 0 );
    }

    close( dclose->descriptor );
    free_descriptor(dclose);
    return;
}

bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_desc("\n\r{g*{G*{g* {RQUIT SPAMMING US DUMBASS {g*{G*{g*{x\n\r\n\r", d);
	return FALSE;
    }

#if defined(MSDOS) || defined(unix)
    for ( ; ; )
    {
	int nRead;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}

void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    if ( d == NULL )
	return;

    if ( length <= 0 )
	length = strlen(txt);

    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

	if ( d == NULL )
	{
	    bug("NULL Descriptor!",0);
	    return;
	}

        if ( d->outsize >= 131072 )
	{
	    close_socket(d);
	    log_string( "Buffer overflow. Closing." );
	    return;
 	}
	outbuf      = alloc_mem( 2 * d->outsize );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    strncpy( d->outbuf + d->outtop, txt, length );
    d->outtop += length;
    return;
}

bool check_parse_name( char *name )
{
    int e;

    if (is_name(name,"! all auto immortal immortals self someone "
		     "something the you jesus aabrazak damnation aod deviant"))
	return FALSE;
	
    for ( e = 1; clan_table[e].name[0] != '\0'; e++ )
    {
	if (!str_prefix(clan_table[e].name, name))
	    return FALSE;
    }

    if ( !str_prefix("!",name)
    ||   !str_prefix("self",name)
    ||   !str_infix("immortal",name)
    ||   !str_infix(" ", name)
    ||   !str_infix("fuck",name)
    ||   !str_infix("shit",name)
    ||   !str_infix("asshole",name)
    ||   !str_infix("pussy",name) )
        return FALSE;
     
    if ( strlen(name) <  2 )
	return FALSE;

    if ( strlen(name) > 12 )
	return FALSE;

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll,adjcaps = FALSE,cleancaps = FALSE;
 	int total_caps = 0;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;

	    if ( isupper(*pc)) /* ugly anti-caps hack */
	    {
		if (adjcaps)
		    cleancaps = TRUE;
		total_caps++;
		adjcaps = TRUE;
	    }
	    else
		adjcaps = FALSE;

	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;

	if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
	    return FALSE;
    }

    return TRUE;
}

void send_to_char_bw( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
        write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}

void send_to_char( const char *txt, CHAR_DATA *ch )
{
    const	char 	*point;
    		char 	*point2;
    		char 	buf[ 262144 ];

    buf[0] = '\0';
    point2 = buf;

    if ( txt && ch && ch->desc )
    {
	if ( IS_SET( ch->act, PLR_COLOUR ) )
	{
	    for ( point = txt ; *point ; point++ )
	    {
		if ( *point == '{' )
		{
		    point++;
		    strcat( buf, colour( *point, ch ) );
		    for( point2 = buf ; *point2 ; point2++ )
			;
		    continue;
		}
		*point2 = *point;
		*++point2 = '\0';
	    }			

	    *point2 = '\0';
            write_to_buffer( ch->desc, buf, point2 - buf );
	} else {
	    for( point = txt ; *point ; point++ )
	    {
		if( *point == '{' )
		{
		    point++;
		    if( *point == '-' )
		    {
			*point2 = '~';
			*++point2 = '\0';
			continue;
		    }

		    else if( *point != '{' )
			continue;
		}
		*point2 = *point;
		*++point2 = '\0';
	    }
	    *point2 = '\0';
            write_to_buffer( ch->desc, buf, point2 - buf );
	}
    }
    return;
}

void send_to_desc( const char *txt, DESCRIPTOR_DATA *d )
{
    const       char    *point;
                char    *point2;
                char    buf[ 262144 ];

    buf[0] = '\0';
    point2 = buf;

    if( txt && d )
    {
	for( point = txt ; *point ; point++ )
	{
	    if( *point == '{' )
	    {
		point++;
		strcat( buf, colour2( *point ) );
		for( point2 = buf ; *point2 ; point2++ )
		    ;
		continue;
	    }
	    *point2 = *point;
	    *++point2 = '\0';
	}
	*point2 = '\0';
	write_to_buffer( d, buf, point2 - buf );
    }
    return;
}

void page_to_char_bw( const char *txt, CHAR_DATA *ch )
{
    if ( txt == NULL || ch->desc == NULL )
	return;

    if ( ch->pcdata && ch->pcdata->lines == 0 )
    {
	send_to_char( txt, ch );
	return;
    }
	
    ch->desc->showstr_head = alloc_mem( strlen( txt ) + 1 );
    strcpy( ch->desc->showstr_head, txt );
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string( ch->desc, "" );
}

void page_to_char( const char *txt, CHAR_DATA *ch )
{
    const 	char	*point;
		char	*point2;
    		char	buf[ 262144 ];

    buf[0] = '\0';
    point2 = buf;

    if ( txt && ch->desc )
    {
	if ( ch->desc->connected != CON_PLAYING )
	{
	    send_to_desc( txt, ch->desc );
	    return;
	}

//	if( (actlen_color(txt) + actlen_color(ch->desc->showstr_head)) >= 65536*2 )
	if ( actlen_color( txt ) >= 262144 )
	{
	    send_to_char("Input exceeds page_to_char buffer, Output terminated.\n\r",ch);
	    return;
	}

	if( IS_SET( ch->act, PLR_COLOUR ) )
	{
	    for( point = txt ; *point ; point++ )
	    {
		if( *point == '{' )
		{
		    point++;
		    strcat( buf, colour( *point, ch ) );
		    for( point2 = buf ; *point2 ; point2++ )
			;
		    continue;
		}
		*point2 = *point;
		*++point2 = '\0';
	    }
	    *point2 = '\0';
	    free_string( ch->desc->showstr_head );
	    ch->desc->showstr_head  = str_dup( buf );
	    ch->desc->showstr_point = ch->desc->showstr_head;
	    show_string( ch->desc, "" );
	} else {
	    for( point = txt ; *point ; point++ )
	    {
		if( *point == '{' )
		{
		    point++;
		    if( *point == '-' )
		    {
			*point2 = '~';
			*++point2 = '\0';
			continue;
		    }

		    else if( *point != '{' )
			continue;
		}
		*point2 = *point;
		*++point2 = '\0';
	    }
	    *point2 = '\0';
	    free_string( ch->desc->showstr_head );
	    ch->desc->showstr_head  = str_dup( buf );
	    ch->desc->showstr_point = ch->desc->showstr_head;
	    show_string( ch->desc, "" );
	}
    }
    return;
}

void show_string( struct descriptor_data *d, char *input )
{
    char buffer[262144];
    char buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument( input, buf );

    if ( buf[0] != '\0' )
    {
	if ( d->showstr_head )
	{
	    free_string( d->showstr_head );
	    d->showstr_head = 0;
	}

    	d->showstr_point  = 0;
	return;
    }

    if ( d->character && d->character->pcdata )
	show_lines = d->character->pcdata->lines;
    else
	show_lines = 0;

    for ( scan = buffer; ; scan++, d->showstr_point++ )
    {
	if ( ( ( *scan = *d->showstr_point ) == '\n' || *scan == '\r' )
	&&   ( toggle = -toggle ) < 0 )
	    lines++;

	else if ( !*scan || ( show_lines > 0 && lines >= show_lines ) )
	{
	    *scan = '\0';

	    write_to_buffer( d, buffer, strlen( buffer ) );

	    for ( chk = d->showstr_point; isspace( *chk ); chk++ );
	    {
		if ( !*chk )
		{
		    if ( d->showstr_head )
		    {
			free_string( d->showstr_head );
			d->showstr_head = 0;
		    }

		    d->showstr_point = 0;
		}
	    }

	    return;
	}
    }

    return;
}
	

/* quick sex fixer */
void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
    	ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}

void act( const char *format, CHAR_DATA *ch, const void *arg1, 
	      const void *arg2, int type, int min_pos )
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    CHAR_DATA 		*to;
    CHAR_DATA 		*vch = ( CHAR_DATA * ) arg2;
    OBJ_DATA 		*obj1 = ( OBJ_DATA  * ) arg1;
    OBJ_DATA 		*obj2 = ( OBJ_DATA  * ) arg2;
    const	char 	*str;
    char 		*i;
    char 		*point;
    char 		*i2;
    char 		fixed[ MAX_STRING_LENGTH ];
    char 		buf[ MAX_STRING_LENGTH   ];
    char 		fname[ MAX_INPUT_LENGTH  ];
    bool		fColour = FALSE, found = FALSE;
    int place = 0;

    if( !format || !*format || !ch || !ch->in_room )
	return;

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
        if ( !vch )
        {
            bug( "Act: null vch with TO_VICT.", 0 );
            return;
        }

	if ( !vch->in_room )
	    return;

        to = vch->in_room->people;
    }
 
    for( ; to ; to = to->next_in_room )
    {
	if ( (!IS_NPC(to) && to->desc == NULL )
	||   ( IS_NPC(to) && to->desc == NULL && !HAS_TRIGGER_MOB(to, TRIG_ACT) )
	||   to->position < min_pos
        ||   ( type == TO_CHAR && to != ch )
        ||   ( type == TO_VICT && ( to != vch || to == ch ) )
        ||   ( type == TO_ROOM && to == ch )
        ||   ( type == TO_NOTVICT && ( to == ch || to == vch ) ) )
            continue;

	if ( (type == TO_ROOM || type == TO_NOTVICT)
	&&   !IS_NPC(ch) && !IS_NPC(to)
	&&   ch->in_room != NULL && to->in_room != NULL
	&&   IS_SET(ch->in_room->room_flags,ROOM_ARENA)
	&&   IS_SET(to->in_room->room_flags,ROOM_ARENA)
	&&   ch->pcdata->match != to->pcdata->match )
	    continue;
 
	buf[0]	= '\0';	
        point   = buf;
        str     = format;

        while( *str )
        {
            if( *str != '$' && *str != '{' )
            {
		if ( !found )
		    found = TRUE;

                *point++ = *str++;
                continue;
            }

	    i = NULL;
	    switch( *str )
	    {
		case '$':
		    fColour = TRUE;
		    ++str;
		    i = " <@@@> ";
		    if ( !arg2 && *str >= 'A' && *str <= 'Z' && *str != 'G' )
		    {
			bug( "Act: missing arg2 for code %d.", *str );
			i = " <@@@> ";
		    }
		    else
		    {
			switch ( *str )
			{
			    default:
				i = "$";
				--str;
				break;

			    case 't': 
				i = arg1 == NULL ? "$t" : (char *) arg1;
				break;

			    case 'T': 
				i = arg2 == NULL ? "$T" : (char *) arg2;
				break;

			    case 'n': 
				i = ch == NULL ? "$n" : PERS( ch,  to  );
				break;

			    case 'N': 
				i = vch == NULL ? "$N" : PERS( vch, to  );
				break;

			    case 'e': 
				i = ch == NULL ? "$e" :
				  he_she  [URANGE(0, ch  ->sex, 2)];
				break;

			    case 'E': 
				i = vch == NULL ? "$E" :
				  he_she  [URANGE(0, vch ->sex, 2)];
				break;

			    case 'm': 
				i = ch == NULL ? "$m" :
				  him_her [URANGE(0, ch  ->sex, 2)];
				break;

			    case 'M': 
				i = vch == NULL ? "$M" :
				  him_her [URANGE(0, vch ->sex, 2)];
				break;

			    case 's': 
				i = ch == NULL ? "$s" :
				  his_her [URANGE(0, ch  ->sex, 2)];
				break;

			    case 'S': 
				i = vch == NULL ? "$S" :
				  his_her [URANGE(0, vch ->sex, 2)];
				break;
 
			    case 'p':
				i = obj1 == NULL ? "$p" :
				  can_see_obj( to, obj1 )
				  ? obj1->short_descr
				  : "something";
				break;
 
			    case 'P':
				i = obj2 == NULL ? "$P" : 
				  can_see_obj( to, obj2 )
				  ? obj2->short_descr
				  : "something";
				break;
 
			    case 'd':
				if ( !arg2 || ((char *) arg2)[0] == '\0' )
				    i = "door";
				else
				{
				    strcpy( fname, arg2 );
				    i = fname;
				}
				break;

			    case 'G':
				if ( ch == NULL )
				    i = "$G";
				else if ( IS_EVIL( ch ) )
				    i = mud_stat.evil_god_string;
				else if ( IS_GOOD( ch ) )
				    i = mud_stat.good_god_string;
				else
				    i = mud_stat.neut_god_string;
				break;

			}
		    }
		    break;

		case '{':
		    fColour = FALSE;
		    ++str;
		    i = NULL;
		    if( *str && IS_SET( to->act, PLR_COLOUR ) )
		    {
			i = colour( *str, to );
			if ( !found )
			{
			    place += strlen( i );
			    found = TRUE;
			}
		    }
		    break;

		default:
		    fColour = FALSE;
		    *point++ = *str++;
		    break;
	    }

            ++str;
	    if( fColour && i )
	    {
		fixed[0] = '\0';
		i2 = fixed;

		if( IS_SET( to->act, PLR_COLOUR ) )
		{
		    for( i2 = fixed ; *i ; i++ )
	            {
			if( *i == '{' )
			{
			    i++;
			    if (*i != '\0')
			    {
				if ( !found )
				{
				    place += strlen( colour( *i, to ) );
				    found = TRUE;
				}

				strcat( fixed, colour( *i, to ) );
				for( i2 = fixed ; *i2 ; i2++ )
				    ;
				continue;
			    }
			}

			if ( !found )
			    found = TRUE;

			*i2 = *i;
			*++i2 = '\0';
		    }
		    *i2 = '\0';
		    i = &fixed[0];
		} else {
		    for( i2 = fixed ; *i ; i++ )
	            {
			if( *i == '{' )
			{
			    i++;
			    if (*i != '\0')
			    {
				if( *i == '-' )
				{
				    *i2 = '~';
				    *++i2 = '\0';
				    continue;
				}

				else if( *i != '{' )
				    continue;
			    }
			}
			*i2 = *i;
			*++i2 = '\0';
		    }			
		    *i2 = '\0';
		    i = &fixed[0];
		}
	    }

	    if( i )
	    {
		while( ( *point = *i ) != '\0' )
		{
		    ++point;
		    ++i;
		}
	    }
        }
 
        *point++	= '\n';
        *point++	= '\r';
        *point		= '\0';

	buf[place] = UPPER( buf[place] );

	if ( to->desc != NULL && to->desc->connected == CON_PLAYING )
	    write_to_buffer( to->desc, buf, point - buf );

	else if ( MOBtrigger )
	    p_act_trigger( buf, to, NULL, NULL, ch, arg1, arg2, TRIG_ACT );

	if ( type == TO_ROOM || type == TO_NOTVICT )
	{
	    OBJ_DATA *obj, *obj_next;
	    CHAR_DATA *tch, *tch_next;

	    point = buf;
	    str = format;

	    while( *str != '\0' )
		*point++ = *str++;

	    *point   = '\0';

	    for( obj = ch->in_room->contents; obj; obj = obj_next )
	    {
		obj_next = obj->next_content;

		if ( HAS_TRIGGER_OBJ( obj, TRIG_ACT ) )
		    p_act_trigger( buf, NULL, obj, NULL, ch, NULL, NULL, TRIG_ACT );
	    }

	    for( tch = ch; tch; tch = tch_next )
	    {
		tch_next = tch->next_in_room;

		for ( obj = tch->carrying; obj; obj = obj_next )
		{
		    obj_next = obj->next_content;

		    if ( HAS_TRIGGER_OBJ( obj, TRIG_ACT ) )
			p_act_trigger( buf, NULL, obj, NULL, ch, NULL, NULL, TRIG_ACT );
		}
	    }

	    if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_ACT ) )
		p_act_trigger( buf, NULL, NULL, ch->in_room, ch, NULL, NULL, TRIG_ACT );
	}
    }

    return;
}

void combat( const char *format, CHAR_DATA *ch, const void *arg1, 
	      const void *arg2, int type, long flag )
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    CHAR_DATA 		*to;
    CHAR_DATA 		*vch = ( CHAR_DATA * ) arg2;
    OBJ_DATA 		*obj1 = ( OBJ_DATA  * ) arg1;
    OBJ_DATA 		*obj2 = ( OBJ_DATA  * ) arg2;
    const 	char 	*str;
    char 		*i;
    char 		*point;
    char 		*i2;
    char 		fixed[ MAX_STRING_LENGTH ];
    char 		buf[ MAX_STRING_LENGTH   ];
    char 		fname[ MAX_INPUT_LENGTH  ];
    bool		fColour = FALSE, found = FALSE;
    int place = 0;

    if ( !format || !*format || !ch || !ch->in_room )
	return;

    to = ch->in_room->people;

    if ( type == TO_VICT )
    {
        if ( !vch )
        {
            bug( "Combat: null vch with TO_VICT.", 0 );
            return;
        }

	if ( !vch->in_room )
	    return;

        to = vch->in_room->people;
    }
 
    for( ; to ; to = to->next_in_room )
    {
	if ( (type == TO_ROOM || type == TO_NOTVICT)
	&&   !IS_NPC(ch) && !IS_NPC(to)
	&&   ch->in_room != NULL && to->in_room != NULL
	&&   IS_SET(ch->in_room->room_flags,ROOM_ARENA)
	&&   IS_SET(to->in_room->room_flags,ROOM_ARENA)
	&&   ch->pcdata->match != to->pcdata->match )
	    continue;
 
	if ( to->desc == NULL
        ||   (type == TO_CHAR && to != ch)
        ||   (type == TO_VICT && (to != vch || to == ch))
        ||   (type == TO_ROOM && (to == ch || to == vch))
        ||   (type == TO_NOTVICT && (to == ch || to == vch))
	||   (to != ch && to != vch && (to->position < POS_RESTING
				    ||  IS_SET(to->combat,COMBAT_OTHER)))
	||   IS_SET(to->combat,COMBAT_ON)
	||   IS_SET(to->combat,flag) )
            continue;
 
	buf[0]	= '\0';
        point   = buf;
        str     = format;

        while( *str )
        {
            if( *str != '$' && *str != '{' )
            {
		if ( !found )
		    found = TRUE;

                *point++ = *str++;
                continue;
            }

	    i = NULL;
	    switch( *str )
	    {
		case '$':
		    fColour = TRUE;
		    ++str;
		    i = " <@@@> ";
		    if ( !arg2 && *str >= 'A' && *str <= 'Z' && *str != 'G' )
		    {
			bug( "Combat: missing arg2 for code %d.", *str );
			i = " <@@@> ";
		    }
		    else
		    {
			switch ( *str )
			{
			    default:
				i = "$";
				--str;
				break;

			    case 't': 
				i = arg1 == NULL ? "$t" : (char *) arg1;
				break;

			    case 'T': 
				i = arg2 == NULL ? "$T" : (char *) arg2;
				break;

			    case 'n': 
				i = ch == NULL ? "$n" : PERS( ch,  to  );
				break;

			    case 'N': 
				i = vch == NULL ? "$N" : PERS( vch, to  );
				break;

			    case 'e': 
				i = ch == NULL ? "$e" :
				  he_she  [URANGE(0, ch  ->sex, 2)];
				break;

			    case 'E': 
				i = vch == NULL ? "$E" :
				  he_she  [URANGE(0, vch ->sex, 2)];
				break;

			    case 'm': 
				i = ch == NULL ? "$m" :
				  him_her [URANGE(0, ch  ->sex, 2)];
				break;

			    case 'M': 
				i = vch == NULL ? "$M" :
				  him_her [URANGE(0, vch ->sex, 2)];
				break;

			    case 's': 
				i = ch == NULL ? "$s" :
				  his_her [URANGE(0, ch  ->sex, 2)];
				break;

			    case 'S': 
				i = vch == NULL ? "$S" :
				  his_her [URANGE(0, vch ->sex, 2)];
				break;
 
			    case 'p':
				i = obj1 == NULL ? "$p" :
				  can_see_obj( to, obj1 )
				  ? obj1->short_descr
				  : "something";
				break;
 
			    case 'P':
				i = obj2 == NULL ? "$P" : 
				  can_see_obj( to, obj2 )
				  ? obj2->short_descr
				  : "something";
				break;
 
			    case 'd':
				if ( !arg2 || ((char *) arg2)[0] == '\0' )
				{
				    i = "door";
				}
				else
				{
				    strcpy( fname, arg2 );
				    i = fname;
				}
				break;

			    case 'G':
				if ( ch == NULL )
				    i = "$G";
				else if ( IS_EVIL( ch ) )
				    i = mud_stat.evil_god_string;
				else if ( IS_GOOD( ch ) ) 
				    i = mud_stat.good_god_string;
				else
				    i = mud_stat.neut_god_string;
				break;
			}
		    }
		    break;

		case '{':
		    fColour = FALSE;
		    ++str;
		    i = NULL;
		    if( IS_SET( to->act, PLR_COLOUR ) )
		    {
			i = colour( *str, to );
			if ( !found )
			{
			    place += strlen( i );
			    found = TRUE;
			}
		    }
		    break;

		default:
		    fColour = FALSE;
		    *point++ = *str++;
		    break;
	    }

            ++str;
	    if( fColour && i )
	    {
		fixed[0] = '\0';
		i2 = fixed;

		if( IS_SET( to->act, PLR_COLOUR ) )
		{
		    for( i2 = fixed ; *i ; i++ )
	            {
			if( *i == '{' )
			{
			    i++;
			    if (*i != '\0')
			    {
				if ( !found )
				{
				    place += strlen( colour( *i, to ) );
				    found = TRUE;
				}

				strcat( fixed, colour( *i, to ) );
				for( i2 = fixed ; *i2 ; i2++ )
				    ;
				continue;
			    }
			}

			if ( !found )
			    found = TRUE;

			*i2 = *i;
			*++i2 = '\0';
		    }
		    *i2 = '\0';
		    i = &fixed[0];
		} else {
		    for( i2 = fixed ; *i ; i++ )
	            {
			if( *i == '{' )
			{
			    i++;
			    if (*i != '\0')
			    {
				if( *i == '-' )
				{
				    *i2 = '~';
				    *++i2 = '\0';
				    continue;
				}

				else if( *i != '{' )
				    continue;
			    }
			}
			*i2 = *i;
			*++i2 = '\0';
		    }			
		    *i2 = '\0';
		    i = &fixed[0];
		}
	    }

	    if( i )
	    {
		while( ( *point = *i ) != '\0' )
		{
		    ++point;
		    ++i;
		}
	    }
        }
 
        *point++	= '\n';
        *point++	= '\r';
        *point		= '\0';

	buf[place] = UPPER( buf[place] );

	if ( to->desc != NULL && (to->desc->connected == CON_PLAYING))
	    write_to_buffer( to->desc, buf, point - buf );
	else if ( MOBtrigger )
	    p_act_trigger( buf, to, NULL, NULL, ch, arg1, arg2, TRIG_ACT );

	if ( type == TO_ROOM || type == TO_NOTVICT )
	{
	    OBJ_DATA *obj, *obj_next;
	    CHAR_DATA *tch, *tch_next;

	    point = buf;
	    str = format;

	    while( *str != '\0' )
		*point++ = *str++;

	    *point   = '\0';

	    for( obj = ch->in_room->contents; obj; obj = obj_next )
	    {
		obj_next = obj->next_content;

		if ( HAS_TRIGGER_OBJ( obj, TRIG_ACT ) )
		    p_act_trigger( buf, NULL, obj, NULL, ch, NULL, NULL, TRIG_ACT );
	    }

	    for( tch = ch; tch; tch = tch_next )
	    {
		tch_next = tch->next_in_room;

		for ( obj = tch->carrying; obj; obj = obj_next )
		{
		    obj_next = obj->next_content;

		    if ( HAS_TRIGGER_OBJ( obj, TRIG_ACT ) )
			p_act_trigger( buf, NULL, obj, NULL, ch, NULL, NULL, TRIG_ACT );
		}
	    }

	    if ( HAS_TRIGGER_ROOM( ch->in_room, TRIG_ACT ) )
		p_act_trigger( buf, NULL, NULL, ch->in_room, ch, NULL, NULL, TRIG_ACT );
	}
    }

    return;
}

char *colour_clear( CHAR_DATA *ch )
{
    if (ch->pcdata && ch->pcdata->color)
    {
	if (ch->pcdata->color == 1)
	    sprintf( clcode, R_RED );
	else if (ch->pcdata->color == 2)
	    sprintf( clcode, R_GREEN );
	else if (ch->pcdata->color == 3)
	    sprintf( clcode, R_YELLOW );
	else if (ch->pcdata->color == 4)
	    sprintf( clcode, R_BLUE );
	else if (ch->pcdata->color == 5)
	    sprintf( clcode, R_MAGENTA );
	else if (ch->pcdata->color == 6)
	    sprintf( clcode, R_CYAN );
	else if (ch->pcdata->color == 7)
	    sprintf( clcode, R_WHITE );
	else if (ch->pcdata->color == 8)
	    sprintf( clcode, R_D_GREY );
	else if (ch->pcdata->color == 9)
	    sprintf( clcode, R_B_RED );
	else if (ch->pcdata->color == 10)
	    sprintf( clcode, R_B_GREEN );
	else if (ch->pcdata->color == 11)
	    sprintf( clcode, R_B_YELLOW );
	else if (ch->pcdata->color == 12)
	    sprintf( clcode, R_B_BLUE );
	else if (ch->pcdata->color == 13)
	    sprintf( clcode, R_B_MAGENTA );
	else if (ch->pcdata->color == 14)
	    sprintf( clcode, R_B_CYAN );
	else if (ch->pcdata->color == 15)
	    sprintf( clcode, R_B_WHITE );
	else if (ch->pcdata->color == 16)
	    sprintf( clcode, R_BLACK );
	else
	    sprintf( clcode, CLEAR );
    }
    else
    {
	sprintf( clcode, CLEAR );
    }
    return clcode;
}

char *colour_channel( int colornum, CHAR_DATA *ch )
{
    if (colornum == 1)
	sprintf( clcode, C_RED );
    else if (colornum == 2)
	sprintf( clcode, C_GREEN );
    else if (colornum == 3)
	sprintf( clcode, C_YELLOW );
    else if (colornum == 4)
	sprintf( clcode, C_BLUE );
    else if (colornum == 5)
	sprintf( clcode, C_MAGENTA );
    else if (colornum == 6)
	sprintf( clcode, C_CYAN );
    else if (colornum == 7)
	sprintf( clcode, C_WHITE );
    else if (colornum == 8)
	sprintf( clcode, C_D_GREY );
    else if (colornum == 9)
	sprintf( clcode, C_B_RED );
    else if (colornum == 10)
	sprintf( clcode, C_B_GREEN );
    else if (colornum == 11)
	sprintf( clcode, C_B_YELLOW );
    else if (colornum == 12)
	sprintf( clcode, C_B_BLUE );
    else if (colornum == 13)
	sprintf( clcode, C_B_MAGENTA );
    else if (colornum == 14)
	sprintf( clcode, C_B_CYAN );
    else if (colornum == 15)
	sprintf( clcode, C_B_WHITE );
    else if (colornum == 16)
	sprintf( clcode, C_BLACK );
    else
	sprintf( clcode, colour_clear( ch ));

    return clcode;
}

char * channel( char *arg, char *backup )
{
    int pos = channel_lookup( arg );

    if ( pos == -1 )
	return backup;

    return channel_table[pos].color_default;
}

char *colour( char type, CHAR_DATA *vch )
{
    CHAR_DATA *ch = vch->desc && vch->desc->original ? vch->desc->original : vch;

    int number;

    if( IS_NPC( ch ) && ch->desc == NULL )
	return ( "" );

    switch( type )
    {
	default:
	case 'x':
        case '0':
	    sprintf( clcode, colour_clear( ch ) );
	    break;
	case 'z':
	    sprintf( clcode, BLINK );
	    break;
	case '+':
	    sprintf( clcode, "\e[4m" );
	    break;
	case '=':
	    sprintf( clcode, "\e[41m" );
	    break;
	case '(':
	    sprintf( clcode, "\n\r" );
	    break;
	case 'b':
        case '4':
            sprintf( clcode, C_BLUE );
            break;
	case 'c':
        case '6':
            sprintf( clcode, C_CYAN );
            break;
	case 'g':
        case '2':
            sprintf( clcode, C_GREEN );
            break;
	case 'm':
        case '5':
            sprintf( clcode, C_MAGENTA );
            break;
	case 'r':
        case '1':
            sprintf( clcode, C_RED );
            break;
	case 'w':
        case '7':
            sprintf( clcode, C_WHITE );
            break;
	case 'y':
        case '3':
            sprintf( clcode, C_YELLOW );
            break;
	case 'B':
        case '$':
            sprintf( clcode, C_B_BLUE );
            break;
	case 'C':
        case '^':
            sprintf( clcode, C_B_CYAN );
            break;
	case 'G':
        case '@':
            sprintf( clcode, C_B_GREEN );
            break;
	case 'M':
        case '%':
            sprintf( clcode, C_B_MAGENTA );
            break;
	case 'R':
        case '!':
            sprintf( clcode, C_B_RED );
            break;
	case 'W':
        case '&':
            sprintf( clcode, C_B_WHITE );
            break;
	case 'Y':
        case '#':
            sprintf( clcode, C_B_YELLOW );
            break;
	case 'D':
        case '8':
            sprintf( clcode, C_D_GREY );
            break;
        case '*':
	    sprintf( clcode, C_BLACK );
	    break;
	case '-':
	    sprintf( clcode, "~" );
	    break;
	case '?':
	    number = number_range(0,14);
	    if (number == 0)
		sprintf( clcode, C_RED );
	    else if (number == 1)
		sprintf( clcode, C_GREEN );
	    else if (number == 2)
		sprintf( clcode, C_YELLOW );
	    else if (number == 3)
		sprintf( clcode, C_BLUE );
	    else if (number == 4)
		sprintf( clcode, C_MAGENTA );
	    else if (number == 5)
		sprintf( clcode, C_CYAN );
	    else if (number == 6)
		sprintf( clcode, C_WHITE );
	    else if (number == 7)
		sprintf( clcode, C_D_GREY );
	    else if (number == 8)
		sprintf( clcode, C_B_RED );
	    else if (number == 9)
		sprintf( clcode, C_B_GREEN );
	    else if (number == 10)
		sprintf( clcode, C_B_YELLOW );
	    else if (number == 11)
		sprintf( clcode, C_B_BLUE );
	    else if (number == 12)
		sprintf( clcode, C_B_MAGENTA );
	    else if (number == 13)
		sprintf( clcode, C_B_CYAN );
	    else
		sprintf( clcode, C_B_WHITE );
	    break;
	case 'A':	/* Auction Channel */
	    if (ch->pcdata && ch->pcdata->color_auc)
		sprintf( clcode, colour_channel(ch->pcdata->color_auc, ch));
	    else
		sprintf( clcode, channel( "barter", C_B_BLUE ) );
	    break;
	case 'E':	/* Clan Gossip Channel */
	    if (ch->pcdata && ch->pcdata->color_cgo)
		sprintf( clcode, colour_channel(ch->pcdata->color_cgo, ch));
	    else
		sprintf( clcode, channel( "clangossip", C_B_BLUE ) );
	    break;
	case 'F':	/* Clan Talk Channel */
	    if (ch->pcdata && ch->pcdata->color_cla)
		sprintf( clcode, colour_channel(ch->pcdata->color_cla, ch));
	    else
		sprintf( clcode, C_B_MAGENTA );
	    break;
	case 'H':	/* Gossip Channel */
	    if (ch->pcdata && ch->pcdata->color_gos)
		sprintf( clcode, colour_channel(ch->pcdata->color_gos, ch));
	    else
		sprintf( clcode, channel( "gossip", C_B_BLUE ) );
	    break;
	case 'J':	/* Grats Channel */
	    if (ch->pcdata && ch->pcdata->color_gra)
		sprintf( clcode, colour_channel(ch->pcdata->color_gra, ch));
	    else
		sprintf( clcode, channel( "grats", C_YELLOW ) );
	    break;
	case 'K':	/* Group Tell Channel */
	    if (ch->pcdata && ch->pcdata->color_gte)
		sprintf( clcode, colour_channel(ch->pcdata->color_gte, ch));
	    else
		sprintf( clcode, C_CYAN );
	    break;
	case 'L':	/* Immortal Talk Channel */
	    if (ch->pcdata && ch->pcdata->color_imm)
		sprintf( clcode, colour_channel(ch->pcdata->color_imm, ch));
	    else
		sprintf( clcode, channel( "immtalk", C_B_RED ) );
	    break;
	case 'P':	/* Question+Answer Channel */
	    if (ch->pcdata && ch->pcdata->color_que)
		sprintf( clcode, colour_channel(ch->pcdata->color_que, ch));
	    else
		sprintf( clcode, channel( "answer", C_B_YELLOW ) );
	    break;
	case 'Q':	/* Quote Channel */
	    if (ch->pcdata && ch->pcdata->color_quo)
		sprintf( clcode, colour_channel(ch->pcdata->color_quo, ch));
	    else
		sprintf( clcode, channel( "quote", C_GREEN ) );
	    break;
	case 'S':	/* Say Channel */
	    if (ch->pcdata && ch->pcdata->color_say)
		sprintf( clcode, colour_channel(ch->pcdata->color_say, ch));
	    else
		sprintf( clcode, C_MAGENTA );
	    break;
	case 'T':	/* Shout+Yell Channel */
	    if (ch->pcdata && ch->pcdata->color_sho)
		sprintf( clcode, colour_channel(ch->pcdata->color_sho, ch));
	    else
		sprintf( clcode, C_RED );
	    break;
	case 'U':	/* Tell+Reply Channel */
	    if (ch->pcdata && ch->pcdata->color_tel)
		sprintf( clcode, colour_channel(ch->pcdata->color_tel, ch));
	    else
		sprintf( clcode, C_CYAN );
	    break;
	case 'V':	/* Wiznet Messages */
	    if (ch->pcdata && ch->pcdata->color_wiz)
		sprintf( clcode, colour_channel(ch->pcdata->color_wiz, ch));
	    else
		sprintf( clcode, C_WHITE );
	    break;
	case 'a':	/* Mobile Talk */
	    if (ch->pcdata && ch->pcdata->color_mob)
		sprintf( clcode, colour_channel(ch->pcdata->color_mob, ch));
	    else
		sprintf( clcode, C_MAGENTA );
	    break;
	case 'd':	/* Chat Channel */
	    if (ch->pcdata && ch->pcdata->color_cht)
		sprintf( clcode, colour_channel(ch->pcdata->color_cht, ch));
	    else
		sprintf( clcode, C_CYAN );
	    break;
	case 'e':	/* Room Title */
	    if (ch->pcdata && ch->pcdata->color_roo)
		sprintf( clcode, colour_channel(ch->pcdata->color_roo, ch));
	    else
		sprintf( clcode, C_B_BLUE );
	    break;
	case 'f':	/* Opponent Condition */
	    if (ch->pcdata && ch->pcdata->color_con)
		sprintf( clcode, colour_channel(ch->pcdata->color_con, ch));
	    else
		sprintf( clcode, C_B_RED );
	    break;
	case 'h':	/* Fight Actions */
	    if (ch->pcdata && ch->pcdata->color_fig)
		sprintf( clcode, colour_channel(ch->pcdata->color_fig, ch));
	    else
		sprintf( clcode, C_B_BLUE );
	    break;
	case 'i':	/* Opponents Fight Actions */
	    if (ch->pcdata && ch->pcdata->color_opp)
		sprintf( clcode, colour_channel(ch->pcdata->color_opp, ch));
	    else
		sprintf( clcode, C_CYAN );
	    break;
	case 'j':	/* Disarm Messages */
	    if (ch->pcdata && ch->pcdata->color_dis)
		sprintf( clcode, colour_channel(ch->pcdata->color_dis, ch));
	    else
		sprintf( clcode, C_B_YELLOW );
	    break;
	case 'k':	/* Witness Messages */
	    if (ch->pcdata && ch->pcdata->color_wit)
		sprintf( clcode, colour_channel(ch->pcdata->color_wit, ch));
	    else
		sprintf( clcode, colour_clear( ch ));
	    break;
	case 'l':	/* Quest Gossip */
            if (ch->pcdata && ch->pcdata->color_qgo)
                sprintf( clcode, colour_channel(ch->pcdata->color_qgo, ch));
            else
                sprintf( clcode, channel( "questgossip", C_B_BLUE ) );
            break;
        case 'n':       /* OOC */
            if (ch->pcdata && ch->pcdata->color_ooc)
                sprintf( clcode, colour_channel(ch->pcdata->color_ooc, ch));
            else
                sprintf( clcode, channel( "ooc", C_B_CYAN ) );
            break;
        case 'o':       /* RACE */
            if (ch->pcdata && ch->pcdata->color_rac)
                sprintf( clcode, colour_channel(ch->pcdata->color_rac, ch));
            else
                sprintf( clcode, channel( "race", C_B_BLUE ) );
            break;
        case 'p':       /* FLAME */
            if (ch->pcdata && ch->pcdata->color_fla)
                sprintf( clcode, colour_channel(ch->pcdata->color_fla, ch));
            else
                sprintf( clcode, channel( "flame", C_B_RED ) );
            break;
	case 'q':
            if (ch->pcdata && ch->pcdata->color_olc1)
                sprintf( clcode, colour_channel(ch->pcdata->color_olc1, ch));
            else
                sprintf( clcode, C_YELLOW);
	    break;
	case 's':
            if (ch->pcdata && ch->pcdata->color_olc2)
                sprintf( clcode, colour_channel(ch->pcdata->color_olc2, ch));
            else
                sprintf( clcode, C_B_BLUE );
	    break;
	case 't':
            if (ch->pcdata && ch->pcdata->color_olc3)
                sprintf( clcode, colour_channel(ch->pcdata->color_olc3, ch));
            else
                sprintf( clcode, C_WHITE );
	    break;
        case 'u':       /* HERO */
            if (ch->pcdata && ch->pcdata->color_her)
                sprintf( clcode, colour_channel(ch->pcdata->color_her, ch));
            else
                sprintf( clcode, channel( "herotalk", C_GREEN ) );
            break;
        case 'v':       /* IC */
            if (ch->pcdata && ch->pcdata->color_ic)
                sprintf( clcode, colour_channel(ch->pcdata->color_ic, ch));
            else
                sprintf( clcode, channel( "ic", C_YELLOW ) );
            break;
        case '9':
            if (ch->pcdata && ch->pcdata->color_pra)
                sprintf( clcode, colour_channel(ch->pcdata->color_pra, ch));
            else
                sprintf( clcode, C_B_RED);
            break;
	case '{':
	    sprintf( clcode, "%c", '{' );
	    break;
    }
    return clcode;
}

char *colour2( char type )
{
    int number;

    switch( type )
    {
	default:
	case 'x':
        case '0':
	    sprintf( clcode, CLEAR );
	    break;
	case 'z':
	    sprintf( clcode, BLINK );
	    break;
	case '+':
	    sprintf( clcode, "\e[4m" );
	    break;
	case '=':
	    sprintf( clcode, "\e[7;1;33m" );
	    break;
	case '(':
	    sprintf( clcode, "\n\r" );
	    break;
	case 'b':
        case '4':
            sprintf( clcode, C_BLUE );
            break;
	case 'c':
        case '6':
            sprintf( clcode, C_CYAN );
            break;
	case 'g':
        case '2':
            sprintf( clcode, C_GREEN );
            break;
	case 'm':
        case '5':
            sprintf( clcode, C_MAGENTA );
            break;
	case 'r':
        case '1':
            sprintf( clcode, C_RED );
            break;
	case 'w':
        case '7':
            sprintf( clcode, C_WHITE );
            break;
	case 'y':
        case '3':
            sprintf( clcode, C_YELLOW );
            break;
	case 'B':
        case '$':
            sprintf( clcode, C_B_BLUE );
            break;
	case 'C':
        case '^':
            sprintf( clcode, C_B_CYAN );
            break;
	case 'G':
        case '@':
            sprintf( clcode, C_B_GREEN );
            break;
	case 'M':
        case '%':
            sprintf( clcode, C_B_MAGENTA );
            break;
	case 'R':
        case '!':
            sprintf( clcode, C_B_RED );
            break;
	case 'W':
        case '&':
            sprintf( clcode, C_B_WHITE );
            break;
	case 'Y':
        case '#':
            sprintf( clcode, C_B_YELLOW );
            break;
	case 'D':
        case '8':
            sprintf( clcode, C_D_GREY );
            break;
        case '*':
	    sprintf( clcode, C_BLACK );
	    break;
	case '-':
	    sprintf( clcode, "~" );
	    break;
	case '?':
	    number = number_range(0,14);
	    if (number == 0)
		sprintf( clcode, C_RED );
	    else if (number == 1)
		sprintf( clcode, C_GREEN );
	    else if (number == 2)
		sprintf( clcode, C_YELLOW );
	    else if (number == 3)
		sprintf( clcode, C_BLUE );
	    else if (number == 4)
		sprintf( clcode, C_MAGENTA );
	    else if (number == 5)
		sprintf( clcode, C_CYAN );
	    else if (number == 6)
		sprintf( clcode, C_WHITE );
	    else if (number == 7)
		sprintf( clcode, C_D_GREY );
	    else if (number == 8)
		sprintf( clcode, C_B_RED );
	    else if (number == 9)
		sprintf( clcode, C_B_GREEN );
	    else if (number == 10)
		sprintf( clcode, C_B_YELLOW );
	    else if (number == 11)
		sprintf( clcode, C_B_BLUE );
	    else if (number == 12)
		sprintf( clcode, C_B_MAGENTA );
	    else if (number == 13)
		sprintf( clcode, C_B_CYAN );
	    else
		sprintf( clcode, C_B_WHITE );
	    break;
	case 'A':	/* Auction Channel */
	    sprintf( clcode, channel( "barter", C_B_BLUE ) );
	    break;
	case 'E':	/* Clan Gossip Channel */
	    sprintf( clcode, channel( "clangossip", C_B_BLUE ) );
	    break;
	case 'F':	/* Clan Talk Channel */
	    sprintf( clcode, C_B_MAGENTA );
	    break;
	case 'H':	/* Gossip Channel */
	    sprintf( clcode, channel( "gossip", C_B_BLUE ) );
	    break;
	case 'J':	/* Grats Channel */
	    sprintf( clcode, channel( "grats", C_YELLOW ) );
	    break;
	case 'K':	/* Group Tell Channel */
	    sprintf( clcode, C_CYAN );
	    break;
	case 'L':	/* Immortal Talk Channel */
	    sprintf( clcode, channel( "immtalk", C_B_RED ) );
	    break;
	case 'P':	/* Question+Answer Channel */
	    sprintf( clcode, channel( "answer", C_B_YELLOW ) );
	    break;
	case 'Q':	/* Quote Channel */
	    sprintf( clcode, channel( "quote", C_GREEN ) );
	    break;
	case 'S':	/* Say Channel */
	    sprintf( clcode, C_MAGENTA );
	    break;
	case 'T':	/* Shout+Yell Channel */
	    sprintf( clcode, C_RED );
	    break;
	case 'U':	/* Tell+Reply Channel */
	    sprintf( clcode, C_CYAN );
	    break;
	case 'V':	/* Wiznet Messages */
	    sprintf( clcode, C_WHITE );
	    break;
	case 'a':	/* Mobile Talk */
	    sprintf( clcode, C_MAGENTA );
	    break;
	case 'd':	/* Chat Channel */
	    sprintf( clcode, C_CYAN );
	    break;
	case 'e':	/* Room Title */
	    sprintf( clcode, C_B_BLUE );
	    break;
	case 'f':	/* Opponent Condition */
	    sprintf( clcode, C_B_RED );
	    break;
	case 'h':	/* Fight Actions */
	    sprintf( clcode, C_B_BLUE );
	    break;
	case 'i':	/* Opponents Fight Actions */
	    sprintf( clcode, C_CYAN );
	    break;
	case 'j':	/* Disarm Messages */
	    sprintf( clcode, C_B_YELLOW );
	    break;
	case 'k':	/* Witness Messages */
	    sprintf( clcode, CLEAR );
	    break;
	case 'l':	/* Quest Gossip */
	    sprintf( clcode, channel( "questgossip", C_B_BLUE ) );
            break;
        case 'n':       /* OOC */
	    sprintf( clcode, channel( "ooc", C_B_CYAN ) );
            break;
        case 'o':       /* RACE */
	    sprintf( clcode, channel( "race", C_B_BLUE ) );
            break;
        case 'p':       /* FLAME */
	    sprintf( clcode, channel( "flame", C_B_RED ) );
            break;
	case 'q':
	    sprintf( clcode, C_YELLOW);
	    break;
	case 's':
	    sprintf( clcode, C_B_BLUE );
	    break;
	case 't':
	    sprintf( clcode, C_WHITE );
	    break;
        case 'u':       /* HERO */
	    sprintf( clcode, channel( "herotalk", C_GREEN ) );
            break;
        case 'v':       /* IC */
	    sprintf( clcode, channel( "ic", C_YELLOW ) );
            break;
        case '9':
	    sprintf( clcode, C_B_RED);
            break;
	case '{':
	    sprintf( clcode, "%c", '{' );
	    break;
    }

    return clcode;
}

void do_copyover (CHAR_DATA *ch, char * argument)
{
    FILE *fp;
    DESCRIPTOR_DATA *d, *d_next;
    char buf [100], buf2[100];
    extern int port, control;

    prepare_reboot( );

    fp = fopen (COPYOVER_FILE, "w");

    if (!fp)
    {
	send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
	perror ("do_copyover:fopen");
	return;
    }

    for (d = descriptor_list; d != NULL; d = d_next)
    {
	CHAR_DATA * och = d->original ? d->original : d->character;
	d_next = d->next;

	if (!d->character || d->connected > CON_PLAYING)
	{
	    send_to_desc("\n\rThe mud is rebooting, try again in a few minutes.\n\r",d);
	    close_socket (d);
	} else { 
	    fprintf( fp, "%d %d %s %s %s\n", d->descriptor, d->out_compress ? 1 : 0, och->name, d->host, d->hostip );
	    save_char_obj(och,0);
	    sprintf( buf,"\n\r{g*{G*{g* {DCOPYOVER {r-{R-{r-{R%s{r-{R-{r- {DCOPYOVER {g*{G*{g*{x\n\r",ch->name);
	    write_to_desc(buf,d);
	}

	compressEnd( d );

    }

    fprintf (fp, "-1\n");
    fclose (fp);

    sprintf (buf,  "%d", port);
    sprintf (buf2, "%d", control);

    if (port == MAIN_GAME_PORT)
       
	execl(EXE_FILE,"rot", buf, buf2, (char *) NULL);
    else
	execl("../src/rot","../src/rot", buf, buf2, (char *) NULL);

    perror ("do_copyover: execl");
    send_to_char ("Copyover FAILED!\n\r",ch);
}

void copyover_recover ()
{
    DESCRIPTOR_DATA *d;
    FILE *fp;
    char name [100];
    char host[MAX_STRING_LENGTH];
    char hostip[MAX_STRING_LENGTH];
    int desc, compress;
    bool fOld;

    if ( (fp = fopen(COPYOVER_FILE, "r")) == NULL )
    {
	perror ("copyover_recover:fopen");
	exit( 1 );
    }

    for ( ; ; )
    {
	fscanf (fp, "%d %d %s %s %s\n", &desc, &compress, name, host, hostip);

	if (desc == -1)
	    break;

 	d = new_descriptor();
	d->descriptor = desc;

	if ( compress )
	    compressStart( d );

	if ( !write_to_desc( "\n\r{RRestoring data...{x\n\r", d ) )
	{
	    close (desc);
	    continue;
	}

	d->host = str_dup (host);
	d->hostip = str_dup(hostip);
	d->next = descriptor_list;
	descriptor_list = d;
	d->connected = CON_COPYOVER_RECOVER;

	fOld = load_char_obj (d, name, FALSE, FALSE);

	if (!fOld)
	{
	    write_to_descriptor (d, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
	    close_socket (d);
	} else {
//	    write_to_buffer( d, compress_will, 0 );
	    send_to_char( "{RData restoration complete.{x\n\r\n\r", d->character );
	    if (!d->character->in_room)
		d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);

	    d->character->next = char_list;
	    char_list = d->character;
	    d->character->pcdata->next_player = player_list;
	    player_list = d->character;

	    mud_stat.connect_since_reboot++;

	    free_string( d->character->pcdata->socket );
	    d->character->pcdata->socket = str_dup( d->host );
	    char_to_room (d->character, d->character->in_room);
	    do_look (d->character, "auto");
	    act ("$n rejoins $t.",
		d->character, mud_stat.mud_name_string, NULL, TO_ROOM, POS_RESTING);
	    d->connected = CON_PLAYING;
	    if (d->character->pcdata->pnote != NULL)
		send_to_char("{YYou have a note in progress!{x\n\r",d->character);
	    racial_spells(d->character,TRUE);
	    do_devote_assign( d->character );
	    if (d->character->pet != NULL)
	    {
		char_to_room(d->character->pet,d->character->in_room);
		act("$n rejoins $t.",
		    d->character->pet,mud_stat.mud_name_string,NULL,TO_ROOM,POS_RESTING);
	    }
	}
    }
    fclose (fp);
    unlink (COPYOVER_FILE);
}

void printf_to_char (CHAR_DATA *ch, char *fmt, ...)
{
    char buf [MAX_STRING_LENGTH];
    va_list args;
    va_start (args, fmt);
    vsnprintf (buf, MAX_STRING_LENGTH, fmt, args);
    va_end (args);
	
    send_to_char (buf, ch);
    return;
}

void do_test( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    if ( !IS_TRUSTED( ch, MAX_LEVEL+5 ) )
    {
	send_to_char( "Restricted command.\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "clear_list" ) )
    {
	DESC_CHECK_DATA *dc, *dc_next;

	for ( dc = desc_check_list; dc != NULL; dc = dc_next )
	{
	    dc_next = dc->next;

	    free_desc_check( dc );
	}

	desc_check_list = NULL;
	send_to_char( "Desc_check_list cleared.\n\r", ch );
    }

    else if ( !str_cmp( arg, "object_balance" ) )
    {
	OBJ_INDEX_DATA *pObj;
	int vnum, match = 0;

	for ( vnum = 0; match < top_obj_index; vnum++ )
	{
	    if ( ( pObj = get_obj_index( vnum ) ) != NULL )
	    {
		match++;
		object_balance( pObj, pObj->level );
	    }
	}

    }

    else if ( !str_cmp( arg, "find_eq" ) )
    {
	AFFECT_DATA *paf, *paf_next, *prev;
	OBJ_INDEX_DATA *pObjIndex;
	char buf[MAX_STRING_LENGTH];
	int nMatch = 0, pos = 0, vnum;

	for ( vnum = 0; nMatch < top_obj_index; vnum++ )
	{
	    if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	    {
		nMatch++;

		for ( paf = pObjIndex->affected; paf; paf = paf_next )
		{
		    paf_next = paf->next;

		    if ( paf->where == TO_DAM_MODS )
			continue;

		    switch( paf->location )
		    {
			case APPLY_NONE:
			case APPLY_STR:
			case APPLY_DEX:
			case APPLY_INT:
			case APPLY_WIS:
			case APPLY_CON:
			case APPLY_SEX:
			case APPLY_MANA:
			case APPLY_HIT:
			case APPLY_MOVE:
			case APPLY_AC:
			case APPLY_HITROLL:
			case APPLY_DAMROLL:
			case APPLY_SAVES:
			    break;

			default:
			    pos++;
			    sprintf( buf, "%3d) [%5d] (%2d) %s\n\r",
				pos, pObjIndex->vnum,
				paf->location, pObjIndex->short_descr );
			    send_to_char( buf, ch );

			    if ( pObjIndex->affected == paf )
				pObjIndex->affected = paf->next;
			    else
			    {
				for ( prev = pObjIndex->affected; prev; prev = prev->next )
				{
				    if ( prev->next == paf )
				    {
					prev->next = paf->next;
					break;
				    }
				}
			    }
			    free_affect( paf );
			    break;
		    }
		}
	    }
	}
    }

    else if ( !str_cmp( arg, "list" ) )
    {
	DESC_CHECK_DATA *dc;
	char buf[MAX_STRING_LENGTH];	

	if ( desc_check_list == NULL )
	    send_to_char( "No current desc_check_list.\n\r", ch );
	else
	{
	    for ( dc = desc_check_list; dc != NULL; dc = dc->next )
	    {
		sprintf( buf, "Name: %s | Host: %s | Time: %d\n",
		    dc->name, dc->host, dc->wait_time );
		send_to_char( buf, ch );
	    }
	}
    }

    else if ( !str_cmp( arg, "mob_clear" ) )
    {
	MOB_INDEX_DATA *pMob;
	int found = 0, vnum;

	for ( vnum = 0; found < top_mob_index; vnum++ )
	{
	    if ( ( pMob = get_mob_index( vnum ) ) != NULL )
	    {
		found++;
		pMob->perm_mob_pc_deaths[0] = 0;
		pMob->perm_mob_pc_deaths[1] = 0;
	    }
	}

	send_to_char( "All mob kill and death info cleared.\n\r", ch );
    }

    else if ( !str_cmp( arg, "crash" ) )
    {
	send_to_char( ch->pIndexData->player_name, ch );
    }

    else if ( !str_cmp( arg, "pfile_fix" ) )
    {
	DESCRIPTOR_DATA *d;
	CHAR_DATA *wch;
	FILE *list;
	char name[50];
	sh_int count = 0;

	log_string( "Initializing automated pfile re-write!" );

	system( "dir -1 ../player/*/* >../player/names.txt" );

	if ( ( list = fopen( "../player/names.txt", "a" ) ) != NULL )
	{
	    fprintf( list, "#\n\r" );
	    fclose( list );
	}

	if ( ( list = fopen( "../player/names.txt", "r" ) ) != NULL )
	{
	    for ( ; ; )
	    {
		strcpy( name, fread_word( list ) );

		if ( name[0] == '#' )
		    break;

		d = new_descriptor( );
		load_char_obj( d, name+12, FALSE, FALSE );
		wch = d->character;
		free_descriptor( d );
		wch->desc = NULL;
		count++;

		save_char_obj( wch, 2 );

		free_char( wch );
	    }

	    fclose( list );
	}

	unlink( "../player/names.txt" );

	sprintf( log_buf, "Pfile fix complete, %d files found.\n\r", count );
	send_to_char( log_buf, ch );

	log_string( "Automated pfile re-write completed!" );
    }

    else if ( !str_cmp( arg, "rank" ) )
    {
	DESCRIPTOR_DATA *d;
	CHAR_DATA *wch;
	FILE *list;
	char name[50];
	sh_int count = 0;

	log_string( "Initializing automated chart ranks." );

	system( "dir -1 ../player/*/* >../player/names.txt" );

	if ( ( list = fopen( "../player/names.txt", "a" ) ) != NULL )
	{
	    fprintf( list, "#\n\r" );
	    fclose( list );
	}

	if ( ( list = fopen( "../player/names.txt", "r" ) ) != NULL )
	{
	    for ( ; ; )
	    {
		strcpy( name, fread_word( list ) );

		if ( name[0] == '#' )
		    break;

		d = new_descriptor( );
		load_char_obj( d, name+12, FALSE, FALSE );
		wch	  = d->character;
		free_descriptor( d );
		wch->desc = NULL;
		count++;

		update_chart_ranks( wch );

		free_char( wch );
	    }

	    fclose( list );
	}

	unlink( "../player/names.txt" );

	sprintf( log_buf, "Chart ranking complete, %d files found.\n\r", count );
	send_to_char( log_buf, ch );

	log_string( "Automated chart ranking completed!" );
    }

    else if ( !str_cmp( arg, "pfile_wipe" ) )
    {
	FILE *list, *pfile, *deleted;
	CHAR_DATA *wch;
	char name[50], time[50];
	sh_int dele_count = 0, kept_count = 0;

	log_string( "Initializing automated pfile wipe!" );

	system( "dir -1 ../player/*/* >../player/names.txt" );

	if ( ( list = fopen( "../player/names.txt", "a" ) ) != NULL )
	{
	    fprintf( list, "#\n\r" );
	    fclose( list );
	}

	if ( ( list = fopen( "../player/names.txt", "r" ) ) != NULL )
	{
	    if ( ( deleted = fopen("../player/deleted.txt", "a" ) ) != NULL )
	    {
		strftime( time, 50, "{%m/%d at %H:%M}", localtime( &current_time ) );
		fprintf( deleted, "### %s ###\n", time );

		for ( ; ; )
		{
		    strcpy( name, fread_word( list ) );

		    if ( name[0] == '#' )
			break;

		    wch		= new_char( );
		    wch->pcdata	= new_pcdata( );

		    if ( ( pfile = fopen( name, "r" ) ) != NULL )
		    {
			for ( ; ; )
			{
			    char *word, letter = fread_letter( pfile );

			    if ( letter == '*' )
			    {
				fread_to_eol( pfile );
				continue;
			    }

			    if ( letter != '#' )
			    {
				bug( "pfile_wipe: # not found.", 0 );
				break;
			    }

			    word = fread_word( pfile );

			    if ( !str_cmp( word, "PLAYER" ) )
				fread_char( wch, pfile, 2 );
			    break;
			}

			fclose( pfile );
		    }

/* 4 months */	    if ( ( wch->level < LEVEL_HERO
		    &&     difftime( current_time, wch->pcdata->llogoff) > 7884000 )
/* 1 year */	    ||   difftime( current_time, wch->pcdata->llogoff ) > 31536000 )
		    {
			char backup[50];

			fprintf( deleted, "Deleting %s.\n", name );
			dele_count++;

			if ( wch->level > LEVEL_HERO )
			    update_wizlist( wch, 1 );

			if ( is_clan( wch ) )
			{
			    update_clanlist( wch, FALSE );
			    check_roster( wch, TRUE );
			}

			unrank_charts( wch );

			sprintf( backup, "../backup/%s", name+10 );
			unlink( backup );
			unlink( name );
		    }
		    else
			kept_count++;

		    free_char( wch );	    
		}

		fprintf( deleted, "\nPfiles Deleted: %d\nPfiles Kept   : %d\n\n\n",
		    dele_count, kept_count );
		fclose( deleted );
	    }

	    fclose( list );
	}

	unlink( "../player/names.txt" );

	log_string( "Automated pfile_wipe completed!" );

	sprintf( log_buf, "Automated pfile_wipe completed!\n\r"
		      "Pfiles Kept   : %d\n\rPfiles Deleted: %d\n\r",
	    kept_count, dele_count );
	send_to_char( log_buf, ch );
    }

    else
	send_to_char( "Invalid test argument.\n\r", ch );
}

bool write_to_descriptor( DESCRIPTOR_DATA *d, char *txt, int length )
{
    int iStart, nWrite, nBlock;

    if ( d == NULL )
	return FALSE;

    if ( d->out_compress )
	return writeCompressed( d, txt, length );

    if ( length <= 0 )
	length = strlen(txt);

    {
	struct timeval tv;

	fd_set write;
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	for ( iStart = 0; iStart < length; iStart += nWrite )
	{
	    FD_ZERO( &write );
	    FD_SET( d->descriptor, &write);

	    if ( select( d->descriptor+1, NULL, &write, NULL, &tv ) < 0 )
		iStart = length+1;

	    nBlock = UMIN( length - iStart, 4096 );
	    if ( ( nWrite = send( d->descriptor, txt + iStart, nBlock, 0 ) ) < 0 )
		{ perror( "Write_to_descriptor" ); return FALSE; }
	}
    } 

    return TRUE;
}

