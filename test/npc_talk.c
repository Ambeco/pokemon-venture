#include "global.h"
#include "test/test.h"
#include "test/overworld_script.h"
#include "city_hinters.h"
#include "constants/characters.h"
#include "constants/pokemon.h"
#include "constants/region_map_sections.h"
#include "constants/script_commands.h"
#include "data.h"
#include "battle_main.h"
#include "event_data.h"
#include "line_break.h"
#include "npc_talk.h"
#include "plot_stage.h"
#include "region_city.h"
#include "region_county.h"
#include "region_tile.h"
#include "string_util.h"
#include "subplot.h"
#include "text.h"

#define LILYCOVE COUNTY_ID(4, 0)
#define RUSTBORO COUNTY_ID(0, 1)

extern const u8 LilycoveCity_EventScript_Man3Generic[];
extern const u8 LilycoveCity_EventScript_RichBoyGeneric[];
extern const u8 RustboroCity_EventScript_WomanGeneric[];
extern const u8 RustboroCity_EventScript_TwinGeneric[];
extern const u8 Common_EventScript_GenericNpcTalk[];

#define NUM_MAIN_INDICES (NUM_COUNTIES * 4)

// The field message window's text area is 208 px wide (lines end at a newline, scroll or clear control code).
static s32 GetWidestLineWidth(const u8 *str)
{
    static EWRAM_DATA u8 sLine[300];
    s32 widest = 0;
    u32 i, n = 0;

    for (i = 0; ; i++)
    {
        u8 c = str[i];

        if (c == EOS || c == CHAR_NEWLINE || c == CHAR_PROMPT_SCROLL || c == CHAR_PROMPT_CLEAR)
        {
            sLine[n] = EOS;
            widest = max(widest, GetStringWidth(FONT_NORMAL, sLine, 0));
            n = 0;
            if (c == EOS)
                break;
        }
        else
        {
            sLine[n++] = c;
        }
    }
    return widest;
}

static bool32 ContainsString(const u8 *haystack, const u8 *needle)
{
    u32 len = StringLength(needle), i, hay = StringLength(haystack);

    for (i = 0; i + len <= hay; i++)
    {
        if (StringCompareN(haystack + i, needle, len) == 0)
            return TRUE;
    }
    return FALSE;
}

static bool32 IsGymType(u32 type)
{
    return type >= TYPE_NORMAL && type <= TYPE_FAIRY && type != TYPE_MYSTERY;
}

TEST("NpcTalk: Every city theme, building role and the thank-yous have text, and every gym type a tip")
{
    u32 n, type;

    for (n = 0; n < NUM_CITY_THEMES; n++)
    {
        EXPECT_GE(GetNpcThemePoolSize(n), 1);
        EXPECT(GetNpcThemeLine(n, 0) != NULL);
    }
    for (n = 0; n < NUM_BUILDING_ROLES; n++)
    {
        EXPECT_GE(GetNpcRolePoolSize(1u << n), 1);
        EXPECT(GetNpcRoleLine(1u << n, 0) != NULL);
    }
    EXPECT_GE(GetNpcThanksPoolSize(), 1);
    for (type = 0; type < NUMBER_OF_MON_TYPES; type++)
        EXPECT_EQ(GetNpcGymTypeTip(type) != NULL, IsGymType(type));
}

TEST("NpcTalk: Roles that are no single building bit have no pool")
{
    EXPECT_EQ(GetNpcRolePoolSize(0), 0);
    EXPECT_EQ(GetNpcRolePoolSize(BUILDING_GYM | BUILDING_MART), 0);
    EXPECT_EQ(GetNpcRolePoolSize(1u << NUM_BUILDING_ROLES), 0);
}

// Each city's own theme and thanks lines plus every role line, with the city expanded, as the game shows them.
TEST("NpcTalk: Every pool line fits the message box with every city's name")
{
    static EWRAM_DATA u8 sText[300];
    u32 county, n, pick;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        if (!DoesCountyHaveCity(county))
            continue;
        for (pick = 0; pick < GetNpcThemePoolSize(GetCountyCityTheme(county)); pick++)
        {
            ExpandHintTemplate(sText, GetNpcThemeLine(GetCountyCityTheme(county), pick), county, &(struct BeatAnchor){BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0});
            BreakStringForFieldMessage(sText);
            EXPECT_LE(GetWidestLineWidth(sText), 208);
        }
        for (n = 0; n < NUM_BUILDING_ROLES; n++)
        {
            for (pick = 0; pick < GetNpcRolePoolSize(1u << n); pick++)
            {
                ExpandHintTemplate(sText, GetNpcRoleLine(1u << n, pick), county, &(struct BeatAnchor){BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0});
                BreakStringForFieldMessage(sText);
                EXPECT_LE(GetWidestLineWidth(sText), 208);
            }
        }
        for (pick = 0; pick < GetNpcThanksPoolSize(); pick++)
        {
            ExpandHintTemplate(sText, GetNpcThanksLine(pick), county, &(struct BeatAnchor){BEAT_ANCHOR_NPC, ROUTE_ID_NONE, ROUTE_ID_NONE, BEAT_LOCATION_NONE, 0});
            BreakStringForFieldMessage(sText);
            EXPECT_LE(GetWidestLineWidth(sText), 208);
        }
    }
}

