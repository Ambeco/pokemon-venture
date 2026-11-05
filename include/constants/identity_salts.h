#ifndef GUARD_CONSTANTS_IDENTITY_SALTS_H
#define GUARD_CONSTANTS_IDENTITY_SALTS_H

// Every salt passed to IdentityHash/IdentityRng, so no two purposes share one. A salt that is used
// as `SALT + n` (n small) owns the next n values too; leave those unassigned. test/identity_salts.c
// checks that the table in that file lists every salt here and that the ranges do not overlap.
#define REGION_SALT_ROUTE_BEND       1  // + horizontal (1 or 2)
#define REGION_SALT_ROUTE_CROSSING   3  // + horizontal (3 or 4)
#define REGION_SALT_ROUTE_INTRA      5
#define REGION_SALT_PITSTOP          7
#define REGION_SALT_PITSTOP_DEAL     9
#define REGION_SALT_MCGUFFIN_STRIDE  11
#define REGION_SALT_MCGUFFIN_CROSS   13

#endif // GUARD_CONSTANTS_IDENTITY_SALTS_H
