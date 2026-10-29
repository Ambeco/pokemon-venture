#ifndef GUARD_REGION_ROUTE_H
#define GUARD_REGION_ROUTE_H

/**
 * Inter-county routes: which two locations a pair of adjacent counties connects, and which region
 * tiles the route between them occupies.
 *
 * Both functions are pure functions of the unordered pair, so either county (or the field map
 * between them) computes the same answer without storing it or asking the other county's
 * generated content. They read only the two counties' location coordinates and the region seed.
 *
 * A route is a shortest monotone path: its first tile leaves the first location in the direction of
 * the neighbor, and its last tile enters the second location in that same direction, so routes
 * from one location to different neighbors never share a first tile. The single bend sits on a
 * column (row, for north-south pairs) chosen by hash from strictly between the two locations.
 * Exception: locations on facing border tiles one apart with a cross-axis offset have no tile
 * between them, so the route runs along the second location's own column and enters it sideways.
 *
 * Route tiles are numbered from the west (or north) end whichever county asks. They belong to no
 * county's route mask: this is the generator's answer, and the hardcoded Emerald route tiles in
 * region_tile.h are placeholders that do not follow it.
 */

#include "global.h"
#include "region_tile.h"

#define REGION_SALT_ROUTE_BEND 1

// Picks the closest pair of locations (city or special) between two adjacent counties; ties prefer
// cities, then lower location index. posA is in countyA, posB in countyB. FALSE if either county
// has no location.
bool32 GetCountyPairRouteEndpoints(u32 countyA, u32 countyB, struct RegionTilePos *posA, struct RegionTilePos *posB);

// Tiles strictly between two locations of adjacent counties; symmetric in its arguments.
u32 GetRouteTileCount(struct RegionTilePos a, struct RegionTilePos b);

// index-th tile of the route, counted from the west (or north) location; symmetric in a and b.
struct RegionTilePos GetRouteTile(struct RegionTilePos a, struct RegionTilePos b, u32 index);

#endif // GUARD_REGION_ROUTE_H
