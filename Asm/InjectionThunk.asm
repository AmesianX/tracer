        title "Injection Thunk Assembly Routine"
        option nokeyword:<Length>

;++
;
; Copyright (c) 2017 Trent Nelson <trent@trent.me>
;
; Module Name:
;
;   InjectionThunk.asm
;
; Abstract:
;
;   This module implements the injection thunk routine.
;
;--

include Asm.inc

;
; Define a local stack structure to be used within our injection routine.
; The base address of this routine will live in rsp after the prologue has run.
;

Locals struct

    ;
    ; Define home parameter space.
    ;

    CalleeHomeRcx   dq      ?
    CalleeHomeRdx   dq      ?
    CalleeHomeR8    dq      ?
    CalleeHomeR9    dq      ?

    ;
    ; Define space for additional function arguments (5-8) to be passed in via
    ; the stack.
    ;

    CalleeParam5    dq      ?
    CalleeParam6    dq      ?
    CalleeParam7    dq      ?
    CalleeParam8    dq      ?

    ;
    ; Define local variables.
    ;

    Temp1           dq      ?

    ;
    ; Any non-volatile registers we use go after this point.  It's convenient
    ; being able to reference a pushed non-volatile register via this Locals
    ; struct (e.g. `mov rax, Locals.SavedRdi[rbp]`), especially when debugging.
    ; However, because we use `rax_push_reg <reg>` to save them in the prologue,
    ; they automatically have stack space allocated for them.  Thus, we need to
    ; account for that by removing the non-volatile registers from the stack
    ; size we allocate via alloc_stack.  This is done by using an anonymous
    ; union for the first non-volatile register with a consistent name (i.e.
    ; FirstNvRegister), and then using that name in the definition of the
    ; LOCALS_SIZE equ below to subtract the non-volatile registers from the
    ; locals stack space.  This gives us the best of both worlds: we can access
    ; the variables via the Locals struct, and our prologue and epilogue code
    ; is kept simple (`alloc_stack LOCALS_SIZE`; `add rsp, LOCALS_SIZE`).
    ;

    union
        FirstNvRegister     dq      ?
        SavedRbp            dq      ?
    ends

    SavedRbx                dq      ?
    SavedRdi                dq      ?
    SavedRsi                dq      ?
    SavedR12                dq      ?
    SavedR13                dq      ?
    SavedR14                dq      ?
    SavedR15                dq      ?

    ;
    ; The return address of our caller is next, followed by home parameter
    ; space provided for rcx-r9.
    ;

    ReturnAddress           dq      ?
    HomeRcx                 dq      ?
    HomeRdx                 dq      ?
    HomeR8                  dq      ?
    HomeR9                  dq      ?

Locals ends

LOCALS_SIZE  equ ((sizeof Locals) + (Locals.FirstNvRegister - (sizeof Locals)))

;
; Define supporting UNICODE_STRING and STRING structures for ModulePath and
; FunctionName respectively.
;

UNICODE_STRING struct
    Length          dw      ?
    MaximumLength   dw      ?
    Padding         dd      ?
    Buffer          dq      ?
UNICODE_STRING ends
PUNICODE_STRING typedef ptr UNICODE_STRING

STRING struct
    Length          dw      ?
    MaximumLength   dw      ?
    Padding         dd      ?
    Buffer          dq      ?
STRING ends

;
; Define the LARGE_INTEGER structure.
;

LARGE_INTEGER union
    struct
        LowPart     dw      ?
        HighPart    dw      ?
    ends
    QuadPart        dq      ?
LARGE_INTEGER ends

;
; Define the INJECTION_FUNCTIONS structure.  This encapsulates all function
; pointers available for use as part of injection.
;

INJECTION_FUNCTIONS struct
    RtlAddFunctionTable     dq      ?
    LoadLibraryExW          dq      ?
    GetProcAddress          dq      ?
    SetEvent                dq      ?
    ResetEvent              dq      ?
    GetThreadContext        dq      ?
    SetThreadContext        dq      ?
    SuspendThread           dq      ?
    ResumeThread            dq      ?
    OpenEventW              dq      ?
    CloseHandle             dq      ?
    SignalObjectAndWait     dq      ?
    WaitForSingleObjectEx   dq      ?
    OutputDebugStringA      dq      ?
    OutputDebugStringW      dq      ?
    NtQueueApcThread        dq      ?
    NtTestAlert             dq      ?
    QueueUserAPC            dq      ?
    SleepEx                 dq      ?
    ExitThread              dq      ?
    GetExitCodeThread       dq      ?
    DeviceIoControl         dq      ?
    CreateFileW             dq      ?
    CreateFileMappingW      dq      ?
    OpenFileMappingW        dq      ?
    MapViewOfFileEx         dq      ?
    FlushViewOfFile         dq      ?
    UnmapViewOfFileEx       dq      ?
    VirtualAllocEx          dq      ?
    VirtualFreeEx           dq      ?
    VirtualProtectEx        dq      ?
    VirtualQueryEx          dq      ?
