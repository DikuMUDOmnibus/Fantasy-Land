/*
 * Simple Scripts for Diku (SimScripts)
 * Author: Herb Gilliland ("Locke" of The Isles)
 * Copyright: (c) 2012
 * License: AL 2.0 ( "Artistic License 2.0" )
 */

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "simscripts.h"
#include <malloc.h>

extern int top_room;

extern char *  const   day_name        [];
extern char *  const   month_name      [];

void raw_kill args( ( CHAR_DATA *victim, CHAR_DATA *killer ) );                    /* fight.c */
void wear_obj args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, bool show ) );  /* act_obj.c */

SIMSCRIPT *simscripts=NULL;   // master list of scripts
SIMVAR *simglobals=NULL;
SIMEXEC *simexecutions=NULL;  // master list of executing script instances

/*
 * Local macros
 */


/*
 * Local functions
 */

void execute_build_script  args( ( SIMEXEC *spawner, int vnum ) );
SIMSCRIPT *get_build_script  args( ( int vnum ) ) ;
SIMSCRIPT *get_build_script_by_name  args( ( char *name ) );
void parse_variables  args( ( SIMEXEC *e, char *inout ) );
SIMSCRIPT *new_script args( ( void ) );
SIMEXEC *execute_script  args( ( CHAR_DATA *caller, SIMSCRIPT *spawner, void *owner, int type ) );
void execute_build_script  args( ( SIMEXEC *spawner, int vnum ) );
SIMSCRIPT *get_build_script  args( ( int vnum ) ) ;
SIMSCRIPT *get_build_script_by_name  args( ( char *name ) );
int *string_to_array args( ( char *in, int *count ) );
char *one_line args( ( char *in, char *out ) );
OBJ_DATA *find_obj args( ( OBJ_DATA *list, int vnum ) );
OBJ_DATA *find_in_obj args( (  OBJ_DATA *list, int vnum ) );
OBJ_DATA *find_obj_name args( ( OBJ_DATA *list, char *name ) );
OBJ_DATA *find_in_obj_name args( (  OBJ_DATA *list, char *name ) );
SIMSCRIPT *find_sim_script args( ( int vnum ) );
void sim_script_goto args( ( SIMEXEC *e, char *label ) );
void continue_execution args( ( SIMEXEC *e ) );
int group_size args( ( CHAR_DATA *player ) );
CHAR_DATA *group_random_member args( ( CHAR_DATA *player ) );
char *string_copy_to args( ( char *in, char *value ) );
char *script_type_to_string args( ( int t ) );
int script_string_to_type   args( ( char *in ) );

char *string_copy_to args( ( char *in, char *value ) ) {
 char *q=in;
 char *p=value;
 while ( *p != '\0' ) {
  *q=*p;
  p++;
  q++;
 }
 return q;
}

