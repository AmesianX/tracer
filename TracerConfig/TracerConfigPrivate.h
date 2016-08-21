#pragma once

#include "TracerConfig.h"

INITIALIZE_TRACER_CONFIG InitializeTracerConfig;
INITIALIZE_GLOBAL_TRACER_CONFIG InitializeGlobalTracerConfig;

DESTROY_TRACER_CONFIG DestroyTracerConfig;
DESTROY_GLOBAL_TRACER_CONFIG DestroyGlobalTracerConfig;

CREATE_TRACE_SESSION_DIRECTORY CreateTraceSessionDirectory;