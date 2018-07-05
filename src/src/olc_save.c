/**************************************************************************
 *  File: olc_save.c                                                       *
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
/* OLC_SAVE.C
 * This takes care of saving all the .are information.
 * Notes:
 * -If a good syntax checker is used for setting vnum ranges of areas
 *  then it would become possible to just cycle through vnums instead
 *  of using the iHash stuff and checking that the room or reset or
 *  mob etc is part of that area.
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"

void	save_clans		args( ( void ) );
void	save_skills		args( ( void ) );
void	save_random_data	args( ( void ) );
void	save_commands		args( ( void ) );

char *fwrite_flag( long flags, char buf[] )
{
    char offset;
    char *cp;

    buf[0] = '\0';

    if ( flags == 0 )
    {
	strcpy( buf, "0" );
	return buf;
    }

    /* 32 -- number of bits in a long */

    for ( offset = 0, cp = buf; offset < 32; offset++ )
	if ( flags & ( (long)1 << offset ) )
	{
	    if ( offset <= 'Z' - 'A' )
		*(cp++) = 'A' + offset;
	    else
		*(cp++) = 'a' + offset - ( 'Z' - 'A' + 1 );
	}

    *cp = '\0';

    return buf;
}

char *fix_string( const char *str )
{
    static char strfix[MAX_STRING_LENGTH * 2];
    int i, o;

    if ( str == NULL )
        return '\0';

    for ( o = i = 0; str[i+o] != '\0'; i++ )
    {
        if ( str[i+o] == '\r' )
            o++;
        strfix[i] = str[i+o];
    }

    strfix[i] = '\0';
    return strfix;
}

int sort_area_list( const void *v1, const void *v2 )
{
    AREA_DATA *a1 = *(AREA_DATA **) v1;
    AREA_DATA *a2 = *(AREA_DATA **) v2;

    if ( a1->min_vnum > a2->min_vnum )
	return +1;
    else if ( a1->min_vnum < a2->min_vnum )
	return -1;
    else
	return 0;
}

void save_area_list( )
{
    FILE *fp;
    AREA_DATA *pArea, *areas[top_area];
    sh_int pos = -1;

    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {
	pos++;
	areas[pos] = pArea;
    }

    qsort ( areas, top_area, sizeof(areas[0]), sort_area_list );

    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Save_area_list: fopen", 0 );
	perror( TEMP_FILE );
    } else {
	for ( pos = 0; pos < top_area; pos++ )
	{
	    pArea = areas[pos];

	    if ( pArea == NULL )
		break;

	    fprintf( fp, "%s\n", pArea->file_name );
	}

	fprintf( fp, "$\n" );
	fclose( fp );
    }

    rename( TEMP_FILE, AREA_LIST );

    return;
}

int alphabetize_helps( const void *v1, const void *v2 )
{
    HELP_DATA *a1 = *(HELP_DATA **) v1;
    HELP_DATA *a2 = *(HELP_DATA **) v2;
    int i;

    if ( a1->clan == 0 && a2->clan != 0 )	return -1;
    if ( a2->clan == 0 && a1->clan != 0 )	return  1;

    for ( i = 0; a1->keyword[i] != '\0'; i++ )
    {
	if ( a1->keyword[i] == a2->keyword[i] )	continue;
	if ( a2->keyword[i] == '\0' )		return  1;
	if ( a1->keyword[i] > a2->keyword[i] )	return  1;
	if ( a1->keyword[i] < a2->keyword[i] )	return -1;
    }
    return 0;
}

void save_helps( void )
{
    HELP_DATA *pHelp, *helps[top_help];
    FILE * fp;
    int pos = 0, lvl;

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
	helps[pos++] = pHelp;

    qsort( helps, pos, sizeof( pHelp ), alphabetize_helps );

    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Open_help: fopen", 0 );
	perror( TEMP_FILE );
	return;
    }

    for ( lvl = 0; lvl < pos; lvl++ )
    {
	pHelp = helps[lvl];

	fprintf( fp, "#HELP\n"					);

	if ( pHelp->clan != 0 )
	    fprintf( fp, "Clan %s~\n", clan_table[pHelp->clan].name );

	fprintf( fp, "Levl %d\n", pHelp->level			);
	fprintf( fp, "Name %s~\n", pHelp->name			);
	fprintf( fp, "Keyw %s~\n", pHelp->keyword		);
	fprintf( fp, "Text\n%s~\n", fix_string( pHelp->text ) 	);
	fprintf( fp, "End\n\n"					);
    }

    fprintf( fp, "#END\n" );

    fclose( fp );
    rename( TEMP_FILE, HELPS_FILE );
    mud_stat.helps_changed = FALSE;
}