void parse_variables( SIMEXEC *e, char *inout ) {
 char temp[MSL];
 sprintf( temp, "%s", inout );
 char parsed[MSL];
 char *p=temp;
 char *q=parsed;
 while ( *p != '\0' ) {
  if ( *p == '%' ) {
   char vname[MSL];
   char *r=vname;
   p++;
   while ( *p != '\0' && *p != '%' ) { *r=*p; p++; }
   *r='\0';
   if ( *p != '%' ) p++;
   if ( strlen(r) <= SIMSCRIPT_MAX_VARNAME_LENGTH ) {
    if ( !str_cmp(r,"object" ) ) {
     if ( e->object ) {
      q=string_copy_to( q, e->object->pIndexData->short_descr );
     }
    } else if ( !str_cmp(r, "mobile" ) ) {
     if ( e->mobile ) {
      q=string_copy_to( q, IS_NPC(e->mobile) ? e->mobile->pIndexData->short_descr : e->mobile->name );
     }
    } else if ( !str_cmp(r, "mobile-himher" ) ) {
     if ( e->mobile ) {
      q=string_copy_to( q, e->mobile->sex == SEX_MALE ? "him" : e->mobile->sex == SEX_FEMALE ? "her" : "it" );
     }
    } else if ( !str_cmp(r, "mobile-heshe" ) ) {
     if ( e->mobile ) {
      q=string_copy_to( q, e->mobile->sex == SEX_MALE ? "he" : e->mobile->sex == SEX_FEMALE ? "she" : "it" );
     }
    } else if ( !str_cmp(r, "mobile-hisher" ) ) {
     if ( e->mobile ) {
      q=string_copy_to( q, e->mobile->sex == SEX_MALE ? "his" : e->mobile->sex == SEX_FEMALE ? "her" : "its" );
     }
    } else if ( !str_cmp(r, "player" ) ) {
     if ( e->player ) {
      q=string_copy_to( q, e->player->name );
     }
    } else if ( !str_cmp(r, "player-himher" ) ) {
     if ( e->player ) {
      q=string_copy_to( q, e->player->sex == SEX_MALE ? "him" : e->player->sex == SEX_FEMALE ? "her" : "it" );
     }
    } else if ( !str_cmp(r, "player-heshe" ) ) {
     if ( e->player ) {
      q=string_copy_to( q, e->player->sex == SEX_MALE ? "he" : e->player->sex == SEX_FEMALE ? "she" : "it" );
     }
    } else if ( !str_cmp(r, "player-hisher" ) ) {
     if ( e->player ) {
      q=string_copy_to( q, e->player->sex == SEX_MALE ? "his" : e->player->sex == SEX_FEMALE ? "her" : "its" );
     }
    } else if ( !str_cmp(r, "member" ) ) {
     if ( e->member ) {
      q=string_copy_to( q, e->player->name );
     }
    } else if ( !str_cmp(r, "member-himher" ) ) {
     if ( e->member ) {
      q=string_copy_to( q, e->member->sex == SEX_MALE ? "him" : e->member->sex == SEX_FEMALE ? "her" : "it" );
     }
    } else if ( !str_cmp(r, "member-heshe" ) ) {
     if ( e->member ) {
      q=string_copy_to( q, e->member->sex == SEX_MALE ? "he" : e->member->sex == SEX_FEMALE ? "she" : "it" );
     }
    } else if ( !str_cmp(r, "member-hisher" ) ) {
     if ( e->member ) {
      q=string_copy_to( q, e->member->sex == SEX_MALE ? "his" : e->member->sex == SEX_FEMALE ? "her" : "its" );
     }
    } else if ( !str_cmp(r, "self" ) || !str_cmp(r,"owner") ) {
     if ( e->o ) {
      q=string_copy_to( q, e->o->pIndexData->short_descr );
     } else if ( e->r ) {
      q=string_copy_to( q, e->r->name );
     } else if ( e->m ) {
      q=string_copy_to( q, e->m->pIndexData->player_name );
     }
    } else if ( !str_cmp(r, "self-himher" ) || !str_cmp(r,"owner-himher") ) {
     if ( e->m ) {
      q=string_copy_to( q, e->m->sex == SEX_MALE ? "him" : e->member->sex == SEX_FEMALE ? "her" : "it" );
     } else q=string_copy_to( q, "it" );
    } else if ( !str_cmp(r, "self-heshe" ) || !str_cmp(r,"owner-heshe") ) {
     if ( e->m ) {
      q=string_copy_to( q, e->m->sex == SEX_MALE ? "he" : e->member->sex == SEX_FEMALE ? "she" : "it" );
     } else q=string_copy_to( q, "it" );
    } else if ( !str_cmp(r, "self-hisher" ) || !str_cmp(r,"owner-hisher") ) {
     if ( e->m ) {
      q=string_copy_to( q, e->m->sex == SEX_MALE ? "his" : e->member->sex == SEX_FEMALE ? "her" : "its" );
     } else q=string_copy_to( q, "it" );
    } else if ( !str_cmp(r, "here" ) ) {
     if ( e->o ) {
      if ( e->o->in_room ) q=string_copy_to( q, e->o->in_room->name );
      else if ( e->o->carried_by ) q=string_copy_to( q, IS_NPC(e->o->carried_by) ? e->o->carried_by->pIndexData->short_descr : e->o->carried_by->name );
      else if ( e->o->in_obj ) q=string_copy_to( q, e->o->in_obj->pIndexData->short_descr );
     } else if ( e->r ) q=string_copy_to( q, e->r->name );
     else if ( e->m ) q=string_copy_to( q, e->m->in_room->name );
    } else if ( !str_cmp(r, "room" ) ) {
     if ( e->last ) q=string_copy_to( q, e->last->name );
    } else if ( !str_cmp(r, "time" ) ) {
     char year[MSL];
     sprintf( year, "%d o'clock %s",
      time_info.hour == 0 ? 12 : time_info.hour > 12 ? time_info.hour-12 : time_info.hour,
      time_info.hour < 12 ? "am" : "pm"
     );
     q=string_copy_to(q,year);
    } else if ( !str_cmp(r, "date" ) ) {
     char year[MSL];
     sprintf( year, "%d%s of %s, %d", time_info.day+1,
      time_info.day%10 == 1 ? "st" : (time_info.day+1)%10 ==2 ? "nd" : (time_info.day+1)%10 == 3 ? "rd" : "th",
      month_name[time_info.month], time_info.year );
     q=string_copy_to(q,year);
    } else if ( !str_cmp(r, "weekday" ) ) {
     q=string_copy_to(q,day_name[(time_info.day+1) % 7]);
    } else if ( !str_cmp(r, "day" ) ) {
     char year[MSL];
     sprintf( year, "%d", time_info.day+1 );
     q=string_copy_to(q,year);
    } else if ( !str_cmp(r, "month" ) ) {
     q=string_copy_to(q,month_name[time_info.month]);
    } else if ( !str_cmp(r, "year" ) ) {
     char year[MSL];
     sprintf( year, "%d", time_info.year );
     q=string_copy_to(q,year);
    } else if ( !str_cmp(r, "ampm" ) ) {
     if ( time_info.hour < 12 ) q=string_copy_to( q, "am" ); else q=string_copy_to( q, "pm" );
    } else if ( !str_cmp(r, "hour" ) ) {
     char hour[MSL];
     sprintf( hour, "%d", time_info.hour == 0 ? 12 : time_info.hour > 12 ? time_info.hour-12 : time_info.hour );
     q=string_copy_to( q, hour );
    } else if ( !str_cmp(r, "sky" ) ) {
     switch ( weather_info.sky ) {
         case SKY_CLOUDY: q=string_copy_to( q, "cloudy" ); break;
      case SKY_CLOUDLESS: q=string_copy_to( q, "clear"  ); break;
        case SKY_RAINING: q=string_copy_to( q, "rainy"  ); break;
      case SKY_LIGHTNING: q=string_copy_to( q, "stormy" ); break;
                 default: q=string_copy_to( q, "unclear" ); break;
     }
    } else if ( !str_cmp(r, "sun" ) ) {
    } else { /* search for a variable */ }
   } else {
    char buf[MSL];
    sprintf(buf, "Warning: %d length variable name found in script %d, not parsed", strlen(r), e->script->vnum );
    bug(buf,0);
   }
  } else {
   *q=*p;
   q++;
   p++;
  }
 }
 *q='\0';
 sprintf( inout, "%s", parsed );
}

