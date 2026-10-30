#ifndef GUARD_CONSTANTS_RANDOMIZATION_H
#define GUARD_CONSTANTS_RANDOMIZATION_H

// One bit per randomization in SaveBlock2's randomizations mask (remaining_work.md milestones).
#define RANDOMIZE_NPC_TEXT      (1 << 0)
#define RANDOMIZE_SUBPLOTS      (1 << 1)
#define RANDOMIZE_ARC_CUTSCENES (1 << 2)
#define RANDOMIZE_TYPES         (1 << 3)
#define RANDOMIZE_ENCOUNTERS    (1 << 4)
#define RANDOMIZE_MOVES         (1 << 5)
#define RANDOMIZE_TRAINERS      (1 << 6)

#define RANDOMIZE_ALL           ((1 << 7) - 1)

#endif // GUARD_CONSTANTS_RANDOMIZATION_H
