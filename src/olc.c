/***************************************************************************
 *  File: olc.c                                                            *
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

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "olc.h"
#include "recycle.h"

DECLARE_SPELL_FUN( spell_null	);

AREA_DATA *get_area_from_editor	args( ( CHAR_DATA *ch ) );
bool	show_commands		args( ( CHAR_DATA *ch, char *argument ) );
int	channel_lookup		args( ( char *argument ) );
int	cmd_lookup		args( ( char *command ) );
void	reset_clan_mobs		args( ( MOB_INDEX_DATA *pMob ) );
void	reset_clan_objects	args( ( OBJ_INDEX_DATA *pObj ) );

const	struct			editor_cmd_type editor_table[];

/*
 * The version info.  Please use this info when reporting bugs.
 * It is displayed in the game by typing 'version' while editing.
 * Do not remove these from the code - by request of Jason Dinkel
 */
bool show_version( CHAR_DATA *ch, char *argument )
{
    send_to_char( "ILAB Online Creation [Beta 1.0, ROM 2.3 modified]\n\r"
		  "     Port a ROM 2.4 v1.00\n\r\n\r"
		  "     By Jason(jdinkel@mines.colorado.edu)\n\r"
		  "     Modified for use with ROM 2.3\n\r"
		  "     By Hans Birkeland (hansbi@ifi.uio.no)\n\r"
		  "     Modificado para uso en ROM 2.4b4a\n\r"
		  "     Por Birdie (itoledo@ramses.centic.utem.cl)\n\r\n\r"
		  "     (Apr. 7, 1995 - ROM mod, Apr 16, 1995)\n\r"
		  "     (Port a ROM 2.4 - Nov 2, 1996)\n\r"
		  "     Version actual : 1.5a - Mar 9, 1997\n\r\n\r"
		  "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)\n\r", ch );
    return FALSE;
}

const struct olc_cmd_type aedit_table[] =
{
    { IM, "alignment",		aedit_alignment		},
    { ML, "balance",		aedit_balance		},
    { IM, "builder",		aedit_builder		},
    { ML, "clan",		aedit_clan		},
    {  0, "commands",		show_commands		},
    { IM, "create",		aedit_create		},
    { ML, "delete",		aedit_delete		},
    { IM, "directions",		aedit_directions	},
    { L4, "filename",		aedit_file		},
    { IM, "flags",		aedit_flags		},
    { IM, "flagrooms",		aedit_flag_rooms	},
    { IM, "lvnum",		aedit_lvnum		},
    { IM, "levels",		aedit_levels		},
    { IM, "music",		aedit_music		},
    { IM, "name",		aedit_name		},
    { L4, "reset",		aedit_reset		},
    { IM, "run_vnum",		aedit_run_vnum		},
    { IM, "security",		aedit_security		},
    {  0, "show",		aedit_show		},
    { IM, "unflagrooms",	aedit_unflag_rooms	},
    { IM, "uvnum",		aedit_uvnum		},
    {  0, "version",		show_version		},
    { IM, "vnum",		aedit_vnum		},
    { IM, "xscan",		aedit_exit_scan		},
    {  0, "?",			show_help		},
    {  0, NULL,			0			}
};

const struct olc_cmd_type channel_edit_table[] =
{
    { IM, "arena",		channel_edit_arena	},
    { IM, "censor",		channel_edit_censor	},
    { IM, "char_string",	channel_edit_char	},
    { IM, "color",		channel_edit_color	},
    {  0, "commands",		show_commands		},
    { IM, "drunk",		channel_edit_drunk	},
    { IM, "level",		channel_edit_level	},
    { IM, "other_string",	channel_edit_others	},
    { IM, "pretitle",		channel_edit_pretitle	},
    { IM, "quiet",		channel_edit_quiet	},
    {  0, "show",		channel_edit_show	},
    {  0, NULL,			0			}
};

const struct olc_cmd_type clan_edit_table[] =
{
    {  0, "color_name",		clan_edit_color_name	},
    {  0, "commands",		show_commands		},
    {  0, "delete",		clan_edit_delete	},
    {  0, "description",	clan_edit_description	},
    { IM, "donation",		clan_edit_donation	},
    { IM, "independent",	clan_edit_independent	},
    { IM, "max_member",		clan_edit_max_member	},
    {  0, "name",		clan_edit_name		},
    { IM, "pkill",		clan_edit_pkill		},
    {  0, "rank",		clan_edit_rank		},
    { IM, "recall",		clan_edit_recall	},
    {  0, "show",		clan_edit_show		},
    {  0, "who_name",		clan_edit_who_name	},
    {  0, NULL,			0			}
};

const struct olc_cmd_type class_edit_table[] =
{
    { IM, "base_group",		class_edit_base_group	},
    {  0, "commands",		show_commands		},
    { IM, "copy",		class_edit_copy		},
    { IM, "create",		class_edit_create	},
    { IM, "default_group",	class_edit_def_group	},
    { L1, "delete",		class_edit_delete	},
    { IM, "disabled",		class_edit_disabled	},
    { L2, "groups",		class_edit_groups	},
    { IM, "hitpoints",		class_edit_hitpoints	},
    { IM, "mana_class",		class_edit_mana_class	},
    { IM, "max_hp",		class_edit_max_hp	},
    { IM, "min_hp",		class_edit_min_hp	},
    { ML, "name",		class_edit_name		},
    {  0, "show",		class_edit_show		},
    { L2, "skills",		class_edit_skills	},
    { IM, "stat",		class_edit_stat		},
    { IM, "sub_class",		class_edit_sub_class	},
    { IM, "thacs",		class_edit_thac		},
    { IM, "thac00",		class_edit_thac00	},
    { IM, "thac32",		class_edit_thac32	},
    { IM, "tier",		class_edit_tier		},
    { IM, "who_name",		class_edit_who_name	},
    {  0, NULL,			0			}
};

const struct olc_cmd_type command_edit_table[] =
{
    { IM, "commands",		show_commands		},
    { ML, "name",		command_edit_name	},
    { IM, "show",		command_edit_show	},
    {  0, NULL,			0			}
};

const struct olc_cmd_type game_edit_table[] =
{
    { L5, "capslock",		game_edit_capslock	},
    { L5, "colorlock",		game_edit_colorlock	},
    {  0, "commands",		show_commands		},
    { L1, "connections",	game_edit_connections	},
    { IM, "evil_god",		game_edit_evil_god	},
    { IM, "good_god",		game_edit_good_god	},
    { L1, "imm_timeout",	game_edit_imm_timer	},
    { L1, "ld_immortal",	game_edit_ld_immortal	},
    { L1, "ld_mortal",		game_edit_ld_mortal	},
    { L1, "log_all",		game_edit_log_all	},
    { L1, "max_ever",		game_edit_max_ever	},
    { L1, "mort_timeout",	game_edit_mortal_timer	},
    { L1, "most_today",		game_edit_most_today	},
    { L1, "multilock",		game_edit_multilock	},
    { L4, "name",		game_edit_name		},
    { IM, "neutral_god",	game_edit_neutral_god	},
    { L5, "newlock",		game_edit_newlock	},
    { ML, "qexp",		game_edit_quest_exp	},
    { ML, "qgold",		game_edit_quest_gold	},
    { ML, "qobject",		game_edit_quest_object	},
    { ML, "qpoints",		game_edit_quest_points	},
    { ML, "qpracs",		game_edit_quest_pracs	},
    { ML, "qvnum",		game_edit_quest_vnum	},
    { L1, "randoms",		game_edit_random	},
    { IM, "show",		game_edit_show		},
    { L1, "unique",		game_edit_unique	},
    { L1, "wizlock",		game_edit_wizlock	},
    {  0, NULL,			0			}
};

const struct olc_cmd_type gredit_table[] =
{
    {  0, "commands",		show_commands		},
    { IM, "cost",		gredit_cost		},
    { IM, "create",		gredit_create		},
    { IM, "delete",		gredit_delete		},
    { IM, "groups",		gredit_groups		},
    { IM, "name",		gredit_name		},
    {  0, "show",		gredit_show		},
    { IM, "skills",		gredit_spells		},
    { IM, "spells",		gredit_spells		},
    {  0, NULL,			0			}
};

const struct olc_cmd_type hedit_table[] =
{
    { IM, "clan",		hedit_clan		},
    {  0, "commands",		show_commands		},
    {  0, "cshow",		hedit_color_show	},
    { IM, "delete",		hedit_delete		},
    {  0, "desc",		hedit_desc		},
    {  0, "keywords",		hedit_keywords		},
    { IM, "level",		hedit_level		},
    { IM, "make",		hedit_make		},
    {  0, "name",		hedit_name		},
    {  0, "show",		hedit_show		},
    {  0, "text",		hedit_desc		},
    {  0, "immtext",		hedit_immtext           },
    {  0, "?",			show_help		},
    {  0, NULL,			0			}
};

const struct olc_cmd_type medit_table[] =
{
    { IM, "absorption",		medit_absorption	},
    { IM, "act",		medit_act		},
    { IM, "addmprog",		medit_addmprog		},
    { IM, "affect",		medit_affect		},
    {  0, "alignment",		medit_align		},
    { IM, "armor",		medit_ac		},
    { IM, "balance",		medit_balance		},
    { IM, "bank_branch",	medit_bank_branch	},
    {  0, "commands",		show_commands		},
    {  0, "class",		medit_class		},
    { IM, "copy",		medit_copy		},
    { IM, "create",		medit_create		},
    { IM, "dam_mod",		medit_dam_mod		},
    { IM, "damdice",		medit_damdice		},
    {  0, "damtype",		medit_damtype		},
    { IM, "delete",		medit_delete		},
    { IM, "delmprog",		medit_delmprog		},
    {  0, "desc",		medit_desc		},
    {  0, "desc_die",		medit_die_desc		},
    {  0, "desc_say",		medit_say_desc		},
    { IM, "exp_mod",		medit_exp_mod		},
    { IM, "group",		medit_group		},
    { IM, "hitpoints",		medit_hitpoints		},
    { IM, "hitroll",		medit_hitroll		},
    { IM, "level",		medit_level		},
    {  0, "long",		medit_long		},
    { IM, "manapoints",		medit_manapoints	},
    { IM, "max_world",		medit_max_world		},
    {  0, "name",		medit_name		},
    {  0, "part",		medit_part		},
    {  0, "position",		medit_position		},
    { IM, "race",		medit_race		},
    { IM, "reflection",		medit_reflection	},
    { IM, "regen_hit",		medit_regen_hit		},
    { IM, "regen_mana",		medit_regen_mana	},
    { IM, "regen_move",		medit_regen_move	},
    { IM, "saves",		medit_saves		},
    {  0, "sex",		medit_sex		},
    { IM, "shield",		medit_shield		},
    { IM, "shop",		medit_shop		},
    {  0, "short",		medit_short		},
    {  0, "show",		medit_show		},
    { IM, "size",		medit_size		},
    { IM, "skills",		medit_skills		},
    { IM, "skill_percent",	medit_skill_percentage	},
    { IM, "spec",		medit_spec		},
    {  0, "version",		show_version		},
    { IM, "wealth",		medit_gold		},
    {  0, "?",			show_help		},
    {  0, NULL,			0			}
};

