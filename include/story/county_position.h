#ifndef GUARD_COUNTY_POSITION_H
#define GUARD_COUNTY_POSITION_H

/**
 * Pure helpers for a county's packed var 0 and its bit-mask cursor. The layout, reset and skip-ahead rules
 * are in design_story.md, "Story state storage"; this file only encodes them.
 *
 * Var 0 holds (chapter << 12 | stage << 8 | state). The chapter is an index into the current slot's
 * unsorted chapter list, not a place in its order: `order` arrays list unsorted indices in required order, so
 * reads never need the order and only advancing does. Advancing skips chapters whose bit is set in `doneMask`
 * (bit i is the chapter with unsorted index i) and clears the stage and state of the chapter it lands on.
 */

#include "global.h"

#define COUNTY_POS_CHAPTER_SHIFT 12
#define COUNTY_POS_STAGE_SHIFT   8
#define COUNTY_POS_MAX_CHAPTERS  16
#define COUNTY_POS_MAX_STAGES    16
#define COUNTY_POS_STATE_BITS    8

#define COUNTY_MASK_VARS         3  // vars 1-3 of the slice
#define COUNTY_MASK_BITS_PER_VAR 16

struct CountyBitMask
{
    u8 var;   // 1-3: the offset from the slice start
    u8 shift;
    u8 width;
};

u32 GetCountyPosChapter(u16 position);
u32 GetCountyPosStage(u16 position);
u32 GetCountyPosState(u16 position);
u16 PackCountyPosition(u32 chapter, u32 stage, u32 state);
u16 SetCountyPosStage(u16 position, u32 stage);
u16 SetCountyPosState(u16 position, u32 state);

// Writes the position of the first chapter in `order` that is not done (stage 0, state 0), or returns FALSE
// and leaves `position` alone when every chapter is done or `numOrder` is 0.
bool32 StartCountyPosition(u16 *position, const u8 *order, u32 numOrder, u16 doneMask);

// Moves `position` to the chapter after its current one in `order`, skipping done chapters. Returns FALSE and
// leaves `position` alone when none remains, which is when the main-story-index should advance.
bool32 AdvanceCountyPosition(u16 *position, const u8 *order, u32 numOrder, u16 doneMask);

// Hands out the next `width` bits (1-16) from vars 1-3 and moves the cursor, which starts at 0. A mask that
// does not fit the rest of its var starts the next one. Returns FALSE (cursor unchanged) when vars 1-3 are full.
bool32 AllocCountyBits(u32 *cursor, u32 width, struct CountyBitMask *out);

u32 GetCountyMaskedBits(u16 varValue, const struct CountyBitMask *mask);
u16 SetCountyMaskedBits(u16 varValue, const struct CountyBitMask *mask, u32 value);

#endif // GUARD_COUNTY_POSITION_H
