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

#define ML 	MAX_LEVEL	/* implementor */
#define L1	MAX_LEVEL - 1  	/* creator */
#define L2	MAX_LEVEL - 2	/* supreme being */
#define L3	MAX_LEVEL - 3	/* deity */
#define L4 	MAX_LEVEL - 4	/* god */
#define L5	MAX_LEVEL - 5	/* immortal */
#define L6	MAX_LEVEL - 6	/* demigod */
#define L7	MAX_LEVEL - 7	/* angel */
#define L8	MAX_LEVEL - 8	/* avatar */
#define IM	LEVEL_IMMORTAL 	/* angel */

struct	cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
    sh_int		position;
    sh_int		level;
    sh_int		tier;
    sh_int		log;
    sh_int              show;
};

extern	const	struct	cmd_type	cmd_table	[];

DECLARE_DO_FUN( do_accept	);
DECLARE_DO_FUN( do_account	);
DECLARE_DO_FUN( do_aclear	);
DECLARE_DO_FUN( do_addlag	);
DECLARE_DO_FUN(	do_advance	);
DECLARE_DO_FUN( do_aedit	);
DECLARE_DO_FUN( do_affects	);
DECLARE_DO_FUN( do_afk		);
DECLARE_DO_FUN( do_alia		);
DECLARE_DO_FUN( do_alias	);
DECLARE_DO_FUN( do_alist	);
DECLARE_DO_FUN(	do_allow	);
DECLARE_DO_FUN( do_ambush	);
DECLARE_DO_FUN( do_announce	);
DECLARE_DO_FUN( do_answer	);
DECLARE_DO_FUN(	do_areas	);
DECLARE_DO_FUN( do_arena	);
DECLARE_DO_FUN( do_arenastatus	);
DECLARE_DO_FUN( do_asave	);
DECLARE_DO_FUN( do_ask		);
DECLARE_DO_FUN(	do_at		);
DECLARE_DO_FUN(	do_auction	);
DECLARE_DO_FUN( do_autoaction	);
DECLARE_DO_FUN(	do_backstab	);
DECLARE_DO_FUN( do_backup	);
DECLARE_DO_FUN( do_balance	);
DECLARE_DO_FUN(	do_bamfin	);
DECLARE_DO_FUN(	do_bamfout	);
DECLARE_DO_FUN(	do_ban		);
DECLARE_DO_FUN( do_barter	);
DECLARE_DO_FUN( do_bash		);
DECLARE_DO_FUN( do_berserk	);
DECLARE_DO_FUN( do_bet		);
DECLARE_DO_FUN( do_bonus	);
DECLARE_DO_FUN( do_bounty	);
DECLARE_DO_FUN(	do_brandish	);
DECLARE_DO_FUN( do_brew		);
DECLARE_DO_FUN(	do_bug		);
DECLARE_DO_FUN(	do_buy		);
DECLARE_DO_FUN( do_camouflage	);
DECLARE_DO_FUN(	do_cast		);
DECLARE_DO_FUN( do_censor	);
DECLARE_DO_FUN( do_clan_edit	);
DECLARE_DO_FUN( do_clan_manage	);
DECLARE_DO_FUN( do_cloak	);
DECLARE_DO_FUN(	do_cdonate	);
DECLARE_DO_FUN(	do_cgossip	);
DECLARE_DO_FUN( do_challenge	);
DECLARE_DO_FUN( do_changes	);
DECLARE_DO_FUN( do_channels	);
DECLARE_DO_FUN( do_channel_edit	);
DECLARE_DO_FUN( do_charts	);
DECLARE_DO_FUN( do_chat		);
DECLARE_DO_FUN( do_checkeq	);
DECLARE_DO_FUN( do_checkstats	);
DECLARE_DO_FUN(	do_circle	);
DECLARE_DO_FUN( do_clantalk	);
DECLARE_DO_FUN( do_clanlist	);
DECLARE_DO_FUN(	do_class	);
DECLARE_DO_FUN( do_class_edit	);
DECLARE_DO_FUN( do_clead	);
DECLARE_DO_FUN( do_clone	);
DECLARE_DO_FUN(	do_close	);
DECLARE_DO_FUN( do_colour	);
DECLARE_DO_FUN( do_combat	);
DECLARE_DO_FUN(	do_commands	);
DECLARE_DO_FUN( do_compress	);
DECLARE_DO_FUN(	do_compare	);
DECLARE_DO_FUN( do_configure	);
DECLARE_DO_FUN(	do_consider	);
DECLARE_DO_FUN( do_copyover	);
DECLARE_DO_FUN(	do_corner	);
DECLARE_DO_FUN( do_count	);
DECLARE_DO_FUN(	do_credits	);
DECLARE_DO_FUN( do_crush	);
DECLARE_DO_FUN( do_curse	);
DECLARE_DO_FUN( do_customwho	);
DECLARE_DO_FUN( do_cyclone	);
DECLARE_DO_FUN( do_decline	);
DECLARE_DO_FUN( do_delet	);
DECLARE_DO_FUN( do_delete	);
DECLARE_DO_FUN( do_demote	);
DECLARE_DO_FUN( do_deposit	);
DECLARE_DO_FUN(	do_description	);
DECLARE_DO_FUN( do_devote	);
DECLARE_DO_FUN( do_dirt		);
DECLARE_DO_FUN(	do_disarm	);
DECLARE_DO_FUN( do_disable      ); 
DECLARE_DO_FUN(	do_disconnect	);
DECLARE_DO_FUN( do_doas		);
DECLARE_DO_FUN(	do_donate	);
DECLARE_DO_FUN(	do_down		);
DECLARE_DO_FUN(	do_drink	);
DECLARE_DO_FUN(	do_drop		);
DECLARE_DO_FUN(	do_east		);
DECLARE_DO_FUN(	do_eat		);
DECLARE_DO_FUN(	do_echo		);
DECLARE_DO_FUN(	do_emote	);
DECLARE_DO_FUN( do_engage	);
DECLARE_DO_FUN( do_engineer	);
DECLARE_DO_FUN( do_enter	);
DECLARE_DO_FUN( do_envenom	);
DECLARE_DO_FUN(	do_equipment	);
DECLARE_DO_FUN(	do_examine	);
DECLARE_DO_FUN(	do_exits	);
DECLARE_DO_FUN( do_exp_mod	);
DECLARE_DO_FUN(	do_feed		);
DECLARE_DO_FUN(	do_fill		);
DECLARE_DO_FUN( do_finger	);
DECLARE_DO_FUN( do_flag		);
DECLARE_DO_FUN( do_flame	);
DECLARE_DO_FUN(	do_flee		);
DECLARE_DO_FUN(	do_follow	);
DECLARE_DO_FUN(	do_force	);
DECLARE_DO_FUN( do_forest_meld	);
DECLARE_DO_FUN(	do_forge	);
DECLARE_DO_FUN(	do_forget	);
DECLARE_DO_FUN(	do_freeze	);
DECLARE_DO_FUN( do_ftick	);
DECLARE_DO_FUN( do_gain		);
DECLARE_DO_FUN( do_game_edit	);
DECLARE_DO_FUN(	do_get		);
DECLARE_DO_FUN( do_ghost	);
DECLARE_DO_FUN(	do_give		);
DECLARE_DO_FUN( do_gmote	);
DECLARE_DO_FUN( do_gossip	);
DECLARE_DO_FUN(	do_goto		);
DECLARE_DO_FUN( do_gouge	);
DECLARE_DO_FUN( do_grab		);
DECLARE_DO_FUN( do_grant	);
DECLARE_DO_FUN( do_grats	);
DECLARE_DO_FUN( do_gredit	);
DECLARE_DO_FUN(	do_group	);
DECLARE_DO_FUN( do_groups	);
DECLARE_DO_FUN(	do_gset		);
DECLARE_DO_FUN(	do_gtell	);
DECLARE_DO_FUN( do_guild    	);
DECLARE_DO_FUN( do_heal		);
DECLARE_DO_FUN( do_heel		);
DECLARE_DO_FUN( do_hedit	);
DECLARE_DO_FUN(	do_help		);
DECLARE_DO_FUN( do_herotalk	);
DECLARE_DO_FUN(	do_hide		);
DECLARE_DO_FUN(	do_holylight	);
DECLARE_DO_FUN( do_hone		);
DECLARE_DO_FUN( do_hurl		);
DECLARE_DO_FUN( do_ic		);
DECLARE_DO_FUN(	do_idea		);
DECLARE_DO_FUN( do_ident	);
DECLARE_DO_FUN(	do_immtalk	);
DECLARE_DO_FUN( do_imotd	);
DECLARE_DO_FUN( do_incognito	);
DECLARE_DO_FUN( do_index	);
DECLARE_DO_FUN( do_info		);
DECLARE_DO_FUN(	do_inventory	);
DECLARE_DO_FUN(	do_invis	);
DECLARE_DO_FUN(	do_iquest	);
DECLARE_DO_FUN( do_jog		);
DECLARE_DO_FUN( do_junk		);
DECLARE_DO_FUN(	do_kick		);
DECLARE_DO_FUN(	do_kill		);
DECLARE_DO_FUN(	do_list		);
DECLARE_DO_FUN( do_load		);
DECLARE_DO_FUN(	do_lock		);
DECLARE_DO_FUN(	do_log		);
DECLARE_DO_FUN(	do_look		);
DECLARE_DO_FUN( do_lore		);
DECLARE_DO_FUN( do_lunge	);
DECLARE_DO_FUN( do_medit	);
DECLARE_DO_FUN( do_member	);
DECLARE_DO_FUN(	do_memory	);
DECLARE_DO_FUN( do_mpedit	);
DECLARE_DO_FUN( do_mplist	);
DECLARE_DO_FUN(	do_mwhere	);
DECLARE_DO_FUN( do_mob          );
DECLARE_DO_FUN( do_motd		);
DECLARE_DO_FUN( do_mpstat       );
DECLARE_DO_FUN( do_mpdump       );
DECLARE_DO_FUN(	do_mpoint	);
DECLARE_DO_FUN(	do_murder	);
DECLARE_DO_FUN( do_newbie	);
DECLARE_DO_FUN( do_news		);
DECLARE_DO_FUN( do_nocancel	);
DECLARE_DO_FUN( do_nochannels	);
DECLARE_DO_FUN( do_noexp	);
DECLARE_DO_FUN( do_nofollow	);
DECLARE_DO_FUN(	do_norestore	);
DECLARE_DO_FUN(	do_north	);
DECLARE_DO_FUN( do_nosummon	);
DECLARE_DO_FUN(	do_note		);
DECLARE_DO_FUN(	do_notell	);
DECLARE_DO_FUN(	do_notitle	);
DECLARE_DO_FUN( do_notran	);
DECLARE_DO_FUN( do_nowar	);
DECLARE_DO_FUN( do_obfuscate	);
DECLARE_DO_FUN( do_oedit	);
DECLARE_DO_FUN( do_olc		);
DECLARE_DO_FUN( do_olevel	);
DECLARE_DO_FUN( do_onslaught	);
DECLARE_DO_FUN( do_ooc		);
DECLARE_DO_FUN( do_opdump	);
DECLARE_DO_FUN( do_opedit	);
DECLARE_DO_FUN( do_opstat	);
DECLARE_DO_FUN(	do_open		);
DECLARE_DO_FUN( do_oplist	);
DECLARE_DO_FUN(	do_order	);
DECLARE_DO_FUN( do_outfit	);
DECLARE_DO_FUN( do_overhand	);
DECLARE_DO_FUN( do_owhere	);
DECLARE_DO_FUN( do_pathfind	);
DECLARE_DO_FUN( do_pardon	);
DECLARE_DO_FUN(	do_password	);
DECLARE_DO_FUN(	do_peace	);
DECLARE_DO_FUN( do_pecho	);
DECLARE_DO_FUN(	do_peek		);
DECLARE_DO_FUN( do_penalty	);
DECLARE_DO_FUN( do_permban	);
DECLARE_DO_FUN(	do_pick		);
DECLARE_DO_FUN( do_pkcheck	);
DECLARE_DO_FUN( do_plist	);
DECLARE_DO_FUN( do_pload	);
DECLARE_DO_FUN( do_pmote	);
DECLARE_DO_FUN( do_pour		);
DECLARE_DO_FUN(	do_practice	);
DECLARE_DO_FUN( do_pray		);
DECLARE_DO_FUN( do_prefi	);
DECLARE_DO_FUN( do_prefix	);
DECLARE_DO_FUN( do_pretitle	);
DECLARE_DO_FUN( do_preturn	);
DECLARE_DO_FUN( do_promote	);
DECLARE_DO_FUN( do_prompt	);
DECLARE_DO_FUN( do_pswitch	);
DECLARE_DO_FUN( do_pull		);
DECLARE_DO_FUN( do_punload	);
DECLARE_DO_FUN(	do_purge	);
DECLARE_DO_FUN(	do_put		);
DECLARE_DO_FUN( do_qgossip	);
DECLARE_DO_FUN(	do_quaff	);
DECLARE_DO_FUN( do_quest	);
DECLARE_DO_FUN( do_quiet	);
DECLARE_DO_FUN(	do_quit		);
DECLARE_DO_FUN( do_quote	);
DECLARE_DO_FUN( do_race_edit	);
DECLARE_DO_FUN( do_racetalk	);
DECLARE_DO_FUN( do_racial	);
DECLARE_DO_FUN( do_raze		);
DECLARE_DO_FUN( do_read		);
DECLARE_DO_FUN(	do_reboo	);
DECLARE_DO_FUN(	do_recall	);
DECLARE_DO_FUN(	do_recho	);
DECLARE_DO_FUN(	do_recite	);
DECLARE_DO_FUN(	do_recover	);
DECLARE_DO_FUN( do_redit	);
DECLARE_DO_FUN( do_reload	);
DECLARE_DO_FUN(	do_remembe	);
DECLARE_DO_FUN(	do_remember	);
DECLARE_DO_FUN(	do_remove	);
DECLARE_DO_FUN( do_rename	);
DECLARE_DO_FUN( do_repair	);
DECLARE_DO_FUN( do_repent	);
DECLARE_DO_FUN( do_replay	);
DECLARE_DO_FUN(	do_reply	);
DECLARE_DO_FUN(	do_report	);
DECLARE_DO_FUN(	do_rerol	);
DECLARE_DO_FUN(	do_reroll	);
DECLARE_DO_FUN(	do_rescue	);
DECLARE_DO_FUN( do_resets	);
DECLARE_DO_FUN(	do_rest		);
DECLARE_DO_FUN(	do_restring	);
DECLARE_DO_FUN(	do_restore	);
DECLARE_DO_FUN( do_retreat	);
DECLARE_DO_FUN(	do_return	);
DECLARE_DO_FUN( do_revive	);
DECLARE_DO_FUN( do_roster	);
DECLARE_DO_FUN(	do_roundhouse);
DECLARE_DO_FUN( do_rpdump	);
DECLARE_DO_FUN( do_rpedit	);
DECLARE_DO_FUN( do_rplist	);
DECLARE_DO_FUN( do_rpstat	);
DECLARE_DO_FUN(	do_rstat	);
DECLARE_DO_FUN( do_rub		);
DECLARE_DO_FUN( do_rules	);
DECLARE_DO_FUN(	do_sacrifice	);
DECLARE_DO_FUN(	do_save		);
DECLARE_DO_FUN(	do_say		);
DECLARE_DO_FUN( do_scalp	);
DECLARE_DO_FUN( do_scan		);
DECLARE_DO_FUN( do_scatter	);
DECLARE_DO_FUN(	do_score	);
DECLARE_DO_FUN( do_scribe	);
DECLARE_DO_FUN( do_scroll	);
DECLARE_DO_FUN( do_second	);
DECLARE_DO_FUN(	do_sell		);
DECLARE_DO_FUN( do_set		);
DECLARE_DO_FUN( do_sharpen	);
DECLARE_DO_FUN(	do_shout	);
DECLARE_DO_FUN( do_showclass	);
DECLARE_DO_FUN(	do_shutdow	);
DECLARE_DO_FUN(	do_shutdown	);
DECLARE_DO_FUN(	do_sign		);
DECLARE_DO_FUN( do_sit		);
DECLARE_DO_FUN( do_skedit	);
DECLARE_DO_FUN( do_skills	);
DECLARE_DO_FUN(	do_sla		);
DECLARE_DO_FUN(	do_slay		);
DECLARE_DO_FUN(	do_sleep	);
DECLARE_DO_FUN( do_slip		);
DECLARE_DO_FUN(	do_slookup	);
DECLARE_DO_FUN( do_smite	);
DECLARE_DO_FUN(	do_sneak	);
DECLARE_DO_FUN(	do_snoop	);
DECLARE_DO_FUN( do_social	);
DECLARE_DO_FUN( do_socials	);
DECLARE_DO_FUN( do_social_edit	);
DECLARE_DO_FUN( do_sound	);
DECLARE_DO_FUN(	do_south	);
DECLARE_DO_FUN( do_sockets	);
DECLARE_DO_FUN( do_spells	);
DECLARE_DO_FUN( do_spellup	);
DECLARE_DO_FUN(	do_split	);
DECLARE_DO_FUN(	do_sset		);
DECLARE_DO_FUN( do_stake	);
DECLARE_DO_FUN( do_stalk	);
DECLARE_DO_FUN(	do_stand	);
DECLARE_DO_FUN( do_stat		);
DECLARE_DO_FUN(	do_steal	);
DECLARE_DO_FUN( do_storage	);
DECLARE_DO_FUN( do_strangle	);
DECLARE_DO_FUN( do_string	);
DECLARE_DO_FUN(	do_switch	);
DECLARE_DO_FUN( do_tackle	);
DECLARE_DO_FUN( do_talk		);
DECLARE_DO_FUN(	do_tell		);
DECLARE_DO_FUN( do_test		);
DECLARE_DO_FUN(	do_time		);
DECLARE_DO_FUN(	do_title	);
DECLARE_DO_FUN(	do_track	);
DECLARE_DO_FUN(	do_train	);
DECLARE_DO_FUN(	do_transfer	);
DECLARE_DO_FUN( do_trip		);
DECLARE_DO_FUN(	do_trust	);
DECLARE_DO_FUN(	do_twit		);
DECLARE_DO_FUN(	do_typo		);
DECLARE_DO_FUN( do_unalias	);
DECLARE_DO_FUN( do_unallow	);
DECLARE_DO_FUN( do_unban	);
DECLARE_DO_FUN(	do_unlock	);
DECLARE_DO_FUN( do_unread	);
DECLARE_DO_FUN(	do_up		);
DECLARE_DO_FUN(	do_value	);
DECLARE_DO_FUN( do_vap		);
DECLARE_DO_FUN( do_vape		);
DECLARE_DO_FUN(	do_vdpi		);
DECLARE_DO_FUN(	do_vdth		);
DECLARE_DO_FUN(	do_vdtr		);
DECLARE_DO_FUN(	do_visible	);
DECLARE_DO_FUN(	do_vload	);
DECLARE_DO_FUN( do_vnum		);
DECLARE_DO_FUN(	do_voodoo	);
DECLARE_DO_FUN( do_vote		);
DECLARE_DO_FUN(	do_wake		);
DECLARE_DO_FUN(	do_wear		);
DECLARE_DO_FUN(	do_weather	);
DECLARE_DO_FUN( do_wecho	);
DECLARE_DO_FUN(	do_west		);
DECLARE_DO_FUN(	do_where	);
DECLARE_DO_FUN(	do_who		);
DECLARE_DO_FUN( do_whois	);
DECLARE_DO_FUN( do_whostring	);
DECLARE_DO_FUN(	do_wimpy	);
DECLARE_DO_FUN(	do_wipe		);
DECLARE_DO_FUN( do_withdraw	);
DECLARE_DO_FUN(	do_wizhelp	);
DECLARE_DO_FUN( do_wizlist	);
DECLARE_DO_FUN( do_wiznet	);
DECLARE_DO_FUN( do_wizslap	);
DECLARE_DO_FUN( do_worth	);
DECLARE_DO_FUN(	do_yell		);
DECLARE_DO_FUN(	do_zap		);
DECLARE_DO_FUN( do_zecho	);

