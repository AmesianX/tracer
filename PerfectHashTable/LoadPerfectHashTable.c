/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    LoadPerfectHashTable.c

Abstract:

    This module implements functionality for loading on-disk representations
    of previously created perfect hash tables (from CreatePerfectHashTable())
    via the LoadPerfectHashTable() routine.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOLEAN
LoadPerfectHashTable(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PPERFECT_HASH_TABLE_KEYS Keys,
    PCUNICODE_STRING Path,
    PPERFECT_HASH_TABLE *TablePointer
    )
/*++

Routine Description:

    Initializes a new PERFECT_HASH_TABLE structure based on an on-disk
    representation of the table created via CreatePerfectHashTable().

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an initialized ALLOCATOR structure that
        will be used to allocate memory for the underlying PERFECT_HASH
        structure.

    Keys - Optionally supplies a pointer to the keys for the hash table.

    Path - Supplies a pointer to a UNICODE_STRING structure representing the
        fully-qualified, NULL-terminated path of the file to be used to load
        the table.

    TablePointer - Supplies the address of a variable that will receive
        the address of the newly created PERFECT_HASH_TABLE structure
        if the routine is successful, or NULL if the routine fails.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PWSTR Dest;
    PWSTR Source;
    PBYTE Buffer;
    BOOLEAN Success;
    ULONG LastError;
    ULONG ShareMode;
    ULONG FlagsAndAttributes;
    PVOID BaseAddress;
    HANDLE FileHandle;
    HANDLE MappingHandle;
    ULARGE_INTEGER AllocSize;
    FILE_STANDARD_INFO FileInfo;
    ULONG_INTEGER PathBufferSize;
    ULONG_INTEGER InfoPathBufferSize;
    ULARGE_INTEGER ExpectedEndOfFile;
    ULONG_INTEGER AlignedPathBufferSize;
    ULONG_INTEGER AlignedInfoPathBufferSize;
    ULONGLONG NumberOfKeys;
    ULONGLONG NumberOfTableElements;
    PPERFECT_HASH_TABLE Table;
    PTABLE_INFO_ON_DISK_HEADER Header;
    PERFECT_HASH_TABLE_ALGORITHM_ID AlgorithmId;
    UNICODE_STRING InfoSuffix = RTL_CONSTANT_STRING(L":Info");

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TablePointer)) {
        return FALSE;
    }

    if (!IsValidMinimumDirectoryNullTerminatedUnicodeString(Path)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer up-front.
    //

    *TablePointer = NULL;

    //
    // Calculate the size required to store a copy of the Path's unicode string
    // buffer, plus a trailing NULL.
    //

    PathBufferSize.LongPart = Path->Length + sizeof(Path->Buffer[0]);

    //
    // Ensure we haven't overflowed MAX_USHORT.
    //

    ASSERT(!PathBufferSize.HighPart);

    //
    // Align up to a 16 byte boundary.
    //

    AlignedPathBufferSize.LongPart = (
        ALIGN_UP(
            PathBufferSize.LongPart,
            16
        )
    );

    //
    // Repeat overflow check.
    //

    ASSERT(!AlignedPathBufferSize.HighPart);

    //
    // Calculate the size required for the :Info stream's fully-qualified path
    // name, which we obtain by adding the main path's length to the length
    // of the suffix, then plus the trailing NULL.
    //

    InfoPathBufferSize.LongPart = (

        //
        // Account for the length in bytes of the incoming path name.
        //

        Path->Length +

        //
        // Account for the length of the suffix.
        //

        InfoSuffix.Length +

        //
        // Account for a trailing NULL.
        //

        sizeof(InfoSuffix.Buffer[0])
    );

    //
    // Overflow check.
    //

    ASSERT(!InfoPathBufferSize.HighPart);

    //
    // Align up to a 16 byte boundary.
    //

    AlignedInfoPathBufferSize.LongPart = (
        ALIGN_UP(
            InfoPathBufferSize.LongPart,
            16
        )
    );

    ASSERT(!AlignedInfoPathBufferSize.HighPart);

    //
    // Calculate the entire allocation size, including size of the table
    // structure and supporting unicode string buffers.
    //

    AllocSize.QuadPart = (

        //
        // Account for the table structure.
        //

        sizeof(*Table) +

        //
        // Account for the unicode string buffer backing the path.
        //

        AlignedPathBufferSize.LongPart +

        //
        // Account for the unicode string buffer backing the :Info stream.
        //

        AlignedInfoPathBufferSize.LongPart

    );

    //
    // Sanity check we haven't overflowed MAX_ULONG.
    //


    ASSERT(!AllocSize.HighPart);

    //
    // Proceed with allocation.
    //

    Table = (PPERFECT_HASH_TABLE)(
        Allocator->Calloc(Allocator->Context,
                          1,
                          AllocSize.LowPart)
    );

    if (!Table) {
        return FALSE;
    }

    //
    // Allocation was successful, continue with initialization.
    //

    Table->SizeOfStruct = sizeof(*Table);
    Table->Rtl = Rtl;
    Table->Keys = Keys;
    Table->Allocator = Allocator;

    //
    // Clear the flags, then toggle the loaded bit, indicating that this table
    // structure was derived from the LoadPerfectHashTable() routine, and not
    // the CreatePerfectHashTable() routine.
    //

    Table->Flags.AsULong = 0;
    Table->Flags.Loaded = TRUE;

    //
    // Carve out the backing memory structures for the path's unicode buffer.
    //

    Buffer = RtlOffsetToPointer(Table, sizeof(*Table));

    //
    // Wire up the path structure.
    //

    Table->Path.Buffer = (PWSTR)Buffer;
    Table->Path.Length = Path->Length;
    Table->Path.MaximumLength = Path->MaximumLength;

    //
    // Copy the path provided as a parameter to our local table's copy.
    //

    CopyMemory(Table->Path.Buffer, Path->Buffer, Path->Length);

    //
    // Verify the buffer is NULL-terminated.
    //

    Dest = &Table->Path.Buffer[Table->Path.Length >> 1];
    ASSERT(*Dest == L'\0');

    //
    // Advance past the aligned path buffer size such that we're positioned at
    // the start of the info stream buffer.
    //

    Buffer += AlignedPathBufferSize.LongPart;
    Table->InfoStreamPath.Buffer = (PWSTR)Buffer;
    Table->InfoStreamPath.MaximumLength = InfoPathBufferSize.LowPart;
    Table->InfoStreamPath.Length = (
        Table->InfoStreamPath.MaximumLength -
        sizeof(Table->InfoStreamPath.Buffer[0])
    );

    //
    // Copy the full path into the info stream buffer.
    //

    CopyMemory(Table->InfoStreamPath.Buffer,
               Table->Path.Buffer,
               Table->Path.Length);

    //
    // Advance the Dest pointer to the end of the path buffer (e.g. after the
    // ".pht1" suffix).  Assert we're looking at a NULL character.
    //

    Dest = Table->InfoStreamPath.Buffer;
    Dest += (Table->Path.Length >> 1);
    ASSERT(*Dest == L'\0');

    //
    // Copy the :Info suffix over, then NULL-terminate.
    //

    Source = InfoSuffix.Buffer;

    while (*Source) {
        *Dest++ = *Source++;
    }

    *Dest = L'\0';

    //
    // We've finished initializing our two unicode string buffers for the
    // backing file and it's :Info counterpart.  Initialize some aliases for
    // the CreateFile() calls, then attempt to open the :Info stream.
    //

    ShareMode = (
        FILE_SHARE_READ  |
        FILE_SHARE_WRITE |
        FILE_SHARE_DELETE
    );

    FlagsAndAttributes = FILE_FLAG_OVERLAPPED;

    FileHandle = CreateFileW(Table->InfoStreamPath.Buffer,
                             GENERIC_READ,
                             ShareMode,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_OVERLAPPED,
                             NULL);

    Table->InfoStreamFileHandle = FileHandle;

    if (!FileHandle || FileHandle == INVALID_HANDLE_VALUE) {

        //
        // Failed to open the file.  Without the :Info file, we can't proceed,
        // as it contains metadata information about the underlying perfect
        // hash table (such as algorithm used, hash function used, etc).  So,
        // error out.
        //

        LastError = GetLastError();
        goto Error;
    }

    //
    // Successfully opened the :Info stream.  Obtain the current size of the
    // file and make sure it has a file size that meets our minimum :Info
    // on-disk size.
    //

    Success = GetFileInformationByHandleEx(
        FileHandle,
        (FILE_INFO_BY_HANDLE_CLASS)FileStandardInfo,
        &FileInfo,
        sizeof(FileInfo)
    );

    if (!Success) {
        LastError = GetLastError();
        goto Error;
    }

    if (FileInfo.EndOfFile.QuadPart < sizeof(*Header)) {

        //
        // File is too small, it can't be an :Info we know about.
        //

        goto Error;
    }

    //
    // The file is a sensible non-zero size.  Proceed with creating a file
    // mapping.  We use 0 as the file size such that we inherit whatever the
    // current file size is.
    //

    MappingHandle = CreateFileMappingW(FileHandle,
                                       NULL,
                                       PAGE_READWRITE,
                                       0,
                                       0,
                                       NULL);

    Table->InfoStreamMappingHandle = MappingHandle;

    if (!MappingHandle || MappingHandle == INVALID_HANDLE_VALUE) {
        LastError = GetLastError();
        goto Error;
    }

    //
    // Successfully created the mapping handle.  Now, map it.
    //

    BaseAddress = MapViewOfFile(MappingHandle, FILE_MAP_READ, 0, 0, 0);

    Table->InfoStreamBaseAddress = BaseAddress;

    if (!BaseAddress) {
        LastError = GetLastError();
        goto Error;
    }

    //
    // We've obtained the TABLE_INFO_ON_DISK_HEADER structure.  Ensure the
    // magic values are what we expect.
    //

    Header = (PTABLE_INFO_ON_DISK_HEADER)BaseAddress;

    if (Header->Magic.LowPart  != TABLE_INFO_ON_DISK_MAGIC_LOWPART ||
        Header->Magic.HighPart != TABLE_INFO_ON_DISK_MAGIC_HIGHPART) {

        //
        // Magic values don't match what we expect.  Abort the loading efforts.
        //

        goto Error;
    }

    //
    // Verify the size of the struct reported by the header is at least as
    // large as our header structure size.
    //

    if (Header->SizeOfStruct < sizeof(*Header)) {
        __debugbreak();
        goto Error;
    }

    //
    // If a Keys parameter has been provided, compare the number of keys it
    // reports to the number of keys registered in the :Info header.  Error
    // out if they differ.
    //

    if (ARGUMENT_PRESENT(Keys)) {

        if (Keys->NumberOfElements.QuadPart != Header->NumberOfKeys.QuadPart) {
            __debugbreak();
            goto Error;
        }

    }

    //
    // Validate the algorithm ID.  We use this as a lookup directly into the
    // loader routines array, so validation is especially important.
    //

    AlgorithmId = Header->AlgorithmId;
    if (!IsValidPerfectHashTableAlgorithmId(AlgorithmId)) {
        __debugbreak();
        goto Error;
    }

    //
    // Validate the hash function ID.
    //

    if (!IsValidPerfectHashTableHashFunctionId(Header->HashFunctionId)) {
        __debugbreak();
        goto Error;
    }

    //
    // Validate the masking type.
    //

    if (!IsValidPerfectHashTableMaskingType(Header->MaskingType)) {
        __debugbreak();
        goto Error;
    }

    //
    // We only support 32-bit (4 byte) keys at the moment.  Enforce this
    // restriction now.

    if (Header->KeySizeInBytes != sizeof(ULONG)) {
        __debugbreak();
        goto Error;
    }

    //
    // Ensure both the number of keys and number of table elements are non-zero,
    // and that the number of keys is less than or equal to the number of table
    // elements.
    //

    NumberOfKeys = Header->NumberOfKeys.QuadPart;
    NumberOfTableElements = Header->NumberOfTableElements.QuadPart;

    if (NumberOfKeys == 0 || NumberOfTableElements == 0) {
        __debugbreak();
        goto Error;
    }

    if (NumberOfKeys > NumberOfTableElements) {
        __debugbreak();
        goto Error;
    }

    //
    // Algorithm looks valid.  Proceed with opening up the actual table data
    // file.
    //

    FileHandle = CreateFileW(Table->Path.Buffer,
                             GENERIC_READ,
                             ShareMode,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_OVERLAPPED,
                             NULL);

    Table->FileHandle = FileHandle;

    if (!FileHandle || FileHandle == INVALID_HANDLE_VALUE) {

        //
        // Failed to open the underlying data file.  Abort.
        //

        LastError = GetLastError();
        goto Error;
    }

    //
    // Successfully opened the data file.  Get the file information.
    //

    Success = GetFileInformationByHandleEx(
        FileHandle,
        (FILE_INFO_BY_HANDLE_CLASS)FileStandardInfo,
        &FileInfo,
        sizeof(FileInfo)
    );

    if (!Success) {
        LastError = GetLastError();
        goto Error;
    }

    //
    // We can determine the expected file size by multipling the number of
    // table elements by the key size; both of which are available in the
    // :Info header.
    //

    ExpectedEndOfFile.QuadPart = NumberOfTableElements * Header->KeySizeInBytes;

    //
    // Sanity check that the expected end of file is not 0.
    //

    ASSERT(ExpectedEndOfFile.QuadPart > 0);

    //
    // Compare the expected value to the actual on-disk file size.  They should
    // be identical.  If they're not, abort.
    //

    if (FileInfo.EndOfFile.QuadPart != ExpectedEndOfFile.QuadPart) {

        //
        // Sizes don't match, abort.
        //

        LastError = GetLastError();
        goto Error;
    }

    //
    // File size is valid, proceed with creating a mapping section.  Don't
    // specify a mapping size; just inherit the default.
    //

    MappingHandle = CreateFileMappingW(FileHandle,
                                       NULL,
                                       PAGE_READWRITE,
                                       0,
                                       0,
                                       NULL);

    Table->MappingHandle = MappingHandle;

    if (!MappingHandle || MappingHandle == INVALID_HANDLE_VALUE) {

        //
        // Couldn't obtain a mapping, abort.
        //

        LastError = GetLastError();
        goto Error;
    }

    //
    // Created the mapping successfully.  Now, map it.
    //

    BaseAddress = MapViewOfFile(MappingHandle, FILE_MAP_READ, 0, 0, 0);

    Table->BaseAddress = BaseAddress;

    if (!BaseAddress) {

        //
        // Failed to map the contents into memory.  Abort.
        //

        LastError = GetLastError();
        goto Error;
    }

    //
    // Allocate an array for the table values (i.e. the things stored when the
    // Insert(Key, Value) routine is called).  This will have the same
    // dimensions as the key array and can be indexed directly by the result
    // of the perfect hash routine.
    //

    BaseAddress = Allocator->Calloc(Allocator->Context,
                                    NumberOfKeys,
                                    Header->KeySizeInBytes);

    Table->ValuesBaseAddress = BaseAddress;

    if (!BaseAddress) {
        goto Error;
    }

    //
    // We've completed loading the :Info structure and corresponding data array.
    // Call the algorithm's loader routine to give it a chance to continue
    // initialization.
    //

    Success = LoaderRoutines[AlgorithmId](Table);

    if (Success) {

        //
        // We're finally done!
        //

        goto End;
    }

    //
    // Intentional follow-on to Error.
    //

Error:

    Success = FALSE;

    //
    // Call the destroy routine on the table if one is present.  This will
    // take care of cleaning up any partial resource state regarding handles
    // and mappings etc.  i.e. it's safe to call at any point during the table
    // initialization.
    //

    if (Table) {

        if (!DestroyPerfectHashTable(&Table, NULL)) {

            //
            // There's nothing we can do here.
            //

            NOTHING;
        }

        //
        // N.B. DestroyPerfectHashTable() should clear the Table pointer.
        //      Assert that invariant now.
        //

        ASSERT(Table == NULL);
    }

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Update the caller's pointer and return.
    //
    // N.B. Table could be NULL here, which is fine.
    //

    *TablePointer = Table;

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :