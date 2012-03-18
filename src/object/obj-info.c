/**
 * File: obj-info.c
 * Purpose: Object description code.
 *
 * Copyright (c) 2010 Andi Sidwell
 * Copyright (c) 2004 Robert Ruehlmann
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
#include "attack.h"
#include "effects.h"
#include "cmds.h"
#include "object/tvalsval.h"
#include "z-textblock.h"
#include "object/slays.h"
#include "object/pval.h"

/*
 * Describes a flag-name pair.
 */
typedef struct
{
	int flag;
	const char *name;
} flag_type;



/*** Utility code ***/

/*
 * Given an array of strings, as so:
 *  { "intelligence", "fish", "lens", "prime", "number" },
 *
 * ... output a list like "intelligence, fish, lens, prime, number.\n".
 */
static void info_out_list(textblock *tb, const char *list[], size_t count)
{
	size_t i;

	for (i = 0; i < count; i++)
	{
		textblock_append(tb, list[i]);
		if (i != (count - 1)) textblock_append(tb, ", ");
	}

	textblock_append(tb, ".\n");
}


/*
 *
 */
static size_t info_collect(textblock *tb, const flag_type list[], size_t max,
		const bitflag flags[OF_SIZE], const char *recepticle[])
{
	size_t i, count = 0;

	for (i = 0; i < max; i++)
	{
		if (of_has(flags, list[i].flag))
			recepticle[count++] = list[i].name;
	}

	return count;
}


/*** Big fat data tables ***/

static const flag_type pval_flags[] =
{
	{ OF_STR,     "strength" },
	{ OF_INT,     "intelligence" },
	{ OF_WIS,     "wisdom" },
	{ OF_DEX,     "dexterity" },
	{ OF_CON,     "constitution" },
	{ OF_CHR,     "charisma" },
	{ OF_STEALTH, "stealth" },
	{ OF_INFRA,   "infravision" },
	{ OF_TUNNEL,  "tunneling" },
	{ OF_SPEED,   "speed" },
	{ OF_BLOWS,   "attack speed" },
	{ OF_SHOTS,   "shooting speed" },
	{ OF_MIGHT,   "shooting power" },
	{ OF_LIGHT,   "light radius" },
};

static const flag_type immunity_flags[] =
{
	{ OF_IM_ACID, "acid" },
	{ OF_IM_ELEC, "lightning" },
	{ OF_IM_FIRE, "fire" },
	{ OF_IM_COLD, "cold" },
};

static const flag_type vuln_flags[] =
{
	{ OF_VULN_ACID, "acid" },
	{ OF_VULN_ELEC, "electricity" },
	{ OF_VULN_FIRE, "fire" },
	{ OF_VULN_COLD, "cold" },
};

static const flag_type resist_flags[] =
{
	{ OF_RES_ACID,  "acid" },
	{ OF_RES_ELEC,  "lightning" },
	{ OF_RES_FIRE,  "fire" },
	{ OF_RES_COLD,  "cold" },
	{ OF_RES_POIS,  "poison" },
	{ OF_RES_LIGHT, "light" },
	{ OF_RES_DARK,  "dark" },
	{ OF_RES_SOUND, "sound" },
	{ OF_RES_SHARD, "shards" },
	{ OF_RES_NEXUS, "nexus"  },
	{ OF_RES_NETHR, "nether" },
	{ OF_RES_CHAOS, "chaos" },
	{ OF_RES_DISEN, "disenchantment" },
};

static const flag_type protect_flags[] =
{
	{ OF_RES_FEAR,  "fear" },
	{ OF_RES_BLIND, "blindness" },
	{ OF_RES_CONFU, "confusion" },
	{ OF_RES_STUN,  "stunning" },
};

static const flag_type ignore_flags[] =
{
	{ OF_IGNORE_ACID, "acid" },
	{ OF_IGNORE_ELEC, "electricity" },
	{ OF_IGNORE_FIRE, "fire" },
	{ OF_IGNORE_COLD, "cold" },
};

static const flag_type hates_flags[] =
{
	{ OF_HATES_ACID, "acid" },
	{ OF_HATES_ELEC, "electricity" },
	{ OF_HATES_FIRE, "fire" },
	{ OF_HATES_COLD, "cold" },
};

static const flag_type sustain_flags[] =
{
	{ OF_SUST_STR, "strength" },
	{ OF_SUST_INT, "intelligence" },
	{ OF_SUST_WIS, "wisdom" },
	{ OF_SUST_DEX, "dexterity" },
	{ OF_SUST_CON, "constitution" },
	{ OF_SUST_CHR, "charisma" },
};

static const flag_type misc_flags[] =
{
	{ OF_BLESSED, "Blessed by the gods" },
	{ OF_SLOW_DIGEST, "Slows your metabolism" },
	{ OF_IMPAIR_HP, "Impairs hitpoint recovery" },
	{ OF_IMPAIR_MANA, "Impairs mana recovery" },
	{ OF_AFRAID, "Makes you afraid of melee, and worse at shooting and casting spells" },
	{ OF_FEATHER, "Feather Falling" },
	{ OF_REGEN, "Speeds regeneration" },
	{ OF_FREE_ACT, "Prevents paralysis" },
	{ OF_HOLD_LIFE, "Sustains your life force" },
	{ OF_TELEPATHY, "Grants telepathy" },
	{ OF_SEE_INVIS, "Grants the ability to see invisible things" },
	{ OF_AGGRAVATE, "Aggravates creatures nearby" },
	{ OF_DRAIN_EXP, "Drains experience" },
	{ OF_TELEPORT, "Induces random teleportation" },
};


