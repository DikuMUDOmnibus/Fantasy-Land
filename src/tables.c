/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.   *
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
#include <stdio.h>
#include <time.h>
#include "merc.h"

struct channel_data channel_table[20] = { };

const struct devote_type devote_table[DEVOTE_CURRENT] =
{
    { "body",			DEVOTE_BODY,		"Moves/AC"	},
    { "mind",			DEVOTE_MIND,		"Saves/Wis"	},
    { "spirit",			DEVOTE_SPIRIT,		"Mana/Int"	},
    { "grace",			DEVOTE_GRACE,		"Hitroll/Dex"	},
    { "force",			DEVOTE_FORCE,		"Damroll/Str"	},
    { "life",			DEVOTE_LIFE,		"HP/Con"	},
    { "skills",			DEVOTE_SKILLS,		"More Skills"	},
    { "spells",			DEVOTE_SPELLS,		"More Spells"	},
    { "eq",			DEVOTE_EQ,		"More Eq"	}
};

const struct flag_type trap_type_table[TRAP_MAX] =
{
    { "damage",			TRAP_DAMAGE,		TRUE	},
    { "mana",			TRAP_MANA,		TRUE	},
    { "teleport",		TRAP_TELEPORT,		TRUE	},
    { "latch",			TRAP_LATCH,		TRUE	}
};

const struct spells_type object_affects[] =
{
    { "blindness",		AFF_BLIND,		TO_AFFECTS	},
    { "detect evil",		AFF_DETECT_EVIL,	TO_AFFECTS	},
    { "detect invis",		AFF_DETECT_INVIS,	TO_AFFECTS	},
    { "detect magic",		AFF_DETECT_MAGIC,	TO_AFFECTS	},
    { "detect hidden",		AFF_DETECT_HIDDEN,	TO_AFFECTS	},
    { "detect good",		AFF_DETECT_GOOD,	TO_AFFECTS	},
    { "detect neutral",		AFF_DETECT_NEUTRAL,	TO_AFFECTS	},
    { "faerie fire",		AFF_FAERIE_FIRE,	TO_AFFECTS	},
    { "infravision",		AFF_INFRARED,		TO_AFFECTS	},
    { "curse",			AFF_CURSE,		TO_AFFECTS	},
    { "farsight",		AFF_FARSIGHT,		TO_AFFECTS	},
    { "poison",			AFF_POISON,		TO_AFFECTS	},
    { "hide",			AFF_HIDE,		TO_AFFECTS	},
    { "sleep",			AFF_SLEEP,		TO_AFFECTS	},
    { "charm person",		AFF_CHARM,		TO_AFFECTS	},
    { "fly",			AFF_FLYING,		TO_AFFECTS	},
    { "pass door",		AFF_PASS_DOOR,		TO_AFFECTS	},
    { "haste",			AFF_HASTE,		TO_AFFECTS	},
    { "calm",			AFF_CALM,		TO_AFFECTS	},
    { "plague",			AFF_PLAGUE,		TO_AFFECTS	},
    { "weaken",			AFF_WEAKEN,		TO_AFFECTS	},
    { "darkvision",		AFF_DARK_VISION,	TO_AFFECTS	},
    { "regeneration",		AFF_REGENERATION,	TO_AFFECTS	},
    { "slow",			AFF_SLOW,		TO_AFFECTS	},
    { "giant strength",		AFF_GIANT_STRENGTH,	TO_AFFECTS	},
    { "detect terrain",		AFF_DETECT_TERRAIN,	TO_AFFECTS	},

    { "absorb magic",		SHD_ABSORB,		TO_SHIELDS	},
    { "protection voodoo",	SHD_PROTECT_VOODOO,	TO_SHIELDS	},
    { "invisibility",		SHD_INVISIBLE,		TO_SHIELDS	},
    { "iceshield",		SHD_ICE,		TO_SHIELDS	},
    { "fireshield",		SHD_FIRE,		TO_SHIELDS	},
    { "shockshield",		SHD_SHOCK,		TO_SHIELDS	},
    { "rockshield",		SHD_ROCK,		TO_SHIELDS	},
    { "shrapnelshield",		SHD_SHRAPNEL,		TO_SHIELDS	},
    { "sanctuary",		SHD_SANCTUARY,		TO_SHIELDS	},
    { "thornshield",		SHD_THORN,		TO_SHIELDS	},
    { "vampiricshield",		SHD_VAMPIRIC,		TO_SHIELDS	},
    { "divine aura",		SHD_DIVINE_AURA,	TO_SHIELDS	},
    { "acidshield",		SHD_ACID,		TO_SHIELDS	},
    { "protection neutral",	SHD_PROTECT_NEUTRAL,	TO_SHIELDS	},
    { "protection evil",	SHD_PROTECT_EVIL,	TO_SHIELDS	},
    { "protection good",	SHD_PROTECT_GOOD,	TO_SHIELDS	},
    { "mana shield",		SHD_MANA,		TO_SHIELDS	},
    { "divinity",		SHD_DIVINITY,		TO_SHIELDS	},
    { "watershield",		SHD_WATER,		TO_SHIELDS	},
    { NULL,			0,			0		}
};