void save_variable_data( )
{
    FILE *fp;
    PKILL_RECORD *pk_record;
    char buf[100];
    int channel;

    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Save_variable_data: error opening file.", 0 );
	return;
    }

    if ( mud_stat.max_ever > 0 )
	fprintf( fp, "Max_On %d\n", mud_stat.max_ever		);

    if ( mud_stat.most_today > 0 )
	fprintf( fp, "MostOn %d\n", mud_stat.most_today		);

    if ( mud_stat.capslock )
	fprintf( fp, "Cpslck %d\n", mud_stat.capslock		);

    if ( mud_stat.colorlock )
	fprintf( fp, "Clrlck %d\n", mud_stat.colorlock		);

    if ( mud_stat.newlock )
	fprintf( fp, "Newlck %d\n", mud_stat.newlock		);

    if ( mud_stat.wizlock )
	fprintf( fp, "Wizlck %d\n", mud_stat.wizlock		);

    if ( !mud_stat.multilock )
	fprintf( fp, "Mltlck %d\n", mud_stat.multilock		);

    if ( mud_stat.fLogAll )
	fprintf( fp, "LogAll %d\n", mud_stat.fLogAll		);

    if ( mud_stat.exp_mod[0] != 1 || mud_stat.exp_mod[1] != 1 )
	fprintf( fp, "ExpMod %d %d\n",
	    mud_stat.exp_mod[0], mud_stat.exp_mod[1]		);

    if ( mud_stat.good_god_string != NULL )
	fprintf( fp, "Good_G %s~\n", mud_stat.good_god_string	);

    if ( mud_stat.evil_god_string != NULL )
	fprintf( fp, "Evil_G %s~\n", mud_stat.evil_god_string	);

    if ( mud_stat.neut_god_string != NULL )
	fprintf( fp, "Neut_G %s~\n", mud_stat.neut_god_string	);

    if ( mud_stat.mud_name_string != NULL )
	fprintf( fp, "MudNam %s~\n", mud_stat.mud_name_string	);

    fprintf( fp, "QstObj %d %d\n",
	mud_stat.quest_object[0], mud_stat.quest_object[1] );

    fprintf( fp, "QsVnum %d %d\n",
	mud_stat.quest_obj_vnum[0], mud_stat.quest_obj_vnum[0] );

    fprintf( fp, "QsGold %d %d\n",
	mud_stat.quest_gold[0], mud_stat.quest_gold[1] );

    fprintf( fp, "QPoint %d %d\n",
	mud_stat.quest_points[0], mud_stat.quest_points[1] );

    fprintf( fp, "QPracs %d %d\n",
	mud_stat.quest_pracs[0], mud_stat.quest_pracs[1] );

    fprintf( fp, "QstExp %d %d\n",
	mud_stat.quest_exp[0], mud_stat.quest_exp[1] );

    fprintf( fp, "Random %d %d\n",
	mud_stat.random_vnum[0], mud_stat.random_vnum[1] );

    fprintf( fp, "Unique %d %d\n",
	mud_stat.unique_vnum[0], mud_stat.unique_vnum[1] );

    fprintf( fp, "M-Time %d\n", mud_stat.timeout_mortal );
    fprintf( fp, "I-Time %d\n", mud_stat.timeout_immortal );
    fprintf( fp, "Mor-LD %d\n", mud_stat.timeout_ld_mort );
    fprintf( fp, "Imm-LD %d\n", mud_stat.timeout_ld_imm );

    for ( pk_record = recent_list; pk_record != NULL; pk_record = pk_record->next )
    {
	fprintf( fp, "PKills %s~ %s~ %s~ %s~ %d %d %ld %d %d %s~\n",
	    pk_record->killer_name,     pk_record->victim_name,
	    pk_record->killer_clan,     pk_record->victim_clan,
	    pk_record->level[0],        pk_record->level[1],
	    pk_record->pkill_time,      pk_record->pkill_points,
	    pk_record->bounty,          pk_record->assist_string );
    }

    for ( channel = 0; channel_table[channel].name != NULL; channel++ )
    {
	fprintf( fp, "\nChann %s\n",channel_table[channel].name		  );
	fprintf( fp, "Self  %s~\n", channel_table[channel].ch_string	  );
	fprintf( fp, "Other %s~\n", channel_table[channel].other_string	  );
	fprintf( fp, "Arena %d\n",  channel_table[channel].arena	  );
	fprintf( fp, "Censr %d\n",  channel_table[channel].censor	  );
	fprintf( fp, "Drunk %d\n",  channel_table[channel].drunk	  );
	fprintf( fp, "PTitl %d\n",  channel_table[channel].pretitle	  );
	fprintf( fp, "Quiet %d\n",  channel_table[channel].quiet	  );
	fprintf( fp, "Color %s\n",  color_string( channel_table[channel].color_default ) );;
	fprintf( fp, "Flags %s\n",  fwrite_flag( channel_table[channel].bit, buf ) );
	fprintf( fp, "Level %d\n",  channel_table[channel].level	  );
	fprintf( fp, "Vrble %c\n",  channel_table[channel].color_variable );
	fprintf( fp, "End\n"						  );
    }

    fprintf( fp, "\nEND\n" );

    fclose ( fp );

    rename( TEMP_FILE, VARIABLE_FILE );

    mud_stat.changed = FALSE;
}

void save_mobprogs( FILE *fp, AREA_DATA *pArea )
{
    PROG_CODE *pMprog;
    bool found = FALSE;
    int i;

    for( i = pArea->min_vnum; i <= pArea->max_vnum; i++ )
    {
	if ( (pMprog = get_prog_index( i, PRG_MPROG ) ) != NULL)
	{
	    if ( !found )
	    {
		fprintf(fp, "#MOBPROGS");
		found = TRUE;
	    }

	    fprintf(fp, "\n#%d\n", i);
	    fprintf(fp, "%s~\n", pMprog->author);
	    fprintf(fp, "%s~\n", pMprog->name);
	    fprintf(fp, "%s~\n", fix_string(pMprog->code));
	}
    }

    if ( found )
	fprintf(fp,"#0\n\n");
    return;
}

