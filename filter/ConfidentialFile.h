///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2011 - 2012
//
// ԭʼ�ļ�����     : ConfidentialFile.h
// ��������         : AntinvaderDriver
// ����ʱ��         : 2011-07-28
//
//
// ����             : �����ļ������ĵ�ͷ�ļ�
//
// ����ά��:
//  0001 [2011-07-28] ����汾.������һ�汾ȫ������
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ntdef.h>
#include <ntifs.h>
#include <fltKernel.h>
#include "Common.h"

////////////////////////
//      �궨��
////////////////////////

// �ļ��ڴ��־
#define MEM_TAG_FILE_TABLE          'cftm'

// �ļ��������Ĵ�С
#define FILE_STREAM_CONTEXT_SIZE    sizeof(_CUST_FILE_STREAM_CONTEXT)

// ������
#define FILE_STREAM_CONTEXT_LOCK_ON(_file_data)  \
    KeEnterCriticalRegion(); \
    ExAcquireResourceExclusiveLite(_file_data->prResource, TRUE); \
    KeLeaveCriticalRegion()

#define FILE_STREAM_CONTEXT_LOCK_OFF(_file_data) \
    KeEnterCriticalRegion(); \
    ExReleaseResourceLite(_file_data->prResource); \
    KeLeaveCriticalRegion()

////////////////////////
//      ��������
////////////////////////

// �����ļ�ͷ��С Ϊ��ҳ�����ʼ����Ϊ4k
#define CONFIDENTIAL_FILE_HEAD_SIZE         sizeof(CUST_FILE_ENCRYPTION_HEAD)

// ���ܱ�ʶ����
#define ENCRYPTION_HEAD_LOGO_SIZE           12

// ���ܱ�ʶ, ע���޸ļ��ܱ�ʶ��Ҫ��ENCRYPTION_HEAD_LOGO_SIZE�޸�Ϊ��Ӧ����ֵ.
#define ENCRYPTION_HEADER_BEGIN             { L'E',L'N',L'C',L'R',L'Y',L'P',L'T',L' ',L'F',L'L',L'A',L'G' }
#define ENCRYPTION_HEADER_END               { L'G',L'A',L'L',L'F',L' ',L'T',L'P',L'Y',L'R',L'C',L'N',L'E' }

////////////////////////
//      �ṹ����
////////////////////////

// ������ǰ�ļ������������Ļ�������, ���ܽ��̻��Ƿǻ��ܽ������ڷ���.
typedef enum _FILE_OPEN_STATUS {
    OPEN_STATUS_UNKNOWN = 0,           // �����л�,��Ҫˢ������
    OPEN_STATUS_CONFIDENTIAL,       // ��ǰ�ǻ��ܽ������ڷ���
    OPEN_STATUS_NOT_CONFIDENTIAL    // ��ǰ�Ƿǻ��ܽ������ڷ���
} FILE_OPEN_STATUS;

// ������ǰ�ļ��ǻ����ļ����Ƿǻ����ļ�.
typedef enum _FILE_ENCRYPTED_TYPE {
    ENCRYPTED_TYPE_UNKNOWN = 0,         // δ֪״̬
    ENCRYPTED_TYPE_ENCRYPTED,        // �ļ��ǻ��ܵ�
    ENCRYPTED_TYPE_NOT_ENCRYPTED     // �ļ��Ƿǻ��ܵ�
} FILE_ENCRYPTED_TYPE;

// �ļ��������Ľṹ��
typedef struct _CUST_FILE_STREAM_CONTEXT
{
    PERESOURCE prResource;                  // ȡ����Դ
    FILE_ENCRYPTED_TYPE fctEncrypted;       // �Ƿ񱻼���
    ULONG ulReferenceTimes;                 // ���ü���
    BOOLEAN bIsNeedRewriteFileEncryptedHeadWhenClose;               // �Ƿ���Ҫ�ڹر��ļ�ʱ��д����ͷ
    BOOLEAN bCached;                        // �Ƿ񻺳�
    LARGE_INTEGER nFileValidLength ;        // �ļ���Ч��С
    LARGE_INTEGER nFileSize ;               // �ļ�ʵ�ʴ�С �����˼���ͷ��
    FILE_OPEN_STATUS fosOpenStatus;         // ��ǰ����������/�����Ž��̴�
    PVOID pfcbFCB;                          // ����FCB��ַ
//  PVOID pfcbNoneCachedFCB;                // �ǻ���FCB��ַ
    UNICODE_STRING usName;                  // �ļ�����
    UNICODE_STRING usPostFix;               // �ļ���׺��
//  UNICODE_STRING usPath;                  // �ļ�·��
    WCHAR wszVolumeName[NORMAL_NAME_LENGTH] ;               // ������

} CUST_FILE_STREAM_CONTEXT, * PCUST_FILE_STREAM_CONTEXT;

