#include "global.h"
#include "assertf.h"
#include "event_object_movement.h"
#include "scene.h"
#include "constants/event_object_movement.h"

#define WINDOW_WIDTH (2 * SCENE_WINDOW_HALF_WIDTH + 1)
#define WINDOW_HEIGHT (2 * SCENE_WINDOW_HALF_HEIGHT + 1)
#define WINDOW_CELLS (WINDOW_WIDTH * WINDOW_HEIGHT)
#define CELL_UNSEEN 0

STATIC_ASSERT(WINDOW_CELLS < 256, SceneQueueIndexFitsU8);

static EWRAM_DATA u8 sSearchCameFrom[WINDOW_CELLS] = {0}; // the direction taken into each cell, CELL_UNSEEN if not reached
static EWRAM_DATA u8 sSearchQueue[WINDOW_CELLS] = {0};

void SceneResolveSide(s16 x, s16 y, enum Direction facing, u32 side, s16 *outX, s16 *outY)
{
    enum Direction dir = facing == DIR_NONE ? DIR_SOUTH : facing;

    *outX = x;
    *outY = y;
    switch (side)
    {
    case SCENE_SIDE_SELF:
        return;
    case SCENE_SIDE_FRONT:
        break;
    case SCENE_SIDE_BEHIND:
        dir = GetOppositeDirection(dir);
        break;
    case SCENE_SIDE_LEFT:
        dir = GetNinetyDegreeDirection(dir, FALSE);
        break;
    case SCENE_SIDE_RIGHT:
        dir = GetNinetyDegreeDirection(dir, TRUE);
        break;
    default:
        assertf(FALSE, "scene side %d does not exist; did you mean one of enum SceneSide?", side)
        {
            return;
        }
    }
    MoveCoords(dir, outX, outY);
}

enum Direction SceneDirectionToward(s16 fromX, s16 fromY, s16 toX, s16 toY)
{
    s32 dx = toX - fromX, dy = toY - fromY;
    s32 ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;

    if (ax == 0 && ay == 0)
        return DIR_NONE;
    if (ay >= ax)
        return dy > 0 ? DIR_SOUTH : DIR_NORTH;
    return dx > 0 ? DIR_EAST : DIR_WEST;
}

u32 SceneRingLength(const struct SceneRing *ring)
{
    return 2 * (ring->width + ring->height) - 4;
}

void SceneRingTile(const struct SceneRing *ring, u32 index, s16 *outX, s16 *outY)
{
    u32 w = ring->width, h = ring->height;

    index %= SceneRingLength(ring);
    if (index < w - 1)
    {
        *outX = ring->x + index;
        *outY = ring->y;
    }
    else if (index < w + h - 2)
    {
        *outX = ring->x + w - 1;
        *outY = ring->y + (index - (w - 1));
    }
    else if (index < 2 * w + h - 3)
    {
        *outX = ring->x + w - 1 - (index - (w + h - 2));
        *outY = ring->y + h - 1;
    }
    else
    {
        *outX = ring->x;
        *outY = ring->y + h - 1 - (index - (2 * w + h - 3));
    }
}

u32 SceneRingIndexOf(const struct SceneRing *ring, s16 x, s16 y)
{
    u32 i, length = SceneRingLength(ring);

    for (i = 0; i < length; i++)
    {
        s16 tileX, tileY;

        SceneRingTile(ring, i, &tileX, &tileY);
        if (tileX == x && tileY == y)
            return i;
    }
    return SCENE_RING_NONE;
}

void SceneRingDirs(const struct SceneRing *ring, u32 startIndex, u32 count, u8 *dirs)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        s16 fromX, fromY, toX, toY;

        SceneRingTile(ring, startIndex + i, &fromX, &fromY);
        SceneRingTile(ring, startIndex + i + 1, &toX, &toY);
        dirs[i] = SceneDirectionToward(fromX, fromY, toX, toY);
    }
}

static s32 AbsDiff(s32 a, s32 b)
{
    return a > b ? a - b : b - a;
}

bool32 SceneFindRing(s16 centerX, s16 centerY, u32 searchRadius, s16 avoidX, s16 avoidY, u32 avoidRadius, SceneTileScore score, void *context, struct SceneRing *out)
{
    static const u8 sizes[][2] = {{3, 3}, {4, 3}, {3, 4}};
    bool32 found = FALSE;
    s32 bestScore = 0, bestDistance = 0;
    s32 ox, oy;
    u32 sizeIndex;

    for (oy = centerY - (s32)searchRadius; oy <= centerY + (s32)searchRadius; oy++)
    {
        for (ox = centerX - (s32)searchRadius; ox <= centerX + (s32)searchRadius; ox++)
        {
            for (sizeIndex = 0; sizeIndex < ARRAY_COUNT(sizes); sizeIndex++)
            {
                struct SceneRing candidate = {ox, oy, sizes[sizeIndex][0], sizes[sizeIndex][1]};
                u32 i, length = SceneRingLength(&candidate);
                s32 total = 0, distance;
                bool32 usable = TRUE;

                for (i = 0; i < length && usable; i++)
                {
                    s16 x, y;
                    s32 tileScore, dx, dy;

                    SceneRingTile(&candidate, i, &x, &y);
                    dx = AbsDiff(x, avoidX);
                    dy = AbsDiff(y, avoidY);
                    tileScore = score(context, x, y);
                    if (tileScore < 0 || (dx > dy ? dx : dy) <= (s32)avoidRadius)
                        usable = FALSE;
                    total += tileScore;
                }
                if (!usable)
                    continue;
                // Twice the distance from the center to the ring's center, in tiles.
                distance = AbsDiff(2 * ox + candidate.width - 1, 2 * centerX) + AbsDiff(2 * oy + candidate.height - 1, 2 * centerY);
                if (!found || total > bestScore || (total == bestScore && distance < bestDistance))
                {
                    found = TRUE;
                    bestScore = total;
                    bestDistance = distance;
                    *out = candidate;
                }
            }
        }
    }
    return found;
}

