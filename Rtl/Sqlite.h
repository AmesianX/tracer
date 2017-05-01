/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    Sqlite.h

Abstract:

    WIP.

--*/

#include "stdafx.h"
#include "sqlite3.h"

//
// Define primitive SQLITE3 typedefs.
//

typedef sqlite3_int64 SQLITE3_INT64;
typedef SQLITE3_INT64 *PSQLITE3_INT64;

typedef sqlite3_uint64 SQLITE3_UINT64;
typedef SQLITE3_UINT64 *PSQLITE3_UINT64;

//
// N.B.: We use SQLITE3 for the sqlite3_api_routines structure; sqlite3 is
//       referred to as SQLITE3_DB.  The rest of the typedefs mirror their
//       sqlite3 counterparts.
//

typedef sqlite3 SQLITE3_DB;
typedef SQLITE3_DB *PSQLITE3_DB;

typedef struct _SQLITE3_VTAB {
    const struct _SQLITE3_MODULE *Module;
    LONG NumberOfOpenCursors;
    LONG Padding;
    PCSZ ErrorMessage;
} SQLITE3_VTAB;
typedef SQLITE3_VTAB *PSQLITE3_VTAB;

typedef struct _SQLITE3_VTAB_CURSOR {
    PSQLITE3_VTAB VirtualTable;
} SQLITE3_VTAB_CURSOR;
typedef SQLITE3_VTAB_CURSOR *PSQLITE3_VTAB_CURSOR;

typedef enum _SQLITE3_CONSTRAINT_OPERATOR {
    EqualConstraint                    = 2,
    GreaterThanConstraint              = 4,
    LessThanOrEqualConstraint          = 8,
    LessThanConstraint                 = 16,
    GreaterThanOrEqualConstraint       = 32,
    MatchConstraint                    = 64,
    LikeConstraint                     = 65,
    GlobConstraint                     = 66,
    RegExpConstraint                   = 67
} SQLITE3_CONSTRAINT_OPERATOR;

#define IsEqualConstraint(Op) ((UCHAR)Op == (UCHAR)EqualConstraint)
#define IsGreaterThanConstraint(Op) ((UCHAR)Op == (UCHAR)GreaterThanConstraint)
#define IsLessThanOrEqualConstraint(Op) ((UCHAR)Op == (UCHAR)LessThanOrEqualConstraint)
#define IsLessThanConstraint(Op) ((UCHAR)Op == (UCHAR)LessThanConstraint)
#define IsGreaterThanOrEqualConstraint(Op) ((UCHAR)Op == (UCHAR)GreaterThanOrEqualConstraint)
#define IsMatchConstraint(Op) ((UCHAR)Op == (UCHAR)MatchConstraint)
#define IsLikeConstraint(Op) ((UCHAR)Op == (UCHAR)LikeConstraint)
#define IsGlobConstraint(Op) ((UCHAR)Op == (UCHAR)GlobConstraint)
#define IsRegExpConstraint(Op) ((UCHAR)Op == (UCHAR)RegExpConstraint)

typedef struct _SQLITE3_INDEX_CONSTRAINT {
    LONG ColumnNumber;
    UCHAR Op;
    UCHAR Usable;
    SHORT Padding;
    LONG TermOffset;
} SQLITE3_INDEX_CONSTRAINT;
typedef SQLITE3_INDEX_CONSTRAINT *PSQLITE3_INDEX_CONSTRAINT;

typedef struct _SQLITE3_INDEX_ORDER_BY {
    LONG ColumnNumber;
    UCHAR IsDescending;
    UCHAR Padding[3];
} SQLITE3_INDEX_ORDER_BY;
typedef SQLITE3_INDEX_ORDER_BY *PSQLITE3_INDEX_ORDER_BY;

typedef struct _SQLITE3_INDEX_CONSTRAINT_USAGE {
    LONG ArgumentIndex;
    UCHAR Omit;
    UCHAR Padding[3];
} SQLITE3_INDEX_CONSTRAINT_USAGE;
typedef SQLITE3_INDEX_CONSTRAINT_USAGE *PSQLITE3_INDEX_CONSTRAINT_USAGE;

