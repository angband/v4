/*
 * File: slays.c
 * Purpose: encapsulation of slay_table and accessor functions for slays/brands
 *
 * Copyright (c) 2010 Chris Carr and Peter Denison
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
#include "object/pval.h"

/**
 * Info about slays (see src/slays.h for structure)
 */
const struct slay slay_table[] =
{
	#define SLAY(a, b, c, d, e, f, g, h, i, j, k) \
		{ SL_##a, b, c, d, e, f, g, h, i, j, k},
	#include "list-slays.h"
	#undef SLAY
};

/**
 * Cache of slay values (for object_power)
 */
static struct flag_cache *slay_cache;


/**
 * Get a random slay (or brand).
 * We use randint1 because the first entry in slay_table is null.
 *
 * \param mask is the set of slays from which we are choosing.
 */
const struct slay *random_slay(const bitflag mask[OF_SIZE])
{
	const struct slay *s_ptr;
	do {
		s_ptr = &slay_table[randint1(SL_MAX - 1)];
	} while (!of_has(mask, s_ptr->object_flag));

	return s_ptr;
}


/**
 * Match slays in flags against a chosen flag mask
 *
 * count is the number of matches
 * \param flags is the flagset to analyse for matches
 * \param mask is the flagset against which to test
 * \param desc[] is the array of descriptions of matching slays - can be null
 * \param brand[] is the array of descriptions of brands - can be null
 *
 * desc[] and brand[] must be >= SL_MAX in size
 */
int list_slays(const bitflag flags[OF_SIZE], const bitflag mask[OF_SIZE],
	const char *desc[], const char *brand[])
{
	int i, count = 0;
	bitflag f[OF_SIZE];

	/* We are only interested in the flags specified in mask */
	of_copy(f, flags);
	of_inter(f, mask);

	/* Collect slays */
	for (i = 0; i < SL_MAX; i++) {
		const struct slay *s_ptr = &slay_table[i];
		if (of_has(f, s_ptr->object_flag)) {
			if (brand)
				brand[count] = s_ptr->brand;
			if (desc)
				desc[count] = s_ptr->desc;
			count++;
		}
	}

	return count;
}


/**
 * Notice any slays on a particular object which are in mask.
 *
 * \param o_ptr is the object on which we are noticing slays
 * \param mask is the flagset within which we are noticing them
 */
void object_notice_slays(object_type *o_ptr, const bitflag mask[OF_SIZE])
{
	bool learned;
	bitflag f[OF_SIZE];
	char o_name[40];
	int i;

	/* We are only interested in the flags specified in mask */
	object_flags(o_ptr, f);
	of_inter(f, mask);

	/* if you learn a slay, print a message */
/* CC: this should use of_next over o_ptr->flags */
	for (i = 0; i < SL_MAX; i++) {
		const struct slay *s_ptr = &slay_table[i];
		if (of_has(f, s_ptr->object_flag)) {
			learned = object_notice_flag(o_ptr, s_ptr->object_flag, TRUE);
			if (EASY_LEARN && learned) {
				object_desc(o_name, sizeof(o_name), o_ptr, ODESC_BASE);
				msg("Your %s %s!", o_name, s_ptr->active_verb);
			}
		}
	}

	object_check_for_ident(o_ptr);
}


/**
 * Extract the multiplier from a given mult array against a given monster.
 *
 * \param mult[] is the array of slay multipliers (must be >= SL_MAX)
 * \param m_ptr is the monster being attacked
 * \param best_s_ptr is the best applicable slay_table entry, or NULL if no
 *  slay already known
 * \param real is whether this is a real attack (where we learn stuff) or a sim
 * \param learn_flags is the set of object flags we've learned (can be NULL)
 */
void improve_attack_modifier(s16b mult[], const monster_type *m_ptr,
	const struct slay **best_s_ptr, bitflag *learn_flags, bool real)
{
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];
	int i, bestmult = 100, oldbest = 0;

	for (i = 0; i < SL_MAX; i++) {
		const struct slay *s_ptr = &slay_table[i];
		oldbest = bestmult;

		/* If it's a brand the monster doesn't resist or a matching slay */
		if (((s_ptr->brand && !rf_has(r_ptr->flags, s_ptr->resist_flag)) ||
				(s_ptr->monster_flag && rf_has(r_ptr->flags,
				s_ptr->monster_flag))) && mult[i] > 100) {
			/* In a real attack, learn about object and monster flags */
			if (real) {
				of_on(learn_flags, s_ptr->object_flag);
				if (m_ptr->ml) {
					if (s_ptr->monster_flag)
						rf_on(l_ptr->flags, s_ptr->monster_flag);
					if (s_ptr->resist_flag)
						rf_on(l_ptr->flags, s_ptr->resist_flag);
				}
			}
			if (mult[i] > bestmult)
				bestmult = mult[i];
		}

		/* If the monster is explicitly vulnerable, mult will be 1x higher */
		if ((s_ptr->vuln_flag && rf_has(r_ptr->flags, s_ptr->vuln_flag)) &&
				mult[i] > 100) {
			if (real) {
				of_on(learn_flags, s_ptr->object_flag);
				if (m_ptr->ml)
					rf_on(l_ptr->flags, s_ptr->vuln_flag);
			}
			if (mult[i] + 100 > bestmult)
				bestmult = mult[i] + 100;
		}

		/* use this slay if it's better than the previous best */
		if (*best_s_ptr == NULL || bestmult > oldbest)
			*best_s_ptr = s_ptr;
	}
}


