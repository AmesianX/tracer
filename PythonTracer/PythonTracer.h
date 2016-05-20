// Copyright(c) Trent Nelson <trent@trent.me>
// All rights reserved.

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include <Windows.h>
#include "../Rtl/Rtl.h"
#include "../Tracer/Tracer.h"
#include "../Tracer/Tracing.h"
#include "../Python/Python.h"

typedef enum _TraceEventType {
    // PyTrace_* constants.
    TraceEventType_PyTrace_CALL = 0,
    TraceEventType_PyTrace_EXCEPTION = 1,
    TraceEventType_PyTrace_LINE = 2,
    TraceEventType_PyTrace_RETURN = 3,
    TraceEventType_PyTrace_C_CALL = 4,
    TraceEventType_PyTrace_C_EXCEPTION = 5,
    TraceEventType_PyTrace_C_RETURN = 6,
} TraceEventType;

typedef struct _EVENT_TYPE {
    TraceEventType  Id;
    PCWSTR          Name;
    PCSTR           NameA;
} EVENT_TYPE, *PEVENT_TYPE;

static const EVENT_TYPE EventTypes[] = {

    {
        TraceEventType_PyTrace_CALL,
        L"PyTrace_CALL",
        "PyTrace_CALL"
    },

    {
        TraceEventType_PyTrace_EXCEPTION,
        L"PyTrace_EXCEPTION",
        "PyTrace_EXCEPTION"
    },

    {
        TraceEventType_PyTrace_LINE,
        L"PyTrace_LINE",
        "PyTrace_LINE"
    },

    {
        TraceEventType_PyTrace_RETURN,
        L"PyTrace_RETURN",
        "PyTrace_RETURN"
    },

    {
        TraceEventType_PyTrace_C_CALL,
        L"PyTrace_C_CALL",
        "PyTrace_C_CALL"
    },

    {
        TraceEventType_PyTrace_C_EXCEPTION,
        L"PyTrace_C_EXCEPTION",
        "PyTrace_C_EXCEPTION"
    },

    {
        TraceEventType_PyTrace_C_RETURN,
        L"PyTrace_C_RETURN",
        "PyTrace_C_RETURN"
    },
};

static const DWORD NumberOfTraceEventTypes = (
    sizeof(EventTypes) /
    sizeof(EVENT_TYPE)
);

typedef enum _PYTHON_TRACE_EVENT_TYPE {
    Call        =         1,
    Exception   =   1 <<  1, // 2
    Line        =   1 <<  2, // 4
    Return      =   1 <<  3, // 8
    Invalid     =   1 << 31
} PYTHON_TRACE_EVENT_TYPE, *PPYTHON_TRACE_EVENT_TYPE;

typedef struct _PYTHON_TRACE_EVENT {
    // 8 bytes
    union {
        LARGE_INTEGER Timestamp;
        LARGE_INTEGER Elapsed;
    };

    // 8 bytes
    DECLSPEC_ALIGN(8)
    PPYTHON_FUNCTION Function;

    // 4 bytes
    union {
        ULONG Flags;
        PYTHON_TRACE_EVENT_TYPE Type;
        struct {
            ULONG IsCall:1;         // PyTrace_CALL
            ULONG IsException:1;    // PyTrace_EXCEPTION
            ULONG IsLine:1;         // PyTrace_LINE
            ULONG IsReturn:1;       // PyTrace_RETURN
            ULONG IsC:1;
            ULONG IsReverseJump:1;
        };
    };

    // 4 bytes
    union {
        ULONG Padding1;
        ULONG LineNumber;
    };
} PYTHON_TRACE_EVENT, *PPYTHON_TRACE_EVENT, **PPPYTHON_TRACE_EVENT;

typedef struct _PYTHON_TRACE_CALL_EVENT {
    LIST_ENTRY ListEntry;
    union {
        LARGE_INTEGER Timestamp;
        LARGE_INTEGER Elapsed;
    };
    PPYTHON_TRACE_EVENT CallEvent;
    PPYTHON_TRACE_EVENT ReturnEvent;
} PYTHON_TRACE_CALL_EVENT, *PPYTHON_TRACE_CALL_EVENT;

//
// Forward decls.
//
typedef struct _PYTHON_TRACE_CONTEXT PYTHON_TRACE_CONTEXT;
typedef PYTHON_TRACE_CONTEXT *PPYTHON_TRACE_CONTEXT;

typedef BOOLEAN (PYTHON_TRACE_CALL)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PPYTHON_TRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef PYTHON_TRACE_CALL *PPYTHON_TRACE_CALL;