/*** Code that makes use of the data tables ***/

/*
 * Describe an item's curses.
 */
static bool describe_curses(textblock *tb, const object_type *o_ptr,
		const bitflag flags[OF_SIZE])
{
	if (of_has(flags, OF_PERMA_CURSE))
		textblock_append_c(tb, TERM_L_RED, "Permanently cursed.\n");
	else if (of_has(flags, OF_HEAVY_CURSE))
		textblock_append_c(tb, TERM_L_RED, "Heavily cursed.\n");
	else if (of_has(flags, OF_LIGHT_CURSE))
		textblock_append_c(tb, TERM_L_RED, "Cursed.\n");
	else
		return FALSE;

	return TRUE;
}


/*
 * Describe stat modifications.
 */
static bool describe_stats(textblock *tb, const object_type *o_ptr,
		bitflag flags[MAX_PVALS][OF_SIZE], oinfo_detail_t mode)
{
	const char *descs[N_ELEMENTS(pval_flags)];
	size_t count, i;
	bool full = mode & OINFO_FULL;
	bool dummy = mode & OINFO_DUMMY;
	bool search = FALSE;

	if (!o_ptr->num_pvals && !dummy)
		return FALSE;

	for (i = 0; i < o_ptr->num_pvals; i++) {
		count = info_collect(tb, pval_flags, N_ELEMENTS(pval_flags), flags[i], descs);

		if (count) {
			if ((object_this_pval_is_visible(o_ptr, i) || full) && !dummy)
				textblock_append_c(tb, (o_ptr->pval[i] > 0) ? TERM_L_GREEN : TERM_RED,
					"%+i ", o_ptr->pval[i]);
			else
				textblock_append(tb, "Affects your ");

			info_out_list(tb, descs, count);
		}
		if (of_has(flags[i], OF_SEARCH))
			search = TRUE;
	}

	if (search)	{
		if ((object_this_pval_is_visible(o_ptr, which_pval(o_ptr, OF_SEARCH))
				|| full) && !dummy)	{
			textblock_append_c(tb, (o_ptr->pval[which_pval(o_ptr, OF_SEARCH)] > 0) ? TERM_L_GREEN : TERM_RED,
				"%+i%% ", o_ptr->pval[which_pval(o_ptr, OF_SEARCH)] * 5);
			textblock_append(tb, "to searching.\n");
		}
		else if (count)
			textblock_append(tb, "Also affects your searching skill.\n");
		else
			textblock_append(tb, "Affects your searching skill.\n");
	}

	return TRUE;
}


/*
 * Describe immunities granted by an object.
 */
static bool describe_immune(textblock *tb, const bitflag flags[OF_SIZE])
{
	const char *i_descs[N_ELEMENTS(immunity_flags)];
	const char *r_descs[N_ELEMENTS(resist_flags)];
	const char *p_descs[N_ELEMENTS(protect_flags)];
	const char *v_descs[N_ELEMENTS(vuln_flags)];
	size_t count;

	bool prev = FALSE;

	/* Immunities */
	count = info_collect(tb, immunity_flags, N_ELEMENTS(immunity_flags),
			flags, i_descs);
	if (count)
	{
		textblock_append(tb, "Provides immunity to ");
		info_out_list(tb, i_descs, count);
		prev = TRUE;
	}

	/* Resistances */
	count = info_collect(tb, resist_flags, N_ELEMENTS(resist_flags),
			flags, r_descs);
	if (count)
	{
		textblock_append(tb, "Provides resistance to ");
		info_out_list(tb, r_descs, count);
		prev = TRUE;
	}

	/* Protections */
	count = info_collect(tb, protect_flags, N_ELEMENTS(protect_flags),
			flags, p_descs);
	if (count)
	{
		textblock_append(tb, "Provides protection from ");
		info_out_list(tb, p_descs, count);
		prev = TRUE;
	}

	/* Vulnerabilities */
	count = info_collect(tb, vuln_flags, N_ELEMENTS(vuln_flags),
			flags, v_descs);
	if (count)
	{
		textblock_append(tb, "Makes you vulnerable to ");
		info_out_list(tb, v_descs, count);
		prev = TRUE;
	}

	return prev;
}


/*
 * Describe IGNORE_ flags of an object.
 */
static bool describe_ignores(textblock *tb, const bitflag flags[OF_SIZE])
{
	const char *descs[N_ELEMENTS(ignore_flags)];
	size_t count = info_collect(tb, ignore_flags, N_ELEMENTS(ignore_flags),
			flags, descs);

	if (!count)
		return FALSE;

	textblock_append(tb, "Cannot be harmed by ");
	info_out_list(tb, descs, count);

	return TRUE;
}

/*
 * Describe HATES_ flags of an object.
 */
static bool describe_hates(textblock *tb, const bitflag flags[OF_SIZE])
{
	const char *descs[N_ELEMENTS(hates_flags)];
	size_t count = info_collect(tb, hates_flags, N_ELEMENTS(hates_flags),
			flags, descs);

	if (!count)
		return FALSE;

	textblock_append(tb, "Can be destroyed by ");
	info_out_list(tb, descs, count);

	return TRUE;
}


