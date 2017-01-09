///////////////////////////////////////////////////////////////////////////////
///
/// ��Ȩ���� (c) 2011 - 2012
///
/// ԭʼ�ļ�����     : FileFunction.h
/// ��������         : AntinvaderDriver
/// ����ʱ��         : 2011-03-26
///
///
/// ����             : �����ļ���Ϣ�Ĺ�������
///
/// ����ά��:
///  0000 [2011-03-26] ����汾.
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

//#include <ntdef.h>
#include <ntifs.h>
#include <fltKernel.h>

#include "CallbackRoutine.h"
#include "ConfidentialFile.h"

////////////////////////
//      �궨��
///////////////////////

/*
// �����ļ��ж��������� ����ָ����������Ϊ�����ļ�
#define CONFIDENTIAL_FILE_CONDITION_NAME                0x00000001  // �ļ���ȫ��ƥ��
#define CONFIDENTIAL_FILE_CONDITION_NAME_CONTAIN        0x00000002  // �ļ�������ָ���ַ�
#define CONFIDENTIAL_FILE_CONDITION_PATH_CONTAIN        0x00000003  // ·������ָ���ַ�
#define CONFIDENTIAL_FILE_CONDITION_BOTH_PATH_NAME      0x00000004  // ����·��ƥ��
#define CONFIDENTIAL_FILE_CONDITION_FOLDER              0x00000005  // ����·���а������ļ���(�ļ���ʹ�þ���·��)
#define CONFIDENTIAL_FILE_CONDITION_FOLDER_CONTAIN      0x00000006  // ����·���а������ļ���(���ļ���ֻҪ����ȷ������)
*/

// FileIsEncrypted Flag��־
#define FILE_IS_ENCRYPTED_DO_NOT_CHANGE_OPEN_WAY        0x00000001
#define FILE_IS_ENCRYPTED_DO_NOT_WRITE_LOGO             0x00000002

// �����ڴ��־
#define MEM_FILE_TAG                                    'file'

// ���ܱ�ʶ ע���޸ü��ܱ�ʶ��Ҫ��ENCRYPTION_HEAD_LOGO_SIZE�޸�Ϊ��Ӧ����ֵ
#define ENCRYPTION_HEADER                               { L'A',L'N',L'T',L'I',L'N',L'V',L'A',L'D',L'E',L'R', \
                                                         L'_',L'E',L'N',L'C',L'R',L'Y',L'P',L'T',L'E',L'D' }
////////////////////////
//      ȫ�ֱ���
////////////////////////

// ���ڴ�ż���ͷ�ڴ����������
extern NPAGED_LOOKASIDE_LIST nliNewFileHeaderLookasideList;

////////////////////////
//      ��������
////////////////////////

// һ����ļ�·������ ���ڲ²�򿪽��̵�·������,��������Ľ�Ϊ׼ȷ
// �������Ч��,���õ�Խ��ʱ�临�Ӷ�ԽС,�ռ临�Ӷ�Խ��
#define NORMAL_FILE_PATH_LENGTH         NORMAL_PATH_LENGTH

////////////////////////
//      �ṹ����
////////////////////////

// �ڷ������дʱ�ж��Ƕ�����д
typedef enum _READ_WRITE_TYPE
{
    RwRead = 1,
    RwWrite
} READ_WRITE_TYPE;

////////////////////////
//      ��������
////////////////////////
NTSTATUS
FileCreateByObjectNotCreated(
    __in PFLT_INSTANCE pfiInstance,
    __in PFLT_FILE_NAME_INFORMATION pfniFileNameInformation,
    __in PFLT_PARAMETERS pfpParameters,
    __in_opt ULONG ulDesiredAccess,
    __out HANDLE * phFileHandle
);

NTSTATUS
FileSetSize(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT pfoFileObject,
    __in PLARGE_INTEGER pnFileSize
);

NTSTATUS
FileGetStandardInformation(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT pfoFileObject,
    __inout_opt PLARGE_INTEGER pnAllocateSize,
    __inout_opt PLARGE_INTEGER pnFileSize,
    __inout_opt BOOLEAN *pbDirectory
);

static
VOID
FileCompleteCallback(
    __in PFLT_CALLBACK_DATA CallbackData,
    __in PFLT_CONTEXT Context
);

NTSTATUS
FileWriteEncryptionHeader(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT  pfoFileObject,
    __in PVOLUME_CONTEXT pvcVolumeContext,
    __in PFILE_STREAM_CONTEXT  pscFileStreamContext
);

NTSTATUS
FileIsEncrypted(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT pfoFileObjectOpened,
    __in PFLT_CALLBACK_DATA pfcdCBD,
    __in PVOLUME_CONTEXT pvcVolumeContext,
    __in PFILE_STREAM_CONTEXT  pscFileStreamContext,
    __in ULONG  ulFlags
);

void FileClearCache(PFILE_OBJECT pFileObject);

NTSTATUS
FileCreateForHeaderWriting(
    __in PFLT_INSTANCE pfiInstance,
    __in PUNICODE_STRING puniFileName,
    __out HANDLE * phFileHandle
);

USHORT
FileGetFilePostfixName(
    __in PUNICODE_STRING  puniFileName,
    __inout_opt PUNICODE_STRING puniPostfixName
);

NTSTATUS
FileReadEncryptionHeaderAndDeconstruct(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT  pfoFileObject,
    __in PVOLUME_CONTEXT pvcVolumeContext,
    __in PFILE_STREAM_CONTEXT  pscFileStreamContext
);
