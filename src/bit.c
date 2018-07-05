/***************************************************************************
 *  File: bit.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was written by Jason Dinkel and inspired by Russ Taylor,     *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 The code below uses a table lookup system that is based on suggestions
 from Russ Taylor.  There are many routines in handler.c that would benefit
 with the use of tables.  You may consider simplifying your code base by
 implementing a system like below with such functions. -Jason Dinkel
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

struct flag_stat_type
{
    const struct flag_type *structure;
    bool stat;
};

const struct flag_stat_type flag_stat_table[] =
{
    {	area_flags,		FALSE	},
    {	door_resets,		FALSE	},
    {   sex_flags,		TRUE	},
    {   room_flags,		FALSE	},
    {   sector_type,		TRUE	},
    {   type_flags,		TRUE	},
    {   extra_flags,		FALSE	},
    {   wear_flags,		FALSE	},
    {   act_flags,		FALSE	},
    {   affect_flags,		FALSE	},
    {	shield_flags,		FALSE	},
    {   apply_flags,		TRUE	},
    {   wear_loc_flags,		TRUE	},
    {   wear_loc_strings,	TRUE	},
    {   container_flags,	FALSE	},
    {   part_flags,             FALSE   },
    {   ac_type,                TRUE    },
    {   size_flags,             TRUE    },
    {   position_flags,         TRUE    },
    {   weapon_class,           TRUE    },
    {   weapon_type2,           FALSE   },
    {   apply_types,		TRUE	},
    {	portal_flags,		FALSE	},
    {	exit_flags,		FALSE	},
    {   0,			0	}
};
    
bool is_stat( const struct flag_type *flag_table )
{
    int flag;

    for (flag = 0; flag_stat_table[flag].structure; flag++)
    {
	if ( flag_stat_table[flag].structure == flag_table
	  && flag_stat_table[flag].stat )
	    return TRUE;
    }
    return FALSE;
}

long flag_lookup2 (const char *name, const struct flag_type *flag_table)
{
    int flag;
 
    for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
        if ( !str_prefix( name, flag_table[flag].name )
          && flag_table[flag].settable )
            return flag_table[flag].bit;
    }
 
    return NO_FLAG;
}

long flag_value( const struct flag_type *flag_table, char *argument )
{
    char word[MAX_INPUT_LENGTH];
    long bit, marked = 0;
    bool found = FALSE;

    if ( is_stat( flag_table ) )
    {
	one_argument( argument, word );

	return flag_lookup2( word, flag_table );
    }

    for (; ;)
    {
        argument = one_argument( argument, word );

        if ( word[0] == '\0' )
	    break;

        if ( ( bit = flag_lookup2( word, flag_table ) ) != NO_FLAG )
        {
            SET_BIT( marked, bit );
            found = TRUE;
        }
    }

    if ( found )
	return marked;
    else
	return NO_FLAG;
}

char *flag_string( const struct flag_type *flag_table, long bits )
{
    static char buf[512];
    int flag;

    buf[0] = '\0';

    for ( flag = 0; flag_table[flag].name != NULL; flag++ )
    {
	if ( !is_stat( flag_table ) && IS_SET( bits, flag_table[flag].bit ) )
	{
	    strcat( buf, ", " );
	    strcat( buf, flag_table[flag].name );
	}
	else if ( flag_table[flag].bit == bits )
	{
	    strcat( buf, ", " );
	    strcat( buf, flag_table[flag].name );
	    break;
	}
    }

    return ( buf[0] != '\0' ) ? buf+2 : "none";
}


