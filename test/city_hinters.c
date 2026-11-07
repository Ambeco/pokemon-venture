#include "global.h"
#include "test/test.h"
#include "city_hinters.h"
#include "region_city.h"
#include "region_county.h"
#include "subplot.h"

#define LILYCOVE COUNTY_ID(4, 0)

TEST("CityHinters: Lilycove holds one hinter per museum stage, close, bound to that stage")
{
    struct HinterPlacement out[MAX_COUNTY_HINTERS];
    u32 i, subplot = 0, count = PlanCountyHinters(LILYCOVE, out, ARRAY_COUNT(out));

    for (i = 0; i < count && i < ARRAY_COUNT(out); i++)
    {
        if (out[i].kind != HINTER_SUBPLOT)
            continue;
        EXPECT_EQ(out[i].slot, 0);
        EXPECT_EQ(out[i].beat, subplot);
        EXPECT_EQ(out[i].distance, SUBPLOT_CLOSE);
        EXPECT_EQ(out[i].targetCounty, LILYCOVE);
        subplot++;
    }
    EXPECT_EQ(subplot, 2);
}

TEST("CityHinters: Every county plans within capacity, with unique selectors and valid roles")
{
    u32 county;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        struct HinterPlacement out[MAX_COUNTY_HINTERS];
        u32 i, j, count;

        if (!DoesCountyHaveCity(county))
            continue;
        count = PlanCountyHinters(county, out, ARRAY_COUNT(out));
        ASSUME(count <= ARRAY_COUNT(out));
        for (i = 0; i < count; i++)
        {
            if (out[i].role != 0)
                EXPECT(DoesCountyCityHaveBuilding(county, out[i].role));
            for (j = i + 1; j < count; j++)
            {
                EXPECT(out[i].selector != out[j].selector);
            }
        }
    }
}

TEST("CityHinters: No building holds more hinters than its capacity")
{
    u32 county;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        struct HinterPlacement out[MAX_COUNTY_HINTERS];
        u32 i, j, count;

        if (!DoesCountyHaveCity(county))
            continue;
        count = PlanCountyHinters(county, out, ARRAY_COUNT(out));
        ASSUME(count <= ARRAY_COUNT(out));
        for (i = 0; i < count; i++)
        {
            u32 here = 0;

            for (j = 0; j < count; j++)
            {
                if (out[j].role == out[i].role)
                    here++;
            }
            EXPECT_LE(here, GetHinterRoleCapacity(out[i].role));
        }
    }
}

TEST("CityHinters: Main hinters point at the next beat; far unless that beat is in the same county")
{
    u32 county;
    u32 mainCount = 0;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        struct HinterPlacement out[MAX_COUNTY_HINTERS];
        u32 i, host, count;

        if (!DoesCountyHaveCity(county))
            continue;
        count = PlanCountyHinters(county, out, ARRAY_COUNT(out));
        ASSUME(count <= ARRAY_COUNT(out));
        for (i = 0; i < count; i++)
        {
            if (out[i].kind != HINTER_MAIN)
                continue;
            mainCount++;
            EXPECT(GetMainBeatHost(out[i].slot, &host));
            EXPECT_EQ(host, county);
            EXPECT_EQ(out[i].beat, GetNextMainBeat(out[i].slot));
            EXPECT(GetMainBeatHost(out[i].beat, &host));
            EXPECT_EQ(out[i].targetCounty, host);
            EXPECT_EQ(out[i].distance, host == county ? SUBPLOT_CLOSE : SUBPLOT_FAR);
        }
    }
    EXPECT(mainCount > 0);
}

TEST("CityHinters: The next beat skips empty lists and has no successor after the last")
{
    u32 index, host, last = HINTER_NO_BEAT;

    for (index = 0; index < NUM_COUNTIES * 4; index++)
    {
        if (!GetMainBeatHost(index, &host))
            continue;
        if (last != HINTER_NO_BEAT)
            EXPECT_EQ(GetNextMainBeat(last), index);
        last = index;
    }
    EXPECT(last != HINTER_NO_BEAT);
    EXPECT_EQ(GetNextMainBeat(last), HINTER_NO_BEAT);
}

TEST("CityHinters: Role capacities follow the placeholder budget")
{
    EXPECT_EQ(GetHinterRoleCapacity(0), 4);
    EXPECT_EQ(GetHinterRoleCapacity(BUILDING_GYM), 3);
    EXPECT_EQ(GetHinterRoleCapacity(BUILDING_SERVICE), 1);
    EXPECT_EQ(GetHinterRoleCapacity(BUILDING_POKEMON_CENTER), 2);
}
