// Step-3 unlock key of each block (Emerald.md, "Main-story-index table"): block x unlocks county x + 1.
// A gym key means its badge and HM. The Letter is not an item of its own; its get is Devon Goods recovered.

#define OPEN_UNLOCK                 {PLOT_UNLOCK_OPEN, 0}
#define BLOCKER_UNLOCK              {PLOT_UNLOCK_BLOCKER, 0}
#define KEY_UNLOCK(kind, ordinal)   {PLOT_UNLOCK_KEYS, 1, {{kind, ordinal}}}
#define KEYS_UNLOCK(k1, o1, k2, o2) {PLOT_UNLOCK_KEYS, 2, {{k1, o1}, {k2, o2}}}

static const struct PlotUnlock sEmeraldUnlocks[] =
{
    KEY_UNLOCK(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_STARTER),                  // 0 Littleroot -> Oldale
    KEY_UNLOCK(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_POKEDEX),                  // 1 -> Petalburg
    KEY_UNLOCK(PLOT_KIND_SPECIAL, PLOT_SPECIAL_WAILMER_PAIL),               // 2 -> Rustboro
    KEY_UNLOCK(PLOT_KIND_DEVON, PLOT_DEVON_RECOVERED),                      // 3 -> Dewford (the Letter)
    KEY_UNLOCK(PLOT_KIND_DEVON, PLOT_DEVON_LETTER_DELIVERED),               // 4 -> Slateport
    KEY_UNLOCK(PLOT_KIND_DEVON, PLOT_DEVON_GOODS_DELIVERED),                // 5 -> Cycling Road
    KEY_UNLOCK(PLOT_KIND_RIVAL, 3),                                         // 6 -> Mauville
    KEY_UNLOCK(PLOT_KIND_GYM, 3),                                           // 7 -> Verdanturf (Rock Smash + Dynamo Badge)
    BLOCKER_UNLOCK,                                                         // 8 -> Fallarbor
    BLOCKER_UNLOCK,                                                         // 9 -> Meteor Falls
    KEY_UNLOCK(PLOT_KIND_ARC, 3),                                           // 10 -> Lavaridge (Meteorite stolen)
    OPEN_UNLOCK,                                                            // 11 -> Fortree
    KEY_UNLOCK(PLOT_KIND_GYM, 5),                                           // 12 -> Weather Institute (Surf)
    KEY_UNLOCK(PLOT_KIND_ARC, 5),                                           // 13 -> Mt. Pyre (Shelly)
    KEY_UNLOCK(PLOT_KIND_ARC, 7),                                           // 14 -> Lilycove (Groudon awakened)
    KEY_UNLOCK(PLOT_KIND_ARC, 9),                                           // 15 -> Shoal Cave (blockade gone)
    BLOCKER_UNLOCK,                                                         // 16 -> Mossdeep
    KEYS_UNLOCK(PLOT_KIND_GYM, 7, PLOT_KIND_ARC, 11),                       // 17 -> Sootopolis (Dive + Kyogre awakened)
    KEY_UNLOCK(PLOT_KIND_ARC, 12),                                          // 18 -> Pacifidlog (Cave of Origin; Surf is held from block 12)
    KEY_UNLOCK(PLOT_KIND_GYM, 8),                                           // 19 -> Ever Grande (Waterfall + Rain Badge)
    KEY_UNLOCK(PLOT_KIND_SCRIPTED, PLOT_SCRIPTED_CHAMPION),                 // 20 -> reserved counties (Hall of Fame)
};
