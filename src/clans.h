#include "olc.h"

DECLARE_DO_FUN( do_say		);

ROSTER_DATA *roster_free;

typedef sh_int	COUNT_FUN	args( ( CHAR_DATA *ch, int type ) );
typedef bool	CLAN_CMD	args( ( CHAR_DATA *ch, char *argument, int type ) );	

#define CLAN_COMMAND( fun )	bool fun( CHAR_DATA *ch, char *argument, int type )

#define DO_NOT_CHARGE	-1
#define COUNT_ALL	0
#define COUNT_GUARD	1
#define COUNT_SHOP	2

#define TO_ROOM_FLAG	0
#define TO_SECTOR	1

#define FLAG_HALL	A
#define FLAG_LEADER	B

struct price_type
{
    char	*name;
    CLAN_CMD	*cmd;
    long	flags;
    sh_int	cost_cubic;
    sh_int	cost_aquest;
    sh_int	cost_iquest;
    sh_int	type;
    COUNT_FUN	*mult;
    char	*notes;
};

struct clan_flag_type
{
    char	*name;
    sh_int	cost_cubic;
    sh_int	cost_aquest;
    sh_int	cost_iquest;
    sh_int	where;
    long	bit;
    long	restrict;
};

struct obj_apply_type
{
    char	*name;
    sh_int	mod;
    sh_int	max;
    sh_int	where;
    sh_int	cost_cubic;
    sh_int	cost_aquest;
    sh_int	cost_iquest;
};

sh_int count_none	args( ( CHAR_DATA *ch, int type ) );
sh_int count_rooms	args( ( CHAR_DATA *ch, int type ) );
sh_int count_mobiles	args( ( CHAR_DATA *ch, int type ) );
sh_int count_object	args( ( CHAR_DATA *ch, int type ) );

const struct price_type		price_table[];
const struct clan_flag_type	mob_flag_table[];
const struct clan_flag_type	room_flag_table[];
const struct clan_flag_type	obj_flag_table[];
const struct clan_flag_type	exit_flag_table[];
const struct obj_apply_type	obj_apply_table[];
