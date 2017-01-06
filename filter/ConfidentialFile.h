///////////////////////////////////////////////////////////////////////////////
///
/// ��Ȩ���� (c) 2011 - 2012
///
/// ԭʼ�ļ�����     : ConfidentialFile.h
/// ��������         : AntinvaderDriver
/// ����ʱ��         : 2011-07-28
///
///
/// ����             : �����ļ������ĵ�ͷ�ļ�
///
/// ����ά��:
///  0001 [2011-07-28] ����汾.������һ�汾ȫ������
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ntdef.h>
#include <ntifs.h>
#include <fltKernel.h>

////////////////////////
//      �궨��
////////////////////////

// �ļ��ڴ��־
#define MEM_TAG_FILE_TABLE          'cftm'

// �ļ��������Ĵ�С
#define FILE_STREAM_CONTEXT_SIZE    sizeof(_FILE_STREAM_CONTEXT)

// ������
#define FILE_STREAM_CONTEXT_LOCK_ON(_file_data)     KeEnterCriticalRegion();ExAcquireResourceExclusiveLite( _file_data->prResource , TRUE );KeLeaveCriticalRegion()
#define FILE_STREAM_CONTEXT_LOCK_OFF(_file_data)    KeEnterCriticalRegion();ExReleaseResourceLite( _file_data->prResource );KeLeaveCriticalRegion()

////////////////////////
//      ��������
////////////////////////

// �����ļ�ͷ��С Ϊ��ҳ�����ʼ����Ϊ4k
#define CONFIDENTIAL_FILE_HEAD_SIZE                     (1024 * 1)

// ���ܱ�ʶ����
#define ENCRYPTION_HEAD_LOGO_SIZE                       40
// sizeof(ENCRYPTION_HEADER)

////////////////////////
//      �ṹ����
////////////////////////

// ������ǰ�ļ������������Ļ�������  ���ܽ��̻��Ƿǻ��ܽ������ڷ���
typedef enum _FILE_OPEN_STATUS {
    OPEN_STATUS_FREE = 0,// �����л�,��Ҫˢ������
    OPEN_STATUS_CONFIDENTIAL,// ��ǰ�ǻ��ܽ������ڷ���
    OPEN_STATUS_NOT_CONFIDENTIAL// ��ǰ�Ƿǻ��ܽ������ڷ���
} FILE_OPEN_STATUS;

// ������ǰ�ļ��ǻ����ļ����Ƿǻ����ļ�.
typedef enum _FILE_ENCRYPTED_TYPE {
    ENCRYPTED_TYPE_UNKNOWN = 0,// ��֪���������
    ENCRYPTED_TYPE_CONFIDENTIAL,// �ļ��ǻ��ܵ�
    ENCRYPTED_TYPE_NOT_CONFIDENTIAL// �ļ��Ƿǻ��ܵ�
} FILE_ENCRYPTED_TYPE;

// �ļ��������Ľṹ��
typedef struct _FILE_STREAM_CONTEXT
{
//  KSPIN_LOCK kslLock;                     // ������
//  KIRQL irqlLockIRQL;                     // ȡ���ж�
    PERESOURCE prResource;                  // ȡ����Դ
    FILE_ENCRYPTED_TYPE fctEncrypted;       // �Ƿ񱻼���
    ULONG ulReferenceTimes;                 // ���ü���
    BOOLEAN bUpdateWhenClose;               // �Ƿ���Ҫ�ڹر��ļ�ʱ��д����ͷ
    BOOLEAN bCached;                        // �Ƿ񻺳�
    LARGE_INTEGER nFileValidLength ;        // �ļ���Ч��С
    LARGE_INTEGER nFileSize ;               // �ļ�ʵ�ʴ�С �����˼���ͷ��
    FILE_OPEN_STATUS fosOpenStatus;         // ��ǰ����������(TRUE)��������(FALSE)
    PVOID pfcbFCB;                          // ����FCB��ַ
//  PVOID pfcbNoneCachedFCB;                // �ǻ���FCB��ַ
    UNICODE_STRING usName;                  // �ļ�����
    UNICODE_STRING usPostFix;               // �ļ���׺��
//  UNICODE_STRING usPath;                  // �ļ�·��
    WCHAR wszVolumeName[64] ;               // ������

} FILE_STREAM_CONTEXT, * PFILE_STREAM_CONTEXT;