INJECTION_FUNCTIONS ends

;
; Define the injection object structures.
;

INJECTION_OBJECT_EVENT struct
    Handle  dq	?
INJECTION_OBJECT_EVENT ends

INJECTION_OBJECT_FILE_MAPPING struct
    FileName                UNICODE_STRING      { }
    DesiredAccess           dd                  ?
    ShareMode               dd                  ?
    CreationDisposition     dd                  ?
    FlagsAndAttributes      dd                  ?
    FileHandle              dq                  ?
    MappingHandle           dq                  ?
    FileOffset              LARGE_INTEGER       { }
    MappingSize             LARGE_INTEGER       { }
    BaseAddress             dq                  ?
INJECTION_OBJECT_FILE_MAPPING ends

INJECTION_OBJECT_CONTEXT union
    AsEvent         INJECTION_OBJECT_EVENT          { }
    AsFileMapping   INJECTION_OBJECT_FILE_MAPPING   { }
INJECTION_OBJECT_CONTEXT ends
PINJECTION_OBJECT_CONTEXT typedef ptr INJECTION_OBJECT_CONTEXT

INJECTION_OBJECT_TYPE typedef dd
PINJECTION_OBJECT_TYPE typedef ptr INJECTION_OBJECT_TYPE

INJECTION_OBJECT struct
    Name        PUNICODE_STRING             ?
    ObjectType  PINJECTION_OBJECT_TYPE      ?
    Context     PINJECTION_OBJECT_CONTEXT   ?
    Unused      dq                          ?
INJECTION_OBJECT ends
PINJECTION_OBJECT typedef ptr INJECTION_OBJECT

INJECTION_OBJECTS struct
    StructSizeInBytes                       dw                          ?
    NumberOfObjects                         dw                          ?
    SizeOfInjectionObjectInBytes            dw                          ?
    SizeOfInjectionObjectContextInBytes     dw                          ?
    TotalAllocSizeInBytes                   dd                          ?
    Flags                                   dd                          ?
    Objects                                 PINJECTION_OBJECT           ?
    Names                                   PUNICODE_STRING             ?
    Types                                   PINJECTION_OBJECT_TYPE      ?
    Contexts                                PINJECTION_OBJECT_CONTEXT   ?
    Errors                                  dq                          ?
INJECTION_OBJECTS ends
PINJECTION_OBJECTS typedef ptr INJECTION_OBJECT

;
; Define helper typedefs for structures that are nicer to work with in assembly
; than their long uppercase names.
;

Object typedef INJECTION_OBJECT
Objects typedef INJECTION_OBJECTS
Functions typedef INJECTION_FUNCTIONS

;
; Define the RTL_INJECTION_THUNK_CONTEXT structure.
;

Thunk struct
    Flags                   dd                      ?
    EntryCount              dw                      ?
    UserDataOffset          dw                      ?
    FunctionTable           dq                      ?
    BaseCodeAddress         dq                      ?
    InjectionFunctions      INJECTION_FUNCTIONS     { }
    InjectionObjects        PINJECTION_OBJECTS      ?
    ModulePath              UNICODE_STRING          { }
    FunctionName            STRING                  { }
Thunk ends

;
; Define thunk flags.
;

DebugBreakOnEntry           equ     1
HasInjectionObjects         equ     2
HasModuleAndFunction        equ     4

;
; Define injection object types.
;

EventObjectType             equ     1
FileMappingObjectType       equ     2

;
; Define error codes.
;

RtlAddFunctionTableFailed   equ     1000
LoadLibraryWFailed          equ     1001
GetProcAddressFailed        equ     1002
InvalidInjectionContext     equ     1003

