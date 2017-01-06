/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreIndex.h

Abstract:

    This module defines trace store enumerations.  Each top-level trace store
    (i.e. non-metadata trace stores) is allocated an ID, which is captured in
    the TRACE_STORE_ID structure.  TRACE_STORE_INDEX, on the other hand, is an
    enumeration that supplies the index offset of a trace store -- normal or
    metadata, within the TRACE_STORES struct.

    Thus, if one wanted to obtain a pointer to the TRACE_STORE structure for
    the event store, this would be done as follows:

        PTRACE_STORE EventStore;
        EventStore = TraceStores->Stores[TraceStoreEventIndex];

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _TRACE_STORE_ID {
    TraceStoreNullId                        =   0,
    TraceStoreEventId                       =   1,
    TraceStoreStringBufferId                =   2,
    TraceStoreFunctionTableId               =   3,
    TraceStoreFunctionTableEntryId          =   4,
    TraceStorePathTableId                   =   5,
    TraceStorePathTableEntryId              =   6,
    TraceStoreSessionId                     =   7,
    TraceStoreStringArrayId                 =   8,
    TraceStoreStringTableId                 =   9,
    TraceStoreEventTraitsExId               =  10,
    TraceStoreWsWatchInfoExId               =  11,
    TraceStoreWsWorkingSetExInfoId          =  12,
    TraceStoreCCallStackTableId             =  13,
    TraceStoreCCallStackTableEntryId        =  14,
    TraceStoreCModuleTableId                =  15,
    TraceStoreCModuleTableEntryId           =  16,
    TraceStorePythonCallStackTableId        =  17,
    TraceStorePythonCallStackTableEntryId   =  18,
    TraceStorePythonModuleTableId           =  19,
    TraceStorePythonModuleTableEntryId      =  20,
    TraceStoreLineTableId                   =  21,
    TraceStoreLineTableEntryId              =  22,
    TraceStoreLineStringBufferId            =  23,
    TraceStoreCallStackId                   =  24,
    TraceStorePerformanceId                 =  25,
    TraceStorePerformanceDeltaId            =  26,
    TraceStoreInvalidId                     =  27
} TRACE_STORE_ID, *PTRACE_STORE_ID;

#define MAX_TRACE_STORE_IDS TraceStoreInvalidId-1

#define TRACE_STORE_BITMAP_SIZE_IN_QUADWORDS 1
C_ASSERT(MAX_TRACE_STORE_IDS <= (TRACE_STORE_BITMAP_SIZE_IN_QUADWORDS * 64));

FORCEINLINE
TRACE_STORE_ID
ArrayIndexToTraceStoreId(
    _In_ USHORT Index
    )
{
    return (TRACE_STORE_ID)(Index + 1);
}

FORCEINLINE
USHORT
TraceStoreIdToArrayIndex(
    _In_ TRACE_STORE_ID TraceStoreId
    )
{
    return (USHORT)(TraceStoreId - 1);
}

typedef enum _TRACE_STORE_METADATA_ID {
    TraceStoreMetadataNullId = 0,
    TraceStoreMetadataMetadataInfoId = 1,
    TraceStoreMetadataAllocationId,
    TraceStoreMetadataRelocationId,
    TraceStoreMetadataAddressId,
    TraceStoreMetadataAddressRangeId,
    TraceStoreMetadataAllocationTimestampId,
    TraceStoreMetadataAllocationTimestampDeltaId,
    TraceStoreMetadataInfoId,
    TraceStoreMetadataInvalidId
} TRACE_STORE_METADATA_ID, *PTRACE_STORE_METADATA_ID;

#define MAX_TRACE_STORE_METADATA_IDS TraceStoreMetadataInvalidId

FORCEINLINE
TRACE_STORE_METADATA_ID
ArrayIndexToTraceStoreMetadataId(
    _In_ USHORT Index
    )
{
    return (TRACE_STORE_METADATA_ID)(Index + 1);
}

FORCEINLINE
USHORT
TraceStoreMetadataIdToArrayIndex(
    _In_ TRACE_STORE_METADATA_ID TraceStoreMetadataId
    )
{
    return (USHORT)(TraceStoreMetadataId - 1);
}

//
// N.B. To add a new index for each store to the following enum in vim, mark
//      the enum boundaries with ma and mb, then run the command:
//
//          :'a,'b s/TraceStore\(.\+\)AllocationIndex,$/TraceStore\1AllocationIndex,\r    TraceStore\1RelocationIndex,/
//
//      Replacing 'RelocationIndex' with the name of the new index you want
//      to add.
//

