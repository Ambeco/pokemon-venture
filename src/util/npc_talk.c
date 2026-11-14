#include "global.h"
#include "assertf.h"
#include "line_break.h"
#include "string_util.h"
#include "text.h"
#include "data.h"
#include "battle_main.h"
#include "constants/pokemon.h"
#include "constants/identity_salts.h"
#include "city_hinters.h"
#include "identity_seed.h"
#include "event_data.h"
#include "global.fieldmap.h"
#include "npc_talk.h"
#include "plot_stage.h"
#include "region_city.h"
#include "region_county.h"
#include "region_tile.h"
#include "subplot.h"

#define NUM_MAIN_INDICES (NUM_COUNTIES * 4)
#define LINES_PER_POOL   2

struct TalkPool
{
    const u8 *lines[LINES_PER_POOL];
};

static const struct TalkPool sThemePools[NUM_CITY_THEMES] =
{
    [CITY_THEME_NONE] = {{COMPOUND_STRING("{HINT_CITY} is a quiet place."), COMPOUND_STRING("Nothing much happens in {HINT_CITY}.")}},
    [CITY_THEME_START] = {{COMPOUND_STRING("Every TRAINER's journey starts in a small town like {HINT_CITY}."), COMPOUND_STRING("I've never left {HINT_CITY}, but one day I will.")}},
    [CITY_THEME_SCARCE] = {{COMPOUND_STRING("Everything is hard to come by in {HINT_CITY}."), COMPOUND_STRING("We make do with very little here in {HINT_CITY}.")}},
    [CITY_THEME_NATURE] = {{COMPOUND_STRING("The trees around {HINT_CITY} are wonderful this time of year."), COMPOUND_STRING("{HINT_CITY} is surrounded by nature. The air is so clean!")}},
    [CITY_THEME_INDUSTRY] = {{COMPOUND_STRING("Machines hum all day in {HINT_CITY}. You get used to the noise."), COMPOUND_STRING("{HINT_CITY} runs on hard work.")}},
    [CITY_THEME_TINY_ISLAND] = {{COMPOUND_STRING("There isn't much room on this island, but everyone knows everyone."), COMPOUND_STRING("The sea is all you see from {HINT_CITY}.")}},
    [CITY_THEME_PORT] = {{COMPOUND_STRING("Ships come and go from {HINT_CITY} all day long."), COMPOUND_STRING("The harbor in {HINT_CITY} never sleeps.")}},
    [CITY_THEME_ENTERTAINMENT] = {{COMPOUND_STRING("There is always something going on in {HINT_CITY}!"), COMPOUND_STRING("{HINT_CITY} is the place to be for fun and games.")}},
    [CITY_THEME_HIGHLAND] = {{COMPOUND_STRING("The air is thin up here in {HINT_CITY}, but the view is worth it."), COMPOUND_STRING("{HINT_CITY} sits high above the rest of the region.")}},
    [CITY_THEME_FARMLAND] = {{COMPOUND_STRING("The fields around {HINT_CITY} are golden this season."), COMPOUND_STRING("Most folks in {HINT_CITY} work the land.")}},
    [CITY_THEME_HOT_SPRINGS] = {{COMPOUND_STRING("The hot springs of {HINT_CITY} soothe any ache."), COMPOUND_STRING("Steam drifts through {HINT_CITY} all year round.")}},
    [CITY_THEME_TREETOP] = {{COMPOUND_STRING("Our homes in {HINT_CITY} are built high in the trees."), COMPOUND_STRING("You can see {HINT_CITY} from far away, up among the branches.")}},
    [CITY_THEME_CAPITAL_COAST] = {{COMPOUND_STRING("{HINT_CITY} is where everything in the region comes together."), COMPOUND_STRING("The coast of {HINT_CITY} is lovely, and busy too.")}},
    [CITY_THEME_CHERISH] = {{COMPOUND_STRING("{HINT_CITY} is a place to cherish."), COMPOUND_STRING("Take your time in {HINT_CITY}. There's no need to rush.")}},
    [CITY_THEME_ANCIENT] = {{COMPOUND_STRING("{HINT_CITY} is older than anyone can remember."), COMPOUND_STRING("Ancient stories are told in {HINT_CITY}.")}},
    [CITY_THEME_OCEAN_SUNRISE] = {{COMPOUND_STRING("The sun rises over the sea first in {HINT_CITY}."), COMPOUND_STRING("A sunrise in {HINT_CITY} is a sight to see.")}},
    [CITY_THEME_LEAGUE] = {{COMPOUND_STRING("The strongest TRAINERS come to {HINT_CITY}."), COMPOUND_STRING("Everyone in {HINT_CITY} dreams of the POKéMON LEAGUE.")}},
};

