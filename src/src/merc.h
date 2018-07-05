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

#include <zlib.h>

#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#define DECLARE_OBJ_FUN( fun )		OBJ_FUN	  fun
#define DECLARE_ROOM_FUN( fun )		ROOM_FUN  fun

/* system calls */
int unlink();
int system();

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int				sh_int;
typedef int				bool;
#define unix
#else
typedef short   int			sh_int;
typedef unsigned char			bool;
#endif

#define CLEAR_SCREEN	"\x01B[2J"
#define CLEAR           "[0m"		/* Resets Colour        */
#define BLINK           "[5m"		/* Blink                */
#define C_BLACK		"[0;30m"	/* Normal Colours       */
#define C_RED           "[0;31m"
#define C_GREEN         "[0;32m"
#define C_YELLOW        "[0;33m"
#define C_BLUE          "[0;34m"
#define C_MAGENTA       "[0;35m"
#define C_CYAN          "[0;36m"
#define C_WHITE         "[0;37m"
#define C_D_GREY        "[1;30m"	/* Light Colors         */
#define C_B_RED         "[1;31m"
#define C_B_GREEN       "[1;32m"
#define C_B_YELLOW      "[1;33m"
#define C_B_BLUE        "[1;34m"
#define C_B_MAGENTA     "[1;35m"
#define C_B_CYAN        "[1;36m"
#define C_B_WHITE       "[1;37m"
#define R_BLACK		"[0;30m"	/* Reset Colours       */
#define R_RED           "[0;31m"
#define R_GREEN         "[0;32m"
#define R_YELLOW        "[0;33m"
#define R_BLUE          "[0;34m"
#define R_MAGENTA       "[0;35m"
#define R_CYAN          "[0;36m"
#define R_WHITE         "[0;37m"
#define R_D_GREY        "[1;30m"	/* Reset Light         */
#define R_B_RED         "[1;31m"
#define R_B_GREEN       "[1;32m"
#define R_B_YELLOW      "[1;33m"
#define R_B_BLUE        "[1;34m"
#define R_B_MAGENTA     "[1;35m"
#define R_B_CYAN        "[1;36m"
#define R_B_WHITE       "[1;37m"

/*
 * Structure types.
 */
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct	wiz_data		WIZ_DATA;
typedef struct 	buf_type	 	BUFFER;
typedef struct	char_data		CHAR_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef	struct	multiplay_data		MULTIPLAY_DATA;
typedef	struct	desc_check_data		DESC_CHECK_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	mem_data		MEM_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef	struct	pkill_data		PKILL_DATA;
typedef	struct	pkill_record		PKILL_RECORD;
typedef struct  gen_data		GEN_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef	struct	room_damage_data	ROOM_DAMAGE_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	weather_data		WEATHER_DATA;
typedef struct  prog_list		PROG_LIST;
typedef struct  prog_code		PROG_CODE;
typedef struct  auction_data		AUCTION_DATA;
typedef struct  roster_data		ROSTER_DATA;
typedef struct  disabled_data		DISABLED_DATA;
typedef	struct	grant_data		GRANT_DATA;
typedef	struct	censor_data		CENSOR_DATA;
typedef	struct	channel_data		CHANNEL_DATA;
typedef	struct	arena_type		ARENA_DATA;
typedef struct	obj_multi_data		OBJ_MULTI_DATA;
typedef struct	poll_data		POLL_DATA;
typedef	struct	vote_data		VOTE_DATA;
typedef	struct	stat_data		STAT_DATA;
typedef struct	clan_cost_type		COST_DATA;

/*
 * Function types.
 */
typedef	void DO_FUN	args( ( CHAR_DATA *ch, char *argument ) );
typedef void OBJ_FUN	args( ( OBJ_DATA *obj, char *argument ) );
typedef void ROOM_FUN	args( ( ROOM_INDEX_DATA *room, char *argument ) );
typedef bool SPEC_FUN	args( ( CHAR_DATA *ch ) );
typedef bool SPELL_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo,
				int target ) );

/*
 * String and memory management parameters.
 */
#define	COMPRESS_BUF_SIZE	65536
#define	MAX_KEY_HASH		 1024
#define MAX_STRING_LENGTH	 9816 /* 4608 */
#define MAX_INPUT_LENGTH	  512 /* 256 */
#define PAGELEN			   25

#define ED_AREA		 1
#define ED_ROOM		 2
#define ED_OBJECT	 3
#define ED_MOBILE	 4
#define ED_MPCODE	 5
#define ED_HELP		 6
#define ED_SKILL	 7
#define ED_GROUP	 8
#define ED_CLASS	 9
#define ED_CLAN		10
#define ED_RACE		11
#define ED_SOCIAL	12
#define ED_ROOM_DAM	13
#define ED_SHOP		14
#define ED_OPCODE       15
#define ED_RPCODE       16
#define ED_CHANNEL	17
#define ED_GAME_STAT	18
#define ED_PREFIX	19
#define ED_SUFFIX	20
#define ED_COMMAND	21

#define MAX_GQUEST_MOB		   26
#define MAX_BANK		   10
#define MAX_IN_GROUP		   20
#define MAX_TRACK		   20
#define MAX_LIQUID		   36
#define MAX_DAMAGE_MESSAGE	  327
#define MAX_FORGET		   20
#define MAX_CRNK		    6
#define MAX_LEVEL		  160
#define MAX_ALIAS		   50
#define MAX_ARENA_TEAMS		   10
#define MAX_POLL_RESPONSES	   10
#define MAIN_GAME_PORT		 6000
#define LEVEL_HERO		   (MAX_LEVEL - 9)
#define LEVEL_IMMORTAL		   (MAX_LEVEL - 8)

#define PULSE_PER_SECOND	    4
#define PULSE_VIOLENCE		  (  3 * PULSE_PER_SECOND)
#define PULSE_MOBILE		  (  6 * PULSE_PER_SECOND)
#define PULSE_TICK		  ( 45 * PULSE_PER_SECOND)
#define PULSE_AREA		  (120 * PULSE_PER_SECOND)
#define PULSE_AUCTION             ( 20 * PULSE_PER_SECOND)
#define PULSE_MINUTE              ( 60 * PULSE_PER_SECOND)
#define PULSE_QUEST		  ( 60 * PULSE_PER_SECOND)
#define PULSE_PKTIMER		  (  2 * PULSE_PER_SECOND)
#define PULSE_NEWBIE		  ( 30 * PULSE_PER_SECOND)

#define IMPLEMENTOR		MAX_LEVEL
#define	CREATOR			(MAX_LEVEL - 1)
#define SUPREME			(MAX_LEVEL - 2)
#define DEITY			(MAX_LEVEL - 3)
#define GOD			(MAX_LEVEL - 4)
#define IMMORTAL		(MAX_LEVEL - 5)
#define DEMI			(MAX_LEVEL - 6)
#define KNIGHT			(MAX_LEVEL - 7)
#define SQUIRE			(MAX_LEVEL - 8)
#define HERO			LEVEL_HERO

#define TRAP_DAMAGE		0
#define TRAP_MANA		1
#define TRAP_TELEPORT		2
#define TRAP_LATCH		3
#define TRAP_MAX		4

#define DAM_ALL			-1
#define DAM_ACID                0
#define DAM_BASH                1
#define DAM_CHARM		2
#define DAM_COLD                3
#define DAM_DISEASE             4
#define DAM_ENERGY              5
#define DAM_FIRE                6
#define DAM_FEAR		7
#define DAM_HOLY                8
#define DAM_IRON		9
#define DAM_LIGHT		10
#define DAM_LIGHTNING           11
#define DAM_MAGIC		12
#define DAM_MENTAL              13
#define DAM_NEGATIVE            14
#define DAM_AIR                 15
#define DAM_OTHER               16
#define DAM_PIERCE              17
#define DAM_POISON              18
#define DAM_SILVER		19
#define DAM_SLASH               20
#define DAM_SOUND		21
#define DAM_WATER		22
#define DAM_EARTH		23
#define DAM_SHADOW		24
#define DAM_WOOD		25
#define DAM_MAX			26

#define VALUE_HIT_POINT		0
#define VALUE_MANA_POINT	1
#define VALUE_MOVE_POINT	2

#define VALUE_SILVER		0
#define VALUE_GOLD		1
#define VALUE_PLATINUM		2
#define VALUE_QUEST_POINT	3
#define VALUE_IQUEST_POINT	4
#define MAX_AUCTION_PAYMENT	5

#define BAN_SUFFIX		A
#define BAN_PREFIX		B
#define BAN_NEWBIES		C
#define BAN_ALL			D
#define BAN_PERMIT		E
#define BAN_PERMANENT		F

#define ALLOW_SUFFIX		A
#define ALLOW_PREFIX		B
#define ALLOW_ITEMS		C
#define ALLOW_CONNECTS		D

#define SUN_DARK		    0
#define SUN_RISE		    1
#define SUN_LIGHT		    2
#define SUN_SET			    3

#define SKY_CLOUDLESS		    0
#define SKY_CLOUDY		    1
#define SKY_RAINING		    2
#define SKY_LIGHTNING		    3

#define CON_PLAYING			 0
#define CON_GET_NAME			 1
#define CON_GET_OLD_PASSWORD		 2
#define CON_CONFIRM_NEW_NAME		 3
#define CON_GET_NEW_PASSWORD		 4
#define CON_CONFIRM_NEW_PASSWORD	 5
#define CON_GET_NEW_RACE		 6
#define CON_GET_NEW_SEX			 7
#define CON_GET_NEW_CLASS		 8
#define CON_GET_ALIGNMENT		 9
#define CON_DEFAULT_CHOICE		10
#define CON_GEN_GROUPS			11
#define CON_PICK_WEAPON			12
#define CON_READ_IMOTD			13
#define CON_READ_MOTD			14
#define CON_BREAK_CONNECT		15
#define CON_COPYOVER_RECOVER		16
#define CON_REROLLING			17

#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3
#define TO_ALL		    4

#define MAX_TRADE	 5

#define STAT_STR 	0
#define STAT_INT	1
#define STAT_WIS	2
#define STAT_DEX	3
#define STAT_CON	4
#define MAX_STATS 	5

#define DUR_TICKS	0
#define DUR_ROUNDS	1

#define TO_AFFECTS	0
#define TO_SHIELDS	1
#define TO_OBJECT	2
#define TO_WEAPON	3
#define TO_DAM_MODS	4
#define TO_ACT		5

#define DEVOTE_BODY	0
#define DEVOTE_MIND	1
#define DEVOTE_SPIRIT	2
#define DEVOTE_GRACE	3
#define DEVOTE_FORCE	4
#define DEVOTE_LIFE	5
#define DEVOTE_SKILLS	6
#define DEVOTE_SPELLS	7
#define DEVOTE_EQ	8
#define DEVOTE_CURRENT	9

#define FORGE_SCALE_PERCENT	5
#define DEVOTE_BASE_EXP		30000
#define DEVOTE_TNL_PERCENT	15

#define A		  	1
#define B			2
#define C			4
#define D			8
#define E			16
#define F			32
#define G			64
#define H			128

#define I			256
#define J			512
#define K		        1024
#define L		 	2048
#define M			4096
#define N		 	8192
#define O			16384
#define P			32768

#define Q			65536
#define R			131072
#define S			262144
#define T			524288
#define U			1048576
#define V			2097152
#define W			4194304
#define X			8388608
#define Y			16777216
#define Z			33554432
#define aa			67108864 	 /* doubled due to conflicts */
#define bb			134217728
#define cc			268435456
#define dd			536870912
#define ee 		        1073741824

#define ACT_NOQUEST		(A)
#define ACT_SENTINEL	    	(B)		/* Stays in one room	*/
#define ACT_SCAVENGER	      	(C)		/* Picks up objects	*/
#define	ACT_SMART_MOB		(D)
#define ACT_AREA_ATTACK		(E)
#define ACT_AGGRESSIVE		(F)    		/* Attacks PC's		*/
#define ACT_STAY_AREA		(G)		/* Won't leave area	*/
#define ACT_WIMPY		(H)
#define ACT_PET			(I)		/* Auto set for pets	*/
#define ACT_TRAIN		(J)		/* Can train PC's	*/
#define ACT_PRACTICE		(K)		/* Can practice PC's	*/
#define ACT_SEE_ALL		(L)
#define ACT_NO_BODY		(M)		/* Will not leave a corpse */
#define ACT_NB_DROP		(N)		/* Corpseless will drop all */
#define ACT_NO_RETURN_HOME	(O)
#define ACT_ASSIST_ALL       	(P)
#define ACT_ASSIST_ALIGN        (Q)
#define ACT_ASSIST_RACE		(R)
#define ACT_ASSIST_PLAYERS      (S)
#define ACT_ASSIST_GUARD       	(T)
#define ACT_ASSIST_VNUM		(U)
#define ACT_NOPURGE		(V)
#define ACT_NO_WHERE		(W)
#define ACT_IS_SATAN		(X)
#define ACT_IS_PRIEST		(Z)
#define ACT_IS_HEALER		(aa)
#define ACT_GAIN		(bb)

