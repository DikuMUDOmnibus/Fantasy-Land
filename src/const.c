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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

char * const penalty_name [PENALTY_MAX] =
{
    "corner", "nochannel", "notitle", "freeze", "norestore", "notell"
};

char * const dir_name [MAX_DIR] =
{
    "north", "east", "south", "west", "up", "down"
};

const sh_int rev_dir [MAX_DIR] =
{
    2, 3, 0, 1, 5, 4
};

const sh_int movement_loss [SECT_MAX] =
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6
};

const struct weapon_type weapon_table [] =
{
    { "none",	0,			0,		    &gsn_hand_to_hand },
    { "sword",	OBJ_SCHOOL_SWORD,	WEAPON_SWORD,	    &gsn_sword	      },
    { "mace",	OBJ_SCHOOL_MACE,	WEAPON_MACE,	    &gsn_mace 	      },
    { "dagger",	OBJ_SCHOOL_DAGGER,	WEAPON_DAGGER,	    &gsn_dagger	      },
    { "axe",	OBJ_SCHOOL_AXE,		WEAPON_AXE,	    &gsn_axe	      },
    { "spear",	OBJ_SCHOOL_SPEAR,	WEAPON_SPEAR,	    &gsn_spear	      },
    { "flail",	OBJ_SCHOOL_FLAIL,	WEAPON_FLAIL,	    &gsn_flail	      },
    { "whip",	OBJ_SCHOOL_WHIP,	WEAPON_WHIP,	    &gsn_whip	      },
    { "polearm",OBJ_SCHOOL_POLEARM,	WEAPON_POLEARM,	    &gsn_polearm      },
    { "quarterstaff", OBJ_QUARTERSTAFF,	WEAPON_QUARTERSTAFF,&gsn_quarterstaff },
    { NULL,	0,				0,	    NULL	      }
};

const struct wiznet_type wiznet_table[] =
{
   {    "on",           WIZ_ON,         0  },
   {    "prefix",	WIZ_PREFIX,	0  },
   {    "ticks",        WIZ_TICKS,      IM },
   {    "logins",       WIZ_LOGINS,     IM },
   {    "sites",        WIZ_SITES,      L1 },
   {    "links",        WIZ_LINKS,      L7 },
   {	"newbies",	WIZ_NEWBIE,	0  },
   {    "deaths",       WIZ_DEATHS,     IM },
   {    "resets",       WIZ_RESETS,     L4 },
   {    "mobdeaths",    WIZ_MOBDEATHS,  L4 },
   {    "flags",	WIZ_FLAGS,	L5 },
   {	"penalties",	WIZ_PENALTIES,	L5 },
   {	"saccing",	WIZ_SACCING,	L5 },
   {	"levels",	WIZ_LEVELS,	IM },
   {	"load",		WIZ_LOAD,	L2 },
   {	"restore",	WIZ_RESTORE,	L2 },
   {	"snoops",	WIZ_SNOOPS,	L2 },
   {	"switches",	WIZ_SWITCHES,	L2 },
   {	"secure",	WIZ_SECURE,	L1 },
   {	"passwords",	WIZ_PASSWORDS,	L1 },
   {	"memory",	WIZ_MEMCHECK,	L1 },
   {	"pkills",	WIZ_PKILLS,	L3 },
   {	"other",	WIZ_OTHER,	L5 },
   {	"clans",	WIZ_CLANS,	L2 },
   {	"chats",	WIZ_CHATS,	L2 },
   {	"player_log",	WIZ_PLAYER_LOG,	L1 },
   {	"tells",	WIZ_TELLS,	ML },
   {	NULL,		0,		0  }
};

const struct wiznet_type config_flags[] =
{
    { "Area Names",		CONFIG_AREA_NAME,	 0 },
    { "Brief Mode",		CONFIG_BRIEF,		 0 },
    { "Char Combine",		CONFIG_CHAR_COMBINE,	 0 },
    { "Compact Mode",		CONFIG_COMPACT,		 0 },
    { "Deaf Mode",		CONFIG_DEAF,		 0 },
    { "Inv No Combine",		CONFIG_NO_INV_COMBINE,	 0 },
    { "Loot Combine",		CONFIG_LOOT_COMBINE,	 0 },
    { "Long Flags",		CONFIG_LONG,		 0 },
    { "Show Affects",		CONFIG_SHOW_AFFECTS,	 0 },
    { "Store Tells",		CONFIG_STORE,		 0 },
    { "Goto Bypass",		CONFIG_GOTO_BYPASS,	IM },
    { "Redit Goto",		CONFIG_REDIT_GOTO,	IM },
    { "No Imm Spellups",	CONFIG_NO_SPELLUP,	 0 },
    { "ASCII Depictions",       CONFIG_ASCII,            0 },
    { NULL,			0,			 0 }
};

