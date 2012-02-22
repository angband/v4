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
#define SLAY_CACHE_SIZE	257
static struct flag_cache **slay_cache;


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
	int i, bestmult = 0, oldbest = 0;

	for (i = 1; i < SL_MAX; i++) {
		const struct slay *s_ptr = &slay_table[i];
		oldbest = bestmult;

		/* If it's a brand the monster doesn't resist or a matching slay */
		if (((s_ptr->brand && !rf_has(r_ptr->flags, s_ptr->resist_flag)) ||
				(s_ptr->monster_flag && rf_has(r_ptr->flags,
				s_ptr->monster_flag))) && mult[i] > 0) {
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
		if (s_ptr->vuln_flag && rf_has(r_ptr->flags, s_ptr->vuln_flag) &&
				mult[i] != 0) {
			if (real) {
				of_on(learn_flags, s_ptr->object_flag);
				if (m_ptr->ml)
					rf_on(l_ptr->flags, s_ptr->vuln_flag);
			}
			if (mult[i] + 100 > bestmult)
				bestmult = mult[i] + 100;
		}

		/* use this slay if it's better than the previous best */
		if (bestmult > oldbest)
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
 * Hash the slay multipliers passed in
 *
 * DJB2 hash function from comp.lang.c attributed to Dan Bernstein
 */
static size_t slay_hash(s16b mult[], size_t length, size_t table_size)
{
	unsigned long hash = 5381;
	size_t i;

	for (i = 0; i < length; i++) {
		hash = ((hash << 5) + hash) + mult[i];
	}
	return hash % table_size;
}


/**
 * Compare two sets of slay multipliers
 *
 * \return True if they are equal, False if they are not
 */
static bool slay_match(s16b mults1[], s16b mults2[])
{
	int i;

	for (i = 0; i < SL_MAX; i++)
		if (mults1[i] != mults2[i])
			return false;

	return true;
}


/**
 * Check the slay cache for a combination of slays and return a slay value
 *
 * \param mult is the set of slay multipliers to look for
 *
 * \return 0 if an entry was not found
 *         the cached value if an entry was found
 */
u32b check_slay_cache(s16b mult[])
{
	size_t hash;
	struct flag_cache *entry;

	hash = slay_hash(mult, SL_MAX, SLAY_CACHE_SIZE);
	entry = slay_cache[hash];

	while (entry != NULL) {
		if (slay_match(mult, entry->mults))
			break;
		entry = entry->next;
	}

	if (entry) {
		return entry->value;
	} else {
		return 0;
	}
}

/**
 * Fill in a value in the slay cache. Return TRUE if a change is made.
 *
 * \param index is the set of slay flags whose value we are adding
 * \param value is the value of the slay flags in index
 */
void add_slay_cache(s16b mult[], u32b value)
{
	int i;
	size_t hash;
	struct flag_cache **entry_p;

	hash = slay_hash(mult, SL_MAX, SLAY_CACHE_SIZE);
	entry_p = &slay_cache[hash];

	while (*entry_p)
		entry_p = &((*entry_p)->next);

	*entry_p = C_ZNEW(1, struct flag_cache);
	(*entry_p)->value = value;
	(*entry_p)->next = NULL;
	for (i = 0; i < SL_MAX; i++)
		(*entry_p)->mults[i] = mult[i];
}

/**
 * Create a cache ready for slay combinations found on ego items.
 *
 * The power values corresponding to these combinations will be populated at
 * a later date. This is to speed up slay_power(), which will be called
 * many times for ego items during the game.
 *
 * The cache is a hash table, with linked lists of entries at each hash
 * position, each entry initialised to NULL
 */
errr create_slay_cache(void)
{
    /* Allocate slay_cache */
    slay_cache = C_ZNEW(SLAY_CACHE_SIZE, struct flag_cache *);

    /* Success */
    return 0;
}

void free_slay_cache(void)
{
	int i;
	struct flag_cache *entry;
	struct flag_cache *prev;

	for (i = 0; i < SLAY_CACHE_SIZE; i++) {
		entry = slay_cache[i];
		while (entry) {
			prev = entry;
			entry = entry->next;
			mem_free(prev);
		}
	}
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
 * Return the slay from a given slay index
 */
const struct slay *lookup_slay_by_index(int index)
{
	const struct slay *s_ptr = &slay_table[index];

	assert(index < SL_MAX);

	return s_ptr;
}

/**
 * Check the slays on an object and update the mult[] array if any of the
 * object's slay pvals are better than those already in the array. Returns
 * TRUE if changes were made.
 * \param o_ptr is the object we're checking
 * \param mult[] is the array of mults that we're updating - it must be at
 * least SL_MAX in size
 */
bool object_slay_mults(const object_type *o_ptr, s16b mult[])
{
	size_t i;
	bitflag slay_flags[OF_SIZE], allslays[OF_SIZE];
	bool changed = FALSE;
	int newmult = 0;

	/* Find the slay flags on this object */
	create_mask(allslays, FALSE, OFT_SLAY, OFT_BRAND, OFT_HURT, OFT_MAX);
	of_copy(slay_flags, o_ptr->flags);
	of_inter(slay_flags, allslays);

	/* Cycle through them */
	for (i = of_next(slay_flags, FLAG_START); i != FLAG_END;
			i = of_next(slay_flags, i + 1)) {
		const struct slay *s_ptr = lookup_slay(i);

		/* Disallow forbidden off-weapon slays */
		if (wield_slot(o_ptr) > INVEN_BOW && wield_slot(o_ptr) < INVEN_TOTAL
				&& !s_ptr->nonweap) continue;

		/* Use -1 for HURT flags, which don't use a pval for modifying damage */
		if (obj_flag_type(s_ptr->object_flag) == OFT_HURT) {
			mult[s_ptr->index] = -1;
			continue;
		}

		/* Use the multiplier if it's higher than the existing one */
		newmult = o_ptr->pval[which_pval(o_ptr, i)];
		if (newmult > mult[s_ptr->index]) {
			mult[s_ptr->index] = newmult;
			changed = TRUE;
		}
	}

	return changed;
}