void save_mobile( FILE *fp, MOB_INDEX_DATA *pMobIndex )
{
    PROG_LIST *pMprog;
    char buf[MAX_STRING_LENGTH];
    sh_int race = pMobIndex->race, dam_mod;

    fprintf( fp, "\n#%d\n",         pMobIndex->vnum );
    fprintf( fp, "%s~\n",         pMobIndex->player_name );
    fprintf( fp, "%s~\n",         pMobIndex->short_descr );
    fprintf( fp, "%s~\n",         fix_string( pMobIndex->long_descr ) );
    fprintf( fp, "%s~\n",         fix_string( pMobIndex->description) );
    fprintf( fp, "%s~\n",         race_table[race].name );

    for ( dam_mod = 0; dam_mod < DAM_MAX; dam_mod++ )
	fprintf( fp, "%d ", pMobIndex->damage_mod[dam_mod] );

    fprintf( fp, "\n%s ",	fwrite_flag( pMobIndex->act, buf ) );
    fprintf( fp, "%s ",	          fwrite_flag( pMobIndex->affected_by, buf ) );
    fprintf( fp, "%s ",           fwrite_flag( pMobIndex->shielded_by, buf ) );
    fprintf( fp, "%d %d\n",       pMobIndex->alignment, pMobIndex->group);
    fprintf( fp, "%d %d %d ",	  pMobIndex->level, pMobIndex->hitroll, pMobIndex->saves );
    fprintf( fp, "%d %d ",     pMobIndex->hit[0], 
	   	     	          pMobIndex->hit[1] );
    fprintf( fp, "%d %d ",     pMobIndex->mana[0], 
	     	     	          pMobIndex->mana[1] );
    fprintf( fp, "%d %d %d ",     pMobIndex->damage[DICE_NUMBER], 
	     	     	          pMobIndex->damage[DICE_TYPE], 
	     	     	          pMobIndex->damage[DICE_BONUS] );
    fprintf( fp, "%s\n",          attack_table[pMobIndex->dam_type].name );
    fprintf( fp, "%d %d %d %d\n", pMobIndex->ac[AC_PIERCE],
	     	     	          pMobIndex->ac[AC_BASH],
	     	     	          pMobIndex->ac[AC_SLASH],
	     	     	          pMobIndex->ac[AC_EXOTIC] );
    fprintf( fp, "%s %s %s %ld\n",
	                          position_flags[pMobIndex->start_pos].name,
	         	     	  position_flags[pMobIndex->default_pos].name,
	         	     	  sex_flags[pMobIndex->sex].name,
	         	     	  pMobIndex->wealth );
    fprintf( fp, "%d %s %s %s\n", pMobIndex->skill_percentage,
				pMobIndex->class == -1 ? "random_class" :
				class_table[pMobIndex->class].name,
				size_flags[pMobIndex->size].name,
				fwrite_flag( pMobIndex->parts, buf ) );

    if (pMobIndex->absorption)
	fprintf( fp, "A %d\n", pMobIndex->absorption );

    if ( pMobIndex->bank_branch != 0 )
	fprintf( fp, "B %d\n", pMobIndex->bank_branch );

    if (pMobIndex->die_descr && pMobIndex->die_descr[0])
	fprintf( fp, "D %s~\n", pMobIndex->die_descr );

    if ( pMobIndex->exp_percent != 100 )
	fprintf( fp, "E %d\n", pMobIndex->exp_percent );

    if ( pMobIndex->regen[0] != 0 )
	fprintf( fp, "F %d\n", pMobIndex->regen[0] );

    if ( pMobIndex->regen[1] != 0 )
	fprintf( fp, "G %d\n", pMobIndex->regen[1] );

    if ( pMobIndex->regen[2] != 0 )
	fprintf( fp, "H %d\n", pMobIndex->regen[2] );

    if (pMobIndex->max_world )
	fprintf( fp, "W %d\n", pMobIndex->max_world );

    if (pMobIndex->perm_mob_pc_deaths[0] != 0
    ||  pMobIndex->perm_mob_pc_deaths[1] != 0 )
	fprintf( fp, "P %ld %ld\n", pMobIndex->perm_mob_pc_deaths[0],
	    pMobIndex->perm_mob_pc_deaths[1] );

    if (pMobIndex->reflection)
	fprintf( fp, "R %d\n", pMobIndex->reflection );

    if (pMobIndex->say_descr && pMobIndex->say_descr[0])
	fprintf( fp, "T %s~\n", pMobIndex->say_descr );

    for ( dam_mod = 0; skill_table[dam_mod].name[0] != '\0'; dam_mod++ )
    {
	if ( pMobIndex->learned[dam_mod] != pMobIndex->skill_percentage
	&&   str_cmp( skill_table[dam_mod].name, "deleted" ) )
	     fprintf( fp, "S '%s' %d\n",
		skill_table[dam_mod].name, pMobIndex->learned[dam_mod] );
    }

    for (pMprog = pMobIndex->mprogs; pMprog; pMprog = pMprog->next)
    {
        fprintf(fp, "M %s %d %s~\n",
        prog_type_to_name(pMprog->trig_type), pMprog->vnum,
                pMprog->trig_phrase);
    }

    return;
}

void save_mobiles( FILE *fp, AREA_DATA *pArea )
{
    MOB_INDEX_DATA *pMob;
    bool found = FALSE;
    int i;

    for( i = pArea->min_vnum; i <= pArea->max_vnum; i++ )
    {
	if ( (pMob = get_mob_index( i )) )
	{
	    if ( !found )
	    {
		fprintf( fp, "#MOBILES" );
		found = TRUE;
	    }

	    save_mobile( fp, pMob );
	}
    }

    if ( found )
	fprintf( fp, "#0\n\n" );

    return;
}

void save_objprogs( FILE *fp, AREA_DATA *pArea )
{
    PROG_CODE *pOprog;
    bool found = FALSE;
    int i;

    for ( i = pArea->min_vnum; i <= pArea->max_vnum; i++ )
    {
	if ( ( pOprog = get_prog_index( i, PRG_OPROG ) ) != NULL )
	{
	    if ( !found )
	    {
		fprintf( fp, "#OBJPROGS" );
		found = TRUE;
	    }

	    fprintf( fp, "\n#%d\n", i );
	    fprintf( fp, "%s~\n", pOprog->author) ;
	    fprintf( fp, "%s~\n", pOprog->name );
	    fprintf( fp, "%s~\n", fix_string( pOprog->code ) );
	}
    }

    if ( found )
	fprintf( fp, "#0\n\n" );
}

