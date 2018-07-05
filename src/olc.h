/***************************************************************************
 *  File: olc.h                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 * This is a header file for all the OLC files.  Feel free to copy it into
 * merc.h if you wish.  Many of these routines may be handy elsewhere in
 * the code.  -Jason Dinkel
 */

/*
 * New typedefs.
 */
typedef	bool OLC_FUN		args( ( CHAR_DATA *ch, char *argument ) );
#define DECLARE_OLC_FUN( fun )	OLC_FUN    fun

#define MAX_MOB	1		/* Default maximum number for resetting mobs */

DECLARE_DO_FUN( do_help );

struct olc_cmd_type
{
    int			level;
    char * const	name;
    OLC_FUN *		olc_fun;
};

struct	editor_cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
};

AREA_DATA *get_area_data	args ( ( int vnum ) );
void add_reset			args ( ( ROOM_INDEX_DATA *room, 
				         RESET_DATA *pReset, int index ) );
bool show_help                  args ( ( CHAR_DATA *ch, char *argument ) );
int  social_lookup		args ( ( const char *name ) );

DECLARE_OLC_FUN( aedit_alignment	);
DECLARE_OLC_FUN( aedit_balance		);
DECLARE_OLC_FUN( aedit_builder		);
DECLARE_OLC_FUN( aedit_clan		);
DECLARE_OLC_FUN( aedit_create		);
DECLARE_OLC_FUN( aedit_delete		);
DECLARE_OLC_FUN( aedit_directions	);
DECLARE_OLC_FUN( aedit_exit_scan	);
DECLARE_OLC_FUN( aedit_file		);
DECLARE_OLC_FUN( aedit_flags		);
DECLARE_OLC_FUN( aedit_flag_rooms	);
DECLARE_OLC_FUN( aedit_levels		);
DECLARE_OLC_FUN( aedit_lvnum		);
DECLARE_OLC_FUN( aedit_music		);
DECLARE_OLC_FUN( aedit_name		);
DECLARE_OLC_FUN( aedit_reset		);
DECLARE_OLC_FUN( aedit_run_vnum		);
DECLARE_OLC_FUN( aedit_security		);
DECLARE_OLC_FUN( aedit_show		);
DECLARE_OLC_FUN( aedit_uvnum		);
DECLARE_OLC_FUN( aedit_unflag_rooms	);
DECLARE_OLC_FUN( aedit_vnum		);

DECLARE_OLC_FUN( channel_edit_arena	);
DECLARE_OLC_FUN( channel_edit_censor	);
DECLARE_OLC_FUN( channel_edit_char	);
DECLARE_OLC_FUN( channel_edit_color	);
DECLARE_OLC_FUN( channel_edit_drunk	);
DECLARE_OLC_FUN( channel_edit_level	);
DECLARE_OLC_FUN( channel_edit_others	);
DECLARE_OLC_FUN( channel_edit_pretitle	);
DECLARE_OLC_FUN( channel_edit_quiet	);
DECLARE_OLC_FUN( channel_edit_show	);

DECLARE_OLC_FUN( clan_edit_color_name	);
DECLARE_OLC_FUN( clan_edit_create	);
DECLARE_OLC_FUN( clan_edit_delete	);
DECLARE_OLC_FUN( clan_edit_description	);
DECLARE_OLC_FUN( clan_edit_donation	);
DECLARE_OLC_FUN( clan_edit_independent	);
DECLARE_OLC_FUN( clan_edit_max_member	);
DECLARE_OLC_FUN( clan_edit_name		);
DECLARE_OLC_FUN( clan_edit_pkill	);
DECLARE_OLC_FUN( clan_edit_rank		);
DECLARE_OLC_FUN( clan_edit_recall	);
DECLARE_OLC_FUN( clan_edit_show		);
DECLARE_OLC_FUN( clan_edit_who_name	);

DECLARE_OLC_FUN( class_edit_base_group	);
DECLARE_OLC_FUN( class_edit_copy	);
DECLARE_OLC_FUN( class_edit_create	);
DECLARE_OLC_FUN( class_edit_def_group	);
DECLARE_OLC_FUN( class_edit_delete	);
DECLARE_OLC_FUN( class_edit_disabled	);
DECLARE_OLC_FUN( class_edit_groups	);
DECLARE_OLC_FUN( class_edit_hitpoints	);
DECLARE_OLC_FUN( class_edit_mana_class	);
DECLARE_OLC_FUN( class_edit_max_hp	);
DECLARE_OLC_FUN( class_edit_min_hp	);
DECLARE_OLC_FUN( class_edit_name	);
DECLARE_OLC_FUN( class_edit_show	);
DECLARE_OLC_FUN( class_edit_skills	);
DECLARE_OLC_FUN( class_edit_stat	);
DECLARE_OLC_FUN( class_edit_sub_class	);
DECLARE_OLC_FUN( class_edit_thac	);
DECLARE_OLC_FUN( class_edit_thac00	);
DECLARE_OLC_FUN( class_edit_thac32	);
DECLARE_OLC_FUN( class_edit_tier	);
DECLARE_OLC_FUN( class_edit_who_name	);

