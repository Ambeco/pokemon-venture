#ifndef GUARD_CHAPTER_H
#define GUARD_CHAPTER_H

/**
 * Chapter descriptor, stage accessor and NPC-talk query (design_chapters.md, "Chapter abstraction").
 *
 * A chapter is a linear list of stages. A county hosts up to CHAPTERS_PER_COUNTY chapters, each in a
 * numbered slot; callers address a chapter by (county, slot) and never by identity, so moving a chapter to
 * another host moves no state. A stored chapter keeps its stage in the host county's slice var for that
 * slot (a county's var 0 is reserved and unused). An adapter chapter has no state of its own:
 * its stage is the highest stage whose done-predicate is true, over Emerald's old flags and vars, and it
 * cannot be set. The query functions cannot tell the two apart.
 *
 * Requirements and produced keys (design_chapters.md, "Placement and requirements"): a chapter lists the building
 * roles its host city needs (those it names itself, the roles of every stage anchor and extraRoles, so a
 * cutscene across several buildings needs them all), the badges it needs and the keys it consumes. A key is
 * named relative to the host (an index into the host county's McGuffin pool) or as a planner item, so it
 * moves with the chapter. CheckChapterPlacement is the static placement check, IsChapterAvailable the runtime
 * one. A stage's hint may take a number (ChapterStage.hintCount), which the talk special puts in
 * {STR_VAR_1}. A derived stage needs no stored counter: use an adapter chapter with a STORY_PRED_BADGES_GE
 * predicate per stage.
 *
 * Slots are valid only for counties whose slice is free of legacy vars (COUNTY_VAR_SLICES_FREE). Specials
 * for scripts act on the county of the current map's MAPSEC, which must be a county's city for now.
 */

#include "global.h"
#include "plot_stage.h"
#include "region_route.h"
#include "region_unlock.h"

#define CHAPTERS_PER_COUNTY (VARS_PER_COUNTY - 1)
#define COUNTY_VAR_SLICES_FREE ((LEGACY_VARS_START - COUNTY_VARS_START) / VARS_PER_COUNTY)

enum ChapterBackend
{
    CHAPTER_STORED,
    CHAPTER_ADAPTER,
    CHAPTER_PROXY, // a chain entry standing for a parallel subplot (design_chapters.md, "Parallel subplots"): done exactly when requiredKeys[0] is produced
};

// ChapterHost.slot of a proxy: it has no stage var, so it takes no slot and is found by descriptor.
#define CHAPTER_SLOT_STATELESS 0xFD

// Where the talking NPC is relative to the beat: far = another city or county, close = at the place.
enum ChapterDistance
{
    CHAPTER_FAR,
    CHAPTER_CLOSE,
};

// Pseudo-stages around a chapter's real stages 0..numStages-1: BEFORE is "not started" (a chain entry the
// chain has not reached), AFTER is "completed". Hint ranges may use them as ends; the stored stage never is one.
#define CHAPTER_STAGE_BEFORE 0xFE
#define CHAPTER_STAGE_AFTER  0xFF

// What triggers a beat (design_city_npcs.md, "City planning", beat placement).
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
// `locationKind`, on `routeId`, or, with none of those set, outdoors in the city. All anchors of a chapter
// belong to its host county, whose city is the anchor's city.
struct BeatAnchor
{
    u8 kind;
    u8 routeId;       // ROUTE_ID_NONE if not on a route
    u8 routeId2;      // ROUTE_ROUTE only
    u8 locationKind;  // enum RouteLocationKind or TownLocationKind, BEAT_LOCATION_NONE if none
    u16 role;         // BuildingRole bit, 0 if none
};

struct ChapterTextList
{
    const u8 *const *texts;
    u8 count;
};

#define TEXT_LIST(array) {array, ARRAY_COUNT(array)}

// What NPCs say while the chapter's stage is in [firstStage, lastStage] (design_city_npcs.md, "City planning"); the
// ends are real stages or pseudo-stages. Ranges of one chapter must not overlap, and together they must
// cover every real stage. Far = another city, close = the host city. An NPC picks by its selector, so one
// NPC always says the same line while the range holds. An empty list means "say generic text".
struct ChapterHint
{
    u8 firstStage;
    u8 lastStage;
    struct ChapterTextList far;
    struct ChapterTextList close;
};

// What a stage's hint template counts in {STR_VAR_1}.
enum ChapterHintCount
{
    CHAPTER_COUNT_NONE,
    CHAPTER_COUNT_BADGES_SHORT, // badges still missing for the chapter's minBadges
};

enum ChapterKeyType
{
    CHAPTER_KEY_NONE,
    CHAPTER_KEY_HOST_POOL, // a = index into the host county's McGuffin pool (GetCountyMcGuffinId)
    CHAPTER_KEY_ITEM,      // a = enum PlotKind, b = ordinal; a planner item that other hosts may consume
    CHAPTER_KEY_SHOP_ITEM, // a | b << 8 = item id; a Mart item (SHOP_ITEM_KEY), produced by the chapter whose stage is buying it
};