const struct olc_cmd_type mpedit_table[] =
{
    { IM, "author",		mpedit_author		},
    {  0, "commands",		show_commands		},
    { IM, "create",		mpedit_create		},
    { IM, "code",		mpedit_code		},
    { IM, "delete",		mpedit_delete		},
    { IM, "name",		mpedit_name		},
    {  0, "show",		mpedit_show		},
    {  0, "?",			show_help		},
    {  0, NULL,			0			}
};

const struct olc_cmd_type oedit_table[] =
{
    { IM, "addaffect",		oedit_addaffect		},
    { IM, "addapply",		oedit_addapply		},
    { L4, "balance",		oedit_balance		},
    { IM, "class",		oedit_class		},
    {  0, "commands",		show_commands		},
    { IM, "copy",		oedit_copy		},
    { IM, "cost",		oedit_cost		},
    { IM, "create",		oedit_create		},
    { IM, "dam_mod",		oedit_dam_mod		},
    { IM, "delaffect",		oedit_delaffect		},
    { IM, "delete",		oedit_delete		},
    { IM, "addoprog",		oedit_addoprog 		},
    { IM, "deloprog",		oedit_deloprog		},
    {  0, "ed",			oedit_ed		},
    { IM, "extra",		oedit_extra		},
    { L2, "forged",		oedit_forge		},
    {  0, "history",		oedit_history		},
    { IM, "level",		oedit_level		},
    {  0, "long",		oedit_long		},
    {  0, "name",		oedit_name		},
    { L2, "quest",		oedit_quest		},
    {  0, "short",		oedit_short		},
    {  0, "show",		oedit_show		},
    { IM, "size",		oedit_size		},
    { IM, "success",		oedit_success		},
    { IM, "targets",		oedit_targets		},
    { IM, "type",		oedit_type		},
    { IM, "v0",			oedit_value0		},
    { IM, "v1",			oedit_value1		},
    { IM, "v2",			oedit_value2		},
    { IM, "v3",			oedit_value3		},
    { IM, "v4",			oedit_value4		},
    {  0, "version",		show_version		},
    { IM, "weight",		oedit_weight		},
    { IM, "wear",		oedit_wear		},
    {  0, "?",			show_help		},
    {  0, NULL,			0			}
};

const struct olc_cmd_type opedit_table[] =
{
    { IM, "author",		mpedit_author		},
    {  0, "commands",		show_commands		},
    { IM, "create",		opedit_create		},
    { IM, "code",		opedit_code		},
    { IM, "delete",		opedit_delete		},
    { IM, "name",		mpedit_name		},
    {  0, "show",		opedit_show		},
    {  0, "?",			show_help		},
    {  0, NULL,			0			}
};

const struct olc_cmd_type race_edit_table[] =
{
    { IM, "affects",		race_edit_affects	},
    { IM, "attack",		race_edit_attack	},
    { IM, "base_stats",		race_edit_base_stats	},
    { IM, "class",		race_edit_class		},
    {  0, "commands",		show_commands		},
    { IM, "create",		race_edit_create	},
    { IM, "dam_mods",		race_edit_dam_mod	},
    { L1, "delete",		race_edit_delete	},
    { IM, "disabled",		race_edit_disabled	},
    { IM, "max_stats",		race_edit_max_stats	},
    { IM, "multiplier",		race_edit_multiplier	},
    { ML, "name",		race_edit_name		},
    { IM, "parts",		race_edit_parts		},
    { IM, "pc_race",		race_edit_pc_race	},
    { IM, "points",		race_edit_points	},
    { IM, "shields",		race_edit_shields	},
    {  0, "show",		race_edit_show		},
    { IM, "size",		race_edit_size		},
    { IM, "skills",		race_edit_skills	},
    { IM, "who_name",		race_edit_who_name	},
    {  0, NULL,			0			}
};

const struct olc_cmd_type random_edit_table[] =
{
    { L1, "addaffect",		random_edit_addaffect	},
    { L1, "affects",		random_edit_affects	},
    { L1, "align",		random_edit_align	},
    { L1, "create",		random_edit_create	},
    { L1, "dam_mod",		random_edit_dam_mod	},
    { L1, "delaffect",		random_edit_delaffect	},
    { L1, "delete",		random_edit_delete	},    
    { L1, "level",		random_edit_level	},
    { L1, "name",		random_edit_name	},
    { L1, "shields",		random_edit_shields	},
    {  0, "commands",		show_commands		},
    {  0, "show",		random_edit_show	},
    {  0, NULL,			0			}
};

const struct olc_cmd_type redit_table[] =
{
    { IM, "addrprog",		redit_addrprog  	},
    {  0, "commands",		show_commands		},
    { IM, "copy",		redit_copy		},
    { IM, "create",		redit_create		},
    { IM, "down",		redit_down		},
    { IM, "damage",		redit_damage		},
    {  0, "desc",		redit_desc		},
    { IM, "delete",		redit_delete		},
    { IM, "delrprog",		redit_delrprog		},
    { IM, "east",		redit_east		},
    { IM, "ed",			redit_ed		},
    { IM, "flags",		redit_room		},
    {  0, "format",		redit_format		},
    { IM, "health",		redit_heal		},
    { IM, "mana",		redit_mana		},
    { IM, "max_people",		redit_max_people	},
    { IM, "mreset",		redit_mreset		},
    { IM, "mshow",		redit_mshow		},
    { IM, "music",		redit_music		},
    { IM, "north",		redit_north		},
    {  0, "name",		redit_name		},
    {  0, "title",		redit_name		},
    { IM, "oreset",		redit_oreset		},
    { IM, "oshow",		redit_oshow		},
    { IM, "room",		redit_room		},
    { IM, "south",		redit_south		},
    { IM, "sector",		redit_sector		},
    {  0, "show",		redit_show		},
    { IM, "snake",		redit_snake		},
    { IM, "up",			redit_up		},
    {  0, "version",		show_version		},
    { IM, "west",		redit_west		},
    {  0, "?",			show_help		},
    {  0, NULL,			0			}
};

const struct olc_cmd_type room_damage_table[]=
{
    {  0, "commands",		show_commands		},
    { IM, "create",		rdam_create		},
    { IM, "damtype",		rdam_damtype		},
    { IM, "delete",		rdam_delete		},
    { IM, "range",		rdam_range		},
    { IM, "room_msg",		rdam_room_msg		},
    { IM, "show",		rdam_show		},
    { IM, "success",		rdam_success		},
    { IM, "victim_msg",		rdam_victim_msg		},
    {  0, NULL,			0			}
};

const struct olc_cmd_type rpedit_table[] =
{
    { IM, "author",		mpedit_author		},
    {  0, "commands",		show_commands		},
    { IM, "create",		rpedit_create		},
    { IM, "code",		rpedit_code		},
    { IM, "delete",		rpedit_delete		},
    { IM, "name",		mpedit_name		},
    {  0, "show",		rpedit_show		},
    {  0, "?",			show_help		},
    {  0, NULL,			0			}
};

const struct olc_cmd_type shop_edit_table[] =
{
    {  0, "commands",		show_commands		},
    { IM, "delete",		shop_edit_delete	},
    { IM, "hours",		shop_edit_hours		},
    { IM, "markdown",		shop_edit_markdown	},
    { IM, "markup",		shop_edit_markup	},
    {  0, "show",		shop_edit_show		},
    { IM, "trade",		shop_edit_trade		},
    {  0, NULL,			0			}
};

const struct olc_cmd_type skedit_table[] =
{
    { IM, "beats",		skedit_beats		},
    { ML, "clan-cost",		skedit_clan_cost	},
    {  0, "commands",		show_commands		},
    { IM, "copy",		skedit_copy		},
    { IM, "create",		skedit_create		},
    { IM, "dam_noun",		skedit_dam_noun		},
    { IM, "delete",		skedit_delete		},
    { IM, "flags",		skedit_flags		},
    { IM, "group",		skedit_group		},
    { IM, "hitpoints",		skedit_cost_hp		},
    { IM, "level",		skedit_level		},
    { IM, "mana",		skedit_cost_mana	},
    { IM, "move",		skedit_cost_move	},
    { IM, "name",		skedit_name		},
    { IM, "obj_off",		skedit_obj_off		},
    { IM, "position",		skedit_position		},
    { IM, "rating",		skedit_rating		},
    { IM, "room_off",		skedit_room_off		},
    {  0, "show",		skedit_show		},
    { IM, "sound_cast",		skedit_sound_cast	},
    { IM, "sound_off",		skedit_sound_off	},
    { ML, "target",		skedit_target		},
    { IM, "wear_off",		skedit_wear_off		},
    {  0, NULL,			0			}
};

const struct olc_cmd_type social_edit_table[] =
{
    { IM, "cfound",		social_edit_cfound	},
    { IM, "cnoarg",		social_edit_cnoarg	},
    {  0, "commands",		show_commands		},
    { IM, "create",		social_edit_create	},
    { IM, "cself",		social_edit_cself	},
    { IM, "delete",		social_edit_delete	},
    { IM, "name",		social_edit_name	},
    { IM, "ofound",		social_edit_ofound	},
    { IM, "onoarg",		social_edit_onoarg	},
    { IM, "oself",		social_edit_oself	},
    {  0, "show",		social_edit_show	},
    { IM, "vfound",		social_edit_vfound	},
    {  0, NULL,			0			}
};

void check_olc_delete( CHAR_DATA *ch )
{
    DESCRIPTOR_DATA *d;
    void *pEdit = ch->desc->pEdit;
    int editor = ch->desc->editor;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
	if ( d->editor == editor
	&&   d->pEdit == pEdit )
	{
	    d->pEdit = NULL;
	    d->editor = 0;

	    if ( d->character )
		send_to_char( "Someone has just deleted your OLC target.\n\r", d->character );
	}
    }
}

void show_olc_cmds( CHAR_DATA *ch, const struct olc_cmd_type *olc_table )
{
    BUFFER *final = new_buf( );
    char buf[MAX_STRING_LENGTH];
    int cmd, col = 0, trust = get_trust( ch );

    for ( cmd = 0; olc_table[cmd].name != NULL; cmd++ )
    {
	if ( olc_table[cmd].level <= trust )
	{
	    sprintf( buf, "{s[{t%3d{s] {q%-14s",
		olc_table[cmd].level, olc_table[cmd].name );
	    add_buf( final, buf );

	    if ( ++col % 4 == 0 )
		add_buf( final, "\n\r" );
	}
    }

    if ( col % 4 != 0 )
	add_buf( final, "{x\n\r" );
    else
	add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );

    return;
}

