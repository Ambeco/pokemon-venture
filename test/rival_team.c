#include "global.h"
#include "test/test.h"
#include "constants/pokemon.h"
#include "rival_team.h"

#define KEY_MASK ((1u << TYPE_FIRE) | (1u << TYPE_GRASS) | (1u << TYPE_WATER))

static struct RivalMember Mon(u16 species, u8 type1, u8 type2, u8 catchRate)
{
    struct RivalMember m = {.species = species, .family = species, .types = {type1, type2}, .catchRate = catchRate};

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

TEST("RivalTeam: A new team holds only the starter, protected")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45);

    InitRivalTeam(&team, &starter);
    EXPECT_EQ(team.count, 1);
    EXPECT_EQ(team.protectedCount, 1);
    EXPECT_EQ(GetRivalTeamTypeMask(&team), 1u << TYPE_FIRE);
}

TEST("RivalTeam: A team without a starter starts empty and protects nobody")
{
    struct RivalTeam team;
    struct RivalMember first = Mono(1, TYPE_NORMAL, 200);

    InitRivalTeam(&team, NULL);
    EXPECT_EQ(team.count, 0);
    EXPECT_EQ(team.protectedCount, 0);
    EXPECT(FoldRivalCandidate(&team, &first, NULL));
    EXPECT_EQ(team.count, 1);
}

TEST("RivalTeam: Rule 4 gives a free slot to a candidate that overlaps nothing")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), normal = Mono(2, TYPE_NORMAL, 255);

    InitRivalTeam(&team, &starter);
    EXPECT(FoldRivalCandidate(&team, &normal, NULL));
    EXPECT_EQ(team.count, 2);
    EXPECT(HasSpecies(&team, 2));
}

TEST("RivalTeam: The team never exceeds six members")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45);
    u32 i;

    InitRivalTeam(&team, &starter);
    for (i = 0; i < 12; i++)
    {
        struct RivalMember m = Mono(10 + i, TYPE_NORMAL + i % 8, 255);

        FoldRivalCandidate(&team, &m, NULL);
    }
    EXPECT(team.count <= RIVAL_TEAM_MAX);
    EXPECT_EQ(team.members[0].species, 1);
}

TEST("RivalTeam: A species already on the team is ignored")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), normal = Mono(2, TYPE_NORMAL, 255);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, &normal, NULL);
    EXPECT(!FoldRivalCandidate(&team, &normal, NULL));
    EXPECT_EQ(team.count, 2);
}

TEST("RivalTeam: Rule 2 takes a missing key type and drops the member it overlaps")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), flyer = Mono(2, TYPE_FLYING, 255), grassFlyer = Mon(3, TYPE_GRASS, TYPE_FLYING, 120);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, &flyer, NULL);
    EXPECT(FoldRivalCandidate(&team, &grassFlyer, NULL));
    EXPECT(HasSpecies(&team, 3));
    EXPECT(!HasSpecies(&team, 2));
    EXPECT_EQ(team.count, 2);
}

TEST("RivalTeam: Rule 1 refuses a swap that removes the only fire type, even to add a missing grass type")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_WATER, 45), fireFlyer = Mon(2, TYPE_FIRE, TYPE_FLYING, 200), grassFlyer = Mon(3, TYPE_GRASS, TYPE_FLYING, 120);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, &fireFlyer, NULL);
    EXPECT(!FoldRivalCandidate(&team, &grassFlyer, NULL));
    EXPECT(HasSpecies(&team, 2));
    EXPECT(!HasSpecies(&team, 3));
}

TEST("RivalTeam: A candidate that overlaps the starter cannot add a missing key type")
{
    struct RivalTeam team;
    struct RivalMember starter = Mon(1, TYPE_FIRE, TYPE_FIGHTING, 45), grassFighter = Mon(2, TYPE_GRASS, TYPE_FIGHTING, 100);

    InitRivalTeam(&team, &starter);
    EXPECT(!FoldRivalCandidate(&team, &grassFighter, NULL));
    EXPECT_EQ(team.count, 1);
}

TEST("RivalTeam: Rule 2 on a full team with no overlap replaces the highest catch rate member that keeps coverage")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), grass = Mono(9, TYPE_GRASS, 90);
    struct RivalMember filler[] = {Mono(2, TYPE_NORMAL, 255), Mono(3, TYPE_ROCK, 100), Mono(4, TYPE_GROUND, 120), Mono(5, TYPE_ICE, 110), Mono(6, TYPE_BUG, 130)};
    u32 i;

    InitRivalTeam(&team, &starter);
    for (i = 0; i < ARRAY_COUNT(filler); i++)
        FoldRivalCandidate(&team, &filler[i], NULL);
    ASSUME(team.count == RIVAL_TEAM_MAX);
    EXPECT(FoldRivalCandidate(&team, &grass, NULL));
    EXPECT(!HasSpecies(&team, 2));
    EXPECT(HasSpecies(&team, 9));
    EXPECT_EQ(team.count, RIVAL_TEAM_MAX);
}

TEST("RivalTeam: Rule 3 swaps in a lower catch rate candidate that overlaps one member")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), normal = Mono(2, TYPE_NORMAL, 255), rare = Mon(3, TYPE_NORMAL, TYPE_ROCK, 60);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, &normal, NULL);
    EXPECT(FoldRivalCandidate(&team, &rare, NULL));
    EXPECT(HasSpecies(&team, 3));
    EXPECT(!HasSpecies(&team, 2));
}