const struct info_type info_table[] =
{
    {	"on",		INFO_ON		},
    {	"deaths",	INFO_DEATHS	},
    {	"bounty",	INFO_BOUNTY	},
    {	"other",	INFO_OTHER	},
    {	"notes",	INFO_NOTES	},
    {	NULL,		0		}
};

const struct info_type combat_table[] =
{
    {	"on",		COMBAT_ON		},
    {	"flags",	COMBAT_FLAGS		},
    {	"shields",	COMBAT_SHIELDS		},
    {	"evasion",	COMBAT_EVASION		},
    {	"other",	COMBAT_OTHER		},
    {	"counter",	COMBAT_COUNTER		},
    {	"critical",	COMBAT_CRITICAL		},
    {	"bleeding",	COMBAT_BLEEDING		},
    {	"misses",	COMBAT_MISSES		},
    {	"chain spam",	COMBAT_CHAIN_SPAM	},
    {	"all shields",	COMBAT_SHD_COMBINE	},
    {	"meter",	COMBAT_METER		},
    {	NULL,		0			}
};

const struct attack_type attack_table[MAX_DAMAGE_MESSAGE+2] =
{
    { "none",		"hit",			-1		},  /*  0 */

    { "acid",		"acid",			DAM_ACID	},
    { "acbite",		"acidic bite",		DAM_ACID	},
    { "caustic",	"caustic touch",	DAM_ACID	},
    { "corrosion",	"corrosion",		DAM_ACID	},
    { "digestion",	"digestion",		DAM_ACID	},
    { "disintegration",	"disintegration",	DAM_ACID	},
    { "dissolve",	"dissolve",		DAM_ACID	},
    { "erosion",	"erosion",		DAM_ACID	},
    { "etching",	"etching",		DAM_ACID	},
    { "oxidization",	"oxidization",		DAM_ACID	},
    { "rust",		"rust",			DAM_ACID	},
    { "sizzle",		"sizzle",		DAM_ACID	},
    { "slime",		"slime",		DAM_ACID	},

    { "air",		"air",			DAM_AIR		},
    { "breeze",		"breeze",		DAM_AIR		},
    { "draft",		"draft",		DAM_AIR		},
    { "gust",		"gust",			DAM_AIR		},
    { "hurricane",	"hurricane",		DAM_AIR		},
    { "tempest",	"tempest",		DAM_AIR		},
    { "wind",		"wind",			DAM_AIR		},
    { "zephyr",		"zephyr",		DAM_AIR		},

    { "backhand",	"backhand",		DAM_BASH	},
    { "bash",		"bash",			DAM_BASH	},
    { "beating",	"beating",		DAM_BASH	},
    { "bitchslap",	"bitchslap",		DAM_BASH	},
    { "blast",		"blast",		DAM_BASH	},
    { "charge",		"charge",		DAM_BASH	},
    { "collision",	"collision",		DAM_BASH	},
    { "crack",		"crack",		DAM_BASH	},
    { "crush",		"crush",		DAM_BASH	},
    { "fracture",	"fracture",		DAM_BASH	},
    { "kick",		"kick",			DAM_BASH	},
    { "knee",		"knee",			DAM_BASH	},
    { "pimpslap",	"pimpslap",		DAM_BASH	},
    { "pound",		"pound",		DAM_BASH	},
    { "punch",		"punch",		DAM_BASH	},
    { "slap",		"slap",			DAM_BASH	},
    { "slug",		"slug",			DAM_BASH	},
    { "smack",		"smack",		DAM_BASH	},
    { "smash",		"smash",		DAM_BASH	},
    { "suction",	"suction",		DAM_BASH	},
    { "thwack",		"thwack",		DAM_BASH	},
    { "whack",		"whack",		DAM_BASH	},

    { "allure",		"allure",		DAM_CHARM	},
    { "appeal",		"appeal",		DAM_CHARM	},
    { "beauty",		"stunning beauty",	DAM_CHARM	},
    { "bewitching",	"bewitching",		DAM_CHARM	},
    { "charm",		"charm",		DAM_CHARM	},
    { "con",		"con",			DAM_CHARM	},
    { "hex",		"hex",			DAM_CHARM	},
    { "mesmerization",	"mesmerization",	DAM_CHARM	},
    { "possession",	"possession",		DAM_CHARM	},

    { "arctic",		"arctic wind",		DAM_COLD	},
    { "blizzard",	"blizzard",		DAM_COLD	},
    { "chill",		"chill",		DAM_COLD	},
    { "cold",		"cold",			DAM_COLD	},
    { "frbite",		"freezing bite",	DAM_COLD	},
    { "frigidity",	"frigidity",		DAM_COLD	},
    { "freeze",		"freeze",		DAM_COLD	},
    { "frost",		"frost",		DAM_COLD	},
    { "glacier",	"glacier",		DAM_COLD	},
    { "hail",		"hail",			DAM_COLD	},
    { "hailstone",	"hailstone",		DAM_COLD	},
    { "ice",		"ice",			DAM_COLD	},
    { "icicle",		"icicle",		DAM_COLD	},
    { "sleet",		"sleet",		DAM_COLD	},
    { "snow",		"snow",			DAM_COLD	},
    { "snowball",	"snowball",		DAM_COLD	},
    { "snowstorm",	"snowstorm",		DAM_COLD	},

    { "affliction",	"affliction",		DAM_DISEASE	},
    { "blight",		"blight",		DAM_DISEASE	},
    { "cancer",		"cancer",		DAM_DISEASE	},
    { "common_cold",	"common cold",		DAM_DISEASE	},
    { "contagion",	"contagion",		DAM_DISEASE	},
    { "contamination",	"contamination",	DAM_DISEASE	},
    { "decrepitude",	"decrepitude",		DAM_DISEASE	},
    { "disease",	"disease",		DAM_DISEASE	},
    { "epidemic",	"epidemic",		DAM_DISEASE	},
    { "fever",		"fever",		DAM_DISEASE	},
    { "illness",	"illness",		DAM_DISEASE	},
    { "infection",	"infection",		DAM_DISEASE	},
    { "infirmity",	"infirmity",		DAM_DISEASE	},
    { "malady",		"malady",		DAM_DISEASE	},
    { "plague",		"plague",		DAM_DISEASE	},
    { "sickness",	"sickness",		DAM_DISEASE	},
    { "virus",		"virus",		DAM_DISEASE	},

    { "avalanche",	"avalanche",		DAM_EARTH	},
    { "dust",		"dust",			DAM_EARTH	},
    { "earth",		"earth",		DAM_EARTH	},
    { "earthquake",	"earthquake",		DAM_EARTH	},
    { "falling_rock",	"falling rock",		DAM_EARTH	},
    { "landslide",	"landslide",		DAM_EARTH	},
    { "mudslide",	"mudslide",		DAM_EARTH	},
    { "sand",		"sand",			DAM_EARTH	},
    { "sandstorm",	"sandstorm",		DAM_EARTH	},
    { "stone",		"stone",		DAM_EARTH	},

    { "ardor",		"ardor",		DAM_ENERGY	},
    { "energy",		"energy",		DAM_ENERGY	},
    { "force",		"force",		DAM_ENERGY	},
    { "gravity",	"gravity",		DAM_ENERGY	},
    { "intensity",	"intensity",		DAM_ENERGY	},
    { "might",		"might",		DAM_ENERGY	},
    { "power",		"power",		DAM_ENERGY	},
    { "pressure",	"pressure",		DAM_ENERGY	},
    { "spirit",		"spirit",		DAM_ENERGY	},
    { "strength",	"strength",		DAM_ENERGY	},
    { "wrath",		"wrath",		DAM_ENERGY	},
    { "zeal",		"zeal",			DAM_ENERGY	},

    { "angst",		"angst",		DAM_FEAR	},
    { "anxiety",	"anxiety",		DAM_FEAR	},
    { "despair",	"despair",		DAM_FEAR	},
    { "dismay",		"dismay",		DAM_FEAR	},
    { "distress",	"distress",		DAM_FEAR	},
    { "fear",		"fear",			DAM_FEAR	},
    { "fright",		"fright",		DAM_FEAR	},
    { "horror",		"horror",		DAM_FEAR	},
    { "nightmare",	"nightmare",		DAM_FEAR	},
    { "panic",		"panic",		DAM_FEAR	},
    { "phobia",		"phobia",		DAM_FEAR	},
    { "terror",		"terror",		DAM_FEAR	},

    { "blaze",		"blaze",		DAM_FIRE	},
    { "char",		"char",			DAM_FIRE	},
    { "combustion",	"combustion",		DAM_FIRE	},
    { "corona",		"corona",		DAM_FIRE	},
    { "ember",		"ember",		DAM_FIRE	},
    { "fire",		"fire",			DAM_FIRE	},
    { "flame",		"flame",		DAM_FIRE	},
    { "flare",		"flare",		DAM_FIRE	},
    { "flbite",		"flaming bite", 	DAM_FIRE	},
    { "heat",		"heat",			DAM_FIRE	},
    { "holocaust",	"holocaust",		DAM_FIRE	},
    { "inferno",	"inferno",		DAM_FIRE	},
    { "pyre",		"pyre",			DAM_FIRE	},
    { "scorch",		"scorch",		DAM_FIRE	},
    { "spark",		"spark",		DAM_FIRE	},
    { "warmth",		"warmth",		DAM_FIRE	},

    { "chastity",	"chastity",		DAM_HOLY	},
    { "divine",		"divine power",		DAM_HOLY	},
    { "divinity",	"divinity",		DAM_HOLY	},
    { "exorcism",	"exorcism",		DAM_HOLY	},
    { "faith",		"faith",		DAM_HOLY	},
    { "glory",		"glory",		DAM_HOLY	},
    { "holy",		"holy",			DAM_HOLY	},
    { "morality",	"morality",		DAM_HOLY	},
    { "piety",		"piety",		DAM_HOLY	},
    { "retribution",	"retribution",		DAM_HOLY	},
    { "sanctity",	"sanctity",		DAM_HOLY	},
    { "virtue",		"virtue",		DAM_HOLY	},

    { "adamantium",	"adamantium",		DAM_IRON	},
    { "alloy",		"alloy",		DAM_IRON	},
    { "clash",		"clash",		DAM_IRON	},
    { "cold_steel",	"cold steel",		DAM_IRON	},
    { "ferric",		"ferric touch",		DAM_IRON	},
    { "iron",		"iron",			DAM_IRON	},
    { "lead",		"lead",			DAM_IRON	},
    { "metal",		"metal",		DAM_IRON	},
    { "steel",		"steel",		DAM_IRON	},
    { "tempered",	"tempered blade",	DAM_IRON	},

    { "aurora",		"aurora",		DAM_LIGHT	},
    { "brightness",	"brightness",		DAM_LIGHT	},
    { "dawn",		"dawn",			DAM_LIGHT	},
    { "flash",		"flash",		DAM_LIGHT	},
    { "glare",		"glare",		DAM_LIGHT	},
    { "glow",		"glow",			DAM_LIGHT	},
    { "illumination",	"illumination",		DAM_LIGHT	},
    { "light",		"light",		DAM_LIGHT	},
    { "luminescence",	"luminescence",		DAM_LIGHT	},
    { "luminosity",	"luminosity",		DAM_LIGHT	},
    { "radiance",	"radiance",		DAM_LIGHT	},
    { "scintillation",	"scintillation",	DAM_LIGHT	},
    { "shimmering",	"shimmering",		DAM_LIGHT	},
    { "stunflash",	"stun flash",		DAM_LIGHT	},
    { "sunlight",	"sunlight",		DAM_LIGHT	},

    { "bolt",		"lightning bolt",	DAM_LIGHTNING	},
    { "current",	"current",		DAM_LIGHTNING	},
    { "electricity",	"electricity",		DAM_LIGHTNING	},
    { "electrocution",	"electrocution",	DAM_LIGHTNING	},
    { "electron",	"electron",		DAM_LIGHTNING	},
    { "lightning",	"lightning",		DAM_LIGHTNING	},
    { "magnetism",	"magnetism",		DAM_LIGHTNING	},
    { "neutron",	"neutron",		DAM_LIGHTNING	},
    { "proton",		"proton",		DAM_LIGHTNING	},
    { "shbite",		"shocking bite",	DAM_LIGHTNING	},
    { "shock",		"shock",		DAM_LIGHTNING	},
    { "spark",		"spark",		DAM_LIGHTNING	},
    { "thunderbolt",	"thunderbolt",		DAM_LIGHTNING	},
    { "voltage",	"voltage",		DAM_LIGHTNING	},
    { "zap",		"zap",			DAM_LIGHTNING	},

    { "abracadabra",	"abracadabra",		DAM_MAGIC	},
    { "arcane",		"arcane power",		DAM_MAGIC	},
    { "enchantment",	"enchantment",		DAM_MAGIC	},
    { "magic",		"magic",		DAM_MAGIC	},
    { "mumbo-jumbo",	"mumbo-jumbo",		DAM_MAGIC	},
    { "sorcery",	"sorcery",		DAM_MAGIC	},
    { "voodoo",		"voodoo",		DAM_MAGIC	},
    { "wizardry",	"wizardry",		DAM_MAGIC	},

    { "bewilderment",	"bewilderment",		DAM_MENTAL	},
    { "confusion",	"confusion",		DAM_MENTAL	},
    { "electrokinesis",	"electrokinesis",	DAM_MENTAL	},
    { "hypnotism",	"hypnotism",		DAM_MENTAL	},
    { "insanity",	"insanity",		DAM_MENTAL	},
    { "intelligence",	"intelligence",		DAM_MENTAL	},
    { "misunderstand",	"misunderstanding",	DAM_MENTAL	},
    { "neurosis",	"neurosis",		DAM_MENTAL	},
    { "psychosis",	"psychosis",		DAM_MENTAL	},
    { "pyrokinesis",	"pyrokinesis",		DAM_MENTAL	},
    { "swedish",	"swedish reasoning",	DAM_MENTAL	},
    { "telekinesis",	"telekinesis",		DAM_MENTAL	},
    { "typo",		"typo",			DAM_MENTAL	},

    { "anti-matter",	"anti-matter",		DAM_NEGATIVE	},
    { "cruelty",	"cruelty",		DAM_NEGATIVE	},
    { "demonic",	"demonic wrath",	DAM_NEGATIVE	},
    { "diabolism",	"diabolism",		DAM_NEGATIVE	},
    { "drain",		"life drain",		DAM_NEGATIVE	},
    { "eforce",		"evil force",		DAM_NEGATIVE	},
    { "hellfire",	"hellfire",		DAM_NEGATIVE	},
    { "unholy",		"unholy power",		DAM_NEGATIVE	},
    { "vampirism",	"vampirism",		DAM_NEGATIVE	},

    { "bite",		"bite",			DAM_PIERCE	},
    { "cherry",		"popping",       	DAM_PIERCE	},
    { "chomp",		"chomp",		DAM_PIERCE	},
    { "drill",		"drill",		DAM_PIERCE	},
    { "nibble",		"nibble",		DAM_PIERCE	},
    { "peck",		"peck",			DAM_PIERCE	},
    { "penetration",	"penetration",		DAM_PIERCE	},
    { "pierce",		"pierce",		DAM_PIERCE	},
    { "prick",		"prick",		DAM_PIERCE	},
    { "rend",		"rend",			DAM_PIERCE	},
    { "scratch",	"scratch",		DAM_PIERCE	},
    { "spike",		"spike",		DAM_PIERCE	},
    { "stab",		"stab",			DAM_PIERCE	},
    { "sting",		"sting",		DAM_PIERCE	},
    { "thrust",		"thrust",		DAM_PIERCE	},
    { "tooth",		"tooth",		DAM_PIERCE	},

    { "bacteria",	"bacteria",		DAM_POISON	},
    { "fart",		"fart",			DAM_POISON	},
    { "gas",		"gas",			DAM_POISON	},
    { "injection",	"lethal injection",	DAM_POISON	},
    { "mgas",		"marsh gas",		DAM_POISON	},
    { "noxious",	"noxious cloud",	DAM_POISON	},
    { "odor",		"odor",			DAM_POISON	},
    { "poison",		"poison",		DAM_POISON	},
    { "septic",		"septic touch",		DAM_POISON	},
    { "toxin",		"toxin",		DAM_POISON	},
    { "vbite",		"venomous bite",	DAM_POISON	},
    { "venom",		"venom",		DAM_POISON	},

    { "darkness",	"darkness",		DAM_SHADOW	},
    { "dusk",		"dusk",			DAM_SHADOW	},
    { "gloom",		"gloom",		DAM_SHADOW	},
    { "haze",		"haze",			DAM_SHADOW	},
    { "midnight",	"midnight",		DAM_SHADOW	},
    { "obfuscation",	"obfuscation",		DAM_SHADOW	},
    { "obscurity",	"obscurity",		DAM_SHADOW	},
    { "shade",		"shade",		DAM_SHADOW	},
    { "shadow",		"shadow",		DAM_SHADOW	},
    { "umbra",		"umbra",		DAM_SHADOW	},

    { "argent",		"argent",		DAM_SILVER	},
    { "bullet",		"silver bullet",	DAM_SILVER	},
    { "crescent",	"crescent moon",	DAM_SILVER	},
    { "mithril",	"mithril",		DAM_SILVER	},
    { "moonlight",	"moonlight",		DAM_SILVER	},
    { "nugget",		"silver nugget",	DAM_SILVER	},
    { "second",		"second place",		DAM_SILVER	},
    { "silver",		"silver",		DAM_SILVER	},
    { "silvery",	"silvery touch",	DAM_SILVER	},
    { "sterling",	"sterling touch",	DAM_SILVER	},

    { "carve",		"carve",		DAM_SLASH	},
    { "chop",		"chop",			DAM_SLASH	},
    { "claw",		"claw",			DAM_SLASH	},
    { "cleave",		"cleave",		DAM_SLASH	},
    { "cut",		"cut",			DAM_SLASH	},
    { "gash",		"gash",			DAM_SLASH	},
    { "grep",		"grep",			DAM_SLASH	},
    { "hack",		"hack",			DAM_SLASH	},
    { "incision",	"incision",		DAM_SLASH	},
    { "laceration",	"laceration",		DAM_SLASH	},
    { "slash",		"slash",		DAM_SLASH	},
    { "slice",		"slice", 		DAM_SLASH	},	
    { "slit",		"slit",			DAM_SLASH	},
    { "snip",		"snip",			DAM_SLASH	},
    { "whip",		"whip",			DAM_SLASH	},

    { "cry",		"cry",			DAM_SOUND	},
    { "dischord",	"dischord",		DAM_SOUND	},
    { "dissonance",	"dissonance",		DAM_SOUND	},
    { "hollar",		"hollar",		DAM_SOUND	},
    { "howl",		"howl",			DAM_SOUND	},
    { "resonance",	"resonance",		DAM_SOUND	},
    { "sboom",		"sonic boom",		DAM_SOUND	},
    { "scream",		"scream",		DAM_SOUND	},
    { "screech",	"screech",		DAM_SOUND	},
    { "shockwave",	"shockwave",		DAM_SOUND	},
    { "shout",		"shout",		DAM_SOUND	},
    { "shriek",		"shriek",		DAM_SOUND	},
    { "shrill",		"shrill cry",		DAM_SOUND	},
    { "sound",		"sound",		DAM_SOUND	},
    { "squeak",		"squeak",		DAM_SOUND	},
    { "squeal",		"squeal",		DAM_SOUND	},
    { "thunderclap",	"thunderclap",		DAM_SOUND	},
    { "voice",		"voice",	        DAM_SOUND	},
    { "yell",		"yell",			DAM_SOUND	},
    { "yelp",		"yelp",			DAM_SOUND	},
    { "yip",		"yip",			DAM_SOUND	},
    { "yowl",		"yowl",			DAM_SOUND	},

    { "deluge",		"deluge",		DAM_WATER	},
    { "drip",		"drip",			DAM_WATER	},
    { "drowning",	"drowning",		DAM_WATER	},
    { "flflood",	"flash flood",		DAM_WATER	},
    { "flood",		"flood",		DAM_WATER	},
    { "rain",		"rain",			DAM_WATER	},
    { "rainstorm",	"rainstorm",		DAM_WATER	},
    { "river",		"river",		DAM_WATER	},
    { "tide",		"tide",			DAM_WATER	},
    { "torrent",	"torrent",		DAM_WATER	},
    { "torrential",	"torrential",    	DAM_WATER	},
    { "water",		"water",		DAM_WATER	},
    { "waterfall",	"waterfall",		DAM_WATER	},
    { "waterwall",	"waterwall",		DAM_WATER	},

    { "branch",		"tree branch",		DAM_WOOD	},
    { "lumber",		"lumber",		DAM_WOOD	},
    { "sawdust",	"sawdust",		DAM_WOOD	},
    { "shrubbery",	"shrubbery",		DAM_WOOD	},
    { "sliver",		"sliver",		DAM_WOOD	},
    { "splinter",	"splinter",		DAM_WOOD	},
    { "splintering",	"splintering",		DAM_WOOD	},
    { "stick",		"pointy stick",		DAM_WOOD	},
    { "timber",		"timber",		DAM_WOOD	},
    { "wood",		"wood",			DAM_WOOD	}, /* 326 */

    { NULL,		NULL,			0		}
};