/**
 * React to slays which hurt a monster
 *
 * \param obj_flags is the set of flags we're testing for slays
 * \param mon_flags is the set of flags we're adjusting as a result
 */
void react_to_slay(bitflag *obj_flags, bitflag *mon_flags)
{
	int i;
	for (i = 0; i < SL_MAX; i++) {
		const struct slay *s_ptr = &slay_table[i];
		if (of_has(obj_flags, s_ptr->object_flag) && s_ptr->monster_flag)
			rf_on(mon_flags, s_ptr->monster_flag);
	}
}


/**
 * Check the slay cache for a combination of slays and return a slay value
 *
 * \param index is the set of slay flags to look for
 */
s32b check_slay_cache(bitflag *index)
{
	int i;

	for (i = 0; !of_is_empty(slay_cache[i].flags); i++)
		if (of_is_equal(index, slay_cache[i].flags)) break;

	return slay_cache[i].value;
}


/**
 * Fill in a value in the slay cache. Return TRUE if a change is made.
 *
 * \param index is the set of slay flags whose value we are adding
 * \param value is the value of the slay flags in index
 */
bool fill_slay_cache(bitflag *index, s32b value)
{
	int i;

	for (i = 0; !of_is_empty(slay_cache[i].flags); i++) {
		if (of_is_equal(index, slay_cache[i].flags)) {
			slay_cache[i].value = value;
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Create a cache of slay combinations found on ego items, and the values of
 * these combinations. This is to speed up slay_power(), which will be called
 * many times for ego items during the game.
 *
 * \param items is the set of ego types from which we are extracting slay
 * combinations
 */
errr create_slay_cache(struct ego_item *items)
{
    int i;
    int j;
    int count = 0;
    bitflag cacheme[OF_SIZE];
    bitflag slay_mask[OF_SIZE];
    bitflag **dupcheck;
    ego_item_type *e_ptr;

    /* Build the slay mask */
	create_mask(slay_mask, FALSE, OFT_SLAY, OFT_KILL, OFT_BRAND, OFT_MAX);

    /* Calculate necessary size of slay_cache */
    dupcheck = C_ZNEW(z_info->e_max, bitflag *);

    for (i = 0; i < z_info->e_max; i++) {
        dupcheck[i] = C_ZNEW(OF_SIZE, bitflag);
        e_ptr = items + i;

        /* Find the slay flags on this ego */
        of_copy(cacheme, e_ptr->flags);
        of_inter(cacheme, slay_mask);

        /* Only consider non-empty combinations of slay flags */
        if (!of_is_empty(cacheme)) {
            /* Skip previously scanned combinations */
            for (j = 0; j < i; j++)
                if (of_is_equal(cacheme, dupcheck[j])) continue;

            /* msg("Found a new slay combo on an ego item"); */
            count++;
            of_copy(dupcheck[i], cacheme);
        }
    }

    /* Allocate slay_cache with an extra empty element for an iteration stop */
    slay_cache = C_ZNEW((count + 1), struct flag_cache);
    count = 0;

    /* Populate the slay_cache */
    for (i = 0; i < z_info->e_max; i++) {
        if (!of_is_empty(dupcheck[i])) {
            of_copy(slay_cache[count].flags, dupcheck[i]);
            slay_cache[count].value = 0;
            count++;
            /*msg("Cached a slay combination");*/
        }
    }

    for (i = 0; i < z_info->e_max; i++)
        FREE(dupcheck[i]);
    FREE(dupcheck);

    /* Success */
    return 0;
}

void free_slay_cache(void)
{
	mem_free(slay_cache);
}


/**
 * Return whether a given flagset contains a flag which hurts this
 * monster
 */
bool obj_hurts_mon(bitflag *flags, const monster_type *m_ptr)
{
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	size_t i;

	for (i = 0; i < SL_MAX; i++) {
		const struct slay *s_ptr = &slay_table[i];
		if (of_has(flags, s_ptr->object_flag) && ((s_ptr->monster_flag
				&& rf_has(r_ptr->flags,	s_ptr->monster_flag)) ||
				(s_ptr->resist_flag && !rf_has(r_ptr->flags,
				s_ptr->resist_flag)) || (s_ptr->vuln_flag &&
				rf_has(r_ptr->flags, s_ptr->vuln_flag))))
			return TRUE;
	}
	return FALSE;
}


/**
 * Return the slay from a given slay flag
 */
const struct slay *lookup_slay(int flag)
{
	size_t i;

	for (i = 0; i < SL_MAX; i++) {
		const struct slay *s_ptr = &slay_table[i];
		if (s_ptr->object_flag == flag)
			return s_ptr;
	}

	msg("Illegal flag in lookup_slay");
	assert(0);
}

/**
 * Check the slays on an object and update the mult[] array if any of the
 * object's slay pvals are better than those already in the array. Returns
 * TRUE if changes were made.
 * \param o_ptr is the object we're checking
 * \param mult[] is the array of mults that we're updating - it must be at
 * least SL_MAX in size
 */
bool object_slay_mults(object_type *o_ptr, s16b mult[])
{
	size_t i;
	bitflag slay_flags[OF_SIZE], allslays[OF_SIZE];
	bool changed = FALSE;
	int newmult = 0;

	/* Find the slay flags on this object */
	create_mask(allslays, FALSE, OFT_SLAY, OFT_BRAND, OFT_MAX);
	of_copy(slay_flags, o_ptr->flags);
	of_inter(slay_flags, allslays);

	/* Cycle through them */
	for (i = of_next(slay_flags, FLAG_START); i != FLAG_END;
			i = of_next(slay_flags, i + 1)) {
		const struct slay *s_ptr = lookup_slay(i);

		/* Disallow forbidden off-weapon slays */
		if (wield_slot(o_ptr) > INVEN_BOW && wield_slot(o_ptr) < INVEN_TOTAL
				&& !s_ptr->nonweap) continue;

		/* Use the multiplier if it's higher than the existing one */
		newmult = o_ptr->pval[which_pval(o_ptr, i)];
		if (newmult > mult[s_ptr->index]) {
			mult[s_ptr->index] = newmult;
			changed = TRUE;
		}
	}

	return changed;
}
