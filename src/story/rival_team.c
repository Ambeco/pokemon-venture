#include "global.h"
#include "assertf.h"
#include "constants/pokemon.h"
#include "rival_team.h"

#define KEY_TYPES ((1u << TYPE_FIRE) | (1u << TYPE_GRASS) | (1u << TYPE_WATER))

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

static bool32 HoldsFamily(const struct RivalTeam *team, u32 family)
{
    u32 i;

    if (team == NULL)
        return FALSE;
    for (i = 0; i < team->count; i++)
    {
        if (team->members[i].family == family)
            return TRUE;
    }
    return FALSE;
}

// A secondary rival's member of a family the primary now holds counts as catch rate 256, so even a 255 candidate replaces it.
static u32 GetEffectiveCatchRate(const struct RivalMember *member, const struct RivalTeam *exclude)
{
    return HoldsFamily(exclude, member->family) ? 256 : member->catchRate;
}

static u32 GetProtectedMask(const struct RivalTeam *team)
{
    return (1u << team->protectedCount) - 1;
}

// Rule 1 and the size limit: dropping `dropIndices` and adding the candidate must keep every key type the team
// covers now, and fit in RIVAL_TEAM_MAX.
static bool32 CanReplace(const struct RivalTeam *team, u32 candMask, u32 dropIndices)
{
    u32 kept = GetRivalTeamTypeMask(team) & KEY_TYPES;

    if (team->count - CountBits(dropIndices) + 1 > RIVAL_TEAM_MAX)
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

// The droppable member with the highest effective catch rate that passes `CanReplace`, limited to those above
// `catchRateAbove`; -1 if none. Protected members are never droppable and the candidate may not overlap them.
static int FindMemberToDrop(const struct RivalTeam *team, u32 candMask, int catchRateAbove, const struct RivalTeam *exclude)
{
    int best = -1;
    u32 i, protectedMask = GetProtectedMask(team);

    if (candMask & GetMaskWithout(team, ~protectedMask))
        return -1;
    for (i = team->protectedCount; i < team->count; i++)
    {
        u32 othersMask = GetMaskWithout(team, (1u << i) | protectedMask);

        if ((int)GetEffectiveCatchRate(&team->members[i], exclude) <= catchRateAbove || (candMask & othersMask) != 0)
            continue;
        if (!CanReplace(team, candMask, 1u << i))
            continue;
        if (best < 0 || GetEffectiveCatchRate(&team->members[i], exclude) > GetEffectiveCatchRate(&team->members[best], exclude))
            best = i;
    }
    return best;
}

void InitRivalTeam(struct RivalTeam *team, const struct RivalMember *starter)
{
    team->count = 0;
    team->protectedCount = 0;
    if (starter != NULL)
    {
        team->count = 1;
        team->protectedCount = 1;
        team->members[0] = *starter;
    }
}

u32 GetRivalTeamTypeMask(const struct RivalTeam *team)
{
    return GetMaskWithout(team, 0);
}

bool32 FoldRivalCandidate(struct RivalTeam *team, const struct RivalMember *candidate, const struct RivalTeam *exclude)
{
    u32 i, candMask = GetMemberMask(candidate), teamMask = GetRivalTeamTypeMask(team), overlapDrop = 0;
    int drop;

    assertf(team->count <= RIVAL_TEAM_MAX && team->protectedCount <= team->count, "rival team has %d members (%d protected) but holds at most %d; did you call InitRivalTeam?", team->count, team->protectedCount, RIVAL_TEAM_MAX)
    {
        return FALSE;
    }
    if (HoldsFamily(exclude, candidate->family))
        return FALSE;
    for (i = 0; i < team->count; i++)
    {
        if (team->members[i].species == candidate->species)
            return FALSE;
        if (i >= team->protectedCount && (GetMemberMask(&team->members[i]) & candMask))
            overlapDrop |= 1u << i;
    }
    if (candMask & KEY_TYPES & ~teamMask)
    {
        // Rule 2: a missing fire, grass or water type is always taken, dropping the members it overlaps.
        if (candMask & GetMaskWithout(team, ~GetProtectedMask(team)))
            return FALSE;
        if (overlapDrop == 0 && team->count >= RIVAL_TEAM_MAX)
        {
            drop = FindMemberToDrop(team, candMask, -1, exclude);
            if (drop < 0)
                return FALSE;
            overlapDrop = 1u << drop;
        }
        if (!CanReplace(team, candMask, overlapDrop))
            return FALSE;
        ApplyReplace(team, candidate, overlapDrop);
        return TRUE;
    }
    if (team->count < RIVAL_TEAM_MAX && !(teamMask & candMask))
    {
        // Rule 4: a free slot takes a candidate that overlaps nothing.
        ApplyReplace(team, candidate, 0);
        return TRUE;
    }
    // Rule 3: replace a member with a higher catch rate, if the candidate overlaps no other member.
    drop = FindMemberToDrop(team, candMask, candidate->catchRate, exclude);
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

bool32 FoldRivalRoute(struct RivalTeam *team, const struct RivalMember *candidates, u32 numCandidates, const struct RivalTeam *exclude)
{
    bool32 changed = FALSE;
    u32 k, doneMask = 0;

    assertf(numCandidates <= RIVAL_ROUTE_MAX_CANDIDATES, "route has %d rival candidates but the limit is %d; did you pass the whole encounter table instead of its distinct species?", numCandidates, RIVAL_ROUTE_MAX_CANDIDATES)
    {
        return FALSE;
    }
    for (k = 0; k < numCandidates; k++)
    {
        u32 next = PickNextCandidate(team, candidates, numCandidates, doneMask);

        doneMask |= 1u << next;
        changed |= FoldRivalCandidate(team, &candidates[next], exclude);
    }
    return changed;
}