/*
 * Describe stat sustains.
 */
static bool describe_sustains(textblock *tb, const bitflag flags[OF_SIZE])
{
	const char *descs[N_ELEMENTS(sustain_flags)];
	size_t count = info_collect(tb, sustain_flags, N_ELEMENTS(sustain_flags),
			flags, descs);

	if (!count)
		return FALSE;

	textblock_append(tb, "Sustains ");
	info_out_list(tb, descs, count);

	return TRUE;
}


/*
 * Describe miscellaneous powers.
 */
static bool describe_misc_magic(textblock *tb, const bitflag flags[OF_SIZE])
{
	size_t i;
	bool printed = FALSE;

	for (i = 0; i < N_ELEMENTS(misc_flags); i++)
	{
		if (of_has(flags, misc_flags[i].flag))
		{
			textblock_append(tb, "%s.  ", misc_flags[i].name);
			printed = TRUE;
		}
	}

	if (printed)
		textblock_append(tb, "\n");

	return printed;
}


/*
 * Describe slays and brands on weapons
 */
static bool describe_slays(textblock *tb, const bitflag flags[OF_SIZE],
		const object_type *o_ptr)
{
	bool fulldesc, printed = FALSE;
	bitflag slay_mask[OF_SIZE], brand_mask[OF_SIZE];
	bitflag slay_flags[OF_SIZE], brand_flags[OF_SIZE];
	size_t i = 0, slay_count = 0, brand_count = 0;
	s16b slay_mult[SL_MAX] = { 0 };
	const struct slay *s_ptr = NULL;
	const char *slay_descs[SL_MAX] = { 0 };

	create_mask(slay_mask, FALSE, OFT_SLAY, OFT_MAX);
	create_mask(brand_mask, FALSE, OFT_BRAND, OFT_MAX);
	of_copy(slay_flags, flags);
	of_copy(brand_flags, flags);
	of_inter(slay_flags, slay_mask);
	of_inter(brand_flags, brand_mask);

	if (kind_is_weapon(o_ptr->tval) || kind_is_bow(o_ptr->tval) ||
			kind_is_ammo(o_ptr->tval) || o_ptr->tval == TV_FLASK)
		fulldesc = FALSE;
	else
		fulldesc = TRUE;

	object_slay_mults(o_ptr, slay_mult);

	/* Slays */
	if (!of_is_empty(slay_flags)) {
		if (fulldesc)
			textblock_append(tb, "It causes your melee attacks to slay ");
		else
			textblock_append(tb, "Slays ");
		for (i = of_next(slay_flags, FLAG_START); i != FLAG_END;
				i = of_next(slay_flags, i + 1)) {
			s_ptr = lookup_slay(i);
			slay_descs[slay_count++] = string_make(format("%s (x%.2f)",
				s_ptr->desc, (100 + slay_mult[s_ptr->index]) / 100.0));
		}
		info_out_list(tb, slay_descs, slay_count);
		printed = TRUE;
	}
	/* Brands */
	if (!of_is_empty(brand_flags)) {
		if (fulldesc)
			textblock_append(tb, "It brands your melee attacks with ");
		else
			textblock_append(tb, "Branded with ");
		for (i = of_next(brand_flags, FLAG_START); i != FLAG_END;
				i = of_next(brand_flags, i + 1)) {
			s_ptr = lookup_slay(i);
			slay_descs[brand_count++] = string_make(format("%s (x%.2f)",
				s_ptr->brand, (100 + slay_mult[s_ptr->index]) / 100.0));
		}
		info_out_list(tb, slay_descs, brand_count);
		printed = TRUE;
	}
	/* Free the string memory */
	for (i = 0; i < SL_MAX; i++)
		string_free((char *)slay_descs[i]);

	return printed;
}


/*
 * Describe blows.
 */
static bool describe_blows(textblock *tb, player_state state)
{
	/* Write to the text block */
	textblock_append_c(tb, TERM_L_GREEN, "%d.%d ",
			state.num_blows / 100, (state.num_blows / 10) % 10);
	textblock_append(tb, "blow%s/round.\n",
			(state.num_blows > 100) ? "s" : "");
    textblock_append_c(tb, TERM_L_GREEN, "%d.%dx ",
            state.dam_multiplier / 100, (state.dam_multiplier / 10) % 10);
    textblock_append(tb, "damage multiplier.\n");

	return TRUE;
}


/*
 * Describe damage.
 */