int group_size( CHAR_DATA *player ) {
 if ( IS_NPC(player) ) return 0;
 int groupsize=1;
 CHAR_DATA *a;
 for ( a=char_list; a; a=a->next ) if ( !IS_NPC(a) && a!=player && in_group(a,player) ) groupsize++;
 return groupsize;
}

CHAR_DATA *group_random_member( CHAR_DATA *player ) {
 if ( IS_NPC(player) ) return player;
 int groupsize=group_size(player);
 if ( groupsize==1 ) return player;
 int select=number_range(0,groupsize);
 CHAR_DATA *a;
 for ( a=char_list; a; a=a->next) if ( !IS_NPC(a) && a!=player && in_group(a,player) ) if ( --select == 0 ) return a;
 // otherwise just pick the first available one who isn't the player
 for ( a=char_list; a; a=a->next) if ( !IS_NPC(a) && a!=player && in_group(a,player) ) return a;
 // egads now what
 return player;
}

SIMSCRIPT *find_sim_script( int vnum ) {
 SIMSCRIPT *s;
 for ( s=simscripts; s; s=s->next ) if ( s->vnum == vnum ) return s;
 return NULL;
}

OBJ_DATA *find_obj( OBJ_DATA *list, int vnum ) {
 OBJ_DATA *o;
 for ( o=list; o; o=o->next_content ) if ( o->pIndexData->vnum == vnum ) return o;
 return NULL;
}

OBJ_DATA *find_in_obj( OBJ_DATA *list, int vnum ) {
 OBJ_DATA *o,*p;
 for ( o=list; o; o=o->next_content ) if ( (p=find_obj(o->contains,vnum)) ) return p;
 return NULL;
}

OBJ_DATA *find_obj_name( OBJ_DATA *list, char *name ) {
 OBJ_DATA *o;
 for ( o=list; o; o=o->next_content ) if ( !str_cmp( o->pIndexData->short_descr, name ) ) return o;
 for ( o=list; o; o=o->next_content ) if ( is_name( o->pIndexData->name, name ) ) return o;
 return NULL;
}

OBJ_DATA *find_in_obj_name( OBJ_DATA *list, char *name ) {
 OBJ_DATA *o,*p;
 for ( o=list; o; o=o->next_content ) if ( (p=find_obj_name(o->contains,name)) ) return p;
 return NULL;
}

int *string_to_array( char *in, int *count ) {
 char arg[MSL];
 char *p;
 int *list=NULL;
 (*count)=0;
 p=in;
 arg[0]='\0';
 while ( *p != '\0' ) {
  p=one_argument( p, arg );
  if ( arg[0] != '\0' ) {
   int value=atoi(arg);
   if ( list == NULL ) {
    list=malloc( sizeof(int) );
    list[0]=value;
    (*count)=1;
   } else {
    int *newlist = malloc( sizeof(int) * ((*count) + 1 ) );
    int i=0;
    for ( ; i< (*count); i++ ) newlist[i]=list[i];
    newlist[(*count)]=value;
    (*count)++;
   }
  }
 }
 return list; // make sure to delete later
}

char *one_line( char *in, char *out ) {
 char *p=in;
 char *q=out;
 while ( *p != '\0' ) {
  if ( *p == '\r' ) {
   p++;
   continue;
  }
  if ( *p == '\n' ) {
   p++;
   *q='\0';
   break;
  }
  *q=*p;
  q++;
  p++;
 }
 return p;
}


SIMSCRIPT *new_script( void ) {
 SIMSCRIPT *script=malloc( sizeof(SIMSCRIPT) );
 script->next=NULL;
 script->vnum=0;
 script->name=str_dup("");
 script->keywords=str_dup("");
 script->script=str_dup("");
 script->flags=0;
 script->type=1;
 script->pace=1;
 script->max=1;
 script->concurrent=0;
 script->ritual=0;
 script->reagents=str_dup("");
 script->quantities=str_dup("");
 return script;
}

SIMGLUE *new_glue( void ) {
 SIMGLUE *glue=malloc( sizeof(SIMGLUE) );
 glue->next=NULL;
 glue->vnum=0;
 return glue;
}

SIMEXEC *execute_script( CHAR_DATA *caller, SIMSCRIPT *spawner, void *owner, int type ) {
 SIMEXEC *e=malloc( sizeof(SIMEXEC) );
 e->gotos=0;
 e->paused=0;
 e->flags=0;
 e->o=NULL;
 e->r=NULL;
 e->m=NULL;
 e->object=NULL;
 e->player=caller;
 e->member=NULL;
 e->mobile=NULL;
 e->last=NULL;
 e->script=spawner;
 e->script->concurrent++;
 e->pace=spawner->pace;
 e->execution=spawner->script;
 e->next=simexecutions;
 simexecutions=e;
 switch ( type ) {
  case SIMSCRIPT_OBJ:  e->o=(OBJ_DATA *)owner;
  case SIMSCRIPT_ROOM: e->r=(ROOM_INDEX_DATA *)owner;
  case SIMSCRIPT_MOB:  e->m=(CHAR_DATA *)owner;
 }
 return e;
}

