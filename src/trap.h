/* trap.h - trap interface */

#ifndef TRAP_H
#define TRAP_H

bool trap_check_hit(int power);
extern void hit_trap(int y, int x);
extern void place_trap(struct cave *c, int y, int x);
extern void remove_trap(struct cave *c, int y, int x);
extern void reveal_trap(struct cave *c, int y, int x);

#endif /* !TRAP_H */
