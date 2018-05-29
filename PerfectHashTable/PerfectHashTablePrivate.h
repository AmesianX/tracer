/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    PerfectHashTablePrivate.h

Abstract:

    This is the private header file for the PerfectHashTable component.  It defines
    function typedefs and function declarations for all major (i.e. not local
    to the module) functions available for use by individual modules within
    this component.

--*/

#ifndef _PERFECT_HASH_TABLE_INTERNAL_BUILD
#error PerfectHashTablePrivate.h being included but _PERFECT_HASH_TABLE_INTERNAL_BUILD not set.
#endif

#pragma once

#include "stdafx.h"

#ifndef ASSERT
#define ASSERT(Condition)   \
    if (!(Condition)) {     \
        __debugbreak();     \
    }
#endif

//
// Define the PERFECT_HASH_TABLE_KEYS_FLAGS structure.
//

typedef union _PERFECT_HASH_TABLE_FLAGS_KEYS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Unused bits.
        //

        ULONG Unused:32;
    };

    LONG AsLong;
    ULONG AsULong;
} PERFECT_HASH_TABLE_KEYS_FLAGS;
C_ASSERT(sizeof(PERFECT_HASH_TABLE_KEYS_FLAGS) == sizeof(ULONG));
typedef PERFECT_HASH_TABLE_KEYS_FLAGS *PPERFECT_HASH_TABLE_KEYS_FLAGS;

//
// Define the PERFECT_HASH_TABLE_KEYS structure.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _PERFECT_HASH_TABLE_KEYS {

    //
    // Reserve a slot for a vtable.  Currently unused.
    //

    PPVOID Vtbl;

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _PERFECT_HASH_TABLE_KEYS))
        ULONG SizeOfStruct;

    //
    // Flags.
    //

    PERFECT_HASH_TABLE_KEYS_FLAGS Flags;

    //
    // Pointer to an initialized RTL structure.
    //

    PRTL Rtl;

    //
    // Pointer to an initialized ALLOCATOR structure.
    //

    PALLOCATOR Allocator;

    //
    // Pointer to the API structure in use.
    //

    PPERFECT_HASH_TABLE_ANY_API AnyApi;

    //
    // Number of keys in the mapping.
    //

    ULARGE_INTEGER NumberOfElements;

    //
    // Handle to the underlying keys file.
    //

    HANDLE FileHandle;

    //
    // Handle to the memory mapping for the keys file.
    //

    HANDLE MappingHandle;

    //
    // Base address of the memory map.
    //

    PVOID BaseAddress;

    //
    // Fully-qualifed, NULL-terminated path of the source keys file.
    //

    UNICODE_STRING Path;

} PERFECT_HASH_TABLE_KEYS;
typedef PERFECT_HASH_TABLE_KEYS *PPERFECT_HASH_TABLE_KEYS;

//
// Algorithms are required to register a callback routine with the perfect hash
// table context that matches the following signature.  This routine will be
// called for each work item it pushes to the context's main threadpool, with
// a pointer to the SLIST_ENTRY that was popped off the list.
//

typedef
VOID
(CALLBACK PERFECT_HASH_TABLE_MAIN_WORK_CALLBACK)(
    _In_ PTP_CALLBACK_INSTANCE Instance,
    _In_ PPERFECT_HASH_TABLE_CONTEXT Context,
    _In_ PSLIST_ENTRY ListEntry
    );
typedef PERFECT_HASH_TABLE_MAIN_WORK_CALLBACK
      *PPERFECT_HASH_TABLE_MAIN_WORK_CALLBACK;

//
// Additionally, algorithms can register a callback routine for performing
// file-oriented operations in the main threadpool (not directly related to
// graph solving).
//

typedef
VOID
(CALLBACK PERFECT_HASH_TABLE_FILE_WORK_CALLBACK)(
    _In_ PTP_CALLBACK_INSTANCE Instance,
    _In_ PPERFECT_HASH_TABLE_CONTEXT Context,
    _In_ PSLIST_ENTRY ListEntry
    );