// Indexed by the bit number of the BuildingRole.
static const struct TalkPool sRolePools[NUM_BUILDING_ROLES] =
{
    [0] = {{COMPOUND_STRING("The POKéMON CENTER in {HINT_CITY} is always busy."), COMPOUND_STRING("Rest your POKéMON before heading out.")}},
    [1] = {{COMPOUND_STRING("The MART in {HINT_CITY} has everything a TRAINER needs."), COMPOUND_STRING("Stock up on POTIONS before you go.")}},
    [2] = {{COMPOUND_STRING("The GYM in {HINT_CITY} is a proving ground."), COMPOUND_STRING("Train hard before you challenge the GYM.")}},
    [3] = {{COMPOUND_STRING("Only the best get this far."), COMPOUND_STRING("The LEAGUE is the end of the road for most TRAINERS.")}},
    [4] = {{COMPOUND_STRING("Home is where your POKéMON are healed."), COMPOUND_STRING("Come back and rest any time.")}},
    [5] = {{COMPOUND_STRING("Your things are safe on the PC."), COMPOUND_STRING("Don't leave POKéMON in the PC for too long. They get lonely.")}},
    [6] = {{COMPOUND_STRING("The MARKET in {HINT_CITY} has bargains."), COMPOUND_STRING("Everything is fresh at the MARKET.")}},
    [7] = {{COMPOUND_STRING("Many travelers stay at the MOTEL in {HINT_CITY}."), COMPOUND_STRING("A good night's sleep does wonders.")}},
    [8] = {{COMPOUND_STRING("The HERB SHOP in {HINT_CITY} sells bitter remedies."), COMPOUND_STRING("Herbal medicine works, but it tastes awful.")}},
    [9] = {{COMPOUND_STRING("Soak in the HOT SPRINGS. You'll feel like new."), COMPOUND_STRING("POKéMON love the hot springs too.")}},
    [10] = {{COMPOUND_STRING("The HALL in {HINT_CITY} hosts all kinds of events."), COMPOUND_STRING("Something is always happening at the HALL.")}},
    [11] = {{COMPOUND_STRING("The FAN CLUB in {HINT_CITY} is full of devoted members."), COMPOUND_STRING("Fans can talk about POKéMON for hours.")}},
    [12] = {{COMPOUND_STRING("The kids at the SCHOOL in {HINT_CITY} study hard."), COMPOUND_STRING("Knowing your types wins battles.")}},
    [13] = {{COMPOUND_STRING("They offer a useful service here."), COMPOUND_STRING("Ask the staff. They know their stuff.")}},
    [14] = {{COMPOUND_STRING("This is a nice house, isn't it?"), COMPOUND_STRING("We've lived in {HINT_CITY} for years.")}},
};

static const struct TalkPool sThanksPool =
{
    {COMPOUND_STRING("Thanks to you, things are calmer in {HINT_CITY} these days."), COMPOUND_STRING("Everyone in {HINT_CITY} owes you one!")}
};

// Which type beats a gym of this type. Unlisted types (none, mystery, stellar) have no gym.
static const u8 *const sGymTypeTips[NUMBER_OF_MON_TYPES] =
{
    [TYPE_NORMAL] = COMPOUND_STRING("FIGHTING moves are a good bet."),
    [TYPE_FIGHTING] = COMPOUND_STRING("FLYING and PSYCHIC moves are a good bet."),
    [TYPE_FLYING] = COMPOUND_STRING("ELECTRIC and ICE moves are a good bet."),
    [TYPE_POISON] = COMPOUND_STRING("GROUND and PSYCHIC moves are a good bet."),
    [TYPE_GROUND] = COMPOUND_STRING("WATER, GRASS and ICE moves are a good bet."),
    [TYPE_ROCK] = COMPOUND_STRING("WATER and GRASS moves are a good bet."),
    [TYPE_BUG] = COMPOUND_STRING("FIRE and FLYING moves are a good bet."),
    [TYPE_GHOST] = COMPOUND_STRING("GHOST and DARK moves are a good bet."),
    [TYPE_STEEL] = COMPOUND_STRING("FIRE and GROUND moves are a good bet."),
    [TYPE_FIRE] = COMPOUND_STRING("WATER and ROCK moves are a good bet."),
    [TYPE_WATER] = COMPOUND_STRING("GRASS and ELECTRIC moves are a good bet."),
    [TYPE_GRASS] = COMPOUND_STRING("FIRE and ICE moves are a good bet."),
    [TYPE_ELECTRIC] = COMPOUND_STRING("GROUND moves are a good bet."),
    [TYPE_PSYCHIC] = COMPOUND_STRING("BUG, GHOST and DARK moves are a good bet."),
    [TYPE_ICE] = COMPOUND_STRING("FIRE and FIGHTING moves are a good bet."),
    [TYPE_DRAGON] = COMPOUND_STRING("ICE and DRAGON moves are a good bet."),
    [TYPE_DARK] = COMPOUND_STRING("FIGHTING and FAIRY moves are a good bet."),
    [TYPE_FAIRY] = COMPOUND_STRING("STEEL and POISON moves are a good bet."),
};