#define PLR_DETAIL_EXIT		(A)
#define PLR_COLOUR		(B)
#define PLR_AUTOASSIST		(C)
#define PLR_AUTOEXIT		(D)
#define PLR_AUTOLOOT		(E)
#define PLR_AUTOSAC             (F)
#define PLR_AUTOGOLD		(G)
#define PLR_AUTOSPLIT		(H)
#define PLR_NOTRAN		(I)
#define PLR_AUTOPEEK		(J)
#define PLR_CUSTOM_WHO		(K)
#define PLR_GQUEST		(L)
#define PLR_HOLYLIGHT		(N)
#define PLR_CENSORED		(P)
#define PLR_NOSUMMON		(Q)
#define PLR_NOFOLLOW		(R)
#define PLR_QUESTOR             (S)
#define PLR_NOCANCEL		(T)
#define PLR_CLOAKED_EQ		(U)
#define PLR_NOEXP		(V)
#define PLR_LOG			(W)
#define PLR_TWIT		(Z)
#define PLR_REROLL		(dd)

#define PART_HEAD               (A)
#define PART_ARMS               (B)
#define PART_LEGS               (C)
#define PART_HEART              (D)
#define PART_BRAINS             (E)
#define PART_GUTS               (F)
#define PART_HANDS              (G)
#define PART_FEET               (H)
#define PART_FINGERS            (I)
#define PART_EAR                (J)
#define PART_EYE		(K)
#define PART_LONG_TONGUE        (L)
#define PART_EYESTALKS          (M)
#define PART_TENTACLES          (N)
#define PART_FINS               (O)
#define PART_WINGS              (P)
#define PART_TAIL               (Q)
#define PART_CLAWS              (U)
#define PART_FANGS              (V)
#define PART_HORNS              (W)
#define PART_SCALES             (X)
#define PART_TUSKS		(Y)

#define AFF_BLIND		(A)
#define AFF_DETECT_TERRAIN	(B)
#define AFF_DETECT_EVIL		(C)
#define AFF_DETECT_INVIS	(D)
#define AFF_DETECT_MAGIC	(E)
#define AFF_DETECT_HIDDEN	(F)
#define AFF_DETECT_GOOD		(G)
#define AFF_DETECT_NEUTRAL	(H)
#define AFF_FAERIE_FIRE		(I)
#define AFF_INFRARED		(J)
#define AFF_CURSE		(K)
#define AFF_FARSIGHT		(L)
#define AFF_POISON		(M)
#define AFF_SNEAK		(P)
#define AFF_HIDE		(Q)
#define AFF_SLEEP		(R)
#define AFF_CHARM		(S)
#define AFF_FLYING		(T)
#define AFF_PASS_DOOR		(U)
#define AFF_HASTE		(V)
#define AFF_CALM		(W)
#define AFF_PLAGUE		(X)
#define AFF_WEAKEN		(Y)
#define AFF_DARK_VISION		(Z)
#define AFF_REGENERATION        (cc)
#define AFF_SLOW		(dd)
#define AFF_GIANT_STRENGTH	(ee)

#define SHD_PROTECT_VOODOO	(A)
#define SHD_INVISIBLE		(B)
#define SHD_ICE			(C)
#define SHD_FIRE		(D)
#define SHD_SHOCK		(E)
#define	SHD_ROCK		(F)
#define SHD_SHRAPNEL		(G)
#define SHD_SANCTUARY		(H)
#define SHD_THORN		(I)
#define SHD_VAMPIRIC		(J)
#define SHD_DIVINE_AURA		(K)
#define SHD_ACID		(L)
#define SHD_PROTECT_NEUTRAL	(M)
#define SHD_PROTECT_EVIL	(N)
#define SHD_PROTECT_GOOD	(O)
#define SHD_MANA		(P)
#define SHD_DIVINITY		(Q)
#define SHD_WATER		(R)
#define SHD_ABSORB		(S)

#define SOUND_ON		(A)
#define SOUND_NOMUSIC		(B)
#define SOUND_NOCOMBAT		(C)
#define SOUND_NOWEATHER		(D)
#define SOUND_NOSKILL		(E)
#define SOUND_NOCLAN		(F)
#define SOUND_NOZONE		(G)
#define SOUND_NOMISC		(H)

#define SEX_NEUTRAL		0
#define SEX_MALE		1
#define SEX_FEMALE		2
#define SEX_RANDOM		3

#define AC_PIERCE		0
#define AC_BASH			1
#define AC_SLASH		2
#define AC_EXOTIC		3

#define DICE_NUMBER			0
#define DICE_TYPE			1
#define DICE_BONUS			2

#define SIZE_TINY			0
#define SIZE_SMALL			1
#define SIZE_MEDIUM			2
#define SIZE_LARGE			3
#define SIZE_HUGE			4
#define SIZE_GIANT			5
#define SIZE_NONE			6

#define MOB_VNUM_DEMON		1
#define MOB_VNUM_CORPSE		2
#define MOB_VNUM_ANIMATE	3
#define MOB_VNUM_ENGINEER	4

#define OBJ_VNUM_SILVER_ONE	      1
#define OBJ_VNUM_GOLD_ONE	      2
#define OBJ_VNUM_GOLD_SOME	      3
#define OBJ_VNUM_SILVER_SOME	      4
#define OBJ_VNUM_COINS		      5
#define OBJ_VNUM_PLATINUM_ONE	      6
#define OBJ_VNUM_PLATINUM_SOME	      7
#define OBJ_VNUM_CORPSE_PC	      9
#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_EAR		     11
#define OBJ_VNUM_SEVERED_HEAD	     12
#define OBJ_VNUM_TORN_HEART	     13
#define OBJ_VNUM_SLICED_ARM	     14
#define OBJ_VNUM_SLICED_LEG	     15
#define OBJ_VNUM_GUTS		     16
#define OBJ_VNUM_BRAINS		     17
#define OBJ_VNUM_FOOT		     18
#define OBJ_VNUM_WING		     19
#define OBJ_VNUM_HAND		     20
#define OBJ_VNUM_FINGER		     21
#define OBJ_VNUM_EYE		     22
#define OBJ_VNUM_DISC		     23
#define OBJ_VNUM_BLOOD		     24
#define OBJ_VNUM_PORTAL		     25
#define OBJ_VNUM_FOG		     41
#define OBJ_VNUM_LIGHT_BALL	     42
#define OBJ_VNUM_VOODOO		     44
#define OBJ_VNUM_BAG		     47
#define OBJ_VNUM_GENERIC	     48
#define OBJ_VNUM_ENGINEER_GRENADE    55
#define OBJ_VNUM_SOULBLADE	     57
#define OBJ_VNUM_PIT		   3010

#define OBJ_SCHOOL_MACE	   	3700
#define OBJ_SCHOOL_DAGGER	3701
#define OBJ_SCHOOL_SWORD	3702
#define OBJ_SCHOOL_SPEAR	3717
#define OBJ_SCHOOL_AXE	   	3719
#define OBJ_SCHOOL_FLAIL	3720
#define OBJ_SCHOOL_WHIP	   	3721
#define OBJ_SCHOOL_POLEARM	3722
#define	OBJ_QUARTERSTAFF	3729

#define OBJ_VNUM_SCHOOL_VEST	   3703
#define OBJ_VNUM_SCHOOL_SHIELD	   3704
#define OBJ_VNUM_SCHOOL_BANNER     3716
#define OBJ_VNUM_MAP		   3162
#define OBJ_VNUM_WMAP		   3163
#define OBJ_VNUM_EMAP		   3169

#define OBJ_VNUM_QUEST_SIGN	      8
#define	OBJ_VNUM_POTION		     58
#define	OBJ_VNUM_SCROLL		     46

#define	OBJ_VNUM_CUBIC		   3386
#define	OBJ_VNUM_DPOUCH		   3383
#define	OBJ_VNUM_CPOUCH		   3387

#define FORGE_STONE_ADAMANTIUM	35000
#define FORGE_STONE_MITHRIL	35001
#define FORGE_STONE_TITANIUM	35002
#define FORGE_STONE_STEEL	35003
#define FORGE_STONE_BRONZE	35004

#define ITEM_LIGHT		 1
#define ITEM_SCROLL		 2
#define ITEM_WAND		 3
#define ITEM_STAFF		 4
#define ITEM_WEAPON		 5
#define ITEM_TREASURE		 6
#define ITEM_ARMOR		 7
#define ITEM_POTION		 8
#define ITEM_CLOTHING		 9
#define ITEM_FURNITURE		10
#define ITEM_TRASH		11
#define ITEM_CONTAINER		12
#define ITEM_DRINK_CON		13
#define ITEM_KEY		14
#define ITEM_FOOD		15
#define ITEM_MONEY		16
#define ITEM_BOAT		17
#define ITEM_CORPSE_NPC		18
#define ITEM_CORPSE_PC		19
#define ITEM_FOUNTAIN		20
#define ITEM_PILL		21
#define ITEM_MAP		22
#define ITEM_PORTAL		23
#define ITEM_WARP_STONE		24
#define ITEM_GEM		26
#define ITEM_JEWELRY		27
#define ITEM_ENGINEER_TOOL	28
#define ITEM_DEMON_STONE	29
#define ITEM_EXIT		30
#define ITEM_PIT		31
#define ITEM_SLOTS		32
#define ITEM_COMPONENT_BREW	33
#define ITEM_COMPONENT_SCRIBE	34
#define ITEM_FORGE_STONE	35
#define ITEM_GRENADE		36
#define ITEM_TRAP		37
#define ITEM_MINE		38

#define ITEM_GLOW		(A)
#define ITEM_HUM		(B)
#define ITEM_DARK		(C)
#define ITEM_FORGED		(D)
#define ITEM_EVIL		(E)
#define ITEM_INVIS		(F)
#define ITEM_MAGIC		(G)
#define ITEM_NODROP		(H)
#define ITEM_BLESS		(I)
#define ITEM_ANTI_GOOD		(J)
#define ITEM_ANTI_EVIL		(K)
#define ITEM_ANTI_NEUTRAL	(L)
#define ITEM_NOREMOVE		(M)
#define ITEM_INVENTORY		(N)
#define ITEM_NOPURGE		(O)
#define ITEM_ROT_DEATH		(P)
#define ITEM_VIS_DEATH		(Q)
#define ITEM_DISINTEGRATE	(R)
#define ITEM_NONMETAL		(S)
#define ITEM_NOLOCATE		(T)
#define ITEM_MELT_DROP		(U)
#define ITEM_RANDOM_STATS	(V)
#define ITEM_SELL_EXTRACT	(W)
#define ITEM_NO_SAC		(X)
#define ITEM_BURN_PROOF		(Y)
#define ITEM_NOUNCURSE		(Z)
#define ITEM_AQUEST		(bb)
#define ITEM_QUESTPOINT		(cc)
#define ITEM_SPECIAL_SAVE	(dd)
//#define ITEM_HERO		(ee)

#define ITEM_TAKE		(A)
#define ITEM_WEAR_FINGER	(B)
#define ITEM_WEAR_NECK		(C)
#define ITEM_WEAR_BODY		(D)
#define ITEM_WEAR_HEAD		(E)
#define ITEM_WEAR_LEGS		(F)
#define ITEM_WEAR_FEET		(G)
#define ITEM_WEAR_HANDS		(H)
#define ITEM_WEAR_ARMS		(I)
#define ITEM_WEAR_SHIELD	(J)
#define ITEM_WEAR_ABOUT		(K)
#define ITEM_WEAR_WAIST		(L)
#define ITEM_WEAR_WRIST		(M)
#define ITEM_WIELD		(N)
#define ITEM_HOLD		(O)
#define ITEM_WEAR_FLOAT		(Q)
#define ITEM_WEAR_FACE		(R)
#define ITEM_WEAR_EAR		(S)
#define	ITEM_WEAR_BACK		(T)
#define ITEM_WEAR_ANKLE		(U)
#define ITEM_WEAR_CHEST		(V)
#define ITEM_WEAR_SOUL		(W)
#define ITEM_WEAR_EYES		(X)
#define ITEM_WEAR_CLAN		(Y)