typedef PERFECT_HASH_TABLE_FILE_WORK_CALLBACK
      *PPERFECT_HASH_TABLE_FILE_WORK_CALLBACK;


//
// Define a runtime context to encapsulate threadpool resources.  This is
// passed to CreatePerfectHashTable() and allows for algorithms to search for
// perfect hash solutions in parallel.
//

typedef union _PERFECT_HASH_TABLE_CONTEXT_FLAGS {
    struct {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} PERFECT_HASH_TABLE_CONTEXT_FLAGS;
C_ASSERT(sizeof(PERFECT_HASH_TABLE_CONTEXT_FLAGS) == sizeof(ULONG));
typedef PERFECT_HASH_TABLE_CONTEXT_FLAGS *PPERFECT_HASH_TABLE_CONTEXT_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _PERFECT_HASH_TABLE_CONTEXT {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _PERFECT_HASH_TABLE_CONTEXT))
        ULONG SizeOfStruct;

    //
    // Flags.
    //

    PERFECT_HASH_TABLE_CONTEXT_FLAGS Flags;

    //
    // The algorithm in use.
    //

    PERFECT_HASH_TABLE_ALGORITHM_ID AlgorithmId;

    //
    // The masking type in use.
    //

    PERFECT_HASH_TABLE_MASKING_TYPE MaskingType;

    //
    // The hash function in use.
    //

    PERFECT_HASH_TABLE_HASH_FUNCTION_ID HashFunctionId;

    //
    // Pointer to an initialized RTL structure.
    //

    PRTL Rtl;

    //
    // Pointer to an initialized allocator.
    //

    PALLOCATOR Allocator;

    //
    // Pointer to the API structure in use.
    //

    PPERFECT_HASH_TABLE_ANY_API AnyApi;

    //
    // Pointer to the active perfect hash table.
    //

    struct _PERFECT_HASH_TABLE *Table;

    //
    // Define the events used to communicate various internal state changes
    // between the CreatePerfectHashTable() function and the algorithm-specific
    // creation routine.
    //
    // N.B. All of these events are created with the manual reset flag set to
    //      TRUE, such that they stay signalled even after they have satisfied
    //      a wait.
    //

    //
    // A global "shutdown" event handle that threads can query to determine
    // whether or not they should continue processing at various internal
    // checkpoints.
    //

    union {
        HANDLE ShutdownEvent;
        PVOID FirstEvent;
    };

    //
    // This event will be set if an algorithm was successful in finding a
    // perfect hash.  Either it or the FailedEvent will be set; never both.
    //

    HANDLE SucceededEvent;

    //
    // This event will be set if an algorithm failed to find a perfect hash
    // solution.  This may be due to the algorithm exhausting all possible
    // options, hitting a time limit, or potentially as a result of being
    // forcibly terminated or some other internal error.  It will never be
    // set if SucceededEvent is also set.
    //

    HANDLE FailedEvent;

    //
    // The following event is required to be set by an algorithm's creation
    // routine upon completion (regardless of success or failure).  This event
    // is waited upon by the CreatePerfectHashTable() function, and thus, is
    // critical in synchronizing the execution of parallel perfect hash solution
    // finding.
    //

    HANDLE CompletedEvent;

    //
    // The following event is set when a worker thread has completed preparing
    // the underlying backing file in order for the solved graph to be persisted
    // to disk.
    //

    HANDLE PreparedFileEvent;

    //
    // The following event is set when a worker thread has completed saving the
    // solved graph to disk.
    //

    union {
        HANDLE SavedFileEvent;
        PVOID LastEvent;
    };

    //
    // N.B. All events are created as named events, using the random object
    //      name generation helper Rtl->CreateRandomObjectNames().  This will
    //      fill out an array of PUNICODE_STRING pointers.  The next field
    //      points to the first element of that array.  Subsequent fields
    //      capture various book-keeping items about the random object names
    //      allocation (provided by the Rtl routine).
    //

    PUNICODE_STRING ObjectNames;
    PPUNICODE_STRING ObjectNamesPointerArray;
    PWSTR ObjectNamesWideBuffer;
    ULONG SizeOfObjectNamesWideBuffer;

    //
    // We fill in the following field for convenience.  This also aligns us
    // out to an 8 byte boundary.
    //

    ULONG NumberOfObjects;

    //
    // Number of attempts made by the algorithm to find a solution.
    //

    volatile ULONG Attempts;

    //
    // The main threadpool callback environment, used for solving perfect hash
    // solutions in parallel.
    //

    TP_CALLBACK_ENVIRON MainCallbackEnv;
    PTP_CLEANUP_GROUP MainCleanupGroup;
    PTP_POOL MainThreadpool;
    PTP_WORK MainWork;
    SLIST_HEADER MainWorkListHead;
    ULONG MinimumConcurrency;
    ULONG MaximumConcurrency;

    //
    // The algorithm is responsible for registering an appropriate callback
    // for main thread work items in this next field.
    //

    PPERFECT_HASH_TABLE_MAIN_WORK_CALLBACK MainWorkCallback;

    //
    // A threadpool for offloading file operations.
    //

    TP_CALLBACK_ENVIRON FileCallbackEnv;
    PTP_CLEANUP_GROUP FileCleanupGroup;
    PTP_POOL FileThreadpool;
    PTP_WORK FileWork;
    SLIST_HEADER FileWorkListHead;

    //
    // Provide a means for file work callbacks to indicate an error back to
    // the creation routine by incrementing the following counter.
    //

    volatile ULONG FileWorkErrors;
    volatile ULONG FileWorkLastError;

    //
    // The algorithm is responsible for registering an appropriate callback
    // for file work threadpool work items in this next field.
    //

    PPERFECT_HASH_TABLE_FILE_WORK_CALLBACK FileWorkCallback;

    //
    // If a threadpool worker thread finds a perfect hash solution, it will
    // enqueue a "Finished!"-type work item to a separate threadpool, captured
    // by the following callback environment.  This allows for a separate
    // threadpool worker to schedule the cancellation of other in-progress
    // and outstanding perfect hash solution attempts without deadlocking.
    //
    // This threadpool environment is serviced by a single thread.
    //
    // N.B. This cleanup only refers to the main graph solving thread pool.
    //      The file threadpool is managed by the implicit lifetime of the
    //      algorithm's creation routine (e.g. CreatePerfectHashTableImplChm01).
    //

    TP_CALLBACK_ENVIRON FinishedCallbackEnv;
    PTP_POOL FinishedThreadpool;
    PTP_WORK FinishedWork;
    SLIST_HEADER FinishedWorkListHead;

    //
    // If a worker thread successfully finds a perfect hash solution, it will
    // push its solution to the FinishedListHead above, then submit a finished
    // work item via SubmitThreadpoolWork(Context->FinishedWork).
    //
    // This callback will be processed by the finished group above, and provides
    // a means for that thread to set the ShutdownEvent and cancel outstanding
    // main work callbacks.
    //
    // N.B. Although we only need one solution, we don't prevent multiple
    //      successful solutions from being pushed to the FinishedListHead.
    //      Whatever the first solution is that the finished callback pops
    //      off that list is the solution that wins.
    //

    volatile ULONGLONG FinishedCount;

    //
    // Similar to the Finished group above, provide an Error group that also
    // consists of a single thread.  If a main threadpool worker thread runs
    // into a fatal error that requires termination of all in-progress and
    // outstanding threadpool work items, it can just dispatch a work item
    // to this particular pool (e.g. SubmitThreadpoolWork(Context->ErrorWork)).
    //
    // There is no ErrorListHead as no error information is captured that needs
    // communicating back to a central location.
    //

    TP_CALLBACK_ENVIRON ErrorCallbackEnv;
    PTP_POOL ErrorThreadpool;
    PTP_WORK ErrorWork;

    //
    // An opaque pointer that can be used by the algorithm to stash additional
    // context.
    //

    PVOID AlgorithmContext;

    //
    // An opaque pointer that can be used by the hash function to stash
    // additional context.
    //

    PVOID HashFunctionContext;

    //
    // An opaque pointer to the winning solution (i.e. the solved graph).
    //

    PVOID SolvedContext;

} PERFECT_HASH_TABLE_CONTEXT;
typedef PERFECT_HASH_TABLE_CONTEXT *PPERFECT_HASH_TABLE_CONTEXT;