const struct flag_type damage_mod_table[DAM_MAX] =
{
    { "acid",			DAM_ACID,		TRUE	},
    { "bash",			DAM_BASH,		TRUE	},
    { "charm",			DAM_CHARM,		TRUE	},
    { "cold",			DAM_COLD,		TRUE	},
    { "disease",		DAM_DISEASE,		TRUE	},
    { "energy",			DAM_ENERGY,		TRUE	},
    { "fire", 			DAM_FIRE,		TRUE	},
    { "fear",			DAM_FEAR,		TRUE	},
    { "holy",			DAM_HOLY,		TRUE	},
    { "iron",			DAM_IRON,		TRUE	},
    { "light",			DAM_LIGHT,		TRUE	},
    { "lightning",		DAM_LIGHTNING,		TRUE	},
    { "magic",			DAM_MAGIC,		TRUE	},
    { "mental",			DAM_MENTAL,		TRUE	},
    { "negative",		DAM_NEGATIVE,		TRUE	},
    { "air",			DAM_AIR,		TRUE	},
    { "other",			DAM_OTHER,		TRUE	},
    { "pierce",			DAM_PIERCE,		TRUE	},
    { "poison",			DAM_POISON,		TRUE	},
    { "silver",			DAM_SILVER,		TRUE	},
    { "slash",			DAM_SLASH,		TRUE	},
    { "sound",			DAM_SOUND,		TRUE	},
    { "water",			DAM_WATER,		TRUE	},
    { "earth",			DAM_EARTH,		TRUE	},
    { "shadow",			DAM_SHADOW,		TRUE	},
    { "wood",			DAM_WOOD,		TRUE	}
};

const struct flag_type plr_flags[] =
{
    { "detailexit",	PLR_DETAIL_EXIT,	FALSE	},
    { "color",		PLR_COLOUR,		FALSE	},
    { "autoassist",	PLR_AUTOASSIST,		FALSE	},
    { "autoexit",	PLR_AUTOEXIT,		FALSE	},
    { "autoloot",	PLR_AUTOLOOT,		FALSE	},
    { "autosac",	PLR_AUTOSAC,		FALSE	},
    { "autogold",	PLR_AUTOGOLD,		FALSE	},
    { "autosplit",	PLR_AUTOSPLIT,		FALSE	},
    { "notran",		PLR_NOTRAN,		FALSE	},
    { "autopeek",	PLR_AUTOPEEK,		FALSE	},
    { "custom_who",	PLR_CUSTOM_WHO,		FALSE	},
    { "holylight",	PLR_HOLYLIGHT,		FALSE	},
    { "nosummon",	PLR_NOSUMMON,		FALSE	},
    { "nofollow",	PLR_NOFOLLOW,		FALSE	},
    { "censored",	PLR_CENSORED,		TRUE	},
    { "noexp",		PLR_NOEXP,		FALSE	},
    { "log",		PLR_LOG,		FALSE	},
    { "twit",		PLR_TWIT,		FALSE	},
    { "reroll",		PLR_REROLL,		FALSE	},
    { "gquest",		PLR_GQUEST,		FALSE	},
    { NULL,		0,			FALSE	}
};

const struct flag_type affect_flags[] =
{
    { "blind",			AFF_BLIND,		TRUE	},
    { "detect_evil",		AFF_DETECT_EVIL,	TRUE	},
    { "detect_invis",		AFF_DETECT_INVIS,	TRUE	},
    { "detect_magic",		AFF_DETECT_MAGIC,	TRUE	},
    { "detect_hidden",		AFF_DETECT_HIDDEN,	TRUE	},
    { "detect_good",		AFF_DETECT_GOOD,	TRUE	},
    { "detect_neutral",		AFF_DETECT_NEUTRAL,	TRUE	},
    { "faerie_fire",		AFF_FAERIE_FIRE,	TRUE	},
    { "infrared",		AFF_INFRARED,		TRUE	},
    { "curse",			AFF_CURSE,		TRUE	},
    { "farsight",		AFF_FARSIGHT,		TRUE	},
    { "poison",			AFF_POISON,		TRUE	},
    { "sneak",			AFF_SNEAK,		TRUE	},
    { "hide",			AFF_HIDE,		TRUE	},
    { "sleep",			AFF_SLEEP,		TRUE	},
    { "charm",			AFF_CHARM,		TRUE	},
    { "flying",			AFF_FLYING,		TRUE	},
    { "pass_door",		AFF_PASS_DOOR,		TRUE	},
    { "haste",			AFF_HASTE,		TRUE	},
    { "calm",			AFF_CALM,		TRUE	},
    { "plague",			AFF_PLAGUE,		TRUE	},
    { "weaken",			AFF_WEAKEN,		TRUE	},
    { "darkvision",		AFF_DARK_VISION,	TRUE	},
    { "regeneration",		AFF_REGENERATION,	TRUE	},
    { "slow",			AFF_SLOW,		TRUE	},
    { "giant_strength",		AFF_GIANT_STRENGTH,	TRUE	},
    { NULL,			0,			0	}
};

const struct flag_type shield_flags[] =
{
    { "protect_voodoo",		SHD_PROTECT_VOODOO,	TRUE	},
    { "invisibility",		SHD_INVISIBLE,		TRUE	},
    { "iceshield",		SHD_ICE,		TRUE	},
    { "fireshield",		SHD_FIRE,		TRUE	},
    { "shockshield",		SHD_SHOCK,		TRUE	},
    { "rockshield",		SHD_ROCK,		TRUE	},
    { "shrapnelshield",		SHD_SHRAPNEL,		TRUE	},
    { "sanctuary",		SHD_SANCTUARY,		TRUE	},
    { "thornshield",		SHD_THORN,		TRUE	},
    { "vampiricshield",		SHD_VAMPIRIC,		TRUE	},
    { "divine_aura",		SHD_DIVINE_AURA,	TRUE	},
    { "acidshield",		SHD_ACID,		TRUE	},
    { "protect_neutral",	SHD_PROTECT_NEUTRAL,	TRUE	},
    { "protect_evil",		SHD_PROTECT_EVIL,	TRUE	},
    { "protect_good",		SHD_PROTECT_GOOD,	TRUE	},
    { "mana_shield",		SHD_MANA,		TRUE	},
    { "divinity",		SHD_DIVINITY,		TRUE	},
    { "watershield",		SHD_WATER,		TRUE	},
    { "absorb",			SHD_ABSORB,		TRUE	},
    { NULL,			0,			0	}
};