#define WEAPON_EXOTIC		0
#define WEAPON_SWORD		1
#define WEAPON_DAGGER		2
#define WEAPON_SPEAR		3
#define WEAPON_MACE		4
#define WEAPON_AXE		5
#define WEAPON_FLAIL		6
#define WEAPON_WHIP		7
#define WEAPON_POLEARM		8
#define WEAPON_QUARTERSTAFF	9
#define MAX_WEAPON              10

#define WEAPON_FLAMING		(A)
#define WEAPON_FROST		(B)
#define WEAPON_VAMPIRIC		(C)
#define WEAPON_SHARP		(D)
#define WEAPON_VORPAL		(E)
#define WEAPON_TWO_HANDS	(F)
#define WEAPON_SHOCKING		(G)
#define WEAPON_POISON		(H)

#define GATE_NORMAL_EXIT	(A)
#define GATE_NOCURSE		(B)
#define GATE_GOWITH		(C)
#define GATE_BUGGY		(D)
#define GATE_RANDOM		(E)

#define STAND_AT		(A)
#define STAND_ON		(B)
#define STAND_IN		(C)
#define SIT_AT			(D)
#define SIT_ON			(E)
#define SIT_IN			(F)
#define REST_AT			(G)
#define REST_ON			(H)
#define REST_IN			(I)
#define SLEEP_AT		(J)
#define SLEEP_ON		(K)
#define SLEEP_IN		(L)
#define PUT_AT			(M)
#define PUT_ON			(N)
#define PUT_IN			(O)
#define PUT_INSIDE		(P)

#define APPLY_NONE		      0
#define APPLY_STR		      1
#define APPLY_DEX		      2
#define APPLY_INT		      3
#define APPLY_WIS		      4
#define APPLY_CON		      5
#define APPLY_SEX		      6
#define APPLY_DAMAGE		      7
#define APPLY_MAGIC_POWER	      8
#define APPLY_REGEN_HP		      9
#define APPLY_REGEN_MANA	     10
#define APPLY_REGEN_MOVE	     11
#define APPLY_MANA		     12
#define APPLY_HIT		     13
#define APPLY_MOVE		     14
#define APPLY_AC		     17
#define APPLY_HITROLL		     18
#define APPLY_DAMROLL		     19
#define APPLY_SAVES		     20
#define APPLY_SIZE		     26

#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8
#define CONT_PUT_ON		     16

#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_CORNER	      3
#define ROOM_VNUM_TEMPLE	   3001
#define ROOM_VNUM_TEMPLEB	   3365
#define ROOM_VNUM_PIT		   3367
#define ROOM_VNUM_ALTAR		   3054
#define ROOM_VNUM_ALTARB	   3366
#define ROOM_VNUM_SCHOOL	   3700

#define ROOM_DARK		(A)
#define ROOM_ARENA		(B)
#define ROOM_NO_MOB		(C)
#define ROOM_INDOORS		(D)
#define ROOM_WAR		(E)
#define ROOM_ICY		(F)
#define ROOM_CLAN_PORTAL	(G)
#define ROOM_NO_GATE		(H)
#define ROOM_PRIVATE		(J)
#define ROOM_SAFE		(K)
#define ROOM_SOLITARY		(L)
#define ROOM_PET_SHOP		(M)
#define ROOM_NO_RECALL		(N)
#define ROOM_IMP_ONLY		(O)
#define ROOM_GODS_ONLY		(P)
#define ROOM_HEROES_ONLY	(Q)
#define ROOM_NEWBIES_ONLY	(R)
#define ROOM_LAW		(S)
#define ROOM_NOWHERE		(T)

#define DIR_NORTH		      0
#define DIR_EAST		      1
#define DIR_SOUTH		      2
#define DIR_WEST		      3
#define DIR_UP			      4
#define DIR_DOWN		      5
#define MAX_DIR			6

#define EX_ISDOOR		(A)
#define EX_CLOSED		(B)
#define EX_LOCKED		(C)
#define EX_HIDDEN		(D)
#define EX_NOBLINK		(E)
#define EX_PICKPROOF		(F)
#define EX_NOPASS		(G)
#define EX_BASHPROOF		(H)
#define EX_NO_SCAN		(I)
#define EX_NOCLOSE		(K)
#define EX_NOLOCK		(L)

#define SECT_INSIDE		      0
#define SECT_CITY		      1
#define SECT_FIELD		      2
#define SECT_FOREST		      3
#define SECT_HILLS		      4
#define SECT_MOUNTAIN		      5
#define SECT_WATER_SWIM		      6
#define SECT_WATER_NOSWIM	      7
#define SECT_UNUSED		      8
#define SECT_AIR		      9
#define SECT_DESERT		     10
#define SECT_MAX		     11

#define WEAR_NONE		     -1
#define WEAR_LIGHT		      0
#define WEAR_FINGER_L		      1
#define WEAR_FINGER_R		      2
#define WEAR_NECK_1		      3
#define WEAR_NECK_2		      4
#define WEAR_BODY		      5
#define WEAR_HEAD		      6
#define WEAR_LEGS		      7
#define WEAR_FEET		      8
#define WEAR_HANDS		      9
#define WEAR_ARMS		     10
#define WEAR_SHIELD		     11
#define WEAR_ABOUT		     12
#define WEAR_WAIST		     13
#define WEAR_WRIST_L		     14
#define WEAR_WRIST_R		     15
#define WEAR_WIELD		     16
#define WEAR_HOLD		     17
#define WEAR_FLOAT		     18
#define WEAR_SECONDARY		     19
#define WEAR_FACE		     20
#define WEAR_EAR_L		     21
#define WEAR_EAR_R		     22
#define WEAR_BACK		     23
#define	WEAR_ANKLE_L		     24
#define WEAR_ANKLE_R		     25
#define WEAR_CHEST		     26
#define WEAR_EYES		     27
#define WEAR_CLAN		     28
#define WEAR_SOUL1		     29
#define WEAR_SOUL2		     30
#define WEAR_SOUL3		     31
#define WEAR_SOUL4		     32
#define WEAR_SOUL5		     33
#define WEAR_SOUL6		     34
#define WEAR_SOUL7		     35
#define WEAR_SOUL8		     36
#define WEAR_SOUL9		     37
#define WEAR_SOUL10		     38
#define WEAR_SOUL11		     39
#define WEAR_SOUL12		     40
#define WEAR_SOUL13		     41
#define WEAR_SOUL14		     42
#define WEAR_SOUL15		     43
#define WEAR_SHEATH_1		     44
#define WEAR_SHEATH_2		     45
#define MAX_WEAR		     46

#define COND_DRUNK		      0
#define COND_FULL		      1
#define COND_THIRST		      2
#define COND_HUNGER		      3

#define POS_DEAD		      0
#define POS_MORTAL		      1
#define POS_INCAP		      2
#define POS_STUNNED		      3
#define POS_SLEEPING		      4
#define POS_RESTING		      5
#define POS_SITTING		      6
#define POS_FIGHTING		      7
#define POS_STANDING		      8

#define DAZE_SKILL		0
#define DAZE_SPELL		1
#define DAZE_FLEE		2
#define MAX_DAZE		3

#define QUEST_OFF		0
#define QUEST_ON		1
#define QUEST_LOCK		2

#define CONFIG_DEAF		(A)
#define CONFIG_COMPACT		(B)
#define CONFIG_BRIEF		(C)
#define CONFIG_NO_INV_COMBINE	(D)
#define CONFIG_LOOT_COMBINE	(E)
#define CONFIG_SHOW_AFFECTS	(F)
#define CONFIG_LONG		(G)
#define CONFIG_STORE		(H)
#define CONFIG_AREA_NAME	(I)
#define CONFIG_GOTO_BYPASS	(J)
#define CONFIG_REDIT_GOTO	(K)
#define CONFIG_CHAR_COMBINE	(L)
#define CONFIG_NO_SPELLUP	(M)

#define COMM_QUIET              (A)
#define COMM_NOAUCTION		(B)
#define COMM_NOWIZ              (C)
#define COMM_NOOOC              (D)
#define COMM_NOGOSSIP           (E)
#define COMM_NOASK              (F)
#define COMM_NORACE		(G)
#define COMM_NOCLAN		(H)
#define COMM_NOQUOTE		(I)
#define COMM_NOFLAME		(J)
#define COMM_NOHERO		(K)
#define COMM_NOARENA		(L)
#define COMM_PROMPT		(N)
#define COMM_NOIC		(O)
#define COMM_NOPRAY		(P)
#define COMM_NONEWBIE		(Q)
#define COMM_NOGRATS		(R)
#define COMM_WIPED		(S)
#define COMM_NOCGOSSIP          (X)
#define COMM_AFK		(Z)
#define COMM_NOQGOSSIP		(cc)
#define COMM_NOSOCIAL           (dd)

#define WIZ_ON			(A)
#define WIZ_TICKS		(B)
#define WIZ_LOGINS		(C)
#define WIZ_SITES		(D)
#define WIZ_LINKS		(E)
#define WIZ_DEATHS		(F)
#define WIZ_RESETS		(G)
#define WIZ_MOBDEATHS		(H)
#define WIZ_FLAGS		(I)
#define WIZ_PENALTIES		(J)
#define WIZ_SACCING		(K)
#define WIZ_LEVELS		(L)
#define WIZ_SECURE		(M)
#define WIZ_SWITCHES		(N)
#define WIZ_SNOOPS		(O)
#define WIZ_RESTORE		(P)
#define WIZ_LOAD		(Q)
#define WIZ_NEWBIE		(R)
#define WIZ_PREFIX		(S)
#define WIZ_PLAYER_LOG		(T)
#define WIZ_PASSWORDS		(U)
#define WIZ_MEMCHECK		(V)
#define WIZ_PKILLS		(W)
#define WIZ_OTHER		(X)
#define WIZ_CLANS		(Y)
#define WIZ_CHATS		(Z)
#define WIZ_TELLS		(aa)

#define INFO_ON			(A)
#define INFO_DEATHS		(B)
#define INFO_BOUNTY		(C)
#define INFO_OTHER		(D)
#define INFO_NOTES		(E)

#define COMBAT_ON		(A)
#define COMBAT_FLAGS		(B)
#define COMBAT_SHIELDS		(C)
#define COMBAT_EVASION		(D)
#define COMBAT_OTHER		(E)
#define COMBAT_COUNTER		(F)
#define COMBAT_CRITICAL		(G)
#define COMBAT_BLEEDING		(H)
#define COMBAT_MISSES		(I)
#define COMBAT_CHAIN_SPAM	(J)
#define COMBAT_SHD_COMBINE	(K)
#define COMBAT_METER            (L)

#define	ARENA_PROGRESSING	(A)
#define ARENA_NO_POTION		(B)
#define ARENA_NO_SCROLL		(C)
#define ARENA_NO_PILL		(D)
#define ARENA_NO_FLEE		(E)
#define	ARENA_PLUS_HEALER	(F)

#define MEM_CUSTOMER	A
#define MEM_SELLER	B
#define MEM_HOSTILE	C
#define MEM_AFRAID	D

#define PENALTY_CORNER		0
#define PENALTY_NOCHANNEL	1
#define PENALTY_NOTITLE		2
#define PENALTY_FREEZE		3
#define PENALTY_NORESTORE	4
#define PENALTY_NOTELL		5
#define PENALTY_MAX		6

#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000

#define TAR_IGNORE		    0
#define TAR_CHAR_OFFENSIVE	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_SELF		    3
#define TAR_OBJ_INV		    4
#define TAR_OBJ_CHAR_DEF	    5
#define TAR_OBJ_CHAR_OFF	    6
#define TAR_OBJ_TRAN		    7

#define TARGET_CHAR		    0
#define TARGET_OBJ		    1
#define TARGET_ROOM		    2
#define TARGET_NONE		    3

#define TRIG_ACT	(A)
#define TRIG_BRIBE	(B)
#define TRIG_DEATH	(C)
#define TRIG_ENTRY	(D)
#define TRIG_FIGHT	(E)
#define TRIG_GIVE	(F)
#define TRIG_GREET	(G)
#define TRIG_GRALL	(H)
#define TRIG_KILL	(I)
#define TRIG_HPCNT	(J)
#define TRIG_RANDOM	(K)
#define TRIG_SPEECH	(L)
#define TRIG_EXIT	(M)
#define TRIG_EXALL	(N)
#define TRIG_DELAY	(O)
#define TRIG_GET	(Q)
#define TRIG_DROP	(R)
#define TRIG_SIT	(S)

#define PRG_MPROG	0
#define PRG_OPROG	1
#define PRG_RPROG	2