//
// Define the PERFECT_HASH_TABLE_FLAGS structure.
//

typedef union _PERFECT_HASH_TABLE_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // When set, indicates the table came from CreatePerfectHashTable().
        //
        // Invariant:
        //
        //  - If Created == TRUE:
        //      Assert Loaded == FALSE
        //

        ULONG Created:1;

        //
        // When set, indicates the table came from LoadPerfectHashTable().
        //
        // Invariant:
        //
        //  - If Loaded == TRUE:
        //      Assert Created == FALSE
        //

        ULONG Loaded:1;

        //
        // Unused bits.
        //

        ULONG Unused:30;
    };

    LONG AsLong;
    ULONG AsULong;
} PERFECT_HASH_TABLE_FLAGS;
C_ASSERT(sizeof(PERFECT_HASH_TABLE_FLAGS) == sizeof(ULONG));
typedef PERFECT_HASH_TABLE_FLAGS *PPERFECT_HASH_TABLE_FLAGS;

//
// Define the PERFECT_HASH_TABLE structure.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _PERFECT_HASH_TABLE {

    //
    // Reserve a slot for a vtable.
    //

    PPVOID Vtbl;

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _PERFECT_HASH_TABLE)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    PERFECT_HASH_TABLE_FLAGS Flags;

    //
    // Generic singly-linked list entry.
    //

    SLIST_ENTRY ListEntry;

    //
    // Pointer to an initialized RTL structure.
    //

    PRTL Rtl;

    //
    // Pointer to an initialized ALLOCATOR structure.
    //

    PALLOCATOR Allocator;

    //
    // Pointer to the API structure in use.
    //

    PPERFECT_HASH_TABLE_ANY_API AnyApi;

    //
    // Pointer to the keys corresponding to this perfect hash table.  May be
    // NULL.
    //

    PPERFECT_HASH_TABLE_KEYS Keys;

    //
    // Pointer to the PERFECT_HASH_TABLE_CONTEXT structure in use.
    //

    PPERFECT_HASH_TABLE_CONTEXT Context;

    //
    // Handle to the backing file.
    //

    HANDLE FileHandle;

    //
    // Handle to the memory mapping for the backing file.
    //

    HANDLE MappingHandle;

    //
    // Base address of the memory map for the backing file.
    //

    union {
        PVOID BaseAddress;
        PULONG Data;
    };

    //
    // Fully-qualified, NULL-terminated path of the backing file.  The path is
    // automatically derived from the keys file.
    //

    UNICODE_STRING Path;

    //
    // Handle to the info stream backing file.
    //

    HANDLE InfoStreamFileHandle;

    //
    // Handle to the memory mapping for the backing file.
    //

    HANDLE InfoStreamMappingHandle;

    //
    // Base address of the memory map for the :Info stream.
    //

    union {
        PVOID InfoStreamBaseAddress;
        struct _TABLE_INFO_ON_DISK_HEADER *Header;
    };

    //
    // Fully-qualified, NULL-terminated path of the :Info stream associated with
    // the path above.
    //

    UNICODE_STRING InfoStreamPath;

    //
    // Capture the mapping size and actual structure size for the :Info stream.
    //

    ULARGE_INTEGER InfoMappingSizeInBytes;
    ULARGE_INTEGER InfoActualStructureSizeInBytes;

    //
    // If a table is loaded successfully, an array will be allocated for storing
    // values (as part of the Insert()/Lookup() API), the base address for which
    // is captured by the next field.
    //

    union {
        PVOID ValuesBaseAddress;
        PULONG Values;
    };

} PERFECT_HASH_TABLE;
typedef PERFECT_HASH_TABLE *PPERFECT_HASH_TABLE;

