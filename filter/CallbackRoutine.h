///////////////////////////////////////////////////////////////////////////////
///
/// ��Ȩ���� (c) 2011 - 2012
///
/// ԭʼ�ļ�����     : CallbackRoutine.h
/// ��������         : AntinvaderDriver
/// ����ʱ��         : 2011-03-20
///
///
/// ����             : Antinvader �ص���������
///
/// ����ά��:
///  0000 [2011-03-20] ����汾.
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

//#include <ntdef.h>
#include <ntifs.h>
#include <fltKernel.h>

////////////////////////
//     �ṹ����
////////////////////////

// ��ص�����
typedef enum _POST_CALLBACK_FLAG
{
    // ʲôҲ����
    DoNothing = 0,

    // ����ʱʹ��,�����ļ��������ļ���
    CreateAddToTable,

    // ����ǻ��淽ʽ�򿪵�����
    CreateAddNoneCached,

    // ����ʱ��֯����
    CreateDenied

} POST_CALLBACK_FLAG, * PPOST_CALLBACK_FLAG;

// ��ص������Ľṹ
typedef struct _POST_CALLBACK_CONTEXT
{
    // ��־
    POST_CALLBACK_FLAG pcFlags;

    // ����
    PVOID pData;

} POST_CALLBACK_CONTEXT, * PPOST_CALLBACK_CONTEXT;

// �������Ľṹ
typedef struct _VOLUME_CONTEXT {

    // ���������
    UNICODE_STRING uniName;

    //  Holds the name of file system mounted on volume(old)
    //  Now it is useless
    // UNICODE_STRING FsName;

    //
    //  Holds the sector size for this volume.
    //
    ULONG ulSectorSize;

    //
    // ��ȡ�ļ��ж��Ƿ����ʱʹ�õ���������
    //
    PNPAGED_LOOKASIDE_LIST pnliReadEncryptedSignLookasideList;

    // Holds encryption/decryption context
    ///COUNTER_MODE_CONTEXT* aes_ctr_ctx ;

    // key. used to encrypt/decrypt files in the volume
//  UCHAR szKey[MAX_KEY_LENGTH] ;
    // key digest. used to verify whether specified file
    // can be decrypted/encrypted by this key
//  UCHAR szKeyHash[HASH_SIZE] ;

    // spinlock to synchornize encryption/decryption process
//  KSPIN_LOCK FsCryptSpinLock ;

    // Table to hold file context defined in file.h(old)
    // Now it is useless
//  RTL_GENERIC_TABLE FsCtxTable ;

    // mutex to synchronize file context table(old)
    // Now it is used to synchronize encryption/decryption process
//  FAST_MUTEX FsCtxTableMutex ;

} VOLUME_CONTEXT, * PVOLUME_CONTEXT;

/*
// �ļ���������
typedef struct _STREAM_CONTEXT {

    // ������������ĵ��ļ�����
    UNICODE_STRING FileName;

    // ������
    WCHAR wszVolumeName[64] ;

    // file key hash
//  UCHAR szKeyHash[HASH_SIZE] ;

    // Number of times we saw a create on this stream
    // used to verify whether a file flag can be written
    // into end of file and file data can be flush back.
//    LONG RefCount;

    // �ļ���Ч��С
    LARGE_INTEGER FileValidLength ;

    // �ļ�ʵ�ʴ�С �����˼���ͷ��
    LARGE_INTEGER FileSize ;

    // Trail Length
//  ULONG uTrailLength ;

    // desired access
//  ULONG uAccess ;

    // Flags
    BOOLEAN bIsFileCrypt ;    // (init false)set after file flag is written into end of file
    BOOLEAN bEncryptOnWrite ; // (init true)set when file is to be supervised, or set when file is already encrypted.
    BOOLEAN bDecryptOnRead ;  // (init false)set when non-encrypted file is first paging written, or set when file is already encrypted.
    BOOLEAN bHasWriteData ;    // (init false)If data is written into file during the life cycle of the stream context. This flag is used to judge whether to write tail flag when file is closed.
    BOOLEAN bFirstWriteNotFromBeg ; // (useless now.)if file is not encrypted yet, whether the first write offset is 0
    BOOLEAN bHasPPTWriteData ;  // (init false)If user click save button in un-encrypts ppt file, this flag is set to TRUE and this file will be encrypted in THE LAST IRP_MJ_CLOSE

    // Holds encryption/decryption context specified to this file
    ///COUNTER_MODE_CONTEXT* aes_ctr_ctx ;

    // Lock used to protect this context.
    PERESOURCE Resource;

    // Spin lock used to protect this context when irql is too high
    KSPIN_LOCK Resource1 ;

} STREAM_CONTEXT, *PSTREAM_CONTEXT;*/