#define SKILL_DISABLED		(A)
#define SKILL_GLOBAL_SPELLUP	(B)
#define SKILL_NO_EMPOWER_SCROLL	(C)
#define SKILL_NO_EMPOWER_POTION	(D)
#define SKILL_NO_BREW		(E)
#define SKILL_NO_SCRIBE		(F)
#define SKILL_CAN_DISPEL	(G)
#define SKILL_CAN_CANCEL	(H)
#define SKILL_NO_CLAN_POTION	(I)
#define SKILL_NO_CLAN_SCROLL	(J)
#define SKILL_NO_CLAN_PILL	(K)
#define SKILL_NO_CLAN_WAND	(L)
#define SKILL_NO_CLAN_STAFF	(M)

struct	affect_data
{
    AFFECT_DATA *	next;
    sh_int		where;
    sh_int		type;
    sh_int		level;
    sh_int		dur_type;
    sh_int		duration;
    sh_int		location;
    sh_int		modifier;
    int			bitvector;
};

struct	area_data
{
    AREA_DATA *		next;
    char *		file_name;
    char *		name;
    char *		music;
    char *		builder;
    char *		directions;
    char		alignment;
    sh_int		age;
    sh_int		nplayer;
    sh_int		clan;
    sh_int		vnum;
    sh_int		security;
    sh_int		min_level;
    sh_int		max_level;
    long		area_flags;
    int 		min_vnum;
    int			max_vnum;
    int			run_vnum;
};

struct	arena_type
{
    ARENA_DATA	*next;
    CHAR_DATA	*team[MAX_ARENA_TEAMS];
    char	team_name[MAX_ARENA_TEAMS][MAX_STRING_LENGTH];
    sh_int	number;
    long	specials;
    bool	*disabled_skills;
};

struct attack_type
{
    char *	name;			/* name */
    char *	noun;			/* message */
    sh_int   	damage;			/* damage class */
};

struct auction_data
{
    AUCTION_DATA *	next;
    OBJ_DATA *		item;
    CHAR_DATA *		owner;
    CHAR_DATA *		high_bidder;
    sh_int		status;
    sh_int		slot;
    sh_int		bid_type;
    int			bid_amount;
};

struct	ban_data
{
    BAN_DATA *	next;
    char *	name;
    sh_int	ban_flags;
    sh_int	level;
};

struct buf_type
{
    BUFFER *	next;
    char *	string; /* buffer's string */
    sh_int	state;  /* error state of the buffer */
    int		size;   /* size in k */
    int		length;
};

struct	censor_data
{
    CENSOR_DATA	*next;
    char	*word;
    char	*replace;
};

struct	char_data
{
    CHAR_DATA *		next;
    CHAR_DATA *		next_in_room;
    CHAR_DATA *		master;
    CHAR_DATA *		leader;
    CHAR_DATA *		fighting;
    CHAR_DATA *		pet;
    CHAR_DATA *		mprog_target;
    MEM_DATA *		memory;
    SPEC_FUN *		spec_fun;
    MOB_INDEX_DATA *	pIndexData;
    DESCRIPTOR_DATA *	desc;
    AFFECT_DATA *	affected;
    OBJ_DATA *		carrying;
    OBJ_DATA *		on;
    ROOM_INDEX_DATA *	in_room;
    AREA_DATA *		zone;
    PC_DATA *		pcdata;
    sh_int		group;
    sh_int		sex;
    sh_int		class;
    sh_int		race;
    sh_int		level;
    sh_int		magic_power;
    sh_int		wait;
    sh_int		daze[MAX_DAZE];
    sh_int		invis_level;
    sh_int		incog_level;
    sh_int		ghost_level;
    sh_int		position;
    sh_int		carry_weight;
    sh_int		carry_number;
    sh_int		saving_throw;
    sh_int		alignment;
    sh_int		hitroll;
    sh_int		damroll;
    sh_int		armor[4];
    sh_int		track_to[MAX_TRACK];
    sh_int		track_from[MAX_TRACK];
    sh_int		perm_stat[MAX_STATS];
    sh_int		mod_stat[MAX_STATS];
    sh_int		size;
    sh_int		damage[3];
    sh_int		dam_type;
    sh_int		start_pos;
    sh_int		default_pos;
    sh_int		mprog_delay;
    sh_int		mprog_state;
    sh_int		stunned;
    sh_int		remember;
    sh_int		arena_number;
    sh_int		damage_mod[DAM_MAX];
    sh_int		*learned;
    sh_int		clan;
    sh_int		timer;
    sh_int		trust;
    sh_int		regen[3];
    char *		name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    char *		prompt;
    long		exp;
    long		act;
    long		comm;
    long		configure;
    long		wiznet;
    long		sound;
    long		info;
    long		combat;
    long		id;
    long		affected_by;
    long		shielded_by;
    long		parts;
    bool		valid;
    int			platinum;
    int			gold;
    int			silver;
    int			hit;
    int			max_hit;
    int			mana;
    int			max_mana;
    int			move;
    int 		max_move;
};

struct channel_data
{
    char	*name;			// Name of the channel.
    char	*ch_string;		// String ch sees.
    char	*other_string;		// String others see.
    char	*color_default;		// Default colour of text.
    char	color_variable;		// Custom color code variable.
    bool	arena;			// Can be used in arena?
    bool	censor;			// Is the channel censored?
    bool	drunk;			// Does drunk affect?
    bool	pretitle;		// Does pretitle show?
    bool	quiet;			// Does channel work with quiet on?
    long	bit;			// Comm bit used.
    int		level;			// Min level of channel.
};

struct clan_cost_type
{
    sh_int	cubic;
    sh_int	aquest;
    sh_int	iquest;
    sh_int	level;
    sh_int	max;
};

struct clan_type
{
    PKILL_RECORD *	death_list;
    PKILL_RECORD *	kills_list;
    ROSTER_DATA *	roster;
    char *		name;
    char *		color;
    char *		who_name;
    char *		exname;
    char *		crnk[MAX_CRNK];
    sh_int		max_mem;
    sh_int		members;
    sh_int		kills;
    sh_int		deaths;
    sh_int		edit_clan;
    sh_int		edit_obj;
    sh_int		edit_room;
    sh_int		edit_mob;
    sh_int		edit_help;
    sh_int		two_way_time;
    bool		independent;
    bool		pkill;
    int			hall;
    int			pit;
    int			portal_room;
    int			two_way_link;
    int			cubics;
    int			aquest;
    int			iquest;
};

struct	class_type
{
    char *	name;			/* the full name of the class */
    char 	who_name	[10];	/* Three-letter name for 'who'	*/
    sh_int	attr_prime;		/* Prime attribute		*/
    sh_int	thac0_00;		/* Thac0 for level  0		*/
    sh_int	thac0_32;		/* Thac0 for level 32		*/
    sh_int	hp_min;			/* Min hp gained on leveling	*/
    sh_int	hp_max;			/* Max hp gained on leveling	*/
    sh_int      mana_percent;
    bool	disabled;
    char *	base_group;		/* base skills gained		*/
    char *	default_group;		/* default skills gained	*/
    sh_int	sub_class;
    sh_int	tier;
};

struct	con_app_type
{
    sh_int	hitp;
    sh_int	shock;
};

struct desc_check_data
{
    DESC_CHECK_DATA *	next;
    char		name[MAX_INPUT_LENGTH];
    char		host[MAX_INPUT_LENGTH];
    sh_int		wait_time;
};

struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    void *              pEdit;		/* OLC */
    char **             pString;	/* OLC */
    char *		host;
    char *		hostip;
    char *		outbuf;
    char *              run_buf;
    char *              run_head;
    char *		showstr_head;
    char *		showstr_point;
    char		inbuf		[4 * MAX_INPUT_LENGTH];
    char		incomm		[4 * MAX_INPUT_LENGTH];
    char		inlast		[4 * MAX_INPUT_LENGTH];
    char 		a_cur		[4 * MAX_INPUT_LENGTH];
    bool		fcommand;
    bool		valid;
    sh_int		descriptor;
    sh_int		connected;
    sh_int		timer;
    sh_int		repeat;
    sh_int		editor;		/* OLC */
    sh_int		shd_damage[2];
    int			outsize;
    int			outtop;
    z_stream *		out_compress;		/* MCCP */
    unsigned char *	out_compress_buf;	/* MCCP */
};

struct	dex_app_type
{
    sh_int	defensive;
};

struct disabled_data
{
    DISABLED_DATA	*next;
    struct cmd_type const *command;
    char 		*disabled_by;
    sh_int 		level;
};

struct	exit_data
{
    union
    {
	ROOM_INDEX_DATA *	to_room;
	int			vnum;
    } u1;
    int			exit_info;
    int			key;
    char *		keyword;
    char *		description;
    EXIT_DATA *		next;		/* OLC */
    int			rs_flags;	/* OLC */
    int			orig_door;	/* OLC */
//    bool		color;		/* Pathfind */
};

struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	/* Next in list                     */
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};

struct function_type
{
    char *	string;
    SPELL_FUN *	spell;
    sh_int *	gsn;
};

struct gen_data
{
    GEN_DATA	*next;
    bool	*skill_chosen;
    bool	*group_chosen;
    int		points_chosen;
};

struct	grant_data
{
    GRANT_DATA	*next;
    char	*command;
    char	*granter;
};

struct  group_type
{
    char *	name;
    sh_int	*rating;
    char *	spells[MAX_IN_GROUP];
};

struct	help_data
{
    HELP_DATA *	next;
    char *	keyword;
    char *	name;
    char *	text;
    sh_int	level;
    sh_int	clan;
};

struct info_type
{
    char *	name;
    long	flag;
};

struct	int_app_type
{
    sh_int	learn;
};

struct item_type
{
    sh_int	type;
    char *	name;
};

struct liq_type
{
    char *	liq_name;
    char *	liq_color;
    sh_int	liq_affect[5];
};

struct mem_data
{
    MEM_DATA 	*next;
    int		id;
    int 	reaction;
    time_t 	when;
};

struct	mob_index_data
{
    MOB_INDEX_DATA *	next;
    PROG_LIST *         mprogs;
    AREA_DATA *		area;		/* OLC */
    SHOP_DATA *		pShop;
    SPEC_FUN *		spec_fun;
    char *		player_name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    char *		die_descr;
    char *		say_descr;
    sh_int		max_world;
    sh_int		reflection;
    sh_int		absorption;
    sh_int		group;
    sh_int		count;
    sh_int		alignment;
    sh_int		level;
    sh_int		hitroll;
    sh_int		ac[4];
    sh_int 		dam_type;
    sh_int		start_pos;
    sh_int		default_pos;
    sh_int		sex;
    sh_int		race;
    sh_int		size;
    sh_int		bank_branch;
    sh_int		damage_mod[DAM_MAX];
    sh_int		class;
    sh_int		*learned;
    sh_int		skill_percentage;
    sh_int		saves;
    sh_int		exp_percent;
    sh_int		regen[3];
    long		act;
    long		affected_by;
    long		shielded_by;
    long		wealth;
    long		parts;
    long		mprog_flags;
    long		perm_mob_pc_deaths[2];
    int			vnum;
    int 		hit[2];
    int 		mana[2];
    int 		damage[3];
    int			mob_pc_deaths[2];
};

struct	multiplay_data
{
    MULTIPLAY_DATA *	next;
    char *		name;
    char *		host;
    sh_int		allow_flags;
};

struct	note_data
{
    NOTE_DATA *	next;
    sh_int	type;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    char *	text;
    time_t  	date_stamp;
};

struct	obj_data
{
    OBJ_DATA *		next;
    OBJ_DATA *		next_content;
    OBJ_DATA *		contains;
    OBJ_DATA *		in_obj;
    OBJ_DATA *		on;
    OBJ_MULTI_DATA *	multi_data;
    CHAR_DATA *		carried_by;
    CHAR_DATA *		dropped_by;
    CHAR_DATA *		disarmed_from;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
    CHAR_DATA *		oprog_target;
    bool		enchanted;
    char *	        owner;
    char *		killer;
    char *		name;
    char *		short_descr;
    char *		description;
    char *		loader;
    long		extra_flags;
    int			last_room;
    int			cost;
    long		wear_flags;
    int			value	[5];
    sh_int		wear_loc;
    sh_int		item_type;
    sh_int		weight;
    sh_int		level;
    sh_int		size;
    sh_int		timer;
    sh_int		looted_items;
    sh_int		arena_number;
    sh_int		oprog_delay;
    sh_int		success;
};

struct	obj_index_data
{
    OBJ_INDEX_DATA *	next;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    AREA_DATA *		area;		/* OLC */
    PROG_LIST *		oprogs;
    char *		name;
    char *		short_descr;
    char *		description;
    char *		history;
    sh_int		reset_num;
    sh_int		size;
    sh_int		item_type;
    sh_int		level;
    sh_int		count;
    sh_int		weight;
    sh_int		quest_points;
    sh_int		forge_count;
    sh_int		success;
    sh_int		targets;
    bool		*class_can_use;
    long		oprog_flags;
    long		extra_flags;
    long		wear_flags;
    int			forge_vnum;
    int			vnum;
    int			cost;
    int			value[5];
};