bool show_commands( CHAR_DATA *ch, char *argument )
{
    switch ( ch->desc->editor )
    {
	case ED_AREA:
	    show_olc_cmds( ch, aedit_table );
	    break;
	case ED_ROOM:
	    show_olc_cmds( ch, redit_table );
	    break;
	case ED_OBJECT:
	    show_olc_cmds( ch, oedit_table );
	    break;
	case ED_MOBILE:
	    show_olc_cmds( ch, medit_table );
	    break;
	case ED_MPCODE:
	    show_olc_cmds( ch, mpedit_table );
	    break;
	case ED_OPCODE:
	    show_olc_cmds( ch, opedit_table );
	    break;
	case ED_RPCODE:
	    show_olc_cmds( ch, rpedit_table );
	    break;
        case ED_HELP:
            show_olc_cmds( ch, hedit_table );
            break;
	case ED_SKILL:
	    show_olc_cmds( ch, skedit_table );
	    break;
	case ED_GROUP:
	    show_olc_cmds( ch, gredit_table );
	    break;
	case ED_CLASS:
	    show_olc_cmds( ch, class_edit_table );
	    break;
	case ED_RACE:
	    show_olc_cmds( ch, race_edit_table );
	    break;
	case ED_CLAN:
	    show_olc_cmds( ch, clan_edit_table );
	    break;
	case ED_CHANNEL:
	    show_olc_cmds( ch, channel_edit_table );
	    break;
	case ED_COMMAND:
	    show_olc_cmds( ch, command_edit_table );
	    break;
	case ED_SOCIAL:
	    show_olc_cmds( ch, social_edit_table );
	    break;
	case ED_ROOM_DAM:
	    show_olc_cmds( ch, room_damage_table );
	    break;
	case ED_SHOP:
	    show_olc_cmds( ch, shop_edit_table );
	    break;
	case ED_GAME_STAT:
	    show_olc_cmds( ch, game_edit_table );
	    break;
	case ED_PREFIX:
	case ED_SUFFIX:
	    show_olc_cmds( ch, random_edit_table );
	    break;
    }
    return FALSE;
}

AREA_DATA *get_area_data( int vnum )
{
    AREA_DATA *pArea;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        if ( pArea->vnum == vnum )
            return pArea;
    }

    return 0;
}

void edit_done( CHAR_DATA *ch )
{
    ch->desc->pEdit = NULL;
    ch->desc->editor = 0;
    do_asave( ch, "changed" ); // Add by Locke 10/23/2012
}

void aedit( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd, trust;

    EDIT_AREA( ch, pArea );

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
	return;

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "AEdit:  Insufficient security to modify area.\n\r", ch );
	edit_done( ch );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	aedit_show( ch, argument );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; aedit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= aedit_table[cmd].level
	&&   !str_prefix( command, aedit_table[cmd].name ) )
	{
	    if ( (*aedit_table[cmd].olc_fun) ( ch, argument ) )
		SET_BIT( pArea->area_flags, AREA_CHANGED );
	    return;
	}
    }

    interpret( ch, arg );
}

void shop_edit( CHAR_DATA *ch, char *argument )
{
    SHOP_DATA *pShop;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "NPC's are not allowed to use this function.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	shop_edit_show( ch, argument );
	return;
    }

    EDIT_SHOP( ch, pShop );

    if ( !str_cmp( command, "done" ) )
    {
	ch->desc->pEdit = ( void * ) ( get_mob_index( pShop->keeper ) );
	ch->desc->editor = ED_MOBILE;
	medit_show( ch, "" );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; shop_edit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= shop_edit_table[cmd].level
	&&   !str_prefix( command, shop_edit_table[cmd].name ) )
	{
	    if ( (*shop_edit_table[cmd].olc_fun) ( ch, argument ) )
		SET_BIT( get_mob_index( pShop->keeper )->area->area_flags, AREA_CHANGED );
	    return;
	}
    }

    interpret( ch, arg );
}

void skedit( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "NPC's are not allowed to use this function.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	skedit_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; skedit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= skedit_table[cmd].level
	&&   !str_prefix( command, skedit_table[cmd].name ) )
	{
	    if ( (*skedit_table[cmd].olc_fun) ( ch, argument ) )
		mud_stat.skills_changed = TRUE;
	    return;
	}
    }

    interpret( ch, arg );
}

void gredit( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "NPC's are not allowed to use this function.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	gredit_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; gredit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= gredit_table[cmd].level
	&&   !str_prefix( command, gredit_table[cmd].name ) )
	{
	    if ( (*gredit_table[cmd].olc_fun) ( ch, argument ) )
		mud_stat.skills_changed = TRUE;
	    return;
	}
    }

    interpret( ch, arg );
}

void class_edit( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "NPC's are not allowed to use this function.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	class_edit_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; class_edit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= class_edit_table[cmd].level
	&&   !str_prefix( command, class_edit_table[cmd].name ) )
	{
	    if ( (*class_edit_table[cmd].olc_fun) ( ch, argument ) )
		mud_stat.classes_changed = TRUE;
	    return;
	}
    }

    interpret( ch, arg );
}

void clan_edit( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int clan, cmd, trust;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "NPC's are not allowed to use this function.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	clan_edit_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );
	return;
    }

    EDIT_TABLE( ch, clan );
    trust = get_trust( ch );

    for ( cmd = 0; clan_edit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= clan_edit_table[cmd].level
	&&   !str_prefix( command, clan_edit_table[cmd].name ) )
	{
	    if ( !IS_IMMORTAL( ch )
	    &&   clan_edit_table[cmd].olc_fun != clan_edit_delete
	    &&   clan_table[clan].edit_clan <= 0 )
	    {
		send_to_char( "Your edit time has expired..\n\r", ch );
		return;
	    }

	    if ( ( *clan_edit_table[cmd].olc_fun ) ( ch, argument ) )
		mud_stat.clans_changed = TRUE;
	    return;
	}
    }

    interpret( ch, arg );
}

void channel_edit( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "NPC's are not allowed to use this function.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	channel_edit_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; channel_edit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= channel_edit_table[cmd].level
	&&   !str_prefix( command, channel_edit_table[cmd].name ) )
	{
	    if ( (*channel_edit_table[cmd].olc_fun) ( ch, argument ) )
		mud_stat.changed = TRUE;
	    return;
	}
    }

    interpret( ch, arg );
}

void game_edit( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "NPC's are not allowed to use this function.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	game_edit_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; game_edit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= game_edit_table[cmd].level
	&&   !str_prefix( command, game_edit_table[cmd].name ) )
	{
	    if ( (*game_edit_table[cmd].olc_fun) ( ch, argument ) )
		mud_stat.changed = TRUE;
	    return;
	}
    }

    interpret( ch, arg );
}

void command_edit( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "NPC's are not allowed to use this function.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	command_edit_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; command_edit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= command_edit_table[cmd].level
	&&   !str_prefix( command, command_edit_table[cmd].name ) )
	{
	    if ( (*command_edit_table[cmd].olc_fun) ( ch, argument ) )
		mud_stat.changed = TRUE;
	    return;
	}
    }

    interpret( ch, arg );
}

void random_edit( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "NPC's are not allowed to use this function.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	random_edit_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; random_edit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= random_edit_table[cmd].level
	&&   !str_prefix( command, random_edit_table[cmd].name ) )
	{
	    if ( (*random_edit_table[cmd].olc_fun) ( ch, argument ) )
		mud_stat.randoms_changed = TRUE;
	    return;
	}
    }

    interpret( ch, arg );
}

void race_edit( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "NPC's are not allowed to use this function.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	race_edit_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; race_edit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= race_edit_table[cmd].level
	&&   !str_prefix( command, race_edit_table[cmd].name ) )
	{
	    if ( (*race_edit_table[cmd].olc_fun) ( ch, argument ) )
		mud_stat.races_changed = TRUE;
	    return;
	}
    }

    interpret( ch, arg );
}

void social_edit( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "NPC's are not allowed to use this function.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	social_edit_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; social_edit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= social_edit_table[cmd].level
	&&   !str_prefix( command, social_edit_table[cmd].name ) )
	{
	    if ( (*social_edit_table[cmd].olc_fun) ( ch, argument ) )
		mud_stat.socials_changed = TRUE;
	    return;
	}
    }

    interpret( ch, arg );
}

void hedit( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
    {
	send_to_char( "NPC's are not allowed to use this function.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	hedit_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );
        do_asave( ch, "helps" );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; hedit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= hedit_table[cmd].level
	&&   !str_prefix( command, hedit_table[cmd].name ) )
	{
	    if ( (*hedit_table[cmd].olc_fun) ( ch, argument ) )
		mud_stat.helps_changed = TRUE;
	    return;
	}
    }

    interpret( ch, arg );
}

void room_damage_edit( CHAR_DATA *ch, char *argument )
{
    ROOM_DAMAGE_DATA *pRoom;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd, trust;

    EDIT_RDAM( ch, pRoom );

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
	return;

    if ( command[0] == '\0' )
    {
	rdam_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	ch->desc->pEdit = (void *)ch->in_room;
	ch->desc->editor = ED_ROOM;
	redit_show( ch, "" );
        do_asave( ch, "changed" );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; room_damage_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= room_damage_table[cmd].level
	&&   !str_prefix( command, room_damage_table[cmd].name ) )
	{
	    if ( (*room_damage_table[cmd].olc_fun) ( ch, argument ) )
		SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
	    return;
	}
    }

    interpret( ch, arg );
}

void redit( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom;
    AREA_DATA *pArea;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd, trust;

    EDIT_ROOM( ch, pRoom );
    pArea = pRoom->area;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( IS_NPC( ch ) )
	return;

    if ( !IS_BUILDER( ch, pArea ) )
    {
        send_to_char( "REdit:  Insufficient security to modify room.\n\r", ch );
	edit_done( ch );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	redit_show( ch, argument );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; redit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= redit_table[cmd].level
	&&   !str_prefix( command, redit_table[cmd].name ) )
	{
	    if ( (*redit_table[cmd].olc_fun) ( ch, argument ) )
		SET_BIT( pArea->area_flags, AREA_CHANGED );
	    return;
	}
    }

    interpret( ch, arg );
}

void oedit( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_STRING_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_OBJ( ch, pObj );
    pArea = pObj->area;

    if ( IS_NPC( ch ) )
	return;

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "OEdit: Insufficient security to modify area.\n\r", ch );
	edit_done( ch );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );

	if ( !IS_IMMORTAL( ch ) )
	    reset_clan_objects( pObj );

	return;
    }

    if ( command[0] == '\0' )
    {
	oedit_show( ch, argument );
	return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; oedit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= oedit_table[cmd].level
	&&   !str_prefix( command, oedit_table[cmd].name ) )
	{
	    if ( (*oedit_table[cmd].olc_fun) ( ch, argument ) )
		SET_BIT( pArea->area_flags, AREA_CHANGED );
	    return;
	}
    }

    interpret( ch, arg );
}

void medit( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int cmd, trust;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_MOB( ch, pMob );
    pArea = pMob->area;

    if ( !IS_BUILDER( ch, pArea ) )
    {
	send_to_char( "MEdit: Insufficient security to modify area.\n\r", ch );
	edit_done( ch );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	edit_done( ch );

	if ( !IS_IMMORTAL( ch ) )
	    reset_clan_mobs( pMob );

	return;
    }

    if ( command[0] == '\0' )
    {
        medit_show( ch, argument );
        return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; medit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= medit_table[cmd].level
	&&   !str_prefix( command, medit_table[cmd].name ) )
	{
	    if ( (*medit_table[cmd].olc_fun) ( ch, argument ) )
		SET_BIT( pArea->area_flags, AREA_CHANGED );
	    return;
	}
    }

    interpret( ch, arg );
}