typedef struct _SQLITE3_INDEX_INFO {

    //
    // Inputs.
    //

    LONG NumberOfConstraints;
    LONG Padding1;
    PSQLITE3_INDEX_CONSTRAINT Constraints;

    LONG NumberOfOrderBys;
    LONG Padding2;
    PSQLITE3_INDEX_ORDER_BY OrderBys;

    //
    // Outputs.
    //

    PSQLITE3_INDEX_CONSTRAINT_USAGE ConstraintUsage;

    LONG IndexNumber;
    LONG Padding3;

    PSZ IndexString;

    LONG NeedToFreeIndexString;
    LONG OrderByConsumed;

    DOUBLE EstimatedCost;

    SQLITE3_INT64 EstimatedRows;

    LONG IndexFlags;
    LONG Padding4;

    SQLITE3_UINT64 ColumnsUsedMask;

} SQLITE3_INDEX_INFO;
typedef SQLITE3_INDEX_INFO *PSQLITE3_INDEX_INFO;
C_ASSERT(sizeof(struct sqlite3_index_info) == sizeof(SQLITE3_INDEX_INFO));

typedef sqlite3_context SQLITE3_CONTEXT;
typedef SQLITE3_CONTEXT *PSQLITE3_CONTEXT;

typedef sqlite3_value SQLITE3_VALUE;
typedef SQLITE3_VALUE *PSQLITE3_VALUE;

typedef
VOID
(SQLITE3_FUNCTION)(
    _In_ PSQLITE3_CONTEXT Context,
    _In_ LONG ArgumentNumber,
    _Outptr_ PSQLITE3_VALUE Value
    );
typedef SQLITE3_FUNCTION *PSQLITE3_FUNCTION;

//
// Define SQLITE3_MODULE function pointer typedefs.
//

typedef
LONG
(SQLITE3_CREATE_OR_CONNECT)(
    _In_ PSQLITE3_DB Database,
    _In_ PVOID Aux,
    _In_ LONG NumberOfArguments,
    _In_ PCSZ Arguments,
    _Outptr_ PSQLITE3_VTAB *VirtualTable
    );
typedef SQLITE3_CREATE_OR_CONNECT *PSQLITE3_CREATE_OR_CONNECT;
typedef SQLITE3_CREATE_OR_CONNECT SQLITE3_CREATE;
typedef SQLITE3_CREATE *PSQLITE3_CREATE;
typedef SQLITE3_CREATE_OR_CONNECT SQLITE3_CONNECT;
typedef SQLITE3_CONNECT *PSQLITE3_CONNECT;

typedef
LONG
(SQLITE3_BEST_INDEX)(
    _In_ PSQLITE3_VTAB VirtualTable,
    _Inout_ PSQLITE3_INDEX_INFO IndexInfo
    );
typedef SQLITE3_BEST_INDEX *PSQLITE3_BEST_INDEX;

typedef
LONG
(SQLITE3_DISCONNECT)(
    _In_ PSQLITE3_VTAB VirtualTable
    );
typedef SQLITE3_DISCONNECT *PSQLITE3_DISCONNECT;

typedef
LONG
(SQLITE3_DESTROY)(
    _In_ PSQLITE3_VTAB VirtualTable
    );
typedef SQLITE3_DESTROY *PSQLITE3_DESTROY;

typedef
LONG
(SQLITE3_OPEN)(
    _In_ PSQLITE3_VTAB VirtualTable,
    _Inout_ PSQLITE3_VTAB_CURSOR *Cursor
    );
typedef SQLITE3_OPEN *PSQLITE3_OPEN;

typedef
LONG
(SQLITE3_CLOSE)(
    _In_ PSQLITE3_VTAB_CURSOR Cursor
    );
typedef SQLITE3_CLOSE *PSQLITE3_CLOSE;

