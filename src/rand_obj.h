typedef struct	random_mod	MOD_DATA;
typedef struct	randoms_data	RANDOM_DATA;

RANDOM_DATA *prefix_table, *suffix_table;
MOD_DATA *random_mod_free;

int maxPrefix, maxSuffix;

struct excep_apply_data
{
    sh_int	apply_type;
    sh_int	min;
    sh_int	max;
};

struct random_mod
{
    MOD_DATA *next;
    sh_int	where;
    sh_int	location;
    sh_int	min;
    sh_int	max;
};

struct randoms_data
{
    MOD_DATA	*mods;
    char	*name;
    sh_int	level;
    sh_int	align;
    long	affect;
    long	shield;
};

sh_int	random_lookup		args( ( RANDOM_DATA *table, char *argument ) );
void	show_random_table	args( ( CHAR_DATA *ch, RANDOM_DATA *table ) );
