#include "global.h"
#include "county_position.h"

STATIC_ASSERT(COUNTY_POS_MAX_CHAPTERS == 1 << (16 - COUNTY_POS_CHAPTER_SHIFT), ChapterBitsMismatch);
STATIC_ASSERT(COUNTY_POS_MAX_STAGES == 1 << (COUNTY_POS_CHAPTER_SHIFT - COUNTY_POS_STAGE_SHIFT), StageBitsMismatch);
STATIC_ASSERT(COUNTY_POS_STATE_BITS == COUNTY_POS_STAGE_SHIFT, StateBitsMismatch);

u32 GetCountyPosChapter(u16 position)
{
    return position >> COUNTY_POS_CHAPTER_SHIFT;
}

u32 GetCountyPosStage(u16 position)
{
    return (position >> COUNTY_POS_STAGE_SHIFT) & (COUNTY_POS_MAX_STAGES - 1);
}

u32 GetCountyPosState(u16 position)
{
    return position & ((1 << COUNTY_POS_STATE_BITS) - 1);
}

u16 PackCountyPosition(u32 chapter, u32 stage, u32 state)
{
    assertf(chapter < COUNTY_POS_MAX_CHAPTERS, "chapter index %d >= %d; a slot holds at most %d required chapters", chapter, COUNTY_POS_MAX_CHAPTERS, COUNTY_POS_MAX_CHAPTERS)
    {
        return 0;
    }
    assertf(stage < COUNTY_POS_MAX_STAGES, "stage %d >= %d; split the chapter into two", stage, COUNTY_POS_MAX_STAGES)
    {
        return 0;
    }
    assertf(state < (1 << COUNTY_POS_STATE_BITS), "chapter state %d does not fit in %d bits; did you mean to request a bit mask from vars 1-3?", state, COUNTY_POS_STATE_BITS)
    {
        return 0;
    }
    return chapter << COUNTY_POS_CHAPTER_SHIFT | stage << COUNTY_POS_STAGE_SHIFT | state;
}

u16 SetCountyPosStage(u16 position, u32 stage)
{
    return PackCountyPosition(GetCountyPosChapter(position), stage, GetCountyPosState(position));
}

u16 SetCountyPosState(u16 position, u32 state)
{
    return PackCountyPosition(GetCountyPosChapter(position), GetCountyPosStage(position), state);
}

static bool32 LandOnFirstUndone(u16 *position, const u8 *order, u32 first, u32 numOrder, u16 doneMask)
{
    u32 i;

    for (i = first; i < numOrder; i++)
    {
        if (!(doneMask & 1 << order[i]))
        {
            *position = PackCountyPosition(order[i], 0, 0);
            return TRUE;
        }
    }
    return FALSE;
}

bool32 StartCountyPosition(u16 *position, const u8 *order, u32 numOrder, u16 doneMask)
{
    return LandOnFirstUndone(position, order, 0, numOrder, doneMask);
}

bool32 AdvanceCountyPosition(u16 *position, const u8 *order, u32 numOrder, u16 doneMask)
{
    u32 i, current = GetCountyPosChapter(*position);

    for (i = 0; i < numOrder; i++)
    {
        if (order[i] == current)
            return LandOnFirstUndone(position, order, i + 1, numOrder, doneMask);
    }
    assertf(FALSE, "chapter %d is not in the slot's order of %d chapters; did the order change while the chapter was current?", current, numOrder)
    {
        return FALSE;
    }
    return FALSE;
}

bool32 AllocCountyBits(u32 *cursor, u32 width, struct CountyBitMask *out)
{
    u32 var, shift;

    assertf(width >= 1 && width <= COUNTY_MASK_BITS_PER_VAR, "a bit mask of %d bits cannot fit in one var (1-%d)", width, COUNTY_MASK_BITS_PER_VAR)
    {
        return FALSE;
    }
    var = *cursor / COUNTY_MASK_BITS_PER_VAR;
    shift = *cursor % COUNTY_MASK_BITS_PER_VAR;
    if (shift + width > COUNTY_MASK_BITS_PER_VAR)
    {
        var++;
        shift = 0;
    }
    if (var >= COUNTY_MASK_VARS)
        return FALSE;
    out->var = 1 + var;
    out->shift = shift;
    out->width = width;
    *cursor = var * COUNTY_MASK_BITS_PER_VAR + shift + width;
    return TRUE;
}

u32 GetCountyMaskedBits(u16 varValue, const struct CountyBitMask *mask)
{
    return (varValue >> mask->shift) & ((1u << mask->width) - 1);
}

u16 SetCountyMaskedBits(u16 varValue, const struct CountyBitMask *mask, u32 value)
{
    u32 bits = (1u << mask->width) - 1;

    assertf(value <= bits, "value %d does not fit in a %d-bit mask", value, mask->width)
    {
        return varValue;
    }
    return (varValue & ~(bits << mask->shift)) | value << mask->shift;
}