typedef enum _TRACE_STORE_INDEX {
    TraceStoreEventIndex = 0,
    TraceStoreEventMetadataInfoIndex,
    TraceStoreEventAllocationIndex,
    TraceStoreEventRelocationIndex,
    TraceStoreEventAddressIndex,
    TraceStoreEventAddressRangeIndex,
    TraceStoreEventAllocationTimestampIndex,
    TraceStoreEventAllocationTimestampDeltaIndex,
    TraceStoreEventInfoIndex,
    TraceStoreStringBufferIndex,
    TraceStoreStringBufferMetadataInfoIndex,
    TraceStoreStringBufferAllocationIndex,
    TraceStoreStringBufferRelocationIndex,
    TraceStoreStringBufferAddressIndex,
    TraceStoreStringBufferAddressRangeIndex,
    TraceStoreStringBufferAllocationTimestampIndex,
    TraceStoreStringBufferAllocationTimestampDeltaIndex,
    TraceStoreStringBufferInfoIndex,
    TraceStoreFunctionTableIndex,
    TraceStoreFunctionTableMetadataInfoIndex,
    TraceStoreFunctionTableAllocationIndex,
    TraceStoreFunctionTableRelocationIndex,
    TraceStoreFunctionTableAddressIndex,
    TraceStoreFunctionTableAddressRangeIndex,
    TraceStoreFunctionTableAllocationTimestampIndex,
    TraceStoreFunctionTableAllocationTimestampDeltaIndex,
    TraceStoreFunctionTableInfoIndex,
    TraceStoreFunctionTableEntryIndex,
    TraceStoreFunctionTableEntryMetadataInfoIndex,
    TraceStoreFunctionTableEntryAllocationIndex,
    TraceStoreFunctionTableEntryRelocationIndex,
    TraceStoreFunctionTableEntryAddressIndex,
    TraceStoreFunctionTableEntryAddressRangeIndex,
    TraceStoreFunctionTableEntryAllocationTimestampIndex,
    TraceStoreFunctionTableEntryAllocationTimestampDeltaIndex,
    TraceStoreFunctionTableEntryInfoIndex,
    TraceStorePathTableIndex,
    TraceStorePathTableMetadataInfoIndex,
    TraceStorePathTableAllocationIndex,
    TraceStorePathTableRelocationIndex,
    TraceStorePathTableAddressIndex,
    TraceStorePathTableAddressRangeIndex,
    TraceStorePathTableAllocationTimestampIndex,
    TraceStorePathTableAllocationTimestampDeltaIndex,
    TraceStorePathTableInfoIndex,
    TraceStorePathTableEntryIndex,
    TraceStorePathTableEntryMetadataInfoIndex,
    TraceStorePathTableEntryAllocationIndex,
    TraceStorePathTableEntryRelocationIndex,
    TraceStorePathTableEntryAddressIndex,
    TraceStorePathTableEntryAddressRangeIndex,
    TraceStorePathTableEntryAllocationTimestampIndex,
    TraceStorePathTableEntryAllocationTimestampDeltaIndex,
    TraceStorePathTableEntryInfoIndex,
    TraceStoreSessionIndex,
    TraceStoreSessionMetadataInfoIndex,
    TraceStoreSessionAllocationIndex,
    TraceStoreSessionRelocationIndex,
    TraceStoreSessionAddressIndex,
    TraceStoreSessionAddressRangeIndex,
    TraceStoreSessionAllocationTimestampIndex,
    TraceStoreSessionAllocationTimestampDeltaIndex,
    TraceStoreSessionInfoIndex,
    TraceStoreStringArrayIndex,
    TraceStoreStringArrayMetadataInfoIndex,
    TraceStoreStringArrayAllocationIndex,
    TraceStoreStringArrayRelocationIndex,
    TraceStoreStringArrayAddressIndex,
    TraceStoreStringArrayAddressRangeIndex,
    TraceStoreStringArrayAllocationTimestampIndex,
    TraceStoreStringArrayAllocationTimestampDeltaIndex,
    TraceStoreStringArrayInfoIndex,
    TraceStoreStringTableIndex,
    TraceStoreStringTableMetadataInfoIndex,
    TraceStoreStringTableAllocationIndex,
    TraceStoreStringTableRelocationIndex,
    TraceStoreStringTableAddressIndex,
    TraceStoreStringTableAddressRangeIndex,
    TraceStoreStringTableAllocationTimestampIndex,
    TraceStoreStringTableAllocationTimestampDeltaIndex,
    TraceStoreStringTableInfoIndex,
    TraceStoreEventTraitsExIndex,
    TraceStoreEventTraitsExMetadataInfoIndex,
    TraceStoreEventTraitsExAllocationIndex,
    TraceStoreEventTraitsExRelocationIndex,
    TraceStoreEventTraitsExAddressIndex,
    TraceStoreEventTraitsExAddressRangeIndex,
    TraceStoreEventTraitsExAllocationTimestampIndex,
    TraceStoreEventTraitsExAllocationTimestampDeltaIndex,
    TraceStoreEventTraitsExInfoIndex,
    TraceStoreWsWatchInfoExIndex,
    TraceStoreWsWatchInfoExMetadataInfoIndex,
    TraceStoreWsWatchInfoExAllocationIndex,
    TraceStoreWsWatchInfoExRelocationIndex,
    TraceStoreWsWatchInfoExAddressIndex,
    TraceStoreWsWatchInfoExAddressRangeIndex,
    TraceStoreWsWatchInfoExAllocationTimestampIndex,
    TraceStoreWsWatchInfoExAllocationTimestampDeltaIndex,
    TraceStoreWsWatchInfoExInfoIndex,
    TraceStoreWorkingSetExInfoIndex,
    TraceStoreWorkingSetExInfoMetadataInfoIndex,
    TraceStoreWorkingSetExInfoAllocationIndex,
    TraceStoreWorkingSetExInfoRelocationIndex,
    TraceStoreWorkingSetExInfoAddressIndex,
    TraceStoreWorkingSetExInfoAddressRangeIndex,
    TraceStoreWorkingSetExInfoAllocationTimestampIndex,
    TraceStoreWorkingSetExInfoAllocationTimestampDeltaIndex,
    TraceStoreWorkingSetExInfoInfoIndex,
    TraceStoreCCallStackTableIndex,
    TraceStoreCCallStackTableMetadataInfoIndex,
    TraceStoreCCallStackTableAllocationIndex,
    TraceStoreCCallStackTableRelocationIndex,
    TraceStoreCCallStackTableAddressIndex,
    TraceStoreCCallStackTableAddressRangeIndex,
    TraceStoreCCallStackTableAllocationTimestampIndex,
    TraceStoreCCallStackTableAllocationTimestampDeltaIndex,
    TraceStoreCCallStackTableInfoIndex,
    TraceStoreCCallStackTableEntryIndex,
    TraceStoreCCallStackTableEntryMetadataInfoIndex,
    TraceStoreCCallStackTableEntryAllocationIndex,
    TraceStoreCCallStackTableEntryRelocationIndex,
    TraceStoreCCallStackTableEntryAddressIndex,
    TraceStoreCCallStackTableEntryAddressRangeIndex,
    TraceStoreCCallStackTableEntryAllocationTimestampIndex,
    TraceStoreCCallStackTableEntryAllocationTimestampDeltaIndex,
    TraceStoreCCallStackTableEntryInfoIndex,
    TraceStoreCModuleTableIndex,
    TraceStoreCModuleTableMetadataInfoIndex,
    TraceStoreCModuleTableAllocationIndex,
    TraceStoreCModuleTableRelocationIndex,
    TraceStoreCModuleTableAddressIndex,
    TraceStoreCModuleTableAddressRangeIndex,
    TraceStoreCModuleTableAllocationTimestampIndex,
    TraceStoreCModuleTableAllocationTimestampDeltaIndex,
    TraceStoreCModuleTableInfoIndex,
    TraceStoreCModuleTableEntryIndex,
    TraceStoreCModuleTableEntryMetadataInfoIndex,
    TraceStoreCModuleTableEntryAllocationIndex,
    TraceStoreCModuleTableEntryRelocationIndex,
    TraceStoreCModuleTableEntryAddressIndex,
    TraceStoreCModuleTableEntryAddressRangeIndex,
    TraceStoreCModuleTableEntryAllocationTimestampIndex,
    TraceStoreCModuleTableEntryAllocationTimestampDeltaIndex,
    TraceStoreCModuleTableEntryInfoIndex,
    TraceStorePythonCallStackTableIndex,
    TraceStorePythonCallStackTableMetadataInfoIndex,
    TraceStorePythonCallStackTableAllocationIndex,
    TraceStorePythonCallStackTableRelocationIndex,
    TraceStorePythonCallStackTableAddressIndex,
    TraceStorePythonCallStackTableAddressRangeIndex,
    TraceStorePythonCallStackTableAllocationTimestampIndex,
    TraceStorePythonCallStackTableAllocationTimestampDeltaIndex,
    TraceStorePythonCallStackTableInfoIndex,
    TraceStorePythonCallStackTableEntryIndex,
    TraceStorePythonCallStackTableEntryMetadataInfoIndex,
    TraceStorePythonCallStackTableEntryAllocationIndex,
    TraceStorePythonCallStackTableEntryRelocationIndex,
    TraceStorePythonCallStackTableEntryAddressIndex,
    TraceStorePythonCallStackTableEntryAddressRangeIndex,
    TraceStorePythonCallStackTableEntryAllocationTimestampIndex,
    TraceStorePythonCallStackTableEntryAllocationTimestampDeltaIndex,
    TraceStorePythonCallStackTableEntryInfoIndex,
    TraceStorePythonModuleTableIndex,
    TraceStorePythonModuleTableMetadataInfoIndex,
    TraceStorePythonModuleTableAllocationIndex,
    TraceStorePythonModuleTableRelocationIndex,
    TraceStorePythonModuleTableAddressIndex,
    TraceStorePythonModuleTableAddressRangeIndex,
    TraceStorePythonModuleTableAllocationTimestampIndex,
    TraceStorePythonModuleTableAllocationTimestampDeltaIndex,
    TraceStorePythonModuleTableInfoIndex,
    TraceStorePythonModuleTableEntryIndex,
    TraceStorePythonModuleTableEntryMetadataInfoIndex,
    TraceStorePythonModuleTableEntryAllocationIndex,
    TraceStorePythonModuleTableEntryRelocationIndex,
    TraceStorePythonModuleTableEntryAddressIndex,
    TraceStorePythonModuleTableEntryAddressRangeIndex,
    TraceStorePythonModuleTableEntryAllocationTimestampIndex,
    TraceStorePythonModuleTableEntryAllocationTimestampDeltaIndex,
    TraceStorePythonModuleTableEntryInfoIndex,
    TraceStoreLineTableIndex,
    TraceStoreLineTableMetadataInfoIndex,
    TraceStoreLineTableAllocationIndex,
    TraceStoreLineTableRelocationIndex,
    TraceStoreLineTableAddressIndex,
    TraceStoreLineTableAddressRangeIndex,
    TraceStoreLineTableAllocationTimestampIndex,
    TraceStoreLineTableAllocationTimestampDeltaIndex,
    TraceStoreLineTableInfoIndex,
    TraceStoreLineTableEntryIndex,
    TraceStoreLineTableEntryMetadataInfoIndex,
    TraceStoreLineTableEntryAllocationIndex,
    TraceStoreLineTableEntryRelocationIndex,
    TraceStoreLineTableEntryAddressIndex,
    TraceStoreLineTableEntryAddressRangeIndex,
    TraceStoreLineTableEntryAllocationTimestampIndex,
    TraceStoreLineTableEntryAllocationTimestampDeltaIndex,
    TraceStoreLineTableEntryInfoIndex,
    TraceStoreLineStringBufferIndex,
    TraceStoreLineStringBufferMetadataInfoIndex,
    TraceStoreLineStringBufferAllocationIndex,
    TraceStoreLineStringBufferRelocationIndex,
    TraceStoreLineStringBufferAddressIndex,
    TraceStoreLineStringBufferAddressRangeIndex,
    TraceStoreLineStringBufferAllocationTimestampIndex,
    TraceStoreLineStringBufferAllocationTimestampDeltaIndex,
    TraceStoreLineStringBufferInfoIndex,
    TraceStoreCallStackIndex,
    TraceStoreCallStackMetadataInfoIndex,
    TraceStoreCallStackAllocationIndex,
    TraceStoreCallStackRelocationIndex,
    TraceStoreCallStackAddressIndex,
    TraceStoreCallStackAddressRangeIndex,
    TraceStoreCallStackAllocationTimestampIndex,
    TraceStoreCallStackAllocationTimestampDeltaIndex,
    TraceStoreCallStackInfoIndex,
    TraceStorePerformanceIndex,
    TraceStorePerformanceMetadataInfoIndex,
    TraceStorePerformanceAllocationIndex,
    TraceStorePerformanceRelocationIndex,
    TraceStorePerformanceAddressIndex,
    TraceStorePerformanceAddressRangeIndex,
    TraceStorePerformanceAllocationTimestampIndex,
    TraceStorePerformanceAllocationTimestampDeltaIndex,
    TraceStorePerformanceInfoIndex,
    TraceStorePerformanceDeltaIndex,
    TraceStorePerformanceDeltaMetadataInfoIndex,
    TraceStorePerformanceDeltaAllocationIndex,
    TraceStorePerformanceDeltaRelocationIndex,
    TraceStorePerformanceDeltaAddressIndex,
    TraceStorePerformanceDeltaAddressRangeIndex,
    TraceStorePerformanceDeltaAllocationTimestampIndex,
    TraceStorePerformanceDeltaAllocationTimestampDeltaIndex,
    TraceStorePerformanceDeltaInfoIndex,
    TraceStoreInvalidIndex
} TRACE_STORE_INDEX, *PTRACE_STORE_INDEX;

#define MAX_TRACE_STORES TraceStoreInvalidIndex

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