//
// Metadata about a perfect hash table is stored in an NTFS stream named :Info
// that is tacked onto the end of the perfect hash table's file name.  Define
// a structure, TABLE_INFO_ON_DISK_HEADER, that literally represents the on-disk
// layout of this metadata.  Each algorithm implementation must write out an
// info record that conforms with this common header.  They are free to extend
// it with additional details.
//

typedef union _TABLE_INFO_ON_DISK_HEADER_FLAGS {

    struct {

        //
        // Unused bits.
        //

        ULONG Unused:32;

    };

    LONG AsLong;
    ULONG AsULong;

} TABLE_INFO_ON_DISK_HEADER_FLAGS;
C_ASSERT(sizeof(TABLE_INFO_ON_DISK_HEADER_FLAGS) == sizeof(ULONG));

typedef struct _Struct_size_bytes_(SizeOfStruct) _TABLE_INFO_ON_DISK_HEADER {

    //
    // A magic value used to identify if the structure.
    //

    ULARGE_INTEGER Magic;

    //
    // Size of the structure, in bytes.
    //
    // N.B. We don't allocate this with a _Field_range_ SAL annotation as the
    //      value will vary depending on which parameters were used to create
    //      the table.
    //

    ULONG SizeOfStruct;

    //
    // Flags.
    //

    TABLE_INFO_ON_DISK_HEADER_FLAGS Flags;

    //
    // Algorithm that was used.
    //

    PERFECT_HASH_TABLE_ALGORITHM_ID AlgorithmId;

    //
    // Hash function that was used.
    //

    PERFECT_HASH_TABLE_HASH_FUNCTION_ID HashFunctionId;

    //
    // Masking type.
    //

    PERFECT_HASH_TABLE_MASKING_TYPE MaskingType;

    //
    // Size of an individual key element, in bytes.
    //

    ULONG KeySizeInBytes;

    //
    // Number of keys in the input set.  This is used to size an appropriate
    // array for storing values.
    //

    ULARGE_INTEGER NumberOfKeys;

    //
    // Final number of elements in the underlying table.  This will vary
    // depending on how the graph was created.
    //

    ULARGE_INTEGER NumberOfTableElements;

    //
    // Seed data.
    //

    ULONG Seed1;
    ULONG Seed2;
    ULONG Seed3;
    ULONG Seed4;

} TABLE_INFO_ON_DISK_HEADER;
typedef TABLE_INFO_ON_DISK_HEADER *PTABLE_INFO_ON_DISK_HEADER;