// ����ͷ�ṹ��
typedef struct _FILE_ENCRYPTION_HEAD
{
    WCHAR wEncryptionLogo[ENCRYPTION_HEAD_LOGO_SIZE];   // 40
    WCHAR wSeperate0[4];        // 8
    ULONG ulVersion;
    WCHAR wSeperate1[4];        // 8
    LONGLONG nFileValidLength;  // 8
    WCHAR wSeperate2[4];        // 8
    LONGLONG nFileRealSize;     // 8
    WCHAR wSeperate3[4];        // 8
    WCHAR wMD5Check[32];
    WCHAR wSeperate4[4];        // 8
    WCHAR wCRC32Check[32];
    WCHAR wSeperate5[4];        // 8
    WCHAR wKeyEncrypted[32];

} FILE_ENCRYPTION_HEAD, * PFILE_ENCRYPTION_HEAD;

///////////////////////
//      ��������
///////////////////////

NTSTATUS
FctCreateContextForSpecifiedFileStream(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT pfoFileObject,
    __inout PFILE_STREAM_CONTEXT * dpscFileStreamContext
   );

NTSTATUS
FctUpdateStreamContextFileName(
    __in PUNICODE_STRING pusName,
    __inout PFILE_STREAM_CONTEXT  pscFileStreamContext
    );

NTSTATUS
FctFreeStreamContext(
    __inout PFILE_STREAM_CONTEXT  pscFileStreamContext
    );

NTSTATUS
FctInitializeContext(
    __inout PFILE_STREAM_CONTEXT pscFileStreamContext,
    __in PFLT_CALLBACK_DATA pfcdCBD,
    __in PFLT_FILE_NAME_INFORMATION pfniFileNameInformation
   );

NTSTATUS
FctGetSpecifiedFileStreamContext(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT pfoFileObject,
    __inout PFILE_STREAM_CONTEXT * dpscFileStreamContext
   );

FILE_ENCRYPTED_TYPE
FctGetFileConfidentialCondition(
    __in    PFILE_STREAM_CONTEXT  pscFileStreamContext
    );

VOID
FctDereferenceFileContext(
    __inout PFILE_STREAM_CONTEXT  pscFileStreamContext
    );

VOID
FctReferenceFileContext(
    __inout PFILE_STREAM_CONTEXT  pscFileStreamContext
    );

VOID
FctUpdateFileConfidentialCondition(
    __inout PFILE_STREAM_CONTEXT  pscFileStreamContext,
    __in    FILE_ENCRYPTED_TYPE   fetFileEncryptedType
    );

VOID FctReleaseStreamContext(
    __inout PFILE_STREAM_CONTEXT    pscFileStreamContext
    );

BOOLEAN
FctIsReferenceCountZero(
    __in    PFILE_STREAM_CONTEXT  pscFileStreamContext
    );

BOOLEAN
FctIsUpdateWhenCloseFlag(
    __inout PFILE_STREAM_CONTEXT    pscFileStreamContext
    );

VOID
FctSetUpdateWhenCloseFlag(
    __inout PFILE_STREAM_CONTEXT    pscFileStreamContext,
    __in    BOOLEAN                 bSet
    );

VOID
FctGetFileValidSize(
    __in    PFILE_STREAM_CONTEXT  pscFileStreamContext,
    __inout PLARGE_INTEGER        pnFileValidSize
    );

VOID
FctUpdateFileValidSize(
    __inout PFILE_STREAM_CONTEXT  pscFileStreamContext,
    __in    PLARGE_INTEGER        pnFileValidSize,
    __in    BOOLEAN               bSetUpdateWhenClose
    );

BOOLEAN
FctUpdateFileValidSizeIfLonger(
    __inout PFILE_STREAM_CONTEXT  pscFileStreamContext,
    __in    PLARGE_INTEGER        pnFileValidSize,
    __in    BOOLEAN               bSetUpdateWhenClose
    );

NTSTATUS
FctConstructFileHead(
    __in    PFILE_STREAM_CONTEXT pscFileStreamContext,
    __inout PVOID pFileHead
    );

NTSTATUS
FctDeconstructFileHead(
    __inout PFILE_STREAM_CONTEXT  pscFileStreamContext,
    __in PVOID  pFileHead
    );