int count_executions( SIMSCRIPT *s ) {
 int count=0;
 SIMEXEC *e;
 for ( e=simexecutions; e; e=e->next ) if ( e->script == s ) count++;
 return count;
}

bool trigger_mob( CHAR_DATA *mob, CHAR_DATA *plr, int type, char *arg ) {
 return FALSE;
}

bool trigger_room( ROOM_INDEX_DATA *room, CHAR_DATA *plr, int type, char *arg ) {
 return FALSE;
}

bool trigger_obj( OBJ_DATA *obj, CHAR_DATA *plr, int type, char *arg ) {
 return FALSE;
}

void sim_script_goto( SIMEXEC *e, char *label ) {
 if ( *label == '\0' ) return;
 if ( e->gotos > SIMSCRIPT_MAX_GOTOS ) {
  e->execution=NULL;
  return;
 }
 char *script=e->script->script;
 char buf[MSL];
 while ( *script != '\0' ) {
  script=one_line(script,buf);
  char arg[MSL];
  char *w=buf;
  w=one_argument(w,arg);
  if ( *arg != '\0' ) {
   char ender=arg[strlen(arg)-1];
   if ( ender == ':' ) {
    arg[strlen(arg)-1]='\0';
    if ( !str_cmp( arg, label ) ) {
     e->execution=script;
     e->gotos++;
     break;
    }
   }
  }
 }
}


void modify_variables( SIMEXEC *e, SIMVAR *v, char *arg2, char *arg3, char *arg4, char *arg5 ) {
}