static bool describe_damage(textblock *tb, const object_type *o_ptr,
		player_state state, oinfo_detail_t mode)
{
	size_t i, cnt = 0;
	int dam, total_dam;
	object_type *bow = &p_ptr->inventory[INVEN_BOW];
	object_type object_type_body;
	object_type *i_ptr = &object_type_body;

	bool weapon = (wield_slot(o_ptr) == INVEN_WIELD);
	bool ammo   = (p_ptr->state.ammo_tval == o_ptr->tval) &&
	              (bow->kind);
	bool full = mode & OINFO_FULL;

	/* Defaults for weapons which are changed later for ammo */
	int attack_type = ATTACK_MELEE;

	/* Take a copy of o_ptr, in case we need to mess with its dice */
	object_copy(i_ptr, o_ptr);

	/* Use displayed dice if real dice not known */
	if (!full && !object_attack_plusses_are_visible(o_ptr)) {
		i_ptr->dd = o_ptr->kind->dd;
		i_ptr->ds = o_ptr->kind->ds;
	}

	/* Add ammo and launcher slays for ammo objects, and set type/mult */
	if (ammo) {
		/* Need to use only known ammo slays unless mode is FULL */
		object_slay_mults(o_ptr, state.slay_mult);
		/* Oh dear. Need to use only known slays from bow in all cases */
		object_slay_mults(bow, state.slay_mult);
		attack_type = ATTACK_MISSILE;
	}

	/* CC notes to self: need to ensure that calc_damage returns only the
	 * damage available from known prowess (for weapon or bow+ammo)
	 * by using object_attack_plusses_are_visible -
	 * calc_bonuses takes care of this for the rest */

	textblock_append(tb, "Average damage/round: ");

	/* Iterate over slays */
	for (i = 0; i < SL_MAX; i++) {
		const struct slay *s_ptr = lookup_slay_by_index(i);
		/* Ignore slays with 0 mult (i.e. x1.00) after the first time */
		if (i && !state.slay_mult[i]) continue;

		/* Calculate the damage using this slay */
		/* N.B. Actually needs to return 10x damage */
		dam = calc_damage(i_ptr, state, i, attack_type, NULL, AVERAGE);

		/* Multiply by blows or shots */
		if (weapon)
			total_dam = (dam * state.num_blows) / 100;
		else
			total_dam = dam * p_ptr->state.num_shots;

		/* Append the result to the textblock */
		if (cnt)
			textblock_append(tb, ", ");
		if (total_dam <= 0)
			textblock_append_c(tb, TERM_L_RED, "%d", 0);
		else if (total_dam % 10)
			textblock_append_c(tb, TERM_L_GREEN, "%d.%d" , total_dam / 10,
				total_dam % 10);
		else
			textblock_append_c(tb, TERM_L_GREEN, "%d", total_dam / 10);

		textblock_append(tb, " vs. %s", s_ptr->desc);
		cnt++;
	}
	textblock_append(tb, ".\n");

	return TRUE;
}


/*
 * Describe combat advantages.
 */
static bool describe_combat(textblock *tb, const object_type *o_ptr,
		oinfo_detail_t mode)
{
	bool full = mode & OINFO_FULL;

	object_type *bow = &p_ptr->inventory[INVEN_BOW];

	bitflag f[OF_SIZE];

	bool weapon = (wield_slot(o_ptr) == INVEN_WIELD);
	bool ammo   = (p_ptr->state.ammo_tval == o_ptr->tval) &&
	              (bow->kind);

	/* The player's hypothetical state, were they to wield this item */
	player_state state;

	/* Abort if we've nothing to say */
	if (mode & OINFO_DUMMY) return FALSE;

	if (!weapon && !ammo) {
		/* Potions can have special text */
		if (o_ptr->tval != TV_POTION ||
				o_ptr->dd == 0 || o_ptr->ds == 0 ||
				!object_flavor_is_aware(o_ptr))
			return FALSE;

		textblock_append(tb, "It can be thrown at creatures with damaging effect.\n");
		return TRUE;
	}

	if (full)
		object_flags(o_ptr, f);
	else
		object_flags_known(o_ptr, f);

	textblock_append_c(tb, TERM_L_WHITE, "Combat info:\n");

	if (weapon)	{
		object_type inven[INVEN_TOTAL];

		memcpy(inven, p_ptr->inventory, INVEN_TOTAL * sizeof(object_type));
		inven[INVEN_WIELD] = *o_ptr;

		if (full) object_know_all_flags(&inven[INVEN_WIELD]);

		/* Calculate the player's hypothetical state */
		calc_bonuses(inven, &state, TRUE);

		/* Warn about heavy weapons */
		if (adj_str_hold[state.stat_ind[A_STR]] < o_ptr->weight / 10)
			textblock_append_c(tb, TERM_L_RED, "You are too weak to use this weapon.\n");

        textblock_append(tb, "Receives %d%% of your finesse score, %d%% of your prowess score.\n", o_ptr->balance, o_ptr->heft);

		/* Describe blows */
		describe_blows(tb, state);
	} else { /* Ammo */
		/* Range of the weapon */
		int tdis = 6 + 2 * p_ptr->state.ammo_mult;

		/* Output the range */
		textblock_append(tb, "Hits targets up to ");
		textblock_append_c(tb, TERM_L_GREEN, format("%d", tdis * 10));
		textblock_append(tb, " feet away.\n");
	}

	/* Describe damage */
	describe_damage(tb, o_ptr, state, mode);

	/* Note the impact flag */
	if (of_has(f, OF_IMPACT))
		textblock_append(tb, "Sometimes creates earthquakes on impact.\n");

	/* Add breakage chance */
	if (ammo) {
		int chance = breakage_chance(o_ptr, TRUE);
		textblock_append_c(tb, TERM_L_GREEN, "%d%%", chance);
		textblock_append(tb, " chance of breaking upon contact.\n");
	}

	/* Something has been said */
	return TRUE;
}


/*
 * Describe objects that can be used for digging.
 */