//
// Define an enumeration to capture the type of file work operations we want
// to be able to dispatch to the file work threadpool callback.
//

typedef enum _FILE_WORK_ID {

    //
    // Null ID.
    //

    FileWorkNullId = 0,

    //
    // Initial file preparation once the underlying sizes required are known.
    //

    FileWorkPrepareId = 1,

    //
    // Perfect hash solution has been solved and is ready to be saved to disk.
    //

    FileWorkSaveId,

    //
    // Invalid ID, this must come last.
    //

    FileWorkInvalidId

} FILE_WORK_ID;

FORCEINLINE
BOOLEAN
IsValidFileWorkId(
    _In_ FILE_WORK_ID FileWorkId
    )
{
    return (
        FileWorkId > FileWorkNullId &&
        FileWorkId < FileWorkInvalidId
    );
}

//
// Define a file work item structure that will be pushed to the context's
// file work list head.
//

typedef struct _FILE_WORK_ITEM {

    //
    // Singly-linked list entry for the structure.
    //

    SLIST_ENTRY ListEntry;

    //
    // Type of work requested.
    //

    FILE_WORK_ID FileWorkId;

    //
    // Pad out to an 8 byte boundary.
    //

    ULONG Unused;

} FILE_WORK_ITEM;
typedef FILE_WORK_ITEM *PFILE_WORK_ITEM;

