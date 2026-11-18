#ifndef GUARD_RIVAL_TEAM_H
#define GUARD_RIVAL_TEAM_H

/**
 * The rival's team and the swap rules that fold a route's wild species into it (design_pokemon.md, "Rival teams").
 *
 * Pure logic: it reads no vars and no maps. A candidate is a species judged by its line's final form, so the
 * caller supplies that form's types and catch rate. Member 0 is the rival's starter and is never dropped.
 *
 * FoldRivalRoute repeats its pass until nothing changes, so folding the same route again is a no-op and the
 * route needs no "seen" flag. Walking back into an old route can still change the team if a later route changed
 * it in between (rule 2 may then accept what was refused before).
 */

#include "global.h"

#define RIVAL_TEAM_MAX      6
#define RIVAL_ROUTE_MAX_CANDIDATES 32

struct RivalMember
{
    u16 species;
    u8 types[2]; // final form's types; equal when mono-typed
    u8 catchRate; // final form's catch rate
};

struct RivalTeam
{
    u8 count;
    struct RivalMember members[RIVAL_TEAM_MAX];
};

// A team holding only the starter.
void InitRivalTeam(struct RivalTeam *team, const struct RivalMember *starter);

// Bit mask over `enum Type` of the types the team covers.
u32 GetRivalTeamTypeMask(const struct RivalTeam *team);

// Applies rules 1 to 4 to one candidate with the team capped at `cap` members (at most RIVAL_TEAM_MAX).
// Returns TRUE iff the team changed.
bool32 FoldRivalCandidate(struct RivalTeam *team, u32 cap, const struct RivalMember *candidate);

// Folds every candidate of one route (at most RIVAL_ROUTE_MAX_CANDIDATES): candidates adding a missing fire,
// grass or water type first, then lowest catch rate first, repeated until the team stops changing.
// Returns TRUE iff the team changed.
bool32 FoldRivalRoute(struct RivalTeam *team, u32 cap, const struct RivalMember *candidates, u32 numCandidates);

#endif // GUARD_RIVAL_TEAM_H
