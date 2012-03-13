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
	if (cave->trap_max < z_info->tr_max) {
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
	
	for (i = 1; i <= z_info->trap_max; i++) {
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
 * Pick a level-appropriate trap and put it in the dungeon.
 */
void pick_and_place_trap(struct cave *c, int y, int x) {
	int trap_idx, hidden;
	struct trap *t_ptr;
	struct trap trap_body;
	
	assert(cave_in_bounds(c, y, x));

	/* Remove this when we can have trapped doors etc. */
	assert(cave_isfloor(c, y, x));
	
	/* There is already a trap here */
	if (c->trap[y][x] > 0) return;

	/* Pick a trap */
	trap_idx = get_trap_num(p_ptr->depth);
	
	/* No valid traps */
	if (!trap_idx) return;

	/* Create a trap of the given type */
	t_ptr = &trap_body;
	(void)WIPE(t_ptr, trap_type);
	
	/* Fill out defaults */
	t_ptr->kind = &trap_info[trap_idx];
	t_ptr->x = x;
	t_ptr->y = y;
	
	/* Use trap kind's hidden value +/- 5 */
	hidden = trap_info[trap_idx].hidden;
	if (hidden == 0) {
		/* Special case -- hidden = 0 means never hidden */
		t_ptr->hidden = 0;
	} else {
		t_ptr->hidden = hidden + randint1(11) - 6;
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

void remove_trap(struct cave *c, int y, int x) {
	c->trap[y][x] = 0;

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