DECLARE_OLC_FUN( command_edit_name	);
DECLARE_OLC_FUN( command_edit_show	);

DECLARE_OLC_FUN( game_edit_capslock	);
DECLARE_OLC_FUN( game_edit_colorlock	);
DECLARE_OLC_FUN( game_edit_connections	);
DECLARE_OLC_FUN( game_edit_evil_god	);
DECLARE_OLC_FUN( game_edit_good_god	);
DECLARE_OLC_FUN( game_edit_imm_timer	);
DECLARE_OLC_FUN( game_edit_ld_immortal	);
DECLARE_OLC_FUN( game_edit_ld_mortal	);
DECLARE_OLC_FUN( game_edit_log_all	);
DECLARE_OLC_FUN( game_edit_max_ever	);
DECLARE_OLC_FUN( game_edit_mortal_timer	);
DECLARE_OLC_FUN( game_edit_most_today	);
DECLARE_OLC_FUN( game_edit_multilock	);
DECLARE_OLC_FUN( game_edit_name		);
DECLARE_OLC_FUN( game_edit_neutral_god	);
DECLARE_OLC_FUN( game_edit_newlock	);
DECLARE_OLC_FUN( game_edit_quest_exp	);
DECLARE_OLC_FUN( game_edit_quest_gold	);
DECLARE_OLC_FUN( game_edit_quest_object	);
DECLARE_OLC_FUN( game_edit_quest_points	);
DECLARE_OLC_FUN( game_edit_quest_pracs	);
DECLARE_OLC_FUN( game_edit_quest_vnum	);
DECLARE_OLC_FUN( game_edit_random	);
DECLARE_OLC_FUN( game_edit_show		);
DECLARE_OLC_FUN( game_edit_unique	);
DECLARE_OLC_FUN( game_edit_wizlock	);

DECLARE_OLC_FUN( gredit_cost		);
DECLARE_OLC_FUN( gredit_create		);
DECLARE_OLC_FUN( gredit_name		);
DECLARE_OLC_FUN( gredit_delete		);
DECLARE_OLC_FUN( gredit_groups		);
DECLARE_OLC_FUN( gredit_show		);
DECLARE_OLC_FUN( gredit_spells		);

DECLARE_OLC_FUN( hedit_clan		);
DECLARE_OLC_FUN( hedit_color_show	);
DECLARE_OLC_FUN( hedit_delete 		);
DECLARE_OLC_FUN( hedit_desc   		);
DECLARE_OLC_FUN( hedit_immtext   	);
DECLARE_OLC_FUN( hedit_keywords   	);
DECLARE_OLC_FUN( hedit_level   		);
DECLARE_OLC_FUN( hedit_make	 	);
DECLARE_OLC_FUN( hedit_name		);
DECLARE_OLC_FUN( hedit_show    		);

