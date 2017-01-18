///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2011 - 2012
//
// ԭʼ�ļ�����     : AntinvaderDef.h
// ��������         : AntinvaderDef
// ����ʱ��         : 2011-03-20
//
//
// ����             : Antinvader����������Ҫͷ�ļ�,���������;�̬���Ͷ���
//
// ����ά��:
//  0000 [2011-03-20] ����汾.
//
///////////////////////////////////////////////////////////////////////////////

// $Id$

#ifndef __ANTINVADERDRIVER_H_VERSION__
#define __ANTINVADERDRIVER_H_VERSION__ 100

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "drvCommon.h"
#include "drvVersion.h"

#include <ntdef.h>
#include <ntifs.h>

// ����Լ��
#include "Common.h"
#include "KeLog.h"

#ifndef FILE_DEVICE_ANTINVADERDRIVER
#define FILE_DEVICE_ANTINVADERDRIVER    0x8000
#endif

// ����֧��
// #if DBG

#define FILE_OBJECT_NAME_BUFFER(_file_object)       (_file_object)->FileName.Buffer

/* add by xiaogang
ͨ��PsGetCurrentProcess��������ȡ��ǰ���������Ľ��̵�EPROCESS�ṹ�ĵ�ַ.EPROCESS�ṹ��0x174ƫ�ƴ�����Ž�����.
˼·����:
��������ļ��غ���DriverEntry��������System�����еģ�
(1) ͨ��PsGetCurrentProcess���Ի�ȡSystem���̵��ں�EPROCESS�ṹ�ĵ�ַ,
(2) �Ӹõ�ַ��ʼѰ��"System"�ַ�����
(3) �ҵ��˱���EPROCESS�Ľ�������ŵ�ƫ�ƴ�,�õ���������EPROCESS�ṹ��ƫ�ƺ�,
(4) ���̵���������ʱ��,�Ϳ���ֱ���ڸ�ƫ�ƴ���ȡ��ǰ��������
*/
#define CURRENT_PROCESS_NAME_BUFFER                 ((PCHAR)PsGetCurrentProcess() + s_stGlobalProcessNameOffset)

// ׷�ٷ�ʽ
#define DEBUG_TRACE_ERROR               0x00000001  // Errors - whenever we return a failure code
#define DEBUG_TRACE_LOAD_UNLOAD         0x00000002  // Loading/unloading of the filter
#define DEBUG_TRACE_INSTANCES           0x00000004  // Attach / detatch of instances
#define DEBUG_TRACE_DATA_OPERATIONS     0x00000008  // Operation to access / modify in memory metadata
#define DEBUG_TRACE_ALL_IO              0x00000010  // All IO operations tracked by this filter
#define DEBUG_TRACE_NORMAL_INFO         0x00000020  // Misc. information
#define DEBUG_TRACE_IMPORTANT_INFO      0x00000040  // Important information
#define DEBUG_TRACE_CONFIDENTIAL        0x00000080
#define DEBUG_TRACE_TEMPORARY           0x00000100
#define DEBUG_TRACE_ALL                 0xFFFFFFFF  // All flags

// ��ǰ DebugTrace() Mask
#if 0
#define DEBUG_TRACE_MASK                DEBUG_TRACE_ALL
#else
#define DEBUG_TRACE_MASK                DEBUG_TRACE_TEMPORARY | DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL
#endif

// ��ǰ DebugTraceEx() Mask
#if 1
#define DEBUG_TRACE_EX_MASK             DEBUG_TRACE_ALL
#else
#define DEBUG_TRACE_EX_MASK             DEBUG_TRACE_TEMPORARY | DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL
#endif

#define KD_DEBUG_TRACE_DISABLE          0
#define KD_DEBUG_TRACE_TO_DBGPRINT      1
#define KD_DEBUG_TRACE_TO_LOG           2

//
// DebugTrace ��Ϣ�����ģʽ: 0 �ر�, 1 ����� DbgPrint, 2 ����� Log �ļ�.
//
#define KD_DEBUG_TRACE_MODE             KD_DEBUG_TRACE_TO_DBGPRINT

#if defined(KD_DEBUG_TRACE_MODE) && (KD_DEBUG_TRACE_MODE == KD_DEBUG_TRACE_TO_LOG)

#define DebugTrace(_Level, _ProcedureName, _Data, ...) \
    if ((_Level) & (DEBUG_TRACE_MASK)) { \
        KeLog_LogPrint("[Antinvader:"_ProcedureName"]\n\t\t\t"_Data"\n", __VA_ARGS__); \
    }