static u32 GetPoolSize(const struct TalkPool *pool)
{
    u32 n = 0;

    while (n < LINES_PER_POOL && pool->lines[n] != NULL)
        n++;
    return n;
}

u32 GetNpcThemePoolSize(u32 theme)
{
    return theme < NUM_CITY_THEMES ? GetPoolSize(&sThemePools[theme]) : 0;
}

const u8 *GetNpcThemeLine(u32 theme, u32 pick)
{
    u32 size = GetNpcThemePoolSize(theme);

    assertf(size != 0, "city theme %d has no talk lines; did you mean to add its pool to sThemePools?", theme)
    {
        return sThemePools[CITY_THEME_NONE].lines[0];
    }
    return sThemePools[theme].lines[pick % size];
}

// Bit number of a single BuildingRole bit, or NUM_BUILDING_ROLES when role is not exactly one known bit.
static u32 GetRoleIndex(u32 role)
{
    u32 n;

    for (n = 0; n < NUM_BUILDING_ROLES; n++)
    {
        if (role == (1u << n))
            return n;
    }
    return NUM_BUILDING_ROLES;
}

u32 GetNpcRolePoolSize(u32 role)
{
    u32 n = GetRoleIndex(role);

    return n < NUM_BUILDING_ROLES ? GetPoolSize(&sRolePools[n]) : 0;
}

const u8 *GetNpcRoleLine(u32 role, u32 pick)
{
    u32 size = GetNpcRolePoolSize(role);

    assertf(size != 0, "building role 0x%x has no talk lines; did you mean to pass one BuildingRole bit, or to add its pool to sRolePools?", role)
    {
        return sRolePools[0].lines[0];
    }
    return sRolePools[GetRoleIndex(role)].lines[pick % size];
}

u32 GetNpcThanksPoolSize(void)
{
    return GetPoolSize(&sThanksPool);
}

const u8 *GetNpcThanksLine(u32 pick)
{
    return sThanksPool.lines[pick % GetNpcThanksPoolSize()];
}

const u8 *GetNpcGymTypeTip(u32 type)
{
    return type < NUMBER_OF_MON_TYPES ? sGymTypeTips[type] : NULL;
}

bool32 IsArcHinterNpc(u32 county, u32 localId)
{
    // Offsetting by a hash of the county spreads which ids hint, while any NPC_TALK_PERIOD consecutive ids
    // always hold exactly one.
    return (localId + IdentityHash(GetPlayerIdentitySeed(), NPC_SALT_TALK, county)) % NPC_TALK_PERIOD == 0;
}

// First main-story-index at or after `index` with something to do, or HINTER_NO_BEAT.
static u32 GetCurrentBeat(u32 index)
{
    u32 host;

    if (index >= NUM_MAIN_INDICES)
        return HINTER_NO_BEAT;
    if (GetMainBeatHost(index, &host))
        return index;
    return GetNextMainBeat(index);
}

// TRUE iff the county hosts a beat and the main-story-index is past all of them.
static bool32 IsCountyDoneWithArc(u32 county, u32 mainIndex)
{
    u32 index, host;

    for (index = mainIndex; index < NUM_MAIN_INDICES; index++)
    {
        if (GetMainBeatHost(index, &host) && host == county)
            return FALSE;
    }
    for (index = 0; index < mainIndex && index < NUM_MAIN_INDICES; index++)
    {
        if (GetMainBeatHost(index, &host) && host == county)
            return TRUE;
    }
    return FALSE;
}