typedef
LONG
(SQLITE3_FILTER)(
    _In_ PSQLITE3_VTAB_CURSOR Cursor,
    _In_ LONG IndexNumber,
    _In_ PCSZ IndexString,
    _In_ LONG ArgumentCount,
    _In_ PSQLITE3_VALUE *Arguments
    );
typedef SQLITE3_FILTER *PSQLITE3_FILTER;

typedef
LONG
(SQLITE3_NEXT)(
    _In_ PSQLITE3_VTAB_CURSOR Cursor
    );
typedef SQLITE3_NEXT *PSQLITE3_NEXT;

typedef
LONG
(SQLITE3_EOF)(
    _In_ PSQLITE3_VTAB_CURSOR Cursor
    );
typedef SQLITE3_EOF *PSQLITE3_EOF;

typedef
LONG
(SQLITE3_COLUMN)(
    _In_ PSQLITE3_VTAB_CURSOR Cursor,
    _In_ PSQLITE3_CONTEXT Context,
    _In_ LONG ColumnNumber
    );
typedef SQLITE3_COLUMN *PSQLITE3_COLUMN;

typedef
LONG
(SQLITE3_ROWID)(
    _In_ PSQLITE3_VTAB_CURSOR Cursor,
    _Inout_ PSQLITE3_INT64 *Rowid
    );
typedef SQLITE3_ROWID *PSQLITE3_ROWID;

typedef
LONG
(SQLITE3_UPDATE)(
    _In_ PSQLITE3_VTAB VirtualTable,
    _In_ LONG Unknown1,
    _Inout_ PSQLITE3_VALUE *Value,
    _Inout_ PSQLITE3_INT64 Unknown2
    );
typedef SQLITE3_UPDATE *PSQLITE3_UPDATE;

typedef
LONG
(SQLITE3_BEGIN)(
    _In_ PSQLITE3_VTAB VirtualTable
    );
typedef SQLITE3_BEGIN *PSQLITE3_BEGIN;

typedef
LONG
(SQLITE3_SYNC)(
    _In_ PSQLITE3_VTAB VirtualTable
    );
typedef SQLITE3_SYNC *PSQLITE3_SYNC;

typedef
LONG
(SQLITE3_COMMIT)(
    _In_ PSQLITE3_VTAB VirtualTable
    );
typedef SQLITE3_COMMIT *PSQLITE3_COMMIT;

typedef
LONG
(SQLITE3_ROLLBACK)(
    _In_ PSQLITE3_VTAB VirtualTable
    );
typedef SQLITE3_ROLLBACK *PSQLITE3_ROLLBACK;

typedef
LONG
(SQLITE3_FIND_FUNCTION)(
    _In_ PSQLITE3_VTAB VirtualTable,
    _In_ LONG ArgumentNumber,
    _In_ PCSZ FunctionName,
    _Outptr_ PSQLITE3_FUNCTION Function,
    _Outptr_ PPVOID Argument
    );
typedef SQLITE3_FIND_FUNCTION *PSQLITE3_FIND_FUNCTION;

typedef
LONG
(SQLITE3_RENAME)(
    _In_ PSQLITE3_VTAB VirtualTable,
    _In_ PCSZ NewName
    );
typedef SQLITE3_RENAME *PSQLITE3_RENAME;

typedef
LONG
(SQLITE3_SAVEPOINT)(
    _In_ PSQLITE3_VTAB VirtualTable,
    _In_ LONG Unknown
    );
typedef SQLITE3_SAVEPOINT *PSQLITE3_SAVEPOINT;

typedef
LONG
(SQLITE3_RELEASE)(
    _In_ PSQLITE3_VTAB VirtualTable,
    _In_ LONG Unknown
    );
typedef SQLITE3_RELEASE *PSQLITE3_RELEASE;

typedef
LONG
(SQLITE3_ROLLBACK_TO)(
    _In_ PSQLITE3_VTAB VirtualTable,
    _In_ LONG Unknown
    );
typedef SQLITE3_ROLLBACK_TO *PSQLITE3_ROLLBACK_TO;

//
// Define SQLITE3_MODULE structure.
//

