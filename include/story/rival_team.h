#ifndef GUARD_RIVAL_TEAM_H
#define GUARD_RIVAL_TEAM_H

/**
 * A rival's team and the swap rules that fold a route's wild species into it (design_pokemon.md, "Rival teams").
 *
 * Pure logic: it reads no vars and no maps. A candidate is a species judged by its line's final form, so the
 * caller supplies that form's types and catch rate, and the line's base species as its family. The team always
 * holds up to RIVAL_TEAM_MAX members; the size of a battle's team is applied only when the battle party is built.
 *
 * The first `protectedCount` members (a traditional starter) are never dropped. A secondary rival passes the
 * primary's team as `exclude`: it never takes a family the primary holds, and counts its own member of such a
 * family as easier than any catch rate, so anything with no conflicting type replaces it.
 *
 * One pass per route; the result need not be stable, so folding a route again may change the team a little.
 */

#include "global.h"

#define RIVAL_TEAM_MAX             6
#define RIVAL_ROUTE_MAX_CANDIDATES 32

struct RivalMember
{
    u16 species;
    u16 family;   // base species of the line
    u8 types[2];  // final form's types; equal when mono-typed
    u8 catchRate; // final form's catch rate
};

struct RivalTeam
{
    u8 count;
    u8 protectedCount;
    struct RivalMember members[RIVAL_TEAM_MAX];
};

// An empty team, or one holding only the starter when `starter` is not NULL.
void InitRivalTeam(struct RivalTeam *team, const struct RivalMember *starter);

// Bit mask over `enum Type` of the types the team covers.
u32 GetRivalTeamTypeMask(const struct RivalTeam *team);

// Applies rules 1 to 4 to one candidate. `exclude` is the primary rival's team, or NULL. Returns TRUE iff the team changed.
bool32 FoldRivalCandidate(struct RivalTeam *team, const struct RivalMember *candidate, const struct RivalTeam *exclude);

// Folds every candidate of one route (at most RIVAL_ROUTE_MAX_CANDIDATES), once each: candidates adding a missing
// fire, grass or water type first, then lowest catch rate first. Returns TRUE iff the team changed.
bool32 FoldRivalRoute(struct RivalTeam *team, const struct RivalMember *candidates, u32 numCandidates, const struct RivalTeam *exclude);

#endif // GUARD_RIVAL_TEAM_H
