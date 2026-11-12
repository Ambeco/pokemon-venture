#ifndef GUARD_SUBPLOT_H
#define GUARD_SUBPLOT_H

/**
 * Subplot descriptor, stage accessor and NPC-talk query (design.md, "Subplot abstraction").
 *
 * A subplot is a linear list of stages. A county hosts up to SUBPLOTS_PER_COUNTY subplots, each in a
 * numbered slot; callers address a subplot by (county, slot) and never by identity, so moving a subplot to
 * another host moves no state. A stored subplot keeps its stage in the host county's slice var for that
 * slot (a county's var 0 is its chain position, see county_chain.h). An adapter subplot has no state of its own:
 * its stage is the highest stage whose done-predicate is true, over Emerald's old flags and vars, and it
 * cannot be set. The query functions cannot tell the two apart.
 *
 * Requirements and produced keys (design.md, "Placement and requirements"): a subplot lists the building
 * roles its host city needs (those it names itself, the roles of every stage anchor and extraRoles, so a
 * cutscene across several buildings needs them all), the badges it needs and the keys it consumes. A key is
 * named relative to the host (an index into the host county's McGuffin pool) or as a planner item, so it
 * moves with the subplot. CheckSubplotPlacement is the static placement check, IsSubplotAvailable the runtime
 * one. A stage's hint may take a number (SubplotStage.hintCount), which the talk special puts in
 * {STR_VAR_1}. A derived stage needs no stored counter: use an adapter subplot with a STORY_PRED_BADGES_GE
 * predicate per stage.
 *
 * Slots are valid only for counties whose slice is free of legacy vars (COUNTY_VAR_SLICES_FREE). Specials
 * for scripts act on the county of the current map's MAPSEC, which must be a county's city for now.
 */

#include "global.h"
#include "plot_stage.h"
#include "region_route.h"
#include "region_unlock.h"

#define SUBPLOTS_PER_COUNTY (VARS_PER_COUNTY - 1)
#define COUNTY_VAR_SLICES_FREE ((LEGACY_VARS_START - COUNTY_VARS_START) / VARS_PER_COUNTY)

enum SubplotBackend
{
    SUBPLOT_STORED,
    SUBPLOT_ADAPTER,
};

// Where the talking NPC is relative to the beat: far = another city or county, close = at the place.
enum SubplotDistance
{
    SUBPLOT_FAR,
    SUBPLOT_CLOSE,
};

// Pseudo-stages around a subplot's real stages 0..numStages-1: BEFORE is "not started" (a chain entry the
// chain has not reached), AFTER is "completed". Hint ranges may use them as ends; the stored stage never is one.
#define SUBPLOT_STAGE_BEFORE 0xFE
#define SUBPLOT_STAGE_AFTER  0xFF

// What triggers a beat (design.md, "City planning", beat placement).
enum BeatAnchorKind
{
    BEAT_ANCHOR_NPC,            // immobile NPC
    BEAT_ANCHOR_ITEM,           // immobile item
    BEAT_ANCHOR_ROUTE_ROUTE,    // transition between routeId and routeId2
    BEAT_ANCHOR_ROUTE_LOCATION, // transition between routeId and the in-county special location of locationKind
    BEAT_ANCHOR_DOOR,           // door of the building with `role`
};

#define BEAT_LOCATION_NONE 0xFF

// Where a beat is triggered. An NPC or item stands in the building `role`, in the special location
// `locationKind`, on `routeId`, or, with none of those set, outdoors in the city. All anchors of a subplot
// belong to its host county, whose city is the anchor's city.
struct BeatAnchor
{
    u8 kind;
    u8 routeId;       // ROUTE_ID_NONE if not on a route
    u8 routeId2;      // ROUTE_ROUTE only
    u8 locationKind;  // enum RouteLocationKind or TownLocationKind, BEAT_LOCATION_NONE if none
    u16 role;         // BuildingRole bit, 0 if none
};

struct SubplotTextList
{
    const u8 *const *texts;
    u8 count;
};

#define TEXT_LIST(array) {array, ARRAY_COUNT(array)}

// What NPCs say while the subplot's stage is in [firstStage, lastStage] (design.md, "City planning"); the
// ends are real stages or pseudo-stages. Ranges of one subplot must not overlap, and together they must
// cover every real stage. Far = another city, close = the host city. An NPC picks by its selector, so one
// NPC always says the same line while the range holds. An empty list means "say generic text".
struct SubplotHint
{
    u8 firstStage;
    u8 lastStage;
    struct SubplotTextList far;
    struct SubplotTextList close;
};

