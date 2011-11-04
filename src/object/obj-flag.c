/*
 * File: src/object/obj-flag.c
 * Purpose: functions to deal with object flags
 *
 * Copyright (c) 2011 Chris Carr
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "angband.h"
#include "randname.h"

/**
 * Details of the different object flags in the game.
 * See src/object/obj-flag.h for structure
 */
struct object_flag object_flag_table[] =
{
    #define OF(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u) \
            { OF_##a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u },
    #include "list-object-flags.h"
    #undef OF
};

const struct object_flag_type flag_type_table[] =
{
    #define OFT(a, b, c) \
            { OFT_##a, b, c },
    #include "list-flag-types.h"
    #undef OF
};

/**
 * Create a "mask" of flags of a specific type or ID threshold.
 *
 * \param f is the flag array we're filling
 * \param id is whether we're masking by ID level
 * \param ... is the list of flags or ID types we're looking for
 *
 * N.B. OFT_MAX must be the last item in the ... list
 */
void create_mask(bitflag *f, bool id, ...)
{
	const struct object_flag *of_ptr;
	int i;
	va_list args;

	of_wipe(f);

	va_start(args, id);

	/* Process each type in the va_args */
    for (i = va_arg(args, int); i != OFT_MAX; i = va_arg(args, int)) {
		for (of_ptr = object_flag_table; of_ptr->index < OF_MAX; of_ptr++)
			if ((id && of_ptr->id == i) || (!id && of_ptr->type == i))
				of_on(f, of_ptr->index);
	}

	va_end(args);

	return;
}

/**
 * Print a message when an object flag is identified by use.
 *
 * \param flag is the flag being noticed
 * \param name is the object name
 */
void flag_message(int flag, char *name)
{
	const struct object_flag *of_ptr = &object_flag_table[flag];

	if (!streq(of_ptr->message, ""))
		msg(of_ptr->message, name);

	return;
}

/**
 * Determine whether a flagset includes any curse flags.
 */
bool cursed_p(bitflag *f)
{
	bitflag f2[OF_SIZE];

	of_wipe(f2);
	create_mask(f2, FALSE, OFT_CURSE, OFT_MAX);

	return of_is_inter(f, f2);
}

/**
 * Determine whether an object flag or its timed equivalent are set in the
 * passed-in flags (which probably come from a state structure). This assumes
 * that there are no p_ptr->timed effects which can be active yet unknown to
 * the player.
 *
 * \param p player to act upon
 * \param flag is the object flag for which we are checking.
 * \param f is the set of flags we're checking
 */
bool check_state(struct player *p, int flag, bitflag *f)
{
	const struct object_flag *of_ptr = &object_flag_table[flag];

	/* Sanity check */
	if (!flag) return FALSE;

	if (of_has(f, flag) || (of_ptr->timed && p->timed[of_ptr->timed]))
		return TRUE;

	return FALSE;
}

/**
 * Log the names of a flagset to a file.
 *
 * \param f is the set of flags we are logging.
 * \param log_file is the file to which we are logging the names.
 */
void log_flags(bitflag *f, ang_file *log_file)
{
	int i;
	const struct object_flag *of_ptr;

	file_putf(log_file, "Object flags are:\n");
	for (i = of_next(f, FLAG_START); i != FLAG_END; i = of_next(f, i + 1)) {
		of_ptr = &object_flag_table[i];
		file_putf(log_file, "%s\n", of_ptr->name);
	}
}

/**
 * Return the name of a flag.
 */
const char *flag_name(int flag)
{
	const struct object_flag *of_ptr = &object_flag_table[flag];

	return of_ptr->name;
}

/**
 * Return the name of a flag's rune.
 */
const char *flag_rune(int flag)
{
	const struct object_flag *of_ptr = &object_flag_table[flag];

	return of_ptr->rune;
}

/**
 * Get the slot multiplier for a flag's power rating
 *
 * \param flag is the flag in question.
 * \param slot is the wield_slot it's in.
 */
s16b slot_mult(int flag, int slot)
{
	const struct object_flag *of_ptr = &object_flag_table[flag];

	switch (slot) {
		case INVEN_WIELD: 	return of_ptr->weapon;
		case INVEN_BOW:		return of_ptr->bow;
		case INVEN_LEFT:
		case INVEN_RIGHT:	return of_ptr->ring;
		case INVEN_NECK:	return of_ptr->amulet;
		case INVEN_LIGHT:	return of_ptr->light;
		case INVEN_BODY:	return of_ptr->body;
		case INVEN_OUTER:	return of_ptr->cloak;
		case INVEN_ARM:		return of_ptr->shield;
		case INVEN_HEAD:	return of_ptr->hat;
		case INVEN_HANDS:	return of_ptr->gloves;
		case INVEN_FEET:	return of_ptr->boots;
		default: 			return 1;
	}
}

/**
 * Return the base power rating for a flag.
 */
s32b flag_power(int flag)
{
	const struct object_flag *of_ptr = &object_flag_table[flag];

	return of_ptr->power;
}

/**
 * Ascertain whether a flag is granular (pval-based) or binary.
 */
bool flag_uses_pval(int flag)
{
	const struct object_flag *of_ptr = &object_flag_table[flag];

	return of_ptr->pval;
}

/**
 * Return the OFT_ type of a flag.
 */
int obj_flag_type(int flag)
{
	const struct object_flag *of_ptr = &object_flag_table[flag];

	return of_ptr->type;
}

/**
 * Return the description of the flag type.
 */
const char *obj_flagtype_name(int of_type)
{
	const struct object_flag_type *oft_ptr = &flag_type_table[of_type];

	return oft_ptr->desc;
}

/**
 * Return the pval weighting of a flag. (Some pvals are more important than
 * others.)
 */
int pval_mult(int flag)
{
	const struct object_flag *of_ptr = &object_flag_table[flag];

	return of_ptr->pval_mult;
}

/**
 * Return the set of flags which are governed by pvals (granular flags).
 */
void create_pval_mask(bitflag *f)
{
	int i;

	of_wipe(f);

	for (i = 0; i < OF_MAX; i++)
		if (flag_uses_pval(i))
			of_on(f, i);

	return;
}

/**
 * Initialise the random rune names, 3 to 8 chars each, in quotes
 */
static char rune_adj[OF_MAX][12];

void init_rune_names(void)
{
	struct object_flag *of_ptr;
	size_t i, j;

	/* Generate the names and put them in the rune_adj array */
    for (i = 0; i < OF_MAX; i++) {
        char buf[12];
        char *end = buf + 1;
        int wordlen = 0;
        bool okay = TRUE;

        strcpy(buf, "\"");
        wordlen = randname_make(RANDNAME_RUNE, 3, 8, end, 9, name_sections);
        buf[wordlen] = '"';
        buf[wordlen + 1] = '\0';

        /* Check this name hasn't already been generated */
        for (j = 0; j < i; j++) {
            if (streq(buf, rune_adj[j])) {
                okay = FALSE;
                break;
            }
        }

        if (okay)
            my_strcpy(rune_adj[i], buf, sizeof(rune_adj[0]));
        else
            /* Have another go at making a name */
            i--;
	}

	/* Now set the of_ptr->rune pointers to the rune_adj array */
	for (i = 0; i < OF_MAX; i++) {
		of_ptr = &object_flag_table[i];
		of_ptr->rune = rune_adj[i];
	}
}