struct obj_multi_data
{
    OBJ_MULTI_DATA *	next;
    char *		socket;
    char * 		dropper;
    sh_int		drop_timer;
};

struct pc_data
{
    ARENA_DATA *	match;
    PC_DATA *		next;
    PKILL_DATA *	recent_pkills;
    PKILL_RECORD *	kills_list;
    PKILL_RECORD *	death_list;
    ROOM_INDEX_DATA *	was_in_room;
    CHAR_DATA *		opponent;
    CHAR_DATA *		next_arena;
    CHAR_DATA *         next_player;
    CHAR_DATA *		reply;
    OBJ_DATA *		restring_item;
    OBJ_DATA *		storage_list[MAX_BANK];
    BUFFER * 		buffer;
    NOTE_DATA *		pnote;
    GEN_DATA *		gen_data;
    GRANT_DATA *	grants;
    char *		pwd;
    char *		identity;
    char *		bamfin;
    char *		bamfout;
    char *		title;
    char *		pretitle;
    char *		who_descr;
    char *		who_output;
    char *		socket;
    char *		forget[MAX_FORGET];
    char *		alias[MAX_ALIAS];
    char *		alias_sub[MAX_ALIAS];
    char *		questroom;
    char *    		questarea;
    char *		prefix;
    time_t              last_note;
    time_t              last_idea;
    time_t              last_penalty;
    time_t              last_news;
    time_t              last_changes;
    time_t		llogoff;
    time_t		logon;
    sh_int		max_storage[MAX_BANK];
    sh_int		true_sex;
    sh_int		pkpoints;
    sh_int		pkills;
    sh_int		pdeath;
    sh_int		assist;
    sh_int		pktimer;
    sh_int		points;
    sh_int		lag;
    sh_int 		security;	/* OLC */ /* Builder security */
    sh_int		last_level;
    sh_int		tier;
    sh_int		dtimer;
    sh_int		clan_rank;
    sh_int		arenawins;
    sh_int		arenaloss;
    sh_int		arenakills;
    sh_int		arenadeath;
    sh_int		team;
    sh_int		spam_count;
    sh_int              nextquest;
    sh_int              countdown;
    sh_int		condition	[4];
    sh_int		tells;
    sh_int		bank_account[MAX_BANK];
    sh_int		penalty_time[PENALTY_MAX];
    sh_int		invited;
    sh_int		practice;
    sh_int		train;
    sh_int		lines;
    sh_int		color;
    sh_int		color_auc;
    sh_int		color_cht;
    sh_int		color_cgo;
    sh_int		color_cla;
    sh_int		color_con;
    sh_int		color_dis;
    sh_int		color_fig;
    sh_int		color_gos;
    sh_int		color_gra;
    sh_int		color_gte;
    sh_int		color_imm;
    sh_int		color_mob;
    sh_int		color_opp;
    sh_int		color_qgo;
    sh_int		color_que;
    sh_int		color_quo;
    sh_int		color_roo;
    sh_int		color_say;
    sh_int		color_sho;
    sh_int		color_tel;
    sh_int		color_wit;
    sh_int		color_wiz;
    sh_int		color_ooc;
    sh_int		color_rac;
    sh_int		color_fla;
    sh_int		color_her;
    sh_int		color_ic;
    sh_int		color_pra;
    sh_int		color_olc1;
    sh_int		color_olc2;
    sh_int		color_olc3;
    long		mobkills;
    long		mobdeath;
    long		devote[DEVOTE_CURRENT+1];
    long		devote_next[DEVOTE_CURRENT+1];
    bool		*group_known;
    bool              	confirm_delete;
    bool              	confirm_reroll;
    bool		confirm_whostring;
    bool		confirm_restring;
    bool		confirm_pretitle;
    bool		confirm_condemn;
    bool		on_quest;
    bool		attacker;
    bool		valid;
    int			devote_points[DEVOTE_CURRENT+1];
    int			recall;
    int			total_questfail;
    int			total_questpoints;
    int			total_questcomplete;
    int			total_questexpire;
    int			total_questattempt;
    int			deviant_points[2];
    int			bounty;
    int			chat_chan;
    int 		perm_hit;
    int 		perm_mana;
    int 		perm_move;
    int			max_damage;
    int			questgiver;
    int                 questpoints;
    int                 questobj;
    int  	        questmob;
    int			played;
    int			gq_mobs[MAX_GQUEST_MOB];
};

struct pkill_data
{
    PKILL_DATA *	next;
    char *		player_name;
    time_t		time;
    bool		killer;
};

struct pkill_record
{
    PKILL_RECORD *	next;
    time_t		pkill_time;
    char *		killer_name;
    char *		victim_name;
    char *		killer_clan;
    char *		victim_clan;
    char *		assist_string;
    sh_int		level[2];
    sh_int		pkill_points;
    sh_int		bounty;
};

struct poll_data
{
    POLL_DATA	*next;
    VOTE_DATA	*vote;
    char	*name;
    char	*question;
    char	*response[MAX_POLL_RESPONSES];
};

struct prog_code
{
    AREA_DATA *		area;
    PROG_CODE *		next;
    char *		author;
    char *		code;
    char *		name;
    int			vnum;
};

struct prog_list
{
    PROG_LIST * 	next;
    char *		trig_phrase;
    char *  		code;
    int			trig_type;
    int			vnum;
};

struct race_type
{
    char *	name;			/* call name of the race */
    char *	skills[5];		/* bonus skills for the race */
    char 	who_name[10];
    bool	pc_race;		/* can be chosen by pcs */
    sh_int	points;			/* cost in points of the race */
    sh_int 	stats[MAX_STATS];	/* starting stats */
    sh_int	*class_mult;		/* exp multiplier for class, * 100 */
    sh_int	max_stats[MAX_STATS];	/* maximum stats */
    sh_int	damage_mod[DAM_MAX];
    sh_int	size;			/* aff bits for the race */
    sh_int	attack;
    long	aff;			/* aff bits for the race */
    long	shd;			/* shd bits for the race */
    long	parts;			/* default parts for the race */
    bool	disabled;
    bool	*class_can_use;
};

struct	reset_data
{
    RESET_DATA *	next;
    char		command;
    sh_int		percent;
    int			arg1;
    int			arg2;
    int			arg3;
    int			arg4;
};

struct room_damage_data
{
    ROOM_DAMAGE_DATA	*next;
    char		*msg_room;
    char		*msg_victim;
    int			damage_type;
    int			damage_min;
    int			damage_max;
    int			success;
};

struct	room_index_data
{
    ROOM_INDEX_DATA *	next;
    ROOM_DAMAGE_DATA *	room_damage;
    CHAR_DATA *		people;
    OBJ_DATA *		contents;
    EXTRA_DESCR_DATA *	extra_descr;
    AREA_DATA *		area;
    EXIT_DATA *		exit	[MAX_DIR];
    RESET_DATA *	reset_first;	/* OLC */
    PROG_LIST *		rprogs;
    CHAR_DATA *		rprog_target;
    long		rprog_flags;
    sh_int		rprog_delay;
    char *		name;
    char *		description;
    char *		music;
    int 		vnum;
    long		room_flags;
    sh_int		sector_type;
    sh_int		light;
    sh_int		heal_rate;
    sh_int 		mana_rate;
    sh_int		max_people;
//    int			heap_index;	//
//    int			steps;		//  Pathfind
//    bool		visited;	//
    ROOM_INDEX_DATA	*visited_new;

};

struct	roster_data
{
    ROSTER_DATA *	next;
    char *		name;
    int			rank;
};

struct	shop_data
{
    SHOP_DATA *	next;			/* Next shop in list		*/
    int		keeper;			/* Vnum of shop keeper mob	*/
    sh_int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    sh_int	profit_buy;		/* Cost multiplier for buying	*/
    sh_int	profit_sell;		/* Cost multiplier for selling	*/
    sh_int	open_hour;		/* First opening hour		*/
    sh_int	close_hour;		/* First closing hour		*/
};

struct	skill_type
{
    COST_DATA *	cost_potion;		/* Clan Stuff */
    COST_DATA * cost_scroll;		// --
    COST_DATA * cost_pill;		// --
    COST_DATA * cost_wand;		// --
    COST_DATA * cost_staff;		// --
    SPELL_FUN *	spell_fun;		/* Spell pointer (for spells)	*/
    char *	name;			/* Name of skill		*/
    char *	noun_damage;		/* Damage message		*/
    char *	msg_off;		/* Wear off message		*/
    char *	msg_obj;		/* Wear off message for obects	*/
    char *	room_msg;
    char *	sound_cast;
    char *	sound_off;
    sh_int *	pgsn;			/* Pointer to associated gsn	*/
    sh_int *	skill_level;		/* Level needed by class	*/
    sh_int *	rating;			/* How hard it is to learn	*/
    sh_int	target;			/* Legal targets		*/
    sh_int	minimum_position;	/* Position for caster / user	*/
    sh_int	cost_hp;		/* Minimum hp used		*/
    sh_int	cost_mana;		/* Minimum mana used		*/
    sh_int	cost_move;		/* Minimum move used		*/
    sh_int	beats;			/* Waiting time after use	*/
    long	flags;
};

struct	social_type
{
    char	name[20];
    char *	char_no_arg;
    char *	others_no_arg;
    char *	char_found;
    char *	others_found;
    char *	vict_found;
    char *	char_not_found;
    char *	char_auto;
    char *	others_auto;
};

struct spec_type
{
    char * 	name;			/* special function name */
    SPEC_FUN *	function;		/* the function */
};

struct stat_data
{
    char *	evil_god_string;
    char *	good_god_string;
    char *	mud_name_string;
    char *	neut_god_string;
    sh_int	connect_since_reboot;
    sh_int	exp_mod[2];
    sh_int	max_ever;
    sh_int	most_today;
    sh_int	quest_gold[2];
    sh_int	quest_points[2];
    sh_int	quest_pracs[2];
    sh_int	quest_exp[2];
    sh_int	quest_object[2];
    sh_int	timeout_mortal;
    sh_int	timeout_immortal;
    sh_int	timeout_ld_mort;
    sh_int	timeout_ld_imm;
    bool	capslock;
    bool	changed;
    bool	colorlock;
    bool	fLogAll;
    bool	classes_changed;
    bool	helps_changed;
    bool	randoms_changed;
    bool	races_changed;
    bool	skills_changed;
    bool	socials_changed;
    bool	clans_changed;
    bool	multilock;
    bool	newlock;
    bool	wizlock;
    int		random_vnum[2];
    int		unique_vnum[2];
    int		quest_obj_vnum[2];

//    long	mob_kills;
//    int		auction_return;
//    int		auction_sold;
//    int		connections;
//    int		deletions;
//    int		heroes;
//    int		levels;
//    int		new_players;
//    int		mob_death;
//    int		quest_complete;
//    int		quest_failed;
//    int		quest_points;
//    int		quest_total;
};

struct	str_app_type
{
    sh_int	tohit;
    sh_int	todam;
    sh_int	carry;
    sh_int	wield;
};

struct struckdrunk
{
    int		min_drunk_level;
    int		number_of_rep;
    char	*replacement[11];
};

struct	target_type
{
    char *	name;
    sh_int	bit;
};

struct	time_info_data
{
    int		hour;
    int		day;
    int		month;
    int		year;
};

struct vote_data
{
    VOTE_DATA	*next_vote;
    char	*voter_name;
    char	*voter_ip;
    sh_int	voter_vote;
};

struct weapon_type
{
    char *	name;
    int		vnum;
    sh_int	type;
    sh_int	*gsn;
};

struct	weather_data
{
    int		mmhg;
    int		change;
    int		sky;
    int		sunlight;
};

struct	wis_app_type
{
    sh_int	practice;
};

struct	wiz_data
{
    WIZ_DATA *	next;
    char *	name;
    sh_int	level;
    bool	valid;
};

struct wiznet_type
{
    char *	name;
    long 	flag;
    sh_int	level;
};