const struct str_app_type str_app[36] =
{
    {  0,  0,   0,  0 },  /* 0  */
    {  1,  1,   3,  1 },  /* 1  */
    {  2,  2,   3,  2 },
    {  3,  3,  10,  3 },  /* 3  */
    {  4,  4,  25,  4 },
    {  5,  5,  55,  5 },  /* 5  */
    {  6,  6,  80,  6 },
    {  7,  7,  90,  7 },
    {  8,  8, 100,  8 },
    {  9,  9, 100,  9 },
    { 10, 10, 115, 10 }, /* 10  */
    { 11, 11, 115, 11 },
    { 12, 12, 130, 12 },
    { 13, 13, 130, 13 }, /* 13  */
    { 14, 14, 140, 14 },
    { 15, 15, 150, 15 }, /* 15  */
    { 16, 16, 165, 16 },
    { 17, 17, 180, 22 },
    { 18, 18, 200, 24 }, /* 18  */
    { 19, 19, 225, 26 },
    { 20, 20, 235, 29 }, /* 20  */
    { 21, 21, 255, 31 },
    { 22, 22, 275, 33 },
    { 23, 23, 290, 35 },
    { 24, 24, 310, 38 },
    { 25, 25, 330, 40 }, /* 25 */
    { 26, 26, 355, 42 },
    { 27, 27, 370, 44 },
    { 28, 28, 390, 45 },
    { 29, 29, 410, 47 },
    { 30, 30, 430, 49 }, /* 30 */
    { 31, 31, 450, 50 },
    { 32, 32, 460, 52 },
    { 33, 33, 470, 53 },
    { 34, 34, 480, 56 },
    { 35, 35, 500, 60 }  /* 35   */
};