void save_object( FILE *fp, OBJ_INDEX_DATA *pObjIndex )
{
    AFFECT_DATA *pAf;
    EXTRA_DESCR_DATA *pEd;
    PROG_LIST *pOprog;
    char buf[MAX_STRING_LENGTH];
    sh_int pos;

    fprintf( fp, "\n#%d\n",    pObjIndex->vnum );
    fprintf( fp, "%s~\n",    pObjIndex->name );
    fprintf( fp, "%s~\n",    pObjIndex->short_descr );
    fprintf( fp, "%s~\n",    fix_string( pObjIndex->description ) );
    fprintf( fp, "%s ",      flag_string( type_flags, pObjIndex->item_type ) );
    fprintf( fp, "%s ",      fwrite_flag( pObjIndex->extra_flags, buf ) );
    fprintf( fp, "%s\n",     fwrite_flag( pObjIndex->wear_flags,  buf ) );

    switch ( pObjIndex->item_type )
    {
        default:
	    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[0], buf ) );
	    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[1], buf ) );
	    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[2], buf ) );
	    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[3], buf ) );
	    fprintf( fp, "%s\n", fwrite_flag( pObjIndex->value[4], buf ) );
	    break;

	case ITEM_COMPONENT_BREW:
	case ITEM_COMPONENT_SCRIBE:
	    fprintf( fp, "%d 0 0 0 0\n", pObjIndex->value[0] );
	    break;

        case ITEM_LIGHT:
	    fprintf( fp, "0 0 %d 0 0\n",
		     pObjIndex->value[2] < 1 ? 999  /* infinite */
		     : pObjIndex->value[2] );
	    break;

        case ITEM_MONEY:
            fprintf( fp, "%d %d 0 0 0\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1]);
            break;
            
        case ITEM_DRINK_CON:
            fprintf( fp, "%d %d '%s' %d 0\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     liq_table[pObjIndex->value[2]].liq_name,
		     pObjIndex->value[3]);
            break;
                    
	case ITEM_FOUNTAIN:
	    fprintf( fp, "%d %d '%s' 0 0\n",
	             pObjIndex->value[0],
	             pObjIndex->value[1],
	             liq_table[pObjIndex->value[2]].liq_name);
	    break;
	    
        case ITEM_CONTAINER:
            fprintf( fp, "%d %s %d %d %d\n",
                     pObjIndex->value[0],
                     fwrite_flag( pObjIndex->value[1], buf ),
                     pObjIndex->value[2],
                     pObjIndex->value[3],
                     pObjIndex->value[4]);
            break;
            
        case ITEM_PIT:
            fprintf( fp, "%d 0 %d %d %d\n",
                     pObjIndex->value[0],
                     pObjIndex->value[2],
                     pObjIndex->value[3],
                     pObjIndex->value[4]);
            break;

        case ITEM_FOOD:
            fprintf( fp, "%d %d 0 %s 0\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     fwrite_flag( pObjIndex->value[3], buf ) );
            break;
            
        case ITEM_PORTAL:
            fprintf( fp, "%d %s %s %d 0\n",
                     pObjIndex->value[0],
                     print_flags( pObjIndex->value[1] ),
                     fwrite_flag( pObjIndex->value[2], buf ),
                     pObjIndex->value[3]);
            break;
            
        case ITEM_FURNITURE:
            fprintf( fp, "%d %d %s %d %d\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     fwrite_flag( pObjIndex->value[2], buf),
                     pObjIndex->value[3],
                     pObjIndex->value[4]);
            break;
            
        case ITEM_WEAPON:
            fprintf( fp, "%s %d %d %s %s\n",
		     flag_string( weapon_class, pObjIndex->value[0] ),
                     pObjIndex->value[1],
                     pObjIndex->value[2],
                     attack_table[pObjIndex->value[3]].name,
                     fwrite_flag( pObjIndex->value[4], buf ) );
            break;
            
        case ITEM_ARMOR:
	case ITEM_DEMON_STONE:
            fprintf( fp, "%d %d %d %d %d\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     pObjIndex->value[2],
                     pObjIndex->value[3],
                     pObjIndex->value[4]);
            break;
            
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
	    fprintf( fp, "%d '%s' '%s' '%s' '%s'\n",
		     pObjIndex->value[0] > 0 ? /* no negative numbers */
		     pObjIndex->value[0]
		     : 0,
		     pObjIndex->value[1] != -1 ?
		     skill_table[pObjIndex->value[1]].name
		     : "",
		     pObjIndex->value[2] != -1 ?
		     skill_table[pObjIndex->value[2]].name
		     : "",
		     pObjIndex->value[3] != -1 ?
		     skill_table[pObjIndex->value[3]].name
		     : "",
		     pObjIndex->value[4] != -1 ?
		     skill_table[pObjIndex->value[4]].name
		     : "");
	    break;

	case ITEM_TRAP:
	    fprintf( fp, "%s %s %d %d %d\n",
		trap_type_table[pObjIndex->value[0]].name,
		damage_mod_table[pObjIndex->value[1]].name, pObjIndex->value[2],
		pObjIndex->value[3], pObjIndex->value[4] );
	    break;

        case ITEM_STAFF:
        case ITEM_WAND:
	    fprintf( fp, "%d ", pObjIndex->value[0] );
	    fprintf( fp, "%d ", pObjIndex->value[1] );
	    fprintf( fp, "%d '%s' 0\n",
		     pObjIndex->value[2],
		     pObjIndex->value[3] != -1 ?
		       skill_table[pObjIndex->value[3]].name
		       : 0 );
	    break;
    }

    fprintf( fp, "%d ", pObjIndex->level );
    fprintf( fp, "%d ", pObjIndex->weight );
    fprintf( fp, "%d\n", pObjIndex->cost );

    for( pAf = pObjIndex->affected; pAf; pAf = pAf->next )
    {
	if ( pAf->where == TO_OBJECT
	||   (pAf->bitvector == 0 && pAf->where != TO_DAM_MODS) )
	        fprintf( fp, "A %d %d\n",  pAf->location, pAf->modifier );
	else
	{
		fprintf( fp, "F " );

		switch(pAf->where)
		{
			case TO_AFFECTS:
				fprintf( fp, "A " );
				break;
                        case TO_SHIELDS:
                                fprintf( fp, "S " );
                                break;  
			case TO_DAM_MODS:
				fprintf( fp, "D " );
				break;
			default:
				bug( "olc_save: Invalid Affect->where", 0);
				break;
		}
		
		fprintf( fp, "%d %d %s\n", pAf->location, pAf->modifier,
				fwrite_flag( pAf->bitvector, buf ) );
	}
    }

    if ( pObjIndex->forge_count > 0 || pObjIndex->forge_vnum > 0 )
	fprintf( fp, "B %d %d\n", pObjIndex->forge_vnum,pObjIndex->forge_count);

    for( pEd = pObjIndex->extra_descr; pEd; pEd = pEd->next )
        fprintf( fp, "E\n%s~\n%s~\n", pEd->keyword, fix_string( pEd->description ) );

    if (pObjIndex->size > -1 && pObjIndex->size < SIZE_NONE)
	fprintf( fp, "H %s~\n", size_flags[pObjIndex->size].name );

    if (pObjIndex->history != NULL && pObjIndex->history[0] != '\0')
	fprintf( fp, "M %s~\n", fix_string(pObjIndex->history) );

    for (pOprog = pObjIndex->oprogs; pOprog; pOprog = pOprog->next)
    {
        fprintf(fp, "O %s %d %s~\n",
        prog_type_to_name(pOprog->trig_type), pOprog->vnum,
                pOprog->trig_phrase);
    }

    if (pObjIndex->quest_points)
	fprintf( fp, "Q %d\n", pObjIndex->quest_points );

    if ( pObjIndex->targets )
	fprintf( fp, "R %d\n", pObjIndex->targets );

    if ( pObjIndex->success != 100 )
	fprintf( fp, "S %d\n", pObjIndex->success );

    for ( pos = 0; class_table[pos].name[0] != '\0'; pos++ )
    {
	if ( !pObjIndex->class_can_use[pos] )
	    fprintf( fp, "T %s\n", class_table[pos].name );
    }

    return;
}
 