DECLARE_DO_FUN( do_showskill	);
DECLARE_DO_FUN( do_todo		);
DECLARE_DO_FUN( do_nohelp	);
DECLARE_DO_FUN( do_tocode	);
DECLARE_DO_FUN( do_changed	);
DECLARE_DO_FUN( do_battlehymn	);
DECLARE_DO_FUN( do_warcry	);
DECLARE_DO_FUN( do_shield_smash	);
DECLARE_DO_FUN( do_dislodge	);
DECLARE_DO_FUN( do_deathblow	);
DECLARE_DO_FUN( do_dismember	);
DECLARE_DO_FUN( do_legsweep     );
DECLARE_DO_FUN( do_bandage      );
DECLARE_DO_FUN( do_assassinate  );
DECLARE_DO_FUN( do_sheath	);
DECLARE_DO_FUN( do_draw		);
DECLARE_DO_FUN( do_mlevel       );
DECLARE_DO_FUN( do_trap		);
DECLARE_DO_FUN( do_storm_blades	);
DECLARE_DO_FUN( do_cross_slash	);
DECLARE_DO_FUN( do_doorbash     );
DECLARE_DO_FUN( do_gash		);
DECLARE_DO_FUN( do_salve	);
DECLARE_DO_FUN( do_charge	);
DECLARE_DO_FUN( do_condemn	);
DECLARE_DO_FUN( do_prefix_edit	);
DECLARE_DO_FUN( do_suffix_edit	);
DECLARE_DO_FUN( do_browse	);
DECLARE_DO_FUN( do_disguise	);
DECLARE_DO_FUN( do_gquest	);
DECLARE_DO_FUN( do_rlist	);
DECLARE_DO_FUN( do_olist	);
DECLARE_DO_FUN( do_mlist	);
DECLARE_DO_FUN( do_command_edit	);
DECLARE_DO_FUN( do_lookup	);
