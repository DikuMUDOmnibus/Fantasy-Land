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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> 
#include "merc.h"
#include "interp.h"
#include "recycle.h"
#include "simscripts.h"

DISABLED_DATA *disabled_first;
char last_command[MAX_STRING_LENGTH];

#define COMMAND_FILE	"../data/info/commands.dat"

/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2
#define LOG_BUILD	3

const	struct	cmd_type	cmd_table	[] =
{
    { "north",		do_north,	POS_STANDING,    0, 1, LOG_NEVER,  0 },
    { "east",		do_east,	POS_STANDING,	 0, 1, LOG_NEVER,  0 },
    { "south",		do_south,	POS_STANDING,	 0, 1, LOG_NEVER,  0 },
    { "west",		do_west,	POS_STANDING,	 0, 1, LOG_NEVER,  0 },
    { "up",		do_up,		POS_STANDING,	 0, 1, LOG_NEVER,  0 },
    { "down",		do_down,	POS_STANDING,	 0, 1, LOG_NEVER,  0 },
    { "spells",		do_spells,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "jog",		do_jog,		POS_STANDING,	 0, 1, LOG_NORMAL, 0 },
    { "run",		do_jog,		POS_STANDING,    0, 1, LOG_NORMAL, 0 },
    { "sprint",		do_jog,		POS_STANDING,    0, 1, LOG_NORMAL, 0 },
    { "cast",		do_cast,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "ooc",            do_ooc,         POS_SLEEPING,    0, 1, LOG_NORMAL, 1 },
    { "racetalk",	do_racetalk,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "bounty",		do_bounty,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "brew",		do_brew,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "buy",		do_buy,		POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "channels",       do_channels,    POS_DEAD,        0, 1, LOG_NORMAL, 1 },
    { "chat",		do_chat,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "exits",		do_exits,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "get",		do_get,		POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "group",          do_group,       POS_SLEEPING,    0, 1, LOG_NORMAL, 1 },
    { "hit",		do_kill,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 0 },
    { "inventory",	do_inventory,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "ic",		do_ic,		POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "kill",		do_kill,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "look",		do_look,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "lore",		do_lore,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "repair",		do_repair,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "mount",		do_mount,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "dismount",	do_dismount,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "clan",		do_clantalk,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "cult",		do_clantalk,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "chant",		do_clantalk,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "clist",		do_clanlist,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "order",		do_order,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "practice",       do_practice,	POS_SLEEPING,    0, 1, LOG_NORMAL, 1 },
    { "pray",		do_pray,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "rest",		do_rest,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "sit",		do_sit,		POS_SLEEPING,    0, 1, LOG_NORMAL, 1 },
    { "stand",		do_stand,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "storage",	do_storage,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "tell",		do_tell,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "unlock",         do_unlock,      POS_RESTING,     0, 1, LOG_NORMAL, 1 },
    { "wield",		do_wear,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "sheath",		do_sheath,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "affects",	do_affects,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "areas",		do_areas,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "bug",		do_bug,		POS_DEAD,	 0, 1, LOG_NORMAL, 1 },

    { "changes",	do_changes,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "commands",	do_commands,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "compare",	do_compare,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "configure",	do_configure,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "consider",	do_consider,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "count",		do_count,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "credits",	do_credits,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "equipment",	do_equipment,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "examine",	do_examine,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "?",		do_help,	POS_DEAD,	 0, 1, LOG_NORMAL, 0 },
    { "help",		do_help,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "idea",		do_idea,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "index",		do_index,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "info",           do_groups,      POS_SLEEPING,    0, 1, LOG_NORMAL, 1 },
    { "motd",		do_motd,	POS_DEAD,        0, 1, LOG_NORMAL, 1 },
    { "newbie",		do_newbie,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "news",		do_news,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "peek",		do_peek,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "read",		do_read,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "report",		do_report,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "rules",		do_rules,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "score",		do_score,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "scan",		do_scan,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "scribe",		do_scribe,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "skills",		do_skills,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "gocial",		do_social,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "social",		do_social,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "socials",	do_socials,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "time",		do_time,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "typo",		do_typo,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "nohelp",		do_nohelp,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "changed",        do_changed,     POS_DEAD,        0, 1, LOG_NORMAL, 1 },
    { "weather",	do_weather,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "who",		do_who,		POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "whois",		do_whois,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "whostring",	do_whostring,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "wizlist",	do_wizlist,	POS_DEAD,        0, 1, LOG_NORMAL, 1 },
    { "worth",		do_worth,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "alia",		do_alia,	POS_DEAD,	 0, 1, LOG_NORMAL, 0 },
    { "alias",		do_alias,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "autoaction",	do_autoaction,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "cloak",		do_cloak,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "colour",         do_colour,      POS_DEAD,        0, 1, LOG_NORMAL, 1 },
    { "color",          do_colour,      POS_DEAD,        0, 1, LOG_NORMAL, 1 },
    { "description",	do_description,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "nofollow",	do_nofollow,	POS_DEAD,        0, 1, LOG_NORMAL, 1 },
    { "nosummon",	do_nosummon,	POS_DEAD,        0, 1, LOG_NORMAL, 1 },
    { "notran",		do_notran,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "nocancel",	do_nocancel,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "outfit",		do_outfit,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "password",	do_password,	POS_DEAD,	 0, 1, LOG_NEVER,  1 },
    { "prompt",		do_prompt,	POS_DEAD,        0, 1, LOG_NORMAL, 1 },
    { "scroll",		do_scroll,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "sound",		do_sound,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "title",		do_title,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "pretitle",	do_pretitle,    POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "unalias",	do_unalias,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "afk",		do_afk,		POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "answer",		do_answer,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "ansi",		do_colour,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "auction",	do_auction,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "emote",		do_emote,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { ":",		do_emote,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "pmote",		do_pmote,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "gossip",		do_gossip,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "=",		do_cgossip,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 0 },
    { "cgossip",	do_cgossip,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "-",		do_qgossip,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 0 },
    { "qgossip",	do_qgossip,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { ",",		do_emote,	POS_RESTING,	 0, 1, LOG_NORMAL, 0 },
    { "grats",		do_grats,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "gtell",		do_gtell,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { ";",		do_gtell,	POS_DEAD,	 0, 1, LOG_NORMAL, 0 },
    { "note",		do_note,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "iquest",		do_iquest,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "gquest",		do_gquest,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "ask",		do_ask,		POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "quote",		do_quote,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "quiet",		do_quiet,	POS_SLEEPING, 	 0, 1, LOG_NORMAL, 1 },
    { "reply",		do_reply,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "replay",		do_replay,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "say",		do_say,		POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "'",		do_say,		POS_RESTING,	 0, 1, LOG_NORMAL, 0 },
    { "shout",		do_shout,	POS_RESTING,	 3, 1, LOG_NORMAL, 1 },
    { "unread",		do_unread,	POS_SLEEPING,    0, 1, LOG_NORMAL, 1 },
    { "yell",		do_yell,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "gmote",		do_gmote,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "brandish",	do_brandish,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "close",		do_close,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "drink",		do_drink,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "draw",		do_draw,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "drop",		do_drop,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "eat",		do_eat,		POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "envenom",	do_envenom,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "hone",		do_hone,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "sharpen",	do_sharpen,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "fill",		do_fill,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "give",		do_give,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "heal",		do_heal,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 }, 
    { "repent",		do_repent,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "curse",		do_curse,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "hold",		do_wear,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "list",		do_list,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "lock",		do_lock,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "open",		do_open,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "pick",		do_pick,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "pour",		do_pour,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "put",		do_put,		POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "quaff",		do_quaff,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "recite",		do_recite,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "remove",		do_remove,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "sell",		do_sell,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "second",		do_second,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "take",		do_get,		POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "sacrifice",	do_sacrifice,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "junk",           do_junk,	POS_STANDING,    0, 1, LOG_NORMAL, 0 },
    { "value",		do_value,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "browse",		do_browse,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "wear",		do_wear,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "zap",		do_zap,		POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "quest",          do_quest,       POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "ambush",		do_ambush,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "backstab",	do_backstab,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "bash",		do_bash,	POS_FIGHTING,    0, 1, LOG_NORMAL, 1 },
    { "bs",		do_backstab,	POS_STANDING,	 0, 1, LOG_NORMAL, 0 },
    { "berserk",	do_berserk,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "camouflage",	do_camouflage,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "forestmeld",	do_forest_meld,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "circle",		do_circle,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "cyclone",	do_cyclone,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "engage",		do_engage,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "engineer",	do_engineer,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "deathblow",	do_deathblow,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "dismember",	do_dismember,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "feed",		do_feed,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "dirt",		do_dirt,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "disarm",		do_disarm,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "flee",		do_flee,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "flame",		do_flame,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "retreat",	do_retreat,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "gouge",		do_gouge,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "hurl",		do_hurl,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "kick",		do_kick,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
	{ "roundhouse",		do_roundhouse,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "murder",		do_murder,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "raze",		do_raze,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "rescue",		do_rescue,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 0 },
    { "rub",		do_rub,		POS_FIGHTING,    0, 1, LOG_NORMAL, 1 },
    { "strangle",	do_strangle,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "tackle",		do_tackle,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "trip",		do_trip,	POS_FIGHTING,    0, 1, LOG_NORMAL, 1 },
    { "mob",		do_mob,		POS_DEAD,	 0, 1, LOG_NEVER,  0 },
    { "account",	do_account,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "accept",         do_accept,      POS_STANDING,    0, 1, LOG_NORMAL, 0 },
    { "decline",	do_decline,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "unchallenge",	do_decline,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "challenge",	do_challenge,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "arena",		do_arena,	POS_DEAD,        0, 1, LOG_NORMAL, 0 },
    { "astat",		do_arenastatus,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "finger",		do_finger,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "promote",	do_promote,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "demote",		do_demote,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "enter", 		do_enter, 	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "roster",		do_roster,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "follow",		do_follow,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "gain",		do_gain,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "groups",		do_groups,	POS_SLEEPING,    0, 1, LOG_NORMAL, 1 },
    { "hide",		do_hide,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "obfuscate",	do_obfuscate,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "member",		do_member,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "quit",		do_quit,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "home",		do_home,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "recall",		do_recall,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "/",		do_recall,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 0 },
    { "barter",		do_barter,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "backup",		do_backup,	POS_DEAD,	 0, 1, LOG_NORMAL, 0 },
    { "save",		do_save,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "sleep",		do_sleep,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "slip",		do_slip,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "sneak",		do_sneak,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "split",		do_split,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "steal",		do_steal,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "train",		do_train,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "track",		do_track,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "visible",	do_visible,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "wake",		do_wake,	POS_SLEEPING,	 0, 1, LOG_NORMAL, 1 },
    { "where",		do_where,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "donate",		do_donate,	POS_STANDING,	 5, 1, LOG_NORMAL, 1 },
    { "overhand",	do_overhand,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "crush",		do_crush,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "onslaught",	do_onslaught,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "lunge",		do_lunge,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "scalp",		do_scalp,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "stake",		do_stake,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "cdonate",	do_cdonate,	POS_STANDING,	 5, 1, LOG_NORMAL, 1 },
    { "class",		do_class,	POS_SLEEPING,    0, 1, LOG_NORMAL, 1 },
    { "forge",		do_forge,	POS_STANDING,    0, 1, LOG_NORMAL, 0 },
    { "forget",		do_forget,	POS_SLEEPING,    0, 1, LOG_NORMAL, 1 },
    { "remembe",	do_remembe,	POS_SLEEPING,    0, 1, LOG_NORMAL, 0 },
    { "remember",	do_remember,	POS_SLEEPING,    0, 1, LOG_NORMAL, 1 },
    { "voodoo",		do_voodoo,	POS_STANDING,	20, 1, LOG_NORMAL, 1 },
    { "balance",	do_balance,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "deposit",	do_deposit,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "withdraw",	do_withdraw,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "vote",		do_vote,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "prefi",		do_prefi,	POS_DEAD,	 0, 1, LOG_NORMAL, 0 },
    { "prefix",		do_prefix,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "clan_manage",	do_clan_manage,	POS_STANDING,   20, 1, LOG_ALWAYS, 1 },
    { "advance",	do_advance,	POS_DEAD,	ML, 1, LOG_ALWAYS, 1 },
    { "clan_edit",	do_clan_edit,	POS_DEAD,	ML, 1, LOG_BUILD,  1 },
    { "class_edit",	do_class_edit,	POS_DEAD,	ML, 1, LOG_BUILD,  1 },
    { "clead",		do_clead,	POS_DEAD,	ML, 1, LOG_NORMAL, 1 },
    { "command_edit",	do_command_edit,POS_DEAD,	ML, 1, LOG_BUILD,  1 },
    { "disable",	do_disable,	POS_DEAD,	ML, 1, LOG_ALWAYS, 1 },
    { "exp_mod",	do_exp_mod,	POS_DEAD,	ML, 1, LOG_ALWAYS, 1 },
    { "ftick",		do_ftick,	POS_DEAD,	ML, 1, LOG_NORMAL, 1 },
    { "grant",		do_grant,	POS_DEAD,	ML, 1, LOG_ALWAYS, 1 },
    { "gredit",		do_gredit,	POS_DEAD,	ML, 1, LOG_BUILD,  1 },
    { "guild",		do_guild,	POS_DEAD,	ML, 1, LOG_ALWAYS, 1 },
    { "pload",          do_pload,       POS_DEAD,       ML, 1, LOG_ALWAYS, 1 },
    { "preturn",	do_preturn,	POS_DEAD,	ML, 1, LOG_ALWAYS, 1 },
    { "pswitch",	do_pswitch,	POS_DEAD,	ML, 1, LOG_ALWAYS, 1 },
    { "punload",        do_punload,     POS_DEAD,       ML, 1, LOG_ALWAYS, 1 },
    { "race_edit",	do_race_edit,	POS_DEAD,	ML, 1, LOG_BUILD,  1 },
    { "rename",         do_rename,      POS_DEAD,       ML, 1, LOG_ALWAYS, 1 },
    { "skedit",		do_skedit,	POS_DEAD,	ML, 1, LOG_BUILD,  1 },
    { "shutdow",	do_shutdow,	POS_DEAD,	ML, 1, LOG_NORMAL, 0 },
    { "shutdown",	do_shutdown,	POS_DEAD,	ML, 1, LOG_ALWAYS, 1 },
    { "test",		do_test,	POS_DEAD,	ML, 1, LOG_ALWAYS, 0 },
    { "trust",		do_trust,	POS_DEAD,	ML, 1, LOG_ALWAYS, 1 },
    { "vap",		do_vap,		POS_DEAD,	ML, 1, LOG_ALWAYS, 0 },
    { "vape",		do_vape,	POS_DEAD,	ML, 1, LOG_ALWAYS, 1 },
    { "allow",		do_allow,	POS_DEAD,	L1, 1, LOG_ALWAYS, 1 },
    { "ban",		do_ban,		POS_DEAD,	L1, 1, LOG_ALWAYS, 1 },
    { "doas",		do_doas,	POS_DEAD,	L1, 1, LOG_NORMAL, 1 },
    { "log",		do_log,		POS_DEAD,	L1, 1, LOG_ALWAYS, 1 },
    { "permban",	do_permban,	POS_DEAD,	L1, 1, LOG_ALWAYS, 1 },
    { "plist",		do_plist,	POS_DEAD,	L1, 1, LOG_NORMAL, 1 },
    { "reboo",		do_reboo,	POS_DEAD,	L1, 1, LOG_NORMAL, 0 },
    { "reboot",		do_copyover,	POS_DEAD,	L1, 1, LOG_ALWAYS, 1 },
    { "reload",		do_reload,	POS_DEAD,	L1, 1, LOG_ALWAYS, 1 },
    { "snoop",		do_snoop,	POS_DEAD,	L1, 1, LOG_ALWAYS, 1 },
    { "sockets",        do_sockets,	POS_DEAD,       L1, 1, LOG_NORMAL, 1 },
    { "users",          do_sockets,	POS_DEAD,       L1, 1, LOG_NORMAL, 1 },
    { "connections",    do_sockets,	POS_DEAD,       L1, 1, LOG_NORMAL, 1 },
    { "unallow",	do_unallow,	POS_DEAD,	L1, 1, LOG_NORMAL, 1 },
    { "unban",		do_unban,	POS_DEAD,	L1, 1, LOG_NORMAL, 1 },
    { "wipe",		do_wipe,	POS_DEAD,	L1, 1, LOG_ALWAYS, 1 },
    { "addlag",		do_addlag,	POS_DEAD,	L2, 1, LOG_ALWAYS, 1 },
    { "bonus",		do_bonus,	POS_DEAD,	L2, 1, LOG_ALWAYS, 1 },
    { "flag",		do_flag,	POS_DEAD,	L2, 1, LOG_ALWAYS, 1 },
    { "game_edit",	do_game_edit,	POS_DEAD,	L2, 1, LOG_NORMAL, 1 },
    { "set",            do_set,         POS_DEAD,       L2, 1, LOG_NORMAL, 1 },
    { "talk",		do_talk,	POS_DEAD,	L2, 1, LOG_NORMAL, 1 },
    { "wecho",		do_wecho,	POS_DEAD,	L2, 1, LOG_NORMAL, 1 },
    { "disconnect",	do_disconnect,	POS_DEAD,	L3, 1, LOG_ALWAYS, 1 },
    { "norestore",	do_norestore,	POS_DEAD,	L3, 1, LOG_ALWAYS, 1 },
    { "notitle",	do_notitle,	POS_DEAD,	L3, 1, LOG_ALWAYS, 1 },
    { "prefix_edit",	do_prefix_edit,	POS_DEAD,	L3, 1, LOG_NORMAL, 1 },
    { "sla",		do_sla,		POS_DEAD,	L3, 1, LOG_NORMAL, 0 },
    { "slay",		do_slay,	POS_DEAD,	L3, 1, LOG_NORMAL, 1 },
    { "suffix_edit",	do_suffix_edit,	POS_DEAD,	L3, 1, LOG_NORMAL, 1 },
    { "channel_edit",	do_channel_edit,POS_DEAD,	L4, 1, LOG_BUILD,  1 },
    { "freeze",		do_freeze,	POS_DEAD,	L4, 1, LOG_ALWAYS, 1 },
    { "gecho",		do_echo,	POS_DEAD,	L4, 1, LOG_NORMAL, 1 },
    { "grab",           do_grab,        POS_DEAD,       L4, 1, LOG_ALWAYS, 1 },
    { "restore",	do_restore,	POS_DEAD,	L4, 1, LOG_NORMAL, 1 },
    { "zecho",		do_zecho,	POS_DEAD,	L4, 1, LOG_NORMAL, 1 },
    { "notell",		do_notell,	POS_DEAD,	L5, 1, LOG_ALWAYS, 1 },
    { "pecho",		do_pecho,	POS_DEAD,	L5, 1, LOG_NORMAL, 1 },
    { "smite",		do_smite,	POS_DEAD,	L5, 1, LOG_ALWAYS, 1 },
    { "wizslap",	do_wizslap,	POS_DEAD,	L5, 1, LOG_NORMAL, 1 },
    { "at",             do_at,          POS_DEAD,       L6, 1, LOG_NORMAL, 1 },
    { "aclear",		do_aclear,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "censor",		do_censor,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "checkeq",	do_checkeq,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "clone",		do_clone,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "corner",		do_corner,	POS_DEAD,	L6, 1, LOG_ALWAYS, 1 },
    { "hedit",		do_hedit,	POS_DEAD,	L6, 1, LOG_BUILD,  1 },
    { "identity",	do_ident,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "load",		do_load,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "memory",		do_memory,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "mlevel",		do_mlevel,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "mpdump",		do_mpdump,	POS_DEAD,	L6, 1, LOG_NEVER,  1 },
    { "mpedit",		do_mpedit,	POS_DEAD,	L6, 1, LOG_BUILD,  1 },
    { "mplist",		do_mplist,	POS_DEAD,	L6, 1, LOG_BUILD,  1 },
    { "mpoint",		do_mpoint,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "mpstat",		do_mpstat,	POS_DEAD,       L6, 1, LOG_NEVER,  1 },
    { "mwhere",		do_mwhere,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "nochannels",	do_nochannels,	POS_DEAD,	L6, 1, LOG_ALWAYS, 1 },
    { "olevel",		do_olevel,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "opdump",		do_opdump,	POS_DEAD,	L6, 1, LOG_BUILD,  1 },
    { "opedit",		do_opedit,	POS_DEAD,	L6, 1, LOG_BUILD,  1 },
    { "oplist",		do_oplist,	POS_DEAD,	L6, 1, LOG_BUILD,  1 },
    { "opstat",		do_opstat,	POS_DEAD,	L6, 1, LOG_BUILD,  1 },
    { "owhere",		do_owhere,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "peace",		do_peace,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "rpdump",		do_rpdump,	POS_DEAD,	L6, 1, LOG_BUILD,  1 },
    { "rpedit",		do_rpedit,	POS_DEAD,	L6, 1, LOG_BUILD,  1 },
    { "rplist",		do_rplist,	POS_DEAD,	L6, 1, LOG_BUILD,  1 },
    { "rpstat",		do_rpstat,	POS_DEAD,	L6, 1, LOG_BUILD,  1 },
    { "sign",		do_sign,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "social_edit",	do_social_edit,	POS_DEAD,	L6, 1, LOG_BUILD,  1 },
    { "spellup",        do_spellup,     POS_DEAD,       L6, 1, LOG_ALWAYS, 1 },
    { "string",		do_string,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "teleport",	do_transfer,    POS_DEAD,	L6, 1, LOG_NORMAL, 0 },
    { "transfer",	do_transfer,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "vnum",		do_vnum,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "find",		do_vnum,	POS_DEAD,	L6, 1, LOG_NORMAL, 1 },
    { "force",		do_force,	POS_DEAD,	L7, 1, LOG_NORMAL, 1 },
    { "pardon",		do_pardon,	POS_DEAD,	L7, 1, LOG_NORMAL, 1 },
    { "return",         do_return,      POS_DEAD,       L7, 1, LOG_NORMAL, 1 },
    { "switch",		do_switch,	POS_DEAD,	L7, 1, LOG_NORMAL, 1 },
    { "twit",		do_twit,	POS_DEAD,	L7, 1, LOG_NORMAL, 1 },
    { "echo",		do_recho,	POS_DEAD,	L8, 1, LOG_NORMAL, 1 },
    { "poofin",		do_bamfin,	POS_DEAD,	L8, 1, LOG_NORMAL, 1 },
    { "poofout",	do_bamfout,	POS_DEAD,	L8, 1, LOG_NORMAL, 1 },
    { "aedit",		do_aedit,	POS_DEAD,	IM, 1, LOG_BUILD,  1 },
    { "alist",		do_alist,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "asave",		do_asave,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "edit",		do_olc,		POS_DEAD,	IM, 1, LOG_BUILD,  1 },
    { "ghost",		do_ghost,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "goto",           do_goto,        POS_DEAD,       IM, 1, LOG_NORMAL, 1 },
    { "gset",		do_gset,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "holylight",	do_holylight,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "immtalk",	do_immtalk,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "]",		do_immtalk,	POS_DEAD,	IM, 1, LOG_NORMAL, 0 },
    { "imotd",          do_imotd,       POS_DEAD,       IM, 1, LOG_NORMAL, 1 },
    { "incognito",	do_incognito,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "invis",		do_invis,	POS_DEAD,	IM, 1, LOG_NORMAL, 0 },
    { "medit",		do_medit,	POS_DEAD,	IM, 1, LOG_BUILD,  1 },
    { "oedit",		do_oedit,	POS_DEAD,	IM, 1, LOG_BUILD,  1 },
    { "penalty",	do_penalty,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "purge",		do_purge,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "recover",	do_recover,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "redit",		do_redit,	POS_DEAD,	IM, 1, LOG_BUILD,  1 },
    { "resets",		do_resets,	POS_DEAD,	IM, 1, LOG_BUILD,  1 },
    { "scatter",	do_scatter,	POS_RESTING,	IM, 1, LOG_NORMAL, 1 },
    { "stat",		do_stat,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "wizinvis",	do_invis,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },

    { "script",         do_scriptedit,  POS_DEAD,       IM, 1, LOG_NORMAL, 1 },
    { "rfind",		do_rlist,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "rlist",		do_rlist,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "olist",		do_olist,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "mlist",		do_mlist,	POS_DEAD,	IM, 1, LOG_NORMAL, 1 },
    { "todo",		do_todo,	POS_DEAD,	L5, 1, LOG_NORMAL, 1 },
    { "tocode",		do_tocode,	POS_DEAD,	L5, 1, LOG_NORMAL, 1 },

    { "herotalk",	do_herotalk,	POS_DEAD,      101, 1, LOG_NORMAL, 1 },
    { "wizhelp",	do_wizhelp,	POS_DEAD,	 0, 2, LOG_NORMAL, 0 },
    { "wiznet",		do_wiznet,	POS_DEAD,	 0, 2, LOG_NORMAL, 0 },
    { "information",	do_info,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "combat",		do_combat,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "restring",	do_restring,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "disguise",	do_disguise,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "checkstats",	do_checkstats,	POS_DEAD,	 0, 1, LOG_NORMAL, 0 },
    { "delet",		do_delet,	POS_DEAD,	 0, 1, LOG_NEVER,  0 },
    { "delete",		do_delete,	POS_STANDING,	 0, 1, LOG_NEVER,  1 },
    { "noexp",		do_noexp,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "customwho",	do_customwho,	POS_DEAD,	 0, 1, LOG_NORMAL, 1 },
    { "rerol",		do_rerol,	POS_DEAD,	 0, 1, LOG_NORMAL, 0 },
    { "reroll",		do_reroll,	POS_STANDING,	 0, 1, LOG_NEVER,  1 },
    { "charts",		do_charts,	POS_DEAD,	 0, 1, LOG_NORMAL, 0 },
    { "pkcheck",	do_pkcheck,	POS_DEAD,	 1, 1, LOG_NORMAL, 1 },
    { "pkill",		do_condemn,	POS_RESTING,	 0, 1, LOG_NORMAL, 0 },
    { "condemned",	do_condemn,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "racial",		do_racial,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "revive",		do_revive,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "stalk",		do_stalk,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "lookup",		do_lookup,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "pull",		do_pull,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "compression",	do_compress,	POS_DEAD,        0, 1, LOG_NORMAL, 1 },
    { "showskill",	do_showskill,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "showclass",	do_showclass,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "devote",		do_devote,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },
    { "heel",		do_heel,	POS_RESTING,	 0, 1, LOG_NORMAL, 1 },

/* New declarations by Stroke */
    { "battlehymn",     do_battlehymn,  POS_FIGHTING,    0, 1, LOG_NORMAL, 1 },
    { "warcry",         do_warcry,      POS_STANDING,    0, 1, LOG_NORMAL, 1 },
    { "smash",          do_shield_smash,POS_FIGHTING,    0, 1, LOG_NORMAL, 1 },
    { "dislodge",       do_dislodge,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "legsweep",       do_legsweep,    POS_FIGHTING,    0, 1, LOG_NORMAL, 1 },
    { "bandage",        do_bandage,     POS_STANDING,    0, 1, LOG_NORMAL, 1 },
    { "assassinate",    do_assassinate,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "trap",		do_trap,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "storm of blades",do_storm_blades,POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "cross slash",	do_cross_slash,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "doorbash",       do_doorbash,    POS_RESTING,     0, 1, LOG_NORMAL, 1 },
    { "charge",		do_charge,	POS_STANDING,	 0, 1, LOG_NORMAL, 1 },
    { "salve",		do_salve,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "gash",		do_gash,	POS_FIGHTING,	 0, 1, LOG_NORMAL, 1 },
    { "",		0,		POS_DEAD,	 0, 1, LOG_NORMAL, 1 }
};

bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;

    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == social_table[cmd].name[0]
	&&   !str_prefix( command, social_table[cmd].name ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
	return FALSE;

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_char( "{DYou are {Rd{re{Ra{rd{w...{RD{rE{RA{rD{D!{x\n\r", ch );
	return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	send_to_char( "{DYou are {Rh{ru{Rr{rt {Dbadly, soon your {Rw{ro{Ru{rn{Rd{rs {Dshall {Wh{we{Wa{wl {Dor you shall {RD{rI{RE{D!{x\n\r", ch );
	return TRUE;

    case POS_STUNNED:
	send_to_char( "You are currently {Rstunned{x and feel a little woozy.{x\n\r", ch );
	return TRUE;

    case POS_SLEEPING:
	/*
	 * I just know this is the path to a 12" 'if' statement.  :(
	 * But two players asked for it already!  -- Furey
	 */
	if ( !str_cmp( social_table[cmd].name, "snore" ) )
	    break;
	send_to_char( "If you can do that while sleeping you are far more talented than me.{x\n\r", ch );
	return TRUE;

    }

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
	act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM,POS_RESTING);
	act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR,POS_RESTING);
    }
    else if ( ( victim = get_char_room( ch, NULL, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
	act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM,POS_RESTING);
	act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR,POS_RESTING);
    }
    else
    {
	act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT,POS_RESTING);
	act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR,POS_RESTING);
	act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT,POS_RESTING);

	if ( !IS_NPC(ch) && IS_NPC(victim)
	&&   !IS_AFFECTED(victim, AFF_CHARM)
	&&   IS_AWAKE(victim)
	&&   can_see( victim, ch )
	&&   victim->desc == NULL)
	{
	    switch ( number_bits( 4 ) )
	    {

	    case 1: case 2: case 3: case 4:
	    case 5: case 6: case 7: case 8:
	    case 9: case 10: case 11:
		break;


	    case 0:
		act( social_table[cmd].others_found,
		    victim, NULL, ch, TO_NOTVICT,POS_RESTING);
		act( social_table[cmd].char_found,
		    victim, NULL, ch, TO_CHAR,POS_RESTING);
		act( social_table[cmd].vict_found,
		    victim, NULL, ch, TO_VICT,POS_RESTING);
		break;
            case 12:
		act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT,POS_RESTING);
		act( "You slap $N.",  victim, NULL, ch, TO_CHAR,POS_RESTING);
		act( "$n slaps you.", victim, NULL, ch, TO_VICT,POS_RESTING);
		break;
	    }
	}
    }

    return TRUE;
}