#define SHOP_ITEM_KEY(item) {CHAPTER_KEY_SHOP_ITEM, (item) & 0xFF, (item) >> 8}

struct ChapterKeyRef
{
    u8 type;
    u8 a;
    u8 b;
};

#define CHAPTER_MAX_REQUIRED_KEYS 2

struct ChapterStage
{
    struct BeatAnchor anchor;   // where the stage's beat is triggered
    u16 hinterRoles;            // BuildingRole bits of the buildings where this stage's hinters may stand; 0 = anywhere in the county
    u8 numHinters;              // hinter NPCs the county must place for this stage, 0 = none
    u16 extraRoles;             // BuildingRole bits of further buildings the stage's cutscene visits besides the anchor
    u8 hintCount;               // enum ChapterHintCount
    struct StoryPredicate done; // adapter backend only: stage reached; stage 0 is the default and never tested
};

// An NPC the chapter controls entirely while its stage is in [firstStage, lastStage].
struct ChapterActor
{
    u8 firstStage;
    u8 lastStage;
    u16 role; // BuildingRole bit of the building hosting the actor; 0 = outdoors
    u16 gfxId;
};

struct ChapterDesc
{
    u8 backend;
    u8 numStages;
    u8 numActors;
    u8 numHints;
    u16 requiredRoles; // BuildingRole bits the host city must have
    u8 minBadges;      // badges that must be held before the chapter starts
    u8 producedKeyStage; // the key is produced once the chapter has reached this stage
    struct ChapterKeyRef producedKey;
    bool8 givesReward; // the end reward is the McGuffin in producedKey, or a rare item when producedKey is none; FALSE = story advancement only
    // Intro McGuffins: every key must be produced before the chapter can start, so a chain entry's
    // predecessor hands it over (design_chapters.md, "County chain").
    struct ChapterKeyRef requiredKeys[CHAPTER_MAX_REQUIRED_KEYS];
    bool8 waitsForLaterKey; // an off-chain chapter that may wait at runtime for a key whose producer comes later in story order (CheckChapterPlacement)
    bool8 producesMainGet; // completion is a get of a main-plot list: mainGet names it, and the news points at this chapter
    struct PlotKeyRef mainGet;
    u8 mainHm; // enum PlotHm: the HM the chapter hands out (a get of its gym's list, not a McGuffin); PLOT_HM_NONE for none
    const struct ChapterStage *stages;
    const struct ChapterHint *hints;
    const struct ChapterActor *actors;
};

// ChapterHost.mainSlot of a chapter that is not on its county's chain: it runs beside the chain from the county's entry.
#define CHAPTER_MAIN_SLOT_OFF_CHAIN 0xFF

// ChapterHost.mainSlot of a chapter hosted for a callback: the callback list (step 1) of main-story block `block`, which
// another county's block owns. It sorts at (block, step 1), not in the host county's own block.
#define CHAPTER_MAIN_SLOT_CALLBACK_BASE 0x40
#define CHAPTER_MAIN_SLOT_CALLBACK(block) (CHAPTER_MAIN_SLOT_CALLBACK_BASE + (block))

struct ChapterHost
{
    u8 county;
    u8 slot;
    const struct ChapterDesc *desc;
    u8 mainSlot; // planned main-story slot (0-3), CHAPTER_MAIN_SLOT_CALLBACK(block) or CHAPTER_MAIN_SLOT_OFF_CHAIN; the static host tables leave it 0, GetPlannedChapterHosts fills it
};

// Where Emerald hosts its ported chapters.
const struct ChapterHost *GetChapterHosts(u32 *numHosts);

struct SubplotHost;

// Where Emerald groups its ported chapters into subplots (subplot.h).
const struct SubplotHost *GetSubplotHosts(u32 *numSubplots);

// The vanilla slot of the main-plot get the chapter supplies (mainGet, else its HM); FALSE when it supplies none.
bool32 GetChapterSuppliedSlot(const struct ChapterDesc *desc, struct PlotSlot *out);

// NULL when the county hosts nothing in the slot.
const struct ChapterDesc *GetCountyChapter(u32 countyId, u32 slot);

// The index-th chapter the county hosts stateless (gym, proxy or adapter, in host-table order); NULL past the last.
const struct ChapterDesc *GetCountyStatelessChapter(u32 countyId, u32 index);

u16 GetCountySlotVar(u32 countyId, u32 slot);

// Adapter chapters are read-only.
u32 GetChapterStage(u32 countyId, u32 slot, const struct ChapterDesc *desc);
void SetChapterStage(u32 countyId, u32 slot, const struct ChapterDesc *desc, u32 stage);