TEST("NpcTalk: Every gym city's gym talk, in all three shapes, fits the message box")
{
    static EWRAM_DATA u8 sText[300];
    u32 county, localId, found;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        if (!DoesCountyHaveCity(county) || GetCountyGymOrdinal(county) == 0 || GetCountyGymType(county) == TYPE_NONE)
            continue;
        found = 0;
        for (localId = 0; localId < 40 && found < 6; localId++)
        {
            if (IsArcHinterNpc(county, localId) || WriteGenericNpcTalk(sText, county, BUILDING_GYM, localId, 0) != NPC_TALK_GYM)
                continue;
            found++;
            EXPECT_LE(GetWidestLineWidth(sText), 208);
        }
        EXPECT_GE(found, 1);
    }
}

TEST("NpcTalk: Exactly one of every four consecutive local ids is an arc hinter in every city")
{
    u32 county, start, i, hinters;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        if (!DoesCountyHaveCity(county))
            continue;
        for (start = 0; start < 12; start++)
        {
            hinters = 0;
            for (i = 0; i < NPC_TALK_PERIOD; i++)
                hinters += IsArcHinterNpc(county, start + i);
            EXPECT_EQ(hinters, 1);
        }
    }
}

// The done-when of Milestone 1, for what exists so far: from every city, every main-story-index with a beat
// ahead is hinted by an NPC there.
TEST("NpcTalk: Every city hints at the current beat of every main-story-index, through its arc hinter")
{
    static EWRAM_DATA u8 sText[300];
    static EWRAM_DATA u8 sNews[300];
    u32 county, localId, index, host;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        if (!DoesCountyHaveCity(county))
            continue;
        for (localId = 0; localId < NPC_TALK_PERIOD && !IsArcHinterNpc(county, localId); localId++)
            ;
        ASSUME(localId < NPC_TALK_PERIOD);
        for (index = 0; index < MAIN_STORY_INDEX_POSTGAME; index++)
        {
            u32 beat = GetMainBeatHost(index, &host) ? index : GetNextMainBeat(index);

            if (beat == HINTER_NO_BEAT)
                continue;
            EXPECT_EQ(WriteGenericNpcTalk(sText, county, 0, localId, index), NPC_TALK_ARC);
            WritePokemonCenterNews(sNews, beat, county);
            EXPECT_EQ(StringCompare(sText, sNews), 0);
        }
    }
}

TEST("NpcTalk: After the arc the arc hinter says no arc hint")
{
    static EWRAM_DATA u8 sText[300];
    u32 county, localId;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        if (!DoesCountyHaveCity(county))
            continue;
        for (localId = 0; localId < NPC_TALK_PERIOD; localId++)
            EXPECT_NE(WriteGenericNpcTalk(sText, county, 0, localId, MAIN_STORY_INDEX_POSTGAME), NPC_TALK_ARC);
    }
}

TEST("NpcTalk: An NPC says the same thing every time while the main-story-index holds")
{
    static EWRAM_DATA u8 sFirst[300];
    static EWRAM_DATA u8 sSecond[300];
    u32 localId;

    for (localId = 0; localId < 12; localId++)
    {
        u32 kind = WriteGenericNpcTalk(sFirst, RUSTBORO, BUILDING_MART, localId, 5);

        EXPECT_EQ(WriteGenericNpcTalk(sSecond, RUSTBORO, BUILDING_MART, localId, 5), kind);
        EXPECT_EQ(StringCompare(sFirst, sSecond), 0);
    }
}

TEST("NpcTalk: Over the course of the arc a non-hinting NPC says more than one thing")
{
    static EWRAM_DATA u8 sFirst[300];
    static EWRAM_DATA u8 sText[300];
    u32 localId, index, different = FALSE;

    for (localId = 0; IsArcHinterNpc(LILYCOVE, localId); localId++)
        ;
    WriteGenericNpcTalk(sFirst, LILYCOVE, 0, localId, 0);
    for (index = 1; index < MAIN_STORY_INDEX_POSTGAME; index++)
    {
        WriteGenericNpcTalk(sText, LILYCOVE, 0, localId, index);
        different |= StringCompare(sFirst, sText) != 0;
    }
    EXPECT(different);
}