void mpedit( CHAR_DATA *ch, char *argument )
{
    PROG_CODE *pMcode;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );

    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_PCODE( ch, pMcode );

    if ( !IS_BUILDER( ch, pMcode->area ) )
    {
	send_to_char( "That vnum is outside your area.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	mpedit_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	MOB_INDEX_DATA *pMobIndex;
	PROG_LIST *code;
	int vnum, nMatch = 0;

	for ( vnum = 0; nMatch < top_mob_index; vnum++ )
	{
	    if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	    {
		nMatch++;
		for ( code = pMobIndex->mprogs; code != NULL; code = code->next )
		{
		    if ( code->vnum == pMcode->vnum )
		    {
			free_string( code->code );
			code->code = str_dup( pMcode->code );
		    }
		}
	    }
	}

        edit_done( ch );
        return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; mpedit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= mpedit_table[cmd].level
	&&   !str_prefix( command, mpedit_table[cmd].name ) )
	{
	    if ( (*mpedit_table[cmd].olc_fun) ( ch, argument ) )
		SET_BIT( pMcode->area->area_flags, AREA_CHANGED );
	    return;
	}
    }

    interpret( ch, arg );
}

void opedit( CHAR_DATA *ch, char *argument )
{
    PROG_CODE *pMcode;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );

    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_PCODE( ch, pMcode );

    if ( !IS_BUILDER( ch, pMcode->area ) )
    {
	send_to_char( "That vnum is outside your area.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	opedit_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	OBJ_INDEX_DATA *pObjIndex;
	PROG_LIST *code;
	int vnum, nMatch = 0;

	for ( vnum = 0; nMatch < top_obj_index; vnum++ )
	{
	    if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
	    {
		nMatch++;
		for ( code = pObjIndex->oprogs; code != NULL; code = code->next )
		{
		    if ( code->vnum == pMcode->vnum )
		    {
			free_string( code->code );
			code->code = str_dup( pMcode->code );
		    }
		}
	    }
	}

        edit_done( ch );
        return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; opedit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= opedit_table[cmd].level
	&&   !str_prefix( command, opedit_table[cmd].name ) )
	{
	    if ( (*opedit_table[cmd].olc_fun) ( ch, argument ) )
		SET_BIT( pMcode->area->area_flags, AREA_CHANGED );
	    return;
	}
    }

    interpret( ch, arg );
}

void rpedit( CHAR_DATA *ch, char *argument )
{
    PROG_CODE *pMcode;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd, trust;

    smash_tilde( argument );

    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_PCODE( ch, pMcode );

    if ( !IS_BUILDER( ch, pMcode->area ) )
    {
	send_to_char( "That vnum is outside your area.\n\r", ch );
	return;
    }

    if ( command[0] == '\0' )
    {
	rpedit_show( ch, argument );
	return;
    }

    if ( !str_cmp( command, "done" ) )
    {
	ROOM_INDEX_DATA *pRoomIndex;
	PROG_LIST *code;
	int vnum, nMatch = 0;

	for ( vnum = 0; nMatch < top_room; vnum++ )
	{
	    if ( ( pRoomIndex = get_room_index( vnum ) ) != NULL )
	    {
		nMatch++;
		for ( code = pRoomIndex->rprogs; code != NULL; code = code->next )
		{
		    if ( code->vnum == pMcode->vnum )
		    {
			free_string( code->code );
			code->code = str_dup( pMcode->code );
		    }
		}
	    }
	}

        edit_done( ch );
        return;
    }

    trust = get_trust( ch );

    for ( cmd = 0; rpedit_table[cmd].name != NULL; cmd++ )
    {
	if ( trust >= rpedit_table[cmd].level
	&&   !str_prefix( command, rpedit_table[cmd].name ) )
	{
	    if ( (*rpedit_table[cmd].olc_fun) ( ch, argument ) )
		SET_BIT( pMcode->area->area_flags, AREA_CHANGED );
	    return;
	}
    }

    interpret( ch, arg );
}

void do_olc( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    int  cmd;

    argument = one_argument( argument, command );

    if ( command[0] == '\0' )
    {
        do_help( ch, "olc_edit" );
        return;
    }

    for ( cmd = 0; editor_table[cmd].name != NULL; cmd++ )
    {
	if ( !str_prefix( command, editor_table[cmd].name ) )
	{
	    (*editor_table[cmd].do_fun) ( ch, argument );
	    return;
	}
    }

    do_help( ch, "olc_edit" );
    return;
}

void do_aedit( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    int value;

    if ( IS_NPC( ch ) )
	return;

    else if ( argument[0] == '\0' )
	pArea = ch->in_room->area;

    else if ( is_number( argument ) )
    {
	value = atoi( argument );
	if ( !( pArea = get_area_data( value ) ) )
	{
	    send_to_char( "That area vnum does not exist.\n\r", ch );
	    return;
	}
    } else {
	char arg[MAX_INPUT_LENGTH];

	if ( ( pArea = area_lookup( ch, argument ) ) == NULL )
	{
	    argument = one_argument( argument, arg );

	    if ( !str_cmp( arg, "create" ) )
		aedit_create( ch, argument );
	    else
		send_to_char( "That area does not exist.\n\r", ch );

	    return;
	}
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
    	send_to_char( "Insufficient security to modify area.\n\r", ch );
    	return;
    }

    ch->desc->pEdit = (void *)pArea;
    ch->desc->editor = ED_AREA;
    aedit_show( ch, "" );
}

void check_skills( CHAR_DATA *ch, char *argument )
{
    bool found;
    int pos, gn, sk, sn;

    if ( argument[0] == '\0' )
	send_to_char( "Syntax: check < groups | levels | nogroups | ratings >\n\r", ch );

    else if ( !str_prefix( argument, "groups" ) )
    {
	char buf[MAX_STRING_LENGTH];

	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    for ( pos = 0; class_table[pos].name[0] != '\0'; pos++ )
	    {
		if ( skill_table[sn].skill_level[pos] < LEVEL_IMMORTAL
		&&   skill_table[sn].spell_fun != spell_null )
		{
		    found = FALSE;
		    buf[0] = '\0';

		    for ( gn = 0; group_table[gn].name[0] != '\0' && !found; gn++ )
		    {
			for ( sk = 0; sk < MAX_IN_GROUP && group_table[gn].spells[sk] != NULL; sk++ )
			{
			    if ( !str_cmp( group_table[gn].spells[sk], skill_table[sn].name ) )
			    {
				if ( group_table[gn].rating[pos] != -1 )
				{
				    found = TRUE;
				    break;
				} else {
				    strcat( buf, ", " );
				    strcat( buf, group_table[gn].name );
				}
			    }
			}
		    }

		    if ( !found )
			printf_to_char( ch, "Spell [%3d] %-20.20s Class [%2d] %-15.15s No group:%s.\n\r",
			    sn, skill_table[sn].name, pos,
			    class_table[pos].name, buf+1 );
		}
	    }
	}

	for ( gn = 0; group_table[gn].name[0] != '\0'; gn++ )
	{
	    for ( sk = 0; sk < MAX_IN_GROUP && group_table[gn].spells[sk] != NULL; sk++ )
	    {
		if ( skill_lookup( group_table[gn].spells[sk] ) == -1
		&&   group_lookup( group_table[gn].spells[sk] ) == -1 )
		    printf_to_char( ch, "Group [%3d] %-20.20s has invalid spell: %s.\n\r",
			gn, group_table[gn].name, group_table[gn].spells[sk] );
	    }

	    for ( pos = 0; class_table[pos].name[0] != '\0'; pos++ )
	    {
		if ( group_table[gn].rating[pos] != -1 )
		{
		    found = FALSE;
		    for ( sk = 0; sk < MAX_IN_GROUP && group_table[gn].spells[sk] != NULL; sk++ )
		    {
			if ( ( sn = skill_lookup( group_table[gn].spells[sk] ) ) != -1 )
//			||   group_lookup( group_table[gn].spells[sk] ) != -1 )
			{
			    if ( skill_table[sn].skill_level[pos] < LEVEL_IMMORTAL )
			    {
				found = TRUE;
				break;
			    }
			}
		    }

		    if ( !found )
			printf_to_char( ch, "Group [%3d] %-20.20s can be gained by class [%2d] %s, but class gets no spells from it.\n\r",
			    gn, group_table[gn].name, pos, class_table[pos].name );
		}
	    }
	}
    }

    else if ( !str_prefix( argument, "nogroups" ) )
    {
	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( skill_table[sn].spell_fun != spell_null )
	    {
		found = FALSE;
		for ( gn = 0; group_table[gn].name[0] != '\0' && !found; gn++ )
		{
		    for ( pos = 0; pos < MAX_IN_GROUP && group_table[gn].spells[pos] != NULL; pos++ )
		    {
			if ( !str_cmp( skill_table[sn].name, group_table[gn].spells[pos] ) )
			{
			    found = TRUE;
			    break;
			}
		    }
		}

		if ( !found )
		{
		    printf_to_char( ch, "Spell [%3d] %-20s belongs to no groups.\n\r",
			sn, skill_table[sn].name );
		}
	    }
	}
    }

    else if ( !str_prefix( argument, "ratings" ) )
    {
	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    for ( pos = 0; class_table[pos].name[0] != '\0'; pos++ )
	    {
		if ( skill_table[sn].skill_level[pos] < LEVEL_IMMORTAL
		&&   skill_table[sn].rating[pos] <= 0 )
		    printf_to_char( ch, "Skill [%3d] %-15.15s has invalid level/rating combinations for [%2d] %s.\n\r",
			sn, skill_table[sn].name, pos, class_table[pos].name );
	    }
	}
    }

    else if ( !str_prefix( argument, "levels" ) )
    {
	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    found = FALSE;
	    for ( pos = 0; class_table[pos].name[0] != '\0'; pos++ )
	    {
		if ( skill_table[sn].skill_level[pos] < LEVEL_IMMORTAL )
		{
		    found = TRUE;
		    break;
		}
	    }

	    if ( !found )
		printf_to_char( ch, "Skill [%3d] %-15.15s is Immortal only.\n\r",
		    sn, skill_table[sn].name );
	}
    }

    else
	check_skills( ch, "" );
}

