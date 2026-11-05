#ifndef GUARD_REGION_PITSTOP_H
#define GUARD_REGION_PITSTOP_H

/**
 * Deals the pool of pitstops out to counties (design.md, "Special locations").
 *
 * The pool is a fixed list of L pitstop places (GetPitstopPoolCount). Slot j of the deal is pool
 * entry (j * stride + offset) mod L, with the stride coprime to L, so no entry repeats. Counties
 * take consecutive windows of slots in an order scrambled by a second coprime stride over their
 * county-story-indices: the county of rank r owns slots floor(L * r / N) up to floor(L * (r + 1) / N),
 * N = NUM_COUNTIES. A county's window depends only on header data (its story index), so counties
 * never read each other's route plans, and windows are disjoint, so no pitstop lands in two counties.
 *
 * Capacity never moves a slot between counties: a county keeps only as many of its window's
 * slots as it has free route tiles (GetCountyFreeRouteTileCount), and the rest of the window goes
 * unused. The deal is a pure function of the player's identity seed. The hardcoded Emerald table
 * still assigns its pitstops by hand; nothing reads the deal until the generator exists.
 */

#include "global.h"

// Pool entries of a window are at most this many (ceil(pool / counties) with Emerald's pool).
#define MAX_DEALT_PITSTOPS 4

// Stride and offset of the deal over the pool, and the county-rank stride and offset. The strides
// are coprime to the pool size (1 for a pool of 1 or 0) and to NUM_COUNTIES.
u32 GetPitstopDealStride(void);
u32 GetPitstopDealOffset(void);

// Position of the county in the dealing order, 0..NUM_COUNTIES-1, unique per county.
u32 GetCountyPitstopRank(u32 countyId);

// Pool indices dealt to the county, before the capacity limit: the county's whole window.
u32 GetCountyPitstopWindow(u32 countyId, u32 *firstSlot);

// Fills out with the pool indices the county gets (window truncated to its free route tiles) and
// returns their count.
u32 DealCountyPitstops(u32 countyId, u8 *out, u32 maxCount);

#endif // GUARD_REGION_PITSTOP_H
