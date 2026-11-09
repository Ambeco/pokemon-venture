#include "global.h"
#include "event_data.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("StoryMode: A player Pokémon survives a lethal hit on 1 HP while story mode is on")
{
    GIVEN {
        FLAG_SET(FLAG_VENTURE_STORY_MODE);
        PLAYER(SPECIES_CATERPIE) { Level(1); }
        OPPONENT(SPECIES_MEWTWO) { Level(100); Moves(MOVE_PSYCHIC); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_PSYCHIC); }
    } THEN {
        EXPECT_EQ(player->hp, 1);
    }
}

SINGLE_BATTLE_TEST("StoryMode: A player Pokémon still faints from a lethal hit while story mode is off")
{
    GIVEN {
        PLAYER(SPECIES_CATERPIE) { Level(1); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_MEWTWO) { Level(100); Moves(MOVE_PSYCHIC); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_PSYCHIC); SEND_OUT(player, 1); }
    } SCENE {
        MESSAGE("Caterpie fainted!");
    }
}

SINGLE_BATTLE_TEST("StoryMode: Story mode does not protect the opponent")
{
    GIVEN {
        FLAG_SET(FLAG_VENTURE_STORY_MODE);
        PLAYER(SPECIES_MEWTWO) { Level(100); Moves(MOVE_PSYCHIC); }
        OPPONENT(SPECIES_CATERPIE) { Level(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_PSYCHIC); }
    } SCENE {
        MESSAGE("The opposing Caterpie fainted!");
    }
}