void do_skedit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int sn;

    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "{qSyntax: {tskedit {s<{tskill_name{s>\n\r"
		      "        {tskedit {s<{tskill_number{s>\n\r"
		      "        {tskedit create {s<{tnew skill name{s>\n\r"
		      "        {tskedit check\n\r"
		      "        {tskedit list\n\r"
		      "        {tskedit disabled\n\r"
		      "        {tskedit class_copy {s<{tfrom class{s> <{tto class{s>\n\r"
		      "        {tskedit class_copy {s<{tfrom class{s> <{tto class{s> {tdiff {s<{t+/- difference{s>\n\r"
		      "        {tskedit class_copy {s<{tfrom class{s> <{tto class{s> {trange {s<{tlevel1{s> <{tlevel2{s>{x\n\r", ch );
	return;
    }

    sn = is_number( argument ) ? atoi( argument ) : skill_lookup( argument );

    if ( sn >= 0 && sn < maxSkill && skill_table[sn].name[0] != '\0' )
    {
	ch->desc->editor = ED_SKILL;
	ch->desc->pEdit = (void *)sn;
	skedit_show( ch, "" );
	return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "create" ) )
    {
        if ( skedit_create( ch, argument ) )
	    mud_stat.skills_changed = TRUE;
	return;
    }

    else if ( !str_cmp( arg, "list" ) )
    {
	BUFFER *final = new_buf( );
	char buf[64];
	sh_int col = 0;

	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    sprintf( buf, "{q%3d{s) {t%-15.15s", sn, skill_table[sn].name );
	    add_buf( final, buf );

	    if ( ++col % 4 == 0 )
		add_buf( final, "\n\r" );
	}

	if ( col % 4 != 0 )
	    add_buf( final, "{x\n\r" );
	else
	    add_buf( final, "{x" );

	page_to_char( final->string, ch );
	free_buf( final );

	return;
    }

    else if ( !str_cmp( arg, "check" ) )
    {
	check_skills( ch, argument );
	return;
    }

    else if ( !str_cmp( arg, "disabled" ) )
    {
	char buf[100];

	send_to_char( "The following skills are disabled:\n\r", ch );

	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( IS_SET( skill_table[sn].flags, SKILL_DISABLED ) )
	    {
		sprintf( buf, "  %s\n\r", skill_table[sn].name );
		send_to_char( buf, ch );
	    }
	}

	return;
    }

    else if ( !str_cmp( arg, "class_copy" ) )
    {
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH], arg4[MAX_INPUT_LENGTH];
	sh_int main_class, clone_class, diff, min_level, max_level;

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	if ( (main_class = class_lookup(arg1)) == -1 )
	{
	    send_to_char("From class not found.\n\r",ch);
	    return;
	}

	if ( (clone_class = class_lookup(arg2)) == -1 )
	{
	    send_to_char("To class not found.\n\r",ch);
	    return;
	}

	if ( arg3[0] == '\0' )
	{
	    diff	= 0;
	    min_level	= 0;
	    max_level	= MAX_LEVEL;
	}

	else if ( !str_cmp( arg3, "diff" ) )
	{
	    if ( argument[0] == '\0' )
	    {
		send_to_char( "Vary the skill levels by how much?\n\r", ch );
		return;
	    }

	    if ( !is_number( argument ) )
	    {
		send_to_char( "That is not a valid number.\n\r", ch );
		return;
	    }

	    diff	= atoi( argument );
	    min_level	= 0;
	    max_level	= MAX_LEVEL;

	    if ( diff < -30 || diff > 30 )
	    {
		send_to_char( "Skill level variations must be between -30 and 30.\n\r", ch );
		return;
	    }
	}

	else if ( !str_cmp( arg3, "range" ) )
	{
	    if ( argument[0] == '\0' )
	    {
		send_to_char( "What skill level range do you wish to copy?\n\r", ch );
		return;
	    }

	    argument = one_argument( argument, arg4 );

	    if ( arg4[0] == '\0' || argument[0] == '\0' )
	    {
		send_to_char( "Minimum level or maximum level not defined.\n\r", ch );
		return;
	    }

	    if ( !is_number( arg4 ) || !is_number( argument ) )
	    {
		send_to_char( "Level ranges must be numeric.\n\r", ch );
		return;
	    }


	    diff	= 0;
	    min_level	= atoi( arg4 );
	    max_level	= atoi( argument );

	    if ( min_level > max_level )
	    {
		send_to_char( "Minimum level must be greater than maximum level.\n\r", ch );
		return;
	    }

	    if ( min_level < 0 || MAX_LEVEL > MAX_LEVEL )
	    {
		send_to_char( "Level ranges must be between 0 and max_level.\n\r", ch );
		return;
	    }
	}

	else
	{
	    do_skedit( ch, "" );
	    return;
	}

	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( skill_table[sn].skill_level[main_class] >= min_level
	    &&   skill_table[sn].skill_level[main_class] <= max_level )
	    {
		if ( skill_table[sn].skill_level[main_class] < LEVEL_IMMORTAL )
		{
		    skill_table[sn].skill_level[clone_class] =
			URANGE( 0, skill_table[sn].skill_level[main_class]+diff,
			    LEVEL_IMMORTAL-1 );
		} else {
		    skill_table[sn].skill_level[clone_class] =
			skill_table[sn].skill_level[main_class];
		}

		skill_table[sn].rating[clone_class] =
		    skill_table[sn].rating[main_class];
	    }
	}

	for ( sn = 0; group_table[sn].name[0] != '\0'; sn++ )
	{
	    group_table[sn].rating[clone_class] =
	    group_table[sn].rating[main_class];
	}

	sprintf(buf, "%s info copied to %s.\n\r",
	    class_table[main_class].name, class_table[clone_class].name);
	send_to_char(buf, ch);

	mud_stat.skills_changed = TRUE;
	return;
    }

    send_to_char( "Invalid skill.\n\r\n\r", ch );
    do_skedit( ch, "" );
}

void do_gredit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int sn;

    if ( IS_NPC( ch ) )
	return;

    else if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: gredit <group_name>\n\r"
		      "        gredit <group_number>\n\r"
		      "        gredit list\n\r"
		      "        gredit create <new group name>\n\r", ch );
	return;
    }

    else if ( !str_cmp( argument, "list" ) )
    {
	BUFFER *final = new_buf( );
	char buf[64];
	sh_int col = 0;

	for ( sn = 0; group_table[sn].name[0] != '\0'; sn++ )
	{
	    sprintf( buf, "{q%2d{s) {t%-25s", sn, group_table[sn].name );
	    add_buf( final, buf );

	    if ( ++col % 3 == 0 )
		add_buf( final, "\n\r" );
	}

	if ( col % 3 != 0 )
	    add_buf( final, "{x\n\r" );
	else
	    add_buf( final, "{x" );

	page_to_char( final->string, ch );
	free_buf( final );

	return;
    }

    sn = is_number( argument ) ? atoi( argument ) : group_lookup( argument );

    if ( sn >= 0 && sn < maxGroup )
    {
	ch->desc->editor = ED_GROUP;
	ch->desc->pEdit = (void *)sn;
	gredit_show( ch, "" );
	return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "create" ) )
    {
	if ( gredit_create( ch, argument ) )
	    mud_stat.skills_changed = TRUE;
	return;
    }

    send_to_char( "Invalid skill group.\n\r\n\r", ch );
    do_gredit( ch, "" );
}

void do_class_edit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int sn;

    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: class_edit <class_name>\n\r"
		      "        class_edit <class_number>\n\r"
		      "        class_edit list\n\r"
		      "        class_edit disabled\n\r"
		      "        class_edit create <new class name>\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "list" ) )
    {
	BUFFER *final = new_buf( );
	char buf[64];
	sh_int col = 0;

	for ( sn = 0; class_table[sn].name[0] != '\0'; sn++ )
	{
	    sprintf( buf, "{q%2d{s) {t%-15s", sn, class_table[sn].name );
	    add_buf( final, buf );

	    if ( ++col % 4 == 0 )
		add_buf( final, "\n\r" );
	}

	if ( col % 4 != 0 )
	    add_buf( final, "{x\n\r" );
	else
	    add_buf( final, "{x" );

	page_to_char( final->string, ch );
	free_buf( final );

	return;
    }

    sn = is_number( argument ) ? atoi( argument ) : class_lookup( argument );

    if ( sn >= 0 && sn < maxClass )
    {
	ch->desc->editor = ED_CLASS;
	ch->desc->pEdit = (void *)sn;
	class_edit_show( ch, "" );
	return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "create" ) )
    {
	if ( class_edit_create( ch, argument ) )
	    mud_stat.classes_changed = TRUE;
	return;
    }

    else if ( !str_cmp( arg, "disabled" ) )
    {
	char buf[100];

	send_to_char( "The following classes are disabled:\n\r", ch );

	for ( sn = 0; class_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( class_table[sn].disabled )
	    {
		sprintf( buf,  " %s\n\r", class_table[sn].name );
		send_to_char( buf, ch );
	    }
	}

	return;
    }

    send_to_char( "Invalid class.\n\r\n\r", ch );
    do_class_edit( ch, "" );
}

void do_clan_edit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int sn;

    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: clan_edit <clan_name>\n\r"
		      "        clan_edit <clan_number>\n\r"
		      "        clan_edit list\n\r"
		      "        clan_edit create <new clan name>\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "list" ) )
    {
	BUFFER *final = new_buf( );
	char buf[64];
	sh_int col = 0;

	for ( sn = 0; clan_table[sn].name[0] != '\0'; sn++ )
	{
	    sprintf( buf, "{q%2d{s) {t%-20s", sn, clan_table[sn].name );
	    add_buf( final, buf );

	    if ( ++col % 4 == 0 )
		add_buf( final, "\n\r" );
	}

	if ( col % 4 != 0 )
	    add_buf( final, "{x\n\r" );
	else
	    add_buf( final, "{x" );

	page_to_char( final->string, ch );
	free_buf( final );

	return;
    }

    sn = is_number( argument ) ? atoi( argument ) : clan_lookup( argument );

    if ( ( sn > 0 || !str_cmp( argument, "none" ) )
    &&   sn < maxClan && clan_table[sn].name != NULL )
    {
	ch->desc->editor = ED_CLAN;
	ch->desc->pEdit = (void *)sn;
	clan_edit_show( ch, "" );
	return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "create" ) )
    {
	clan_edit_create( ch, argument );
	return;
    }

    send_to_char( "Invalid clan.\n\r\n\r", ch );
    do_clan_edit( ch, "" );
}

void do_channel_edit( CHAR_DATA *ch, char *argument )
{
    int sn;

    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: channel_edit <channel_name>\n\r"
		      "        channel_edit list\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "list" ) )
    {
	char buf[64];

	for ( sn = 0; channel_table[sn].name != NULL; sn++ )
	{
	    sprintf( buf, "{q%2d{s) {t%s{x\n\r", sn, channel_table[sn].name );
	    send_to_char( buf, ch );
	}

	return;
    }

    if ( ( sn = channel_lookup( argument ) ) == -1 )
    {
	send_to_char( "Invalid channel.\n\r", ch );
	return;
    }

    ch->desc->editor = ED_CHANNEL;
    ch->desc->pEdit = (void *)sn;
    channel_edit_show( ch, "" );
}

void do_game_edit( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
	return;

    ch->desc->editor = ED_GAME_STAT;
    game_edit_show( ch, "" );
}

