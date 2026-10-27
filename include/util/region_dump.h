#ifndef GUARD_REGION_DUMP_H
#define GUARD_REGION_DUMP_H

/**
 * Text dump of the whole region for debugging: an ASCII map of the 28x15 region cells grouped by
 * county, then one line per county with its header fields and tile roles.
 *
 * Map legend: C city, S special location, @ city with a special location in it, # route, . empty;
 * a space separates county columns and a blank line separates county rows. County lines use ids
 * from COUNTY_ID and MAPSEC numbers for names. Lines are emitted one at a time (at most
 * REGION_DUMP_LINE_SIZE - 1 characters, no newline) so nothing large lives on the stack.
 */

#include "global.h"

#define REGION_DUMP_LINE_SIZE 160

typedef void (*RegionDumpLineFn)(const char *line);

void DumpRegion(RegionDumpLineFn emit);

// Dumps to the mGBA log (a no-op in NDEBUG builds).
void PrintRegionToLog(void);

#endif // GUARD_REGION_DUMP_H