typedef struct _SQLITE3_MODULE {
    LONG Version;
    ULONG Padding;

    PSQLITE3_CREATE Create;
    PSQLITE3_CONNECT Connect;
    PSQLITE3_BEST_INDEX BestIndex;
    PSQLITE3_DISCONNECT Disconnect;
    PSQLITE3_DESTROY Destroy;
    PSQLITE3_OPEN Open;
    PSQLITE3_CLOSE Close;
    PSQLITE3_FILTER Filter;
    PSQLITE3_NEXT Next;
    PSQLITE3_EOF Eof;
    PSQLITE3_COLUMN Column;
    PSQLITE3_ROWID Rowid;
    PSQLITE3_UPDATE Update;
    PSQLITE3_BEGIN Begin;
    PSQLITE3_SYNC Sync;
    PSQLITE3_COMMIT Commit;
    PSQLITE3_ROLLBACK Rollback;
    PSQLITE3_FIND_FUNCTION FindFunction;
    PSQLITE3_RENAME Rename;
    PSQLITE3_SAVEPOINT Savepoint;
    PSQLITE3_RELEASE Release;
    PSQLITE3_ROLLBACK_TO RollbackTo;

} SQLITE3_MODULE;
typedef SQLITE3_MODULE *PSQLITE3_MODULE;
typedef const SQLITE3_MODULE *PCSQLITE3_MODULE;

//
// Define sqlite3 API function pointer typedefs prior to SQLITE3_API_ROUTINE.
//
// (That structure is huge, so we're defining typedefs in a piecemeal fashion
//  as we use them.)
//

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID
(SQLITE3_MALLOC)(
    _In_ LONG Size
    );
typedef SQLITE3_MALLOC *PSQLITE3_MALLOC;

typedef
VOID
(SQLITE3_FREE)(
    _Pre_maybenull_ _Post_invalid_ PVOID Buffer
    );
typedef SQLITE3_FREE *PSQLITE3_FREE;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(NewSize)
PVOID
(SQLITE3_REALLOC)(
    _In_ PVOID Buffer,
    _In_ LONG NewSize
    );
typedef SQLITE3_REALLOC *PSQLITE3_REALLOC;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID
(SQLITE3_MALLOC64)(
    _In_ SQLITE3_UINT64 Size,
    );
typedef SQLITE3_MALLOC64 *PSQLITE3_MALLOC64;

typedef
_Check_return_
_Ret_maybenull_
_Post_writable_byte_size_(NewSize)
PVOID
(SQLITE3_REALLOC64)(
    _In_ PVOID Buffer,
    _In_ SQLITE3_UINT64 NewSize
    );
typedef SQLITE3_REALLOC64 *PSQLITE3_REALLOC64;

typedef
VOID
(SQLITE3_DESTROY_MODULE)(
    _In_opt_ PVOID Context
    );
typedef SQLITE3_DESTROY_MODULE *PSQLITE3_DESTROY_MODULE;

typedef
LONG
(SQLITE3_CREATE_MODULE)(
    _In_ PSQLITE3_DB Database,
    _In_ PCSZ ModuleName,
    _In_ PCSQLITE3_MODULE Module
    );
typedef SQLITE3_CREATE_MODULE *PSQLITE3_CREATE_MODULE;

typedef
LONG
(SQLITE3_CREATE_MODULE_V2)(
    _In_ PSQLITE3_DB Database,
    _In_ PCSZ ModuleName,
    _In_ PCSQLITE3_MODULE Module,
    _In_opt_ PVOID ClientData,
    _In_ PSQLITE3_DESTROY_MODULE DestroyModule
    );
typedef SQLITE3_CREATE_MODULE_V2 *PSQLITE3_CREATE_MODULE_V2;

//
// Define the SQLITE3_API_ROUTINES structure.
//