DECLARE_OLC_FUN( medit_ac		);
DECLARE_OLC_FUN( medit_act		);
DECLARE_OLC_FUN( medit_addmprog		);
DECLARE_OLC_FUN( medit_affect		);
DECLARE_OLC_FUN( medit_align		);
DECLARE_OLC_FUN( medit_balance		);
DECLARE_OLC_FUN( medit_bank_branch	);
DECLARE_OLC_FUN( medit_class		);
DECLARE_OLC_FUN( medit_copy		);
DECLARE_OLC_FUN( medit_create		);
DECLARE_OLC_FUN( medit_damdice		);
DECLARE_OLC_FUN( medit_damtype		);
DECLARE_OLC_FUN( medit_dam_mod		);
DECLARE_OLC_FUN( medit_delete		);
DECLARE_OLC_FUN( medit_delmprog		);
DECLARE_OLC_FUN( medit_desc		);
DECLARE_OLC_FUN( medit_die_desc		);
DECLARE_OLC_FUN( medit_exp_mod		);
DECLARE_OLC_FUN( medit_gold		);
DECLARE_OLC_FUN( medit_group		);
DECLARE_OLC_FUN( medit_hitpoints	);
DECLARE_OLC_FUN( medit_hitroll		);
DECLARE_OLC_FUN( medit_level		);
DECLARE_OLC_FUN( medit_long		);
DECLARE_OLC_FUN( medit_manapoints	);
DECLARE_OLC_FUN( medit_max_world	);
DECLARE_OLC_FUN( medit_name		);
DECLARE_OLC_FUN( medit_part		);
DECLARE_OLC_FUN( medit_position		);
DECLARE_OLC_FUN( medit_race		);
DECLARE_OLC_FUN( medit_reflection	);
DECLARE_OLC_FUN( medit_regen_hit	);
DECLARE_OLC_FUN( medit_regen_mana	);
DECLARE_OLC_FUN( medit_regen_move	);
DECLARE_OLC_FUN( medit_absorption	);
DECLARE_OLC_FUN( medit_say_desc		);
DECLARE_OLC_FUN( medit_saves		);
DECLARE_OLC_FUN( medit_sex		);
DECLARE_OLC_FUN( medit_shield		);
DECLARE_OLC_FUN( medit_shop		);
DECLARE_OLC_FUN( medit_short		);
DECLARE_OLC_FUN( medit_show		);
DECLARE_OLC_FUN( medit_size		);
DECLARE_OLC_FUN( medit_skills		);
DECLARE_OLC_FUN( medit_skill_percentage	);
DECLARE_OLC_FUN( medit_spec		);

DECLARE_OLC_FUN( mpedit_author		);
DECLARE_OLC_FUN( mpedit_code		);
DECLARE_OLC_FUN( mpedit_create		);
DECLARE_OLC_FUN( mpedit_delete		);
DECLARE_OLC_FUN( mpedit_name		);
DECLARE_OLC_FUN( mpedit_show		);

DECLARE_OLC_FUN( oedit_addaffect	);
DECLARE_OLC_FUN( oedit_addapply		);
DECLARE_OLC_FUN( oedit_addoprog		);
DECLARE_OLC_FUN( oedit_balance		);
DECLARE_OLC_FUN( oedit_class		);
DECLARE_OLC_FUN( oedit_copy		);
DECLARE_OLC_FUN( oedit_cost		);
DECLARE_OLC_FUN( oedit_create		);
DECLARE_OLC_FUN( oedit_dam_mod		);
DECLARE_OLC_FUN( oedit_delete		);
DECLARE_OLC_FUN( oedit_delaffect	);
DECLARE_OLC_FUN( oedit_deloprog		);
DECLARE_OLC_FUN( oedit_ed		);
DECLARE_OLC_FUN( oedit_extra            );
DECLARE_OLC_FUN( oedit_forge		);
DECLARE_OLC_FUN( oedit_level            );
DECLARE_OLC_FUN( oedit_history		);
DECLARE_OLC_FUN( oedit_long		);
DECLARE_OLC_FUN( oedit_name		);
DECLARE_OLC_FUN( oedit_quest		);
DECLARE_OLC_FUN( oedit_short		);
DECLARE_OLC_FUN( oedit_show		);
DECLARE_OLC_FUN( oedit_size		);
DECLARE_OLC_FUN( oedit_success		);
DECLARE_OLC_FUN( oedit_targets		);
DECLARE_OLC_FUN( oedit_type             );
DECLARE_OLC_FUN( oedit_value0		);
DECLARE_OLC_FUN( oedit_value1		);
DECLARE_OLC_FUN( oedit_value2		);
DECLARE_OLC_FUN( oedit_value3		);
DECLARE_OLC_FUN( oedit_value4		);
DECLARE_OLC_FUN( oedit_wear             );
DECLARE_OLC_FUN( oedit_weight		);

DECLARE_OLC_FUN( race_edit_affects	);
DECLARE_OLC_FUN( race_edit_attack	);
DECLARE_OLC_FUN( race_edit_base_stats	);
DECLARE_OLC_FUN( race_edit_class	);
DECLARE_OLC_FUN( race_edit_create	);
DECLARE_OLC_FUN( race_edit_dam_mod	);
DECLARE_OLC_FUN( race_edit_delete	);
DECLARE_OLC_FUN( race_edit_disabled	);
DECLARE_OLC_FUN( race_edit_max_stats	);
DECLARE_OLC_FUN( race_edit_multiplier	);
DECLARE_OLC_FUN( race_edit_name		);
DECLARE_OLC_FUN( race_edit_parts	);
DECLARE_OLC_FUN( race_edit_pc_race	);
DECLARE_OLC_FUN( race_edit_points	);
DECLARE_OLC_FUN( race_edit_shields	);
DECLARE_OLC_FUN( race_edit_show		);
DECLARE_OLC_FUN( race_edit_size		);
DECLARE_OLC_FUN( race_edit_skills	);
DECLARE_OLC_FUN( race_edit_who_name	);