u32 SceneFindPath(s16 startX, s16 startY, s16 goalX, s16 goalY, SceneStepCheck canStep, void *context, u8 *dirs)
{
    static const enum Direction order[] = {DIR_NORTH, DIR_SOUTH, DIR_WEST, DIR_EAST};
    s16 originX = startX - SCENE_WINDOW_HALF_WIDTH, originY = startY - SCENE_WINDOW_HALF_HEIGHT;
    s32 goalCol = goalX - originX, goalRow = goalY - originY;
    u32 head = 0, tail = 0, i, length;
    u8 goalCell, cell;

    if (goalCol < 0 || goalCol >= WINDOW_WIDTH || goalRow < 0 || goalRow >= WINDOW_HEIGHT)
        return SCENE_PATH_NONE;
    if (startX == goalX && startY == goalY)
        return 0;

    for (i = 0; i < WINDOW_CELLS; i++)
        sSearchCameFrom[i] = CELL_UNSEEN;
    goalCell = goalRow * WINDOW_WIDTH + goalCol;
    cell = SCENE_WINDOW_HALF_HEIGHT * WINDOW_WIDTH + SCENE_WINDOW_HALF_WIDTH;
    sSearchCameFrom[cell] = DIR_NONE + 1; // the start counts as reached; any non-zero mark will do
    sSearchQueue[tail++] = cell;

    while (head < tail && sSearchCameFrom[goalCell] == CELL_UNSEEN)
    {
        u8 current = sSearchQueue[head++];
        s16 x = originX + current % WINDOW_WIDTH, y = originY + current / WINDOW_WIDTH;

        for (i = 0; i < ARRAY_COUNT(order); i++)
        {
            s16 nx = x, ny = y;
            s32 col, row;
            u8 next;

            MoveCoords(order[i], &nx, &ny);
            col = nx - originX;
            row = ny - originY;
            if (col < 0 || col >= WINDOW_WIDTH || row < 0 || row >= WINDOW_HEIGHT)
                continue;
            next = row * WINDOW_WIDTH + col;
            if (sSearchCameFrom[next] != CELL_UNSEEN || !canStep(context, x, y, order[i]))
                continue;
            sSearchCameFrom[next] = order[i];
            sSearchQueue[tail++] = next;
        }
    }

    if (sSearchCameFrom[goalCell] == CELL_UNSEEN)
        return SCENE_PATH_NONE;

    for (length = 0, cell = goalCell; cell != SCENE_WINDOW_HALF_HEIGHT * WINDOW_WIDTH + SCENE_WINDOW_HALF_WIDTH; length++)
    {
        enum Direction taken = sSearchCameFrom[cell];
        s16 bx = 0, by = 0;

        if (length >= SCENE_MAX_PATH)
            return SCENE_PATH_NONE;
        dirs[length] = taken; // reversed below
        MoveCoords(GetOppositeDirection(taken), &bx, &by);
        cell = (cell / WINDOW_WIDTH + by) * WINDOW_WIDTH + (cell % WINDOW_WIDTH + bx);
    }
    for (i = 0; i < length / 2; i++)
    {
        u8 tmp = dirs[i];
        dirs[i] = dirs[length - 1 - i];
        dirs[length - 1 - i] = tmp;
    }
    return length;
}

u8 *SceneBuildWalkScript(u8 *out, const u8 *dirs, u32 length, bool32 slide)
{
    u32 i;

    assertf(length <= SCENE_MAX_PATH, "walk of %d steps is longer than SCENE_MAX_PATH (%d)", length, SCENE_MAX_PATH)
    {
        length = SCENE_MAX_PATH;
    }
    if (slide)
        *out++ = MOVEMENT_ACTION_LOCK_FACING_DIRECTION;
    for (i = 0; i < length; i++)
        *out++ = GetWalkNormalMovementAction(dirs[i]);
    if (slide)
        *out++ = MOVEMENT_ACTION_UNLOCK_FACING_DIRECTION;
    *out = MOVEMENT_ACTION_STEP_END;
    return out;
}