typedef struct _SQLITE3_API_ROUTINES {
    void * (*aggregate_context)(sqlite3_context*,int nBytes);
    int  (*aggregate_count)(sqlite3_context*);
    int  (*bind_blob)(sqlite3_stmt*,int,const void*,int n,void(*)(void*));
    int  (*bind_double)(sqlite3_stmt*,int,double);
    int  (*bind_int)(sqlite3_stmt*,int,int);
    int  (*bind_int64)(sqlite3_stmt*,int,sqlite_int64);
    int  (*bind_null)(sqlite3_stmt*,int);
    int  (*bind_parameter_count)(sqlite3_stmt*);
    int  (*bind_parameter_index)(sqlite3_stmt*,const char*zName);
    const char * (*bind_parameter_name)(sqlite3_stmt*,int);
    int  (*bind_text)(sqlite3_stmt*,int,const char*,int n,void(*)(void*));
    int  (*bind_text16)(sqlite3_stmt*,int,const void*,int,void(*)(void*));
    int  (*bind_value)(sqlite3_stmt*,int,const sqlite3_value*);
    int  (*busy_handler)(sqlite3*,int(*)(void*,int),void*);
    int  (*busy_timeout)(sqlite3*,int ms);
    int  (*changes)(sqlite3*);
    int  (*close)(sqlite3*);
    int  (*collation_needed)(sqlite3*,void*,void(*)(void*,sqlite3*,
                           int eTextRep,const char*));
    int  (*collation_needed16)(sqlite3*,void*,void(*)(void*,sqlite3*,
                             int eTextRep,const void*));
    const void * (*column_blob)(sqlite3_stmt*,int iCol);
    int  (*column_bytes)(sqlite3_stmt*,int iCol);
    int  (*column_bytes16)(sqlite3_stmt*,int iCol);
    int  (*column_count)(sqlite3_stmt*pStmt);
    const char * (*column_database_name)(sqlite3_stmt*,int);
    const void * (*column_database_name16)(sqlite3_stmt*,int);
    const char * (*column_decltype)(sqlite3_stmt*,int i);
    const void * (*column_decltype16)(sqlite3_stmt*,int);
    double  (*column_double)(sqlite3_stmt*,int iCol);
    int  (*column_int)(sqlite3_stmt*,int iCol);
    sqlite_int64  (*column_int64)(sqlite3_stmt*,int iCol);
    const char * (*column_name)(sqlite3_stmt*,int);
    const void * (*column_name16)(sqlite3_stmt*,int);
    const char * (*column_origin_name)(sqlite3_stmt*,int);
    const void * (*column_origin_name16)(sqlite3_stmt*,int);
    const char * (*column_table_name)(sqlite3_stmt*,int);
    const void * (*column_table_name16)(sqlite3_stmt*,int);
    const unsigned char * (*column_text)(sqlite3_stmt*,int iCol);
    const void * (*column_text16)(sqlite3_stmt*,int iCol);
    int  (*column_type)(sqlite3_stmt*,int iCol);
    sqlite3_value* (*column_value)(sqlite3_stmt*,int iCol);
    void * (*commit_hook)(sqlite3*,int(*)(void*),void*);
    int  (*complete)(const char*sql);
    int  (*complete16)(const void*sql);
    int  (*create_collation)(sqlite3*,const char*,int,void*,
                           int(*)(void*,int,const void*,int,const void*));
    int  (*create_collation16)(sqlite3*,const void*,int,void*,
                             int(*)(void*,int,const void*,int,const void*));
    int  (*create_function)(sqlite3*,const char*,int,int,void*,
                          void (*xFunc)(sqlite3_context*,int,sqlite3_value**),
                          void (*xStep)(sqlite3_context*,int,sqlite3_value**),
                          void (*xFinal)(sqlite3_context*));
    int  (*create_function16)(sqlite3*,const void*,int,int,void*,
                            void (*xFunc)(sqlite3_context*,int,sqlite3_value**),
                            void (*xStep)(sqlite3_context*,int,sqlite3_value**),
                            void (*xFinal)(sqlite3_context*));

    PSQLITE3_CREATE_MODULE CreateModule;

    int  (*data_count)(sqlite3_stmt*pStmt);
    sqlite3 * (*db_handle)(sqlite3_stmt*);
    int (*declare_vtab)(sqlite3*,const char*);
    int  (*enable_shared_cache)(int);
    int  (*errcode)(sqlite3*db);
    const char * (*errmsg)(sqlite3*);
    const void * (*errmsg16)(sqlite3*);
    int  (*exec)(sqlite3*,const char*,sqlite3_callback,void*,char**);
    int  (*expired)(sqlite3_stmt*);
    int  (*finalize)(sqlite3_stmt*pStmt);

    PSQLITE3_FREE Free;

    void  (*free_table)(char**result);
    int  (*get_autocommit)(sqlite3*);
    void * (*get_auxdata)(sqlite3_context*,int);
    int  (*get_table)(sqlite3*,const char*,char***,int*,int*,char**);
    int  (*global_recover)(void);
    void  (*interruptx)(sqlite3*);
    sqlite_int64  (*last_insert_rowid)(sqlite3*);
    const char * (*libversion)(void);
    int  (*libversion_number)(void);

    PSQLITE3_MALLOC Malloc;

    char * (*mprintf)(const char*,...);
    int  (*open)(const char*,sqlite3**);
    int  (*open16)(const void*,sqlite3**);
    int  (*prepare)(sqlite3*,const char*,int,sqlite3_stmt**,const char**);
    int  (*prepare16)(sqlite3*,const void*,int,sqlite3_stmt**,const void**);
    void * (*profile)(sqlite3*,void(*)(void*,const char*,sqlite_uint64),void*);
    void  (*progress_handler)(sqlite3*,int,int(*)(void*),void*);
    void *(*realloc)(void*,int);
    int  (*reset)(sqlite3_stmt*pStmt);
    void  (*result_blob)(sqlite3_context*,const void*,int,void(*)(void*));
    void  (*result_double)(sqlite3_context*,double);
    void  (*result_error)(sqlite3_context*,const char*,int);
    void  (*result_error16)(sqlite3_context*,const void*,int);
    void  (*result_int)(sqlite3_context*,int);
    void  (*result_int64)(sqlite3_context*,sqlite_int64);
    void  (*result_null)(sqlite3_context*);
    void  (*result_text)(sqlite3_context*,const char*,int,void(*)(void*));
    void  (*result_text16)(sqlite3_context*,const void*,int,void(*)(void*));
    void  (*result_text16be)(sqlite3_context*,const void*,int,void(*)(void*));
    void  (*result_text16le)(sqlite3_context*,const void*,int,void(*)(void*));
    void  (*result_value)(sqlite3_context*,sqlite3_value*);
    void * (*rollback_hook)(sqlite3*,void(*)(void*),void*);
    int  (*set_authorizer)(sqlite3*,int(*)(void*,int,const char*,const char*,
                         const char*,const char*),void*);
    void  (*set_auxdata)(sqlite3_context*,int,void*,void (*)(void*));
    char * (*snprintf)(int,char*,const char*,...);
    int  (*step)(sqlite3_stmt*);
    int  (*table_column_metadata)(sqlite3*,const char*,const char*,const char*,
                                char const**,char const**,int*,int*,int*);
    void  (*thread_cleanup)(void);
    int  (*total_changes)(sqlite3*);
    void * (*trace)(sqlite3*,void(*xTrace)(void*,const char*),void*);
    int  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
    void * (*update_hook)(sqlite3*,void(*)(void*,int ,char const*,char const*,
                                         sqlite_int64),void*);
    void * (*user_data)(sqlite3_context*);
    const void * (*value_blob)(sqlite3_value*);
    int  (*value_bytes)(sqlite3_value*);
    int  (*value_bytes16)(sqlite3_value*);
    double  (*value_double)(sqlite3_value*);
    int  (*value_int)(sqlite3_value*);
    sqlite_int64  (*value_int64)(sqlite3_value*);
    int  (*value_numeric_type)(sqlite3_value*);
    const unsigned char * (*value_text)(sqlite3_value*);
    const void * (*value_text16)(sqlite3_value*);
    const void * (*value_text16be)(sqlite3_value*);
    const void * (*value_text16le)(sqlite3_value*);
    int  (*value_type)(sqlite3_value*);
    char *(*vmprintf)(const char*,va_list);
    /* Added ??? */
    int (*overload_function)(sqlite3*, const char *zFuncName, int nArg);
    /* Added by 3.3.13 */
    int (*prepare_v2)(sqlite3*,const char*,int,sqlite3_stmt**,const char**);
    int (*prepare16_v2)(sqlite3*,const void*,int,sqlite3_stmt**,const void**);
    int (*clear_bindings)(sqlite3_stmt*);

    //
    // Added by 3.4.1.
    //

    PSQLITE3_CREATE_MODULE_V2 CreateModule2;

    /* Added by 3.5.0 */
    int (*bind_zeroblob)(sqlite3_stmt*,int,int);
    int (*blob_bytes)(sqlite3_blob*);
    int (*blob_close)(sqlite3_blob*);
    int (*blob_open)(sqlite3*,const char*,const char*,const char*,sqlite3_int64,
                   int,sqlite3_blob**);
    int (*blob_read)(sqlite3_blob*,void*,int,int);
    int (*blob_write)(sqlite3_blob*,const void*,int,int);
    int (*create_collation_v2)(sqlite3*,const char*,int,void*,
                             int(*)(void*,int,const void*,int,const void*),
                             void(*)(void*));
    int (*file_control)(sqlite3*,const char*,int,void*);
    sqlite3_int64 (*memory_highwater)(int);
    sqlite3_int64 (*memory_used)(void);
    sqlite3_mutex *(*mutex_alloc)(int);
    void (*mutex_enter)(sqlite3_mutex*);
    void (*mutex_free)(sqlite3_mutex*);
    void (*mutex_leave)(sqlite3_mutex*);
    int (*mutex_try)(sqlite3_mutex*);
    int (*open_v2)(const char*,sqlite3**,int,const char*);
    int (*release_memory)(int);
    void (*result_error_nomem)(sqlite3_context*);
    void (*result_error_toobig)(sqlite3_context*);
    int (*sleep)(int);
    void (*soft_heap_limit)(int);
    sqlite3_vfs *(*vfs_find)(const char*);
    int (*vfs_register)(sqlite3_vfs*,int);
    int (*vfs_unregister)(sqlite3_vfs*);
    int (*xthreadsafe)(void);
    void (*result_zeroblob)(sqlite3_context*,int);
    void (*result_error_code)(sqlite3_context*,int);
    int (*test_control)(int, ...);
    void (*randomness)(int,void*);
    sqlite3 *(*context_db_handle)(sqlite3_context*);
    int (*extended_result_codes)(sqlite3*,int);
    int (*limit)(sqlite3*,int,int);
    sqlite3_stmt *(*next_stmt)(sqlite3*,sqlite3_stmt*);
    const char *(*sql)(sqlite3_stmt*);
    int (*status)(int,int*,int*,int);
    int (*backup_finish)(sqlite3_backup*);
    sqlite3_backup *(*backup_init)(sqlite3*,const char*,sqlite3*,const char*);
    int (*backup_pagecount)(sqlite3_backup*);
    int (*backup_remaining)(sqlite3_backup*);
    int (*backup_step)(sqlite3_backup*,int);
    const char *(*compileoption_get)(int);
    int (*compileoption_used)(const char*);
    int (*create_function_v2)(sqlite3*,const char*,int,int,void*,
                            void (*xFunc)(sqlite3_context*,int,sqlite3_value**),
                            void (*xStep)(sqlite3_context*,int,sqlite3_value**),
                            void (*xFinal)(sqlite3_context*),
                            void(*xDestroy)(void*));
    int (*db_config)(sqlite3*,int,...);
    sqlite3_mutex *(*db_mutex)(sqlite3*);
    int (*db_status)(sqlite3*,int,int*,int*,int);
    int (*extended_errcode)(sqlite3*);
    void (*log)(int,const char*,...);
    sqlite3_int64 (*soft_heap_limit64)(sqlite3_int64);
    const char *(*sourceid)(void);
    int (*stmt_status)(sqlite3_stmt*,int,int);
    int (*strnicmp)(const char*,const char*,int);
    int (*unlock_notify)(sqlite3*,void(*)(void**,int),void*);
    int (*wal_autocheckpoint)(sqlite3*,int);
    int (*wal_checkpoint)(sqlite3*,const char*);
    void *(*wal_hook)(sqlite3*,int(*)(void*,sqlite3*,const char*,int),void*);
    int (*blob_reopen)(sqlite3_blob*,sqlite3_int64);
    int (*vtab_config)(sqlite3*,int op,...);
    int (*vtab_on_conflict)(sqlite3*);
    /* Version 3.7.16 and later */
    int (*close_v2)(sqlite3*);
    const char *(*db_filename)(sqlite3*,const char*);
    int (*db_readonly)(sqlite3*,const char*);
    int (*db_release_memory)(sqlite3*);
    const char *(*errstr)(int);
    int (*stmt_busy)(sqlite3_stmt*);
    int (*stmt_readonly)(sqlite3_stmt*);
    int (*stricmp)(const char*,const char*);
    int (*uri_boolean)(const char*,const char*,int);
    sqlite3_int64 (*uri_int64)(const char*,const char*,sqlite3_int64);
    const char *(*uri_parameter)(const char*,const char*);
    char *(*vsnprintf)(int,char*,const char*,va_list);
    int (*wal_checkpoint_v2)(sqlite3*,const char*,int,int*,int*);
    /* Version 3.8.7 and later */
    int (*auto_extension)(void(*)(void));
    int (*bind_blob64)(sqlite3_stmt*,int,const void*,sqlite3_uint64,
                     void(*)(void*));
    int (*bind_text64)(sqlite3_stmt*,int,const char*,sqlite3_uint64,
                      void(*)(void*),unsigned char);
    int (*cancel_auto_extension)(void(*)(void));
    int (*load_extension)(sqlite3*,const char*,const char*,char**);

    PSQLITE3_MALLOC64 Malloc64;

    sqlite3_uint64 (*msize)(void*);

    PSQLITE3_REALLOC64 Realloc64;

    void (*reset_auto_extension)(void);
    void (*result_blob64)(sqlite3_context*,const void*,sqlite3_uint64,
                        void(*)(void*));
    void (*result_text64)(sqlite3_context*,const char*,sqlite3_uint64,
                         void(*)(void*), unsigned char);
    int (*strglob)(const char*,const char*);
    /* Version 3.8.11 and later */
    sqlite3_value *(*value_dup)(const sqlite3_value*);
    void (*value_free)(sqlite3_value*);
    int (*result_zeroblob64)(sqlite3_context*,sqlite3_uint64);
    int (*bind_zeroblob64)(sqlite3_stmt*, int, sqlite3_uint64);
    /* Version 3.9.0 and later */
    unsigned int (*value_subtype)(sqlite3_value*);
    void (*result_subtype)(sqlite3_context*,unsigned int);
    /* Version 3.10.0 and later */
    int (*status64)(int,sqlite3_int64*,sqlite3_int64*,int);
    int (*strlike)(const char*,const char*,unsigned int);
    int (*db_cacheflush)(sqlite3*);
    /* Version 3.12.0 and later */
    int (*system_errno)(sqlite3*);
    /* Version 3.14.0 and later */
    int (*trace_v2)(sqlite3*,unsigned,int(*)(unsigned,void*,void*,void*),void*);
    char *(*expanded_sql)(sqlite3_stmt*);
    /* Version 3.18.0 and later */
    void (*set_last_insert_rowid)(sqlite3*,sqlite3_int64);
} SQLITE3, SQLITE3_API_ROUTINES;
typedef SQLITE3 *PSQLITE3;
typedef const SQLITE3 *PCSQLITE3;

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
