/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineCommands.c

Abstract:

    This module defines command constants.

--*/

#include "stdafx.h"

CONST UNICODE_STRING Asterisk = RTL_CONSTANT_STRING(L"*");
CONST UNICODE_STRING ExclamationPoint = RTL_CONSTANT_STRING(L"!");

//
// ExamineSymbols.
//

STRING ExamineSymbolsCommandName =
    RTL_CONSTANT_STRING("x ");

UNICODE_STRING ExamineSymbolsCommandNameWide =
    RTL_CONSTANT_STRING(L"x ");

STRING ExamineSymbolsCommandDisplayName =
    RTL_CONSTANT_STRING("examine symbols");

UNICODE_STRING ExamineSymbolsCommandOptions[] = {
    RTL_CONSTANT_STRING(L"/v "),
    RTL_CONSTANT_STRING(L"/t "),
};

DEBUG_ENGINE_COMMAND_TEMPLATE ExamineSymbolsCommandTemplate = {

    //
    // SizeOfStruct
    //

    sizeof(DEBUG_ENGINE_COMMAND_TEMPLATE),

    //
    // Flags
    //

    {
        1,      // HasOptions
        1,      // HasModuleName
        1,      // HasExclamationPoint
        0,      // MandatoryArgument
        0,      // OptionalArgument
        1,      // SymbolNameDefaultsToAsterisk,
        0,      // Unused
    },

    //
    // Type
    //

    { ExamineSymbolsCommandId },

    //
    // NumberOfOptions
    //

    ARRAYSIZE(ExamineSymbolsCommandOptions),

    //
    // Options
    //

    ExamineSymbolsCommandOptions,

    //
    // CommandName
    //

    &ExamineSymbolsCommandName,

    //
    // CommandNameWide
    //

    &ExamineSymbolsCommandNameWide,

    //
    // CommandDisplayName
    //

    &ExamineSymbolsCommandDisplayName,

    //
    // ParseLinesIntoCustomStructureCallback
    //

    ExamineSymbolsParseLinesIntoCustomStructureCallback,

};

//
// UnassembleFunction.
//

STRING UnassembleFunctionCommandName =
    RTL_CONSTANT_STRING("uf ");

UNICODE_STRING UnassembleFunctionCommandNameWide =
    RTL_CONSTANT_STRING(L"uf ");

STRING UnassembleFunctionCommandDisplayName =
    RTL_CONSTANT_STRING("unassemble function");

UNICODE_STRING UnassembleFunctionCommandOptions[] = {
    RTL_CONSTANT_STRING(L"/c "),
    RTL_CONSTANT_STRING(L"/D "),
    RTL_CONSTANT_STRING(L"/m "),
    RTL_CONSTANT_STRING(L"/o "),
    RTL_CONSTANT_STRING(L"/O "),
    RTL_CONSTANT_STRING(L"/i "),
};

DEBUG_ENGINE_COMMAND_TEMPLATE UnassembleFunctionCommandTemplate = {

    //
    // SizeOfStruct
    //

    sizeof(DEBUG_ENGINE_COMMAND_TEMPLATE),

    //
    // Flags
    //

    {
        1,      // HasOptions
        1,      // HasModuleName
        1,      // HasExclamationPoint
        1,      // MandatoryArgument
        0,      // OptionalArgument
        0,      // SymbolNameDefaultsToAsterisk,
        0,      // Unused
    },

    //
    // Type
    //

    { UnassembleFunctionCommandId },

    //
    // NumberOfOptions
    //

    ARRAYSIZE(UnassembleFunctionCommandOptions),

    //
    // Options
    //

    UnassembleFunctionCommandOptions,

    //
    // CommandName
    //

    &UnassembleFunctionCommandName,

    //
    // CommandNameWide
    //

    &UnassembleFunctionCommandNameWide,

    //
    // CommandDisplayName
    //

    &UnassembleFunctionCommandDisplayName,

    //
    // ParseLinesIntoCustomStructureCallback
    //

    UnassembleFunctionParseLinesIntoCustomStructureCallback,

};

