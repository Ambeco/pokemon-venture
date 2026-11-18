#include "global.h"
#include "assertf.h"
#include "constants/pokemon.h"
#include "rival_team.h"

#define KEY_TYPES ((1u << TYPE_FIRE) | (1u << TYPE_GRASS) | (1u << TYPE_WATER))
#define MAX_PASSES 100 // each change raises (key coverage, size, -sum of catch rates), so the loop ends far sooner

static u32 GetMemberMask(const struct RivalMember *member)
{
    return (1u << member->types[0]) | (1u << member->types[1]);
}

static u32 GetMaskWithout(const struct RivalTeam *team, u32 dropIndices)
{
    u32 i, mask = 0;

    for (i = 0; i < team->count; i++)
    {
        if (!(dropIndices & (1u << i)))
            mask |= GetMemberMask(&team->members[i]);
    }
    return mask;
}

static u32 CountBits(u32 bits)
{
    u32 n = 0;

    for (; bits != 0; bits &= bits - 1)
        n++;
    return n;
}

// Rule 1 and the size cap: dropping `dropIndices` and adding the candidate must keep every key type the team
// covers now, and fit in `cap`.
static bool32 CanReplace(const struct RivalTeam *team, u32 cap, u32 candMask, u32 dropIndices)
{
    u32 kept = GetRivalTeamTypeMask(team) & KEY_TYPES;

    if (team->count - CountBits(dropIndices) + 1 > cap)
        return FALSE;
    return (kept & ~(candMask | GetMaskWithout(team, dropIndices))) == 0;
}

static void ApplyReplace(struct RivalTeam *team, const struct RivalMember *candidate, u32 dropIndices)
{
    u32 i, kept = 0;

    for (i = 0; i < team->count; i++)
    {
        if (!(dropIndices & (1u << i)))
            team->members[kept++] = team->members[i];
    }
    team->members[kept++] = *candidate;
    team->count = kept;
}

// The droppable (non-starter) member with the highest catch rate that passes `CanReplace`, limited to those
// above `catchRateAbove`; -1 if none.
static int FindMemberToDrop(const struct RivalTeam *team, u32 cap, u32 candMask, int catchRateAbove)
{
    int best = -1;
    u32 i;

    for (i = 1; i < team->count; i++)
    {
        u32 othersMask = GetMaskWithout(team, (1u << i) | 1u);

        if ((int)team->members[i].catchRate <= catchRateAbove || (candMask & othersMask) != 0 || (candMask & GetMemberMask(&team->members[0])) != 0)
            continue;
        if (!CanReplace(team, cap, candMask, 1u << i))
            continue;
        if (best < 0 || team->members[i].catchRate > team->members[best].catchRate)
            best = i;
    }
    return best;
}

void InitRivalTeam(struct RivalTeam *team, const struct RivalMember *starter)
{
    team->count = 1;
    team->members[0] = *starter;
}

u32 GetRivalTeamTypeMask(const struct RivalTeam *team)
{
    return GetMaskWithout(team, 0);
}

bool32 FoldRivalCandidate(struct RivalTeam *team, u32 cap, const struct RivalMember *candidate)
{
    u32 i, candMask = GetMemberMask(candidate), teamMask = GetRivalTeamTypeMask(team), overlapDrop = 0;
    int drop;

    assertf(cap >= 1 && cap <= RIVAL_TEAM_MAX && team->count >= 1 && team->count <= cap, "rival team of %d members does not fit the cap %d (max %d); did you call InitRivalTeam and lower the cap?", team->count, cap, RIVAL_TEAM_MAX)
    {
        return FALSE;
    }
    for (i = 0; i < team->count; i++)
    {
        if (team->members[i].species == candidate->species)
            return FALSE;
        if (i > 0 && (GetMemberMask(&team->members[i]) & candMask))
            overlapDrop |= 1u << i;
    }
    if (candMask & KEY_TYPES & ~teamMask)
    {
        // Rule 2: a missing fire, grass or water type is always taken, dropping the members it overlaps.
        if (GetMemberMask(&team->members[0]) & candMask)
            return FALSE;
        if (overlapDrop == 0 && team->count >= cap)
        {
            drop = FindMemberToDrop(team, cap, candMask, -1);
            if (drop < 0)
                return FALSE;
            overlapDrop = 1u << drop;
        }
        if (!CanReplace(team, cap, candMask, overlapDrop))
            return FALSE;
        ApplyReplace(team, candidate, overlapDrop);
        return TRUE;
    }
    if (team->count < cap && !(teamMask & candMask))
    {
        // Rule 4: a free slot takes a candidate that overlaps nothing.
        ApplyReplace(team, candidate, 0);
        return TRUE;
    }
    // Rule 3: replace a member with a higher catch rate, if the candidate overlaps no other member.
    drop = FindMemberToDrop(team, cap, candMask, candidate->catchRate);
    if (drop < 0)
        return FALSE;
    ApplyReplace(team, candidate, 1u << drop);
    return TRUE;
}

static u32 PickNextCandidate(const struct RivalTeam *team, const struct RivalMember *candidates, u32 numCandidates, u32 doneMask)
{
    u32 j, best = numCandidates, missing = KEY_TYPES & ~GetRivalTeamTypeMask(team);
    bool32 bestAdds = FALSE;

    for (j = 0; j < numCandidates; j++)
    {
        bool32 adds = (GetMemberMask(&candidates[j]) & missing) != 0;

        if (doneMask & (1u << j))
            continue;
        if (best == numCandidates || (adds && !bestAdds) || (adds == bestAdds && candidates[j].catchRate < candidates[best].catchRate))
        {
            best = j;
            bestAdds = adds;
        }
    }
    return best;
}

bool32 FoldRivalRoute(struct RivalTeam *team, u32 cap, const struct RivalMember *candidates, u32 numCandidates)
{
    bool32 changedAny = FALSE, changed;
    u32 pass = 0;

    assertf(numCandidates <= RIVAL_ROUTE_MAX_CANDIDATES, "route has %d rival candidates but the limit is %d; did you pass the whole encounter table instead of its distinct species?", numCandidates, RIVAL_ROUTE_MAX_CANDIDATES)
    {
        return FALSE;
    }
    do
    {
        u32 k, doneMask = 0;

        changed = FALSE;
        for (k = 0; k < numCandidates; k++)
        {
            u32 next = PickNextCandidate(team, candidates, numCandidates, doneMask);

            doneMask |= 1u << next;
            changed |= FoldRivalCandidate(team, cap, &candidates[next]);
        }
        changedAny |= changed;
    } while (changed && ++pass < MAX_PASSES);
    return changedAny;
}
