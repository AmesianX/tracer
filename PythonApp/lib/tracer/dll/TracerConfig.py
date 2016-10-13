#===============================================================================
# Imports
#===============================================================================

from ctypes import (
    cast,
    byref,

    Union,

    CDLL,
    POINTER,
    CFUNCTYPE,
)

from ..wintypes import (
    errcheck,
    create_unicode_string,

    Structure,

    BOOL,
    USHORT,
    ULONG,
    PVOID,
    SIZE_T,
    HMODULE,
    SRWLOCK,
    LIST_ENTRY,
    UNICODE_STRING,
    PUNICODE_STRING,
)

from .Allocator import (
    ALLOCATOR,
    PALLOCATOR,
    PPALLOCATOR,
    GET_OR_CREATE_GLOBAL_ALLOCATOR
)

#===============================================================================
# Globals
#===============================================================================
TRACER_REGISTRY_KEY = 'Software\\Tracer'

#===============================================================================
# Aliases
#===============================================================================

#===============================================================================
# Helpers
#===============================================================================
def get_tracer_config_dll_path():
    from ..path import join_path
    from ..util import import_winreg

    winreg = import_winreg()
    registry = winreg.ConnectRegistry(None, winreg.HKEY_CURRENT_USER)
    key = winreg.OpenKey(registry, TRACER_REGISTRY_KEY)

    load_debug = bool(winreg.QueryValueEx(key, 'LoadDebugLibraries')[0])
    if load_debug:
        intermediate = 'x64\\Debug'
    else:
        intermediate = 'x64\\Release'

    installation_dir = winreg.QueryValueEx(key, 'InstallationDirectory')[0]
    winreg.CloseKey(key)

    dll_base_directory = join_path(installation_dir, intermediate)

    path = join_path(dll_base_directory, 'TracerConfig.dll')
    return path

def load_tracer_config_dll():
    path = get_tracer_config_dll_path()
    dll = CDLL(path)
    return dll

#===============================================================================
# Structures
#===============================================================================

class TRACER_PATHS(Structure):
    _fields_ = [
        ('Size', USHORT),
        ('Padding', USHORT * 3),
        ('InstallationDirectory', UNICODE_STRING),
        ('BaseTraceDirectory', UNICODE_STRING),
        ('DefaultPythonDirectory', UNICODE_STRING),
        ('RtlDllPath', UNICODE_STRING),
        ('PythonDllPath', UNICODE_STRING),
        ('TracerHeapDllPath', UNICODE_STRING),
        ('TraceStoreDllPath', UNICODE_STRING),
        ('StringTableDllPath', UNICODE_STRING),
        ('PythonTracerDllPath', UNICODE_STRING),
        ('TlsTracerHeapDllPath', UNICODE_STRING),
        ('TracedPythonSessionDllPath', UNICODE_STRING),
    ]
PTRACER_PATHS = POINTER(TRACER_PATHS)

class TRACER_FLAGS(Structure):
    _fields_ = [
        ('DebugBreakOnEntry', ULONG, 1),
        ('LoadDebugLibraries', ULONG, 1),
        ('DisableTraceSessionDirectoryCompression', ULONG, 1),
        ('DisablePrefaultPages', ULONG, 1),
        ('EnableMemoryTracing', ULONG, 1),
        ('EnableIoCounterTracing', ULONG, 1),
        ('EnableHandleCountTracing', ULONG, 1),
        ('DisableFileFlagOverlapped', ULONG, 1),
        ('DisableFileFlagSequentialScan', ULONG, 1),
        ('EnableFileFlagRandomAccess', ULONG, 1),
        ('EnableFileFlagWriteThrough', ULONG, 1),
    ]
PTRACER_FLAGS = POINTER(TRACER_FLAGS)

class TRACE_SESSION_DIRECTORY(Structure):
    _fields_ = [
        ('ListEntry', LIST_ENTRY),
        ('Directory', UNICODE_STRING),
        ('Context', PVOID),
    ]
PTRACE_SESSION_DIRECTORY = POINTER(TRACE_SESSION_DIRECTORY)

class TRACE_SESSION_DIRECTORIES(Structure):
    _fields_ = [
        ('ListHead', LIST_ENTRY),
        ('Lock', SRWLOCK),
        ('Count', ULONG),
    ]
PTRACE_SESSION_DIRECTORIES = POINTER(TRACE_SESSION_DIRECTORIES)

class TRACER_CONFIG(Structure):
    _fields_ = [
        ('Size', USHORT),
        ('NumberOfElementsPerTraceStore', USHORT),
        ('MaximumTraceStoreId', ULONG),
        ('MaximumTraceStoreIndex', ULONG),
        ('Flags', TRACER_FLAGS),
        ('Allocator', PALLOCATOR),
        ('TlsTracerHeapModule', HMODULE),
        ('ListEntry', LIST_ENTRY),
        ('Paths', TRACER_PATHS),
        ('TraceSessionDirectories', TRACE_SESSION_DIRECTORIES),
    ]
PTRACER_CONFIG = POINTER(TRACER_CONFIG)

#===============================================================================
# Function Types
#===============================================================================

INITIALIZE_TRACER_CONFIG = CFUNCTYPE(
    PTRACER_CONFIG,
    PALLOCATOR,
    PUNICODE_STRING
)
CREATE_AND_INITIALIZE_ALLOCATOR = CFUNCTYPE(BOOL, PPALLOCATOR)
GET_OR_CREATE_GLOBAL_ALLOCATOR = CFUNCTYPE(BOOL, PPALLOCATOR)

INITIALIZE_TRACER_CONFIG.errcheck = errcheck
CREATE_AND_INITIALIZE_ALLOCATOR.errcheck = errcheck
GET_OR_CREATE_GLOBAL_ALLOCATOR.errcheck = errcheck

#===============================================================================
# Binding
#===============================================================================

TracerConfigDll = load_tracer_config_dll()

CreateAndInitializeAllocator = CREATE_AND_INITIALIZE_ALLOCATOR(
    ('CreateAndInitializeDefaultHeapAllocator', TracerConfigDll),
    ((1, 'Allocator'),),
)

GetOrCreateGlobalAllocator = GET_OR_CREATE_GLOBAL_ALLOCATOR(
    ('GetOrCreateGlobalAllocator', TracerConfigDll),
    ((1, 'Allocator'),),
)

InitializeTracerConfig = INITIALIZE_TRACER_CONFIG(
    ('InitializeTracerConfig', TracerConfigDll),
    (
        (1, 'Allocator'),
        (1, 'RegistryKey'),
    ),
)

def load_tracer_config():
    allocator = PALLOCATOR()
    CreateAndInitializeAllocator(byref(allocator))
    registry_key = create_unicode_string(TRACER_REGISTRY_KEY)

    return InitializeTracerConfig(allocator, byref(registry_key)).contents

# vim:set ts=8 sw=4 sts=4 tw=80 ai et                                          :