////////////////////////
//     �궨��
////////////////////////
#define MEM_CALLBACK_TAG                'calb'
#define CALLBACK_IS_CACHED(_piopb)      (!(_piopb->IrpFlags&(IRP_NOCACHE)))?TRUE:FALSE

////////////////////////
//     ȫ�ֱ���
////////////////////////

// ��ص���������������
extern NPAGED_LOOKASIDE_LIST nliCallbackContextLookasideList;

////////////////////////
//      һ�㺯��
////////////////////////
BOOLEAN
InitializePostCallBackContext(
    __in POST_CALLBACK_FLAG pcFlags,
    __in_opt PVOID pData,
    __inout PPOST_CALLBACK_CONTEXT * dpccContext
    );

VOID
UninitializePostCallBackContext(
    __in PPOST_CALLBACK_CONTEXT pccContext
    );

////////////////////////
//     һ��ص�
////////////////////////

NTSTATUS
Antinvader_InstanceSetup (
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in FLT_INSTANCE_SETUP_FLAGS Flags,
    __in DEVICE_TYPE VolumeDeviceType,
    __in FLT_FILESYSTEM_TYPE VolumeFilesystemType
    );

NTSTATUS
Antinvader_InstanceQueryTeardown (
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    );

VOID
Antinvader_InstanceTeardownStart (
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

VOID
Antinvader_InstanceTeardownComplete (
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in FLT_INSTANCE_TEARDOWN_FLAGS Flags
    );

VOID
Antinvader_CleanupContext(
    __in PFLT_CONTEXT pcContext,
    __in FLT_CONTEXT_TYPE pctContextType
    );
////////////////////////
//  ���˻ص�
////////////////////////

// IRP_MJ_CREATE
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreCreate (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    );

FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostCreate (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

// IRP_MJ_CLOSE
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreClose (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    );

FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostClose (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

// IRP_MJ_READ
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreRead (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    );

FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostRead (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostReadWhenSafe (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in PVOID lpContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

// IRP_MJ_WRITE
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreWrite (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    );

FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostWrite (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

// IRP_MJ_CLEANUP
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreCleanUp (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    );

FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostCleanUp (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

// IRP_MJ_SET_INFORMATION
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreSetInformation (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    );

FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostSetInformation (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

// IRP_MJ_DIRECTORY_CONTROL
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreDirectoryControl (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    );

FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostDirectoryControl (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostDirectoryControlWhenSafe (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

// IRP_MJ_QUERY_INFORMATION
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreQueryInformation (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    );

FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostQueryInformation (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    );

/////////////////////////
//      ͨѶ�ص�
/////////////////////////
NTSTATUS
Antinvader_Connect(
    __in PFLT_PORT ClientPort,
    __in PVOID ServerPortCookie,
    __in_bcount(SizeOfContext) PVOID ConnectionContext,
    __in ULONG SizeOfContext,
    __deref_out_opt PVOID *ConnectionCookie
    );

VOID
Antinvader_Disconnect(
    __in_opt PVOID ConnectionCookie
   );


NTSTATUS
Antinvader_Message (
    __in PVOID ConnectionCookie,
    __in_bcount_opt(InputBufferSize) PVOID InputBuffer,
    __in ULONG InputBufferSize,
    __out_bcount_part_opt(OutputBufferSize,*ReturnOutputBufferLength) PVOID OutputBuffer,
    __in ULONG OutputBufferSize,
    __out PULONG ReturnOutputBufferLength
    );
