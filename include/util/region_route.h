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
 * Tiles are listed from the location toward the border. Each county plans all its halves together:
 * every half that can bend (a non-owner half that has to turn) tries its hashed bend first, then
 * bends toward the border (hugging the county edge, where no other route runs), then toward its
 * location; a backtracking search finds a set in which no two halves cross and none passes through
 * another location, whenever one exists. Halves may share a run of tiles (a location on the border
 * column can only go across, so its halves overlap there); they may not cross. When no crossing-free
 * set exists (halves with no choice already cross, as in Emerald's table) every half keeps its
 * hashed bend. Bends stay within the
 * location-to-border span, so halves never detour. The plan is recomputed per county and cached for
 * the last county asked about. Halves exist only toward neighbors DoCountiesHaveRoute joins. The
 * plan also holds the intra-county routes, tile ownership and the lock positions (see the accessors).
 */

#include "global.h"
#include "region_tile.h"

// Route ids: a route to a neighbor county is that county's id; an intra-county route is named by
// the index of its special location.
#define ROUTE_ID_TO_SPECIAL(index) (NUM_COUNTIES + (index))
#define ROUTE_ID_NONE              0xFF

// TRUE iff the route rules (design.md, "Route generation") join two adjacent counties: either one
// routes to the other because it is the capital or has an adjacent story index, or because it is
// the other's highest-indexed neighbor when nothing else qualifies. Symmetric. This says nothing
// about locations; a county without any yields empty halves.
bool32 DoCountiesHaveRoute(u32 countyA, u32 countyB);

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

// Intra-county route from the county's index-th special location to its city, excluding both
// ends. 0 if the county has no city or the location is inside the city. Planned with the halves,
// so its bends avoid them when possible; it may share a run with a half but is fitted after them.
u32 GetCountyIntraRouteTileCount(u32 county, u32 specialIndex);

struct RegionTilePos GetCountyIntraRouteTile(u32 county, u32 specialIndex, u32 index);

// Route id owning a plain route tile, or ROUTE_ID_NONE for a tile that is not on a route or holds a
// location. Where routes share a run of road, the run belongs to the one that opens first (the
// later of its two counties' story indices; an intra-county route opens with its county), ties to
// the lower route id. Route-story-indices and locks read the owner, so every tile has exactly one.
u32 GetRegionTileRouteId(u32 x, u32 y);

// Tile of the lock on the inter-county route between two counties, and the county whose location it
// guards (the one with the higher story index; the lock sits on the earlier county's half). The
// lock is on the first tile after the last tile that half shares with another of the county's
// routes, so the road cannot be joined after it; if the half is empty or all shared, it is the
// later half's border tile. FALSE if the counties have no route or the route has no tile. Only the
// position is decided here; which McGuffin opens it comes from the planner.
bool32 GetRouteLock(u32 countyA, u32 countyB, struct RegionTilePos *pos, u32 *gatedCounty);

// Same for an intra-county route. The earlier end is the special location when the main route into
// the county arrives there, else the city; *gatesCity is TRUE when the special location is earlier.
bool32 GetCountyIntraRouteLock(u32 county, u32 specialIndex, struct RegionTilePos *pos, bool32 *gatesCity);

#endif // GUARD_REGION_ROUTE_H
