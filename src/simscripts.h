// Simple Scripts for Diku
// by Locke (herb.gilliland-@-gmail.com)

#define SIMSCRIPT_MAX_VARNAME_LENGTH 32
#define SIMSCRIPT_MAX_GOTOS 500
#define SIMSCRIPT_MAX_WAIT (60*60*12)  // 12 real hours

#define SIMSCRIPT_VARCHARS "=+-<>?*!^)([]/"


#define SIMSCRIPT_ROOM 0
#define SIMSCRIPT_OBJ 1
#define SIMSCRIPT_MOB 2

#define SIMSCRIPT_INACTIVE 1
#define SIMSCRIPT_CONTINUE_WHEN_OUT_OF_RANGE 2
#define SIMSCRIPT_MAKE_INACTIVE_WHEN_DONE 3
#define SIMSCRIPT_DELETE_WHEN_DONE 4

#define SIMSCRIPT_TYPE_BUILD 0     // special type for build scripts that are executed by other scripts
#define SIMSCRIPT_TYPE_BORN 1      // called when the object is created or the mobile is born
#define SIMSCRIPT_TYPE_DIES 2      // called when the object or mobile is destroyed
#define SIMSCRIPT_TYPE_SPEAKS 3    // called when someone speaks a matching keyword
#define SIMSCRIPT_TYPE_NEAR 4      // called when someone enters the same room (if an object is on the ground)
#define SIMSCRIPT_TYPE_SPELL 5     // called when a spell is cast or being cast
#define SIMSCRIPT_TYPE_SKILL 6     // called when a skill is performed or being performed
#define SIMSCRIPT_TYPE_ORDER 7     // when ordered using the order command
#define SIMSCRIPT_TYPE_COMMAND 8   // when a command matches the keywords, sets the %keywordN% variables
#define SIMSCRIPT_TYPE_COMBAT 9    // continuously re-executed during combat
#define SIMSCRIPT_TYPE_VICTORY 10  // called when combat ends
#define SIMSCRIPT_TYPE_GIVEN 11    // called when given something, or something is placed inside
#define SIMSCRIPT_TYPE_RECIPE 12   // like spells but requires reagents
#define SIMSCRIPT_TYPE_LOADED 13   // called when the owner is loaded from the player file (for pets and such)

#define SCRIPT_DIR  "../area/scripts/"

SIMGLUE *new_glue args( ( void ) );

bool trigger_mob      args( ( CHAR_DATA *mob, CHAR_DATA *plr, int type, char *arg ) );
bool trigger_room     args( ( ROOM_INDEX_DATA *room, CHAR_DATA *plr, int type, char *arg ) );
bool trigger_obj      args( ( OBJ_DATA *obj, CHAR_DATA *plr, int type, char *arg ) );

void continue_executions  args( ( void ) ); // called once per loop
void save_scripts         args( ( void ) ); // used by db.c, save.c
void load_scripts         args( ( void ) );

void save_glue     args( ( FILE *fp, SIMGLUE *list ) );
void load_glue     args( ( SIMGLUE **head, char *filestring ) );
void dispose_glue  args( ( SIMGLUE **head ) );

DECLARE_DO_FUN( do_scriptedit );

// Used only in simscripts.c
// Replaces script variables with their mud equivalents when passed as arguments to a mud command.
// When referring to objects and mobiles this is imperfect, so it only works on players and members
// as a direct reference.  Otherwise, %object% will parse out to the first keyword (which works for
// something in the inventory of a script owner.  %mobile% is even more imperfect. 
// Avoid use of %object% and %mobile% when executing mud commands (wierdness will occur).
// In the case of %room%, the entire room title is provided.
// If you are debugging such a script and the variable appears unparsed, this means you've referred
// to a non-existant thing and probably indicates an issue with your script.
// NOTE: RoT uses mobindex->player_name, not mobindex->name, so you might have to modify this
#define SIM_REP_VAR_MUDCMDARG(arg2) \
    if ( !str_cmp( arg2, "%player%" ) && e->player ) { sprintf( arg2, "%s", e->player->name ); } else \
    if ( !str_cmp( arg2, "%member%" ) && e->member ) { sprintf( arg2, "%s", e->member->name ); } else \
    if ( !str_cmp( arg2, "%object%" ) && e->object ) { \
     one_argument( e->object->pIndexData->name, arg2 ); \
    } else if ( !str_cmp( arg2, "%mobile%" ) && e->mobile ) { \
     one_argument( e->mobile->pIndexData->player_name, arg2 ); \
    } else if ( !str_cmp( arg2, "%room%" ) && e->last ) { \
     sprintf( arg2, "%s", e->last->name ); \
    }

extern SIMSCRIPT *simscripts;   // master list of scripts
extern SIMVAR *simglobals;   // master list of script globals (accessible by all scripts)
extern SIMEXEC *simexecutions;  // master list of executing script instances
