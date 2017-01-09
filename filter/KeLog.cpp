///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2016 - 2017
//
// ԭʼ�ļ�����     : KeLog.cpp
// ��������         : AntinvaderDriver
// ����ʱ��         : 2017-01-09
//
//
// ����             : �ں������� Log �ļ����ģ��
//
// ����ά��:
//  0000 [2017-01-09] ����汾.
//
///////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdarg.h>
//#include <wdm.h>
#include <ntifs.h>

#include "KeLog.h"
#include "AntinvaderDriver.h"
#include "ProcessFunction.h"

//
// See: http://blog.csdn.net/iamrainliang/article/details/2065534
// See: http://bbs.pediy.com/showthread.php?p=1435172
//

//
// Enable log event: for synchronization
//
static KEVENT   s_eventKeLogComplete;
static KEVENT   s_eventFltKeLogComplete;
static WCHAR    s_szLogFile[] = L"\\??\\C:\\KeLog.log";
static WCHAR    s_szFltLogFile[] = L"\\??\\C:\\KeFltLog.log";

//----------------------------------------------------------------------
//
// initialization interface
//
//----------------------------------------------------------------------
//
// initialize the global data structures, when the driver is loading.
// (Call in DriverEntry())
//
NTSTATUS
KeLog_Init()
{
    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    // Initialize the event
    KeInitializeEvent(&s_eventKeLogComplete, SynchronizationEvent, TRUE);
    KeInitializeEvent(&s_eventFltKeLogComplete, SynchronizationEvent, FALSE);
    return STATUS_SUCCESS;
}

NTSTATUS
KeLog_Unload()
{
    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    // Wait for the event done
    KeWaitForSingleObject(&s_eventKeLogComplete, Executive, KernelMode, TRUE, 0);
    KeWaitForSingleObject(&s_eventFltKeLogComplete, Executive, KernelMode, TRUE, 0);
    return STATUS_SUCCESS;
}

static void KeLog_AcquireLock()
{
    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    // Wait for enable log event
    KeWaitForSingleObject(&s_eventKeLogComplete, Executive, KernelMode, TRUE, 0);
    KeClearEvent(&s_eventKeLogComplete);
}

static void KeLog_ReleaseLock()
{
    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    // Set enable log event
    KeSetEvent(&s_eventKeLogComplete, 0, FALSE);
}

void KeLog_GetCurrentTime(PTIME_FIELDS timeFileds)
{
    LARGE_INTEGER sysTime, localTime;

    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    // ��ȡ��ǰϵͳʱ��
    KeQuerySystemTime(&sysTime);

    // ת��Ϊ����ʱ��
    ExSystemTimeToLocalTime(&sysTime, &localTime);

    // ת��Ϊ���ǿ�������ʱ���ʽ
    RtlTimeToTimeFields(&localTime, timeFileds);
}

void KeLog_GetCurrentTimeString(LPSTR time)
{
    TIME_FIELDS sysTime;
    KeLog_GetCurrentTime(&sysTime);

    sprintf(time, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
        (INT32)sysTime.Year, (INT32)sysTime.Month, (INT32)sysTime.Day,
        (INT32)sysTime.Hour, (INT32)sysTime.Minute, (INT32)sysTime.Second, (INT32)sysTime.Milliseconds);
}

static
VOID
KeLog_FileCompleteCallback(
    __in PFLT_CALLBACK_DATA CallbackData,
    __in PFLT_CONTEXT Context
    )
{
    //
    // ������ɱ�־
    //
    KeSetEvent((PRKEVENT)Context, 0, FALSE);
}

