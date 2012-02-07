/* player/player.h - player interface */

#ifndef PLAYER_PLAYER_H
#define PLAYER_PLAYER_H

#include "guid.h"
#include "object/obj-flag.h"
#include "object/object.h"
#include "player/types.h"

/* calcs.c */
extern const int adj_chr_gold[STAT_RANGE];
extern const int adj_str_blow[STAT_RANGE];
extern const int adj_dex_safe[STAT_RANGE];
extern const int adj_con_fix[STAT_RANGE];
extern const int adj_str_hold[STAT_RANGE];
#ifdef ALLOW_BORG
extern const int adj_int_dev[STAT_RANGE];
extern const int adj_wis_sav[STAT_RANGE];
extern const int adj_dex_dis[STAT_RANGE];
extern const int adj_int_dis[STAT_RANGE];
extern const int adj_dex_ta[STAT_RANGE];
extern const int adj_str_to_prowess[STAT_RANGE];
extern const int adj_dex_to_finesse[STAT_RANGE];
extern const int adj_str_wgt[STAT_RANGE];
extern const int adj_str_dig[STAT_RANGE];
extern const int adj_dex_blow[STAT_RANGE];
extern const int adj_con_mhp[STAT_RANGE];
extern const int adj_mag_study[STAT_RANGE];
extern const int adj_mag_mana[STAT_RANGE];
extern const int blows_table[12][12];
#endif

void calc_bonuses(object_type inventory[], player_state *state, bool id_only);
int calc_blows(const object_type *o_ptr, player_state *state);
int calc_multiplier(const object_type *o_ptr, player_state *state);
void notice_stuff(struct player *p);
void update_stuff(struct player *p);
void redraw_stuff(struct player *p);
void handle_stuff(struct player *p);
int weight_remaining(void);
int critical_norm(player_state state, const object_type *o_ptr, int dam,
	int attack_type, u32b *msg_type, aspect dam_aspect);
int calc_damage(const object_type *o_ptr, player_state state, int slay_index,
	int attack_type, u32b *msg_type, aspect dam_aspect);

/* class.c */
extern struct player_class *player_id2class(guid id);

/* player.c */
extern bool player_stat_inc(struct player *p, int stat);
extern bool player_stat_dec(struct player *p, int stat, bool permanent);
extern void player_exp_gain(struct player *p, s32b amount);
extern void player_exp_lose(struct player *p, s32b amount, bool permanent);

extern byte player_hp_attr(struct player *p);
extern byte player_sp_attr(struct player *p);

/* race.c */
extern struct player_race *player_id2race(guid id);

/* spell.c */
#ifdef ALLOW_BORG
extern const byte adj_mag_fail[STAT_RANGE];
extern const int adj_mag_stat[STAT_RANGE];
#endif
int spell_collect_from_book(const object_type *o_ptr, int *spells);
int spell_book_count_spells(const object_type *o_ptr, bool (*tester)(int spell));
bool spell_okay_list(bool (*spell_test)(int spell), const int spells[], int n_spells);
bool spell_okay_to_cast(int spell);
bool spell_okay_to_study(int spell);
bool spell_okay_to_browse(int spell);
bool spell_in_book(int spell, int book);
s16b spell_chance(int spell);
void spell_learn(int spell);
bool spell_cast(int spell, int dir);

/* timed.c */
bool player_set_timed(struct player *p, int idx, int v, bool notify);
bool player_inc_timed(struct player *p, int idx, int v, bool notify, bool check);
bool player_dec_timed(struct player *p, int idx, int v, bool notify);
bool player_clear_timed(struct player *p, int idx, bool notify);
bool player_set_food(struct player *p, int v);

/* util.c */
s16b modify_stat_value(int value, int amount);
bool player_can_cast(void);
bool player_can_cast_msg(void);
bool player_can_study(void);
bool player_can_study_msg(void);
bool player_can_study_book(void);
bool player_can_read(void);
bool player_can_read_msg(void);
bool player_can_fire(void);
bool player_can_fire_msg(void);
bool player_can_refuel(void);
bool player_can_refuel_msg(void);
bool player_confuse_dir(struct player *p, int *dir, bool too);

#endif /* !PLAYER_PLAYER_H */
