#ifndef GUARD_REGION_ROUTE_H
#define GUARD_REGION_ROUTE_H

/**
 * Inter-county routes: which two locations a pair of adjacent counties connects, where the route
 * crosses their shared border, and the half of the route that lies inside one county.
 *
 * The endpoints and the border crossing are pure functions of the unordered county pair, so both
 * counties agree on them without storing anything or reading each other's generated content. The
 * crossing is a single row (column, for north-south pairs) taken from one of the two locations, by
 * hash. The county owning that location runs a straight route to the border along its row. The
 * other county joins its location to the crossing tile on its own border column with an L or Z
 * shape (the bend column is a hash). Each half is therefore decided by one county alone, which
 * knows all its other routes, so it is free to bend a Z around them; two counties never have to
 * agree on anything past the crossing row.
 *
 * Border rows and columns are the outermost of a county. Routes may cross them but generated
 * locations never sit on them. The hardcoded Emerald table does not obey this and is left as it is.
 *
 * Tiles are listed from the location toward the border. The county planner that bends routes
 * around each other does not exist yet, so halves use the default shape and may overlap.
 */

#include "global.h"
#include "region_tile.h"

#define REGION_SALT_ROUTE_BEND     1
#define REGION_SALT_ROUTE_CROSSING 3

// Picks the closest pair of locations (city or special) between two adjacent counties; ties prefer
// cities, then lower location index. posA is in countyA, posB in countyB. FALSE if either county
// has no location.
bool32 GetCountyPairRouteEndpoints(u32 countyA, u32 countyB, struct RegionTilePos *posA, struct RegionTilePos *posB);

// Row (or column, for north-south pairs) where the route crosses the border, and the county whose
// location it comes from. Symmetric in its county arguments. FALSE if either county has no location.
bool32 GetRouteBorderCrossing(u32 countyA, u32 countyB, u32 *crossing, u32 *owner);

// Tiles of the route inside `county` toward `neighbor`, excluding the location itself and ending at
// the border column (row) tile of the crossing. 0 if either county has no location.
u32 GetCountyRouteHalfTileCount(u32 county, u32 neighbor);

struct RegionTilePos GetCountyRouteHalfTile(u32 county, u32 neighbor, u32 index);

#endif // GUARD_REGION_ROUTE_H