const struct flag_type part_flags[] =
{
    { "head",			PART_HEAD,		TRUE	},
    { "arms",			PART_ARMS,		TRUE	},
    { "legs",			PART_LEGS,		TRUE	},
    { "heart",			PART_HEART,		TRUE	},
    { "brains",			PART_BRAINS,		TRUE	},
    { "guts",			PART_GUTS,		TRUE	},
    { "hands",			PART_HANDS,		TRUE	},
    { "feet",			PART_FEET,		TRUE	},
    { "fingers",		PART_FINGERS,		TRUE	},
    { "ear",			PART_EAR,		TRUE	},
    { "eye",			PART_EYE,		TRUE	},
    { "long_tongue",		PART_LONG_TONGUE,	TRUE	},
    { "eyestalks",		PART_EYESTALKS,		TRUE	},
    { "tentacles",		PART_TENTACLES,		TRUE	},
    { "fins",			PART_FINS,		TRUE	},
    { "wings",			PART_WINGS,		TRUE	},
    { "tail",			PART_TAIL,		TRUE	},
    { "claws",			PART_CLAWS,		TRUE	},
    { "fangs",			PART_FANGS,		TRUE	},
    { "horns",			PART_HORNS,		TRUE	},
    { "scales",			PART_SCALES,		TRUE	},
    { "tusks",			PART_TUSKS,		TRUE	},
    { NULL,			0,			0	}
};

const struct flag_type comm_flags[] =
{
    { "nowiz",			COMM_NOWIZ,		TRUE	},
    { "noclangossip",		COMM_NOOOC,		TRUE	},
    { "nogossip",		COMM_NOGOSSIP,		TRUE	},
    { "nocgossip",		COMM_NOCGOSSIP,		TRUE	},
    { "noqgossip",		COMM_NOQGOSSIP,		TRUE	},
    { "noask",			COMM_NOASK,		TRUE	},
    { "noclan",			COMM_NOCLAN,		TRUE	},
    { "nosocial",		COMM_NOSOCIAL,		TRUE	},
    { "noquote",		COMM_NOQUOTE,		TRUE	},
    { "prompt",			COMM_PROMPT,		TRUE	},
    { "nograts",		COMM_NOGRATS,		TRUE	},
    { "wiped",			COMM_WIPED,		TRUE	},
    { "afk",			COMM_AFK,		TRUE	},
    { "noauction",		COMM_NOAUCTION,		TRUE	},
    { "norace",			COMM_NORACE,		TRUE	},
    { "noflame",		COMM_NOFLAME,		TRUE	},
    { "nohero",			COMM_NOHERO,		TRUE	},
    { "noarena",		COMM_NOARENA,		TRUE	},
    { "noic",			COMM_NOIC,		TRUE	},
    { "nopray",			COMM_NOPRAY,		TRUE	},
    { "nonewbie",		COMM_NONEWBIE,		TRUE	},
    { NULL,			0,			0	}
};

const struct flag_type mprog_flags[] =
{
    { "act",			TRIG_ACT,		TRUE	},
    { "bribe",			TRIG_BRIBE,		TRUE 	},
    { "death",			TRIG_DEATH,		TRUE    },
    { "entry",			TRIG_ENTRY,		TRUE	},
    { "fight",			TRIG_FIGHT,		TRUE	},
    { "give",			TRIG_GIVE,		TRUE	},
    { "greet",			TRIG_GREET,		TRUE    },
    { "grall",			TRIG_GRALL,		TRUE	},
    { "kill",			TRIG_KILL,		TRUE	},
    { "hpcnt",			TRIG_HPCNT,		TRUE    },
    { "random",			TRIG_RANDOM,		TRUE	},
    { "speech",			TRIG_SPEECH,		TRUE	},
    { "exit",			TRIG_EXIT,		TRUE    },
    { "exall",			TRIG_EXALL,		TRUE    },
    { "delay",			TRIG_DELAY,		TRUE    },
    { NULL,			0,			TRUE	}
};

const struct flag_type area_flags[] =
{
    { "changed",		AREA_CHANGED,		TRUE	},
    { "unlinked",		AREA_UNLINKED,		TRUE	},
    { "special",		AREA_SPECIAL,		TRUE	},
    { "no_run",			AREA_NO_RUN,		TRUE	},
    { "no_quest",		AREA_NO_QUEST,		TRUE	},
    { NULL,			0,			0	}
};

const struct flag_type sex_flags[] =
{
    { "neutral",		SEX_NEUTRAL,		TRUE	},
    { "male",			SEX_MALE,		TRUE	},
    { "female",			SEX_FEMALE,		TRUE	},
    { "random",			SEX_RANDOM,		TRUE	},
    { NULL,			0,			0	}
};