static bool describe_digger(textblock *tb, const object_type *o_ptr,
		oinfo_detail_t mode)
{
	bool full = mode & OINFO_FULL;

	player_state st;

	object_type inven[INVEN_TOTAL];

	int sl = wield_slot(o_ptr);
	int i;

	bitflag f[OF_SIZE];

	int chances[4]; /* These are out of 1600 */
	static const char *names[4] = { "rubble", "magma veins", "quartz veins", "granite" };

	/* abort if we are a dummy object */
	if (mode & OINFO_DUMMY) return FALSE;

	if (full)
		object_flags(o_ptr, f);
	else
		object_flags_known(o_ptr, f);

	if (sl < 0 || (sl != INVEN_WIELD && !of_has(f, OF_TUNNEL)))
		return FALSE;

	memcpy(inven, p_ptr->inventory, INVEN_TOTAL * sizeof(object_type));

	/*
	 * Hack -- if we examine a ring that is worn on the right finger,
	 * we shouldn't put a copy of it on the left finger before calculating
	 * digging skills.
	 */
	if (o_ptr != &p_ptr->inventory[INVEN_RIGHT])
		inven[sl] = *o_ptr;

	calc_bonuses(inven, &st, TRUE);

	chances[0] = st.skills[SKILL_DIGGING] * 8;
	chances[1] = (st.skills[SKILL_DIGGING] - 10) * 4;
	chances[2] = (st.skills[SKILL_DIGGING] - 20) * 2;
	chances[3] = (st.skills[SKILL_DIGGING] - 40) * 1;

	for (i = 0; i < 4; i++)
	{
		int chance = MAX(0, MIN(1600, chances[i]));
		int decis = chance ? (16000 / chance) : 0;

		if (i == 0 && chance > 0) {
			if (sl == INVEN_WIELD)
				textblock_append(tb, "Clears ");
			else
				textblock_append(tb, "With this item, your current weapon clears ");
		}

		if (i == 3 || (i != 0 && chance == 0))
			textblock_append(tb, "and ");

		if (chance == 0) {
			textblock_append_c(tb, TERM_L_RED, "doesn't affect ");
			textblock_append(tb, "%s.\n", names[i]);
			break;
		}

		textblock_append(tb, "%s in ", names[i]);

		if (chance == 1600) {
			textblock_append_c(tb, TERM_L_GREEN, "1 ");
		} else if (decis < 100) {
			textblock_append_c(tb, TERM_GREEN, "%d.%d ", decis/10, decis%10);
		} else {
			textblock_append_c(tb, (decis < 1000) ? TERM_YELLOW : TERM_RED,
			           "%d ", (decis+5)/10);
		}

		textblock_append(tb, "turn%s%s", decis == 10 ? "" : "s",
				(i == 3) ? ".\n" : ", ");
	}

	return TRUE;
}


static bool describe_food(textblock *tb, const object_type *o_ptr,
		bool subjective, bool full)
{
	/* Describe boring bits */
	if ((o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION) &&
		o_ptr->extent)
	{
		/* Sometimes adjust for player speed */
		int multiplier = extract_energy[p_ptr->state.speed];
		if (!subjective) multiplier = 10;

		if (object_is_known(o_ptr) || full) {
			textblock_append(tb, "Nourishes for around ");
			textblock_append_c(tb, TERM_L_GREEN, "%d", (o_ptr->extent / 2) *
				multiplier / 10);
			textblock_append(tb, " turns.\n");
		} else {
			textblock_append(tb, "Provides some nourishment.\n");
		}

		return TRUE;
	}

	return FALSE;
}


/*
 * Describe things that look like lights.
 */
static bool describe_light(textblock *tb, const object_type *o_ptr,
		const bitflag flags[OF_SIZE], oinfo_detail_t mode)
{
	bool artifact = o_ptr->artifact ? TRUE : FALSE;
	bool no_fuel = of_has(flags, OF_NO_FUEL) ? TRUE : FALSE;
	bool is_light = (o_ptr->tval == TV_LIGHT) ? TRUE : FALSE;
	bool terse = mode & OINFO_TERSE;

	if (no_fuel && !artifact)
		textblock_append(tb, "No fuel required.  ");

	if (!terse && is_light && !no_fuel && o_ptr->sval != SV_LIGHT_TORCH)
	{
		const char *name = (o_ptr->sval == SV_LIGHT_TORCH) ? "torches" : "lanterns";
		int turns = (o_ptr->sval == SV_LIGHT_TORCH) ? FUEL_TORCH : FUEL_LAMP;

		textblock_append(tb, "Refills other %s up to %d turns of fuel.", name, turns);
	}

	textblock_append(tb, "\n");

	return TRUE;
}



/*
 * Describe an object's effect, if any.
 */