extern	sh_int	gsn_locust_swarm;
extern	sh_int	gsn_backdraft;
extern	sh_int	gsn_leech;
extern	sh_int	gsn_electrify;
extern	sh_int	gsn_glacial_aura;
extern	sh_int	gsn_cure_blind;
extern	sh_int	gsn_haste;
extern	sh_int	gsn_divine_heal;
extern	sh_int	gsn_heal;
extern	sh_int	gsn_cure_serious;
extern	sh_int	gsn_cure_critical;
extern	sh_int	gsn_cure_light;
extern	sh_int	gsn_dispel_magic;
extern	sh_int	gsn_slow;
extern	sh_int	gsn_weaken;
extern	sh_int	gsn_acid_breath;
extern	sh_int	gsn_acid_blast;
extern	sh_int	gsn_acid_storm;
extern	sh_int	gsn_frost_breath;
extern	sh_int	gsn_snow_storm;
extern	sh_int	gsn_swarm;
extern	sh_int	gsn_magic_missile;
extern	sh_int	gsn_fire_breath;
extern	sh_int	gsn_fire_storm;
extern	sh_int	gsn_fireball;
extern	sh_int	gsn_flamestrike;
extern	sh_int	gsn_ray_of_truth;
extern	sh_int	gsn_angelfire;
extern  sh_int  gsn_boiling_blood;
extern	sh_int	gsn_lightning_breath;
extern	sh_int	gsn_electrical_storm;
extern	sh_int	gsn_lightning_bolt;
extern	sh_int	gsn_nightmare;
extern	sh_int	gsn_energy_drain;
extern	sh_int	gsn_demonfire;
extern	sh_int	gsn_gas_breath;
extern	sh_int	gsn_downpour;
extern	sh_int	gsn_divinity;
extern	sh_int	gsn_protect_good;
extern	sh_int	gsn_protect_neutral;
extern	sh_int	gsn_protect_evil;
extern	sh_int	gsn_remove_curse;
extern	sh_int	gsn_bless;
extern	sh_int	gsn_cure_disease;
extern	sh_int	gsn_cure_poison;
extern	sh_int	gsn_darkvision;
extern	sh_int	gsn_detect_invis;
extern	sh_int	gsn_detect_hidden;
extern	sh_int	gsn_detect_good;
extern	sh_int	gsn_detect_evil;
extern	sh_int	gsn_detect_magic;
extern	sh_int	gsn_constitution;
extern	sh_int	gsn_intellect;
extern	sh_int	gsn_wisdom;
extern	sh_int	gsn_armor;
extern	sh_int	gsn_barkskin;
extern	sh_int	gsn_shield;
extern	sh_int	gsn_steel_skin;
extern	sh_int	gsn_stone_skin;
extern	sh_int	gsn_frenzy;
extern	sh_int	gsn_farsight;
extern	sh_int	gsn_infravision;
extern	sh_int	gsn_giant_strength;
extern	sh_int	gsn_pass_door;
extern	sh_int	gsn_regeneration;
extern	sh_int	gsn_growth;
extern	sh_int	gsn_mana_shield;
extern	sh_int	gsn_refresh;

extern	sh_int	gsn_brew;
extern	sh_int	gsn_scribe;
extern	sh_int	gsn_slip;
extern	sh_int	gsn_ambush;
extern	sh_int	gsn_backstab;
extern	sh_int	gsn_camouflage;
extern	sh_int	gsn_circle;
extern	sh_int	gsn_dodge;
extern  sh_int  gsn_envenom;
extern	sh_int	gsn_feed;
extern	sh_int	gsn_forest_meld;
extern	sh_int	gsn_hone;
extern	sh_int	gsn_hide;
extern	sh_int	gsn_stalk;
extern	sh_int	gsn_obfuscate;
extern	sh_int	gsn_peek;
extern	sh_int	gsn_pick_lock;
extern	sh_int	gsn_sharpen;
extern	sh_int	gsn_sneak;
extern	sh_int	gsn_steal;
extern	sh_int	gsn_tackle;
extern	sh_int	gsn_troll_revival;

extern  sh_int  gsn_critical;
extern  sh_int  gsn_critdam;
extern  sh_int  gsn_counter;
extern	sh_int	gsn_disarm;
extern	sh_int	gsn_disguise;
extern	sh_int	gsn_axe_mastery;
extern	sh_int	gsn_savage_claws;
extern	sh_int	gsn_knife_fighter;
extern	sh_int	gsn_dual_wield;
extern	sh_int	gsn_engage;
extern	sh_int	gsn_engineer;
extern	sh_int	gsn_enhanced_damage;
extern	sh_int	gsn_evade;
extern	sh_int	gsn_hurl;
extern	sh_int	gsn_kick;
extern	sh_int	gsn_roundhouse;
extern	sh_int	gsn_parry;
extern  sh_int  gsn_phase;
extern	sh_int	gsn_raze;
extern	sh_int	gsn_repair;
extern	sh_int	gsn_rescue;
extern  sh_int	gsn_rub;
extern	sh_int	gsn_scribe;
extern	sh_int	gsn_second_attack;
extern	sh_int	gsn_third_attack;
extern	sh_int	gsn_fourth_attack;
extern	sh_int	gsn_fifth_attack;
extern	sh_int	gsn_sixth_attack;
extern	sh_int	gsn_seventh_attack;

extern	sh_int	gsn_blindness;
extern	sh_int	gsn_bloodbath;
extern	sh_int	gsn_charm_person;
extern	sh_int	gsn_curse;
extern	sh_int	gsn_curse_of_ages;
extern	sh_int	gsn_divine_blessing;
extern	sh_int	gsn_infernal_offer;
extern	sh_int	gsn_invis;
extern	sh_int	gsn_mana_tap;
extern	sh_int	gsn_mass_invis;
extern  sh_int  gsn_plague;
extern	sh_int	gsn_poison;
extern	sh_int	gsn_sleep;
extern  sh_int  gsn_fly;
extern  sh_int  gsn_sanctuary;

extern	sh_int	gsn_acidshield;
extern	sh_int	gsn_divine_aura;
extern	sh_int	gsn_fireshield;
extern	sh_int	gsn_iceshield;
extern	sh_int	gsn_rockshield;
extern	sh_int	gsn_shockshield;
extern	sh_int	gsn_shrapnelshield;
extern	sh_int	gsn_thornshield;
extern	sh_int	gsn_vampiricshield;
extern	sh_int	gsn_watershield;

extern sh_int  gsn_axe;
extern sh_int  gsn_dagger;
extern sh_int  gsn_flail;
extern sh_int  gsn_mace;
extern sh_int  gsn_polearm;
extern sh_int  gsn_quarterstaff;
extern sh_int  gsn_shield_block;
extern sh_int  gsn_shield_levitate;
extern sh_int  gsn_spear;
extern sh_int  gsn_sword;
extern sh_int  gsn_whip;

extern sh_int  gsn_bash;
extern sh_int  gsn_berserk;
extern sh_int  gsn_cyclone;
extern sh_int  gsn_dirt;
extern sh_int  gsn_feed;
extern sh_int  gsn_hand_to_hand;
extern sh_int  gsn_retreat;
extern sh_int  gsn_trip;

extern sh_int  gsn_fast_healing;
extern sh_int  gsn_haggle;
extern sh_int  gsn_lore;
extern sh_int  gsn_master_of_magic;
extern sh_int  gsn_meditation;

extern sh_int  gsn_scrolls;
extern sh_int  gsn_staves;
extern sh_int  gsn_wands;
extern sh_int  gsn_recall;
extern sh_int  gsn_strangle;
extern sh_int  gsn_stun;
extern sh_int  gsn_track;
extern sh_int  gsn_gouge;
extern sh_int  gsn_grip;
extern sh_int  gsn_overhand;
extern sh_int  gsn_crush;
extern sh_int  gsn_onslaught;
extern sh_int  gsn_lunge;
extern sh_int  gsn_scalp;
extern sh_int  gsn_stake;
extern sh_int  gsn_ultra_damage;

/* New GSNs by Stroke */
extern sh_int  gsn_battlehymn;
extern sh_int  gsn_warcry;
extern sh_int  gsn_shield_smash;
extern sh_int  gsn_dislodge;
extern sh_int  gsn_awen;
extern sh_int  gsn_bandage;
extern sh_int  gsn_gash;
extern sh_int  gsn_cure_weaken;
extern sh_int  gsn_deathblow;
extern sh_int  gsn_devotion;
extern sh_int  gsn_dismember;
extern sh_int  gsn_charge;
extern sh_int  gsn_legsweep;
extern sh_int  gsn_quickdraw;
extern sh_int  gsn_salve;
extern sh_int  gsn_spirit;
extern sh_int  gsn_spirit;
extern sh_int  gsn_2nd_dual;
extern sh_int  gsn_3rd_dual;
extern sh_int  gsn_assassinate;
extern sh_int  gsn_trapdisarm;
extern sh_int  gsn_trapset;
extern sh_int  gsn_storm_of_blades;
extern sh_int  gsn_cross_slash;
extern sh_int  gsn_sidestep;
extern sh_int  gsn_cartwheel;
extern sh_int  gsn_eighth_attack;
extern sh_int  gsn_doorbash;

#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))
#define LOAD( literal, field, value )	\
    if ( !str_cmp( word, literal ) )	\
    {					\
	field  = value;			\
	break;				\
    }
#define KEY( literal, field, value )	\
    if ( !str_cmp( word, literal ) )	\
    {					\
	field	= value;		\
	fMatch	= TRUE;			\
	break;				\
    }
#define SKEY( literal, field )		\
    if ( !str_cmp( word, literal ) )	\
    {					\
	free_string( field );		\
	field = fread_string( fp );	\
	fMatch = TRUE;			\
	break;				\
    }
#define IS_NPC(ch)		(ch->pcdata == NULL && ch->pIndexData != NULL)
#define IS_IMMORTAL(ch)		(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust(ch) >= LEVEL_HERO)
#define IS_TRUSTED(ch,level)	(get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define IS_SHIELDED(ch, sn)	(IS_SET((ch)->shielded_by, (sn)))
#define GET_AGE(ch)		((int) (17 + ((ch)->played \
				    + current_time - (ch)->logon )/72000))
#define IS_GOOD(ch)		(ch->alignment >= 350)
#define IS_EVIL(ch)		(ch->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define GET_AC(ch,type)		((ch)->armor[type]			    \
		        + ( IS_AWAKE(ch)			    \
			? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0 ))
#define GET_HITROLL(ch)	\
		((ch)->hitroll+str_app[get_curr_stat(ch,STAT_STR)].tohit)
#define GET_DAMROLL(ch) \
		((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam)
#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS))
#define WAIT_STATE(ch, npulse)	((ch)->wait = IS_IMMORTAL(ch) ? 0 : UMAX((ch)->wait, (npulse)))
#define DAZE_STATE(ch, npulse, type)  ((ch)->daze[type] = UMAX((ch)->daze[type], (npulse)))
#define get_carry_weight(ch)	((ch)->carry_weight + ((ch)->silver/10) +  \
				 ((ch)->gold * 2 / 5) + ((ch)->platinum / 100))
#define HAS_TRIGGER_MOB(ch,trig) (IS_SET((ch)->pIndexData->mprog_flags,(trig)))
#define HAS_TRIGGER_OBJ(obj,trig) (IS_SET((obj)->pIndexData->oprog_flags,(trig)))
#define HAS_TRIGGER_ROOM(room,trig) (IS_SET((room)->rprog_flags,(trig)))

#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))
#define WEIGHT_MULT(obj)	((obj)->item_type == ITEM_CONTAINER ? \
	(obj)->value[4] : 100)

#define PERS(ch, looker)        ( can_see( looker, (ch) ) ?		\
                                ( IS_NPC(ch) ? (ch)->short_descr	\
                                : (ch)->name ) :			\
                                ( IS_NPC(ch) ? "someone" :		\
                                ( IS_IMMORTAL(ch) ?			\
                                ( (ch)->pcdata->identity[0] == '\0' ?	\
                                "An Immortal" : (ch)->pcdata->identity )\
                                : "someone" )))
char *  crypt           args( ( const char *key, const char *salt ) );

#if defined( NOCRYPT )
    #define crypt( s1, s2 )   ( s1 )
#endif

extern	const	struct	str_app_type	str_app		[36];
extern	const	struct	int_app_type	int_app		[36];
extern	const	struct	wis_app_type	wis_app		[36];
extern	const	struct	dex_app_type	dex_app		[36];
extern	const	struct	con_app_type	con_app		[36];
extern	const	struct	weapon_type	weapon_table	[];
extern	const	struct	wiznet_type	wiznet_table	[];
extern	const	struct	wiznet_type	config_flags	[];
extern	const	struct	info_type	info_table	[];
extern	const	struct	info_type	combat_table	[];
extern	const	struct	attack_type	attack_table	[];
extern  const	struct	spec_type	spec_table	[];
extern	const	struct	liq_type	liq_table	[];
extern	const	struct	function_type	function_table	[];
extern	const	struct	target_type	target_table	[];
extern	const	struct	spec_type	spec_table	[];
extern          struct  social_type	*social_table;
extern		struct  race_type	*race_table;
extern		struct	clan_type	*clan_table;
extern		struct  group_type      *group_table;
extern		struct	skill_type	*skill_table;
extern		struct	class_type	*class_table;
extern  char *	const			dir_name	[];
extern	char *	const			penalty_name	[PENALTY_MAX];