const struct flag_type exit_flags[] =
{
    { "door",			EX_ISDOOR,		TRUE    },
    { "closed",			EX_CLOSED,		TRUE	},
    { "locked",			EX_LOCKED,		TRUE	},
    { "hidden",			EX_HIDDEN,		TRUE	},
    { "noblink",		EX_NOBLINK,		TRUE	},
    { "pickproof",		EX_PICKPROOF,		TRUE	},
    { "bashproof",		EX_BASHPROOF,		TRUE	},
    { "nopass",			EX_NOPASS,		TRUE	},
    { "noclose",		EX_NOCLOSE,		TRUE	},
    { "nolock",			EX_NOLOCK,		TRUE	},
    { "noscan",			EX_NO_SCAN,		TRUE	},
    { "jammed",                 EX_JAMMED,              TRUE    },
    { "concealed",              EX_CONCEALED,           TRUE    },
    { "secret",                 EX_SECRET,              TRUE    },
    { "revealed",               EX_REVEALED,            TRUE    },
    { "eat",                    EX_EAT_KEY,             TRUE    },
    { "window",                 EX_WINDOW,              TRUE    },
    { "transparent",            EX_TRANSPARENT,         TRUE    },
    { "sliding",                EX_SLIDING,             TRUE    },
    { "nomove",                 EX_NOMOVE,              TRUE    },
    { "jumponly",               EX_JUMPONLY,            TRUE    },
    { NULL,			0,			0	}
};

const struct flag_type room_flags[] =
{
    { "dark",			ROOM_DARK,		TRUE	},
    { "arena",			ROOM_ARENA,		TRUE	},
    { "war",			ROOM_WAR,		TRUE	},
    { "clan_portal",		ROOM_CLAN_PORTAL,	TRUE	},
    { "icy",			ROOM_ICY,		TRUE	},
    { "no_gate",		ROOM_NO_GATE,		TRUE	},
    { "no_mob",			ROOM_NO_MOB,		TRUE	},
    { "indoors",		ROOM_INDOORS,		TRUE	},
    { "private",		ROOM_PRIVATE,		TRUE    },
    { "safe",			ROOM_SAFE,		TRUE	},
    { "solitary",		ROOM_SOLITARY,		TRUE	},
    { "pet_shop",		ROOM_PET_SHOP,		TRUE	},
    { "no_recall",		ROOM_NO_RECALL,		TRUE	},
    { "imp_only",		ROOM_IMP_ONLY,		TRUE    },
    { "gods_only",	        ROOM_GODS_ONLY,		TRUE    },
    { "heroes_only",		ROOM_HEROES_ONLY,	TRUE	},
    { "newbies_only",		ROOM_NEWBIES_ONLY,	TRUE	},
    { "law",			ROOM_LAW,		TRUE	},
    { "nowhere",		ROOM_NOWHERE,		TRUE	},
    { "saving",		        ROOM_SAVE_CONTENTS,	TRUE	}, // added by Locke 10/25/2012
    { NULL,			0,			0	}
};

const struct flag_type sector_type[] =
{
    {	"inside",	SECT_INSIDE,		TRUE	},
    {	"city",		SECT_CITY,		TRUE	},
    {	"field",	SECT_FIELD,		TRUE	},
    {	"forest",	SECT_FOREST,		TRUE	},
    {	"hills",	SECT_HILLS,		TRUE	},
    {	"mountain",	SECT_MOUNTAIN,		TRUE	},
    {	"swim",		SECT_WATER_SWIM,	TRUE	},
    {	"noswim",	SECT_WATER_NOSWIM,	TRUE	},
    {   "climb",	SECT_CLIMB,		TRUE	},
    {	"air",		SECT_AIR,		TRUE	},
    {	"desert",	SECT_DESERT,		TRUE	},
    {	NULL,		0,			0	}
};

const struct flag_type type_flags[] =
{
    {	"light",		ITEM_LIGHT,		TRUE	},
    {	"scroll",		ITEM_SCROLL,		TRUE	},
    {	"wand",			ITEM_WAND,		TRUE	},
    {	"staff",		ITEM_STAFF,		TRUE	},
    {	"weapon",		ITEM_WEAPON,		TRUE	},
    {	"treasure",		ITEM_TREASURE,		TRUE	},
    {	"armor",		ITEM_ARMOR,		TRUE	},
    {	"potion",		ITEM_POTION,		TRUE	},
    {	"clothing",		ITEM_CLOTHING,		TRUE	},
    {	"furniture",		ITEM_FURNITURE,		TRUE	},
    {	"trash",		ITEM_TRASH,		TRUE	},
    {	"container",		ITEM_CONTAINER,		TRUE	},
    {	"drinkcontainer",	ITEM_DRINK_CON,		TRUE	},
    {	"key",			ITEM_KEY,		TRUE	},
    {	"food",			ITEM_FOOD,		TRUE	},
    {	"money",		ITEM_MONEY,		TRUE	},
    {	"boat",			ITEM_BOAT,		TRUE	},
    {	"npc_corpse",		ITEM_CORPSE_NPC,	TRUE	},
    {	"pc_corpse",		ITEM_CORPSE_PC,		TRUE	},
    {	"fountain",		ITEM_FOUNTAIN,		TRUE	},
    {	"pill",			ITEM_PILL,		TRUE	},
    {	"map",			ITEM_MAP,		TRUE	},
    {   "portal",		ITEM_PORTAL,		TRUE	},
    {   "warp_stone",		ITEM_WARP_STONE,	TRUE	},
    { 	"gem",			ITEM_GEM,		TRUE	},
    {	"jewelry",		ITEM_JEWELRY,		TRUE	},
    {	"pit",			ITEM_PIT,		TRUE	},
    {	"slots",		ITEM_SLOTS,		TRUE	},
    {	"brew_component",	ITEM_COMPONENT_BREW,	TRUE	},
    {	"scribe_componenet",	ITEM_COMPONENT_SCRIBE,	TRUE	},
    {	"forge_stone",		ITEM_FORGE_STONE,	TRUE	},
    {   "demon_stone",          ITEM_DEMON_STONE,       TRUE    },
    {	"engineer_tool",	ITEM_ENGINEER_TOOL,	TRUE	},
    {	"grenade",		ITEM_GRENADE,		TRUE	},
    {	"trap",			ITEM_TRAP,		TRUE	},
    {	"mine",			ITEM_MINE,		TRUE	},
    {	NULL,			0,			0	}
};