#define DebugTraceFile(_Level, _ProcedureName, _FileName, _Data, ...) \
    if ((_Level) & (DEBUG_TRACE_MASK)) { \
        KeLog_LogPrint("[Antinvader:"_ProcedureName"] %s\n\t\t\t"_Data"\n", \
            _FileName, __VA_ARGS__); \
    }

#define DebugTraceFileAndProcess(_Level, _ProcedureName, _FileName, _Data, ...)  \
    if ((_Level) & (DEBUG_TRACE_MASK)) {                                    \
        KeLog_LogPrint("[Antinvader:"_ProcedureName"] %ws: %s\n\t\t\t"_Data"\n", \
            _FileName, CURRENT_PROCESS_NAME_BUFFER, __VA_ARGS__); \
    }

#define FltDebugTrace(_Instance, _Level, _ProcedureName, _Data, ...) \
    if ((_Level) & (DEBUG_TRACE_MASK)) { \
        KeLog_FltLogPrint(_Instance, "[Antinvader:"_ProcedureName"]\n\t\t\t"_Data"\n", __VA_ARGS__); \
    }

#define FltDebugTraceFile(_Instance, _Level, _ProcedureName, _FileName, _Data, ...) \
    if ((_Level) & (DEBUG_TRACE_MASK)) { \
        KeLog_FltLogPrint(_Instance, "[Antinvader:"_ProcedureName"] %s\n\t\t\t"_Data"\n", \
            _FileName, __VA_ARGS__); \
    }

#define FltDebugTraceFileAndProcess(_Instance, _Level, _ProcedureName, _FileName, _Data, ...) \
    if ((_Level) & (DEBUG_TRACE_MASK)) { \
        KeLog_FltLogPrint(_Instance, "[Antinvader:"_ProcedureName"] %ws: %s\n\t\t\t"_Data"\n", \
            _FileName, CURRENT_PROCESS_NAME_BUFFER, __VA_ARGS__); \
    }

#elif defined(KD_DEBUG_TRACE_MODE) && (KD_DEBUG_TRACE_MODE == KD_DEBUG_TRACE_TO_DBGPRINT)

#define DebugTrace(_Level, _ProcedureName, _Data, ...) \
    if ((_Level) & (DEBUG_TRACE_MASK)) { \
        DbgPrint("[Antinvader:"_ProcedureName"]\n\t\t\t"_Data"\n", __VA_ARGS__); \
    }

#define DebugTraceFile(_Level, _ProcedureName, _FileName, _Data, ...) \
    if ((_Level) & (DEBUG_TRACE_MASK)) { \
        DbgPrint("[Antinvader:"_ProcedureName"] %s\n\t\t\t"_Data"\n", \
            _FileName, __VA_ARGS__); \
    }

#define DebugTraceFileAndProcess(_Level, _ProcedureName, _FileName, _Data, ...)  \
    if ((_Level) & (DEBUG_TRACE_MASK)) {                                    \
        DbgPrint("[Antinvader:"_ProcedureName"] %ws: %s\n\t\t\t"_Data"\n", \
            _FileName, CURRENT_PROCESS_NAME_BUFFER, __VA_ARGS__); \
    }

#define FltDebugTrace(_Instance, _Level, _ProcedureName, _Data, ...) \
    if ((_Level) & (DEBUG_TRACE_MASK)) { \
        DbgPrint("[Antinvader:"_ProcedureName"]\n\t\t\t"_Data"\n", __VA_ARGS__); \
    }

#define FltDebugTraceFile(_Instance, _Level, _ProcedureName, _FileName, _Data, ...) \
    if ((_Level) & (DEBUG_TRACE_MASK)) { \
        DbgPrint("[Antinvader:"_ProcedureName"] %s\n\t\t\t"_Data"\n", \
            _FileName, __VA_ARGS__); \
    }

#define FltDebugTraceFileAndProcess(_Instance, _Level, _ProcedureName, _FileName, _Data, ...) \
    if ((_Level) & (DEBUG_TRACE_MASK)) { \
        DbgPrint("[Antinvader:"_ProcedureName"] %ws: %s\n\t\t\t"_Data"\n", \
            _FileName, CURRENT_PROCESS_NAME_BUFFER, __VA_ARGS__); \
    }

#else

// Invalidate all DebugTrace macros
#define DebugTrace(_Level, _ProcedureName, _Data, ...) \
    (void)(0)

#define DebugTraceFile(_Level, _ProcedureName, _FileName, _Data, ...) \
    (void)(0)

#define DebugTraceFileAndProcess(_Level, _ProcedureName, _FileName, _Data, ...) \
    (void)(0)

#define FltDebugTrace(_Instance, _Level, _ProcedureName, _Data, ...) \
    (void)(0)

#define FltDebugTraceFile(_Instance, _Level, _ProcedureName, _FileName, _Data, ...) \
    (void)(0)