;++
;
; LONG
; InjectionThunk(
;     _In_ PRTL_INJECTION_THUNK_CONTEXT Thunk
;     );
;
; Routine Description:
;
;   This routine is the initial entry point of our injection logic.  That is,
;   newly created remoted threads in a target process have their start address
;   set to a copy of this routine (that was prepared in a separate process and
;   then injected via WriteProcessMemory()).
;
;   It is responsible for registering a runtime function for itself, such that
;   appropriate unwind data can be found by the kernel if an exception occurs
;   and the stack is being unwound.  It then loads a library designated by the
;   fully qualified path in the thunk's module path field via LoadLibraryW,
;   then calls GetProcAddress on the returned module for the function name also
;   contained within the thunk.  If an address is successfully resolved, the
;   routine is called with the thunk back as the first parameter, and the return
;   value is propagated back to our caller (typically, this will be the routine
;   kernel32!UserBaseInitThunk).
;
;   In practice, the module path we attempt to load is InjectionThunk.dll, and
;   the function name we resolve is "InjectionRemoteThreadEntry".  This routine
;   is responsible for doing more heavy lifting in C prior to calling the actual
;   caller's end routine.
;
; Arguments:
;
;   Thunk (rcx) - Supplies a pointer to the injection context thunk.
;
; Return Value:
;
;   If an error occured in this routine, an error code (see above) will be
;   returned (ranging in value from -1 to -3).  If this routine succeeded,
;   the return value of the function we were requested to execute will be
;   returned instead.  (Unfortunately, there's no guarantee that this won't
;   overlap with our error codes.)
;
;   This return value will end up as the exit code for the thread if being
;   called in the injection context.
;
;--

        NESTED_ENTRY InjectionThunk, _TEXT$00

;
; Thunk prologue.  Push all non-volatile registers to the stack, allocate
; space for our locals structure and assign a frame pointer.
;

;
; xxx: change this to alloc_stack first, then set_frames, then adjust epilog.
;

        rex_push_reg    rbp
        push_reg        rbx
        push_reg        rdi
        push_reg        rsi
        push_reg        r12
        push_reg        r13
        push_reg        r14
        push_reg        r15
        alloc_stack LOCALS_SIZE

        END_PROLOGUE

;
; Home our Thunk (rcx) parameter register, then save in r12.  The homing of rcx
; isn't technically necessary (as we never re-load it from rcx), but it doesn't
; hurt, and it is useful during development and debugging to help detect certain
; anomalies (like clobbering r12 accidentally, for example).
;

        mov     Locals.HomeRcx[rsp], rcx            ; Home Thunk (rcx) param.
        mov     r12, rcx                            ; Move Thunk into r12.

;
; Check to see if the DebugBreakOnEntry flag is set in the thunk flags.  If it
; is, break.
;

        movsx   r8, word ptr Thunk.Flags[r12]   ; Move flags into r8.
        test    r8, DebugBreakOnEntry           ; Test for debugbreak flag.
        jz      @F                              ; Flag isn't set.
        int     3                               ; Flag is set, so break.

;
; Register a runtime function for this currently executing piece of code.  This
; must be the first step taken by this routine in order to ensure the kernel has
; the appropriate unwind info in place if it needs to unwind this frame.
;

@@:     mov     rcx, Thunk.FunctionTable[r12]           ; Load FunctionTable.
        movsx   rdx, word ptr Thunk.EntryCount[r12]     ; Load EntryCount.
        mov     r8, Thunk.BaseCodeAddress[r12]          ; Load BaseCodeAddress.
        mov     r13, Thunk.InjectionFunctions[r12]      ; Load functions.
        call    Functions.RtlAddFunctionTable[r13]      ; Invoke function.
        test    rax, rax                                ; Check result.
        jz      short @F                                ; Function failed.
        jmp     short Inj10                             ; Function succeeded.

@@:     int     3
        mov     rax, RtlAddFunctionTableFailed          ; Load error code.
        jmp     short Inj90                             ; Jump to epilogue.

;
; Check to see if the thunk has any injection objects.  If it doesn't, proceed
; to the test to see if it has a module path and function name.
;

Inj10:  movsx   r8, word ptr Thunk.Flags[r12]           ; Move flags into r8d.
        test    r8, HasInjectionObjects                 ; Any injection objects?
        jnz     short Inj60                             ; No; check mod+func.

;
; The injection thunk features one or more injection objects.  An object can be
; an event or a file mapping.  We loop through the objects and process each one
; according to its type.
;
; N.B. Register use for this loop is as follows:
;
;
;   r14 - Supplies the address of the INJECTION_OBJECTS structure.
;
;   r15 - Supplies the value of the number of injection objects indicated by
;       the INJECTION_OBJECTS.NumberOfObjects field.
;
;   rdi - Supplies the address of the INJECTION_OBJECT structure currently
;       being processed.
;
;   rsi - Supplies the current loop index.  This is initialized to 0, and will
;       have a maximum value of NumberOfObjects-1.
;
; N.B. Erroneous conditions are handled by an immediate `int 3` breakpoint.
;

