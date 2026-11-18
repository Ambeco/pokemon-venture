#include "global.h"
#include "test/test.h"
#include "constants/pokemon.h"
#include "rival_team.h"

static struct RivalMember Mon(u16 species, u8 type1, u8 type2, u8 catchRate)
{
    struct RivalMember m = {.species = species, .types = {type1, type2}, .catchRate = catchRate};

    return m;
}

static struct RivalMember Mono(u16 species, u8 type, u8 catchRate)
{
    return Mon(species, type, type, catchRate);
}

static bool32 HasSpecies(const struct RivalTeam *team, u16 species)
{
    u32 i;

    for (i = 0; i < team->count; i++)
    {
        if (team->members[i].species == species)
            return TRUE;
    }
    return FALSE;
}

static bool32 TeamsEqual(const struct RivalTeam *a, const struct RivalTeam *b)
{
    u32 i;

    if (a->count != b->count)
        return FALSE;
    for (i = 0; i < a->count; i++)
    {
        if (a->members[i].species != b->members[i].species)
            return FALSE;
    }
    return TRUE;
}

TEST("RivalTeam: A new team holds only the starter")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45);

    InitRivalTeam(&team, &starter);
    EXPECT_EQ(team.count, 1);
    EXPECT_EQ(team.members[0].species, 1);
    EXPECT_EQ(GetRivalTeamTypeMask(&team), 1u << TYPE_FIRE);
}

TEST("RivalTeam: Rule 4 gives a free slot to a candidate that overlaps nothing")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), normal = Mono(2, TYPE_NORMAL, 255);

    InitRivalTeam(&team, &starter);
    EXPECT(FoldRivalCandidate(&team, 2, &normal));
    EXPECT_EQ(team.count, 2);
    EXPECT(HasSpecies(&team, 2));
}

TEST("RivalTeam: Rule 4 does not exceed the cap")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), normal = Mono(2, TYPE_NORMAL, 255), rock = Mono(3, TYPE_ROCK, 255);

    InitRivalTeam(&team, &starter);
    EXPECT(FoldRivalCandidate(&team, 2, &normal));
    EXPECT(!FoldRivalCandidate(&team, 2, &rock));
    EXPECT_EQ(team.count, 2);
}

TEST("RivalTeam: A species already on the team is ignored")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), normal = Mono(2, TYPE_NORMAL, 255);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, 6, &normal);
    EXPECT(!FoldRivalCandidate(&team, 6, &normal));
    EXPECT_EQ(team.count, 2);
}

TEST("RivalTeam: Rule 2 takes a missing key type and drops the member it overlaps")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), flyer = Mono(2, TYPE_FLYING, 255), grassFlyer = Mon(3, TYPE_GRASS, TYPE_FLYING, 120);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, 2, &flyer);
    EXPECT(FoldRivalCandidate(&team, 2, &grassFlyer));
    EXPECT(HasSpecies(&team, 3));
    EXPECT(!HasSpecies(&team, 2));
    EXPECT_EQ(team.count, 2);
}

TEST("RivalTeam: Rule 1 refuses a swap that removes the only fire type, even to add a missing grass type")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_WATER, 45), fireFlyer = Mon(2, TYPE_FIRE, TYPE_FLYING, 200), grassFlyer = Mon(3, TYPE_GRASS, TYPE_FLYING, 120);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, 2, &fireFlyer);
    EXPECT(!FoldRivalCandidate(&team, 2, &grassFlyer));
    EXPECT(HasSpecies(&team, 2));
    EXPECT(!HasSpecies(&team, 3));
}

TEST("RivalTeam: A candidate that overlaps the starter cannot add a missing key type")
{
    struct RivalTeam team;
    struct RivalMember starter = Mon(1, TYPE_FIRE, TYPE_FIGHTING, 45), grassFighter = Mon(2, TYPE_GRASS, TYPE_FIGHTING, 100);

    InitRivalTeam(&team, &starter);
    EXPECT(!FoldRivalCandidate(&team, 6, &grassFighter));
    EXPECT_EQ(team.count, 1);
}

TEST("RivalTeam: Rule 2 on a full team with no overlap replaces the highest catch rate member that keeps coverage")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), normal = Mono(2, TYPE_NORMAL, 255), rock = Mono(3, TYPE_ROCK, 100), grass = Mono(4, TYPE_GRASS, 90);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, 3, &normal);
    FoldRivalCandidate(&team, 3, &rock);
    EXPECT(FoldRivalCandidate(&team, 3, &grass));
    EXPECT(!HasSpecies(&team, 2));
    EXPECT(HasSpecies(&team, 3));
    EXPECT(HasSpecies(&team, 4));
}