extern	const	sh_int			rev_dir		[];
extern	const	sh_int			movement_loss	[SECT_MAX];
extern	const	sh_int	rev_dir         [];


extern		DESCRIPTOR_DATA *	descriptor_list;
extern		MULTIPLAY_DATA	*	multiplay_list;
extern		DESC_CHECK_DATA	*	desc_check_list;
extern		TIME_INFO_DATA		time_info;
extern		DISABLED_DATA	*	disabled_first;
extern		WEATHER_DATA		weather_info;
extern		STAT_DATA		mud_stat;
extern		AUCTION_DATA	  *	auction_list;
extern		ARENA_DATA	*	arena_matches;
extern		PKILL_RECORD	*	recent_list;
extern          PROG_CODE         *     mprog_list;
extern          PROG_CODE         *     rprog_list;
extern          PROG_CODE         *     oprog_list;
extern		HELP_DATA	*	help_first;
extern		SHOP_DATA	*	shop_first;
extern		CHAR_DATA	*	char_list;
extern		CHAR_DATA	*	player_list;
extern          AREA_DATA *             area_first;
extern          AREA_DATA *             area_last;
extern  	SHOP_DATA *		shop_last;
extern		POLL_DATA	  *	first_poll;
extern		MOB_INDEX_DATA *	mob_index_hash  [MAX_KEY_HASH];
extern		OBJ_INDEX_DATA *	obj_index_hash  [MAX_KEY_HASH];
extern		ROOM_INDEX_DATA *	room_index_hash [MAX_KEY_HASH];
extern		OBJ_DATA	*	object_list;
extern		time_t			current_time;
extern		sh_int			global_quest;
extern		sh_int			global_newbie;
extern          char                    str_empty       [1];
extern		char			log_buf		[];
extern          char                    last_command	[MAX_STRING_LENGTH];
extern		bool			MOBtrigger;
extern		bool			perm_affect;
extern          long                    nAllocString;
extern          long                    sAllocString;
extern          long                    nAllocPerm;
extern          long                    sAllocPerm;
extern		int			top_area;
extern          int                     top_ed;
extern          int                     top_exit;
extern          int                     top_help;
extern          int                     top_mob_index;
extern          int                     top_obj_index;
extern		int			top_reset;
extern		int			top_shop;
extern          int                     top_room;
extern		int			maxClan;
extern		int			maxRace;
extern		int			maxSocial;
extern		int			maxGroup;
extern		int			maxSkill;
extern		int			maxClass;
extern		sh_int			auction_ticket;
extern		sh_int			arena_match_count;

#define AREA_LIST       "area.lst"
#define BACKUP_DIR	"../backup/"
#define BAN_FILE	"../data/info/ban.dat"
#define BUG_FILE        "../data/info/bugs.txt"
#define CHANGED_FILE    "../data/info/changed.txt"
#define CHANGES_FILE	"../data/info/change.not"
#define CLASSES_FILE	"../data/info/classes.dat"
#define COPYOVER_FILE   "../data/copyover.dat"
#define DELETED_DIR	"../data/deleted/"
#define DISABLED_FILE	"../data/disable.dat"
#define EXE_FILE        "../area/rot"
#define GOD_DIR         "../gods/"
#define HELPS_FILE	"../data/info/help.are"
#define IDEA_FILE	"../data/info/ideas.not"
#define LAST_COMMAND    "../data/command.dat"
#define NEWS_FILE	"../data/info/news.not"
#define NO_HELP_FILE	"../data/info/nohelp.txt"
#define NOTE_FILE       "../data/info/notes.not"
#define OBJECTS_FILE	"../data/info/objects.dat"
#define PENALTY_FILE	"../data/info/penalty.not"
#define PKLOG		"../log/pkill.txt"
#define PLAYER_DIR      "../player/"
#define RACES_FILE	"../data/info/races.dat"
#define SKILLS_FILE	"../data/info/skills.dat"
#define TEMP_FILE	"../data/data.tmp"
#define TO_CODE_FILE	"../data/info/to_code.txt"
#define TO_DO_FILE	"../data/info/to_do.txt"
#define TYPO_FILE       "../data/info/typo.txt"
#define VARIABLE_FILE	"../data/info/variable.dat"
#define WIZ_FILE	"../data/info/wizlist.dat"

#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define AD	AFFECT_DATA
#define PC	PROG_CODE

/* act_comm.c */
bool	check_forget	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	force_quit	args( ( CHAR_DATA *ch, char *argument ) );
void	stop_follower	args( ( CHAR_DATA *ch ) );
void 	nuke_pets	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch ) );
void	info		args( ( char *string, CHAR_DATA *ch, CHAR_DATA *victim, long flag ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
char	*channel_parse	args( ( CHAR_DATA *ch, char *string, bool censor ) );

/* act_enter.c */
RID  *get_random_room   args ( (CHAR_DATA *ch) );

/* act_info.c */
void	set_title	args( ( CHAR_DATA *ch, char *title ) );
void	set_pretitle	args( ( CHAR_DATA *ch, char *pretitle ) );

/* act_move.c */
void	move_char	args( ( CHAR_DATA *ch, int door, bool follow, bool quiet ) );

/* act_obj.c */
void	set_obj_sockets	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool    can_loot	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *  get_obj         args( ( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container, bool multi_obj ) );
bool	check_objpktest args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	obj_droptest	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
BUFFER *display_stats	args( ( OBJ_DATA *obj, CHAR_DATA *ch, bool content ) );
void	set_obj_loader	args( ( CHAR_DATA *ch, OBJ_DATA *obj, char *string ) );

/* act_wiz.c */
void append_todo	args( ( CHAR_DATA *ch, char *file, char *str ) );
bool can_over_ride	args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool equal ) );
void wiznet		args( (char *string, CHAR_DATA *ch, const void *arg1,
			       long flag, long flag_skip, int min_level ) );
ROOM_INDEX_DATA *       find_location   args( ( CHAR_DATA *ch, char *arg ) );
void	parse_logs	args( ( CHAR_DATA *ch, char *path, char *argument ) );

/* alias.c */
void 	substitute_alias	args( (DESCRIPTOR_DATA *d, char *input) );

/* arena.c */
void	arena_clear	args( ( ARENA_DATA *match ) );
void	arena_recover	args( ( CHAR_DATA *ch ) );
void	set_arena_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	arena_flag	args( ( CHAR_DATA *ch, int flag ) );

/* ban.c */
bool	check_ban	args( ( char *site, int type ) );
bool	check_allow	args( ( char *site, int type ) );

/* bit.c */
struct spells_type
{
    char *	spell;
    int		bit;
    sh_int	location;
};

struct color_type
{
    char *	name;
    char *	color_code;
};

struct flag_type
{
    char *name;
    int bit;
    bool settable;
};

struct devote_type
{
    char	*name;
    int		bit;
    char	*bonus_name;
};

struct	bit_type
{
	const	struct	flag_type *	table;
	char *				help;
};

extern	const	struct	flag_type	act_flags[];
extern	const	struct	flag_type	plr_flags[];
extern	const	struct	spells_type	object_affects[];
extern	const	struct	flag_type	affect_flags[];
extern	const	struct	flag_type	shield_flags[];
extern	const	struct	flag_type	part_flags[];
extern	const	struct	flag_type	comm_flags[];
extern	const	struct	flag_type	extra_flags[];
extern	const	struct	flag_type	wear_flags[];
extern	const	struct	flag_type	weapon_flags[];
extern	const	struct	flag_type	container_flags[];
extern	const	struct	flag_type	room_flags[];
extern 	const	struct  flag_type	mprog_flags[];
extern	const	struct	flag_type	oprog_flags[];
extern	const	struct	flag_type	rprog_flags[];
extern	const	struct	flag_type	sound_flags[];
extern	const	struct	flag_type	trap_type_table[TRAP_MAX];
extern	const	struct	flag_type	damage_mod_table[DAM_MAX];
extern	const	struct	flag_type	skill_flags[];
extern	const	struct	color_type	color_table[];
extern		struct	channel_data	channel_table[20];
extern	const	struct	devote_type	devote_table[DEVOTE_CURRENT];

extern const struct flag_type   part_flags[];
extern const struct flag_type   ac_type[];
extern const struct flag_type	portal_flags[];
extern const struct flag_type	exit_flags[];
extern const struct flag_type   size_flags[];
extern const struct flag_type   position_flags[];
extern const struct flag_type   weapon_class[];
extern const struct flag_type   weapon_type2[];
extern const struct flag_type   furniture_flags[];
extern const struct flag_type	apply_types[];
extern const struct flag_type 	area_flags[];
extern const struct flag_type	room_flags[];
extern const struct flag_type	sector_type[];
extern const struct flag_type	type_flags[];
extern const struct flag_type	extra_flags[];
extern const struct flag_type	wear_flags[];
extern const struct flag_type	act_flags[];
extern const struct flag_type	affect_flags[];
extern const struct flag_type	apply_flags[];
extern const struct flag_type	wear_loc_strings[];
extern const struct flag_type	wear_loc_flags[];
extern const struct flag_type	sex_flags[];
extern const struct flag_type	container_flags[];
extern const struct bit_type	bitvector_type[];
extern const struct flag_type	door_resets[];

char	*flag_string	args ( ( const struct flag_type *flag_table, long bits ) );
long	flag_value	args ( ( const struct flag_type *flag_table, char *argument) );

/* chart.c */
void	load_charts	args( ( bool clear ) );
void	rank_chart	args( ( CHAR_DATA *ch, char *chart, int value ) );
void	unrank_charts	args( ( CHAR_DATA *ch ) );

/* clans.c */
void	update_clanlist	args( ( CHAR_DATA *ch, bool add ) );
void    check_clandeath args( ( CHAR_DATA *killer, CHAR_DATA *victim ) );
void    check_roster    args( ( CHAR_DATA *ch, bool remove ) );
bool   can_use_clan_mob	args( ( CHAR_DATA *ch, CHAR_DATA *mobile ) );

/* comm.c */
void	show_string	args( ( struct descriptor_data *d, char *input) );
void	close_socket	args( ( DESCRIPTOR_DATA *dclose ) );
void	write_to_buffer	args( ( DESCRIPTOR_DATA *d, const char *txt, int length ) );
void	send_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void    send_to_desc    args( ( const char *txt, DESCRIPTOR_DATA *d ) );
void	page_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void    printf_to_char  args( ( CHAR_DATA *ch, char *fmt, ...) ) __attribute__ ((format(printf, 2,3)));
void	act		args( ( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, int min_pos ) );
void	combat		args( ( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, long flag ) );
char	*colour		args( ( char type, CHAR_DATA *vch ) );
char    *colour2        args( ( char type ) );
void	send_to_char_bw	args( ( const char *txt, CHAR_DATA *ch ) );
void	page_to_char_bw	args( ( const char *txt, CHAR_DATA *ch ) );
bool    check_parse_name	args( ( char *name ) );