void save_objects( FILE *fp, AREA_DATA *pArea )
{
    OBJ_INDEX_DATA *pObj;
    bool found = FALSE;
    int i;

    for( i = pArea->min_vnum; i <= pArea->max_vnum; i++ )
    {
	if ( (pObj = get_obj_index( i )) )
	{
	    if ( !found )
	    {
		fprintf( fp, "#OBJECTS" );
		found = TRUE;
	    }

	    save_object( fp, pObj );
	}
    }

    if ( found )
	fprintf( fp, "#0\n\n" );

    return;
}

void save_roomprogs( FILE *fp, AREA_DATA *pArea )
{
    PROG_CODE *pRprog;
    bool found = FALSE;
    int i;

    for ( i = pArea->min_vnum; i <= pArea->max_vnum; i++ )
    {
	if ( ( pRprog = get_prog_index( i, PRG_RPROG ) ) != NULL )
	{
	    if ( !found )
	    {
		fprintf( fp, "#ROOMPROGS" );
		found = TRUE;
	    }

	    fprintf( fp, "\n#%d\n", i );
	    fprintf( fp, "%s~\n", pRprog->author );
	    fprintf( fp, "%s~\n", pRprog->name );
	    fprintf( fp, "%s~\n", fix_string( pRprog->code ) );
	}
    }

    if ( found )
	fprintf( fp, "#0\n\n" );
}

void save_rooms( FILE *fp, AREA_DATA *pArea )
{
    ROOM_INDEX_DATA *pRoomIndex;
    ROOM_DAMAGE_DATA *dam;
    PROG_LIST *pRprog;
    EXTRA_DESCR_DATA *pEd;
    EXIT_DATA *pExit;
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
    int door, iHash;

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
	for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
	{
	    if ( pRoomIndex->area == pArea )
	    {
		if ( !found )
		{
		    fprintf( fp, "#ROOMS" );
		    found = TRUE;
		}

		fprintf( fp, "\n#%d\n",	pRoomIndex->vnum );
		fprintf( fp, "%s~\n",	pRoomIndex->name );
		fprintf( fp, "%s~\n",	fix_string( pRoomIndex->description ) );
		fprintf( fp, "%ld ",	pRoomIndex->room_flags );
		fprintf( fp, "%d\n",	pRoomIndex->sector_type );

		for ( dam = pRoomIndex->room_damage; dam != NULL; dam = dam->next )
		{
		    fprintf( fp, "A %s %d %d %d\n%s~\n%s~\n",
			damage_mod_table[dam->damage_type].name,
			dam->damage_min, dam->damage_max, dam->success,
			dam->msg_victim, dam->msg_room );
		}

		for( door = 0; door < MAX_DIR; door++ )	/* I hate this! */
		{
		    if ( ( pExit = pRoomIndex->exit[door] )
		    &&   pExit->u1.to_room )
		    {
			fprintf( fp, "D%d\n",      pExit->orig_door );
			fprintf( fp, "%s~\n",      fix_string( pExit->description ) );
			fprintf( fp, "%s~\n",      pExit->keyword );
			fprintf( fp, "%s %d %d\n", fwrite_flag(pExit->rs_flags,buf),
			    pExit->key, pExit->u1.to_room->vnum );
		    }
		}

		for ( pEd = pRoomIndex->extra_descr; pEd; pEd = pEd->next )
                    fprintf( fp, "E\n%s~\n%s~\n",
			pEd->keyword, fix_string( pEd->description ) );

		if ( pRoomIndex->mana_rate != 100 || pRoomIndex->heal_rate != 100 )
		    fprintf ( fp, "H %d %d\n",
			pRoomIndex->heal_rate, pRoomIndex->mana_rate );

		if (pRoomIndex->music != NULL )
		    fprintf ( fp, "N %s~\n", pRoomIndex->music );

		if ( pRoomIndex->max_people != 0 )
		    fprintf( fp, "P %d\n", pRoomIndex->max_people );

		for (pRprog = pRoomIndex->rprogs; pRprog; pRprog = pRprog->next)
		{
		    fprintf(fp, "R %s %d %s~\n",
		    prog_type_to_name(pRprog->trig_type), pRprog->vnum,
		    pRprog->trig_phrase);
		}

		fprintf( fp, "S\n" );
            }
        }
    }

    if ( found )
	fprintf( fp, "#0\n\n" );

    return;
}

void save_specials( FILE *fp, AREA_DATA *pArea )
{
    MOB_INDEX_DATA *pMobIndex;
    bool found = FALSE;
    int iHash;
    
    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next )
        {
            if ( pMobIndex && pMobIndex->area == pArea && pMobIndex->spec_fun )
            {
		if ( !found )
		{
		    fprintf( fp, "#SPECIALS\n" );
		    found = TRUE;
		}

                fprintf( fp, "M %d %s\n", pMobIndex->vnum,
                              spec_name( pMobIndex->spec_fun ) );
            }
        }
    }

    if ( found )
	fprintf( fp, "S\n\n" );

    return;
}

bool save_door_resets( FILE *fp, AREA_DATA *pArea )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pExit;
    bool found = FALSE;
    int iHash;
    int door;

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
        {
            if ( pRoomIndex->area == pArea )
            {
                for( door = 0; door < MAX_DIR; door++ )
                {
                    if ( ( pExit = pRoomIndex->exit[door] )
                          && pExit->u1.to_room 
                          && ( IS_SET( pExit->rs_flags, EX_CLOSED )
                          || IS_SET( pExit->rs_flags, EX_LOCKED ) ) )
		    {
		 	if ( !found )
			{
			    fprintf( fp, "#RESETS\n" );
			    found = TRUE;
			}

			fprintf( fp, "D 100 %d %d %d\n",
				pRoomIndex->vnum,
				pExit->orig_door,
				IS_SET( pExit->rs_flags, EX_LOCKED) ? 2 : 1 );
		    }
		}
	    }
	}
    }

    return found;
}