TEST("RivalTeam: Rule 3 swaps in a lower catch rate candidate that overlaps no other member")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), normal = Mono(2, TYPE_NORMAL, 255), rare = Mono(3, TYPE_ROCK, 60);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, 2, &normal);
    EXPECT(FoldRivalCandidate(&team, 2, &rare));
    EXPECT(HasSpecies(&team, 3));
    EXPECT(!HasSpecies(&team, 2));
}

TEST("RivalTeam: Rule 3 keeps a member that is easier to catch than the candidate")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), rare = Mono(2, TYPE_NORMAL, 60), common = Mono(3, TYPE_ROCK, 200);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, 2, &rare);
    EXPECT(!FoldRivalCandidate(&team, 2, &common));
    EXPECT(HasSpecies(&team, 2));
}

TEST("RivalTeam: Rule 3 refuses a candidate overlapping two members")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), a = Mono(2, TYPE_NORMAL, 255), b = Mono(3, TYPE_ROCK, 255), both = Mon(4, TYPE_NORMAL, TYPE_ROCK, 50);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, 3, &a);
    FoldRivalCandidate(&team, 3, &b);
    EXPECT(!FoldRivalCandidate(&team, 3, &both));
    EXPECT_EQ(team.count, 3);
}

TEST("RivalTeam: Rule 3 never drops the starter or a member holding the only key type")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_NORMAL, 255), fire = Mono(2, TYPE_FIRE, 200), rare = Mono(3, TYPE_ROCK, 20);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, 2, &fire);
    EXPECT(!FoldRivalCandidate(&team, 2, &rare));
    EXPECT(HasSpecies(&team, 1));
    EXPECT(HasSpecies(&team, 2));
}

TEST("RivalTeam: Folding a route takes candidates adding a missing key type before cheaper ones")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45);
    struct RivalMember route[] = {Mono(2, TYPE_NORMAL, 30), Mono(3, TYPE_GRASS, 200)};

    InitRivalTeam(&team, &starter);
    EXPECT(FoldRivalRoute(&team, 2, route, ARRAY_COUNT(route)));
    EXPECT(HasSpecies(&team, 3));
    EXPECT(!HasSpecies(&team, 2));
}

TEST("RivalTeam: Folding a route twice gives the same team as folding it once")
{
    u32 seed, i;

    for (seed = 1; seed <= 40; seed++)
    {
        struct RivalTeam once, twice;
        struct RivalMember starter = Mono(1, TYPE_FIRE, 45);
        struct RivalMember route[12];
        u32 state = seed * 2654435761u;

        for (i = 0; i < ARRAY_COUNT(route); i++)
        {
            state = state * 1664525u + 1013904223u;
            route[i] = Mon(2 + i, (state >> 16) % NUMBER_OF_MON_TYPES, (state >> 24) % NUMBER_OF_MON_TYPES, 3 + (state >> 8) % 250);
            if (route[i].types[0] == TYPE_MYSTERY) route[i].types[0] = TYPE_NORMAL;
            if (route[i].types[1] == TYPE_MYSTERY) route[i].types[1] = TYPE_NORMAL;
        }
        InitRivalTeam(&once, &starter);
        FoldRivalRoute(&once, 4, route, ARRAY_COUNT(route));
        twice = once;
        EXPECT(!FoldRivalRoute(&twice, 4, route, ARRAY_COUNT(route)));
        EXPECT(TeamsEqual(&once, &twice));
        EXPECT(once.count <= 4);
        EXPECT_EQ(once.members[0].species, 1);
    }
}

TEST("RivalTeam: Folding never loses a key type the team covered")
{
    u32 seed, i;

    for (seed = 1; seed <= 40; seed++)
    {
        struct RivalTeam team;
        struct RivalMember starter = Mono(1, TYPE_WATER, 45);
        u32 state = seed * 40503u, route, before;

        InitRivalTeam(&team, &starter);
        for (route = 0; route < 6; route++)
        {
            struct RivalMember cands[8];

            for (i = 0; i < ARRAY_COUNT(cands); i++)
            {
                state = state * 1664525u + 1013904223u;
                cands[i] = Mon(100 + route * 8 + i, (state >> 16) % TYPE_FAIRY, (state >> 24) % TYPE_FAIRY, 3 + (state >> 8) % 250);
                if (cands[i].types[0] == TYPE_MYSTERY) cands[i].types[0] = TYPE_NORMAL;
                if (cands[i].types[1] == TYPE_MYSTERY) cands[i].types[1] = TYPE_NORMAL;
            }
            before = GetRivalTeamTypeMask(&team) & ((1u << TYPE_FIRE) | (1u << TYPE_GRASS) | (1u << TYPE_WATER));
            FoldRivalRoute(&team, 1 + route, cands, ARRAY_COUNT(cands));
            EXPECT_EQ(before & GetRivalTeamTypeMask(&team), before);
            EXPECT(team.count <= 1 + route);
        }
    }
}
