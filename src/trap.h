/* trap.h - trap interface */

#ifndef TRAP_H
#define TRAP_H

/**
 * Information about trap kinds.
 */
typedef struct trap_kind
{
	char *name;
	s16b idx;

	struct trap_kind *next;

	int hidden;		/* How hidden is the trap? */

	int min_level;
	int max_level;

	u32b effect;   /**< Effect on entry to grid */

	byte d_attr;   /**< Default feature attribute */
	wchar_t d_char;   /**< Default feature character */

	byte x_attr[3];   /**< Desired feature attribute (set by user/pref file) */
	wchar_t x_char[3];   /**< Desired feature character (set by user/pref file) */
} trap_kind;

/**
 * Information about one particular trap.
 */
typedef struct trap
{
	struct trap_kind *kind;

	u16b hidden;

	byte x;
	byte y;

} trap_type;


bool trap_check_hit(int power);
extern void hit_trap(int y, int x);
extern void place_trap(struct cave *c, struct trap *t_ptr);
extern void pick_and_place_trap(struct cave *c, int y, int x);
extern void remove_trap(struct cave *c, int y, int x);
extern void reveal_trap(struct cave *c, int y, int x);

#endif /* !TRAP_H */