static bool describe_effect(textblock *tb, const object_type *o_ptr, bool full,
		bool only_artifacts, bool subjective)
{
	const char *desc;
	random_value timeout = {0, 0, 0, 0};

	int effect = 0, fail;

	if (o_ptr->artifact)
	{
		if (object_effect_is_known(o_ptr) || full)
		{
			effect = o_ptr->artifact->effect;
			timeout = o_ptr->artifact->time;
		}
		else if (object_effect(o_ptr))
		{
			textblock_append(tb, "It can be activated.\n");
			return TRUE;
		}
	}
	else
	{
		/* Sometimes only print artifact activation info */
		if (only_artifacts == TRUE) return FALSE;

		if (object_effect_is_known(o_ptr) || full)
		{
			effect = o_ptr->kind->effect;
			timeout = o_ptr->kind->time;
		}
		else if (object_effect(o_ptr) != 0)
		{
			if (effect_aim(o_ptr->kind->effect))
				textblock_append(tb, "It can be aimed.\n");
			else if (o_ptr->tval == TV_FOOD)
				textblock_append(tb, "It can be eaten.\n");
			else if (o_ptr->tval == TV_POTION)
				textblock_append(tb, "It can be drunk.\n");
			else if (o_ptr->tval == TV_SCROLL)
				textblock_append(tb, "It can be read.\n");
			else textblock_append(tb, "It can be activated.\n");

			return TRUE;
		}
	}

	/* Forget it without an effect */
	if (!effect) return FALSE;

	/* Obtain the description */
	desc = effect_desc(effect);
	if (!desc) return FALSE;

	if (effect_aim(effect))
		textblock_append(tb, "When aimed, it ");
	else if (o_ptr->tval == TV_FOOD)
		textblock_append(tb, "When eaten, it ");
	else if (o_ptr->tval == TV_POTION)
		textblock_append(tb, "When drunk, it ");
	else if (o_ptr->tval == TV_SCROLL)
	    textblock_append(tb, "When read, it ");
	else
	    textblock_append(tb, "When activated, it ");

	/* Print a colourised description */
	do
	{
		if (isdigit((unsigned char) *desc))
			textblock_append_c(tb, TERM_L_GREEN, "%c", *desc);
		else
			textblock_append(tb, "%c", *desc);
	} while (*desc++);

	textblock_append(tb, ".\n");

	if (randcalc(timeout, 0, MAXIMISE) > 0)
	{
		int min_time, max_time;

		/* Sometimes adjust for player speed */
		int multiplier = extract_energy[p_ptr->state.speed];
		if (!subjective) multiplier = 10;

		textblock_append(tb, "Takes ");

		/* Correct for player speed */
		min_time = randcalc(timeout, 0, MINIMISE) * multiplier / 10;
		max_time = randcalc(timeout, 0, MAXIMISE) * multiplier / 10;

		textblock_append_c(tb, TERM_L_GREEN, "%d", min_time);

		if (min_time != max_time)
		{
			textblock_append(tb, " to ");
			textblock_append_c(tb, TERM_L_GREEN, "%d", max_time);
		}

		textblock_append(tb, " turns to recharge");
		if (subjective && p_ptr->state.speed != 110)
			textblock_append(tb, " at your current speed");

		textblock_append(tb, ".\n");
	}

	if (!subjective || o_ptr->tval == TV_FOOD || o_ptr->tval == TV_POTION ||
		o_ptr->tval == TV_SCROLL)
	{
		return TRUE;
	}
	else
	{
		fail = get_use_device_chance(o_ptr);
		textblock_append(tb, "Your chance of success is %d.%d%%\n", (1000 - fail) /
			10, (1000 - fail) % 10);
	}

	return TRUE;
}


static bool describe_origin(textblock *tb, const object_type *o_ptr)
{
	char origin_text[80];

	if (o_ptr->origin_depth)
		strnfmt(origin_text, sizeof(origin_text), "%d feet (level %d)",
		        o_ptr->origin_depth * 50, o_ptr->origin_depth);
	else
		my_strcpy(origin_text, "town", sizeof(origin_text));

	switch (o_ptr->origin)
	{
		case ORIGIN_NONE:
		case ORIGIN_MIXED:
		case ORIGIN_STOLEN:
			return FALSE;

		case ORIGIN_BIRTH:
			textblock_append(tb, "An inheritance from your family.\n");
			break;

		case ORIGIN_STORE:
			textblock_append(tb, "Bought from a store.\n");
			break;

		case ORIGIN_FLOOR:
			textblock_append(tb, "Found lying on the floor %s %s.\n",
			         (o_ptr->origin_depth ? "at" : "in"),
			         origin_text);
 			break;

		case ORIGIN_PIT:
			textblock_append(tb, "Found lying on the floor in a pit at %s.\n",
			         origin_text);
 			break;

		case ORIGIN_VAULT:
			textblock_append(tb, "Found lying on the floor in a vault at %s.\n",
			         origin_text);
 			break;

		case ORIGIN_SPECIAL:
			textblock_append(tb, "Found lying on the floor of a special room at %s.\n",
			         origin_text);
 			break;

		case ORIGIN_LABYRINTH:
			textblock_append(tb, "Found lying on the floor of a labyrinth at %s.\n",
			         origin_text);
 			break;

		case ORIGIN_CAVERN:
			textblock_append(tb, "Found lying on the floor of a cavern at %s.\n",
			         origin_text);
 			break;

		case ORIGIN_RUBBLE:
			textblock_append(tb, "Found under some rubble at %s.\n",
			         origin_text);
 			break;

		case ORIGIN_DROP:
		case ORIGIN_DROP_SPECIAL:
		case ORIGIN_DROP_PIT:
		case ORIGIN_DROP_VAULT:
		case ORIGIN_DROP_SUMMON:
		case ORIGIN_DROP_BREED:
		case ORIGIN_DROP_POLY:
		case ORIGIN_DROP_WIZARD:
		{
			const char *name;

			if (r_info[o_ptr->origin_xtra].ridx)
				name = r_info[o_ptr->origin_xtra].name;
			else
				name = "monster lost to history";

			textblock_append(tb, "Dropped by ");

			if (rf_has(r_info[o_ptr->origin_xtra].flags, RF_UNIQUE))
				textblock_append(tb, "%s", name);
			else
				textblock_append(tb, "%s%s",
						is_a_vowel(name[0]) ? "an " : "a ", name);

			textblock_append(tb, " %s %s.\n",
					(o_ptr->origin_depth ? "at" : "in"),
					origin_text);
 			break;
		}

		case ORIGIN_DROP_UNKNOWN:
			textblock_append(tb, "Dropped by an unknown monster %s %s.\n",
					(o_ptr->origin_depth ? "at" : "in"),
					origin_text);
			break;

		case ORIGIN_ACQUIRE:
			textblock_append(tb, "Conjured forth by magic %s %s.\n",
					(o_ptr->origin_depth ? "at" : "in"),
					origin_text);
 			break;

		case ORIGIN_CHEAT:
			textblock_append(tb, "Created by debug option.\n");
 			break;

		case ORIGIN_CHEST:
			textblock_append(tb, "Found in a chest from %s.\n",
			         origin_text);
			break;
	}

	textblock_append(tb, "\n");

	return TRUE;
}