u32 GetCountySlotStage(u32 countyId, u32 slot);
void SetCountySlotStage(u32 countyId, u32 slot, u32 stage);

// The hint for a chapter at `chapterStage` (a real stage or a pseudo-stage), from the range covering it and
// the distance. The text is list[selector % count], so one NPC (selector = its identity) always says the
// same thing while the range holds and different NPCs differ. NULL means the NPC says generic text.
const u8 *GetChapterHint(const struct ChapterDesc *desc, u32 chapterStage, u32 distance, u32 selector);

// TRUE iff `chapterStage` (a real stage or a pseudo-stage) lies in the hint's range.
bool32 DoesChapterHintCover(const struct ChapterHint *hint, u32 chapterStage);

// The range covering `chapterStage`; NULL if none (the descriptor test requires every real stage covered).
const struct ChapterHint *FindChapterHint(const struct ChapterDesc *desc, u32 chapterStage);

// The NPC-talk query: the hint for the (county, slot) chapter's current stage. NULL = generic text.
const u8 *GetCountySlotHint(u32 countyId, u32 slot, u32 distance, u32 selector);

// GetCountySlotHint for the index-th stateless chapter (GetCountyStatelessChapter), which has no slot to name.
const u8 *GetCountyStatelessHint(u32 countyId, u32 index, u32 distance, u32 selector);

// TRUE iff completing the chapter is the planner item (kind, ordinal); a gym item names its HM too.
bool32 DoesChapterProduce(const struct ChapterDesc *desc, u32 kind, u32 ordinal);

// The real stage whose anchor and hint count a hint at `stage` uses: BEFORE names stage 0, AFTER the last
// stage, a real stage itself. A hint in a pseudo-stage range thus reads like a hint about the neighbouring
// real stage; a hinter bound to a real beat still passes that beat instead.
u32 ResolveHintStage(const struct ChapterDesc *desc, u32 stage);

// Actors this chapter wants present at `stage` (a real stage or a pseudo-stage; ranges may end at one): fills `out` (up to maxOut) and returns the total count.
u32 GetChapterActors(const struct ChapterDesc *desc, u32 stage, const struct ChapterActor **out, u32 maxOut);

// City whose people are "close" to the anchor: the host county's, since chapters are single-county.
u32 GetBeatAnchorCounty(u32 hostCounty, const struct BeatAnchor *anchor);

// CHAPTER_CLOSE iff the hinter stands in the anchor's city, else CHAPTER_FAR.
u32 GetBeatDistance(u32 hostCounty, const struct BeatAnchor *anchor, u32 hinterCounty);

// Writes the reference for hint texts ("near the POKéMON CENTER", "near the road to SLATEPORT CITY", or
// "in LILYCOVE CITY" when the anchor names no place) at dst and returns the end of the string.
u8 *WriteBeatAnchorReference(u8 *dst, u32 hostCounty, const struct BeatAnchor *anchor);

// Writes the anchor's building or location kind name ("MART", "cave"), or "spot" for a route or city anchor.
u8 *WriteBeatAnchorPlace(u8 *dst, const struct BeatAnchor *anchor);

// Copies the hint template src to dst, replacing each {HINT_ANCHOR} with WriteBeatAnchorReference's phrase
// each {HINT_PLACE} with WriteBeatAnchorPlace's name and each {HINT_CITY} with the host county's city name,
// so a moved chapter names its new city ("the {HINT_PLACE} in {HINT_CITY}").
// Other placeholders pass through for StringExpandPlaceholders. Returns the end (the EOS).
u8 *ExpandHintTemplate(u8 *dst, const u8 *src, u32 hostCounty, const struct BeatAnchor *anchor);

// As ExpandHintTemplate for an NPC of county `referrer` talking about a chapter hosted in hostCounty. `callback` marks a
// hint about a callback beat, which names the host's landmark.
u8 *ExpandHintTemplateFrom(u8 *dst, const u8 *src, u32 referrer, u32 hostCounty, const struct BeatAnchor *anchor, bool32 callback);

// How an NPC of county `referrer` names county `target` (what {HINT_CITY} expands to). A county with a city is its
// city's name. A cityless one is "these parts" (same county), "the area <dir> of here" (adjacent), "the area near
// <landmark>" (a callback hint, or the main story block `mainBlock` is already past the target), "the area <dir> of
// <city or landmark>" (an earlier-numbered neighbor the player has seen), else "the far <dir>, near <landmark>".
// Returns the end of the string.
u8 *WriteCountyReferenceAt(u8 *dst, u32 referrer, u32 target, bool32 callback, u32 mainBlock);

// WriteCountyReferenceAt for the current main-story block (GetMainStoryIndex() >> 2).
u8 *WriteCountyReference(u8 *dst, u32 referrer, u32 target, bool32 callback);