// ����ͷ�ṹ��
typedef struct _CUST_FILE_ENCRYPTION_HEAD
{
    WCHAR wEncryptionLogo_begin[ENCRYPTION_HEAD_LOGO_SIZE];   // 20
    //WCHAR wSeperate0[4];        // 8
    //ULONG ulVersion;            // 4
    //WCHAR wSeperate1[4];        // 8
    LONGLONG nFileValidLength;  // 8
    //WCHAR wSeperate2[4];        // 8
    LONGLONG nFileRealSize;     // 8
    //WCHAR wSeperate3[4];        // 8
    //WCHAR wMD5Check[32];        // 64
    //WCHAR wSeperate4[4];        // 8
    //WCHAR wCRC32Check[32];      // 64
    //WCHAR wSeperate5[4];        // 8
    //WCHAR wKeyEncrypted[32];    // 64
	WCHAR wEncryptionLogo_end[ENCRYPTION_HEAD_LOGO_SIZE];   // 20
} CUST_FILE_ENCRYPTION_HEAD, * PCUST_FILE_ENCRYPTION_HEAD;

///////////////////////
//      ��������
///////////////////////

NTSTATUS
FctCreateCustFileStreamContextForFileObject(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT pfoFileObject,
	__in PFLT_CALLBACK_DATA pfcdCBD,
	__in PFLT_FILE_NAME_INFORMATION pfniFileNameInformation,
    __inout PCUST_FILE_STREAM_CONTEXT * dpscFileStreamContext
);

NTSTATUS
FctUpdateCustFileStreamContextFileName(
    __in PUNICODE_STRING pusName,
    __inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext
);

NTSTATUS
FctFreeCustFileStreamContext(
    __inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext
);

NTSTATUS
FctInitializeCustFileStreamContext(
    __inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext,
    __in PFLT_CALLBACK_DATA pfcdCBD,
    __in PFLT_FILE_NAME_INFORMATION pfniFileNameInformation
);

NTSTATUS
FctGetCustFileStreamContextByFileObject(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT pfoFileObject,
    __inout PCUST_FILE_STREAM_CONTEXT * dpscFileStreamContext
);

FILE_ENCRYPTED_TYPE
FctGetCustFileStreamContextEncryptedType(
    __in    PCUST_FILE_STREAM_CONTEXT pscFileStreamContext
);

VOID
FctDecCustFileStreamContextReferenceCount(
    __inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext
);

//VOID
//FctIncCustFileStreamContextReferenceCount(
//    __inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext
//);

VOID
FctSetCustFileStreamContextEncryptedType(
    __inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext,
    __in    FILE_ENCRYPTED_TYPE  fetFileEncryptedType
);

VOID FctReleaseCustFileStreamContext(
    __inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext
);
BOOLEAN
FctIsCustFileStreamContextReferenceCountZero(
    __in    PCUST_FILE_STREAM_CONTEXT  pscFileStreamContext
);

BOOLEAN
FctIsNeedRewriteFileEncryptedHeadWhenClose(
    __inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext
);

VOID
FctSetIsNeedRewriteFileEncryptedHeadWhenClose(
    __inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext,
    __in    BOOLEAN              bSet
);

VOID
FctGetCustFileStreamContextValidSize(
    __in    PCUST_FILE_STREAM_CONTEXT pscFileStreamContext,
    __inout PLARGE_INTEGER       pnFileValidSize
);

VOID
FctUpdateCustFileStreamContextValidSize(
    __inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext,
    __in    PLARGE_INTEGER       pnFileValidSize,
    __in    BOOLEAN              bSetUpdateWhenClose
);

BOOLEAN
FctUpdateCustFileStreamContextValidSizeIfLonger(
    __inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext,
    __in    PLARGE_INTEGER       pnFileValidSize,
    __in    BOOLEAN              bSetUpdateWhenClose
);

NTSTATUS
FctEncodeCustFileStreamContextEncrytedHead(
    __in    PCUST_FILE_STREAM_CONTEXT pscFileStreamContext,
    __inout PVOID pFileHead
);

NTSTATUS
FctDecodeCustFileStreamContextEncrytedHead(
    __inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext,
    __in PVOID  pFileHead
);