/* db.c */
char *	print_flags	args( ( int flag ));
void	boot_db		args( () );
void	area_update	args( ( void ) );
CD *	create_mobile	args( ( MOB_INDEX_DATA *pMobIndex ) );
void	clone_mobile	args( ( CHAR_DATA *parent, CHAR_DATA *clone) );
OD *	create_object	args( ( OBJ_INDEX_DATA *pObjIndex ) );
void	clone_object	args( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
char *	get_extra_descr	args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *	get_mob_index	args( ( int vnum ) );
OID *	get_obj_index	args( ( int vnum ) );
RID *	get_room_index	args( ( int vnum ) );
PC *    get_prog_index  args( ( int vnum, int type ) );
char	fread_letter	args( ( FILE *fp ) );
int	fread_number	args( ( FILE *fp ) );
long 	fread_flag	args( ( FILE *fp ) );
char *	fread_string	args( ( FILE *fp ) );
char *  fread_string_eol args(( FILE *fp ) );
void	fread_to_eol	args( ( FILE *fp ) );
char *	fread_word	args( ( FILE *fp ) );
long	flag_convert	args( ( char letter) );
void *	alloc_mem	args( ( int sMem ) );
void *	alloc_perm	args( ( int sMem ) );
void	free_mem	args( ( void *pMem, int sMem ) );
char *	str_dup		args( ( const char *str ) );
void	free_string	args( ( char *pstr ) );
int	number_range	args( ( int from, int to ) );
int	number_percent	args( ( void ) );
int	number_door	args( ( void ) );
int	number_bits	args( ( int width ) );
long     number_mm       args( ( void ) );
int	dice		args( ( int number, int size ) );
void	smash_tilde	args( ( char *str ) );
void	smash_dot_slash	args( ( char *str ) );
bool	str_cmp		args( ( const char *astr, const char *bstr ) );
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_prefix_c	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_infix_c	args( ( const char *astr, const char *bstr ) );
char *	str_replace	args( ( char *astr, char *bstr, char *cstr ) );
char *	str_replace_c	args( ( char *astr, char *bstr, char *cstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
char *	capitalize	args( ( const char *str ) );
void	append_file	args( ( CHAR_DATA *ch, char *file, char *str ) );
void	bug		args( ( const char *str, int param ) );
void	log_string	args( ( const char *str ) );
void	tail_chain	args( ( void ) );
void    free_runbuf     args( ( DESCRIPTOR_DATA *d ) );
void    load_rosters    args( ( void ) );
void    copyover_recover args ( ( void ) );
void	load_disabled	args( ( void ) );
void	save_disabled	args( ( void ) );
void	reset_area      args( ( AREA_DATA * pArea ) );
void	reset_room	args( ( ROOM_INDEX_DATA *pRoom, ARENA_DATA *match ) );

/* effect.c */
void	acid_effect	args( (CHAR_DATA *ch, void *vo, int level, int dam, int target) );
void	cold_effect	args( (CHAR_DATA *ch, void *vo, int level, int dam, int target) );
void	fire_effect	args( (CHAR_DATA *ch, void *vo, int level, int dam, int target) );
void	poison_effect	args( (CHAR_DATA *ch, void *vo, int level, int dam, int target) );
void	shock_effect	args( (CHAR_DATA *ch, void *vo, int level, int dam, int target) );

/* fight.c */
bool 	is_safe		args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_voodood	args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool 	is_safe_spell	args( (CHAR_DATA *ch, CHAR_DATA *victim, bool area ) );
void	violence_update	args( ( void ) );
void	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, bool check_area_attack ) );
bool	damage		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
	int dt, int class, bool show, bool secndary, char *special_string ) );
void	update_pos	args( ( CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_pk		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_pktimer	args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool wizshow ) );
bool	check_pktest	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	mobile_attack	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

/* handler.c */
sh_int	dam_type_lookup	args( ( char *argument ) );
AD  	*affect_find 	args( ( AFFECT_DATA *paf, sh_int sn ) );
void	affect_check	args( ( CHAR_DATA *ch, sh_int where, int vector ) );
sh_int	count_users	args( ( OBJ_DATA *obj ) );
void 	deduct_cost	args( ( CHAR_DATA *ch, int cost, int value ) );
void 	add_cost	args( ( CHAR_DATA *ch, int cost, int value ) );
void	affect_enchant	args( ( OBJ_DATA *obj ) );
sh_int	liq_lookup	args( ( const char *name ) );
sh_int	weapon_type	args( ( const char *name ) );
sh_int	attack_lookup	args( ( const char *name ) );
sh_int	race_lookup	args( ( const char *name ) );
sh_int	wiznet_lookup	args( ( const char *name ) );
sh_int	info_lookup	args( ( const char *name ) );
sh_int	combat_lookup	args( ( const char *name ) );
sh_int	class_lookup	args( ( const char *name ) );
bool	is_clan		args( ( CHAR_DATA *ch ) );
bool	is_pkill	args( ( CHAR_DATA *ch ) );
bool	is_same_clan	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
sh_int	get_skill	args( ( CHAR_DATA *ch, sh_int sn ) );
sh_int	get_weapon_sn	args( ( CHAR_DATA *ch, bool secondary ) );
sh_int	get_weapon_skill args(( CHAR_DATA *ch, sh_int sn ) );
sh_int	get_age		args( ( CHAR_DATA *ch ) );
sh_int	get_trust	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_stat	args( ( CHAR_DATA *ch, sh_int stat ) );
sh_int 	get_max_train	args( ( CHAR_DATA *ch, sh_int stat ) );
sh_int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
bool	is_name		args( ( char *str, char *namelist ) );
bool	is_exact_name	args( ( char *str, char *namelist ) );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_to_obj	args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove_obj args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_strip	args( ( CHAR_DATA *ch, sh_int sn ) );
bool	is_affected	args( ( CHAR_DATA *ch, sh_int sn ) );
bool	is_shielded	args( ( CHAR_DATA *ch, int sn ) );
void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
int	apply_ac	args( ( OBJ_DATA *obj, int iWear, int type ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
void	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
CD *	get_char_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument ) );
CD *	get_char_world	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_pc_world	args( ( CHAR_DATA *ch, char *argument ) );
AREA_DATA *area_lookup	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_exit	args( ( char *argument, OBJ_DATA *list ) );
OD *	get_obj_item	args( ( char *argument, OBJ_DATA *list ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_wear	args( ( CHAR_DATA *ch, char *argument, bool character ));
OD *	get_obj_here	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	create_money	args( ( int platinum, int gold, int silver ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
int	get_true_weight	args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	room_is_private	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_see_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *  pretitle        args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    setmin          args( ( int minlevel ) );
void    setmax          args( ( int maxlevel ) );
void    setwartype      args( ( int wartype ) );
void    auto_war        args( ( void ) );
void    end_war         args( ( void ) );
void    reset_affects   args( ( CHAR_DATA *ch ) );
void	spam_check	args( ( CHAR_DATA *ch ) );
void	racial_spells	args( ( CHAR_DATA *ch, bool cast ) );
void	send_sound_char	args( ( CHAR_DATA *ch, int volu, int limit, int pri, char *dir, char *file, long flag ) );
void	send_sound_room	args( ( ROOM_INDEX_DATA *room, int volu, int limit, int pri, char *dir, char *file, long flag ) );
void	send_sound_area	args( ( AREA_DATA *area, int volu, int limit, int pri, char *dir, char *file, long flag ) );
void	music		args( ( CHAR_DATA *ch, int volu, int limit, char *dir, char *file, long flag ) );
bool	check_builder	args( ( CHAR_DATA *ch, AREA_DATA *pArea, bool show ) );
char	*show_dam_mods	args( ( sh_int damage_mod[DAM_MAX] ) );
char	*parse_time	args( ( int time ) );
bool	check_kill_steal args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool show ) );
char *	show_condition	args( ( CHAR_DATA *ch, CHAR_DATA *victim, sh_int type ) );
char *	return_classes	args( ( bool class[maxClass] ) );


/* interp.c */
void	interpret	args( ( CHAR_DATA *ch, char *argument ) );
bool	is_number	args( ( char *arg ) );
int	number_argument	args( ( char *argument, char *arg ) );
int	mult_argument	args( ( char *argument, char *arg) );
char *	one_argument	args( ( char *argument, char *arg_first ) );
int	find_color	args( ( char *argument ) );
char *	color_string	args( ( char *code ) );

/* lookup.c */
int     clan_lookup     args( (const char *name) );

/* magic.c */
sh_int	find_spell	args( ( CHAR_DATA *ch, const char *name) );
int 	mana_cost 	args( ( CHAR_DATA *ch, int min_mana, int level ) );
sh_int	skill_lookup	args( ( const char *name ) );
bool	saves_spell	args( ( int level, CHAR_DATA *ch, CHAR_DATA *victim, int dam_type ) );
void	obj_cast_spell	args( ( int sn, int level, CHAR_DATA *ch,
				    CHAR_DATA *victim, OBJ_DATA *obj, char *argument ) );

/* mob_prog.c */
void	program_flow	args( ( sh_int vnum, char *source, CHAR_DATA *mob, 
				OBJ_DATA *obj, ROOM_INDEX_DATA *room,
				CHAR_DATA *ch, const void *arg1,
				const void *arg2 ) );
void	p_act_trigger	args( ( char *argument, CHAR_DATA *mob, 
				OBJ_DATA *obj, ROOM_INDEX_DATA *room,
				CHAR_DATA *ch, const void *arg1,
				const void *arg2, int type ) );
bool	p_percent_trigger args( ( CHAR_DATA *mob, OBJ_DATA *obj,
				ROOM_INDEX_DATA *room, CHAR_DATA *ch, 
				const void *arg1, const void *arg2, int type ) );
void	p_bribe_trigger  args( ( CHAR_DATA *mob, CHAR_DATA *ch, int amount ) );
bool	p_exit_trigger   args( ( CHAR_DATA *ch, int dir, int type ) );
void	p_give_trigger   args( ( CHAR_DATA *mob, OBJ_DATA *obj, 
				ROOM_INDEX_DATA *room, CHAR_DATA *ch,
				OBJ_DATA *dropped, int type ) );
void 	p_greet_trigger  args( ( CHAR_DATA *ch, int type ) );
void	p_hprct_trigger  args( ( CHAR_DATA *mob, CHAR_DATA *ch ) );

/* mob_cmds.c */
void	mob_interpret	args( ( CHAR_DATA *ch, char *argument ) );
void	obj_interpret	args( ( OBJ_DATA *obj, char *argument ) );
void	room_interpret	args( ( ROOM_INDEX_DATA *room, char *argument ) );
MEM_DATA *get_mem_data	args( ( CHAR_DATA *ch, CHAR_DATA *target ) );
void	mob_remember	args( ( CHAR_DATA *ch, CHAR_DATA *target, int reaction) );
void	mem_fade	args( ( CHAR_DATA *ch ) );
void 	mob_forget	args( ( CHAR_DATA *ch, MEM_DATA *memory ) );

/* note.c */
void    expire_notes    args( ( void ) );

/* olc.c */
bool	run_olc_editor	args( ( DESCRIPTOR_DATA *d ) );
AREA_DATA *get_area_data args( ( int vnum ) );

/* save.c */
char	*initial	args( ( const char *str ) );
void	save_char_obj	args( ( CHAR_DATA *ch, int type ) );
bool	load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name, bool arena_load, bool reload ) );
void    save_special_items args( () );
void	load_special_items args( () );
void 	do_devote_assign args( ( CHAR_DATA *ch ) );
int	get_percentage_bonus args( ( CHAR_DATA *ch, float value ) );

/* skills.c */
bool 	parse_gen_groups args( ( CHAR_DATA *ch,char *argument ) );
void 	list_group_costs args( ( CHAR_DATA *ch ) );
void    list_group_known args( ( CHAR_DATA *ch ) );
long 	exp_per_level	args( ( CHAR_DATA *ch, int points ) );
void 	check_improve	args( ( CHAR_DATA *ch, int sn, bool success,
				    int multiplier ) );
int 	group_lookup	args( (const char *name) );
void	gn_add		args( ( CHAR_DATA *ch, int gn) );
void 	gn_remove	args( ( CHAR_DATA *ch, int gn) );
void 	group_add	args( ( CHAR_DATA *ch, const char *name, bool deduct) );
void	group_remove	args( ( CHAR_DATA *ch, const char *name) );
int     spell_avail     args( ( CHAR_DATA *ch, const char *name) );
bool	cost_of_skill	args( ( CHAR_DATA *ch, int sn ) );

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );
char *	spec_name	args( ( SPEC_FUN *function ) );

/* string.c */
void    string_append   args( ( CHAR_DATA *ch, char **pString ) );
void    string_add      args( ( CHAR_DATA *ch, char *argument ) );
char *  format_string   args( ( char *oldstring, int width ) );
int 	strlen_color	args( ( char * argument ) );
int	actlen_color	args( ( const char * argument ) );
char *	end_string	args( ( const char *txt, int length ) );
char *	begin_string	args( ( const char *txt, int length ) );
char *	center_string	args( ( const char *txt, int length ) );
char *	strip_color	args( ( const char *argument ) );
char *  strip_spaces	args( ( const char *argument ) );
char *	strip_caps	args( ( const char *argument ) );
char *	length_argument	args( ( char *argument, char *arg_first, int length ) );

/* teleport.c */
RID *	room_by_name	args( ( char *target, int level, bool error) );

/* update.c */
void	advance_level	args( ( CHAR_DATA *ch, bool show ) );
void	gain_exp	args( ( CHAR_DATA *ch, int gain ) );
void	gain_condition	args( ( CHAR_DATA *ch, int iCond, int value ) );
void	update_handler	args( ( bool forced ) );

/* wizlist.c */
void	update_wizlist	args( ( CHAR_DATA *ch, int level ) );

#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	AD

#define	AREA_NO_RUN	A
#define	AREA_CHANGED	B
#define	AREA_UNLINKED	C
#define	AREA_SPECIAL	D
#define AREA_NO_QUEST	E

#define NO_FLAG -99	/* Must not be used in flags or stats. */
