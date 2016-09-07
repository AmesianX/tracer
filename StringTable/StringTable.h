/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    StringTable.h

Abstract:

    This is the main header file for the StringTable component.  It defines
    structures and functions related to the implementation of the component.

    The main structures are the STRING_ARRAY structure, which is used by
    callers of this component to indicate the set of strings they'd like to
    add to the string table, and the STRING_TABLE structure, which is the main
    data structure used by this component.

    Functions are provided for creating, destroying and searching for whether
    or not there's a prefix string for a given search string present within a
    table.

    The design is optimized for relatively short strings (less than or equal to
    16 chars), and relatively few of them (less than or equal to 16).  These
    restrictive size constraints facilitate aggresive SIMD optimizations when
    searching for the strings within the table, with the goal to provide low
    latency with very little jitter.

--*/

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#ifdef _STRING_TABLE_INTERNAL_BUILD

//
// This is an internal build of the StringTable component.
//

#define STRING_TABLE_API __declspec(dllexport)
#define STRING_TABLE_DATA extern __declspec(dllexport)

#include "stdafx.h"

#else

//
// We're being included by an external component.
//

#define STRING_TABLE_API __declspec(dllimport)
#define STRING_TABLE_DATA extern __declspec(dllimport)

#include "../Rtl/Rtl.h"

#endif

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////

typedef _Struct_size_bytes_(Size) struct _STRING_ARRAY {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _STRING_ARRAY)) USHORT Size;

    //
    // Number of elements in the array.
    //

    USHORT NumberOfElements;

    //
    // Pad out to 8-bytes.
    //

    ULONG Padding;

    //
    // The string array.  Number of elements in the array is governed by the
    // NumberOfElements field above.
    //

    STRING Strings[ANYSIZE_ARRAY];

} STRING_ARRAY, *PSTRING_ARRAY, **PPSTRING_ARRAY;

//
// The STRING_TABLE struct is an optimized structure for testing whether a
// prefix entry for a string is in a table, with the expectation that the
// strings being compared will be relatively short (ideally <= 16 characters),
// and the table of string prefixes to compare to will be relatively small
// (ideally <= 16 strings).
//
// The overall goal is to be able to prefix match a string with the lowest
// possible (amortized) latency.  Fixed-size, memory-aligned character arrays,
// and SIMD instructions are used to try and achieve this.
//

typedef struct _STRING_TABLE {

    //
    // A bitmap indicating whether or not the given string slot is occupied.
    // The total number of strings occupied can be ascertained by from a popcnt
    // on this field.
    //

    USHORT OccupiedBitmap;

    //
    // A bitmap indicating whether or not the given string at the index given
    // by the bit in the bitmap is continued in the next string table block.
    //
    // N.B.: this implies a table invariant that a bit cannot be set in this
    //       bitmap unless the corresponding bit is set in the OccupiedBitmap.
    //

    USHORT ContinuationBitmap;

    //
    // Horizontal depth of the table; 0 translates to the first 16 characters,
    // 1 to the second 16 characters, etc.
    //

    BYTE HorizontalDepth;

    //
    // Vertical depth of the table; 0 translates to the first array of
    // 16 characters, 1 to the second array of 16 characters, etc.
    //

    BYTE VerticalDepth;

    //
    // The number of quadwords that need be added to this structure's base
    // address in order to derive the base address of the next structure that
    // has the same vertical depth as us, but an incremented horizontal depth.
    // If this value is 0, then this is the last horizontal block for the given
    // vertical depth.  The maximum offset is thus 32KB away.
    //

    BYTE NextHorizontalOffsetInQuadwords;

    //
    // As above, but for the next vertical offset.
    //

    BYTE NextVerticalOffsetInQuadwords;

    //
    // (8-bytes aligned.)
    //

    //
    // Each 4-bit nibble of this 64-byte long long field represents the length,
    // in characters, not including the trailing null, of the string in the
    // corresponding slot.  The lowest 4 bits map to slot 0, the next 4 bits
    // map to slot 1, and so on.
    //

    ULONGLONG StringSizes;

    //
    // (16-bytes aligned.)
    //

    //
    // The first two characters of each string is stored in this array.  We can
    // load it into an XMM register, fill a second XMM register with the search
    // string's first two characters also, and then do a `pand xmm1, xmm2` and
    // popcnt to see if there were any matches.  This allows us to terminate
    // the search early.
    //

    WIDE_CHARACTER FirstAndSecondCharacter[16];

    //
    // (48-bytes aligned; WIDE_CHARACTER is 16-bits.)
    //

    //
    // Pad out to 64-bytes so that our string buffers start on a quadword
    // boundary.
    //

    CHAR Padding[16];

    //
    // Our 16-element array of 16-character arrays.  Occupied slots are governed
    // by the OccupiedBitmap above.
    //

    CHAR StringArray[16][16];

} STRING_TABLE, *PSTRING_TABLE;

//
// Ensure the string array is aligned on a 64-byte boundary.
//

C_ASSERT(FIELD_OFFSET(STRING_TABLE, StringArray) == 64);

//
// This structure is used to communicate matches back to the caller.  (Not yet
// currently flushed out.)
//

typedef struct _STRING_MATCH {

    //
    // Index of the match.  -1 if no match.
    //

    LONG Index;

    //
    // Number of characters matched.
    //

    USHORT NumberOfMatchedCharacters;

    //
    // Pad out to 8-bytes.
    //

    USHORT Padding1;

    //
    // Pointer to the string that was matched.  The underlying buffer will
    // stay valid for as long as the STRING_TABLE struct persists.
    //

    PSTRING String;

} STRING_MATCH, *PSTRING_MATCH, *PPSTRING_MATCH;

////////////////////////////////////////////////////////////////////////////////
// Function Type Definitions
////////////////////////////////////////////////////////////////////////////////

typedef
_Check_return_
_Success_(return != 0)
PSTRING_TABLE
(CREATE_STRING_TABLE)(
    _In_ PALLOCATOR Allocator,
    _In_ PSTRING_ARRAY StringArray
    );
typedef CREATE_STRING_TABLE *PCREATE_STRING_TABLE;
STRING_TABLE_API CREATE_STRING_TABLE CreateStringTable;

typedef
VOID
(DESTROY_STRING_TABLE)(
    _Pre_notnull_ _Post_null_ PPSTRING_TABLE
    );
typedef DESTROY_STRING_TABLE *PDESTROY_STRING_TABLE;
STRING_TABLE_API DESTROY_STRING_TABLE DestroyStringTable;

typedef
BOOL
(IS_PREFIX_OF_STRING_IN_TABLE)(
    _In_ PSTRING_TABLE StringTable,
    _In_ PSTRING String,
    _Out_ PSTRING_MATCH StringMatch
    );
typedef IS_PREFIX_OF_STRING_IN_TABLE *PIS_PREFIX_OF_STRING_IN_TABLE;
STRING_TABLE_API IS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_C;
STRING_TABLE_API IS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_x64_SSE42;

#ifdef __cpp
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