const struct flag_type extra_flags[] =
{
    {	"glow",			ITEM_GLOW,		TRUE	},
    {	"hum",			ITEM_HUM,		TRUE	},
    {	"dark",			ITEM_DARK,		TRUE	},
    {	"evil",			ITEM_EVIL,		TRUE	},
    {	"invis",		ITEM_INVIS,		TRUE	},
    {	"magic",		ITEM_MAGIC,		TRUE	},
    {	"nodrop",		ITEM_NODROP,		TRUE	},
    {	"bless",		ITEM_BLESS,		TRUE	},
    {	"antigood",		ITEM_ANTI_GOOD,		TRUE	},
    {	"antievil",		ITEM_ANTI_EVIL,		TRUE	},
    {	"antineutral",		ITEM_ANTI_NEUTRAL,	TRUE	},
    {	"noremove",		ITEM_NOREMOVE,		TRUE	},
    {	"inventory",		ITEM_INVENTORY,		TRUE	},
    {	"nopurge",		ITEM_NOPURGE,		TRUE	},
    {	"rotdeath",		ITEM_ROT_DEATH,		TRUE	},
    {	"visdeath",		ITEM_VIS_DEATH,		TRUE	},
    {   "nonmetal",		ITEM_NONMETAL,		TRUE	},
    {	"meltdrop",		ITEM_MELT_DROP,		TRUE	},
    {	"sellextract",		ITEM_SELL_EXTRACT,	TRUE	},
    {	"burnproof",		ITEM_BURN_PROOF,	TRUE	},
    {	"nouncurse",		ITEM_NOUNCURSE,		TRUE	},
    {	"nolocate",		ITEM_NOLOCATE,		TRUE	},
    {	"special_save",		ITEM_SPECIAL_SAVE,	TRUE	},
    {   "questpoint",		ITEM_QUESTPOINT,	TRUE	},
    {	"aquest",		ITEM_AQUEST,		TRUE	},
    {	"forged",		ITEM_FORGED,		TRUE	},
    {	"disintegrate",		ITEM_DISINTEGRATE,	TRUE	},
    {	"random_stats",		ITEM_RANDOM_STATS,	TRUE	},
    {	"nosac",		ITEM_NO_SAC,		TRUE	},
    {	NULL,			0,			0	}
};

const struct flag_type wear_flags[] =
{
    {	"take",			ITEM_TAKE,		TRUE	},
    {	"finger",		ITEM_WEAR_FINGER,	TRUE	},
    {	"neck",			ITEM_WEAR_NECK,		TRUE	},
    {	"torso",		ITEM_WEAR_BODY,		TRUE	},
    {	"head",			ITEM_WEAR_HEAD,		TRUE	},
    {	"legs",			ITEM_WEAR_LEGS,		TRUE	},
    {	"feet",			ITEM_WEAR_FEET,		TRUE	},
    {	"hands",		ITEM_WEAR_HANDS,	TRUE	},
    {	"arms",			ITEM_WEAR_ARMS,		TRUE	},
    {	"shield",		ITEM_WEAR_SHIELD,	TRUE	},
    {	"about",		ITEM_WEAR_ABOUT,	TRUE	},
    {	"waist",		ITEM_WEAR_WAIST,	TRUE	},
    {	"wrist",		ITEM_WEAR_WRIST,	TRUE	},
    {	"wield",		ITEM_WIELD,		TRUE	},
    {	"hold",			ITEM_HOLD,		TRUE	},
    {	"float",		ITEM_WEAR_FLOAT,	TRUE	},
    {	"face",			ITEM_WEAR_FACE,		TRUE	},
    {	"ear",			ITEM_WEAR_EAR,		TRUE	},
    {	"back",			ITEM_WEAR_BACK,		TRUE	},
    {	"ankle",		ITEM_WEAR_ANKLE,	TRUE	},
    {	"chest",		ITEM_WEAR_CHEST,	TRUE	},
    {	"soul",			ITEM_WEAR_SOUL,		FALSE	},
    {	"eyes",			ITEM_WEAR_EYES,		TRUE	},
    {	"clan",			ITEM_WEAR_CLAN,		TRUE	},
    {	NULL,			0,			0	}
};

const struct flag_type apply_flags[] =
{
    {	"none",			APPLY_NONE,		TRUE	},
    {	"strength",		APPLY_STR,		TRUE	},
    {	"dexterity",		APPLY_DEX,		TRUE	},
    {	"intelligence",		APPLY_INT,		TRUE	},
    {	"wisdom",		APPLY_WIS,		TRUE	},
    {	"constitution",		APPLY_CON,		TRUE	},
    {	"sex",			APPLY_SEX,		TRUE	},
    {	"mana",			APPLY_MANA,		TRUE	},
    {	"hp",			APPLY_HIT,		TRUE	},
    {	"move",			APPLY_MOVE,		TRUE	},
    {	"ac",			APPLY_AC,		TRUE	},
    {	"hitroll",		APPLY_HITROLL,		TRUE	},
    {	"damroll",		APPLY_DAMROLL,		TRUE	},
    {	"saves",		APPLY_SAVES,		TRUE	},
    {	"magic-power",		APPLY_MAGIC_POWER,	TRUE	},
    {	"hp-regeneration",	APPLY_REGEN_HP,		TRUE	},
    {	"mana-regeneration",	APPLY_REGEN_MANA,	TRUE	},
    {	"move-regeneration",	APPLY_REGEN_MOVE,	TRUE	},
    {	"size",			APPLY_SIZE,		FALSE	},
    {	"+AC,-HR,-DR",		APPLY_DAMAGE,		FALSE	},
    {	NULL,			0,			0	}
};