void save_resets( FILE *fp, AREA_DATA *pArea )
{
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pLastMob = NULL;
    OBJ_INDEX_DATA *pLastObj;
    ROOM_INDEX_DATA *pRoom;
    char buf[MAX_STRING_LENGTH];
    bool found;
    int iHash;

    found = save_door_resets( fp, pArea );

    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
        {
            if ( pRoom->area == pArea )
	    {
		for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
		{
		    if ( !found )
		    {
			fprintf( fp, "#RESETS\n" );
			found = TRUE;
		    }

		    switch ( pReset->command )
		    {
			default:
			    bug( "Save_resets: bad command %c.", pReset->command );
			    break;

			case 'M':
		            pLastMob = get_mob_index( pReset->arg1 );
			    fprintf( fp, "M %d %d %d %d %d\n", 
				pReset->percent, pReset->arg1,
		                pReset->arg2, pReset->arg3, pReset->arg4 );
		            break;

			case 'O':
		            pLastObj = get_obj_index( pReset->arg1 );
		            pRoom = get_room_index( pReset->arg3 );
			    fprintf( fp, "O %d %d 0 %d\n", 
				pReset->percent, pReset->arg1, pReset->arg3 );
		            break;

			case 'P':
		            pLastObj = get_obj_index( pReset->arg1 );
			    fprintf( fp, "P %d %d %d %d %d\n", 
				pReset->percent,
			        pReset->arg1,
			        pReset->arg2,
		                pReset->arg3,
                		pReset->arg4 );
		            break;

			case 'G':
			    fprintf( fp, "G %d %d 0\n", pReset->percent, pReset->arg1 );
		            if ( !pLastMob )
		            {
		                sprintf( buf,
		                    "Save_resets: !NO_MOB! in [%s]", pArea->file_name );
		                bug( buf, 0 );
		            }
		            break;

			case 'E':
			    fprintf( fp, "E %d %d 0 %d\n",
				pReset->percent,
			        pReset->arg1,
		                pReset->arg3 );
			    if ( !pLastMob )
			    {
				sprintf( buf, "Save_resets: !NO_MOB! in [%s]", pArea->file_name );
				bug( buf, 0 );
			    }
		            break;

			case 'D':
		            break;

			case 'R':
		            pRoom = get_room_index( pReset->arg1 );
			    fprintf( fp, "R %d %d %d\n", pReset->percent,
			        pReset->arg1,
		                pReset->arg2 );
		            break;
	            }
	        }
	    }	/* End if correct area */
	}	/* End for pRoom */
    }	/* End for iHash */

    if ( found )
	fprintf( fp, "S\n\n" );
    return;
}

void save_shops( FILE *fp, AREA_DATA *pArea )
{
    SHOP_DATA *pShopIndex;
    MOB_INDEX_DATA *pMobIndex;
    bool found = FALSE;
    int iTrade;
    int iHash;
    
    for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next )
        {
            if ( pMobIndex && pMobIndex->area == pArea && pMobIndex->pShop )
            {
		if ( !found )
		{
		    fprintf( fp, "#SHOPS\n" );
		    found = TRUE;
		}

                pShopIndex = pMobIndex->pShop;

                fprintf( fp, "%d ", pShopIndex->keeper );
                for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
                {
                    if ( pShopIndex->buy_type[iTrade] != 0 )
                    {
                       fprintf( fp, "%d ", pShopIndex->buy_type[iTrade] );
                    }
                    else
                       fprintf( fp, "0 ");
                }
                fprintf( fp, "%d %d ", pShopIndex->profit_buy, pShopIndex->profit_sell );
                fprintf( fp, "%d %d\n", pShopIndex->open_hour, pShopIndex->close_hour );
            }
        }
    }

    if ( found )
	fprintf( fp, "0\n\n" );

    return;
}

void save_area( AREA_DATA *pArea, bool deleted )
{
    FILE *fp;

    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Open_area: fopen", 0 );
	perror( pArea->file_name );
    }

    fprintf( fp, "#AOD_AREA\n" );
    fprintf( fp, "%s~\n", pArea->name );
    fprintf( fp, "%d %d %s %d\n", pArea->min_vnum, pArea->max_vnum,
	print_flags( pArea->area_flags ), pArea->security );

    if ( pArea->clan )
	fprintf( fp, "Clan %s~\n", clan_table[pArea->clan].name );

    if ( pArea->music )
 	fprintf( fp, "Musi %s~\n", pArea->music );

    if ( pArea->run_vnum != 0 )
	fprintf( fp, "Room %d\n", pArea->run_vnum );

    if ( pArea->directions )
	fprintf( fp, "Path %s~\n", pArea->directions );

    fprintf( fp, "Levl %d %d\n", pArea->min_level, pArea->max_level );

    if ( pArea->builder )
	fprintf( fp, "Bldr %s~\n", pArea->builder );

    if ( pArea->alignment != '?' )
	fprintf( fp, "Alig %c\n", pArea->alignment );

    fprintf( fp, "End\n\n" );

    save_mobiles( fp, pArea );
    save_objects( fp, pArea );
    save_rooms( fp, pArea );
    save_specials( fp, pArea );
    save_resets( fp, pArea );
    save_shops( fp, pArea );
    save_mobprogs( fp, pArea );
    save_objprogs( fp, pArea );
    save_roomprogs( fp, pArea );

    fprintf( fp, "#$\n" );

    fclose( fp );

    if ( deleted )
    {
	sprintf( log_buf, "%sareas/%s", DELETED_DIR, pArea->file_name );
	rename( TEMP_FILE, log_buf );
    }
    else
	rename( TEMP_FILE, pArea->file_name );
}

int alphabetize_races( const void *v1, const void *v2 )
{
    struct race_type *idx1 = *(struct race_type **) v1;
    struct race_type *idx2 = *(struct race_type **) v2;
    int i;

    for ( i = 0; idx1->name[i] != '\0'; i++ )
    {
	if ( idx1->pc_race && !idx2->pc_race )
	    return -1;

	if ( idx2->pc_race && !idx1->pc_race )
	    return 1;

	if ( idx1->name[i] == idx2->name[i] )
	    continue;

	if ( idx1->name[i] > idx2->name[i] )
	    return 1;

	if ( idx1->name[i] < idx2->name[i] )
	    return -1;
    }

    return 0;
}