const struct int_app_type int_app[36] =
{
    {  3 },	/*  0 */
    {  5 },	/*  1 */
    {  7 },
    {  8 },	/*  3 */
    {  9 },
    { 10 },	/*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },	/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },	/* 15 */
    { 34 },
    { 37 },
    { 40 },	/* 18 */
    { 43 },
    { 47 },	/* 20 */
    { 52 },
    { 54 },
    { 56 },
    { 58 },
    { 60 },	/* 25 */
    { 62 },
    { 64 },
    { 66 },
    { 69 },
    { 72 },	/* 30 */
    { 75 },
    { 77 },
    { 79 },
    { 81 },
    { 85 }	/* 35 */
};

const struct wis_app_type wis_app[36] =
{
    { 0 },	/*  0 */
    { 0 },	/*  1 */
    { 0 },
    { 0 },	/*  3 */
    { 0 },
    { 1 },	/*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 2 },
    { 2 },	/* 10 */
    { 2 },
    { 2 },
    { 3 },
    { 3 },
    { 3 },	/* 15 */
    { 3 },
    { 3 },
    { 4 },	/* 18 */
    { 4 },
    { 4 },	/* 20 */
    { 4 },
    { 5 },
    { 5 },
    { 5 },
    { 5 },	/* 25 */
    { 5 },
    { 6 },
    { 6 },
    { 6 },
    { 6 },	/* 30 */
    { 7 },
    { 7 },
    { 7 },
    { 8 },
    { 9 }	/* 35 */
};