DECLARE_OLC_FUN( random_edit_addaffect	);
DECLARE_OLC_FUN( random_edit_affects	);
DECLARE_OLC_FUN( random_edit_align	);
DECLARE_OLC_FUN( random_edit_create	);
DECLARE_OLC_FUN( random_edit_dam_mod	);
DECLARE_OLC_FUN( random_edit_delaffect	);
DECLARE_OLC_FUN( random_edit_delete	);
DECLARE_OLC_FUN( random_edit_level	);
DECLARE_OLC_FUN( random_edit_name	);
DECLARE_OLC_FUN( random_edit_shields	);
DECLARE_OLC_FUN( random_edit_show	);

DECLARE_OLC_FUN( redit_addrprog		);
DECLARE_OLC_FUN( redit_copy		);
DECLARE_OLC_FUN( redit_create		);
DECLARE_OLC_FUN( redit_damage		);
DECLARE_OLC_FUN( redit_delete		);
DECLARE_OLC_FUN( redit_delrprog		);
DECLARE_OLC_FUN( redit_desc		);
DECLARE_OLC_FUN( redit_down		);
DECLARE_OLC_FUN( redit_east		);
DECLARE_OLC_FUN( redit_ed		);
DECLARE_OLC_FUN( redit_format		);
DECLARE_OLC_FUN( redit_heal		);
DECLARE_OLC_FUN( redit_mana		);
DECLARE_OLC_FUN( redit_max_people	);
DECLARE_OLC_FUN( redit_mreset		);
DECLARE_OLC_FUN( redit_mshow		);
DECLARE_OLC_FUN( redit_music		);
DECLARE_OLC_FUN( redit_name		);
DECLARE_OLC_FUN( redit_north		);
DECLARE_OLC_FUN( redit_oreset		);
DECLARE_OLC_FUN( redit_oshow		);
DECLARE_OLC_FUN( redit_room		);
DECLARE_OLC_FUN( redit_sector		);
DECLARE_OLC_FUN( redit_show		);
DECLARE_OLC_FUN( redit_snake		);
DECLARE_OLC_FUN( redit_south		);
DECLARE_OLC_FUN( redit_up		);
DECLARE_OLC_FUN( redit_west		);

DECLARE_OLC_FUN( rdam_create		);
DECLARE_OLC_FUN( rdam_damtype		);
DECLARE_OLC_FUN( rdam_delete		);
DECLARE_OLC_FUN( rdam_range		);
DECLARE_OLC_FUN( rdam_room_msg		);
DECLARE_OLC_FUN( rdam_show		);
DECLARE_OLC_FUN( rdam_success		);
DECLARE_OLC_FUN( rdam_victim_msg	);

DECLARE_OLC_FUN( shop_edit_delete	);
DECLARE_OLC_FUN( shop_edit_hours	);
DECLARE_OLC_FUN( shop_edit_markdown	);
DECLARE_OLC_FUN( shop_edit_markup	);
DECLARE_OLC_FUN( shop_edit_show		);
DECLARE_OLC_FUN( shop_edit_trade	);

DECLARE_OLC_FUN( skedit_beats		);
DECLARE_OLC_FUN( skedit_clan_cost	);
DECLARE_OLC_FUN( skedit_copy		);
DECLARE_OLC_FUN( skedit_cost_hp		);
DECLARE_OLC_FUN( skedit_cost_mana	);
DECLARE_OLC_FUN( skedit_cost_move	);
DECLARE_OLC_FUN( skedit_create		);
DECLARE_OLC_FUN( skedit_dam_noun	);
DECLARE_OLC_FUN( skedit_delete		);
DECLARE_OLC_FUN( skedit_flags		);
DECLARE_OLC_FUN( skedit_group		);
DECLARE_OLC_FUN( skedit_level		);
DECLARE_OLC_FUN( skedit_name		);
DECLARE_OLC_FUN( skedit_obj_off		);
DECLARE_OLC_FUN( skedit_position	);
DECLARE_OLC_FUN( skedit_rating		);
DECLARE_OLC_FUN( skedit_room_off	);
DECLARE_OLC_FUN( skedit_show		);
DECLARE_OLC_FUN( skedit_sound_cast	);
DECLARE_OLC_FUN( skedit_sound_off	);
DECLARE_OLC_FUN( skedit_target		);
DECLARE_OLC_FUN( skedit_wear_off	);