void do_race_edit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int sn;

    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: race_edit <race_name>\n\r"
		      "        race_edit <race_number>\n\r"
		      "        race_edit list\n\r"
		      "        race_edit create <new race name>\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "list" ) )
    {
	BUFFER *final = new_buf( );
	char buf[64];
	sh_int col = 0;

	add_buf( final, "{qPlayable Races:\n\r" );

	for ( sn = 0; race_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( race_table[sn].pc_race && !race_table[sn].disabled )
	    {
		sprintf( buf, " {q%2d{s) {t%-15s", sn, race_table[sn].name );
		add_buf( final, buf );

		if ( ++col % 4 == 0 )
		    add_buf( final, "\n\r" );
	    }
	}

	if ( col % 4 != 0 )
	    add_buf( final, "\n\r\n\r{qDisabled Races:\n\r" );
	else
	    add_buf( final, "\n\r{qDisabled Races:\n\r" );

	col = 0;
	for ( sn = 0; race_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( race_table[sn].disabled )
	    {
		sprintf( buf, " {q%2d{s) {t%-15s", sn, race_table[sn].name );
		add_buf( final, buf );

		if ( ++col % 4 == 0 )
		    add_buf( final, "\n\r" );
	    }
	}

	if ( col % 4 != 0 )
	    add_buf( final, "\n\r\n\r{qNon-Playable Races:\n\r" );
	else
	    add_buf( final, "\n\r{qNon-Playable Races:\n\r" );

	col = 0;
	for ( sn = 0; race_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( !race_table[sn].pc_race )
	    {
		sprintf( buf, " {q%2d{s) {t%-15s", sn, race_table[sn].name );
		add_buf( final, buf );

		if ( ++col % 4 == 0 )
		    add_buf( final, "\n\r" );
	    }
	}

	if ( col % 4 != 0 )
	    add_buf( final, "{x\n\r" );
	else
	    add_buf( final, "{x" );

	page_to_char( final->string, ch );
	free_buf( final );

	return;
    }

    sn = is_number( argument ) ? atoi( argument ) : race_lookup( argument );

    if ( sn >= 0 && sn < maxRace )
    {
	ch->desc->editor = ED_RACE;
	ch->desc->pEdit = (void *)sn;
	race_edit_show( ch, "" );
	return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "create" ) )
    {
	if ( race_edit_create( ch, argument ) )
	    mud_stat.races_changed = TRUE;
	return;
    }

    send_to_char( "Invalid race.\n\r\n\r", ch );
    do_race_edit( ch, "" );
}

void do_command_edit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int sn;

    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: command_edit <command_name>\n\r"
		      "        command_edit <command_number>\n\r"
		      "        command_edit list\n\r"
		      "        command_edit create <new command>\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "list" ) )
    {
	BUFFER *final = new_buf( );
	char buf[64];
	sh_int col = 0;

	for ( sn = 0; cmd_table[sn].name[0] != '\0'; sn++ )
	{
	    sprintf( buf, "{q%3d{s) [{q%3d{s] {t%-20s",
		sn, cmd_table[sn].level, cmd_table[sn].name );
	    add_buf( final, buf );

	    if ( ++col % 3 == 0 )
		add_buf( final, "\n\r" );
	}

	if ( col % 3 != 0 )
	    add_buf( final, "{x\n\r" );
	else
	    add_buf( final, "{x" );

	page_to_char( final->string, ch );
	free_buf( final );

	return;
    }

    sn = is_number( argument ) ? atoi( argument ) : cmd_lookup( argument );

    if ( sn >= 0 )
// && sn < maxCommand )
    {
	ch->desc->editor = ED_COMMAND;
	ch->desc->pEdit = (void *)sn;
	command_edit_show( ch, "" );
	return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "create" ) )
    {
//	if ( command_edit_create( ch, argument ) )
//	    mud_stat.races_changed = TRUE;
	return;
    }

    send_to_char( "Invalid command.\n\r\n\r", ch );
    do_command_edit( ch, "" );
}

void do_social_edit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int sn;

    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Syntax: social_edit <social_name>\n\r"
		      "        social_edit <social_number>\n\r"
		      "        social_edit list\n\r"
		      "        social_edit create <new social name>\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "list" ) )
    {
	BUFFER *final = new_buf( );
	char buf[64];
	sh_int col = 0;

	for ( sn = 0; social_table[sn].name[0] != '\0'; sn++ )
	{
	    sprintf( buf, "{q%3d{s) {t%-15s", sn, social_table[sn].name );
	    add_buf( final, buf );

	    if ( ++col % 4 == 0 )
		add_buf( final, "\n\r" );
	}

	if ( col % 4 != 0 )
	    add_buf( final, "{x\n\r" );
	else
	    add_buf( final, "{x" );

	page_to_char( final->string, ch );
	free_buf( final );

	return;
    }

    sn = is_number( argument ) ? atoi( argument ) : social_lookup( argument );

    if ( sn >= 0 && sn < maxSocial )
    {
	ch->desc->editor = ED_SOCIAL;
	ch->desc->pEdit = (void *)sn;
	social_edit_show( ch, "" );
	return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg );

    if ( !str_cmp( arg, "create" ) )
    {
	if ( social_edit_create( ch, argument ) )
	    mud_stat.socials_changed = TRUE;
	return;
    }

    send_to_char( "Invalid social.\n\r\n\r", ch );
    do_social_edit( ch, "" );
}

void do_hedit( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char arg1[MAX_INPUT_LENGTH];
    char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];

    if ( IS_NPC( ch ) )
	return;

    strcpy( arg1,argument );

    if( argument[0] != '\0' )
    {
        argall[0] = '\0';
        while (argument[0] != '\0' )
        {
            argument = one_argument(argument,argone);
            if (argall[0] != '\0')
                strcat(argall," ");
            strcat(argall,argone);
        }

        for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
        {
            if ( is_name( argall, pHelp->keyword ) )
            {
                ch->desc->pEdit=(void *)pHelp;
                ch->desc->editor= ED_HELP;
		hedit_show( ch, "" );
                return;
            }
        }
    }

    argument = one_argument(arg1, arg1);

    if ( !str_cmp( arg1, "make" ) || !str_cmp( arg1, "create" ) )
    {
        if (argument[0] == '\0')
        {
            send_to_char("Syntax: edit help create [topic]\n\r",ch);
            return;
        }

        hedit_make(ch, argument);
        return;
    }

    send_to_char( "HEdit:  There is no default help to edit.\n\r", ch );
}

void do_redit( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( IS_NPC(ch) )
	return;

    if ( arg[0] == '\0' )
	pRoom = ch->in_room;

    else if ( is_number( arg ) )
    {
	if ( !( pRoom = get_room_index( atoi( arg ) ) ) )
	{
	    send_to_char( "REdit:  That vnum does not exist.\n\r", ch );
	    return;
	}
    }

    else if ( !str_cmp( arg, "reset" ) )
    {
	if ( !IS_BUILDER( ch, ch->in_room->area ) )
	{
	    send_to_char( "Insufficient security to modify room.\n\r", ch );
            return;
	}

	reset_room( ch->in_room, NULL );
	send_to_char( "Room reset.\n\r", ch );
	return;
    }

    else if ( !str_cmp( arg, "create" ) )
    {
	if ( redit_create( ch, argument ) )
	{
	    if ( IS_SET( ch->configure, CONFIG_REDIT_GOTO ) )
	    {
		char_from_room( ch );
		char_to_room( ch, (ROOM_INDEX_DATA *)ch->desc->pEdit );
	    }
	}
	return;
    }

    else
    {
	send_to_char( "REdit:  There is no default object to edit.\n\r", ch );
	return;
    }

    if ( !IS_BUILDER( ch, pRoom->area ) )
    {
	send_to_char( "Insuficient security to modify rooms.\n\r" , ch );
	return;
    }

    if ( IS_SET( ch->configure, CONFIG_REDIT_GOTO ) )
    {
	char_from_room( ch );
	char_to_room( ch, pRoom );
    }

    ch->desc->pEdit = (void *)pRoom;
    ch->desc->editor = ED_ROOM;
    redit_show( ch, "" );
}

void do_oedit( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObj;
    char arg1[MAX_STRING_LENGTH];
    int value;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg1 );

    if ( is_number( arg1 ) )
    {
	value = atoi( arg1 );
	if ( !( pObj = get_obj_index( value ) ) )
	{
	    send_to_char( "OEdit:  That vnum does not exist.\n\r", ch );
	    return;
	}

	if ( !IS_BUILDER( ch, pObj->area ) )
	{
	    send_to_char( "Insuficient security to modify objects.\n\r" , ch );
	    return;
	}

	ch->desc->pEdit = (void *)pObj;
	ch->desc->editor = ED_OBJECT;
	oedit_show( ch, "" );
	return;
    }
    else
    {
	if ( !str_cmp( arg1, "create" ) )
	{
	    oedit_create( ch, argument );
	    return;
	}
    }

    send_to_char( "OEdit:  There is no default object to edit.\n\r", ch );
    return;
}

void do_medit( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMob;
    int value;
    char arg1[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg1 );

    if ( is_number( arg1 ) )
    {
	value = atoi( arg1 );
	if ( !( pMob = get_mob_index( value ) ))
	{
	    send_to_char( "MEdit:  That vnum does not exist.\n\r", ch );
	    return;
	}

	if ( !IS_BUILDER( ch, pMob->area ) )
	{
	    send_to_char( "Insuficient security to modify mobs.\n\r" , ch );
	    return;
	}

	ch->desc->pEdit = (void *)pMob;
	ch->desc->editor = ED_MOBILE;
	medit_show( ch, "" );
	return;
    }
    else
    {
	if ( !str_cmp( arg1, "create" ) )
	{
	    medit_create( ch, argument );
	    return;
	}
    }

    send_to_char( "MEdit:  There is no default mobile to edit.\n\r", ch );
    return;
}

void do_mpedit( CHAR_DATA *ch, char *argument )
{
    PROG_CODE *pMcode;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );

    if ( is_number( command ) )
    {
	if (! (pMcode = get_prog_index( atoi(command), PRG_MPROG ) ) )
	{
	    send_to_char("MPEdit: That vnum does not exist.\n\r",ch);
            return;
	}

	if ( !IS_BUILDER( ch, pMcode->area ) )
	{
	    send_to_char( "That vnum is outside your area.\n\r", ch );
	    return;
	}

	ch->desc->pEdit=(void *)pMcode;
	ch->desc->editor= ED_MPCODE;
	mpedit_show( ch, "" );
    }

    else if ( !str_cmp( command, "create" ) )
        mpedit_create( ch, argument );

    else
	send_to_char(	"Syntax: mpedit create [vnum]\n\r"
			"        mpedit [vnum]\n\r", ch );
}

void do_opedit( CHAR_DATA *ch, char *argument )
{
    PROG_CODE *pMcode;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );

    if ( is_number( command ) )
    {
       if (! (pMcode = get_prog_index( atoi(command), PRG_OPROG ) ) )
       {
           send_to_char("OPEdit: That vnum does not exist.\n\r",ch);
           return;
       }

	if ( !IS_BUILDER( ch, pMcode->area ) )
	{
	    send_to_char( "That vnum is outside your area.\n\r", ch );
	    return;
	}

	ch->desc->pEdit=(void *)pMcode;
	ch->desc->editor= ED_OPCODE;
	opedit_show( ch, "" );
    }

    else if ( !str_cmp( command, "create" ) )
        opedit_create( ch, argument );

    else
	send_to_char(	"Syntax: opedit create [vnum]\n\r"
			"        opedit [vnum]\n\r", ch );
}

