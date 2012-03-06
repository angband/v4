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


/*
 * Hack -- instantiate a trap
 *
 * XXX XXX XXX This routine should be redone to reflect trap "level".
 * That is, it does not make sense to have spiked pits at 50 feet.
 * Actually, it is not this routine, but the "trap instantiation"
 * code, which should also check for "trap doors" on quest levels.
 */
void pick_trap(int y, int x)
{
	int feat;

	static const int min_level[] =
	{
		2,		/* Trap door */
		2,		/* Open pit */
		2,		/* Spiked pit */
		2,		/* Poison pit */
		3,		/* Summoning rune */
		1,		/* Teleport rune */
		2,		/* Fire rune */
		2,		/* Acid rune */
		2,		/* Slow rune */
		6,		/* Strength dart */
		6,		/* Dexterity dart */
		6,		/* Constitution dart */
		2,		/* Gas blind */
		1,		/* Gas confuse */
		2,		/* Gas poison */
		2,		/* Gas sleep */
	};

	/* Paranoia */
	if (cave->feat[y][x] != FEAT_INVIS) return;

	/* Pick a trap */
	while (1)
	{
		/* Hack -- pick a trap */
		feat = FEAT_TRAP_HEAD + randint0(16);

		/* Check against minimum depth */
		if (min_level[feat - FEAT_TRAP_HEAD] > p_ptr->depth) continue;

		/* Hack -- no trap doors on quest levels */
		if ((feat == FEAT_TRAP_HEAD + 0x00) && is_quest(p_ptr->depth)) continue;

		/* Hack -- no trap doors on the deepest level */
		if ((feat == FEAT_TRAP_HEAD + 0x00) && (p_ptr->depth >= MAX_DEPTH-1)) continue;

		/* Done */
		break;
	}

	/* Activate the trap */
	cave_set_feat(cave, y, x, feat);
}

/**
 * Returns the index of a "free" monster, or 0 if no slot is available.
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

		/* Count monsters */
		//cave->mon_cnt++;

		return idx;
	}

	/* Warn the player if no index is available 
	 * (except during dungeon creation)
	 */
	if (character_dungeon) {
		msg("Too many traps!");
	}

	/* Try not to crash */
	return 0;
}

void place_trap(struct cave *c, int y, int x) {
	int trap_idx, idx;
	struct trap *t_ptr;
	struct trap *n_ptr;
	struct trap trap_body;
	
	assert(cave_in_bounds(c, y, x));

	/* Remove this when we can have trapped doors etc. */
	assert(cave_isfloor(c, y, x));
	
	/* There is already a trap here */
	if (c->trap[y][x] > 0) return;

	/* Pick a trap */
	/* This should eventually respect min/max depths */
	trap_idx = randint1(z_info->trap_max - 1);

	/* Create a trap of the given type */
	t_ptr = &trap_body;
	(void)WIPE(t_ptr, trap_type);
	
	/* Fill out defaults */
	t_ptr->kind = &trap_info[trap_idx];
	t_ptr->hidden = trap_info[trap_idx].hidden;

	/* Get a new record */
	idx = trap_pop();

	if (!idx) return;

	/* Notify cave of the new trap */
	c->trap[y][x] = idx;

	/* Copy the trap */
	n_ptr = &c->traps[idx];
	COPY(n_ptr, t_ptr, trap_type);
	
	/* Activate the trap */
	// cave_set_trap(cave, y, x, t_ptr);
	
	if (character_dungeon) {
		cave_note_spot(c, y, x);
		cave_light_spot(c, y, x);
	}	
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
	struct trap *t_ptr = &cave->traps[cave->trap[y][x]];

	/* Disturb the player */
	disturb(p_ptr, 0, 0);

	/* Run the effect */
	effect_do(t_ptr->kind->effect, &ident, FALSE, 0, 0, 0);
}