const struct flag_type wear_loc_strings[] =
{
    {	"in the inventory",	WEAR_NONE,	TRUE	},
    {	"as a light",		WEAR_LIGHT,	TRUE	},
    {	"on the left finger",	WEAR_FINGER_L,	TRUE	},
    {	"on the right finger",	WEAR_FINGER_R,	TRUE	},
    {	"around the neck (1)",	WEAR_NECK_1,	TRUE	},
    {	"around the neck (2)",	WEAR_NECK_2,	TRUE	},
    {	"on the torso",		WEAR_BODY,	TRUE	},
    {	"over the head",	WEAR_HEAD,	TRUE	},
    {	"on the legs",		WEAR_LEGS,	TRUE	},
    {	"on the feet",		WEAR_FEET,	TRUE	},
    {	"on the hands",		WEAR_HANDS,	TRUE	},
    {	"on the arms",		WEAR_ARMS,	TRUE	},
    {	"as a shield",		WEAR_SHIELD,	TRUE	},
    {	"about the shoulders",	WEAR_ABOUT,	TRUE	},
    {	"around the waist",	WEAR_WAIST,	TRUE	},
    {	"on the left wrist",	WEAR_WRIST_L,	TRUE	},
    {	"on the right wrist",	WEAR_WRIST_R,	TRUE	},
    {	"wielded",		WEAR_WIELD,	TRUE	},
    {	"secondary",		WEAR_SECONDARY,	TRUE	},
    {	"held in the hands",	WEAR_HOLD,	TRUE	},
    {	"floating nearby",	WEAR_FLOAT,	TRUE	},
    {   "on the left ear",	WEAR_EAR_L,	TRUE	},
    {	"on the right ear",	WEAR_EAR_R,	TRUE	},
    {	"on the back",		WEAR_BACK,	TRUE	},
    {	"around the l-ankle",	WEAR_ANKLE_L,	TRUE	},
    {	"around the r-ankle",	WEAR_ANKLE_R,	TRUE	},
    {	"on the chest",		WEAR_CHEST,	TRUE	},
    {	"on the face of",	WEAR_FACE,	TRUE	},
    {	"over the eyes",	WEAR_EYES,	TRUE	},
    {	"as a clan symbol",	WEAR_CLAN,	TRUE	},
    {	NULL,			0	      , 0	}
};

const struct flag_type wear_loc_flags[] =
{
    {	"none",		WEAR_NONE,	TRUE	},
    {	"light",	WEAR_LIGHT,	TRUE	},
    {	"lfinger",	WEAR_FINGER_L,	TRUE	},
    {	"rfinger",	WEAR_FINGER_R,	TRUE	},
    {	"neck1",	WEAR_NECK_1,	TRUE	},
    {	"neck2",	WEAR_NECK_2,	TRUE	},
    {	"torso",	WEAR_BODY,	TRUE	},
    {	"head",		WEAR_HEAD,	TRUE	},
    {	"legs",		WEAR_LEGS,	TRUE	},
    {	"feet",		WEAR_FEET,	TRUE	},
    {	"hands",	WEAR_HANDS,	TRUE	},
    {	"arms",		WEAR_ARMS,	TRUE	},
    {	"shield",	WEAR_SHIELD,	TRUE	},
    {	"about",	WEAR_ABOUT,	TRUE	},
    {	"waist",	WEAR_WAIST,	TRUE	},
    {	"lwrist",	WEAR_WRIST_L,	TRUE	},
    {	"rwrist",	WEAR_WRIST_R,	TRUE	},
    {	"wielded",	WEAR_WIELD,	TRUE	},
    {	"hold",		WEAR_HOLD,	TRUE	},
    {	"floating",	WEAR_FLOAT,	TRUE	},
    {   "l-ear",	WEAR_EAR_L,	TRUE	},
    {	"r-ear",	WEAR_EAR_R,	TRUE	},
    {	"back",		WEAR_BACK,	TRUE	},
    {	"l-ankle",	WEAR_ANKLE_L,	TRUE	},
    {	"r-ankle",	WEAR_ANKLE_R,	TRUE	},
    {	"chest",	WEAR_CHEST,	TRUE	},
    {	"eyes",		WEAR_EYES,	TRUE	},
    {	"clan",		WEAR_CLAN,	TRUE	},
    {	"face",		WEAR_FACE,	TRUE	},
    {   "secondary",	WEAR_SECONDARY, TRUE	},
    {	NULL,		0,		0	}
};

const struct flag_type container_flags[] =
{
    {	"closeable",		1,		TRUE	},
    {	"pickproof",		2,		TRUE	},
    {	"closed",		4,		TRUE	},
    {	"locked",		8,		TRUE	},
    {	"puton",		16,		TRUE	},
    {	NULL,			0,		0	}
};

const struct flag_type ac_type[] =
{
    {   "pierce",        AC_PIERCE,            TRUE    },
    {   "bash",          AC_BASH,              TRUE    },
    {   "slash",         AC_SLASH,             TRUE    },
    {   "exotic",        AC_EXOTIC,            TRUE    },
    {   NULL,            0,                    0       }
};

const struct flag_type size_flags[] =
{
    {   "tiny",          SIZE_TINY,            TRUE    },
    {   "small",         SIZE_SMALL,           TRUE    },
    {   "medium",        SIZE_MEDIUM,          TRUE    },
    {   "large",         SIZE_LARGE,           TRUE    },
    {   "huge",          SIZE_HUGE,            TRUE    },
    {   "giant",         SIZE_GIANT,           TRUE    },
    {   "adjustable",	 SIZE_NONE,	       TRUE    },
    {   NULL,            0,                    0       }
};