void save_races( void )
{
    FILE *fp;
    struct race_type *races[maxRace];
    int i, p;

    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Save_races, can not open file.", 0 );
	return;
    }

    fprintf( fp, "#MAX_RACE %d\n",	maxRace		);

    for ( i = 0; i < maxRace; i++ )
	races[i] = &race_table[i];

    qsort( races, maxRace, sizeof( races[0] ), alphabetize_races );

    for ( i = 0; i < maxRace; i++ )
    {
	fprintf( fp, "#Race\n"						);
	fprintf( fp, "Name %s~\n",	races[i]->name			);

	if ( races[i]->aff )
	    fprintf( fp, "Affs %s\n",	print_flags( races[i]->aff )	);

	if ( races[i]->shd )
	    fprintf( fp, "Shds %s\n",	print_flags( races[i]->shd )	);

	if ( races[i]->parts )
	    fprintf( fp, "Part %s\n",	print_flags( races[i]->parts )	);

	if ( races[i]->disabled )
	    fprintf( fp, "Dsbl %d\n",	races[i]->disabled		);

	fprintf( fp, "DamM"						);
	for ( p = 0; p < DAM_MAX; p++ )
	    fprintf( fp, " %d",	races[i]->damage_mod[p]			);

	fprintf( fp, "\nSize %d\n",	races[i]->size			);

	if ( races[i]->pc_race )
	{
	    fprintf( fp, "PcRc %d\n",	races[i]->pc_race		);
	    fprintf( fp, "WhoN %s~\n",	races[i]->who_name		);
	    fprintf( fp, "Pnts %d\n",	races[i]->points		);
	    fprintf( fp, "Attk %s~\n",	attack_table[races[i]->attack].name );

	    for ( p = 0; p < 5 && races[i]->skills[p] != NULL; p++ )
		fprintf( fp, "Skil %s~\n", races[i]->skills[p]		);

	    fprintf( fp, "Stat"						);
	    for ( p = 0; p < MAX_STATS; p++ )
		fprintf( fp, " %d",	races[i]->stats[p]		);
	    fprintf( fp, "\n"						);

	    fprintf( fp, "MaxS"						);
	    for ( p = 0; p < MAX_STATS; p++ )
		fprintf( fp, " %d",	races[i]->max_stats[p]		);
	    fprintf( fp, "\n"						);

	    for ( p = 0; class_table[p].name[0] != '\0'; p++ )
		fprintf( fp, "Clas %s %d %d\n",
		    class_table[p].name,
		    races[i]->class_can_use[p],
		    races[i]->class_mult[p] );
	}

	fprintf( fp, "End\n\n"						);
    }

    fprintf( fp, "#End\n" );
    fclose( fp );
    rename( TEMP_FILE, RACES_FILE );
    mud_stat.races_changed = FALSE;
    return;
}

void save_classes( void )
{
    FILE *fp;
    int class, sn;

    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	bug( "Save_classes, can not open file.", 0 );
	return;
    }

    fprintf( fp, "#MAX_CLASS %d\n\n",	maxClass			);

    for ( class = 0; class_table[class].name[0] != '\0'; class++ )
    {
	fprintf( fp, "#Class\n"						);
	fprintf( fp, "Name %s~\n",	class_table[class].name		);
	fprintf( fp, "WhoN %s\n",	class_table[class].who_name	);
	fprintf( fp, "Attr %d\n",	class_table[class].attr_prime	);
	fprintf( fp, "Thac %d %d\n",	class_table[class].thac0_00,
					class_table[class].thac0_32	);
	fprintf( fp, "HitP %d %d\n",	class_table[class].hp_min,
					class_table[class].hp_max	);
	fprintf( fp, "Mana %d\n",	class_table[class].mana_percent	);
	fprintf( fp, "Disa %d\n",	class_table[class].disabled	);
	fprintf( fp, "Base %s~\n",	class_table[class].base_group	);
	fprintf( fp, "Deft %s~\n",	class_table[class].default_group);
	fprintf( fp, "SubC %s\n",
	    class_table[class_table[class].sub_class].name		);
	fprintf( fp, "Tier %d\n",	class_table[class].tier		);

	for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( skill_table[sn].skill_level[class] != LEVEL_IMMORTAL )
		fprintf( fp, "Skil '%s' %d %d\n",
		    skill_table[sn].name,
		    skill_table[sn].skill_level[class],
		    skill_table[sn].rating[class] );
	}

	for ( sn = 0; group_table[sn].name[0] != '\0'; sn++ )
	{
	    if ( group_table[sn].rating[class] != -1 )
		fprintf( fp, "Grup '%s' %d\n",
		    group_table[sn].name,
		    group_table[sn].rating[class] );
	}

	fprintf( fp, "End\n\n"						);
    }

    fprintf( fp, "#End\n" );
    fclose( fp );
    rename( TEMP_FILE, CLASSES_FILE );
    mud_stat.classes_changed = FALSE;
}

int alphabetize_socials( const void *v1, const void *v2 )
{
    struct social_type *a1 = *(struct social_type **) v1;
    struct social_type *a2 = *(struct social_type **) v2;
    int i;

    for ( i = 0; a1->name[i] != '\0'; i++ )
    {
	if ( a1->name[i] == a2->name[i] )	continue;
	if ( a2->name[i] == '\0' )		return 1;
	if ( a1->name[i] > a2->name[i] )	return 1;
	if ( a1->name[i] < a2->name[i] )	return -1;
    }

    return 0;
}

void save_social_table( void )
{
    FILE *fp;
    struct social_type *socials[maxSocial];
    int i;
	
    if ( ( fp = fopen ( "../data/info/social.tmp", "w" ) ) == NULL )
    {
	bug( "Could not open SOCIAL_FILE for writing.", 0 );
	return;
    }

    for ( i = 0; i < maxSocial; i++ )
	socials[i] = &social_table[i];

    qsort( socials, maxSocial, sizeof(socials[0]), alphabetize_socials );

    fprintf ( fp, "%d\n", maxSocial );

    for ( i = 0 ; i < maxSocial ; i++ )
    {
	fprintf( fp, "%s\n%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n%s~\n\n",
	    socials[i]->name,
	    socials[i]->char_no_arg,
	    socials[i]->others_no_arg,
	    socials[i]->char_found,
	    socials[i]->others_found,
	    socials[i]->vict_found,
	    socials[i]->char_auto,
	    socials[i]->others_auto );
    }
		
    fclose ( fp );
    rename( "../data/info/social.tmp", "../data/info/socials.dat" );
    unlink( "../data/info/social.tmp" );
    mud_stat.socials_changed = FALSE;
}