void continue_execution( SIMEXEC *e ) {
 static char line[MSL];
 e->execution=one_line(e->execution,line);
 while ( strlen(line) == 0
      && ( e->execution != NULL
        || *e->execution != '\0' ) )
  e->execution=one_line(e->execution,line);
 if ( strlen(line) == 0
   && ( *e->execution == '\0'
     || e->execution == '\0' ) )
  return;
 char *p=line;
 static char arg1[MSL];
 static char arg2[MSL];
 static char arg3[MSL];
 static char arg4[MSL];
 static char arg5[MSL];
 static char arg6[MSL];

 p=one_argument(p,arg1);
 p=one_argument(p,arg2);
 p=one_argument(p,arg3);
 p=one_argument(p,arg4);
 p=one_argument(p,arg5);
 p=one_argument(p,arg6);

 char first=arg1[0];
 char last=arg1[strlen(arg1)-1];

 if ( first == '#' ) {
 } else if ( first == '_' ) {
  if ( char_isof( arg1[1], SIMSCRIPT_VARCHARS ) ) {
  }
 } else if ( first == '@' ) {
 } else if ( char_isof( first, SIMSCRIPT_VARCHARS ) ) {
 } else if ( last == ':' ) {
 } else if ( !str_cmp( arg1, "end" ) ) {
  e->execution=NULL;
 } else if ( !str_cmp( arg1, "die" ) || !str_cmp( arg1, "death" ) ) {
  e->execution=NULL;
  if ( e->o ) { extract_obj(e->o); e->o=NULL; }
  if ( e->m ) { raw_kill(e->m,NULL); e->m=NULL; }
  if ( e->r ) { e->r=NULL; } // what would this do? bleah
 } else if ( !str_cmp( arg1, "goto" ) ) {
  sim_script_goto( e, arg2 );
 } else if ( !str_cmp( arg1, "walkto" ) ) {
 } else if ( !str_cmp( arg1, "wait" ) ) {
  int wait=atoi(arg2);
  if ( wait < 1 ) wait=1;
  if ( wait > SIMSCRIPT_MAX_WAIT ) wait=SIMSCRIPT_MAX_WAIT;
  e->paused=wait;
 } else if ( !str_cmp( arg1, "randomwait" ) ) {
  int lo=atoi(arg2);
  int hi=atoi(arg3);
  if ( lo < hi ) e->paused=number_range(lo,hi);
  else if ( hi > lo ) e->paused=number_range(hi,lo);
  else e->paused=lo;
 } else if ( !str_cmp( arg1, "pace" ) ) {
  e->pace=atoi(arg2);
  if ( e->pace < 1 ) e->pace=1;
 } else if ( !str_cmp( arg1, "affect" ) ) {
 } else if ( !str_cmp( arg1, "affectgroup" ) ) {
 } else if ( !str_cmp( arg1, "modify" ) ) {
 } else if ( !str_cmp( arg1, "equip" ) ) {
 } else if ( !str_cmp( arg1, "breed" ) ) {
 } else if ( !str_cmp( arg1, "hireling" ) ) {
 } else if ( !str_cmp( arg1, "dispense" ) ) {
 } else if ( !str_cmp( arg1, "give" ) ) {
 } else if ( !str_cmp( arg1, "has" ) ) {
 } else if ( !str_cmp( arg1, "inside" ) && arg2[0]!='\0' ) {
  int vnum=atoi(arg2);
  if ( arg2[0]=='%' ) {
   if ( !str_cmp( arg2, "%object%" ) && e->object ) { vnum=e->object->pIndexData->vnum; }
   else if ( !str_cmp( arg2, "%self%" ) && e->o ) { vnum=e->o->pIndexData->vnum; }
  }
  OBJ_INDEX_DATA *index=get_obj_index(vnum);
  if ( !index ) return;
  int quantity=arg3[0]=='\0' || !is_number(arg3) ? 1 : atoi(arg3);
  if ( quantity <= 1 ) quantity =1;
  if ( quantity >= 100 ) quantity =100;
  if ( e->o ) {
   int i=0;
   for ( ; i<quantity; i++ ) obj_to_obj( e->object=create_object( index ), e->o );
  } else if ( e->m ) {
   if ( !is_number( arg3 ) ) {
    //int wear = flag_value( wear_flags, argument ) );
    //if ( wear==NO_FLAG ) wear=WEAR_NONE;
    obj_to_char( e->object=create_object( index ), e->m );
    //if ( wear != WEAR_NONE ) {
    if ( !str_cmp( arg3, "wear" ) || !str_cmp( arg3, "equip" ) )
     wear_obj( e->m, e->object, TRUE, TRUE );
   }
  } else if ( e->r ) {
   int i=0;
   for ( ; i<quantity; i++ ) obj_to_room( e->object=create_object( index ), e->r );
  }
 } else if ( !str_cmp( arg1, "remove" ) ) {
  int vnum=atoi(arg2);
  if ( arg2[0]=='%' ) {
   if ( !str_cmp( arg2, "%object%" ) && e->object ) { vnum=e->object->pIndexData->vnum; }
   else if ( !str_cmp( arg2, "%self%" ) && e->o ) { vnum=e->o->pIndexData->vnum; }
  }
  OBJ_INDEX_DATA *index=get_obj_index(vnum);
  if ( !index ) return;
  OBJ_DATA *o=NULL;
  if ( !str_cmp( arg3, "self" ) ) {
   if ( e->o ) o=find_obj( e->o->contains, vnum );
   else if ( e->m ) o=find_obj( e->m->carrying, vnum );
   else if ( e->r ) o=find_obj( e->r->contents, vnum );
  } else if ( !str_cmp( arg3, "member" ) ) {
   if ( e->member ) o=find_obj( e->member->carrying, vnum );
  } else if ( e->player ) {
   o=find_obj( e->player->carrying, vnum );
  }
  if ( o ) extract_obj(o);
 } else if ( !str_cmp( arg1, "remall" ) ) {
  if ( arg2[0] == '\0' ) {
   if ( e->o ) {
    while ( e->o->contains ) extract_obj( e->o->contains );
   } else if ( e->r ) {
    while ( e->r->contents ) extract_obj( e->r->contents );
   } else if ( e->m ) {
    while ( e->m->carrying ) extract_obj( e->m->carrying );
   }
   return;
  }
  int vnum=atoi(arg2);
  if ( arg2[0]=='%' ) {
   if ( !str_cmp( arg2, "%object%" ) && e->object ) { vnum=e->object->pIndexData->vnum; }
   else if ( !str_cmp( arg2, "%self%" ) && e->o ) { vnum=e->o->pIndexData->vnum; }
  }
  OBJ_INDEX_DATA *index=get_obj_index(vnum);
  if ( !index ) return;
  OBJ_DATA *o=NULL;
  int removed=0;
  if ( !str_cmp( arg3, "self" ) ) {
   if ( e->o ) while ( (o=find_obj(e->o->contains, vnum)) ) { extract_obj(o); removed++; }
   else if ( e->m ) o=find_obj( e->m->carrying, vnum );
   else if ( e->r ) o=find_obj( e->r->contents, vnum );
  } else if ( !str_cmp( arg3, "member" ) ) {
   if ( e->member ) while ( (o=find_obj( e->member->carrying, vnum)) ) { extract_obj(o); removed++; }
  } else if ( e->player ) {
   while ( (o=find_obj(e->player->carrying,vnum)) ) { extract_obj(o); removed++; }
  }
 } else if ( !str_cmp( arg1, "pay" ) ) {
 } else if ( !str_cmp( arg1, "charge" ) ) {
 } else if ( !str_cmp( arg1, "attack" ) ) {
 } else if ( !str_cmp( arg1, "bomb" ) ) {
 } else if ( !str_cmp( arg1, "hurt" ) ) {
 } else if ( !str_cmp( arg1, "hurtgroup" ) ) {
 } else if ( !str_cmp( arg1, "wander" ) ) {
 } else if ( !str_cmp( arg1, "link" ) ) {
 } else if ( !str_cmp( arg1, "unlink" ) ) {
 } else if ( !str_cmp( arg1, "jump" ) ) {
  ROOM_INDEX_DATA *land=NULL;
  if ( e->r ) return;
  if ( arg2[0] == '\0' ) {
   int vstart=number_range(0,top_room);
   while ( (land=get_room_index(vstart)) == NULL ) {
    vstart++;
    if ( vstart >= top_room ) vstart=0;
   }
  } else land=get_room_index(atoi(arg2));
  if ( land ) {
   if ( e->o ) {
    if ( e->o->in_room ) obj_from_room(e->o);
    else if ( e->o->carried_by ) obj_from_char(e->o);
    else if ( e->o->in_obj ) obj_from_obj(e->o);
    obj_to_room( e->o,land);
   } else if ( e->m ) {
    char_from_room(e->m);
    char_to_room(e->m,land);
   }
  }
 } else if ( !str_cmp( arg1, "got" ) ) {
  if ( !e->o ) return;
  if ( !str_cmp( arg2, "member" ) ) {
   if ( e->member ) {
    if ( e->o->carried_by ) obj_from_char(e->o);
    else if ( e->o->in_obj ) obj_from_obj(e->o);
    else if ( e->o->in_room ) obj_from_room(e->o);
    obj_to_char( e->o, e->member );
   }
  } else  if ( e->player ) {
   if ( e->o->carried_by ) obj_from_char(e->o);
   else if ( e->o->in_obj ) obj_from_obj(e->o);
   else if ( e->o->in_room ) obj_from_room(e->o);
   obj_to_char( e->o, e->player );
  }
 } else if ( !str_cmp( arg1, "move" ) ) {
 } else if ( !str_cmp( arg1, "find" ) ) {
  bool found=FALSE;
  OBJ_DATA *o=NULL;
  if ( is_number( arg2 ) ) {
   int vnum=atoi(arg2);
   if ( e->o ) {
    if ( e->o->in_room ) {
     o=find_obj(e->o->in_room->contents,vnum);
     if ( !o ) o=find_in_obj(e->o->in_room->contents,vnum);
    } else if ( e->o->carried_by ) {
     o=find_obj(e->o->carried_by->carrying,vnum);
//     if ( !o ) o=find_in_obj(e->o->carried_by->carrying,vnum);  // should this happen?
    }
    else if ( e->o->in_obj ) o=find_obj(e->o->in_obj->contains,vnum);
   } else if ( e->r ) {
    o=find_obj(e->r->contents,vnum);
    o=find_in_obj(e->r->contents,vnum);
    if ( o ) { found=TRUE; e->object=o; }
   } else if ( e->m ) {
    o=find_obj(e->m->carrying,vnum);
    if ( !o ) o=find_obj(e->m->in_room->contents,vnum);
    if ( !o ) o=find_in_obj(e->m->carrying,vnum);
   }
  } else {
   if ( e->o ) {
    if ( e->o->in_room ) {
     o=find_obj_name(e->o->in_room->contents,arg2);
     if ( !o ) o=find_in_obj_name(e->o->in_room->contents,arg2);
    } else if ( e->o->carried_by ) {
     o=find_obj_name(e->o->carried_by->carrying,arg2);
//     if ( !o ) o=find_in_obj_name(e->o->carried_by->carrying,arg2);  // should this happen?
    }
    else if ( e->o->in_obj ) o=find_obj_name(e->o->in_obj->contains,arg2);
   } else if ( e->r ) {
    o=find_obj_name(e->r->contents,arg2);
    o=find_in_obj_name(e->r->contents,arg2);
    if ( o ) { found=TRUE; e->object=o; }
   } else if ( e->m ) {
    o=find_obj_name(e->m->carrying,arg2);
    if ( !o ) o=find_obj_name(e->m->in_room->contents,arg2);
    if ( !o ) o=find_in_obj_name(e->m->carrying,arg2);
   }
  }
  if ( o ) {
   found=TRUE;
   e->object=o;
  }
  if ( found ) sim_script_goto( e, arg3 );
  else sim_script_goto( e, arg4 );
 } else if ( !str_cmp( arg1, "command" ) ) {
  //
 } else if ( !str_cmp( arg1, "locate" ) ) {
 } else if ( !str_cmp( arg1, "grouped" ) ) {
  if ( !e->player ) { sim_script_goto( e, arg3 ); return; }
  int groupsize=group_size( e->player );
  if ( groupsize > 1 ) sim_script_goto( e, arg2 );
  else sim_script_goto( e, arg3 );
 } else if ( !str_cmp( arg1, "groupmember" ) ) {
  if ( !e->player || group_size( e->player ) == 1 ) return;
  e->member=group_random_member( e->player );
 } else if ( !str_cmp( arg1, "follower" ) ) {
//  if ( !e->player || count_pc_followers( e->player ) == 0 ) return;
//  e->member=random_pc_follower( e->player );
 } else if ( !str_cmp( arg1, "pets" ) ) {
 } else if ( !str_cmp( arg1, "memberpet" ) ) {
 } else if ( !str_cmp( arg1, "grouppets" ) ) {
 } else if ( !str_cmp( arg1, "memberisplayer" ) ) {
  if ( e->member == e->player ) sim_script_goto( e, arg2 );
  else sim_script_goto( e, arg3 );
 } else if ( !str_cmp( arg1, "mobile" ) ) {
  if ( e->mobile ) sim_script_goto( e, arg2 );
  else sim_script_goto( e, arg3 );
 } else if ( !str_cmp( arg1, "object" ) ) {
  if ( e->object ) sim_script_goto( e, arg2 );
  else sim_script_goto( e, arg3 );
 } else if ( !str_cmp( arg1, "player" ) ) {
  if ( e->player ) sim_script_goto( e, arg2 );
  else sim_script_goto( e, arg3 );
 } else if ( !str_cmp( arg1, "reagents" ) ) {
 } else if ( !str_cmp( arg1, "mix" ) ) {
 } else {
  // Attempt to execute a mud command, works only for mob owners
  if ( e->m ) {
   parse_variables( e, arg1 );
   if ( arg2[0] == '%' ) { SIM_REP_VAR_MUDCMDARG(arg2); } else parse_variables( e, arg2 );
   if ( arg3[0] == '%' ) { SIM_REP_VAR_MUDCMDARG(arg3); } else parse_variables( e, arg3 );
   if ( arg4[0] == '%' ) { SIM_REP_VAR_MUDCMDARG(arg4); } else parse_variables( e, arg4 );
   if ( arg5[0] == '%' ) { SIM_REP_VAR_MUDCMDARG(arg5); } else parse_variables( e, arg5 );
   if ( arg6[0] == '%' ) { SIM_REP_VAR_MUDCMDARG(arg6); } else parse_variables( e, arg6 );
   char cmd[MSL];
   sprintf( cmd, "%s '%s' '%s' '%s' '%s' '%s'", arg1, arg2, arg3, arg4, arg5, arg6 );
   interpret( e->m, cmd );
  }
 }

}

 // called once per loop