static const struct BeatAnchor sNoAnchor = {BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0};

static void WriteExpanded(u8 *dst, const u8 *line, u32 county)
{
    ExpandHintTemplate(dst, line, county, &sNoAnchor);
    BreakStringForFieldMessage(dst);
}

// Three fixed shapes around the leader name, the type name and the type's tip.
static void WriteGymTalk(u8 *dst, u32 county, u32 pick)
{
    static const u8 sCityGymLeader[] = _("The GYM LEADER of {HINT_CITY} is ");
    static const u8 sUses[] = _(" uses ");
    static const u8 sTypePokemon[] = _("-type POKéMON. ");
    static const u8 sHeading[] = _("Heading to the GYM? Expect ");
    static const u8 sFrom[] = _("-type POKéMON from ");
    static const u8 sStop[] = _(". ");
    static const u8 sMeans[] = _(" specializes in ");
    u8 buf[200];
    u8 *end = buf;
    const u8 *tip;
    enum Type type = GetCountyGymType(county);

    switch (pick % 3)
    {
    case 0:
        end = StringCopy(end, sCityGymLeader);
        end = WriteGymLeaderName(end, county);
        end = StringCopy(end, sMeans);
        end = StringCopy(end, gTypesInfo[type].name);
        end = StringCopy(end, sTypePokemon);
        break;
    case 1:
        end = WriteGymLeaderName(end, county);
        end = StringCopy(end, sUses);
        end = StringCopy(end, gTypesInfo[type].name);
        end = StringCopy(end, sTypePokemon);
        break;
    default:
        end = StringCopy(end, sHeading);
        end = StringCopy(end, gTypesInfo[type].name);
        end = StringCopy(end, sFrom);
        end = WriteGymLeaderName(end, county);
        end = StringCopy(end, sStop);
        break;
    }
    tip = sGymTypeTips[type];
    assertf(tip != NULL, "gym type %d has no tip; did you mean to add it to sGymTypeTips?", type)
    {
        tip = sStop;
    }
    StringCopy(end, tip);
    WriteExpanded(dst, buf, county);
}

u32 WriteGenericNpcTalk(u8 *dst, u32 county, u32 role, u32 localId, u32 mainIndex)
{
    u32 h, beat;

    assertf(DoesCountyHaveCity(county), "county %d has no city, so no NPC there has generic talk; did you mean a city's county?", county)
    {
        dst[0] = EOS;
        return NPC_TALK_THEME;
    }
    h = IdentityHash(GetPlayerIdentitySeed(), NPC_SALT_TALK, county | (localId << 8) | (mainIndex << 16));
    beat = GetCurrentBeat(mainIndex);
    if (IsArcHinterNpc(county, localId) && beat != HINTER_NO_BEAT && mainIndex < MAIN_STORY_INDEX_POSTGAME)
    {
        WritePokemonCenterNews(dst, beat, county);
        return NPC_TALK_ARC;
    }
    if (h % 3 == 0 && IsCountyDoneWithArc(county, mainIndex)) // the cheap test first: this scans the main plot
    {
        WriteExpanded(dst, GetNpcThanksLine(h >> 8), county);
        return NPC_TALK_THANKS;
    }
    if (role == BUILDING_GYM && GetCountyGymType(county) != TYPE_NONE)
    {
        WriteGymTalk(dst, county, h >> 8);
        return NPC_TALK_GYM;
    }
    if (role != 0 && (h >> 4) % 4 != 0)
    {
        WriteExpanded(dst, GetNpcRoleLine(role, h >> 8), county);
        return NPC_TALK_ROLE;
    }
    WriteExpanded(dst, GetNpcThemeLine(GetCountyCityTheme(county), h >> 8), county);
    return NPC_TALK_THEME;
}

// Script special: VAR_0x8004 = the BuildingRole bit of the NPC's building (0 outdoors); the talking NPC is
// gSpecialVar_LastTalked. Puts the text in gStringVar4; VAR_RESULT is always TRUE.
void ShowGenericNpcTalk(void)
{
    WriteGenericNpcTalk(gStringVar4, GetCountyFromCityMapSec(gMapHeader.regionMapSectionId), gSpecialVar_0x8004, gSpecialVar_LastTalked, GetMainStoryIndex());
    gSpecialVar_Result = TRUE;
}