const struct dex_app_type dex_app[36] =
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 62 },
    { - 65 },
    { - 68 },
    { - 70 },    /* 25 */
    { - 72 },
    { - 74 },
    { - 79 },
    { - 81 },
    { - 84 },	/* 30 */
    { - 88 },
    { - 90 },
    { - 95 },
    { -100 },
    { -105 }	/* 35 */
};

const struct con_app_type con_app[36] =
{
    { -4, 20 },   /*  0 */
    { -3, 25 },   /*  1 */
    { -2, 30 },
    { -2, 35 },	  /*  3 */
    { -1, 40 },
    { -1, 45 },   /*  5 */
    {  1, 50 },
    {  1, 55 },
    {  2, 60 },
    {  2, 65 },
    {  3, 70 },   /* 10 */
    {  3, 75 },
    {  4, 76 },
    {  4, 77 },
    {  5, 78 },
    {  5, 79 },   /* 15 */
    {  6, 80 },
    {  6, 81 },
    {  7, 82 },   /* 18 */
    {  7, 83 },
    {  8, 84 },   /* 20 */
    {  8, 85 },
    {  9, 86 },
    {  9, 87 },
    { 10, 88 },
    { 10, 89 },    /* 25 */
    { 11, 90 },
    { 11, 91 },
    { 12, 92 },
    { 12, 93 },
    { 13, 94 },   /* 30 */
    { 13, 95 },
    { 14, 96 },
    { 14, 97 },
    { 15, 98 },
    { 16, 99 }   /* 35 */
};