TEST("NpcTalk: Different NPCs of a city say different things")
{
    static EWRAM_DATA u8 sFirst[300];
    static EWRAM_DATA u8 sText[300];
    u32 localId, different = FALSE;

    WriteGenericNpcTalk(sFirst, LILYCOVE, 0, 1, 20);
    for (localId = 2; localId < 40; localId++)
    {
        WriteGenericNpcTalk(sText, LILYCOVE, 0, localId, 20);
        different |= StringCompare(sFirst, sText) != 0;
    }
    EXPECT(different);
}

TEST("NpcTalk: A gym NPC names the leader's type and gives that type's tip")
{
    static EWRAM_DATA u8 sText[300];
    u32 localId, found = 0;
    enum Type type = GetCountyGymType(RUSTBORO);

    for (localId = 0; localId < 40; localId++)
    {
        if (IsArcHinterNpc(RUSTBORO, localId) || WriteGenericNpcTalk(sText, RUSTBORO, BUILDING_GYM, localId, 0) != NPC_TALK_GYM)
            continue;
        found++;
        EXPECT(ContainsString(sText, gTypesInfo[type].name));
    }
    EXPECT_GE(found, 1);
}

TEST("NpcTalk: Once the arc has passed a county's beats, some of its NPCs thank the player")
{
    static EWRAM_DATA u8 sText[300];
    u32 county, localId, thanks;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        u32 index, host;
        bool32 hosts = FALSE;

        if (!DoesCountyHaveCity(county))
            continue;
        for (index = 0; index < NUM_MAIN_INDICES; index++)
            hosts |= GetMainBeatHost(index, &host) && host == county;
        thanks = 0;
        for (localId = 0; localId < 16; localId++)
            thanks += WriteGenericNpcTalk(sText, county, 0, localId, MAIN_STORY_INDEX_POSTGAME) == NPC_TALK_THANKS;
        EXPECT_EQ(thanks != 0, hosts);
    }
}

TEST("NpcTalk: Nobody thanks the player before the arc has begun")
{
    static EWRAM_DATA u8 sText[300];
    u32 county, localId;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        if (!DoesCountyHaveCity(county))
            continue;
        for (localId = 0; localId < 20; localId++)
            EXPECT_NE(WriteGenericNpcTalk(sText, county, 0, localId, 0), NPC_TALK_THANKS);
    }
}

TEST("NpcTalk: The talk special writes the map's county talk for the NPC last talked to and succeeds")
{
    static EWRAM_DATA u8 sExpect[300];
    struct MapHeader saved = gMapHeader;

    gMapHeader.regionMapSectionId = MAPSEC_LILYCOVE_CITY;
    gSpecialVar_LastTalked = 7;
    gStringVar4[0] = EOS;
    RUN_OVERWORLD_SCRIPT(setvar VAR_0x8004, 0; special ShowGenericNpcTalk;);
    gMapHeader = saved;

    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT(gStringVar4[0] != EOS);
    WriteGenericNpcTalk(sExpect, LILYCOVE, 0, 7, GetMainStoryIndex());
    EXPECT_EQ(StringCompare(gStringVar4, sExpect), 0);
}

// `generic_npc_talk 0` is `setvar VAR_0x8004, 0` then `goto Common_EventScript_GenericNpcTalk`.
static void ExpectBoundToGenericTalk(const u8 *script)
{
    static const u8 sPrefix[] = {SCR_OP_SETVAR, 0x04, 0x80, 0x00, 0x00, SCR_OP_GOTO};
    u32 i, target = 0;

    for (i = 0; i < sizeof(sPrefix); i++)
        EXPECT_EQ(script[i], sPrefix[i]);
    for (i = 0; i < 4; i++)
        target |= script[sizeof(sPrefix) + i] << (8 * i);
    EXPECT_EQ(target, (u32)Common_EventScript_GenericNpcTalk);
}

TEST("NpcTalk: The fallbacks of Lilycove's and Rustboro's hinters are bound to the generic talk")
{
    ExpectBoundToGenericTalk(LilycoveCity_EventScript_Man3Generic);
    ExpectBoundToGenericTalk(LilycoveCity_EventScript_RichBoyGeneric);
    ExpectBoundToGenericTalk(RustboroCity_EventScript_WomanGeneric);
    ExpectBoundToGenericTalk(RustboroCity_EventScript_TwinGeneric);
}