void continue_executions() {
 SIMEXEC *e,*e_next,*e_prev;
 e=e_next=e_prev=NULL;
 for ( e=simexecutions; e; e=e_next ) {
  e_next=e->next;
  if ( e->paused > 0 ) { e->paused--; continue; }
  // remove previous completed executions
  if ( e->execution == NULL || *(e->execution) == '\0' ) {
   if ( e_prev == NULL ) {
    simexecutions=e_next;
    e->script->concurrent--;
    free(e);
    continue;
   } else {
    e_prev->next=e_next;
    e->script->concurrent--;
    free(e);
    continue;
   }
  }
  continue_execution(e);
  e_prev=e;
 }
}

 // used by db.c, save.c

void save_scripts() {
}

void load_scripts() {
}


char *script_type_to_string( int t ) {
 switch ( t ) {
   case SIMSCRIPT_TYPE_BUILD: return "build";
    case SIMSCRIPT_TYPE_BORN: return "born";
    case SIMSCRIPT_TYPE_DIES: return "dies";
  case SIMSCRIPT_TYPE_SPEAKS: return "speaks";
    case SIMSCRIPT_TYPE_NEAR: return "near";
   case SIMSCRIPT_TYPE_SPELL: return "spell";
   case SIMSCRIPT_TYPE_SKILL: return "skill";
   case SIMSCRIPT_TYPE_ORDER: return "order";
 case SIMSCRIPT_TYPE_COMMAND: return "command";
  case SIMSCRIPT_TYPE_COMBAT: return "combat";
 case SIMSCRIPT_TYPE_VICTORY: return "victory";
   case SIMSCRIPT_TYPE_GIVEN: return "given";
  case SIMSCRIPT_TYPE_RECIPE: return "recipe";
  case SIMSCRIPT_TYPE_LOADED: return "loaded";
   default: return "invalid";
 }
}