//
// Private function definition for destroying a hash table.  We don't make
// this a public function as the CreatePerfectHashTable() does not return
// a table to the caller, and LoadPerfectHashTable() returns a vtbl pointer
// that we expect the caller to use AddRef()/Release() on correctly in order
// to manage lifetime.
//

typedef
BOOLEAN
(NTAPI DESTROY_PERFECT_HASH_TABLE)(
    _Pre_notnull_ _Post_satisfies_(*PerfectHashTablePointer == 0)
    PPERFECT_HASH_TABLE *PerfectHashTablePointer,
    _In_opt_ PBOOLEAN IsProcessTerminating
    );
typedef DESTROY_PERFECT_HASH_TABLE *PDESTROY_PERFECT_HASH_TABLE;
DESTROY_PERFECT_HASH_TABLE DestroyPerfectHashTable;

//
// TLS-related structures and functions.
//

typedef struct _PERFECT_HASH_TABLE_TLS_CONTEXT {
    PVOID Unused;
} PERFECT_HASH_TABLE_TLS_CONTEXT;
typedef PERFECT_HASH_TABLE_TLS_CONTEXT *PPERFECT_HASH_TABLE_TLS_CONTEXT;

extern ULONG PerfectHashTableTlsIndex;

//
// Function typedefs for private functions.
//

//
// Each algorithm implements a creation routine that matches the following
// signature.  It is called by CreatePerfectHashTable() after it has done all
// the initial heavy-lifting (e.g. parameter validation, table allocation and
// initialization), and thus, has a much simpler function signature.
//

typedef
_Check_return_
_Success_(return != 0)
BOOLEAN
(NTAPI CREATE_PERFECT_HASH_TABLE_IMPL)(
    _Inout_ PPERFECT_HASH_TABLE Table
    );
typedef CREATE_PERFECT_HASH_TABLE_IMPL *PCREATE_PERFECT_HASH_TABLE_IMPL;

//
// For each algorithm, declare the creation impl routine.  These are gathered
// in an array named CreationRoutines[] (see PerfectHashTableConstants.[ch]).
//

CREATE_PERFECT_HASH_TABLE_IMPL CreatePerfectHashTableImplChm01;

//
// Likewise, each algorithm implements a loader routine that matches the
// following signature.  It is called by LoadPerfectHashTable() after it
// has done the initial heavy-lifting (e.g. parameter validation, table
// allocation and initialization), and, thus, has a much simpler function
// signature.
//

typedef
_Check_return_
_Success_(return != 0)
BOOLEAN
(NTAPI LOAD_PERFECT_HASH_TABLE_IMPL)(
    _Inout_ PPERFECT_HASH_TABLE Table
    );
typedef LOAD_PERFECT_HASH_TABLE_IMPL *PLOAD_PERFECT_HASH_TABLE_IMPL;

//
// For each algorithm, declare the loader impl routine.  These are gathered
// in an array named LoaderRoutines[] (see PerfectHashTableConstants.[ch]).
//

LOAD_PERFECT_HASH_TABLE_IMPL LoadPerfectHashTableImplChm01;

//
// The PROCESS_ATTACH and PROCESS_ATTACH functions share the same signature.
//

typedef
_Check_return_
_Success_(return != 0)
(PERFECT_HASH_TABLE_TLS_FUNCTION)(
    _In_    HMODULE     Module,
    _In_    DWORD       Reason,
    _In_    LPVOID      Reserved
    );