const struct flag_type weapon_class[] =
{
    { "exotic",		WEAPON_EXOTIC,		TRUE    },
    { "sword",		WEAPON_SWORD,		TRUE    },
    { "dagger",		WEAPON_DAGGER,		TRUE    },
    { "spear",		WEAPON_SPEAR,		TRUE    },
    { "mace",		WEAPON_MACE,		TRUE    },
    { "axe",		WEAPON_AXE,		TRUE    },
    { "flail",		WEAPON_FLAIL,		TRUE    },
    { "whip",		WEAPON_WHIP,		TRUE    },
    { "polearm",	WEAPON_POLEARM,		TRUE    },
    { "quarterstaff",	WEAPON_QUARTERSTAFF,	TRUE	},
    { NULL,		0,			0       }
};

const struct flag_type weapon_type2[] =
{
    { "flaming",	WEAPON_FLAMING,		TRUE	},
    { "frost",		WEAPON_FROST,		TRUE	},
    { "vampiric",	WEAPON_VAMPIRIC,	TRUE	},
    { "sharp",		WEAPON_SHARP,		TRUE	},
    { "vorpal",		WEAPON_VORPAL,		TRUE	},
    { "twohands",	WEAPON_TWO_HANDS,	TRUE	},
    { "shocking",	WEAPON_SHOCKING,	TRUE	},
    { "poison",		WEAPON_POISON,		TRUE	},
    { NULL,		0,			0	}
};

const struct flag_type position_flags[] =
{
    {   "DEAD",           POS_DEAD,            FALSE   },
    {   "MORTAL",         POS_MORTAL,          FALSE   },
    {   "INCAP",          POS_INCAP,           FALSE   },
    {   "STUNNED",        POS_STUNNED,         FALSE   },
    {   "SLEEPING",       POS_SLEEPING,        TRUE    },
    {   "RESTING",        POS_RESTING,         TRUE    },
    {   "SITTING",        POS_SITTING,         TRUE    },
    {   "FIGHTING",       POS_FIGHTING,        FALSE   },
    {   "STANDING",       POS_STANDING,        TRUE    },
    {   NULL,             0,                   0       }
};

const struct flag_type portal_flags[]=
{
    {   "normal_exit",	  GATE_NORMAL_EXIT,	TRUE	},
    {	"no_curse",	  GATE_NOCURSE,		TRUE	},
    {   "go_with",	  GATE_GOWITH,		TRUE	},
    {   "buggy",	  GATE_BUGGY,		TRUE	},
    {	"random",	  GATE_RANDOM,		TRUE	},
    {   NULL,		  0,			0	}
};

const struct flag_type furniture_flags[]=
{
    {   "stand_at",	  STAND_AT,		TRUE	},
    {	"stand_on",	  STAND_ON,		TRUE	},
    {	"stand_in",	  STAND_IN,		TRUE	},
    {	"sit_at",	  SIT_AT,		TRUE	},
    {	"sit_on",	  SIT_ON,		TRUE	},
    {	"sit_in",	  SIT_IN,		TRUE	},
    {	"rest_at",	  REST_AT,		TRUE	},
    {	"rest_on",	  REST_ON,		TRUE	},
    {	"rest_in",	  REST_IN,		TRUE	},
    {	"sleep_at",	  SLEEP_AT,		TRUE	},
    {	"sleep_on",	  SLEEP_ON,		TRUE	},
    {	"sleep_in",	  SLEEP_IN,		TRUE	},
    {	"put_at",	  PUT_AT,		TRUE	},
    {	"put_on",	  PUT_ON,		TRUE	},
    {	"put_in",	  PUT_IN,		TRUE	},
    {	"put_inside",	  PUT_INSIDE,		TRUE	},
    {	NULL,		  0,			0	}
};

const	struct	flag_type	apply_types	[]	=
{
    {	"affects",	TO_AFFECTS,	TRUE	},
    {	"shields",	TO_SHIELDS,	TRUE	},
    {	NULL,		0,		TRUE	}
};

const	struct	bit_type	bitvector_type	[]	=
{
    {	affect_flags,		"affect"	},
    {	shield_flags,		"shield"	}
};

const struct flag_type sound_flags[] =
{
    { "off",		SOUND_ON,		FALSE	},
    { "nomusic",	SOUND_NOMUSIC,		FALSE	},
    { "nocombat",	SOUND_NOCOMBAT,		FALSE	},
    { "noweather",	SOUND_NOWEATHER,	FALSE	},
    { "nospell",	SOUND_NOSKILL,		FALSE	},
    { "noclan",		SOUND_NOCLAN,		FALSE	},
    { "nozone",		SOUND_NOZONE,		FALSE	},
    { "nomisc",		SOUND_NOMISC,		FALSE	},
    { NULL,		0,			0	}
};

const struct flag_type oprog_flags[] =
{
    {	"act",			TRIG_ACT,		TRUE	},
    {	"fight",		TRIG_FIGHT,		TRUE	},
    {	"give",			TRIG_GIVE,		TRUE	},
    {   "grall",		TRIG_GRALL,		TRUE	},
    {	"random",		TRIG_RANDOM,		TRUE	},
    {   "speech",		TRIG_SPEECH,		TRUE	},
    {	"exall",		TRIG_EXALL,		TRUE	},
    {	"delay",		TRIG_DELAY,		TRUE	},
    {	"drop",			TRIG_DROP,		TRUE	},
    {	"get",			TRIG_GET,		TRUE	},
    {	"sit",			TRIG_SIT,		TRUE	},
    {	NULL,			0,			TRUE	},
};

