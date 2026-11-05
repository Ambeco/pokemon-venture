// Planner inputs for Emerald (Emerald.md, "Planner inputs"). Gym slots are in emerald_cities.h.
// Hosts are county-story-indices: the block itself unless step 1 (a callback).

// Surf-lock block n.
#define EMERALD_SURF_BLOCK 11

#define SLOT(kind, ordinal, block, step, host) {kind, ordinal, block, step, host}

static const struct PlotSlot sEmeraldPlotSlots[] =
{
    // Rival beats
    SLOT(PLOT_KIND_RIVAL, 1, 1, 0, 1),
    SLOT(PLOT_KIND_RIVAL, 2, 3, 2, 3),
    SLOT(PLOT_KIND_RIVAL, 3, 6, 0, 6),
    SLOT(PLOT_KIND_RIVAL, 4, 12, 2, 12),
    SLOT(PLOT_KIND_RIVAL, 5, 15, 0, 15),
    SLOT(PLOT_KIND_WALLY, 1, 7, 0, 7),
    SLOT(PLOT_KIND_WALLY, 2, 20, 1, 18),  // Victory Road
    // Arc beats, in arc order
    SLOT(PLOT_KIND_ARC, 0, 2, 0, 2),    // Aqua: Petalburg Woods grunt
    SLOT(PLOT_KIND_ARC, 1, 3, 0, 3),    // Aqua: Stop Thief
    SLOT(PLOT_KIND_ARC, 2, 5, 2, 5),    // Aqua: Museum
    SLOT(PLOT_KIND_ARC, 3, 10, 0, 10),  // Magma: Meteor Falls
    SLOT(PLOT_KIND_ARC, 4, 11, 0, 11),  // Magma: Mt. Chimney
    SLOT(PLOT_KIND_ARC, 5, 12, 0, 12),  // Aqua: Shelly
    SLOT(PLOT_KIND_ARC, 6, 14, 0, 14),  // Aqua: Red Orb
    SLOT(PLOT_KIND_ARC, 7, 14, 1, 11),  // Magma: Hideout (Lavaridge callback)
    SLOT(PLOT_KIND_ARC, 8, 15, 1, 5),   // Aqua: submarine (Slateport callback)
    SLOT(PLOT_KIND_ARC, 9, 15, 2, 15),  // Aqua: Hideout
    SLOT(PLOT_KIND_ARC, 10, 17, 0, 17), // Magma: Space Center
    SLOT(PLOT_KIND_ARC, 11, 17, 2, 17), // Aqua: Seafloor Cavern
    SLOT(PLOT_KIND_ARC, 12, 18, 0, 18), // Cave of Origin
    SLOT(PLOT_KIND_ARC, 13, 19, 0, 19), // Sky Pillar
    SLOT(PLOT_KIND_ARC, 14, 19, 1, 18), // Rayquaza ends the fight (Sootopolis callback)
    // Devon Goods
    SLOT(PLOT_KIND_DEVON, PLOT_DEVON_RECOVERED, 3, 2, 3),
    SLOT(PLOT_KIND_DEVON, PLOT_DEVON_LETTER_DELIVERED, 4, 2, 4),
    SLOT(PLOT_KIND_DEVON, PLOT_DEVON_GOODS_DELIVERED, 5, 2, 5),
    // Special-item McGuffins (fixed)
    SLOT(PLOT_KIND_SPECIAL, PLOT_SPECIAL_WAILMER_PAIL, 2, 2, 2),
    SLOT(PLOT_KIND_SPECIAL, PLOT_SPECIAL_GO_GOGGLES, 11, 2, 11),
    SLOT(PLOT_KIND_SPECIAL, PLOT_SPECIAL_DEVON_SCOPE, 13, 0, 13),
    // Scripted gets
    SLOT(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_STARTER, 0, 0, 0),
    SLOT(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_POKEDEX, 1, 1, 0),
    SLOT(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_POKE_BALLS, 1, 1, 0),
    SLOT(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_RUNNING_SHOES, 1, 1, 0),
    SLOT(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_WALLY_RALTS, 2, 0, 2),
    SLOT(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_SHIPYARD, 5, 0, 5),
    SLOT(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_ELITE_FOUR, 20, 2, 20),
    SLOT(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_CHAMPION, 20, 2, 20),
    // Utility items
    SLOT(PLOT_KIND_UTILITY, PLOT_UTILITY_POKENAV, 3, 2, 3),
    SLOT(PLOT_KIND_UTILITY, PLOT_UTILITY_OLD_ROD, 4, 0, 4),
    SLOT(PLOT_KIND_UTILITY, PLOT_UTILITY_GOOD_ROD, 12, 2, 12),
    SLOT(PLOT_KIND_UTILITY, PLOT_UTILITY_ITEMFINDER, 6, 0, 6),
    SLOT(PLOT_KIND_UTILITY, PLOT_UTILITY_BICYCLE, 7, 2, 7),
    SLOT(PLOT_KIND_UTILITY, PLOT_UTILITY_COIN_CASE, 7, 2, 7),
    SLOT(PLOT_KIND_UTILITY, PLOT_UTILITY_POKEBLOCK_CASE, 15, 0, 15),
    SLOT(PLOT_KIND_UTILITY, PLOT_UTILITY_SUPER_ROD, 17, 0, 17),
};

// HM handed out by each gym, indexed by gym ordinal - 1.
static const u8 sEmeraldGymHm[NUM_GYM_ORDINALS] =
{
    PLOT_HM_CUT, PLOT_HM_FLASH, PLOT_HM_ROCK_SMASH, PLOT_HM_STRENGTH, PLOT_HM_SURF,
    PLOT_HM_FLY, PLOT_HM_DIVE, PLOT_HM_WATERFALL, PLOT_HM_NONE,
};

// HMs not given at their gym's slot (exception 3).
static const struct PlotSlot sEmeraldDisplacedHms[] =
{
    {PLOT_KIND_GYM, 4, 8, 0, 8, PLOT_HM_STRENGTH, TRUE},
};

// Block of each callback; PLOT_NO_CALLBACK elsewhere.
struct PlotCallback
{
    u8 block;
    u8 host;
};

static const struct PlotCallback sEmeraldPlotCallbacks[] =
{
    {1, 0},
    {11, 2},
    {14, 11},
    {15, 5},
    {19, 18},
    {20, 18},
};

// Authored nominal block of each arc beat; the randomized range is this +-2, clipped.
static const u8 sEmeraldArcNominalBlock[NUM_PLOT_ARC] =
{
    2, 3, 5, 10, 11, 12, 14, 14, 15, 15, 17, 17, 18, 19, 19,
};

static const u8 sEmeraldDevonBlock[NUM_PLOT_DEVON] = {3, 4, 5};
static const u8 sEmeraldSpecialBlock[NUM_PLOT_SPECIAL] = {2, 11, 13};