void do_asave( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    if ( ch == NULL )
    {
	for( pArea = area_first; pArea; pArea = pArea->next )
	{
	    if ( IS_SET(pArea->area_flags, AREA_CHANGED) )
	    {
		REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
		save_area( pArea, FALSE );
	    }
	}

	if ( mud_stat.classes_changed )
	    save_classes( );

	if ( mud_stat.helps_changed )
	    save_helps( );

	if ( mud_stat.races_changed )
	    save_races( );

	if ( mud_stat.skills_changed )
	    save_skills( );

	if ( mud_stat.socials_changed )
	    save_social_table( );

	if ( mud_stat.randoms_changed )
	    save_random_data( );

	if ( mud_stat.changed )
	    save_variable_data( );

	if ( mud_stat.clans_changed )
	    save_clans( );
    }

    else if ( argument[0] == '\0' )
    {
	send_to_char(	"Syntax:\n\r"
			"  asave <vnum>    - saves a particular area\n\r"
			"  asave list      - saves the area.lst file\n\r"
			"  asave area      - saves the area being edited\n\r"
			"  asave changed   - saves all changed zones\n\r"
			"  asave world     - saves the world\n\r"
			"  asave helps     - saves the help files\n\r"
			"  asave races     - saves the races file\n\r"
			"  asave skills    - saves the skills file\n\r"
			"  asave socials   - saves the socials file\n\r"
			"  asave randoms   - saves the random tables\n\r"
			"  asave clans     - saves the clan table\n\r"
			"  asave commands  - saves the command table\n\r"
			"  asave classes   - saves the classes file\n\r", ch );
    }

    else if ( !str_cmp( "world", argument ) )
    {
	save_area_list();

	for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
	{
	    REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
	    save_area( pArea, FALSE );
	}
	send_to_char( "You saved the world.\n\r", ch );
    }

    else if ( !str_cmp( "changed", argument ) )
    {
	char buf[MAX_INPUT_LENGTH];

	save_area_list();

	send_to_char( "Saved zones:\n\r", ch );
	sprintf( buf, "None.\n\r" );

	for( pArea = area_first; pArea != NULL; pArea = pArea->next )
	{
	    if ( IS_SET(pArea->area_flags, AREA_CHANGED) )
	    {
		REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
		save_area( pArea, FALSE );
		if ( IS_BUILDER( ch, pArea ) )
		{
		    sprintf( buf, "%s - '%s'\n\r",
			begin_string(pArea->name,24),
			pArea->file_name );
		    send_to_char( buf, ch );
		}
	    }
        }

	if ( !str_cmp( buf, "None.\n\r" ) )
	    send_to_char( buf, ch );

	if ( mud_stat.changed )
	{
	    send_to_char( "Variable data saved.\n\r", ch );
	    save_variable_data( );
	}

	if ( mud_stat.classes_changed )
	{
	    save_classes( );
	    send_to_char( "Class data saved.\n\r", ch );
	}

	if ( mud_stat.clans_changed )
	{
	    save_clans( );
	    send_to_char( "Clan data saved.\n\r", ch );
	}

	if ( mud_stat.helps_changed )
	{
	    save_helps( );
	    send_to_char( "Helps saved.\n\r", ch );
	}

	if ( mud_stat.races_changed )
	{
	    save_races( );
	    send_to_char( "Race data saved.\n\r", ch );
	}

	if ( mud_stat.skills_changed )
	{
	    save_skills( );
	    send_to_char( "Skill data saved.\n\r", ch );
	}

	if ( mud_stat.randoms_changed )
	{
	    send_to_char( "Random tables saved.\n\r", ch );
	    save_random_data( );
	}

	if ( mud_stat.socials_changed )
	{
	    save_social_table( );
	    send_to_char( "Social data saved.\n\r", ch );
	}
    }

    else if ( !str_cmp( argument, "list" ) )
    {
	send_to_char( "Area list saved.\n\r", ch );
	save_area_list( );
    }

    else if ( !str_cmp( argument, "commands" ) )
    {
	send_to_char( "Commmand table saved.\n\r", ch );
	save_commands( );
    }

    else if ( !str_cmp( argument, "area" ) )
    {
	if ( ch->desc->editor == 0 )
	{
	    send_to_char( "You are not editing an area, therefore an area vnum is required.\n\r", ch );
	    return;
	}
	
	switch ( ch->desc->editor )
	{
	    case ED_AREA:
		pArea = (AREA_DATA *)ch->desc->pEdit;
		break;
	    case ED_ROOM:
		pArea = ( (ROOM_INDEX_DATA *)ch->desc->pEdit )->area;
		break;
	    case ED_OBJECT:
		pArea = ( (OBJ_INDEX_DATA *)ch->desc->pEdit )->area;
		break;
	    case ED_MOBILE:
		pArea = ( (MOB_INDEX_DATA *)ch->desc->pEdit )->area;
		break;
	    default:
		pArea = ch->in_room->area;
		break;
	}

	REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
	save_area_list( );
	save_area( pArea, FALSE );
	send_to_char( "Area saved.\n\r", ch );
    }

    else if ( !str_cmp( argument, "clans" ) )
    {
	save_clans( );
	send_to_char( "Clan data saved.\n\r", ch );
    }

    else if ( !str_cmp( argument, "helps" ) )
    {
        save_helps( );
        send_to_char( "Helps Saved.\n\r", ch);
    }

    else if ( !str_cmp( argument, "randoms" ) )
    {
	send_to_char( "Random tables saved.\n\r", ch );
	save_random_data( );
    }

    else if ( !str_cmp( argument, "skills" ) )
    {
	save_skills( );
	send_to_char( "Skill data saved.\n\r", ch );
    }

    else if ( !str_cmp( argument, "races" ) )
    {
	save_races( );
	send_to_char( "Races saved.\n\r", ch);
    }

    else if ( !str_cmp( argument, "socials" ) )
    {
	save_social_table( );
	send_to_char( "Socials saved.\n\r", ch );
    }

    else if ( !str_cmp( argument, "classes" ) )
    {
	save_classes( );
	send_to_char( "Classes saved.\n\r", ch );
    }

    else if ( is_number( argument ) )
    {
	if ( ( pArea = get_area_data( atoi( argument ) ) ) == NULL )
	    send_to_char( "That area does not exist.\n\r", ch );
	else
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Area [%d] %s saved.\n\r",
		pArea->vnum, pArea->name );
	    send_to_char( buf, ch );

	    save_area_list( );
	    save_area( pArea, FALSE );
	}
    }

    else
	do_asave( ch, "" );
}

