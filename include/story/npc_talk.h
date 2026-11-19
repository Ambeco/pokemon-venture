#ifndef GUARD_NPC_TALK_H
#define GUARD_NPC_TALK_H

/**
 * Generic NPC talk (design_city_npcs.md, "NPC dialogue"): what a city NPC with no story text of its own says.
 *
 * WriteGenericNpcTalk() picks one line from the text pools for (county, building role, NPC local id, main-story-
 * index) and stores nothing: the choice is an identity hash of those four, so an NPC says the same thing every
 * time while the main-story-index holds, and a different NPC (or state) may say something else. In order:
 *
 * 1. Arc hint: a deterministic quarter of a city's NPCs (`IsArcHinterNpc`, exactly one in every four consecutive
 *    local ids) say the Pokémon Center news for the current main beat, so every city hints at every arc state.
 * 2. Thank-you: once the main-story-index is past every beat the county hosts, some NPCs thank the player.
 * 3. Gym talk: an NPC in a GYM names the leader and type and gives a tip for that type.
 * 4. Building talk: an NPC in a building says a line of that building role's pool.
 * 5. Theme talk: an NPC outdoors (or one in three of those in buildings) says a line of the city's theme pool.
 *
 * Every pool is complete: a role or theme without lines asserts, rather than falling back to silence. Lines may
 * use {HINT_CITY}; output is line-broken for the field message box.
 */

#include "global.h"

enum NpcTalkKind
{
    NPC_TALK_ARC,
    NPC_TALK_THANKS,
    NPC_TALK_GYM,
    NPC_TALK_ROLE,
    NPC_TALK_THEME,
};

#define NPC_TALK_PERIOD 4 // one NPC in this many local ids is an arc hinter

// Pools, for tests and tools. Sizes are 0 for an index without a pool.
u32 GetNpcThemePoolSize(u32 theme);
const u8 *GetNpcThemeLine(u32 theme, u32 pick);
u32 GetNpcRolePoolSize(u32 role); // role is one BuildingRole bit
const u8 *GetNpcRoleLine(u32 role, u32 pick);
u32 GetNpcThanksPoolSize(void);
const u8 *GetNpcThanksLine(u32 pick);
const u8 *GetNpcGymTypeTip(u32 type); // NULL for a type no gym uses

// TRUE iff the NPC with this local id in the county's city says an arc hint (while there is a beat to hint).
bool32 IsArcHinterNpc(u32 county, u32 localId);

// Writes the talk of the NPC and returns its enum NpcTalkKind. `role` is the BuildingRole bit of the building
// the NPC stands in, 0 outdoors. The county must have a city.
u32 WriteGenericNpcTalk(u8 *dst, u32 county, u32 role, u32 localId, u32 mainIndex);

#endif // GUARD_NPC_TALK_H