/*
 * Print an item's flavour text.
 *
 * \param tb is the textblock to which we are adding.
 * \param o_ptr is the object we are describing.
 * \param ego is whether we're describing an ego template (as opposed to a
 * real object)
 */
static void describe_flavor_text(textblock *tb, const object_type *o_ptr,
	oinfo_detail_t mode)
{
	int i, count = 0;
	bool ego = mode & OINFO_EGO;
	bool subj = mode & OINFO_SUBJ;

	/* Display the known artifact description */
	if (!OPT(birth_randarts) && o_ptr->artifact &&
			object_is_known(o_ptr) && o_ptr->artifact->text)
		textblock_append(tb, "%s\n\n", o_ptr->artifact->text);

	else if (o_ptr->theme && o_ptr->theme->text &&
			object_theme_is_known(o_ptr))
		textblock_append(tb, "%s\n\n", o_ptr->theme->text);

	/* Display the known object description */
	else if (object_flavor_is_aware(o_ptr) || object_is_known(o_ptr) || ego) {
		bool did_desc = FALSE;

		if (!ego && o_ptr->kind->text) {
			textblock_append(tb, "%s", o_ptr->kind->text);
			did_desc = TRUE;
		}

		/* Display additional affix descriptions */
		for (i = 0; i < MAX_AFFIXES && o_ptr->affix[i]; i++)
			if (o_ptr->affix[i]->text && (ego ||
					object_affix_is_known(o_ptr, o_ptr->affix[i]->eidx))) {
				if (did_desc)
					textblock_append(tb, " ");
				textblock_append(tb, "%s", o_ptr->affix[i]->text);
				did_desc = TRUE;
			}

		if (did_desc)
			textblock_append(tb, "\n\n");
	}

	/* List the affixes on the item */
	for (i = 0; i < MAX_AFFIXES && o_ptr->affix[i]; i++)
		if (object_affix_is_known(o_ptr, o_ptr->affix[i]->eidx)) {
			if (count == 0)
				textblock_append(tb, "This item's known properties are: ");
			else
				textblock_append(tb, ", ");
			textblock_append(tb, "%s", o_ptr->affix[i]->name);
			count++;
		}
	if (count)
		textblock_append(tb, ".\n\n");

	if (!ego && subj && o_ptr->origin != ORIGIN_STORE) {
		/* List the item's known runes */
		count = 0;
		for (i = 0; i < OF_MAX; i++)
			if (of_has(o_ptr->flags, i) && of_has(p_ptr->known_runes, i) &&
					obj_flag_type(i) != OFT_INT && obj_flag_type(i) != OFT_NONE) {
				if (count == 0)
					textblock_append(tb, "This item's known runes are: ");
				else
					textblock_append(tb, ", ");
				textblock_append(tb, "%s", flag_name(i));
				count++;
			}
		if (count)
			textblock_append(tb, ".\n\n");

		/* List the item's unknown runes */
		count = 0;
		for (i = 0; i < OF_MAX; i++)
			if (of_has(o_ptr->flags, i) && !of_has(p_ptr->known_runes, i) &&
					obj_flag_type(i) != OFT_INT && obj_flag_type(i) != OFT_NONE) {
				if (count == 0)
					textblock_append(tb, "This item's unknown runes are: ");
				else
					textblock_append(tb, ", ");
				textblock_append(tb, "%s", flag_rune(i));
				count++;
			}
		if (count)
			textblock_append(tb, ".\n\n");
	}
}

/**
 * Describe random powers on ego items
 *
 * \param tb is the description textblock we're building
 * \param ego is the ego type we're analysing
 */
static bool describe_ego(textblock *tb, const struct ego_item *ego)
{
	if (ego && ego->num_randlines) {
		int i, of_type, tot = 0;
		bitflag f[OF_SIZE];

		for (i = 0; i < ego->num_randlines; i++) {
			/* See whether we recognise the flagset for this choice */
			of_type = obj_flag_type(of_next(ego->randmask[i], FLAG_START));
			create_mask(f, FALSE, of_type, OFT_MAX);

			if (of_is_equal(f, ego->randmask[i])) {
				textblock_append(tb, "It provides %s random %s.  ",
					ego->num_randflags[i] > 1 ? "more than one" : "one",
					obj_flagtype_name(of_type));
			} else /* We don't, so count the number for later */
				tot += ego->num_randflags[i];
		}
		if (tot)
			textblock_append(tb, "It provides %s random power.  ",
				tot > 1 ? "more than one" : "one");

		return TRUE;
	}

	return FALSE;
}