;
; Load the Thunk->InjectionObjects pointer into r14.  Verify that it is not
; NULL, then load the count into r15 and verify it is > 0.  Finally, verify the
; size of the individual injection object matches our sizeof against the struct.
; Break on any errors.
;

        mov     r14, Thunk.InjectionObjects[r12]        ; Load pointer into r14.
        test    rdi, rdi                                ; Check not-NULL.
        jnz     short @F                                ; Pointer is valid.
        int     3                                       ; Break because NULL ptr

@@:     mov     r15, Objects.NumberOfObjects[r12]       ; Load # of objects.
        test    r15, r15                                ; At least 1 objects?
        jnz     short @F                                ; Yes, continue.
        int     3                                       ; No, break.

@@:     movsx   rax, word ptr Objects.SizeOfInjectionObjectInBytes[r14]
        cmp     rax, (sizeof INJECTION_OBJECT)          ; Compare object size.
        je      short @F                                ; Sizes are equal, cont.
        int     3                                       ; Sizes != equal, break.

@@:     movsx   rax, word ptr Objects.SizeOfInjectionObjectContextInBytes[r14]
        cmp     rax, (sizeof INJECTION_OBJECT_CONTEXT)  ; Compare ctx size.
        je      short @F                                ; Sizes are equal, cont.
        int     3                                       ; Sizes != equal, break.

;
; Invariant checks passed.
;

@@:

;
; Verify that the number of objects
;

@@:     mov

;
; The pointer to the base address of the array of injection objects is valid.
; Initialize the loop
;

Inj15:

;
; Check to see if we've been requested to load a module and resolve a function.
;

Inj60:  movsx   r8, word ptr Thunk.Flags[r12]           ; Move flags into r8d.
        test    r8, HasModuleAndFunction                ; Has module+func?
        jz      Inj90                                   ; No; jump to epilogue.

;
; Prepare for a LoadLibraryW() call against the module path in the thunk.
;

        mov     rcx, Thunk.ModulePath.Buffer[r12]       ; Load ModulePath.
        call    Functions.LoadLibraryExW[r13]           ; Call LoadLibraryExW().
        test    rax, rax                                ; Check Handle != NULL.
        jz      short @F                                ; Handle is NULL.
        jmp     short Inj70                             ; Handle is valid.

@@:     mov     rax, LoadLibraryWFailed                 ; Load error code.
        jmp     short Inj90                             ; Jump to epilogue.

;
; Module was loaded successfully.  The Handle value lives in rax.  Save a copy
; in the thunk, then prepare arguments for a call to GetProcAddress().
;

Inj70:  mov     rcx, rax                                ; Load as 1st param.
        mov     rdx, Thunk.FunctionName.Buffer[r12]     ; Load name as 2nd.
        call    Functions.GetProcAddress[r13]           ; Call GetProcAddress().
        test    rax, rax                                ; Check return value.
        jz      short @F                                ; Lookup failed.
        jmp     short Inj80                             ; Lookup succeeded.

@@:     mov     rax, GetProcAddressFailed               ; Load error code.
        jmp     short Inj90                             ; Jump to return.

;
; The function name was resolved successfully.  The function address lives in
; rax.  Load the offset of the user's data buffer and calculate the address
; based on the base address of the thunk.  Call the user's function with that
; address.
;

Inj80:  movsx   r8, word ptr Thunk.UserDataOffset[r12]  ; Load offset.
        mov     rcx, r12                                ; Load base address.
        add     rcx, r8                                 ; Add offset.
        call    rax                                     ; Call the function.

;
; Intentional follow-on to Inj90 to exit the function; rax will be returned back
; to the caller.
;

Inj90:

;
; Begin epilogue.  Deallocate stack space and restore non-volatile registers,
; then return.
;

        add     rsp, LOCALS_SIZE
        pop     r15
        pop     r14
        pop     r13
        pop     r12
        pop     rsi
        pop     rdi
        pop     rbx
        pop     rbp

        ret

        NESTED_END InjectionThunk, _TEXT$00

; vim:set tw=80 ts=8 sw=4 sts=4 et syntax=masm fo=croql comments=:;            :

end