const struct flag_type rprog_flags[] =
{
    {	"act",			TRIG_ACT,		TRUE	},
    {	"fight",		TRIG_FIGHT,		TRUE	},
    {	"drop",			TRIG_DROP,		TRUE	},
    {	"grall",		TRIG_GRALL,		TRUE	},
    {	"random",		TRIG_RANDOM,		TRUE	},
    {	"speech",		TRIG_SPEECH,		TRUE	},
    {	"exall",		TRIG_EXALL,		TRUE	},
    {	"delay",		TRIG_DELAY,		TRUE	},
    {	NULL,			0,			TRUE	},
};

const struct flag_type act_flags[] =
{
    { "noquest",		ACT_NOQUEST,			TRUE	},
    { "sentinel",		ACT_SENTINEL,			TRUE	},
    { "scavenger",		ACT_SCAVENGER,			TRUE	},
    { "aggressive",		ACT_AGGRESSIVE,			TRUE	},
    { "smart_mob",		ACT_SMART_MOB,			TRUE	},
    { "stay_area",		ACT_STAY_AREA,			TRUE	},
    { "wimpy",			ACT_WIMPY,			TRUE	},
    { "pet",			ACT_PET,			TRUE	},
    { "train",			ACT_TRAIN,			TRUE	},
    { "practice",		ACT_PRACTICE,			TRUE	},
    { "no_body",		ACT_NO_BODY,			TRUE	},
    { "nobd_drop",		ACT_NB_DROP,			TRUE	},
    { "nopurge",		ACT_NOPURGE,			TRUE	},
    { "is_satan",		ACT_IS_SATAN,			TRUE	},
    { "is_priest",		ACT_IS_PRIEST,			TRUE	},
    { "healer",			ACT_IS_HEALER,			TRUE	},
    { "gain",			ACT_GAIN,			TRUE	},
    { "assist_all",		ACT_ASSIST_ALL,			TRUE	},
    { "assist_align",		ACT_ASSIST_ALIGN,		TRUE	},
    { "assist_race",		ACT_ASSIST_RACE,		TRUE	},
    { "assist_players",		ACT_ASSIST_PLAYERS,		TRUE	},
    { "assist_guard",		ACT_ASSIST_GUARD,		TRUE	},
    { "assist_vnum",		ACT_ASSIST_VNUM,		TRUE	},
    { "area_attack",		ACT_AREA_ATTACK,		TRUE	},
    { "see_all",		ACT_SEE_ALL,			TRUE	},
    { "no_return_home",		ACT_NO_RETURN_HOME,		TRUE	},
    { "no_where",		ACT_NO_WHERE,			TRUE	},
    { "noscan",                 ACT_NOSCAN,                     TRUE    },
    { NULL,			0,				FALSE	}
};

const struct flag_type skill_flags[] =
{
    { "disabled",		SKILL_DISABLED,			TRUE	},
    { "global_spellup",		SKILL_GLOBAL_SPELLUP,		TRUE	},
    { "no_empower_scroll",	SKILL_NO_EMPOWER_SCROLL,	TRUE	},
    { "no_empower_potion",	SKILL_NO_EMPOWER_POTION,	TRUE	},
    { "no_brew",		SKILL_NO_BREW,			TRUE	},
    { "no_scribe",		SKILL_NO_SCRIBE,		TRUE	},
    { "can_dispel",		SKILL_CAN_DISPEL,		TRUE	},
    { "can_cancel",		SKILL_CAN_CANCEL,		TRUE	},
    { "no_clan_potion",		SKILL_NO_CLAN_POTION,		TRUE	},
    { "no_clan_scroll",		SKILL_NO_CLAN_SCROLL,		TRUE	},
    { "no_clan_pill",		SKILL_NO_CLAN_PILL,		TRUE	},
    { "no_clan_wand",		SKILL_NO_CLAN_WAND,		TRUE	},
    { "no_clan_staff",		SKILL_NO_CLAN_STAFF,		TRUE	},
    { NULL,			0,				FALSE	}
};

const struct color_type color_table[] =
{
    { "RED",		C_RED		},
    { "GREEN",		C_GREEN		},
    { "YELLOW",		C_YELLOW	},
    { "BLUE",		C_BLUE		},
    { "MAGENTA",	C_MAGENTA	},
    { "CYAN",		C_CYAN		},
    { "WHITE",		C_WHITE		},
    { "GREY",		C_D_GREY	},
    { "BRIGHT_RED",	C_B_RED		},
    { "BRIGHT_GREEN",	C_B_GREEN	},
    { "BRIGHT_YELLOW",	C_B_YELLOW	},
    { "BRIGHT_BLUE",	C_B_BLUE	},
    { "BRIGHT_MAGENTA",	C_B_MAGENTA	},
    { "BRIGHT_CYAN",	C_B_CYAN	},
    { "BRIGHT_WHITE",	C_B_WHITE	},
    { "BLACK",		C_BLACK		},
    { NULL,		CLEAR		}
};

const struct flag_type door_resets[] =
{
    { "open and unlocked",	0,		TRUE	},
    { "closed and unlocked",	1,		TRUE	},
    { "closed and locked",	2,		TRUE	},
    { NULL,			0,		0	}
};


/*
 * Takes a string and returns its designated direction.
 */
int get_dir( char *arg )
{
    int door = MAX_DIR;

       if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = DIR_NORTH;
  else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = DIR_EAST;
  else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = DIR_SOUTH;
  else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = DIR_WEST;
  else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = DIR_UP;
  else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = DIR_DOWN;
// else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) ) door = DIR_NW;
// else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) ) door = DIR_NE;
// else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) ) door = DIR_SW;
// else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) ) door = DIR_SE;

    return door;
}