DECLARE_OLC_FUN( social_edit_cfound	);
DECLARE_OLC_FUN( social_edit_cnoarg	);
DECLARE_OLC_FUN( social_edit_create	);
DECLARE_OLC_FUN( social_edit_cself	);
DECLARE_OLC_FUN( social_edit_delete	);
DECLARE_OLC_FUN( social_edit_name	);
DECLARE_OLC_FUN( social_edit_ofound	);
DECLARE_OLC_FUN( social_edit_onoarg	);
DECLARE_OLC_FUN( social_edit_oself	);
DECLARE_OLC_FUN( social_edit_show	);
DECLARE_OLC_FUN( social_edit_vfound	);

DECLARE_OLC_FUN( opedit_author		);
DECLARE_OLC_FUN( opedit_code		);
DECLARE_OLC_FUN( opedit_create		);
DECLARE_OLC_FUN( opedit_delete		);
DECLARE_OLC_FUN( opedit_name		);
DECLARE_OLC_FUN( opedit_show		);

DECLARE_OLC_FUN( rpedit_author		);
DECLARE_OLC_FUN( rpedit_code		);
DECLARE_OLC_FUN( rpedit_create		);
DECLARE_OLC_FUN( rpedit_delete		);
DECLARE_OLC_FUN( rpedit_name		);
DECLARE_OLC_FUN( rpedit_show		);

#define IS_BUILDER(ch, Area)	( ( ch->clan != 0 && ch->clan == Area->clan ) \
		|| ( ch->pcdata && ch->pcdata->security >= Area->security ) )

#define EDIT_MOB(Ch, Mob)	( Mob = (MOB_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_OBJ(Ch, Obj)	( Obj = (OBJ_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_ROOM(Ch, Room)	( Room = (ROOM_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_RDAM(Ch, Rdam)	( Rdam = (ROOM_DAMAGE_DATA *)Ch->desc->pEdit )
#define EDIT_AREA(Ch, Area)	( Area = (AREA_DATA *)Ch->desc->pEdit )
#define EDIT_HELP(Ch, Help)	( Help = (HELP_DATA *)Ch->desc->pEdit )
#define EDIT_PCODE(Ch, Code)	( Code = (PROG_CODE *)Ch->desc->pEdit )
#define EDIT_SHOP(Ch, Code)	( Code = (SHOP_DATA *)Ch->desc->pEdit )
#define EDIT_TABLE(Ch, table)	( table = (int)Ch->desc->pEdit )

/*
 * Prototypes
 */
/* mem.c - memory prototypes. */
#define ED	EXTRA_DESCR_DATA
RESET_DATA	*new_reset_data		args( ( void ) );
void		free_reset_data		args( ( RESET_DATA *pReset ) );
AREA_DATA	*new_area		args( ( void ) );
void		free_area		args( ( AREA_DATA *pArea ) );
EXIT_DATA	*new_exit		args( ( void ) );
void		free_exit		args( ( EXIT_DATA *pExit ) );
ED 		*new_extra_descr	args( ( void ) );
void		free_extra_descr	args( ( ED *pExtra ) );
ROOM_INDEX_DATA *new_room_index		args( ( void ) );
void		free_room_index		args( ( ROOM_INDEX_DATA *pRoom ) );
ROOM_DAMAGE_DATA*new_room_damage	args( ( int type ) );
void		free_room_damage	args( ( ROOM_DAMAGE_DATA *pRoom ) );
AFFECT_DATA	*new_affect		args( ( void ) );
void		free_affect		args( ( AFFECT_DATA* pAf ) );
SHOP_DATA	*new_shop		args( ( void ) );
void		free_shop		args( ( SHOP_DATA *pShop ) );
OBJ_INDEX_DATA	*new_obj_index		args( ( void ) );
void		free_obj_index		args( ( OBJ_INDEX_DATA *pObj ) );
MOB_INDEX_DATA	*new_mob_index		args( ( void ) );
void		free_mob_index		args( ( MOB_INDEX_DATA *pMob ) );
#undef	ED

PROG_LIST	*new_prog		args( ( void ) );
void		free_prog		args( ( PROG_LIST *mp ) );
PROG_CODE	*new_pcode		args( (void) );
void		free_pcode		args( ( PROG_CODE *pMcode));
char		*prog_type_to_name	args( ( int type ) );
void		check_olc_delete	args( ( CHAR_DATA *ch ) );