#define FltDebugTraceFileAndProcess(_Instance, _Level, _ProcedureName, _FileName, _Data, ...) \
    (void)(0)

#endif // KD_DEBUG_TRACE_MODE

#define DebugTraceEx(_Level, _ProcedureName, _Data, ...) \
    if ((_Level) & (DEBUG_TRACE_MASK)) { \
        DbgPrint("[Antinvader:"_ProcedureName"]\n\t\t\t"_Data"\n", __VA_ARGS__); \
    }

#ifndef USE_DEBUG_PRINT
#define USE_DEBUG_PRINT     0
#endif

#if defined(USE_DEBUG_PRINT) && (USE_DEBUG_PRINT != 0)
    #ifdef ADVANCED_DEBUG_PRINT
    #define KdDebugPrint DbgPrint("[%s] %s (line: %d)\n", __##FILE##__, __##FUNCTION##__, __##LINE##__); DbgPrint
    #else
    #define KdDebugPrint DbgPrint
    #endif
#else
    #define KdDebugPrint_void(_x_, ...)  (void)(0)
    #define KdDebugPrint KdDebugPrint_void
#endif

#define ANTINVADER_FILTER_BASE_DIR      L"\\MiniFilter\\Test\\"

/*
//
// ����
//
    KdPrint(("DebugTestRead: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
        "\t\tIRP_BUFFERED_IO:           %d\n"
        "\t\tIRP_CLOSE_OPERATION:       %d\n"
        "\t\tIRP_DEALLOCATE_BUFFER:     %d\n"
        "\t\tIRP_INPUT_OPERATION:       %d\n"
        "\t\tIRP_NOCACHE:               %d\n"
        "\t\tIRP_PAGING_IO:             %d\n"
        "\t\tIRP_SYNCHRONOUS_API:       %d\n"
        "\t\tIRP_SYNCHRONOUS_PAGING_IO: %d\n"
        "\t\tLength:                    %d\n"
        "\t\tByteOffset                 %d\n",
        pIoParameterBlock->IrpFlags&(IRP_BUFFERED_IO),
        pIoParameterBlock->IrpFlags&(IRP_CLOSE_OPERATION),
        pIoParameterBlock->IrpFlags&(IRP_DEALLOCATE_BUFFER),
        pIoParameterBlock->IrpFlags&(IRP_INPUT_OPERATION),
        pIoParameterBlock->IrpFlags&(IRP_NOCACHE),
        pIoParameterBlock->IrpFlags&(IRP_PAGING_IO),
        pIoParameterBlock->IrpFlags&(IRP_SYNCHRONOUS_API),
        pIoParameterBlock->IrpFlags&(IRP_SYNCHRONOUS_PAGING_IO),
        pIoParameterBlock->Parameters.Read.Length,
        pIoParameterBlock->Parameters.Read.ByteOffset.QuadPart)
    );
*/

// ������

#define DebugPrintFileObject(_header, _object, _cached) \
    KdPrint(("[Antinvader] %s.\n"       \
        "\t\tName: %ws\n"               \
        "\t\tCached %d\n"               \
        "\t\tFcb: 0x%X\n",              \
        _header,                        \
        _object->FileName.Buffer,       \
        _cached,                        \
        _object->FsContext              \
    ));

#define DebugPrintFileStreamContext(_header, _object) \
    KdPrint(("[Antinvader] %s.\n"       \
        "\t\tName: %ws\n"               \
        "\t\tCached %d\n"               \
        "\t\tCachedFcb: 0x%X\n"         \
        "\t\tNoneCachedFcb: 0x%X\n",    \
        _header,                        \
        _object->usName.Buffer,         \
        _object->bCached,               \
        _object->pfcbCachedFCB,         \
    ));

// #else

// #define DebugTrace(_Level,_ProcedureName, _Data)         { NOTHING; }
// #define DebugPrintFileObject(_header,_object,_cached)    { NOTHING; }
// #define DebugPrintFileStreamContext(_header,_object)     { NOTHING; }

// #endif

// Values defined for "Method"
// METHOD_BUFFERED
// METHOD_IN_DIRECT
// METHOD_OUT_DIRECT
// METHOD_NEITHER
//
// Values defined for "Access"
// FILE_ANY_ACCESS
// FILE_READ_ACCESS
// FILE_WRITE_ACCESS

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

const ULONG IOCTL_ANTINVADERDRIVER_OPERATION = CTL_CODE(FILE_DEVICE_ANTINVADERDRIVER, 0x01, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA);

//ֻ����notepad.exe
#define TEST_DRIVER_NOTEPAD 1

#endif // __ANTINVADERDRIVER_H_VERSION__
