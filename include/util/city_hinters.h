#ifndef GUARD_CITY_HINTERS_H
#define GUARD_CITY_HINTERS_H

/**
 * City planner hinter steps (design.md, "City planning", steps 5 and 6).
 *
 * PlanCountyHinters() lists the hinter NPCs a county's city must hold. It is derived on demand from the
 * subplot descriptors and the main plot, and stores nothing. A record binds one NPC to one beat: the NPC
 * says the before, current or after entry for that beat, with the distance in the record.
 *
 * Subplot hinters: each subplot hosted by the county contributes numHinters records per stage. They stand
 * in the host city, so they are always SUBPLOT_CLOSE; far records exist only for main-plot hinters.
 * Main-plot hinters: each main-story beat the county hosts gets one hinter for the next beat (the next
 * main-story-index with something to do), who is far unless that beat is also in this county.
 *
 * Placement uses the placeholder NPC budget per building (2 per role, 3 for a gym, 1 for a service building,
 * 4 outdoors). A hinter with no role, or whose roles are full, falls back to any building with room.
 * Planning asserts when a stage names a role the city lacks or when the city has no room left.
 */

#include "global.h"

#define HINTER_NO_BEAT 0xFF

enum HinterKind
{
    HINTER_SUBPLOT,
    HINTER_MAIN,
};

struct HinterPlacement
{
    u8 kind;     // enum HinterKind
    u8 slot;     // subplot slot; main: the county-hosted main-story-index this hinter was planned for
    u8 beat;     // subplot stage; main: the main-story-index of the next beat
    u8 distance; // enum SubplotDistance
    u16 role;    // BuildingRole bit of the building the NPC stands in, 0 = outdoors
    u8 targetCounty; // county hosting the beat
    u8 selector; // distinguishes this NPC's text among the county's hinters
};

#define MAX_COUNTY_HINTERS 32

// Host county of a main-story-index whose list has something to do (an item, or a blocker unlock), else FALSE.
bool32 GetMainBeatHost(u32 index, u32 *county);

// Smallest index above `index` with a host, or HINTER_NO_BEAT.
u32 GetNextMainBeat(u32 index);

// Placeholder NPC capacity of a building role, or of outdoors for role 0.
u32 GetHinterRoleCapacity(u32 role);

// Writes the hint about main-story-index `index` (which must have a host) at dst, for a hinter that is far
// from or close to its host city, and returns the end. The phrase follows the first item of the list.
u8 *WriteMainBeatHint(u8 *dst, u32 index, u32 distance);

// Fills out (up to maxOut) with the county's hinters and returns the total count. Asserts if over capacity.
u32 PlanCountyHinters(u32 countyId, struct HinterPlacement *out, u32 maxOut);

#endif // GUARD_CITY_HINTERS_H
