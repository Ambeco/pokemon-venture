#include "global.h"
#include "assertf.h"
#include "fieldmap.h"
#include "scene.h"
#include "constants/event_object_movement.h"

bool32 SceneTriggerFires(u32 kind, u32 radius, s16 playerX, s16 playerY, s16 doorX, s16 doorY)
{
    s16 frontX, frontY;
    s32 dx = playerX - doorX, dy = playerY - doorY;

    switch (kind)
    {
    case SCENE_TRIGGER_PROXIMITY:
        return (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy) <= (s32)radius;
    case SCENE_TRIGGER_ON_DOOR:
        return dx == 0 && dy == 0;
    case SCENE_TRIGGER_IN_FRONT_OF_DOOR:
        SceneResolveSide(doorX, doorY, DIR_SOUTH, SCENE_SIDE_FRONT, &frontX, &frontY);
        return playerX == frontX && playerY == frontY;
    default:
        assertf(FALSE, "scene trigger kind %d does not exist; did you mean one of enum SceneTriggerKind?", kind)
        {
            return FALSE;
        }
    }
    return FALSE;
}

bool32 SceneExitTriggerFires(enum Direction edge, u32 depth, s16 playerX, s16 playerY, s32 mapWidth, s32 mapHeight)
{
    switch (edge)
    {
    case DIR_NORTH:
        return playerY == (s32)depth;
    case DIR_SOUTH:
        return playerY == mapHeight - 1 - (s32)depth;
    case DIR_WEST:
        return playerX == (s32)depth;
    case DIR_EAST:
        return playerX == mapWidth - 1 - (s32)depth;
    default:
        assertf(FALSE, "exit trigger edge %d is not a cardinal direction; did you mean DIR_NORTH, DIR_SOUTH, DIR_WEST or DIR_EAST?", edge)
        {
            return FALSE;
        }
    }
    return FALSE;
}

// Fires only where the edge row (or column) is walkable straight ahead, so the trigger sits at the gap, not along the whole edge.
static bool32 SceneExitTriggerHolds(const struct SceneTrigger *trigger, s16 playerX, s16 playerY)
{
    s32 width = gMapHeader.mapLayout->width, height = gMapHeader.mapLayout->height;
    s16 edgeX = playerX, edgeY = playerY;

    if (!SceneExitTriggerFires(trigger->edge, trigger->radius, playerX - MAP_OFFSET, playerY - MAP_OFFSET, width, height))
        return FALSE;
    switch (trigger->edge)
    {
    case DIR_NORTH:
        edgeY = MAP_OFFSET;
        break;
    case DIR_SOUTH:
        edgeY = MAP_OFFSET + height - 1;
        break;
    case DIR_WEST:
        edgeX = MAP_OFFSET;
        break;
    default:
        edgeX = MAP_OFFSET + width - 1;
        break;
    }
    return MapGridGetCollisionAt(edgeX, edgeY) == 0;
}

const struct SceneTrigger *SceneFindTrigger(const struct SceneTrigger *table, u32 count, u32 mapGroup, u32 mapNum, s16 playerX, s16 playerY, bool32 mapJustLoaded)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        const struct SceneTrigger *trigger = &table[i];
        const struct MapEvents *events = gMapHeader.events;
        const struct WarpEvent *door;

        if (trigger->mapGroup != mapGroup || trigger->mapNum != mapNum || SceneGetStage(trigger->stageVar) != trigger->stageValue)
            continue;
        if (trigger->kind == SCENE_TRIGGER_ENTER_MAP)
        {
            if (mapJustLoaded)
                return trigger;
            continue;
        }
        if (trigger->kind == SCENE_TRIGGER_EXIT)
        {
            if (SceneExitTriggerHolds(trigger, playerX, playerY))
                return trigger;
            continue;
        }
        assertf(events != NULL && trigger->doorWarp < events->warpCount, "scene trigger for scene %d names door warp %d, but map %d.%d has %d warps; did you mean another warp index?", trigger->sceneId, trigger->doorWarp, mapGroup, mapNum, events == NULL ? 0 : events->warpCount)
        {
            continue;
        }
        door = &events->warps[trigger->doorWarp];
        if (SceneTriggerFires(trigger->kind, trigger->radius, playerX, playerY, door->x + MAP_OFFSET, door->y + MAP_OFFSET))
            return trigger;
    }
    return NULL;
}