TEST("RivalTeam: Rule 3 keeps a member that is easier to catch than the candidate")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), rare = Mono(2, TYPE_NORMAL, 60), common = Mon(3, TYPE_NORMAL, TYPE_ROCK, 200);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, &rare, NULL);
    EXPECT(!FoldRivalCandidate(&team, &common, NULL));
    EXPECT(HasSpecies(&team, 2));
}

TEST("RivalTeam: Rule 3 refuses a candidate overlapping two members")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), a = Mono(2, TYPE_NORMAL, 255), b = Mono(3, TYPE_ROCK, 255), both = Mon(4, TYPE_NORMAL, TYPE_ROCK, 50);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, &a, NULL);
    FoldRivalCandidate(&team, &b, NULL);
    EXPECT(!FoldRivalCandidate(&team, &both, NULL));
    EXPECT_EQ(team.count, 3);
}

TEST("RivalTeam: Rule 3 never drops the starter")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_NORMAL, 255), rare = Mon(2, TYPE_NORMAL, TYPE_ROCK, 20);

    InitRivalTeam(&team, &starter);
    EXPECT(!FoldRivalCandidate(&team, &rare, NULL));
    EXPECT(HasSpecies(&team, 1));
}

TEST("RivalTeam: Rule 3 never drops the only holder of a key type")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_NORMAL, 255), fireFlyer = Mon(2, TYPE_FIRE, TYPE_FLYING, 200), rare = Mon(3, TYPE_FLYING, TYPE_ROCK, 20);

    InitRivalTeam(&team, &starter);
    FoldRivalCandidate(&team, &fireFlyer, NULL);
    EXPECT(!FoldRivalCandidate(&team, &rare, NULL));
    EXPECT(HasSpecies(&team, 2));
}

TEST("RivalTeam: Folding a route takes candidates adding a missing key type before cheaper ones")
{
    struct RivalTeam team;
    struct RivalMember starter = Mono(1, TYPE_FIRE, 45), normal = Mono(2, TYPE_NORMAL, 30), grass = Mono(3, TYPE_GRASS, 200);
    struct RivalMember route[2];

    route[0] = normal;
    route[1] = grass;
    InitRivalTeam(&team, &starter);
    EXPECT(FoldRivalRoute(&team, route, ARRAY_COUNT(route), NULL));
    EXPECT_EQ(team.members[1].species, 3);
    EXPECT_EQ(team.members[2].species, 2);
}

TEST("RivalTeam: A secondary rival never takes a family the primary holds")
{
    struct RivalTeam primary, secondary;
    struct RivalMember wallyMon = Mono(5, TYPE_PSYCHIC, 45), sameFamily = Mono(6, TYPE_NORMAL, 100), other = Mono(7, TYPE_ROCK, 100);

    wallyMon.family = 5;
    sameFamily.family = 5;
    InitRivalTeam(&primary, NULL);
    FoldRivalCandidate(&primary, &wallyMon, NULL);
    InitRivalTeam(&secondary, NULL);
    EXPECT(!FoldRivalCandidate(&secondary, &sameFamily, &primary));
    EXPECT(FoldRivalCandidate(&secondary, &other, &primary));
}

TEST("RivalTeam: A secondary member whose family the primary takes counts as catch rate 255 and is replaced")
{
    struct RivalTeam primary, secondary;
    struct RivalMember mine = Mono(5, TYPE_NORMAL, 20), cand = Mon(8, TYPE_NORMAL, TYPE_ROCK, 255), wallyMon = Mono(6, TYPE_PSYCHIC, 45);

    InitRivalTeam(&primary, NULL);
    InitRivalTeam(&secondary, NULL);
    FoldRivalCandidate(&secondary, &mine, &primary);
    EXPECT(!FoldRivalCandidate(&secondary, &cand, &primary));
    wallyMon.family = mine.family;
    FoldRivalCandidate(&primary, &wallyMon, NULL);
    EXPECT(FoldRivalCandidate(&secondary, &cand, &primary));
    EXPECT(HasSpecies(&secondary, 8));
    EXPECT(!HasSpecies(&secondary, 5));
}

TEST("RivalTeam: One route pass keeps the starter, six members at most and every key type held")
{
    u32 seed, i;

    for (seed = 1; seed <= 40; seed++)
    {
        struct RivalTeam team;
        struct RivalMember starter = Mono(1, TYPE_WATER, 45);
        u32 state = seed * 40503u, route, before;

        InitRivalTeam(&team, &starter);
        for (route = 0; route < 8; route++)
        {
            struct RivalMember cands[8];

            for (i = 0; i < ARRAY_COUNT(cands); i++)
            {
                state = state * 1664525u + 1013904223u;
                cands[i] = Mon(100 + route * 8 + i, (state >> 16) % TYPE_FAIRY, (state >> 24) % TYPE_FAIRY, 3 + (state >> 8) % 250);
                if (cands[i].types[0] == TYPE_MYSTERY)
                    cands[i].types[0] = TYPE_NORMAL;
                if (cands[i].types[1] == TYPE_MYSTERY)
                    cands[i].types[1] = TYPE_NORMAL;
            }
            before = GetRivalTeamTypeMask(&team) & KEY_MASK;
            FoldRivalRoute(&team, cands, ARRAY_COUNT(cands), NULL);
            EXPECT_EQ(before & GetRivalTeamTypeMask(&team), before);
            EXPECT(team.count <= RIVAL_TEAM_MAX);
            EXPECT_EQ(team.members[0].species, 1);
        }
    }
}