// What a stage's hint template counts in {STR_VAR_1}.
enum SubplotHintCount
{
    SUBPLOT_COUNT_NONE,
    SUBPLOT_COUNT_BADGES_SHORT, // badges still missing for the subplot's minBadges
};

enum SubplotKeyType
{
    SUBPLOT_KEY_NONE,
    SUBPLOT_KEY_HOST_POOL, // a = index into the host county's McGuffin pool (GetCountyMcGuffinId)
    SUBPLOT_KEY_ITEM,      // a = enum PlotKind, b = ordinal; a planner item that other hosts may consume
};

struct SubplotKeyRef
{
    u8 type;
    u8 a;
    u8 b;
};

#define SUBPLOT_MAX_REQUIRED_KEYS 2

struct SubplotStage
{
    struct BeatAnchor anchor;   // where the stage's beat is triggered
    u16 hinterRoles;            // BuildingRole bits of the buildings where this stage's hinters may stand; 0 = anywhere in the county
    u8 numHinters;              // hinter NPCs the county must place for this stage, 0 = none
    u16 extraRoles;             // BuildingRole bits of further buildings the stage's cutscene visits besides the anchor
    u8 hintCount;               // enum SubplotHintCount
    struct StoryPredicate done; // adapter backend only: stage reached; stage 0 is the default and never tested
};

// An NPC the subplot controls entirely while its stage is in [firstStage, lastStage].
struct SubplotActor
{
    u8 firstStage;
    u8 lastStage;
    u16 role; // BuildingRole bit of the building hosting the actor; 0 = outdoors
    u16 gfxId;
};

struct SubplotDesc
{
    u8 backend;
    u8 numStages;
    u8 numActors;
    u8 numHints;
    u16 requiredRoles; // BuildingRole bits the host city must have
    u8 minBadges;      // badges that must be held before the subplot starts
    u8 producedKeyStage; // the key is produced once the subplot has reached this stage
    struct SubplotKeyRef producedKey;
    bool8 givesReward; // the end reward is the McGuffin in producedKey, or a rare item when producedKey is none; FALSE = story advancement only
    // Intro McGuffins: every key must be produced before the subplot can start, so a chain entry's
    // predecessor hands it over (design.md, "County chain").
    struct SubplotKeyRef requiredKeys[SUBPLOT_MAX_REQUIRED_KEYS];
    bool8 producesMainGet; // completion is a get of a main-plot list: mainGet names it, and the news points at this subplot
    struct PlotKeyRef mainGet;
    const struct SubplotStage *stages;
    const struct SubplotHint *hints;
    const struct SubplotActor *actors;
};

struct SubplotHost
{
    u8 county;
    u8 slot;
    const struct SubplotDesc *desc;
};

// Where Emerald hosts its ported subplots.
const struct SubplotHost *GetSubplotHosts(u32 *numHosts);

// NULL when the county hosts nothing in the slot.
const struct SubplotDesc *GetCountySubplot(u32 countyId, u32 slot);

u16 GetCountySlotVar(u32 countyId, u32 slot);

// Adapter subplots are read-only.
u32 GetSubplotStage(u32 countyId, u32 slot, const struct SubplotDesc *desc);
void SetSubplotStage(u32 countyId, u32 slot, const struct SubplotDesc *desc, u32 stage);

u32 GetCountySlotStage(u32 countyId, u32 slot);
void SetCountySlotStage(u32 countyId, u32 slot, u32 stage);

// The hint for a subplot at `subplotStage` (a real stage or a pseudo-stage), from the range covering it and
// the distance. The text is list[selector % count], so one NPC (selector = its identity) always says the
// same thing while the range holds and different NPCs differ. NULL means the NPC says generic text.
const u8 *GetSubplotHint(const struct SubplotDesc *desc, u32 subplotStage, u32 distance, u32 selector);

// TRUE iff `subplotStage` (a real stage or a pseudo-stage) lies in the hint's range.
bool32 DoesSubplotHintCover(const struct SubplotHint *hint, u32 subplotStage);

// The range covering `subplotStage`; NULL if none (the descriptor test requires every real stage covered).
const struct SubplotHint *FindSubplotHint(const struct SubplotDesc *desc, u32 subplotStage);