void do_rpedit( CHAR_DATA *ch, char *argument )
{
    PROG_CODE *pMcode;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );

    if ( is_number( command ) )
    {
	if (! (pMcode = get_prog_index( atoi(command), PRG_RPROG ) ) )
	{
           send_to_char("RPEdit: That vnum does not exist.\n\r",ch);
           return;
	}

	if ( !IS_BUILDER( ch, pMcode->area ) )
	{
	    send_to_char( "That vnum is outside your area.\n\r", ch );
	    return;
	}

	ch->desc->pEdit=(void *)pMcode;
	ch->desc->editor= ED_RPCODE;
	rpedit_show( ch, "" );
    }

    else if ( !str_cmp( command, "create" ) )
        rpedit_create( ch, argument );

    else
	send_to_char(	"Syntax: rpedit create [vnum]\n\r"
			"        rpedit [vnum]\n\r", ch );
}

void display_resets( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA	*pRoom;
    RESET_DATA		*pReset;
    MOB_INDEX_DATA	*pMob = NULL;
    char 		buf   [ MAX_STRING_LENGTH ];
    char 		final [ MAX_STRING_LENGTH ];
    int 		iReset = 0;

    pRoom = ch->in_room;

    final[0]  = '\0';

    send_to_char (
  " {wN{Do{w.  L{Doad{ws    D{Descriptio{wn       L{Docatio{wn         V{Dnu{wm   {RMW MR {wD{Descriptio{wn   P{Dcn{wt\n\r"
  "{y==== ======== ============= =================== ======== ===== ============= ====\n\r", ch );

    for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
    {
	OBJ_INDEX_DATA  *pObj;
	MOB_INDEX_DATA  *pMobIndex;
	OBJ_INDEX_DATA  *pObjIndex;
	OBJ_INDEX_DATA  *pObjToIndex;
	ROOM_INDEX_DATA *pRoomIndex;

	final[0] = '\0';
	sprintf( final, "{c[{m%2d{c] ", ++iReset );

	switch ( pReset->command )
	{
	default:
	    sprintf( buf, "{xBad reset command: %c.\n\r", pReset->command );
	    strcat( final, buf );
	    break;

	case 'M':
	    if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) )
	    {
                sprintf( buf, "{xLoad Mobile - Bad Mob %d\n\r", pReset->arg1 );
                strcat( final, buf );
                continue;
	    }

	    if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
	    {
                sprintf( buf, "{xLoad Mobile - Bad Room %d\n\r", pReset->arg3 );
                strcat( final, buf );
                continue;
	    }

            pMob = pMobIndex;

	    sprintf( buf, "{MM{c[{m%5d{c]{x %s {gin room             {CR{c[{m%5d{c] {R%2d{r-{R%2d{x %s {m%3d%%\n\r",
		pReset->arg1, end_string(pMob->short_descr,13),
		pReset->arg3, pReset->arg2, pReset->arg4,
		begin_string(pRoomIndex->name,13), pReset->percent );
            strcat( final, buf );

	    /*
	     * Check for pet shop.
	     * -------------------
	     */
	    {
		ROOM_INDEX_DATA *pRoomIndexPrev;

		pRoomIndexPrev = get_room_index( pRoomIndex->vnum - 1 );
		if ( pRoomIndexPrev
		    && IS_SET( pRoomIndexPrev->room_flags, ROOM_PET_SHOP ) )
		{
		    final[12] = 'B';
                    final[13] = 'P';
		}
	    }

	    break;

	case 'O':
	    if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
	    {
                sprintf( buf, "{xLoad Object - Bad Object %d\n\r",
		    pReset->arg1 );
                strcat( final, buf );
                continue;
	    }

            pObj       = pObjIndex;

	    if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
	    {
                sprintf( buf, "{xLoad Object - Bad Room %d\n\r", pReset->arg3 );
                strcat( final, buf );
                continue;
	    }

            sprintf( buf, "{YO{c[{m%5d{c] {x%s {gin room             {CR{c[{m%5d{c]{x       %s {m%3d%%\n\r",
		pReset->arg1, end_string(pObj->short_descr,13),
		pReset->arg3, begin_string(pRoomIndex->name,13),
		pReset->percent );
            strcat( final, buf );

	    break;

	case 'P':
	    if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
	    {
                sprintf( buf, "{xPut Object - Bad Object %d\n\r",
                    pReset->arg1 );
                strcat( final, buf );
                continue;
	    }

            pObj       = pObjIndex;

	    if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
	    {
                sprintf( buf, "{xPut Object - Bad To Object %d\n\r",
                    pReset->arg3 );
                strcat( final, buf );
                continue;
	    }

	    sprintf( buf, "{YI{c[{m%5d{c]{x %s {ginside              {YO{c[{m%5d{c] {R%2d{r-{R%2d{x %s {m%3d%%\n\r",
		pReset->arg1,
		end_string(pObj->short_descr,13),
		pReset->arg3,
		pReset->arg2,
		pReset->arg4,
		begin_string(pObjToIndex->short_descr,13),
		pReset->percent );
            strcat( final, buf );

	    break;

	case 'G':
	case 'E':
	    if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) )
	    {
                sprintf( buf, "{xGive/Equip Object - Bad Object %d\n\r",
                    pReset->arg1 );
                strcat( final, buf );
                continue;
	    }

            pObj       = pObjIndex;

	    if ( !pMob )
	    {
                sprintf( buf, "{xGive/Equip Object - No Previous Mobile\n\r" );
                strcat( final, buf );
                break;
	    }

	    if ( pMob->pShop && pReset->arg3 == WEAR_NONE )
	    {
	    sprintf( buf,
		"{YO{c[{m%5d{c] {x%s {gin the inventory of {GS{c[{m%5d{c]{x       %s {m%3d%%\n\r",
		pReset->arg1,
		end_string(pObj->short_descr,13),
		pMob->vnum,
		begin_string(pMob->short_descr,13), pReset->percent );
	    }
	    else
	    sprintf( buf,
		"{YO{c[{m%5d{c]{x %s {g%-19.19s {MM{c[{m%5d{c]{x       %s {m%3d%%\n\r",
		pReset->arg1,
		end_string(pObj->short_descr,13),
		(pReset->command == 'G') ?
		    flag_string( wear_loc_strings, WEAR_NONE )
		  : flag_string( wear_loc_strings, pReset->arg3 ),
		  pMob->vnum,
		begin_string(pMob->short_descr,13), pReset->percent );
	    strcat( final, buf );

	    break;

	case 'D':
	    pRoomIndex = get_room_index( pReset->arg1 );
	    sprintf( buf, "{CR{c[{m%5d{c] {x%s {gdoor of %-19.19s reset to %s\n\r",
		pReset->arg1,
		capitalize( dir_name[ pReset->arg2 ] ),
		pRoomIndex->name,
		flag_string( door_resets, pReset->arg3 ) );
	    strcat( final, buf );

	    break;

	case 'R':
	    if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) )
	    {
		sprintf( buf, "{xRandomize Exits - Bad Room %d\n\r",
		    pReset->arg1 );
		strcat( final, buf );
		continue;
	    }

	    sprintf( buf, "{CR{c[{m%5d{c] {gExits are randomized in %s\n\r",
		pReset->arg1, pRoomIndex->name);
	    strcat( final, buf );

	    break;
	}
	strcat(final,"{x");
	page_to_char( final, ch );
    }
    return;
}

void add_reset( ROOM_INDEX_DATA *room, RESET_DATA *pReset, int index )
{
    RESET_DATA *reset;
    int iReset = 0;

    if ( !room->reset_first )
    {
	room->reset_first	= pReset;
	pReset->next		= NULL;
	return;
    }

    index--;

    if ( index == 0 )	/* First slot (1) selected. */
    {
	pReset->next = room->reset_first;
	room->reset_first = pReset;
	return;
    }

    for ( reset = room->reset_first; reset->next; reset = reset->next )
    {
	if ( ++iReset == index )
	    break;
    }

    pReset->next	= reset->next;
    reset->next		= pReset;
    return;
}