//----------------------------------------------------------------------
//
// KeLog_FltPrint
//
// Trace to file.
//
//----------------------------------------------------------------------
BOOLEAN
KeLog_FltPrint(PFLT_INSTANCE pfiInstance, LPCSTR lpszLog, ...)
{
    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    if (KeGetCurrentIrql() > PASSIVE_LEVEL) {
        KeLog_TracePrint(("KeTracePrint: KeLog: IRQL too hight... (IRQL Level = %u\n", (UINT32)KeGetCurrentIrql()));
        return FALSE;
    }

    BOOLEAN success;
    CHAR szBuffer[1024];
    CHAR szTime[64];
    PCHAR pszBuffer = szBuffer;
    ULONG ulBufSize;
    int nSize;
    va_list pArglist;
    ULONG ulLength;
    BOOLEAN bSucceed = FALSE;
    ANSI_STRING ansiProcessName = { 0 };
    TIME_FIELDS sysTime;
    LPCWSTR lpszLogFile = s_szFltLogFile;
    NTSTATUS status;

    if (pfiInstance == NULL) {
        KeLog_TracePrint(("KeTracePrint: KeLog(): KeLog_FltPrint() pfiInstance is nullptr !!\n"));
        return FALSE;
    }

    KeLog_AcquireLock();

    ulLength = FltGetCurrentProcessNameA(&ansiProcessName, &bSucceed);
    if (!bSucceed || ulLength <= 0) {
        // Can not get the process name
        RtlInitAnsiString(&ansiProcessName, "Uknown Name");
    }

    KeLog_GetCurrentTimeString(szTime);

    // Add process name and time string
    sprintf(szBuffer, "[%s][%16s:%d] ", szTime, ansiProcessName.Buffer, (ULONG)PsGetCurrentProcessId());
    pszBuffer = szBuffer + strlen(szBuffer);

    va_start(pArglist, lpszLog);
    // The last argument to wvsprintf points to the arguments  
    nSize = _vsnprintf(pszBuffer, 1024 - (strlen(szBuffer) + 1), lpszLog, pArglist);
    // The va_end macro just zeroes out pArgList for no good reason  
    va_end(pArglist);

    if (nSize > 0)
        pszBuffer[nSize] = 0;
    else
        pszBuffer[0] = 0;

    ulBufSize = strlen(szBuffer);

    // Get the Unicode log filename.
    UNICODE_STRING fileName;

    // Get a handle to the log file object
    fileName.Buffer = NULL;
    fileName.Length = 0;
    fileName.MaximumLength = (wcslen(lpszLogFile) + 1) * sizeof(WCHAR);
    fileName.Buffer = (PWCH)ExAllocatePool(PagedPool, fileName.MaximumLength);
    if (fileName.Buffer == NULL) {
        KeLog_TracePrint(("KeTracePrint: KeLog: KeLog_FltPrint() ExAllocatePool Failed ...\n"));
        return FALSE;
    }
    RtlZeroMemory(fileName.Buffer, fileName.MaximumLength);
    status = RtlAppendUnicodeToString(&fileName, (PWSTR)lpszLogFile);

    __try {
        IO_STATUS_BLOCK IoStatus;
        OBJECT_ATTRIBUTES objectAttributes;
        HANDLE FileHandle = NULL;
        PFILE_OBJECT pfoFileObject = NULL;
        LARGE_INTEGER nOffset = { 0 };

        InitializeObjectAttributes(&objectAttributes,
            (PUNICODE_STRING)&fileName,
            OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

        //
        // FltCreateFile function
        // See: https://msdn.microsoft.com/zh-cn/library/windows/hardware/ff541935(v=vs.85).aspx
        //

        //
        // FltCreateFileEx2 routine
        // See: https://msdn.microsoft.com/library/windows/hardware/ff541939
        //

        //
        // FltCreateFileEx routine
        // See: https://msdn.microsoft.com/zh-cn/library/windows/hardware/ff541937(v=vs.85).aspx
        //
#if 0
        status = FltCreateFileEx(
            pfltGlobalFilterHandle,
            pfiInstance,
            &FileHandle,
            &pfoFileObject,
            FILE_APPEND_DATA,
            &objectAttributes,
            &IoStatus,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_WRITE,
            FILE_OPEN_IF,
            FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
            NULL,
            0,
            IO_IGNORE_SHARE_ACCESS_CHECK);
#endif

        status = FltCreateFileEx(
            pfltGlobalFilterHandle,
            pfiInstance,
            &FileHandle,
            &pfoFileObject,
            FILE_APPEND_DATA | GENERIC_WRITE,
            &objectAttributes,
            &IoStatus,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_OPEN_IF,
            FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
            NULL,
            0,
            IO_IGNORE_SHARE_ACCESS_CHECK);

        if (NT_SUCCESS(status)) {
#if 1
            //
            // See: https://msdn.microsoft.com/zh-cn/library/windows/hardware/ff544610(v=vs.85).aspx
            //
            if (pfoFileObject != NULL) {
                FltWriteFile(
                    pfiInstance,
                    pfoFileObject,
                    &nOffset,
                    ulBufSize,
                    szBuffer,
                    FLTFL_IO_OPERATION_SYNCHRONOUS_PAGING,
                    NULL,
                    KeLog_FileCompleteCallback,
                    &s_eventFltKeLogComplete);
                //
                // �ȴ� FltWriteFile ���.
                //
                KeWaitForSingleObject(&s_eventFltKeLogComplete, Executive, KernelMode, TRUE, 0);
            }
#endif
            // Close �ļ�
            FltClose(FileHandle);
        }

        KeLog_ReleaseLock();
        success = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        KeLog_ReleaseLock();
        KeLog_TracePrint(("KeTracePrint: KeLog(): KeLog_FltPrint() exception code: %0xd !!\n", GetExceptionCode()));
        success = FALSE;
    }

    if (fileName.Buffer)
        ExFreePool(fileName.Buffer);

    return success;
}

//----------------------------------------------------------------------
//
// KeLogPrint
//
// Trace to file.
//
//----------------------------------------------------------------------
BOOLEAN
KeLog_Print(LPCSTR lpszLog, ...)
{
    if (KeGetCurrentIrql() > PASSIVE_LEVEL) {
        KeLog_TracePrint(("KeTracePrint: KeLog: IRQL too hight... (IRQL Level = %u\n", (UINT32)KeGetCurrentIrql()));
        return FALSE;
    }

    BOOLEAN success;
    CHAR szBuffer[1024];
    CHAR szTime[32];
    PCHAR pszBuffer = szBuffer;
    ULONG ulBufSize;
    int nSize;
    va_list pArglist;
    ULONG ulLength;
    BOOLEAN bSucceed = FALSE;
    ANSI_STRING ansiProcessName = { 0 };
    TIME_FIELDS sysTime;
    LPCWSTR lpszLogFile = s_szLogFile;
    NTSTATUS status;

    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    ulLength = FltGetCurrentProcessNameA(&ansiProcessName, &bSucceed);
    if (!bSucceed || ulLength <= 0) {
        // Can not get the process name
        RtlInitAnsiString(&ansiProcessName, "Uknown Name");
    }

    KeLog_GetCurrentTimeString(szTime);

    // Add process name and time string
    sprintf(szBuffer, "[%s][%16s:%d] ", szTime,
        ansiProcessName.Buffer, (ULONG)PsGetCurrentProcessId());
    pszBuffer = szBuffer + strlen(szBuffer);

    va_start(pArglist, lpszLog);
    // The last argument to wvsprintf points to the arguments  
    nSize = _vsnprintf(pszBuffer, 1024 - 32, lpszLog, pArglist);
    // The va_end macro just zeroes out pArgList for no good reason  
    va_end(pArglist);
    if (nSize > 0)
        pszBuffer[nSize] = 0;
    else
        pszBuffer[0] = 0;

    ulBufSize = strlen(szBuffer);

    // Get the Unicode log filename.
    UNICODE_STRING fileName;

    // Get a handle to the log file object
    fileName.Buffer = NULL;
    fileName.Length = 0;
    fileName.MaximumLength = (wcslen(lpszLogFile) + 1) * sizeof(WCHAR);
    fileName.Buffer = (PWCH)ExAllocatePool(PagedPool, fileName.MaximumLength);
    if (fileName.Buffer == NULL) {
        KeLog_TracePrint(("KeTracePrint: KeLog: ExAllocatePool Failed ...\n"));
        return FALSE;
    }
    RtlZeroMemory(fileName.Buffer, fileName.MaximumLength);
    status = RtlAppendUnicodeToString(&fileName, (PWSTR)lpszLogFile);

    KeLog_AcquireLock();

    __try {
        IO_STATUS_BLOCK IoStatus;
        OBJECT_ATTRIBUTES objectAttributes;
        HANDLE FileHandle;

        InitializeObjectAttributes(&objectAttributes,
            (PUNICODE_STRING)&fileName,
            OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

        status = ZwCreateFile(&FileHandle,
            FILE_APPEND_DATA,
            &objectAttributes,
            &IoStatus,
            0,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_WRITE,
            FILE_OPEN_IF,
            FILE_SYNCHRONOUS_IO_NONALERT,
            NULL,
            0);

        if (NT_SUCCESS(status)) {
            ZwWriteFile(FileHandle,
                NULL,
                NULL,
                NULL,
                &IoStatus,
                szBuffer,
                ulBufSize,
                NULL,
                NULL);
            ZwClose(FileHandle);
        }

        KeLog_ReleaseLock();
        success = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        KeLog_ReleaseLock();
        KeLog_TracePrint(("KeTracePrint: KeLog() except: %0xd !!\n", GetExceptionCode()));
        success = FALSE;
    }

    if (fileName.Buffer)
        ExFreePool(fileName.Buffer);

    return success;
}