//
// DisplayType.
//

STRING DisplayTypeCommandName =
    RTL_CONSTANT_STRING("dt ");

UNICODE_STRING DisplayTypeCommandNameWide =
    RTL_CONSTANT_STRING(L"dt ");

STRING DisplayTypeCommandDisplayName =
    RTL_CONSTANT_STRING("display type");

UNICODE_STRING DisplayTypeCommandOptions[] = {
    RTL_CONSTANT_STRING(L"-a "),    // ShowArrayElements
    RTL_CONSTANT_STRING(L"-b "),    // DisplayBlocksRecursively
    RTL_CONSTANT_STRING(L"-c "),    // CompactOutput
    RTL_CONSTANT_STRING(L"-e "),    // ForceTypeEnumeration
    RTL_CONSTANT_STRING(L"-r9 "),   // RecursivelyDumpSubtypes
    RTL_CONSTANT_STRING(L"-v "),    // Verbose
};

DEBUG_ENGINE_COMMAND_TEMPLATE DisplayTypeCommandTemplate = {

    //
    // SizeOfStruct
    //

    sizeof(DEBUG_ENGINE_COMMAND_TEMPLATE),

    //
    // Flags
    //

    {
        1,      // HasOptions
        1,      // HasModuleName
        1,      // HasExclamationPoint
        1,      // MandatoryArgument
        0,      // OptionalArgument
        0,      // SymbolNameDefaultsToAsterisk,
        0,      // Unused
    },

    //
    // Type
    //

    { DisplayTypeCommandId },

    //
    // NumberOfOptions
    //

    ARRAYSIZE(DisplayTypeCommandOptions),

    //
    // Options
    //

    DisplayTypeCommandOptions,

    //
    // CommandName
    //

    &DisplayTypeCommandName,

    //
    // CommandNameWide
    //

    &DisplayTypeCommandNameWide,

    //
    // CommandDisplayName
    //

    &DisplayTypeCommandDisplayName,

    //
    // ParseLinesIntoCustomStructureCallback
    //

    DisplayTypeParseLinesIntoCustomStructureCallback,

};

//
// SettingsMeta.
//

STRING SettingsMetaCommandName =
    RTL_CONSTANT_STRING(".settings ");

UNICODE_STRING SettingsMetaCommandNameWide =
    RTL_CONSTANT_STRING(L".settings ");

STRING SettingsMetaCommandDisplayName =
    RTL_CONSTANT_STRING(".settings");

UNICODE_STRING SettingsMetaCommandOptions[] = {
    RTL_CONSTANT_STRING(L"list "),
    RTL_CONSTANT_STRING(L"load "),
};

DEBUG_ENGINE_COMMAND_TEMPLATE SettingsMetaCommandTemplate = {

    //
    // SizeOfStruct
    //

    sizeof(DEBUG_ENGINE_COMMAND_TEMPLATE),

    //
    // Flags
    //

    {
        1,      // HasOptions
        0,      // HasModuleName
        0,      // HasExclamationPoint
        0,      // MandatoryArgument
        1,      // OptionalArgument
        0,      // SymbolNameDefaultsToAsterisk,
        0,      // Unused
    },

    //
    // Type
    //

    { SettingsMetaCommandId },

    //
    // NumberOfOptions
    //

    ARRAYSIZE(SettingsMetaCommandOptions),

    //
    // Options
    //

    SettingsMetaCommandOptions,

    //
    // CommandName
    //

    &SettingsMetaCommandName,

    //
    // CommandNameWide
    //

    &SettingsMetaCommandNameWide,

    //
    // CommandDisplayName
    //

    &SettingsMetaCommandDisplayName,

    //
    // ParseLinesIntoCustomStructureCallback
    //

    NULL,

};

//
// Array of templates.
//

PDEBUG_ENGINE_COMMAND_TEMPLATE DebugEngineCommandTemplates[] = {
    &ExamineSymbolsCommandTemplate,
    &UnassembleFunctionCommandTemplate,
    &DisplayTypeCommandTemplate,
    &SettingsMetaCommandTemplate,
};

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