typedef PERFECT_HASH_TABLE_TLS_FUNCTION *PPERFECT_HASH_TABLE_TLS_FUNCTION;

PERFECT_HASH_TABLE_TLS_FUNCTION PerfectHashTableTlsProcessAttach;
PERFECT_HASH_TABLE_TLS_FUNCTION PerfectHashTableTlsProcessDetach;

//
// Define TLS Get/Set context functions.
//

typedef
_Check_return_
_Success_(return != 0)
BOOLEAN
(PERFECT_HASH_TABLE_TLS_SET_CONTEXT)(
    _In_ struct _PERFECT_HASH_TABLE_CONTEXT *Context
    );
typedef PERFECT_HASH_TABLE_TLS_SET_CONTEXT *PPERFECT_HASH_TABLE_TLS_SET_CONTEXT;

typedef
_Check_return_
_Success_(return != 0)
struct _PERFECT_HASH_TABLE_CONTEXT *
(PERFECT_HASH_TABLE_TLS_GET_CONTEXT)(
    VOID
    );
typedef PERFECT_HASH_TABLE_TLS_GET_CONTEXT *PPERFECT_HASH_TABLE_TLS_GET_CONTEXT;

extern PERFECT_HASH_TABLE_TLS_SET_CONTEXT PerfectHashTableTlsSetContext;
extern PERFECT_HASH_TABLE_TLS_GET_CONTEXT PerfectHashTableTlsGetContext;

//
// Inline helper functions.
//

#define MAX_RDRAND_RETRY_COUNT 10

FORCEINLINE
BOOLEAN
GetRandomSeeds(
    _Out_ PULARGE_INTEGER Output,
    _Out_opt_ PULARGE_INTEGER Cycles,
    _Out_opt_ PULONG Attempts
    )
/*++

Routine Description:

    Generates a 64-bit random seed using the rdrand64 intrinisic.

Arguments:

    Output - Supplies a pointer to a ULARGE_INTEGER structure that will receive
        the random seed value.

    Cycles - Optionally supplies a pointer to a variable that will receive the
        approximate number of CPU cycles that were required in order to fulfil
        the random seed request.

        N.B. This calls __rdtsc() before and after the __rdseed64_step() call.
             If the pointer is NULL, __rdtsc() is not called either before or
             after.

    Attempts - Optionally supplies the address of a variable that receives the
        number of attempts it took before __rdseed64_step() succeeded.  (This
        is bound by the MAX_RDRAND_RETRY_COUNT constant.)

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    ULONG Index;
    BOOLEAN Success = FALSE;
    ULARGE_INTEGER Start;
    ULARGE_INTEGER End;

    if (ARGUMENT_PRESENT(Cycles)) {
        Start.QuadPart = ReadTimeStampCounter();
    }

    for (Index = 0; Index < MAX_RDRAND_RETRY_COUNT; Index++) {
        if (_rdseed64_step(&Output->QuadPart)) {
            Success = TRUE;
            break;
        }
        YieldProcessor();
    }

    if (ARGUMENT_PRESENT(Cycles)) {
        End.QuadPart = ReadTimeStampCounter();
        Cycles->QuadPart = End.QuadPart - Start.QuadPart;
    }

    if (ARGUMENT_PRESENT(Attempts)) {
        *Attempts = Index + 1;
    }

    return Success;
}

FORCEINLINE
VOID
GetRandomSeedsBlocking(
    _Out_ PULARGE_INTEGER Output
    )
/*++

Routine Description:

    Calls __rdseed64_step() in a loop until it returns successfully.

Arguments:

    Output - Supplies a pointer to a ULARGE_INTEGER structure that will receive
        the random seed value.

Return Value:

    None.

--*/
{
    while (!_rdseed64_step(&Output->QuadPart)) {
        YieldProcessor();
    }
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :