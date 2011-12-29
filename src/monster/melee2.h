/*
 * File: src/monster/melee2.h
 * Purpose: functions for monster melee combat
 *
 * Copyright (c) 2007-11 Angband Devteam
 */
#ifndef MONSTER_MELEE2_H
#define MONSTER_MELEE2_H

#include "angband.h"

extern bool check_hit(struct player *p, int power, int level);
extern bool mon_test_hit(int chance, int ac);
extern void process_monsters(struct cave *c, byte min_energy);
int mon_hp(const struct monster_race *r_ptr, aspect hp_aspect);

#ifdef TEST
extern bool (*testfn_make_attack_normal)(struct monster *m, struct player *p);
#endif /* !TEST */

#endif /* !MONSTER_MELEE2_H */