void do_resets( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char arg5[MAX_INPUT_LENGTH];
    char arg6[MAX_INPUT_LENGTH];
    char arg7[MAX_INPUT_LENGTH];
    RESET_DATA *pReset = NULL;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );
    argument = one_argument( argument, arg5 );
    argument = one_argument( argument, arg6 );
    argument = one_argument( argument, arg7 );

    if ( !IS_BUILDER( ch, ch->in_room->area ) )
    {
	send_to_char( "Resets: Invalid security for editing this area.\n\r",ch);
	return;
    }

    if ( arg1[0] == '\0' )
    {
	if ( ch->in_room->reset_first )
	{
	    send_to_char("{wR{Deset{ws{R: {MM {D= {wmobile{D, {CR {D= {wroom{D, {YO {D= {wobject{D, {BP {D= {wpet{D, {GS {D= {wshopkeeper{x\n\r",ch);
	    display_resets(ch);
	}
	else
	    send_to_char("No resets in this room.\n\r",ch);
    }

    else if ( is_number( arg1 ) )
    {
	ROOM_INDEX_DATA *pRoom = ch->in_room;

	if ( !str_cmp( arg2, "delete" ) )
	{
	    int insert_loc = atoi( arg1 );

	    if ( !ch->in_room->reset_first )
	    {
		send_to_char( "No resets in this room.\n\r", ch );
		return;
	    }

	    if ( insert_loc-1 <= 0 )
	    {
		pReset = pRoom->reset_first;
		pRoom->reset_first = pRoom->reset_first->next;
	    }
	    else
	    {
		int iReset = 0;
		RESET_DATA *prev = NULL;

		for ( pReset = pRoom->reset_first;
		  pReset;
		  pReset = pReset->next )
		{
		    if ( ++iReset == insert_loc )
			break;
		    prev = pReset;
		}

		if ( !pReset )
		{
		    send_to_char( "Reset not found.\n\r", ch );
		    return;
		}

		if ( prev )
		    prev->next = prev->next->next;
		else
		    pRoom->reset_first = pRoom->reset_first->next;
	    }

	    free_reset_data( pReset );
	    send_to_char( "Reset deleted.\n\r", ch );
	    SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
	}
	else
	if ( (!str_cmp( arg2, "mob" ) && is_number( arg3 ))
	  || (!str_cmp( arg2, "obj" ) && is_number( arg3 )) )
	{
	    if ( !str_cmp( arg2, "mob" ) )
	    {
		if ( atoi( arg3 ) < ch->in_room->area->min_vnum
		||   atoi( arg3 ) > ch->in_room->area->max_vnum
		||   get_mob_index( is_number(arg3) ? atoi( arg3 ) : 1 ) == NULL )
		{
		    send_to_char("Mob doesn't exist or is from another area.\n\r",ch);
		    return;
		}

		pReset = new_reset_data();
		pReset->command = 'M';
		pReset->arg1    = atoi( arg3 );
		pReset->arg2    = is_number( arg4 ) ? atoi( arg4 ) : 1; /* Max # */
		pReset->arg3    = ch->in_room->vnum;
		pReset->arg4	= is_number( arg5 ) ? atoi( arg5 ) : 1; /* Min # */
		pReset->percent	= is_number( arg6 ) ?
				  URANGE( 1, atoi( arg6 ), 100 ) : 100;
	    }
	    else if ( !str_cmp( arg2, "obj" ) )
	    {
		if ( atoi( arg3 ) < ch->in_room->area->min_vnum
		||   atoi( arg3 ) > ch->in_room->area->max_vnum )
		{
		    send_to_char( "No such object existsin this area.\n\r", ch );
		    return;
		}

		pReset = new_reset_data();
		pReset->arg1    = atoi( arg3 );

		if ( !str_prefix( arg4, "inside" ) )
		{
		    OBJ_INDEX_DATA *temp;

		    temp = get_obj_index(is_number(arg5) ? atoi(arg5) : 1);

		    if (!temp)
		    {
          		send_to_char( "Couldn't find Object 2.\n\r",ch);
          		return;
		    }

		    if ( ( temp->item_type != ITEM_CONTAINER ) &&
		         ( temp->item_type != ITEM_CORPSE_NPC ) )
		    {
		        send_to_char( "Object 2 not a container.\n\r", ch);
		        return;
		    }

		    pReset->command = 'P';
		    pReset->arg2    = is_number( arg6 ) ? atoi( arg6 ) : 1;
		    pReset->arg3    = is_number( arg5 ) ? atoi( arg5 ) : 1;
		    pReset->arg4    = is_number( arg7 ) ? atoi( arg7 ) : 1;
		    pReset->percent = is_number( argument ) ?
				      URANGE( 1, atoi( argument ), 100 ) : 100;
		}
		else if ( !str_cmp( arg4, "room" ) )
		{
		    if (get_obj_index(atoi(arg3)) == NULL)
		    {
		         send_to_char( "Vnum doesn't exist.\n\r",ch);
		         return;
		    }
		    pReset->command	= 'O';
		    pReset->arg2	= 0;
		    pReset->arg3	= ch->in_room->vnum;
		    pReset->arg4	= 0;
		    pReset->percent	= is_number( arg5 ) ?
					  URANGE( 1, atoi( arg5 ), 100 ) : 100;
		}
		else
		{
		    if ( flag_value( wear_loc_flags, arg4 ) == NO_FLAG )
		    {
			send_to_char( "Resets: '? wear-loc'\n\r", ch );
			return;
		    }

		    if (get_obj_index(atoi(arg3)) == NULL)
		    {
		         send_to_char( "Vnum doesn't exist.\n\r",ch);
		         return;
		    }

		    pReset->arg1 = atoi(arg3);
		    pReset->arg3 = flag_value( wear_loc_flags, arg4 );
		    if ( pReset->arg3 == WEAR_NONE )
			pReset->command = 'G';
		    else
			pReset->command = 'E';
		    pReset->percent	= is_number( arg5 ) ?
					  URANGE( 1, atoi( arg5 ), 100 ) : 100;
		}
	    }
	    add_reset( ch->in_room, pReset, atoi( arg1 ) );
	    SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
	    send_to_char( "Reset added.\n\r", ch );
	}
	else if (!str_cmp( arg2, "random") && is_number(arg3))
	{
	    if (atoi(arg3) < 1 || atoi(arg3) > 6)
	    {
		send_to_char("Invalid argument.\n\r", ch);
		return;
	    }
	    pReset = new_reset_data ();
	    pReset->command = 'R';
	    pReset->arg1 = ch->in_room->vnum;
	    pReset->arg2 = atoi(arg3);
	    add_reset( ch->in_room, pReset, atoi( arg1 ) );
	    SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
	    send_to_char( "Random exits reset added.\n\r", ch);
	    pReset->percent = 100;
	}
	else
	{
	    send_to_char( "Syntax: RESET <number> OBJ <vnum> <wear_loc>\n\r", ch );
	    send_to_char( "        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]\n\r", ch );
	    send_to_char( "        RESET <number> OBJ <vnum> room\n\r", ch );
	    send_to_char( "        RESET <number> MOB <vnum> [max #x area] [max #x room]\n\r", ch );
	    send_to_char( "        RESET <number> DELETE\n\r", ch );
	    send_to_char( "        RESET <number> RANDOM [#x exits]\n\r", ch);
	}
    }
    return;
}

void do_alist( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;
    BUFFER *final = new_buf();
    bool restrict = FALSE;
    char buf[MAX_STRING_LENGTH];
    bool fLinked = FALSE;
    sh_int total = 0, matches = 0;
    int vnum = 0;

    if ( argument[0] != '\0' )
    {
	if ( is_number( argument ) )
	    vnum = atoi( argument );
	else if ( !str_cmp( argument, "linked" ) )
	    fLinked = TRUE;
	restrict = TRUE;
    }

    sprintf( buf, "{w[{c%3s{w] [{B%-26s{w] ({W%s{w-{W%s{w) [{c%-13s{w] {g%3s {w[{GFlags{w]\n\r",
       "Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec" );
    add_buf( final, buf );

    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {
	total++;

	if ( ( restrict && vnum == 0 && !fLinked
	&&     !is_name( argument, strip_color( pArea->name ) ) )
	||   ( restrict && vnum != 0
	&&     ( pArea->min_vnum > vnum || pArea->max_vnum < vnum ) )
	||   ( fLinked && IS_SET( pArea->area_flags, AREA_UNLINKED ) ) )
	    continue;

	sprintf( buf, "{w[{c%3d{w] {B%s {w({W%-5d{w-{W%5d{w) {c%-15.15s {w[{g%d{w] {w[%s%s%s%s%s{w]\n\r",
	    pArea->vnum, end_string( pArea->name, 28 ), pArea->min_vnum,
	    pArea->max_vnum, pArea->file_name, pArea->security,
	    IS_SET( pArea->area_flags, AREA_CHANGED )	? "{YC" : " ",
	    IS_SET( pArea->area_flags, AREA_UNLINKED )	? "{GU" : " ",
	    IS_SET( pArea->area_flags, AREA_SPECIAL )	? "{RS" : " ",
	    IS_SET( pArea->area_flags, AREA_NO_RUN )	? "{rR" : " ",
	    IS_SET( pArea->area_flags, AREA_NO_QUEST )	? "{qQ" : " " );
	add_buf( final, buf );
	matches++;
    }

    sprintf( buf, "\n\r{BFound {W%d {Bmatches out of {W%d {Btotal areas.{x\n\r",
	matches, total );
    add_buf( final, buf );

    page_to_char( final->string, ch );
    free_buf( final );
    return;
}

bool run_olc_editor( DESCRIPTOR_DATA *d )
{
    switch ( d->editor )
    {
	case ED_AREA:
	    aedit( d->character, d->incomm );
	    break;
	case ED_ROOM:
	    redit( d->character, d->incomm );
	    break;
	case ED_OBJECT:
	    oedit( d->character, d->incomm );
	    break;
	case ED_MOBILE:
	    medit( d->character, d->incomm );
	    break;
	case ED_MPCODE:
    	    mpedit( d->character, d->incomm );
    	    break;
	case ED_OPCODE:
    	    opedit( d->character, d->incomm );
    	    break;
	case ED_RPCODE:
    	    rpedit( d->character, d->incomm );
    	    break;
	case ED_HELP:
            hedit( d->character, d->incomm );
            break;
	case ED_SHOP:
	    shop_edit( d->character, d->incomm );
	    break;
	case ED_SKILL:
	    skedit( d->character, d->incomm );
	    break;
	case ED_GROUP:
	    gredit( d->character, d->incomm );
	    break;
	case ED_CLASS:
	    class_edit( d->character, d->incomm );
	    break;
	case ED_CLAN:
	    clan_edit( d->character, d->incomm );
	    break;
	case ED_COMMAND:
	    command_edit( d->character, d->incomm );
	    break;
	case ED_CHANNEL:
	    channel_edit( d->character, d->incomm );
	    break;
	case ED_RACE:
	    race_edit( d->character, d->incomm );
	    break;
	case ED_SOCIAL:
	    social_edit( d->character, d->incomm );
	    break;
	case ED_ROOM_DAM:
	    room_damage_edit( d->character, d->incomm );
	    break;
	case ED_GAME_STAT:
	    game_edit( d->character, d->incomm );
	    break;
	case ED_PREFIX:
	case ED_SUFFIX:
	    random_edit( d->character, d->incomm );
	    break;
	default:
	    return FALSE;
    }

    return TRUE;
}

const struct editor_cmd_type editor_table[] =
{
    { "area",		do_aedit	},
    { "room",		do_redit	},
    { "object",		do_oedit	},
    { "mobile",		do_medit	},
    { "mpcode",		do_mpedit	},
    { "opcode",		do_opedit	},
    { "rpcode",		do_rpedit	},
    { "help",		do_hedit	},
    { "skill",		do_skedit	},
    { "group",		do_gredit	},
    { "class",		do_class_edit	},
    { "clan",		do_clan_edit	},
    { "race",		do_race_edit	},
    { "social",		do_social_edit	},
    { "channel",	do_channel_edit	},
    { "prefix",		do_prefix_edit	},
    { "suffix",		do_suffix_edit	},
    { NULL,		0,		}
};

void list_programs( CHAR_DATA *ch, PROG_CODE *prog, char *argument )
{
    AREA_DATA *pArea = NULL;
    BUFFER *final;
    char buf[MAX_STRING_LENGTH];
    bool fAll = FALSE;
    sh_int count = 1;

    if ( !str_cmp( argument, "all" ) )
	fAll = TRUE;

    else if ( ( pArea = get_area_from_editor( ch ) ) == NULL )
    {
	send_to_char( "You appear to be in a non-existent area.\n\r", ch );
	return;
    }

    final = new_buf( );

    for ( ; prog != NULL; prog = prog->next )
    {
	if ( !fAll
	&&   ( prog->vnum < pArea->min_vnum
	||     prog->vnum > pArea->max_vnum ) )
	    continue;

	sprintf( buf, "{t%3d{s> {qVnum: {t%-5d {qAuthor: {t%-10s {qName: {s%s\n\r",
	    count, prog->vnum, prog->author, prog->name );
	add_buf( final, buf );
	count++;
    }

    if ( count == 1 )
	add_buf( final, "No programs found in this area.  Perhaps try \"(r/m/o)plist all\".\n\r" );
    else
	add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );
}

void do_mplist( CHAR_DATA *ch, char *argument )
{
    list_programs( ch, mprog_list, argument );
    return;
}

void do_oplist( CHAR_DATA *ch, char *argument )
{
    list_programs( ch, oprog_list, argument );
    return;
}

void do_rplist( CHAR_DATA *ch, char *argument )
{
    list_programs( ch, rprog_list, argument );
    return;
}