int script_string_to_type( char *in ) {
 if ( !str_cmp( in, "build"   ) ) return SIMSCRIPT_TYPE_BUILD;
 if ( !str_cmp( in, "born"    ) ) return SIMSCRIPT_TYPE_BORN;
 if ( !str_cmp( in, "dies"    ) ) return SIMSCRIPT_TYPE_DIES;
 if ( !str_cmp( in, "speaks"  ) ) return SIMSCRIPT_TYPE_SPEAKS;
 if ( !str_cmp( in, "near"    ) ) return SIMSCRIPT_TYPE_NEAR;
 if ( !str_cmp( in, "spell"   ) ) return SIMSCRIPT_TYPE_SPELL;
 if ( !str_cmp( in, "skill"   ) ) return SIMSCRIPT_TYPE_SKILL;
 if ( !str_cmp( in, "order"   ) ) return SIMSCRIPT_TYPE_ORDER;
 if ( !str_cmp( in, "command" ) ) return SIMSCRIPT_TYPE_COMMAND;
 if ( !str_cmp( in, "combat"  ) ) return SIMSCRIPT_TYPE_COMBAT;
 if ( !str_cmp( in, "victory" ) ) return SIMSCRIPT_TYPE_VICTORY;
 if ( !str_cmp( in, "given"   ) ) return SIMSCRIPT_TYPE_GIVEN;
 if ( !str_cmp( in, "recipe"  ) ) return SIMSCRIPT_TYPE_RECIPE;
 if ( !str_cmp( in, "loaded"  ) ) return SIMSCRIPT_TYPE_LOADED;
 return -1;
}