// McGuffin id of the key as hosted by hostCounty, MCGUFFIN_ID_NONE for CHAPTER_KEY_NONE.
u32 ResolveChapterKey(const struct ChapterKeyRef *ref, u32 hostCounty);

// The number a stage's hint takes in {STR_VAR_1}; 0 when the stage counts nothing.
u32 GetChapterHintCount(const struct ChapterDesc *desc, u32 beat, u32 badgesHeld);

// Puts the beat's hint count in gStringVar1 for the template's {STR_VAR_1}; call before showing any hint text.
void BufferChapterHintCount(const struct ChapterDesc *desc, u32 beat);

// Every BuildingRole bit the host city must have: requiredRoles, each stage's anchor role and extraRoles.
u32 GetChapterNeededRoles(const struct ChapterDesc *desc);

// TRUE iff a hosted chapter has produced the McGuffin: its key resolves to the id and its stage has reached
// producedKeyStage. `hosts` is the whole set to scan; stages are read through the county slot vars.
bool32 IsMcGuffinProducedBy(const struct ChapterHost *hosts, u32 numHosts, u32 mcguffinId);

// Runtime availability: enough badges held and every consumed key produced by one of `hosts`.
bool32 IsChapterAvailable(const struct ChapterDesc *desc, u32 hostCounty, const struct ChapterHost *hosts, u32 numHosts);

enum ChapterPlacementProblem
{
    CHAPTER_PLACEMENT_OK,
    CHAPTER_PLACEMENT_NO_CITY,
    CHAPTER_PLACEMENT_MISSING_ROLE,
    CHAPTER_PLACEMENT_BEFORE_BADGES,   // produces a main-plot get that comes no later than the minBadges-th gym
    CHAPTER_PLACEMENT_KEY_NOT_EARLIER, // a consumed key has no producer in an earlier county or slot (waitsForLaterKey: no producer at another position)
};

// Static check for putting `desc` in `mainSlot` (a main-story slot or CHAPTER_MAIN_SLOT_OFF_CHAIN) of `county` among
// `hosts`: the county has a city with every needed role; a chapter that produces a main-plot get comes after the
// minBadges-th gym (any other just waits for the badges at runtime); each consumed key has a producer strictly earlier in
// story order, which is the producer's county story index, then its main-story slot (an off-chain chapter is open from
// its county's entry; a callback chapter sits at its callback block's step 1). Producers in the same county and slot tie, and
// a tie is not earlier. An off-chain chapter with waitsForLaterKey may instead wait for a later producer (any other position). `hosts` must not
// list `desc` itself, and carry each host's planned mainSlot (GetPlannedChapterHosts).
u32 CheckChapterPlacement(const struct ChapterDesc *desc, u32 county, u32 mainSlot, const struct ChapterHost *hosts, u32 numHosts);

enum ProxyProblem
{
    PROXY_OK,
    PROXY_NO_SUBPLOT,              // no single chapter of the county produces the proxy's key at its last stage, in a subplot the county hosts
    PROXY_BADGES_BELOW_SUBPLOT,    // soft-lock condition 2: the proxy opens with fewer badges than a chapter of the subplot needs
    PROXY_SUBPLOT_ON_CHAIN,        // condition 4: the subplot has a required entry, so it can wait behind the chain
    PROXY_SUBPLOT_NEEDS_LATER_KEY, // condition 1: the subplot consumes a key not produced strictly before the proxy's position
    PROXY_SUBPLOT_MISPLACED,       // condition 3: the subplot's chapter fails CheckChapterPlacement
};

// The four soft-lock conditions (design_chapters.md, "Parallel subplots (proxy entries)") for one proxy host. `hosts` is
// the planned set including the proxy; the subplot is the one chapter of the proxy's county that produces its key.
u32 CheckProxySoftLock(const struct ChapterHost *proxy, const struct ChapterHost *hosts, u32 numHosts, const struct SubplotHost *subplots, u32 numSubplots);

// TRUE iff the chain before (county, slot) is done: the main-story-index has reached the county's slots
// (4x + s, x its story index), so every earlier county is finished, and the chapter before it in its subplot
// reached its last stage (or it is first in its subplot, or in none). Chapters of other subplots never matter.
// A chapter with no declared key is unlocked by this (design_chapters.md, "County chain").
bool32 IsChainPredecessorComplete(const struct ChapterHost *hosts, u32 numHosts, u32 county, u32 slot, u32 mainStoryIndex);

// IsChapterAvailable plus IsChainPredecessorComplete over Emerald's hosts, for the chapter in (county, slot).
bool32 IsCountySlotAvailable(u32 countyId, u32 slot);

// County whose city has this MAPSEC; asserts there is none.
u32 GetCountyFromCityMapSec(u32 mapSec);

#endif // GUARD_CHAPTER_H