// The NPC-talk query: the hint for the (county, slot) subplot's current stage. NULL = generic text.
const u8 *GetCountySlotHint(u32 countyId, u32 slot, u32 distance, u32 selector);

// TRUE iff completing the subplot is the planner item (kind, ordinal); a gym item names its HM too.
bool32 DoesSubplotProduce(const struct SubplotDesc *desc, u32 kind, u32 ordinal);

// Actors this subplot wants present at `stage`: fills `out` (up to maxOut) and returns the total count.
u32 GetSubplotActors(const struct SubplotDesc *desc, u32 stage, const struct SubplotActor **out, u32 maxOut);

// City whose people are "close" to the anchor: the host county's, since subplots are single-county.
u32 GetBeatAnchorCounty(u32 hostCounty, const struct BeatAnchor *anchor);

// SUBPLOT_CLOSE iff the hinter stands in the anchor's city, else SUBPLOT_FAR.
u32 GetBeatDistance(u32 hostCounty, const struct BeatAnchor *anchor, u32 hinterCounty);

// Writes the reference for hint texts ("near the POKéMON CENTER", "near the road to SLATEPORT CITY", or
// "in LILYCOVE CITY" when the anchor names no place) at dst and returns the end of the string.
u8 *WriteBeatAnchorReference(u8 *dst, u32 hostCounty, const struct BeatAnchor *anchor);

// Copies the hint template src to dst, replacing each {HINT_ANCHOR} with WriteBeatAnchorReference's phrase
// and each {HINT_CITY} with the host county's city name, so a moved subplot names its new city.
// Other placeholders pass through for StringExpandPlaceholders. Returns the end (the EOS).
u8 *ExpandHintTemplate(u8 *dst, const u8 *src, u32 hostCounty, const struct BeatAnchor *anchor);

// McGuffin id of the key as hosted by hostCounty, MCGUFFIN_ID_NONE for SUBPLOT_KEY_NONE.
u32 ResolveSubplotKey(const struct SubplotKeyRef *ref, u32 hostCounty);

// The number a stage's hint takes in {STR_VAR_1}; 0 when the stage counts nothing.
u32 GetSubplotHintCount(const struct SubplotDesc *desc, u32 beat, u32 badgesHeld);

// Puts the beat's hint count in gStringVar1 for the template's {STR_VAR_1}; call before showing any hint text.
void BufferSubplotHintCount(const struct SubplotDesc *desc, u32 beat);

// Every BuildingRole bit the host city must have: requiredRoles, each stage's anchor role and extraRoles.
u32 GetSubplotNeededRoles(const struct SubplotDesc *desc);

// TRUE iff a hosted subplot has produced the McGuffin: its key resolves to the id and its stage has reached
// producedKeyStage. `hosts` is the whole set to scan; stages are read through the county slot vars.
bool32 IsMcGuffinProducedBy(const struct SubplotHost *hosts, u32 numHosts, u32 mcguffinId);

// Runtime availability: enough badges held and every consumed key produced by one of `hosts`.
bool32 IsSubplotAvailable(const struct SubplotDesc *desc, u32 hostCounty, const struct SubplotHost *hosts, u32 numHosts);

enum SubplotPlacementProblem
{
    SUBPLOT_PLACEMENT_OK,
    SUBPLOT_PLACEMENT_NO_CITY,
    SUBPLOT_PLACEMENT_MISSING_ROLE,
    SUBPLOT_PLACEMENT_BEFORE_BADGES,   // produces a main-plot get that comes no later than the minBadges-th gym
    SUBPLOT_PLACEMENT_KEY_NOT_EARLIER, // a consumed key has no producer in an earlier county or slot
};

// Static check for putting `desc` in (county, slot) among `hosts` (which must not list that slot): the
// county has a city with every needed role; a subplot that produces a main-plot get comes after the
// minBadges-th gym (any other just waits for the badges at runtime); each consumed key has a producer
// hosted in an earlier county, or in an earlier slot of this one.
u32 CheckSubplotPlacement(const struct SubplotDesc *desc, u32 county, u32 slot, const struct SubplotHost *hosts, u32 numHosts);

// IsSubplotAvailable over Emerald's hosts, for the subplot in (county, slot).
bool32 IsCountySlotAvailable(u32 countyId, u32 slot);

// County whose city has this MAPSEC; asserts there is none.
u32 GetCountyFromCityMapSec(u32 mapSec);

#endif // GUARD_SUBPLOT_H