void do_scriptedit( CHAR_DATA *ch, char *argument ) {
 char arg1[MSL];
 char arg2[MSL];
 char arg3[MSL];
 char *p=one_argument(argument,arg1);
 p=one_argument(p,arg2);
 p=one_argument(p,arg3);

 save_scripts();
 if ( arg1[0] == '\0' ) {
  send_to_char( "Syntax: scriptedit create <vnum>\n\r", ch );
  send_to_char( "        scriptedit <vnum> edit\n\r", ch );
  send_to_char( "        scriptedit <vnum> name <value>\n\r", ch );
  send_to_char( "        scriptedit list                    (lists all scripts)\n\r", ch );
  send_to_char( "        scriptedit save                    (saves all scripts)\n\r", ch );
  send_to_char( "        scriptedit <vnum>                  (shows the script)\n\r", ch );
  send_to_char( "        scriptedit <vnum> brief            (shows the basic data about a script)\n\r", ch );
  send_to_char( "        scriptedit <vnum> type <value>     (sets the type)\n\r", ch );
  send_to_char( "        scriptedit <vnum> pace <seconds>   (sets script initial speed)\n\r", ch );
  send_to_char( "        scriptedit <vnum> max <copies>     (0=unlimited, 1=singleton, 2...)\n\r", ch );
  send_to_char( "        scriptedit <vnum> ritual <vnum>    (sets the ritual object for recipes or 0 for none)\n\r", ch );
  send_to_char( "        scriptedit <vnum> reagents <value list>    sets the required components for a recipe\n\r", ch );
  send_to_char( "        scriptedit <vnum> quantities <value list>  sets the corresponding component values\n\r", ch );
  send_to_char( "                                                   (default is 1 if none found)\n\r", ch );
  send_to_char( "Types: build, born, dies, speaks, near, spell, skill, order, command, combat,\n\r", ch );
  send_to_char( "       victory, given, recipe, loaded\n\r", ch );
 } else if ( is_number(arg1) ) {
  int vnum=atoi(arg1);
  SIMSCRIPT *a=find_sim_script(vnum);
  OBJ_INDEX_DATA *ritual=get_obj_index( a->ritual );
  if ( !a ) {
   send_to_char( "No such script. Create it with 'scriptedit create <vnum>'\n\r", ch );
   return;
  }
  if ( arg2[0] == '\0' ) {
   char buf[MSL];
   sprintf( buf, "%d brief", vnum );
   do_scriptedit( ch, buf );
   send_to_char( "-----------Script Starts---------\n\r", ch );
   send_to_char( a->script, ch );
   send_to_char( "-----------Script Ends-----------\n\r", ch );
  } else if ( !str_cmp( arg2, "brief" ) ) {
   char buf[MSL];
   sprintf( buf, "Vnum: [%6d] Name: %s\n\rType: %s\n\rRitual Object Vnum: [%6d] (0=none) '%s'\n\r"
                 "Recipe reagents: %s\n\rReagent quantities: %s\n\rScript length: %d characters    Initial pace: %d\n\rMaximum concurrent executions: %d\n\r",
    a->vnum, a->name, script_type_to_string( a->type ), a->ritual, ritual ? ritual->short_descr : "invalid",
    a->reagents ? a->reagents : "none", a->quantities ? a->quantities : "none", strlen(a->script), a->pace, a->max
   );
  } else if ( !str_cmp( arg2, "edit" ) ) {
   string_append( ch, &a->script );
  } else if ( !str_cmp( arg2, "name" ) ) {
   free_string(a->name);
   a->name=str_dup(arg3);
   save_scripts();
   return;
  } else if ( !str_cmp( arg2, "type" ) ) {
   int t=script_string_to_type( arg3 );
   if ( t != -1 ) a->type=t;
   else {
    send_to_char( "Invalid type!\n\r", ch );
    send_to_char( "Types: build, born, dies, speaks, near, spell, skill, order, command, combat,\n\r", ch );
    send_to_char( "       victory, given, recipe, loaded\n\r", ch );
    return;
   }
   save_scripts();
  } else if ( !str_cmp( arg2, "pace" ) ) {
   a->pace=atoi(arg3); if ( a->pace < 1 ) a->pace=1;
   save_scripts();
  } else if ( !str_cmp( arg2, "max" ) ) {
   a->max=atoi(arg3);
   save_scripts();
  } else if ( !str_cmp( arg2, "ritual" ) ) {
   int vnum=atoi(arg3);
   if ( vnum != 0 && get_obj_index(vnum) == NULL ) {
    send_to_char( "Invalid ritual obj vnum.  Setting it, but acts as 0.\n\r", ch );
   }
   a->ritual=vnum;
   save_scripts();
  } else if ( !str_cmp( arg2, "reagents" ) ) {
   free_string(a->reagents);
   a->reagents=str_dup(arg3);
   save_scripts();
  } else if ( !str_cmp( arg2, "quantities" ) ) {
   free_string(a->quantities);
   a->quantities=str_dup(arg3);
   save_scripts();
  }
 } else if ( !str_prefix(arg1,"save") ) {
  save_scripts();
  send_to_char( "Saved.\n\r", ch );
 } else if ( !str_prefix(arg1,"list") ) {
  SIMSCRIPT *a;
  char buf[MSL];
  page_to_char( "[vnum] - name type pace max [ritual object]\n\r", ch );
  for ( a=simscripts; a; a=a->next ) {
   sprintf( buf, "%6d - %33s %10s %d (%d) [%6d]\n\r",
    a->vnum, a->name, script_type_to_string( a->type ), a->pace, a->max, a->ritual );
   page_to_char( buf, ch );
  }
  return;
 } else
 if ( !str_prefix(arg1,"create") ) {
  if ( !is_number( arg3 ) || arg3[0]=='\0' ) {
   send_to_char( "Syntax: scriptedit create <vnum>\n\r", ch );
   return;
  }
  int vnum=atoi(arg3);
  if ( find_sim_script(vnum) != NULL ) {
   send_to_char( "There is already a script:\n\r", ch );
   char buf[MSL];
   sprintf( buf, "%d brief", vnum );
   do_scriptedit( ch, buf );
   return;
  }
 }
}