const struct liq_type liq_table[] =
{
/*    name			color	proof, full, thirst, food, ssize */
    { "water",			"clear",	{   0, 1, 10, 0, 16 }	},
    { "beer",			"amber",	{  12, 1,  8, 1, 12 }	},
    { "red wine",		"burgundy",	{  30, 1,  8, 1,  5 }	},
    { "ale",			"brown",	{  15, 1,  8, 1, 12 }	},
    { "dark ale",		"dark",		{  16, 1,  8, 1, 12 }	},

    { "whisky",			"golden",	{ 120, 1,  5, 0,  2 }	},
    { "lemonade",		"pink",		{   0, 1,  9, 2, 12 }	},
    { "firebreather",		"boiling",	{ 190, 0,  4, 0,  2 }	},
    { "local specialty",	"clear",	{ 151, 1,  3, 0,  2 }	},
    { "slime mold juice",	"green",	{   0, 2, -8, 1,  2 }	},

    { "milk",			"white",	{   0, 2,  9, 3, 12 }	},
    { "tea",			"tan",		{   0, 1,  8, 0,  6 }	},
    { "coffee",			"black",	{   0, 1,  8, 0,  6 }	},
    { "blood",			"red",		{   0, 2, -1, 2,  6 }	},
    { "salt water",		"clear",	{   0, 1, -2, 0,  1 }	},

    { "coke",			"brown",	{   0, 2,  9, 2, 12 }	}, 
    { "root beer",		"brown",	{   0, 2,  9, 2, 12 }   },
    { "elvish wine",		"green",	{  35, 2,  8, 1,  5 }   },
    { "white wine",		"golden",	{  28, 1,  8, 1,  5 }   },
    { "champagne",		"golden",	{  32, 1,  8, 1,  5 }   },

    { "mead",			"honey-colored",{  34, 2,  8, 2, 12 }   },
    { "rose wine",		"pink",		{  26, 1,  8, 1,  5 }	},
    { "benedictine wine",	"burgundy",	{  40, 1,  8, 1,  5 }   },
    { "vodka",			"clear",	{ 130, 1,  5, 0,  2 }   },
    { "cranberry juice",	"red",		{   0, 1,  9, 2, 12 }	},

    { "orange juice",		"orange",	{   0, 2,  9, 3, 12 }   }, 
    { "absinthe",		"green",	{ 200, 1,  4, 0,  2 }	},
    { "brandy",			"golden",	{  80, 1,  5, 0,  4 }	},
    { "aquavit",		"clear",	{ 140, 1,  5, 0,  2 }	},
    { "schnapps",		"clear",	{  90, 1,  5, 0,  2 }   },

    { "icewine",		"purple",	{  50, 2,  6, 1,  5 }	},
    { "amontillado",		"burgundy",	{  35, 2,  8, 1,  5 }	},
    { "sherry",			"red",		{  38, 2,  7, 1,  5 }   },	
    { "framboise",		"red",		{  50, 1,  7, 1,  5 }   },
    { "rum",			"amber",	{ 151, 1,  4, 0,  2 }	},

    { "cordial",		"clear",	{ 100, 1,  5, 0,  2 }   },
    { "dr pepper",		"brown",	{   0, 2,  9, 2, 12 }	},
    { NULL,			NULL,		{   0, 0,  0, 0,  0 }	}
};

