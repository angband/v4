/*
 * File: trap.c
 * Purpose: Trap triggering, selection, and placement
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
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
#include "monster/melee2.h"
#include "cave.h"
#include "effects.h"
#include "spells.h"
#include "trap.h"

/*
 * Determine if a trap affects the player.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match trap power against player armor.
 */
bool trap_check_hit(int power)
{
	return mon_test_hit(power, p_ptr->state.ac + p_ptr->state.to_a);
}

/**
 * Returns the index of a "free" trap, or 0 if no slot is available.
 *
 * This routine should almost never fail, but it *can* happen.
 * The calling code must check for and handle a 0 return.
 */
static s16b trap_pop(void)
{
	int idx;

	/* Normal allocation */
	if (cave->trap_max < z_info->trap_max) {
		/* Get the next hole */
		idx = cave->trap_max;

		/* Expand the array */
		cave->trap_max++;

		return idx;
	}

	/* Warn the player if no index is available 
	 * (except during dungeon creation)
	 */
	if (character_dungeon)
		msg("Too many traps!");

	/* Try not to crash */
	return 0;
}

/** 
 * Pick a level-appropriate trap.
 */
int get_trap_num(int level) {
	int trap_count, trap_idx, i;
	
	trap_count = 0;
	trap_idx = 0;
	
	for (i = 1; i < z_info->trap_kind_max; i++) {
		if (trap_info[i].min_level <= level && trap_info[i].max_level >= level) {
			trap_count++;
			if (one_in_(trap_count))
				trap_idx = i;
		}
	}

	return trap_idx;
}

/**
 * Place the given trap in the dungeon.
 */
void place_trap(struct cave *c, struct trap *t_ptr) {
	int idx;
	int y = t_ptr->y;
	int x = t_ptr->x;
	
	struct trap *n_ptr;
	
	/* Make sure there's not already a trap here */
	if (c->trap[y][x] != 0) return;
	
	/* Get a new record */
	idx = trap_pop();
	if (!idx) return;

	/* Notify cave of the new trap */
	c->trap[y][x] = idx;

	/* Copy the trap */
	n_ptr = &c->traps[idx];
	COPY(n_ptr, t_ptr, trap_type);
	
	if (character_dungeon) {
		cave_note_spot(c, y, x);
		cave_light_spot(c, y, x);
	}	
}

/**
 * Returns a depth-appropriate modifer to the base hiddenness rating of a trap
 */
int trap_hide_modifier(int level) {

	if (level < 36) {
		/* Pre statgain */
		return level / 2;
	} else if (level < 72) {
		/* During statgain -- we assume +1 INT or WIS every level. */
		return (level / 2) + (level - 36) / 2;
	} else {
		/* Post statgain */
		return (level / 2) + 18;
	}
	
}

/**
 * Pick a level-appropriate trap and put it in the dungeon.
 */
void pick_and_place_trap(struct cave *c, int y, int x, int level) {
	int trap_idx, hidden;
	struct trap *t_ptr;
	struct trap trap_body;
	
	assert(cave_in_bounds(c, y, x));

	/* Remove this when we can have trapped doors etc. */
	assert(cave_isfloor(c, y, x));
	
	/* Make sure there's not already a trap here */
	if (c->trap[y][x] != 0) return;

	/* Pick a trap */
	trap_idx = get_trap_num(level);
	
	/* No valid traps */
	if (!trap_idx) return;

	/* Create a trap of the given type */
	t_ptr = &trap_body;
	(void)WIPE(t_ptr, trap_type);
	
	/* Fill out defaults */
	t_ptr->kind = &trap_info[trap_idx];
	t_ptr->x = x;
	t_ptr->y = y;
	
	hidden = trap_info[trap_idx].hidden;
	if (hidden == 0) {
		/* Special case -- hidden = 0 means never hidden */
		t_ptr->hidden = 0;
	} else {
		t_ptr->hidden = trap_hide_modifier(level) + Rand_normal(hidden, 3);
	}

	place_trap(c, t_ptr);
}

void reveal_trap(struct cave *c, int y, int x) {
	struct trap *t_ptr;

	assert(c->trap[y][x] > 0);
	
	t_ptr = cave_trap_at(c, y, x);
	t_ptr->hidden = 0;
	
	cave_light_spot(c, y, x);
}

/**
 * Move a trap from index i1 to index i2 in the trap list.
 */
static void compact_traps_aux(int i1, int i2)
{
	int y, x;

	trap_type *t_ptr;

	/* Do nothing */
	if (i1 == i2) return;

	/* Old trap */
	t_ptr = cave_trap(cave, i1);
	y = t_ptr->y;
	x = t_ptr->x;

	/* Update the cave */
	cave->trap[y][x] = i2;
	
	/* Hack -- move monster */
	COPY(cave_trap(cave, i2), cave_trap(cave, i1), struct trap);

	/* Hack -- wipe hole */
	(void)WIPE(cave_trap(cave, i1), trap_type);
}

/**
 * Compacts and reorders the trap list.
 */
void compact_traps()
{
	int t_idx;

	/* Excise disarmed traps (backwards!) */
	for (t_idx = cave->trap_max - 1; t_idx >= 1; t_idx--) {
		trap_type *t_ptr = cave_trap(cave, t_idx);

		/* Skip real traps */
		if (t_ptr->x && t_ptr->y) continue;

		/* Move last trap into open hole */
		compact_traps_aux(cave->trap_max - 1, t_idx);

		/* Compress "cave->mon_max" */
		cave->trap_max--;
	}
}

void wipe_trap_list(struct cave *c) {
	int t_idx;

	for (t_idx = cave->trap_max - 1; t_idx >= 1; t_idx--) {
		trap_type *t_ptr = cave_trap(cave, t_idx);

		(void)WIPE(t_ptr, trap_type);
	}

	/* Reset "cave->trap_max" */
	cave->trap_max = 1;
}


void remove_trap(struct cave *c, int y, int x) {
	struct trap *t_ptr = cave_trap_at(c, y, x);

	c->trap[y][x] = 0;

	/* Wipe the trap */
	(void)WIPE(t_ptr, trap_type);

	if (character_dungeon) {
		cave_light_spot(c, y, x);
	}
}

/*
 * Handle player hitting a real trap
 */
void hit_trap(int y, int x)
{
	bool ident;
	struct trap *t_ptr = cave_trap_at(cave, y, x);

	/* Disturb the player */
	disturb(p_ptr, 0, 0);

	/* Run the effect */
	effect_do(t_ptr->kind->effect, &ident, FALSE, 0, 0, 0);
}

