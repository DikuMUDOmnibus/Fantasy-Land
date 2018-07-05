/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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

extern char str_empty[1];

NOTE_DATA *	new_note	args( ( void ) );
void		free_note	args( ( NOTE_DATA *note ) );

BAN_DATA *	new_ban		args( ( void ) );
void		free_ban	args( ( BAN_DATA *ban ) );

MULTIPLAY_DATA *new_allow	args( ( void ) );
void		free_allow	args( ( MULTIPLAY_DATA *allow ) );

DESC_CHECK_DATA *new_desc_check	args( ( void ) );
void		free_desc_check	args( ( DESC_CHECK_DATA *desc_check ) );

CENSOR_DATA *	new_censor	args( ( void ) );
void		free_censor	args( ( CENSOR_DATA *censor ) );

WIZ_DATA *	new_wiz		args( ( void ) );
void		free_wiz	args( ( WIZ_DATA *ban ) );

DESCRIPTOR_DATA	*new_descriptor	args( ( void ) );
void		free_descriptor	args( ( DESCRIPTOR_DATA *d ) );

GEN_DATA * 	new_gen_data	args( ( void ) );
void		free_gen_data	args( ( GEN_DATA * gen ) );

EXTRA_DESCR_DATA *new_extra_descr args( ( void ) );
void		free_extra_descr args( (EXTRA_DESCR_DATA *ed) );

AFFECT_DATA *	new_affect	args( ( void ) );
void		free_affect	args( ( AFFECT_DATA *af ) );

OBJ_DATA *	new_obj		args( ( void ) );
void		free_obj	args( ( OBJ_DATA *obj ) );

CHAR_DATA *	new_char	args( ( void ) );
void		free_char	args( ( CHAR_DATA *ch ) );

PKILL_DATA *	new_pkill	args( ( void ) );
void		free_pkill	args( ( PKILL_DATA *pkill ) );

PKILL_RECORD *	new_pk_record	args( ( void ) );
void		free_pk_record	args( ( PKILL_RECORD *pkill ) );

PC_DATA *	new_pcdata	args( ( void ) );
void		free_pcdata	args( ( PC_DATA *pcdata ) );

AUCTION_DATA *	new_auction	args( ( void ) );
void		free_auction	args( ( AUCTION_DATA *auction ) );

void		free_help	args( ( HELP_DATA *pHelp ) );
HELP_DATA *	new_help	args( ( void ) );

long		get_pc_id	args( ( void ) );
long		get_mob_id	args( ( void ) );
MEM_DATA *	new_mem_data	args( ( void ) );
void		free_mem_data	args( ( MEM_DATA *memory ) );
MEM_DATA *	find_memory	args( ( MEM_DATA *memory, long id ) );

BUFFER *	new_buf		args( ( void ) );
BUFFER *	new_buf_size	args( ( int size ) );
void		free_buf	args( ( BUFFER *buffer ) );
bool		add_buf		args( ( BUFFER *buffer, char *string ) );

OBJ_MULTI_DATA	*new_obj_multi	args( ( void ) );
void		free_obj_multi	args( ( OBJ_DATA *obj, OBJ_MULTI_DATA *obj_multi ) );

POLL_DATA	*new_poll	args( ( void ) );
void		free_poll	args( ( POLL_DATA *poll ) );

VOTE_DATA	*new_vote	args( ( void ) );
void		free_vote	args( ( VOTE_DATA *vote ) );

GRANT_DATA	*new_grant	args( ( void ) );
void		free_grant	args( ( GRANT_DATA *grant ) );

sh_int		*new_short	args( ( sh_int size, sh_int value ) );
bool		*new_bool	args( ( sh_int size, bool value ) );

sh_int		*redo_short	args( ( sh_int *data, sh_int size, sh_int pos, sh_int value ) );
bool		*redo_bool	args( ( bool *data, sh_int size, sh_int pos, bool value ) );

void		free_short	args( ( sh_int *data ) );
void		free_bool	args( ( bool *data ) );

void	update_group_data	args( ( int pos ) );
void	update_skill_data	args( ( int pos ) );
void	update_class_data	args( ( int pos ) );