typedef BOOLEAN (PYTHON_TRACE_LINE)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PPYTHON_TRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef PYTHON_TRACE_LINE *PPYTHON_TRACE_LINE;

typedef BOOLEAN (PYTHON_TRACE_RETURN)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PPYTHON_TRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef PYTHON_TRACE_RETURN *PPYTHON_TRACE_RETURN;

typedef BOOLEAN (REGISTER_PYTHON_FUNCTION)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PPYTHON_TRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef REGISTER_PYTHON_FUNCTION *PREGISTER_PYTHON_FUNCTION;

typedef BOOLEAN (PREPARE_TRACE_EVENT)(
    _Inout_  PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_  PPYTHON_TRACE_EVENT   TraceEvent,
    _In_     PPYFRAMEOBJECT        FrameObject,
    _In_opt_ LONG                  EventType,
    _In_opt_ PPYOBJECT             ArgObject
    );
typedef PREPARE_TRACE_EVENT *PPREPARE_TRACE_EVENT;

typedef VOID (CONTINUE_TRACE_EVENT)(
    _Inout_ PPYTHON_TRACE_CONTEXT PythonTraceContext,
    _Inout_ PPYTHON_TRACE_EVENT          Event,
    _In_    PPYFRAMEOBJECT        FrameObject
    );
typedef CONTINUE_TRACE_EVENT *PCONTINUE_TRACE_EVENT;

typedef struct _PYTHON_TRACE_CONTEXT {
    ULONG             Size;
    PRTL              Rtl;
    PPYTHON           Python;
    PTRACE_CONTEXT    TraceContext;
    PPYTRACEFUNC      PythonTraceFunction;
    PVOID             UserData;

    ULONG             Depth;
    ULONG             SkipFrames;
    union {
        ULONG Flags;
        struct {
            ULONG StartedTracing:1;
        };
    };
    ULONG Unused1;

    LARGE_INTEGER Frequency;
    LARGE_INTEGER StartTimestamp;
    LARGE_INTEGER StopTimestamp;
    LARGE_INTEGER LastTimestamp;
    LARGE_INTEGER ThisTimestamp;

    PPYTHON_FUNCTION FirstFunction;

    PREFIX_TABLE ModuleFilterTable;

    LIST_ENTRY Functions;

    PPREPARE_TRACE_EVENT PrepareTraceEvent;
    PCONTINUE_TRACE_EVENT ContinueTraceEvent;

    PREGISTER_PYTHON_FUNCTION RegisterPythonFunction;

    PPYTHON_TRACE_CALL TraceCall;
    PPYTHON_TRACE_LINE TraceLine;
    PPYTHON_TRACE_RETURN TraceReturn;

} PYTHON_TRACE_CONTEXT, *PPYTHON_TRACE_CONTEXT;

TRACER_API
LONG
PyTraceCallbackDummy(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
);

TRACER_API
LONG
PyTraceCallbackBasic(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
);

TRACER_API
LONG
PyTraceCallbackFast(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
);

TRACER_API
BOOL
PyTracePrepareTraceEvent(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PPYTHON_TRACE_EVENT     TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
    );

TRACER_API
VOID
PyTraceContinueTraceEvent(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PPYTHON_TRACE_EVENT     TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );

TRACER_API
BOOL
PyTraceRegisterPythonFunction(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PPYTHON_TRACE_EVENT     TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );

TRACER_API
VOID
PyTraceCall(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PPYTHON_TRACE_EVENT     TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );

TRACER_API
VOID
PyTraceLine(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PPYTHON_TRACE_EVENT     TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );

TRACER_API
VOID
PyTraceReturn(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PPYTHON_TRACE_EVENT     TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    );


TRACER_API
BOOL
InitializePythonTraceContext(
    _In_ PRTL Rtl,
    _Out_bytecap_(*SizeOfPythonTraceContext) PPYTHON_TRACE_CONTEXT
                                             PythonTraceContext,
    _Inout_ PULONG SizeOfPythonTraceContext,
    _In_ PPYTHON Python,
    _In_ PTRACE_CONTEXT TraceContext,
    _In_opt_ PPYTRACEFUNC PythonTraceFunction,
    _In_opt_ PVOID UserData
    );

TRACER_API
BOOL
StartTracing(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    );

TRACER_API
BOOL
StopTracing(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    );

TRACER_API
BOOL
StartProfiling(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    );

TRACER_API
BOOL
StopProfiling(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    );

TRACER_API
BOOL
AddFunction(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_    PVOID                   FunctionObject
    );

TRACER_API
BOOL
AddModuleName(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_    PPYOBJECT               ModuleNameObject
    );

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