/*
 * Output object information
 */
static textblock *object_info_out(const object_type *o_ptr, oinfo_detail_t mode)
{
	bitflag flags[OF_SIZE];
	bitflag pval_flags[MAX_PVALS][OF_SIZE];
	bool something = FALSE;
	bool known = object_is_known(o_ptr);

	bool full = mode & OINFO_FULL;
	bool terse = mode & OINFO_TERSE;
	bool subjective = mode & OINFO_SUBJ;
	bool ego = mode & OINFO_EGO;

	textblock *tb = textblock_new();

	/* Grab the object flags */
	if (full) {
		object_flags(o_ptr, flags);
		object_pval_flags(o_ptr, pval_flags);
	} else {
		object_flags_known(o_ptr, flags);
		object_pval_flags_known(o_ptr, pval_flags);
	}

	if (subjective) describe_origin(tb, o_ptr);
	if (!terse) describe_flavor_text(tb, o_ptr, mode);

	if (!full && !known)
	{
		textblock_append(tb, "You do not know the full extent of this item's powers.\n");
		something = TRUE;
	}

	if (describe_curses(tb, o_ptr, flags)) something = TRUE;
	if (describe_stats(tb, o_ptr, pval_flags, mode)) something = TRUE;
	if (describe_slays(tb, flags, o_ptr)) something = TRUE;
	if (describe_immune(tb, flags)) something = TRUE;
	if (describe_ignores(tb, flags)) something = TRUE;
	dedup_hates_flags(flags);
	if (describe_hates(tb, flags)) something = TRUE;
	if (describe_sustains(tb, flags)) something = TRUE;
	if (describe_misc_magic(tb, flags)) something = TRUE;
	if (ego && describe_ego(tb, o_ptr->ego)) something = TRUE;
	if (something) textblock_append(tb, "\n");

	if (!ego && describe_effect(tb, o_ptr, full, terse, subjective)) {
		something = TRUE;
		textblock_append(tb, "\n");
	}

	if (subjective && describe_combat(tb, o_ptr, mode)) {
		something = TRUE;
		textblock_append(tb, "\n");
	}

	if (!terse && describe_food(tb, o_ptr, subjective, full)) something = TRUE;
	if (describe_light(tb, o_ptr, flags, mode)) something = TRUE;
	if (!terse && subjective && describe_digger(tb, o_ptr, mode)) something = TRUE;

	if (!something)
		textblock_append(tb, "\n\nThis item does not seem to possess any special abilities.");

	return tb;
}


/**
 * Provide information on an item, including how it would affect the current
 * player's state.
 *
 * mode OINFO_FULL should be set if actual player knowledge should be ignored
 * in favour of full knowledge.
 *
 * returns TRUE if anything is printed.
 */
textblock *object_info(const object_type *o_ptr, oinfo_detail_t mode)
{
	mode |= OINFO_SUBJ;
	return object_info_out(o_ptr, mode);
}

/**
 * Provide information on an ego-item type
 */
textblock *object_info_ego(struct ego_item *ego)
{
	object_kind *kind = NULL;
	object_type obj = { 0 };
	int i;

	for (i = 0; i < z_info->k_max; i++) {
		kind = &k_info[i];
		if (!kind->name)
			continue;
		if (kind->tval == ego->tval[0])
			break;
	}

	obj.kind = kind;
	obj.tval = kind->tval;
	obj.sval = kind->sval;
	obj.affix[0] = ego;
	ego_apply_magic(&obj, 0, ego->eidx);

	return object_info_out(&obj, OINFO_FULL | OINFO_EGO | OINFO_DUMMY);
}

/**
 * Provide information on a theme type
 */

textblock *object_info_theme(struct theme *theme)
{
	object_kind *kind = NULL;
	object_type obj = {0};
	int i;

	for (i = 0; i < z_info->k_max; i++) {
		kind = &k_info[i];
		if (!kind->name)
			continue;
		if (kind->tval == theme->tval[0])
			break;
	}

	obj.kind = kind;
	obj.tval = kind->tval;
	obj.sval = kind->sval;
	obj_apply_theme(&obj, 0, theme->index);

	return object_info_out(&obj, OINFO_FULL | OINFO_EGO | OINFO_DUMMY);
}

/**
 * Provide information on an item suitable for writing to the character dump - keep it brief.
 */
void object_info_chardump(ang_file *f, const object_type *o_ptr, int indent, int wrap)
{
	textblock *tb = object_info_out(o_ptr, OINFO_TERSE | OINFO_SUBJ);
	textblock_to_file(tb, f, indent, wrap);
	textblock_free(tb);
}


/**
 * Provide spoiler information on an item.
 *
 * Practically, this means that we should not print anything which relies upon
 * the player's current state, since that is not suitable for spoiler material.
 */
void object_info_spoil(ang_file *f, const object_type *o_ptr, int wrap)
{
	textblock *tb = object_info_out(o_ptr, OINFO_FULL);
	textblock_to_file(tb, f, 0, wrap);
	textblock_free(tb);
}
