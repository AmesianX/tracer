/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    PerfectHashTableConstants.c

Abstract:

    This module defines constants used by the PerfectHashTable component.

--*/

#include "stdafx.h"

//
// Define the array of creation routines.
//

PCREATE_PERFECT_HASH_TABLE_IMPL CreationRoutines[] = {
    NULL,
    CreatePerfectHashTableImplChm01,
    NULL
};

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