bool check_disabled (const struct cmd_type *command)
{
    DISABLED_DATA *p;
	
    for (p = disabled_first; p ; p = p->next)
	if (p->command->do_fun == command->do_fun)
	    return TRUE;

    return FALSE;
}

bool check_granted( CHAR_DATA *ch, char *command )
{
    GRANT_DATA *grant;

    if ( IS_NPC( ch ) )
	return FALSE;

    for ( grant = ch->pcdata->grants; grant != NULL; grant = grant->next )
    {
	if ( !str_cmp( grant->command, command ) )
	    return TRUE;
    }

    return FALSE;
}

void interpret( CHAR_DATA *ch, char *argument )
{
    char cmd_copy[4*MAX_INPUT_LENGTH];
    char command[4*MAX_INPUT_LENGTH];
    char logline[4*MAX_INPUT_LENGTH];
    int cmd;
    int trust;
    int string_count = nAllocString;
    int perm_count = nAllocPerm;
    bool found;

    while ( isspace(*argument) )
	argument++;

    if ( argument[0] == '\0' )
	return;

    strcpy( cmd_copy, argument );
    
    strcpy( logline, argument );

    sprintf( last_command, "%s in room[%d]: %s.",
	ch->name, ch->in_room == NULL ? 0 : ch->in_room->vnum, argument );

    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
	command[0] = argument[0];
	command[1] = '\0';
	argument++;

	while ( isspace(*argument) )
	    argument++;
    } else {
	argument = one_argument( argument, command );
    }

    found = FALSE;
    trust = get_trust( ch );


    /*
     * Find a portal if there is one.
     */
    if ( ch->in_room != NULL && str_cmp( command, "goto" ) && !found ) {
        OBJ_DATA *prop;
        for ( prop = ch->in_room->contents; prop != NULL; prop = prop->next_content )
        {
            if ( can_see_obj( ch, prop ) && is_exact_name( command, prop->name ) && prop->item_type == ITEM_PORTAL ) {
                do_enter( ch, command );
                return;
            }
        }
    }

    // Look for a command in the command table.
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == cmd_table[cmd].name[0]
	&&   !str_prefix( command, cmd_table[cmd].name )
	&&   ( cmd_table[cmd].level <= trust
	||     check_granted( ch, cmd_table[cmd].name ) ) )
	{
	    if (cmd_table[cmd].tier == 1)
	    {
		found = TRUE;
		break;
	    } else if ( IS_TRUSTED(ch,LEVEL_IMMORTAL)
		   ||   (ch->pcdata && ch->pcdata->tier > 1) )
	    {
		found = TRUE;
		break;
	    } else if (ch->level >= LEVEL_HERO)
	    {
		found = TRUE;
		break;
	    }
	}
    }

    if ( !IS_NPC( ch ) )
    {
	if ( ch->pcdata->penalty_time[PENALTY_FREEZE] != 0 )
	{
	    send_to_char( "You're totally frozen!\n\r", ch );
	    return;
	}

	if ( IS_SET( ch->comm, COMM_AFK ) && cmd_table[cmd].do_fun != do_afk )
           do_afk( ch, "" );
    }

    if ( cmd_table[cmd].log == LOG_NEVER )
	strcpy( logline, "" );

    else if ( cmd_table[cmd].log == LOG_BUILD )
	parse_logs( ch, "build", logline );

    if ( mud_stat.fLogAll || cmd_table[cmd].log == LOG_ALWAYS )
    {
	sprintf( log_buf, "Log %s: %s", ch->name, logline );
	wiznet(log_buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));
	log_string( log_buf );
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG)
    &&   cmd_table[cmd].log != LOG_NEVER )
    {
	parse_logs( ch, "log", logline );

	if ( cmd_table[cmd].log != LOG_ALWAYS )
	{
	    sprintf( log_buf, "Log %s: %s", ch->name, logline );
	    wiznet(log_buf,ch,NULL,WIZ_PLAYER_LOG,0,get_trust(ch));
	    log_string( log_buf );
	}
    }

    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    if ( !found )
    {
	if ( !check_social( ch, command, argument ) )
	    send_to_char( "Huh?\n\r", ch );
	return;
    }

    if (check_disabled (&cmd_table[cmd]))
    {
	send_to_char ("This command has been temporarily disabled.\n\r",ch);
	return;
    }

    if ( ch->position < cmd_table[cmd].position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "{DYou are {Rd{re{Ra{rd{D...{RD{rE{RA{rD{D!{x\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "{DYou are {Rh{ru{Rr{rt {Dbadly, soon your {Rw{ro{Ru{rn{Rd{rs {Dshall {Wh{we{Wa{wl {Dor you shall {RD{rI{RE{D!{x\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "You are currently {Rstunned{x and feel a little woozy.{x\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "If you can do that while sleeping you are far more talented than me.{x\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "{DYou are much too {Rc{ro{Rm{rf{Ro{rr{Rt{ra{Rb{rl{Re {Dto do that right now{w...{x\n\r", ch);
	    break;

	case POS_SITTING:
	    send_to_char( "{DGet up off your {Rlazy ass {Dfirst!{x\n\r",ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "{DYou have far more lethal things to worry about!!{x\n\r", ch);
	    break;

	}
	return;
    }

    (*cmd_table[cmd].do_fun) ( ch, argument );

    if (string_count < nAllocString)
    {
	sprintf(log_buf,"Memcheck : Increase in strings :: %s : %s",
	    ch->name, cmd_copy);
	wiznet(log_buf, NULL, NULL, WIZ_MEMCHECK,0,get_trust(ch)) ;
    }

    if (perm_count < nAllocPerm)
    {
	sprintf(log_buf,"Increase in perms :: %s : %s", ch->name, cmd_copy);
	wiznet(log_buf, NULL, NULL, WIZ_MEMCHECK, 0,get_trust(ch));
    }

    tail_chain( );
    return;
}

/*
 * Return true if an argument is completely numeric.
 */
bool is_number ( char *arg )
{
 
    if ( *arg == '\0' )
        return FALSE;
 
    if ( *arg == '+' || *arg == '-' )
        arg++;
 
    for ( ; *arg != '\0'; arg++ )
    {
        if ( !isdigit( *arg ) )
            return FALSE;
    }
 
    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;
    
    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument( char *argument, char *arg )
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
        if ( *pdot == '*' )
        {
            *pdot = '\0';
            number = atoi( argument );
            *pdot = '*';
            strcpy( arg, pdot+1 );
            return number;
        }
    }
 
    strcpy( arg, argument );
    return 1;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

int sort_cmd_table( const void *v1, const void *v2 )
{
    int idx1 = *(int *) v1;
    int idx2 = *(int *) v2;
    int i = 0;

    /* compare the command names */
    for (i = 0; cmd_table[idx1].name[i] != '\0'; i++)
    {
	if (cmd_table[idx1].name[i] == cmd_table[idx2].name[i]) continue;
	if (cmd_table[idx2].name[i] == '\0')                    return  1;
	if (cmd_table[idx1].name[i]  > cmd_table[idx2].name[i]) return  1;
	if (cmd_table[idx1].name[i]  < cmd_table[idx2].name[i]) return -1;
    }
    return 0;
}

void do_commands( CHAR_DATA *ch, char *argument )
{
    BUFFER *final = new_buf();
    char buf[MAX_STRING_LENGTH];
    int index[MAX_STRING_LENGTH*4];
    int cmd, col, i, count;
 
    col = 0;

    for ( count = 0; cmd_table[count].name[0] != '\0'; count++ )
	index[count] = count;

    qsort(index, count, sizeof(int), sort_cmd_table);

    for ( i = 0; i < count; i++ )
    {
	cmd = index[i];

        if ( cmd_table[cmd].level <  LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust( ch ) 
	&&   cmd_table[cmd].show)
	{
	    if ( cmd_table[cmd].tier == 1
	    ||   (ch->pcdata && ch->pcdata->tier > 1)
	    ||   ch->level >= LEVEL_HERO )
	    {
		sprintf( buf, "%-16s", cmd_table[cmd].name );
		add_buf( final, buf );

		if ( ++col %5 == 0 )
		    add_buf( final, "\n\r" );
	    }
	}
    }
 
    if ( col %5 != 0 )
	add_buf( final, "\n\r" );

    page_to_char( final->string, ch );
    free_buf( final );

    return;
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    BUFFER *final;
    char buf[MAX_STRING_LENGTH];
    int cmd, col = 0, lvl = get_trust( ch ), level;
 
    if ( ch->pcdata && ch->pcdata->tier == 1 && lvl < LEVEL_HERO )
	return;

    final = new_buf();

    for( level = MAX_LEVEL; level >= LEVEL_HERO; level-- )
    {
	for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
	{
	    if ( cmd_table[cmd].level == level
	    &&   cmd_table[cmd].show
	    &&   ( cmd_table[cmd].level <= lvl
	    ||     check_granted( ch, cmd_table[cmd].name ) ) )
	    {
		if ( col == 0 )
		{
		    sprintf( buf, "\n\r{DLevel {m%3d{w:{W ", level );
		    add_buf( final, buf );
		}

		sprintf( buf, "%-15s", cmd_table[cmd].name );
		add_buf( final, buf );

		if ( ++col % 4 == 0 )
		    add_buf( final, "\n\r           " );
	    }
	}

	if ( col % 4 != 0 )
	    add_buf( final, "\n\r" );
	col = 0;
    }
 
    add_buf( final, "{x" );

    page_to_char( final->string, ch );
    free_buf( final );

    return;
}

void do_disable (CHAR_DATA *ch, char *argument)
{
    int i;
    DISABLED_DATA *p,*q;
    char buf[MAX_INPUT_LENGTH];
	
    if (IS_NPC(ch))
    {
	send_to_char ("RETURN first.\n\r",ch);
	return;
    }
	
    if (!argument[0])
    {
	if (!disabled_first)
	{
	    send_to_char ("There are no commands disabled.\n\r",ch);
	} else {
	    send_to_char ("Disabled commands:\n\rCommand           Level   Disabled by\n\r",ch);

	    for (p = disabled_first; p; p = p->next)
	    {
		sprintf (buf, "%-17s %5d   %-12s\n\r",
		    p->command->name, p->level, p->disabled_by);
		send_to_char (buf,ch);
	    }
	}

	return;
    }
	
    for (p = disabled_first; p ; p = p->next)
	if (!str_cmp(argument, p->command->name))
	    break;

    if (p)
    {
	if (get_trust(ch) < p->level)
	{
	    send_to_char ("This command was disabled by a higher power.\n\r",ch);
	    return;
	}
	if (disabled_first == p)
	    disabled_first = p->next;
	else
	{
	    for (q = disabled_first; q->next != p; q = q->next);
		q->next = p->next;
	}
	free_string (p->disabled_by);
	free_mem (p,sizeof(DISABLED_DATA));
	save_disabled();
	send_to_char ("Command enabled.\n\r",ch);
    }
    else
    {
	if (!str_cmp(argument,"disable"))
	{
	    send_to_char ("You cannot disable the disable command.\n\r",ch);
	    return;
	}
	for (i = 0; cmd_table[i].name[0] != '\0'; i++)
	    if (!str_cmp(cmd_table[i].name, argument))
		break;

	    if (cmd_table[i].name[0] == '\0')
	    {
		send_to_char ("No such command.\n\r",ch);
		return;
	    }

	    if (cmd_table[i].level > get_trust(ch))
	    {
		send_to_char ("You dot have access to that command; you cannot disable it.\n\r",ch);
		return;
	    }
	    p = alloc_mem (sizeof(DISABLED_DATA));

	    p->command = &cmd_table[i];
	    p->disabled_by = str_dup (ch->name);
	    p->level = get_trust(ch);
	    p->next = disabled_first;
	    disabled_first = p;
		
	    send_to_char ("Command disabled.\n\r",ch);
	    save_disabled(); /* save to disk */
    }
    return;
}

void load_disabled()
{
    FILE *fp;
    DISABLED_DATA *p;
    char *name;
    int i;
	
    disabled_first = NULL;

    fp = fopen (DISABLED_FILE, "r");

    if (!fp)
	return;

    name = fread_word (fp);

    while (str_cmp(name, "END"))
    {
	for (i = 0; cmd_table[i].name[0] ; i++)
	    if (!str_cmp(cmd_table[i].name, name))
		break;

	if (!cmd_table[i].name[0]) /* command does not exist? */
	{
	    bug ("Skipping uknown command in " DISABLED_FILE " file.",0);
	    fread_number(fp); /* level */
	    fread_word(fp); /* disabled_by */
	}
	else
	{
	    p = alloc_mem(sizeof(DISABLED_DATA));
	    p->command = &cmd_table[i];
	    p->level = fread_number(fp);
	    p->disabled_by = str_dup(fread_word(fp)); 
	    p->next = disabled_first;
			
	    disabled_first = p;

	}
		
	    name = fread_word(fp);
    }

    fclose (fp);		
    return;
}

void save_disabled()
{
    FILE *fp;
    DISABLED_DATA *p;
	
    if (!disabled_first) 
    {
	unlink (DISABLED_FILE);
	return;
    }
	
    fp = fopen (DISABLED_FILE, "w");
	
    if (!fp)
    {
	bug ("Could not open " DISABLED_FILE " for writing",0);
	return;
    }
	
    for (p = disabled_first; p ; p = p->next)
	fprintf (fp, "%s %d %s\n", p->command->name, p->level, p->disabled_by);
    fprintf (fp, "END\n");
    fclose (fp);
    return;
}

void do_grant( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    GRANT_DATA *grant;
    char arg[MAX_INPUT_LENGTH];
    bool found = FALSE;
    int cmd;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
	send_to_char( "Syntax: grant <victim> <command>\n\r", ch );
	return;
    }

    if ( ( victim = get_pc_world( ch, arg ) ) == NULL )
    {
	send_to_char( "They are not here.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "You can not grant yourself commands.\n\r", ch );
	return;
    }

    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( cmd_table[cmd].name[0] == argument[0]
	&&   cmd_table[cmd].level <= get_trust( ch )
	&&   !str_cmp( cmd_table[cmd].name, argument ) )
	{
	    found = TRUE;
	    break;
	}
    }

    if ( !found )
    {
	send_to_char( "Command not found.\n\r", ch );
	return;
    }

    for ( grant = victim->pcdata->grants; grant != NULL; grant = grant->next )
    {
	if ( !str_cmp( grant->command, argument ) )
	{
	    if ( grant == victim->pcdata->grants )
		victim->pcdata->grants = victim->pcdata->grants->next;
	    else
	    {
		GRANT_DATA *free;
		for ( free = victim->pcdata->grants; free != NULL; free = free->next )
		{
		    if ( free->next == grant )
		    {
			free->next = grant->next;
			break;
		    }
		}
	    }

	    act( "You strip $N of the powers of $t.",
		ch, argument, victim, TO_CHAR, POS_DEAD );
	    act( "Someone has stripped you of your powers of $t.",
		ch, argument, victim, TO_VICT, POS_DEAD );

	    sprintf( log_buf, "grant: strips %s of %s.", victim->name, argument );
	    parse_logs( ch, "immortal", log_buf );

	    free_grant( grant );
	    return;
	}
    }

    grant			= new_grant( );
    grant->command		= str_dup( argument );
    grant->granter		= str_dup( ch->name );
    grant->next			= victim->pcdata->grants;
    victim->pcdata->grants	= grant;

    act( "You grant $N the powers of $t.",
	ch, argument, victim, TO_CHAR, POS_DEAD );
    act( "Someone has granted you the powers of $t.",
	ch, argument, victim, TO_VICT, POS_DEAD );

    sprintf( log_buf, "grant: grants %s with %s.", victim->name, argument );
    parse_logs( ch, "immortal", log_buf );
}

void save_commands( void )
{
    FILE *fp;
    sh_int pos;

    if ( ( fp = fopen( COMMAND_FILE, "w" ) ) == NULL )
    {
	bug( "save_commands: can not open file.", 0 );
	return;
    }

//    fprintf( fp, "#MAX_COMMAND %d\n",	maxCommand			);

    for ( pos = 0; cmd_table[pos].name[0] != '\0'; pos++ )
    {
	fprintf( fp, "\n#Command\n"					);
	fprintf( fp, "Name %s~\n",	cmd_table[pos].name		);
	fprintf( fp, "Posi %d\n",	cmd_table[pos].position		);
	fprintf( fp, "Levl %d\n",	cmd_table[pos].level		);
	fprintf( fp, "Tier %d\n",	cmd_table[pos].tier		);
	fprintf( fp, "Logs %d\n",	cmd_table[pos].log		);
	fprintf( fp, "Show %d\n",	cmd_table[pos].show		);
	fprintf( fp, "End\n"						);
//    DO_FUN *            do_fun;
    }

    fprintf( fp, "#End\n"						);
    fclose( fp );
}

int cmd_lookup( char *command )
{
    int cmd;

    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	if ( command[0] == cmd_table[cmd].name[0]
	&&   !str_prefix( command, cmd_table[cmd].name ) )
	    return cmd;
    }

    return -1;
}

