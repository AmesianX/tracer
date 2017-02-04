/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    StringTable2Private.h

Abstract:

    This is the private header file for the StringTable2 component.  It defines
    function typedefs and function declarations for all major (i.e. not local
    to the module) functions available for use by individual modules within
    this component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////
// Function typedefs and inline functions for internal modules.
////////////////////////////////////////////////////////////////////////////////

CREATE_STRING_TABLE2 CreateSingleStringTable2;


#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
