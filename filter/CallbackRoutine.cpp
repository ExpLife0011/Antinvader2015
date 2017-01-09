///////////////////////////////////////////////////////////////////////////////
///
/// ��Ȩ���� (c) 2011 - 2012
///
/// ԭʼ�ļ�����     : CallbackRoutine.cpp
/// ��������         : AntinvaderDriver
/// ����ʱ��         : 2011-03-20
///
///
/// ����             : Antinvader �ص�ʵ��
///
/// ����ά��:
///  0000 [2011-03-20] ����汾.
///
///////////////////////////////////////////////////////////////////////////////

#include "CallbackRoutine.h"
#include "AntinvaderDriver.h"
#include "ConfidentialFile.h"
#include "ConfidentialProcess.h"
#include "ProcessFunction.h"
#include "FileFunction.h"
#include "MiniFilterFunction.h"
#include "BasicAlgorithm.h"

// ��ص���������������
NPAGED_LOOKASIDE_LIST nliCallbackContextLookasideList;

////////////////////////////////////
//    һ�㺯��
////////////////////////////////////
/*---------------------------------------------------------
��������:   InitializePostCallBackContext
��������:   ��ʼ���ص�������
�������:
            pcFlags         �ص���־,���
                            POST_CALLBACK_FLAG����
            pData           ��Ҫ������,��ΪNULL
            dpccContext     �����ʼ���õ������ĵ�ַ�Ŀռ�
�������:
            dpccContext     ��ʼ���õ�������
����ֵ:
            TRUE �ɹ� ����ΪFALSE
����:
            ��ص�����Ҫʹ��UninitializePostCallBackContext
            �ͷ���������������Ŀռ�.

����ά��:   2011.5.1    ����汾
---------------------------------------------------------*/
BOOLEAN
InitializePostCallBackContext(
    __in POST_CALLBACK_FLAG pcFlags,
    __in_opt PVOID pData,
    __inout PPOST_CALLBACK_CONTEXT * dpccContext
    )
{
    // ����������ĵ�ַ�ռ�
    PPOST_CALLBACK_CONTEXT pccContext;

    //
    // ����ռ�
    //
    pccContext = (PPOST_CALLBACK_CONTEXT)ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(POST_CALLBACK_CONTEXT),
        MEM_CALLBACK_TAG);

    //
    // ����ռ�ʧ��
    //
    if (!pccContext) {
        return FALSE;
    }

    //
    // �������
    //
    pccContext->pcFlags = pcFlags;
    pccContext->pData = pData;

    //
    // ����
    //
    *dpccContext = pccContext;

    return TRUE;
}

/*---------------------------------------------------------
��������:   UninitializePostCallBackContext
��������:   ��ʼ���ص�������
�������:   dpccContext     ��ʼ���õ������ĵ�ַ
�������:
����ֵ:
����:       ���InitializePostCallBackContextʹ��
����ά��:   2011.5.1    ����汾
---------------------------------------------------------*/
VOID
UninitializePostCallBackContext(
    __in PPOST_CALLBACK_CONTEXT pccContext
    )
{
    ExFreePoolWithTag(pccContext, MEM_CALLBACK_TAG);
}

////////////////////////////////////
//    ���˻ص�����
////////////////////////////////////

/*---------------------------------------------------------
��������:   Antinvader_PreCreate
��������:   Ԥ����ص�IRP_MJ_CREATE
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
�������:
����ֵ:
            FLT_PREOP_SUCCESS_WITH_CALLBACK ʹ�ú�ص�
            FLT_PREOP_SUCCESS_NO_CALLBACK   �޺�ص�
����:
����ά��:   2011.3.20    ����汾
            2011.3.23    �ر�FastIo
            2011.4.9     �����˻��ܽ����ж�,���û���
                               �ļ���
            2011.4.13    �����˴����»���,�жϼ��ܱ�
                               ���ļ��Ƿ����
            2011.4.30    Bug:���ļ���ʱ����ֹ�ļ���,
                                   ���³���,������.
            2011.5.1     �޸��˻ص�������,�Ż���ִ��
                               ˳��.
            2011.7.8     Bug:FileIsEncrypted���޸���
                                   FLT_PARAMETERȴû��֪ͨ
                                   FLTMGR.������.
            2011.7.10   �ٴιر�FastIo
            2011.7.13   Ϊ�˼����´򿪵��ļ�FCB���
                              �������ļ����ܱ�ʱ���ƺ�Post
            2011.7.20   �޸���FileIsEncrypted����
            2011.7.27   �����������֧��
            2011.7.29   �޸��˴���������λ��
---------------------------------------------------------*/
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreCreate (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    )
{
    //
    // û���ļ�����
    //
    if (!pFltObjects->FileObject) {
        DebugTrace(DEBUG_TRACE_NORMAL_INFO, "PreCreate", ("No file object was found.pass now."));
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    //
    // ���ֻ�Ǵ�Ŀ¼ ֱ�ӷŹ�
    //
    if (pfcdCBD->Iopb ->Parameters.Create.Options & FILE_DIRECTORY_FILE) {

        DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO,
            "PreCreate",
            FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
            ("Just open directory.Pass now."));

        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

/*---------------------------------------------------------
��������:   Antinvader_PostCreate
��������:   ������ص�IRP_MJ_CREATE
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
            Flags               �ص������ı�־(ԭ��)
�������:
����ֵ:
            FLT_POSTOP_FINISHED_PROCESSING �ɹ�
����:
����ά��:   2011.3.20    ����汾
            2011.5.1     �޸��������Ľṹ,���������
                               �������ļ���Ĺ���.
            2011.7.13    Ϊ�˼����´򿪵��ļ�FCB���
                               �������ļ����ܱ�ʱ���ƺ�����
            2011.7.17    �����˻����ͷ�
            2011.7.29    ������޸� ���󲿷ֹ���ת�Ƶ�
                               ����
            2012.1.2     �������ͷŻ������
            2012.1.3     ȡ�����ͷŻ���,��ֹ���ܺͷǻ���
                               ����ͬʱ�򿪻����ļ�
            2012.1.3     Bug:�޸ĺ�������������������.
                                   ������.
            2012.1.3     Bug:�޷���ֹͬʱ����ͬ�ļ�..����ing
            2012.1.4     ������ֹ����ͬ�ļ�,רעˢ����
---------------------------------------------------------*/
FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostCreate (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    // Io����������
    PFLT_IO_PARAMETER_BLOCK pIoParameterBlock;

    // ����ֵ
    NTSTATUS status;

    // �ļ�����
    PFILE_OBJECT pfoFileObject;

    // ʵ��
    PFLT_INSTANCE pfiInstance;

    // �Ƿ��ǻ����ļ�
    BOOLEAN bIsEncrypted;

    // ����ͷ����
//  BOOLEAN bHeaderAuto = TRUE;

    // �µ����ļ�����
//  PFILE_OBJECT pfoStreamFileObject;

    // �򿪵��ļ�����
    PFILE_OBJECT pfoFileObjectOpened = NULL;

    // �ļ����
    HANDLE hFile = NULL;

    // ������Ȩ��
    ULONG ulDesiredAccess;

    // ��ȡ�����ļ�����
    PFILE_STREAM_CONTEXT pscFileStreamContext = NULL;

    // ��ǰ�Ƿ��ǻ��淽ʽ
    BOOLEAN bCached;

    // �Ƿ��������
    BOOLEAN bClearCache = FALSE;

    // ��������
    PVOLUME_CONTEXT pvcVolumeContext = 0;

    // �ļ�������Ϣ
    PFLT_FILE_NAME_INFORMATION pfniFileNameInformation = 0;

    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    //
    // ��ȡ��Ҫ�Ĳ���
    //
    pIoParameterBlock = pfcdCBD->Iopb;
    pfiInstance = pFltObjects->Instance;
    pfoFileObject = pFltObjects->FileObject;

    FltDecodeParameters(
        pfcdCBD,
        NULL,
        NULL,
        NULL,
        (LOCK_OPERATION *)&ulDesiredAccess);

    DebugTraceFileAndProcess(
        DEBUG_TRACE_ALL_IO,
        "PostCreate",
        FILE_OBJECT_NAME_BUFFER(pfoFileObject),
        ("PostCreate entered. FltIsOperationSynchronous: %d",
        FltIsOperationSynchronous(pfcdCBD)));

//  if (!IsCurrentProcessConfidential()) {
//      return FLT_POSTOP_FINISHED_PROCESSING;
//  }

    do {
        if (!NT_SUCCESS(pfcdCBD->IoStatus.Status)) {
            //
            // �����ʧ���� ��ô���ù���
            //
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PostCreate",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("File failed to open, pass now."));

            break;
        }

        //
        // ��ȡ��������
        //
        status = FltGetVolumeContext(
            pFltObjects->Filter,
            pFltObjects->Volume,
            (PFLT_CONTEXT *)&pvcVolumeContext);

        //
        // û���ܹ���ȡ�������� ����
        //
        if (!NT_SUCCESS(status)|| pvcVolumeContext == NULL) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PostCreate",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("No volume context was found, pass now."));
            break;
        }

        //
        // ��ȡ�ļ�������Ϣ
        //
        status = FltGetFileNameInformation(
            pfcdCBD,
            // FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
            FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_DEFAULT,
            &pfniFileNameInformation);

        if (!NT_SUCCESS(status)) {
            //
            // û�õ��ļ���Ϣ
            //
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PostCreate",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Cannot get file information, pass now."));

            pfniFileNameInformation = NULL;
            break;
        }

        if (!pfniFileNameInformation->Name.Length) {
            //
            // �ļ�������Ϊ0,���� ���ͷ�pfniFileNameInformation
            //
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PostCreate",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Zero name length, pass now."));

            break;
        }

        //
        // �ȽϾ����ƺʹ����������ͬ˵�����ڴ򿪾� �Ͳ�������
        //
        if (RtlCompareUnicodeString(&pfniFileNameInformation->Name,
                &pfniFileNameInformation->Volume, TRUE) == 0) {

            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PostCreate",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Openning volume, pass now."));

            break;
        }

        //
        // �����ļ��������� ����Ѿ����� ���ü������Զ���1
        //
        status = FctCreateContextForSpecifiedFileStream(
            pfiInstance,
            pfoFileObject,
            &pscFileStreamContext);

        if (status != STATUS_FLT_CONTEXT_ALREADY_DEFINED) {
            if (status == STATUS_NOT_SUPPORTED) {
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_ERROR,
                    "PostCreate",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("Error: File does not supported."));

                ASSERT(FALSE);
                break;
            }

            //
            // ���û�������� ��ô�³�ʼ��������
            //
            status = FctInitializeContext(
                pscFileStreamContext,
                pfcdCBD,
                pfniFileNameInformation);

            if (!NT_SUCCESS(status)) {
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_ERROR,
                    "PostCreate",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("Error: Cannot initialize fct context."));
                break ;
            }

            //
            // ���������Ҫ��ص��ļ�,����Ϊ����Ҫ���
            //
            /*
            if (!PctIsPostfixMonitored(&pscFileStreamContext->usPostFix)) {
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO,
                    "PostCreate",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("Current file postfix not monitored. PostFix: %ws. Length: %d",
                    pscFileStreamContext->usPostFix.Buffer,
                    pscFileStreamContext->usPostFix.Length));
                FctUpdateFileConfidentialCondition(
                    pscFileStreamContext,ENCRYPTED_TYPE_NOT_CONFIDENTIAL);
                break;
            }
            */

        } // if (status != STATUS_FLT_CONTEXT_ALREADY_DEFINED)

        //
        // ���ڶ�����������,����Ѿ��趨�ǻ����ļ�, ��ô�ͷŻ���, �˳�
        //
        if (FctGetFileConfidentialCondition(pscFileStreamContext) == ENCRYPTED_TYPE_CONFIDENTIAL) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_IMPORTANT_INFO|DEBUG_TRACE_CONFIDENTIAL,
                "PostCreate",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("File encripted.Now clear file cache. FCB: 0x%X",
                pfoFileObject->FsContext));

            FileClearCache(pfoFileObject);
            break;
        }

        //
        // ��֪���ǲ����µĻ����ļ�(û�л��ܽ��̴򿪹�),�����Ƿǻ��ܽ��̷���,��ôֱ�ӷŹ�
        //
        if ((FctGetFileConfidentialCondition(pscFileStreamContext) == ENCRYPTED_TYPE_NOT_CONFIDENTIAL)
            && (!IsCurrentProcessConfidential())) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PostCreate",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("None confiditial process create file not encripted. Pass now."));
            break;

        // } else if (pscFileStreamContext->fctEncrypted != ENCRYPTED_TYPE_CONFIDENTIAL) {
        } else {
            //
            // ���ڵ������ һ������֮ǰ�򿪹��Ļ����ļ� һ�����Ƿǻ����ļ��ͷǻ��ܽ��̵����
            // �����ǻ��ܽ��� - �ǻ����ļ�/�´򿪵��ļ� �� �ǻ��ܽ��� - �´򿪵��ļ�
            //
            // �����֪�������ǲ��ǻ����ļ� ��ô���ļ���һ�� ����һ���ǻ��ܽ���
            // �˴����ܵ������,֮ǰĳ����������һ�������ļ�,�����Ļ�����
            // ��ʱ�����Ƿǻ��ܽ�������������ENCRYPTED_TYPE_NOT_CONFIDENTIAL
            // �������¼��.������ENCRYPTED_TYPE_UNKNOWN ������Ӧ�ü��
            //

            status = FileIsEncrypted(
                pfiInstance,
                pfoFileObject,
                pfcdCBD,
                pvcVolumeContext,
                pscFileStreamContext,
                NULL);

            if (status == STATUS_FILE_NOT_ENCRYPTED) {

                DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO,
                    "PostCreate",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("File tested and not encripted. Set now."));

                FctUpdateFileConfidentialCondition(pscFileStreamContext, ENCRYPTED_TYPE_NOT_CONFIDENTIAL);
                break;
            }

            //
            // ���ļ�,����Զ����������ͷ˵��Ҳ�޸Ĺ�Parameter,ʹ��Dirty
            //
            if (status == STATUS_REPARSE_OBJECT) {
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                    "PostCreate",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("New file. Head has been wrriten.Set drity now. FCB: 0x%X",
                    pfoFileObject->FsContext));

                FltSetCallbackDataDirty(pfcdCBD);
                status = STATUS_SUCCESS;

                FctUpdateFileConfidentialCondition(pscFileStreamContext, ENCRYPTED_TYPE_CONFIDENTIAL);

                break;
            } else if (!NT_SUCCESS(status)) {

                DebugTraceFileAndProcess(
                    DEBUG_TRACE_IMPORTANT_INFO,
                    "PostCreate",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("File cannot be tested. Process access denied."));

                //
                // ʧ����,��ֹ����
                //
                FltCancelFileOpen(pFltObjects->Instance, pfoFileObject);

                //
                // ���÷�����Ϣ
                //
                pfcdCBD->IoStatus.Status = STATUS_ACCESS_DENIED;
                pfcdCBD->IoStatus.Information = 0;

                FctUpdateFileConfidentialCondition(pscFileStreamContext, ENCRYPTED_TYPE_NOT_CONFIDENTIAL);

                /* Access shouldn't be denied. */
                ASSERT(FALSE);
                break;
            }

            //
            // ������ܳɹ���ȡ���������ͷ ��Ϊ�Ƿǻ����ļ�
            //
            if (!NT_SUCCESS(FileReadEncryptionHeaderAndDeconstruct(
                pfiInstance,
                pfoFileObject,
                pvcVolumeContext,
                pscFileStreamContext))) {
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
                    "PostCreate",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("Error: Cannot read entire file encryption header."));

                FctUpdateFileConfidentialCondition(pscFileStreamContext, ENCRYPTED_TYPE_NOT_CONFIDENTIAL);

                break;
            } else {
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                    "PostCreate",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("Confidential file head read. Valid length %d",
                    pscFileStreamContext->nFileValidLength.QuadPart));

                FctUpdateFileConfidentialCondition(pscFileStreamContext, ENCRYPTED_TYPE_CONFIDENTIAL);

                FileClearCache(pfoFileObject);
                break;
            }
        }
    } while (0);

    //
    // �ƺ���
    //
    if (NULL != pscFileStreamContext) {
        FctReleaseStreamContext(pscFileStreamContext) ;
    }

    if (pfoFileObjectOpened) {
        ObDereferenceObject(pfoFileObjectOpened);
    }

    if (hFile != NULL) {
        FltClose(hFile);
    }

    if (pfniFileNameInformation) {
        FltReleaseFileNameInformation(pfniFileNameInformation);
    }

    if (pvcVolumeContext) {
        FltReleaseContext(pvcVolumeContext);
    }

    if (pfoFileObjectOpened) {
        ObDereferenceObject(pfoFileObjectOpened);
    }

    DebugTraceFileAndProcess(
        DEBUG_TRACE_NORMAL_INFO,
        "PostCreate",
        FILE_OBJECT_NAME_BUFFER(pfoFileObject),
        ("All finished. ioStatus: 0x%X",pfcdCBD->IoStatus.Status));

    return FLT_POSTOP_FINISHED_PROCESSING;
}

/*---------------------------------------------------------
��������:   Antinvader_PreClose
��������:   Ԥ����ص�IRP_MJ_CLOSE
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
�������:
����ֵ:
����:
����ά��:   2011.3.20    ����汾
            2011.7.10    ���ȡ�����ü���
            2011.7.12    �޸���Pre��Post�Ĺ��ܰ�ȡ��
                               ���ü����ŵ���Post��
---------------------------------------------------------*/
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreClose (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    )
{
    // Io����������
    PFLT_IO_PARAMETER_BLOCK pIoParameterBlock;

    // ʵ��
    PFLT_INSTANCE pfiInstance;

    // �򿪵��ļ�����
    PFILE_OBJECT pfoFileObject;

    // ����ֵ
    BOOLEAN bReturn;

    // �Ƿ����ļ���
    BOOLEAN bDirectory;

    // ����ֵ
    NTSTATUS status;

    // �ļ���������
    PFILE_STREAM_CONTEXT pscFileStreamContext = NULL;

    // ��������
    PVOLUME_CONTEXT pvcVolumeContext = NULL;

    // ����Ĵ�С
    LARGE_INTEGER nAllocateSize;

    // �ļ���С
    LARGE_INTEGER nFileSize;

    // �򿪵��ļ����
    HANDLE hFile = NULL;

    // ���´򿪵��ļ�����
    PFILE_OBJECT pfoFileObjectOpened = NULL;

    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    //
    // ��ȡ��Ҫ�Ĳ���
    //
    pIoParameterBlock = pfcdCBD->Iopb;
    pfiInstance = pIoParameterBlock->TargetInstance;
    pfoFileObject = pIoParameterBlock->TargetFileObject;    // pFltObjects->FileObject;

    DebugTraceFileAndProcess(
        DEBUG_TRACE_ALL_IO,
        "PreClose",
        FILE_OBJECT_NAME_BUFFER(pfoFileObject),
        ("PreClose entered."));

    //
    // ��ȡ�ļ�������Ϣ
    //
    status = FileGetStandardInformation(
        pfiInstance,
        pfoFileObject,
        &nAllocateSize,
        &nFileSize,
        &bDirectory);
    //
    // ʧ�ܻ�����Ŀ¼��ֱ�ӷ���
    //
    if (!NT_SUCCESS(status)) {
        DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO,
            "PreClose",
            FILE_OBJECT_NAME_BUFFER(pfoFileObject),
            ("Cannot get file imformation."));

        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    if (bDirectory) {
        DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO,
            "PreClose",
            FILE_OBJECT_NAME_BUFFER(pfoFileObject),
            ("Closing a dictory. Pass now."));

        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    do {
        //
        // ��ȡ��������
        //
        status = FltGetVolumeContext(
            pFltObjects->Filter,
            pFltObjects->Volume,
            (PFLT_CONTEXT *)&pvcVolumeContext);

        if (!NT_SUCCESS(status)) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreClose",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Cannot get volume context. Pass now."));

            pvcVolumeContext = NULL;
            break;
        }

        //
        // ��ȡ�ļ���������
        //
        status = FctGetSpecifiedFileStreamContext(
            pfiInstance,
            pfoFileObject,
            &pscFileStreamContext);

        if (!NT_SUCCESS(status)) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreClose",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Cannot get file context. Pass now."));

            pscFileStreamContext = NULL;
            break;
        }

        //
        // ����򿪵��ǻ����ļ� // ˢ������ ���ǻ����bug ntfs�ļ�ϵͳ����
        //
        if (FctGetFileConfidentialCondition(pscFileStreamContext) == ENCRYPTED_TYPE_CONFIDENTIAL) {
            //
            // ��������ļ��Ļ�˵����ϵͳ�ڲ��򿪵�,���ǲ����� ����������ü�����1
            //
            if ((pFltObjects->FileObject->Flags & FO_STREAM_FILE) != FO_STREAM_FILE) {
                    DebugTraceFileAndProcess(
                        DEBUG_TRACE_NORMAL_INFO,
                        "PreClose",
                        FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                        ("Not a stream file.Dereference now."));

                    FctDereferenceFileContext(pscFileStreamContext);
                }

            DebugTraceFileAndProcess(
                DEBUG_TRACE_IMPORTANT_INFO|DEBUG_TRACE_CONFIDENTIAL,
                "PreClose",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Closeing an encrypted file. Valid size %d",
                pscFileStreamContext->nFileValidLength));

            //
            // ��������ȫ���ͷ� �����Ҫ����¼���ͷ
            //
            if ((FctIsReferenceCountZero(pscFileStreamContext))&&(FctIsUpdateWhenCloseFlag(pscFileStreamContext))) {
                //
                // �ֶ����ļ�
                //
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                    "PreClose",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("All closed now refresh file encryption header."));

                status = FileCreateForHeaderWriting(pfiInstance, &pscFileStreamContext->usName, &hFile);

                if (!NT_SUCCESS(status)) {
                    DebugTraceFileAndProcess(
                        DEBUG_TRACE_ERROR,
                        "PreClose",
                        FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                        ("Error: Cannot open file. Status: 0x%X", status));
                    ASSERT(FALSE);
                    break;
                }

                status = ObReferenceObjectByHandle(
                    hFile,
                    STANDARD_RIGHTS_ALL,
                    *IoFileObjectType,
                    KernelMode,
                    (PVOID *)&pfoFileObjectOpened,
                    NULL);

                if (!NT_SUCCESS(status)) {
                    DebugTraceFileAndProcess(
                        DEBUG_TRACE_ERROR,
                        "PreClose",
                        FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                        ("Error: Cannot reference object. 0x%X", status));
                    ASSERT(FALSE);
                    break;
                }

                //
                // ��д����ͷ
                //
                status = FileWriteEncryptionHeader(
                    pfiInstance,
                    pfoFileObjectOpened,
                    pvcVolumeContext,
                    pscFileStreamContext);

                if (!NT_SUCCESS(status)) {
                    DebugTraceFileAndProcess(
                        DEBUG_TRACE_ERROR,
                        "PreClose",
                        FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                        ("Error: Cannot update file header. Status: 0x%X", status));
                    ASSERT(FALSE);
                    break;
                }

                FctSetUpdateWhenCloseFlag(pscFileStreamContext, FALSE);

                //
                // д�����ͷŻ���
                //
                FileClearCache(pfoFileObject);

                FctReleaseStreamContext(pscFileStreamContext) ;
                // FctFreeStreamContext(pscFileStreamContext);

                FltDeleteContext(pscFileStreamContext);
                pscFileStreamContext = NULL;
            }

            break;
        }
    } while (0);

    //
    // ����
    //
    if (pfoFileObjectOpened) {
        ObDereferenceObject(pfoFileObjectOpened);
    }

    if (hFile != NULL) {
        FltClose(hFile);
    }

    if (pscFileStreamContext != NULL) {
        FctReleaseStreamContext(pscFileStreamContext);
    }

    if (pvcVolumeContext != NULL) {
        FltReleaseContext(pvcVolumeContext) ;
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

/*---------------------------------------------------------
��������:   Antinvader_PostClose
��������:   ������ص�IRP_MJ_CLOSE
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
            Flags               �ص������ı�־(ԭ��)
�������:
����ֵ:
����:
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostClose (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

//  DebugPrintFileStreamContext("Deleteing",((PFILE_STREAM_CONTEXT)lpCompletionContext));

//  FctDereferenceFileStreamContextObject( (PFILE_STREAM_CONTEXT)lpCompletionContext);

    return FLT_POSTOP_FINISHED_PROCESSING;// STATUS_SUCCESS;
}

/*---------------------------------------------------------
��������:   Antinvader_PreRead
��������:   Ԥ����ص�IRP_MJ_READ
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
�������:
����ֵ:
            FLT_PREOP_SUCCESS_WITH_CALLBACK ����
����:
����ά��:   2011.3.20    ����汾
            2011.4.2     ����˳������˴���
            2011.7.13    ������ƫ�����޸�,��ʱ����
                               OffsetΪ�ռ����յ�ǰƫ�ƶ�
                               ȡ�����.
            2011.7.17    ���������ͷŻ��� �޸��˽ṹ
            2011.7.29    ����������޸�
---------------------------------------------------------*/
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreRead (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    )
{
    UNREFERENCED_PARAMETER(pFltObjects);

    // I/O������,����IRP�����Ϣ
    PFLT_IO_PARAMETER_BLOCK  pIoParameterBlock;

    // �ļ�����
    PFILE_OBJECT pfoFileObject;

    // ƫ����
    PLARGE_INTEGER pliOffset;

    // ����ֵ
    BOOLEAN bReturn;

    // ��ǰ�Ƿ��ǻ����ȡ��ʽ
    BOOLEAN bCachedNow;

    // ����ֵ
    NTSTATUS status;

    // �ļ���������
    PFILE_STREAM_CONTEXT pscFileStreamContext = NULL;

    // ��������
    PVOLUME_CONTEXT pvcVolumeContext = NULL;

    // ����������ֵ
    FLT_PREOP_CALLBACK_STATUS fcsStatus = FLT_PREOP_SUCCESS_NO_CALLBACK ;

    // �ļ���Ч����
    LARGE_INTEGER nFileValidSize;

    // ��������ַ
    ULONG ulBuffer;

    // �ɵĻ�������ַ
    ULONG ulOriginalBuffer;

    // ����������
    ULONG ulDataLength;

    // �µ�Mdl
    PMDL pMemoryDescribeList;

    // �ļ�����

    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    //
    // ��ȡ��Ҫ������
    //
    pIoParameterBlock = pfcdCBD->Iopb;

    pfoFileObject = pFltObjects->FileObject;

    DebugTraceFileAndProcess(
        DEBUG_TRACE_ALL_IO,
        "PreRead",
        FILE_OBJECT_NAME_BUFFER(pfoFileObject),
        ("PreRead entered."));

    //
    // ���û�н���������,��ô�����Ĵ���NULL
    //
    *lpCompletionContext = NULL;

    do {
        //
        // ���ǻ��ܽ��� ֱ�ӷ���
        //
        if (!IsCurrentProcessConfidential()) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreRead",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Not confidential process. Pass now."));
            break;
        }

        //
        // ��ȡ�ļ���������
        //
        status = FctGetSpecifiedFileStreamContext(
            pFltObjects->Instance,
            pfoFileObject,
            &pscFileStreamContext);

        if (!NT_SUCCESS(status)) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreRead",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("No file context find. Reguarded as not confidential file."));

            pscFileStreamContext = NULL;
            break;
        }

        //
        // ��ȡ��������
        //
        status = FltGetVolumeContext(
            pFltObjects->Filter,
            pFltObjects->Volume,
            (PFLT_CONTEXT *)&pvcVolumeContext);

        if (!NT_SUCCESS(status)) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreRead",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("No volum context find. Reguarded as not confidential file."));

            pvcVolumeContext = NULL;
            break;
        }
        //
        // ���δ���� ֱ�ӷ���
        //
        if (FctGetFileConfidentialCondition(pscFileStreamContext) != ENCRYPTED_TYPE_CONFIDENTIAL) {
            // DebugPrintFileObject("Antinvader_PreRead not confidential.",pfoFileObject,bCachedNow);
            DebugTraceFileAndProcess(
                DEBUG_TRACE_IMPORTANT_INFO,
                "PreRead",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Confidential proccess read an not confidential file. File enctype: %d",
                FctGetFileConfidentialCondition(pscFileStreamContext)));
            break;
        }

        //
        // �����������
        //
        if (pIoParameterBlock->Parameters.Read.Length == 0) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreRead",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Zero byte to read.Pass now."));
            break;
        }

        //
        // ����ֻ����IRP_NOCACHE,IRP_PAGING_IO��IRP_SYNCHRONOUS_PAGING_IO ����ȫ���Ź�
        //
        if (!(pIoParameterBlock->IrpFlags & (IRP_NOCACHE | IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO))) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreRead",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Not the operation we intrested.Pass now."));
            break ;
        }

        //
        // �����ȡ�ĳ��ȳ������ļ���Ч����,����EOF
        //
        FctGetFileValidSize(pscFileStreamContext, &nFileValidSize);

        if (pIoParameterBlock->Parameters.Read.ByteOffset.QuadPart >= nFileValidSize.QuadPart) {
            pfcdCBD->IoStatus.Status = STATUS_END_OF_FILE;
            pfcdCBD->IoStatus.Information = 0;
            fcsStatus = FLT_PREOP_COMPLETE;
            break;
        }

        //
        // ����Ƿǻ����д,��ô��д���ȱ������(�����������ŵ�AllocateAndSwapToNewMdlBuffer��)
        //
//        if (pIoParameterBlock->IrpFlags & IRP_NOCACHE) {
//
//           pIoParameterBlock->Parameters.Read.Length =
//              (ULONG)ROUND_TO_SIZE(pIoParameterBlock->Parameters.Read.Length,pvcVolumeContext->ulSectorSize);
//        }

        //
        // ��ֹFastIo
        //
        if (FLT_IS_FASTIO_OPERATION(pfcdCBD)) {
            fcsStatus = FLT_PREOP_DISALLOW_FASTIO;

            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PreRead",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Disallow fast io."));

            break ;
        }

        DebugTraceFileAndProcess(
            DEBUG_TRACE_ALL_IO|DEBUG_TRACE_CONFIDENTIAL,
            "PreRead",
            FILE_OBJECT_NAME_BUFFER(pfoFileObject),
            ("A confidential file and a confidential process encounterd .Enc: %d,",
            pscFileStreamContext->fctEncrypted));

        //
        // ����ƫ������ַ
        //
        pliOffset = &pIoParameterBlock->Parameters.Read.ByteOffset;

        if ((pliOffset->LowPart == FILE_USE_FILE_POINTER_POSITION) && (pliOffset->HighPart == -1)) {
            //
            // ��ʱ���԰��յ�ǰƫ���������
            //
            DebugTraceFileAndProcess(
                DEBUG_TRACE_ALL_IO|DEBUG_TRACE_CONFIDENTIAL,
                "PreRead",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Ignore %s tries to read by current postion.",
                CURRENT_PROCESS_NAME_BUFFER));
            ASSERT(FALSE);
        }

        //
        // IRP�����Ļ���������
        //
        if (pfcdCBD->Flags & FLTFL_CALLBACK_DATA_IRP_OPERATION) {
            bReturn = AllocateAndSwapToNewMdlBuffer(
                pIoParameterBlock,
                pvcVolumeContext,
                (PULONG)lpCompletionContext,
                &pMemoryDescribeList,
                &ulBuffer,
                &ulDataLength,
                Allocate_BufferRead);

            if (!bReturn) {
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
                    "PreRead",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("Error: Cannot swap buffer."));
            }

            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PreRead",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Swap buffer finished."));
        }
        //
        // �޸�ƫ��
        //
        DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
            "PreRead",
            FILE_OBJECT_NAME_BUFFER(pfoFileObject),
            ("PreRead modifing offset. Original offset: %d ",pliOffset->QuadPart));

        pliOffset->QuadPart += CONFIDENTIAL_FILE_HEAD_SIZE;
        fcsStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
    } while (0);

    //
    // �����޸�
    //
    FltSetCallbackDataDirty(pfcdCBD);

    if (pscFileStreamContext != NULL) {
        FctReleaseStreamContext(pscFileStreamContext) ;
    }

    if (pvcVolumeContext != NULL) {
        FltReleaseContext(pvcVolumeContext) ;
    }

    return  fcsStatus;
}

/*---------------------------------------------------------
��������:   Antinvader_PostRead
��������:   ������ص�IRP_MJ_READ
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
            Flags               �ص������ı�־(ԭ��)
�������:
����ֵ:
            FLT_POSTOP_FINISHED_PROCESSING �ɹ�
����:
����ά��:   2011.3.20    ����汾
            2011.4.2     ����˳������˴���
            2011.4.3     ������Ƿ���Ҫ�ͷ��ڴ���ж�
            2011.7.10    ����˻���ͷǻ��淽ʽ����
            2011.7.30    �޸�������Do when safe
---------------------------------------------------------*/
FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostRead (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    // ����ֵ
    NTSTATUS status;

    // ����ֵ
    BOOLEAN  bReturn;

    // ������Mdl��ַ
    PMDL *dpMdlAddressPointer;

    // ���ݻ����ַ
    PVOID  *dpBuffer;

    // ���峤��
    PULONG pulLength;

    // ����ʹ�õķ��ʻ���ķ�ʽ
    LOCK_OPERATION loDesiredAcces;

    // I/O������,����IRP�����Ϣ
    PFLT_IO_PARAMETER_BLOCK  pIoParameterBlock;

    //
    //
    // һЩ΢������Ϊ��ĳЩ�������뽻������.����һ��΢������
    // ʵ�ּ����㷨,��һ���ǻ���(non-cached)IRP_MJ_READ,��һ
    // ���ϣ���ѻ����е����ݽ���.ͬ������д��ʱ��,��ϣ������
    // �ݼ���.�����������:�����޷�������ռ��м���.��Ϊ����
    // IRP_MJ_WRITE,���΢����������ֻ��IoreadAccessȨ��.���
    // ΢���������������Լ����ж�дȨ�޵Ļ�����ȡ��ԭ�Ļ�����.
    // ������ԭ�������е����ݺ�д���»�������,�ټ�������I/O����.
    //
    //

    // ��������ַ
    ULONG ulBuffer;

    // ����������
    ULONG ulDataLength;

    // �µ�Mdl
    PMDL pMemoryDescribeList;

    // ��ǰ�Ƿ��ǻ���״̬
    BOOLEAN bCached;

    // �ļ�����
    PFILE_OBJECT pfoFileObject;

    // ����������ֵ
    FLT_POSTOP_CALLBACK_STATUS fcsStatus = FLT_POSTOP_FINISHED_PROCESSING;

    // �µĻ���
    ULONG ulSwappedBuffer;

    // �ļ���������
    PFILE_STREAM_CONTEXT pscFileStreamContext = NULL;

    // �ļ���Ч��С
    LARGE_INTEGER nFileValidLength;
    //
    // ��������ȡ��װ΢���˻ص�����,���ʧ�ܾͷ���.
    //

    status = FltDecodeParameters(
            pfcdCBD,
            &dpMdlAddressPointer,
            &dpBuffer,
            &pulLength,
            &loDesiredAcces);

    if (!NT_SUCCESS(status)) {
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    //
    // ��ȡ�ļ�����,iopb,������ʵ��,�ص������ĵ�
    //
    pIoParameterBlock = pfcdCBD->Iopb;
    pfoFileObject = pFltObjects->FileObject;

    ulSwappedBuffer = (ULONG)lpCompletionContext;

    DebugTraceFileAndProcess(DEBUG_TRACE_ALL_IO | DEBUG_TRACE_CONFIDENTIAL,
        "PostRead",
        FILE_OBJECT_NAME_BUFFER(pfoFileObject),
        ("PostRead entered."));
    //
    // ����Ԥ�������Ѿ�������Ƿ��ǻ��ܽ���,���ﲻ���ٴμ��.
    //

    //
    // ��һ��ʵ����ж����ʱ��,���˹��������ܵ��ú������ص�,
    // ���Ǵ�ʱ������δ������.��ʱ,��־FLTFL_POST_OPERATION_DRAINING
    // ������.��ʱ�ṩ�˾����ٵ���Ϣ.����΢������Ӧ��������
    // �еĴ�Ԥ�����д����Ĳ���������,������FLT_POSTOP_FINISHED_PROCESSING.
    //

    if (Flags & FLTFL_POST_OPERATION_DRAINING) {
        ASSERT(FALSE);
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    //
    // �������ʧ���� ֱ�ӷ���
    //
    if (!NT_SUCCESS(pfcdCBD->IoStatus.Status) || (pfcdCBD->IoStatus.Information == 0)) {

        DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
            "PostRead",
            FILE_OBJECT_NAME_BUFFER(pfoFileObject),
            ("Read faild. Pass now"));

        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    //
    // ��ȡ�ļ��������� ����ȡ�Ĵ�С
    //
    status = FctGetSpecifiedFileStreamContext(
        pFltObjects->Instance,
        pfoFileObject,
        &pscFileStreamContext);

    if (!NT_SUCCESS(status)) {
        DebugTraceFileAndProcess(
            DEBUG_TRACE_ERROR,
            "PostRead",
            FILE_OBJECT_NAME_BUFFER(pfoFileObject),
            ("Error: Cannot get file context in post operation."));

        ASSERT(FALSE);
        pscFileStreamContext = NULL;
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    //
    // ��ȡ�ļ���Ч��С ����������ĳ��ȳ�����Ч��С,��ô����
    // ���ﴫ���offset�Ǳ�����offset�������޸Ĺ���,������
    // ������ƫ�������϶����ĳ��Ⱥ�valid size���ñȽ�,��������
    //
    FctGetFileValidSize(pscFileStreamContext,&nFileValidLength);

    if (nFileValidLength.QuadPart < (pIoParameterBlock->Parameters.Read.ByteOffset.QuadPart
        + pfcdCBD->IoStatus.Information)) {
        pfcdCBD->IoStatus.Information = (ULONG)(nFileValidLength.QuadPart
            - pIoParameterBlock->Parameters.Read.ByteOffset.QuadPart);
    }
    //
    // ������Ҫ�Ѷ����������ݿ������û���������.ע��
    // ���������pfcdCBD��ԭʼ��(û��������ǰ)���û�
    // ����,һֱ�����˿�����..�������ǵÿ���
    //

    //
    // ��ȡԭʼ��ַ
    //
    if (pIoParameterBlock->Parameters.Read.MdlAddress) {
        DebugTraceFileAndProcess(
            DEBUG_TRACE_DATA_OPERATIONS | DEBUG_TRACE_CONFIDENTIAL,
            "PostRead",
            FILE_OBJECT_NAME_BUFFER(pfoFileObject),
            ("Using mdl buffer."));

        ulBuffer = (ULONG)MmGetSystemAddressForMdlSafe(
            pIoParameterBlock->Parameters.Read.MdlAddress,
            NormalPagePriority);

        //
        // ����ò���Mdl ���ش���
        //
        if (!ulBuffer) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
                "PostRead",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Error: Could not get MDL address."));

            pfcdCBD->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            pfcdCBD->IoStatus.Information = 0;
            return FLT_POSTOP_FINISHED_PROCESSING;
        }
    }
    else if ((pfcdCBD->Flags & FLTFL_CALLBACK_DATA_SYSTEM_BUFFER) ||
             (pfcdCBD->Flags & FLTFL_CALLBACK_DATA_FAST_IO_OPERATION)) {
        //
        // ����� FastIo ������ʹ�� System Buffer ��������, ��ô Buffer λ����
        // Parameters.Read.ReadBuffer ����
        //
        ulBuffer = (ULONG)pIoParameterBlock->Parameters.Read.ReadBuffer;

        DebugTraceFileAndProcess(
                    DEBUG_TRACE_DATA_OPERATIONS|DEBUG_TRACE_CONFIDENTIAL,
                    "PostRead",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("Using system buffer."));
    } else {
        //
        // ������� ������һЩ������û����� �ܿ�����Paged Pool�й�
        // ������DPC�½����޸�
        //
        DebugTraceFileAndProcess(
                    DEBUG_TRACE_IMPORTANT_INFO|DEBUG_TRACE_CONFIDENTIAL,
                    "PostRead",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("Completing processing when safe."));

        if (!FltDoCompletionProcessingWhenSafe(
            pfcdCBD,
            pFltObjects,
            lpCompletionContext,
            Flags,
            Antinvader_PostReadWhenSafe,
            &fcsStatus)) {
            //
            // û��ת����ȫ��IRQL ���ش���
            //
            pfcdCBD->IoStatus.Status = STATUS_UNSUCCESSFUL;
            pfcdCBD->IoStatus.Information = 0;
            return FLT_POSTOP_FINISHED_PROCESSING;
        }
        return fcsStatus;
    }

    //
    // ��ȡ����
    //
    ulDataLength = pfcdCBD->IoStatus.Information;

    //
    // ִ�н���ܲ���
    //
    DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
            "PostRead",
            FILE_OBJECT_NAME_BUFFER(pfoFileObject),
            ("Start deconde. Length: %d", ulDataLength)
  );

    //
    // ����ʹ��,ֱ�����0x77
    //
    __try {
        for (ULONG i = 0; i < ulDataLength; i++) {
            *((char *)(ulSwappedBuffer + i)) = *((char *)(ulSwappedBuffer + i)) ^ 0x77;
        }
    } __finally {
        //
        // �����Ѿ����޸� ���ñ�־λ ������
        //
//      FltSetCallbackDataDirty(pfcdCBD);
    }

    //
    // �����ݿ�����ԭ���Ļ���
    //
    if (ulSwappedBuffer) {
        RtlCopyMemory((PVOID)ulBuffer, (PVOID)ulSwappedBuffer, pfcdCBD->IoStatus.Information);
    }

    //
    // ȡ������
    //
//  FctDereferenceFileStreamContextObject((PFILE_STREAM_CONTEXT)lpCompletionContext);

    // FltSetCallbackDataDirty(pfcdCBD);

    FltSetCallbackDataDirty(pfcdCBD);

    if (ulSwappedBuffer) {
        FreeAllocatedMdlBuffer(ulSwappedBuffer, Allocate_BufferRead);
    }

    if (pscFileStreamContext != NULL) {
        FctReleaseStreamContext(pscFileStreamContext) ;
    }

    return FLT_POSTOP_FINISHED_PROCESSING;// STATUS_SUCCESS;
}

/*---------------------------------------------------------
��������:   Antinvader_PostReadWhenSafe
��������:   ��ȫʱRead��ص�
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
            Flags               ��־λ
�������:
����ֵ:

����:
����ά��:   2011.7.30    ����汾
---------------------------------------------------------*/
FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostReadWhenSafe (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    // ������
    ULONG ulBuffer;

    // ����ֵ
    NTSTATUS status;

    // ���ݳ���
    ULONG ulDataLength;

    // �µĻ���
    ULONG ulSwappedBuffer;

    DebugTraceFileAndProcess(
        DEBUG_TRACE_ALL_IO|DEBUG_TRACE_CONFIDENTIAL,
        "PostReadWhenSafe",
        FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
        ("PostReadWhenSafe entered."));

    //
    // ִ�е�����˵���ǲ���MDL���û����� ��ס(Ҳ���Ǵ���һ��MDL)
    //
    status = FltLockUserBuffer( pfcdCBD);

    if (!NT_SUCCESS(status)) {
        //
        // ������ ���ش�����Ϣ
        //
        DebugTraceFileAndProcess(
            DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
            "PostReadWhenSafe",
            FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
            ("Error: Could not lock MDL address."));

        pfcdCBD->IoStatus.Status = status;
        pfcdCBD->IoStatus.Information = 0;

        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    //
    // ��ȡ��ַ
    //
    ulBuffer = (ULONG)MmGetSystemAddressForMdlSafe(
        pfcdCBD->Iopb->Parameters.Read.MdlAddress,
        NormalPagePriority);

    if (!ulBuffer) {
        //
        // ������ ���ش�����Ϣ
        //
        DebugTraceFileAndProcess(
            DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
            "PostReadWhenSafe",
            FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
            ("Error:Could not get MDL address."));
        pfcdCBD->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        pfcdCBD->IoStatus.Information = 0;

        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    //
    // ��ȡ���� �µĻ���
    //
    ulDataLength = pfcdCBD->IoStatus.Information;
    ulSwappedBuffer = (ULONG)lpCompletionContext;

    //
    // ִ�н���ܲ���
    //
    DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
            "PostReadWhenSafe",
            FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
            ("Start deconde. Length:%d", ulDataLength));

    /*
    __try {
        for (ULONG i = 0; i < ulDataLength; i++) {
            *((char *)(ulBuffer + i)) = *((char *)(ulBuffer + i)) ^ 0x77;
        }
    } __finally {
        //
        // �����Ѿ����޸� ���ñ�־λ ������
        //
        FltSetCallbackDataDirty(pfcdCBD);
    }
    */

    //
    // �����ݿ�����ԭ���Ļ���
    //
    RtlCopyMemory((PVOID)ulBuffer, (PVOID)ulSwappedBuffer, pfcdCBD->IoStatus.Information);

    FltSetCallbackDataDirty(pfcdCBD);
    FreeAllocatedMdlBuffer(ulSwappedBuffer, Allocate_BufferRead);

    return FLT_POSTOP_FINISHED_PROCESSING;
}

/*---------------------------------------------------------
��������:   Antinvader_PreWrite
��������:   Ԥ����ص�IRP_MJ_WRITE
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
�������:
����ֵ:

����:
����ά��:   2011.3.20    ����汾
            2011.4.3     �Ż��˻ص� ����˳�������
            2012.1.2     ������IRP�����˹��� ��������
---------------------------------------------------------*/
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreWrite (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    )
{
    UNREFERENCED_PARAMETER(pFltObjects);

    // ����ֵ
    NTSTATUS status;

    // ����ֵ
    BOOLEAN  bReturn;

    // ��ǰ�Ƿ񻺴�
    BOOLEAN bCachedNow;

    // ����������ֵ Ĭ�ϲ���Ҫ�ص�
    FLT_PREOP_CALLBACK_STATUS pcStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;

    // ������Mdl��ַ
    PMDL *dpMdlAddressPointer;

    // ���ݻ����ַ
    PVOID  *dpBuffer;

    // ���峤��
    PULONG pulLength;

    // ����ʹ�õķ��ʻ���ķ�ʽ
    LOCK_OPERATION loDesiredAcces;

    // I/O������,����IRP�����Ϣ
    PFLT_IO_PARAMETER_BLOCK  pIoParameterBlock;

    //
    //
    // һЩ΢������Ϊ��ĳЩ�������뽻������.����һ��΢������
    // ʵ�ּ����㷨,��һ���ǻ���(non-cached)IRP_MJ_READ,��һ
    // ���ϣ���ѻ����е����ݽ���.ͬ������д��ʱ��,��ϣ������
    // �ݼ���.�����������:�����޷�������ռ��м���.��Ϊ����
    // IRP_MJ_WRITE,���΢����������ֻ��IoreadAccessȨ��.���
    // ΢���������������Լ����ж�дȨ�޵Ļ�����ȡ��ԭ�Ļ�����.
    // ������ԭ�������е����ݺ�д���»�������,�ټ�������I/O����.
    //
    //

    // ��������ַ
    ULONG ulBuffer;

    // �ɵĻ�������ַ
    ULONG ulOriginalBuffer;

    // ����������
    ULONG ulDataLength;

    // �µ�Mdl
    PMDL pMemoryDescribeList;

    // �ļ�����
    PFILE_OBJECT pfoFileObject;

    // ƫ����
    PLARGE_INTEGER  pliOffset;

    // �ļ���С
    LARGE_INTEGER   nFileSize;

    // �ļ���������
    PFILE_STREAM_CONTEXT pscFileStreamContext = NULL;

    // ��������
    PVOLUME_CONTEXT pvcVolumeContext = NULL;

    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    //
    // ��������
    //
    pIoParameterBlock = pfcdCBD->Iopb;
    pfoFileObject = pFltObjects->FileObject;
    *lpCompletionContext = NULL;

    DebugTraceFileAndProcess(
        DEBUG_TRACE_ALL_IO,
        "PreWrite",
        FILE_OBJECT_NAME_BUFFER(pfoFileObject),
        ("PreWrite entered."));

    //
    // ����Ƿ��ǻ��ܽ���
    //
    do {
        //
        // ��ȡ��������
        //
        status = FltGetVolumeContext(
            pFltObjects->Filter,
            pFltObjects->Volume,
            (PFLT_CONTEXT *)&pvcVolumeContext);

        if (!NT_SUCCESS(status)) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreWrite",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("No volum context find. Reguarded as not confidential file."));
            pvcVolumeContext = NULL;
            break;
        }

        //
        // ��ȡ�ļ���������
        //
        status = FctGetSpecifiedFileStreamContext(
            pFltObjects->Instance,
            pfoFileObject,
            &pscFileStreamContext);

        if (!NT_SUCCESS(status)) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreWrite",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("No file context find. Reguarded as not confidential file."));
            pscFileStreamContext = NULL;
            break;
        }

        if (!IsCurrentProcessConfidential()) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreWrite",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Not confidential process. Pass now."));
            break;
        }

        //
        // ��������������,��Щֻ���ļ�����ᷢ��
        // ��ҳ/������д����,��д�벿���ļ�(������
        // Ҳ�����������ô����,�ǲ���д��ҳ�ļ�?)
        // �����������,�����ļ������һ����������
        // �Ķ�һ�������ĵ�����Ӷ����ļ�.������
        // ���Ƿ���Ҫ���м��ܲ���ʱ��д��Ȩ�޽�����
        // �ж�,�Ա�֤������������ᷢ��.
        //
        if (FctGetFileConfidentialCondition(pscFileStreamContext) != ENCRYPTED_TYPE_CONFIDENTIAL) {
            // || !pfoFileObject->WriteAccess) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,    // | DEBUG_TRACE_CONFIDENTIAL,
                "PreWrite",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("The file type we are not interested in."));
            break;
        }

        //
        // ��ȡ��װ΢���˻ص����� ���ʧ�ܾͷ���
        //
        status = FltDecodeParameters(
                pfcdCBD,
                &dpMdlAddressPointer,
                &dpBuffer,
                &pulLength,
                &loDesiredAcces);

        if (!NT_SUCCESS(status)) {
            pcStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
            break;
        }

        //
        // ��ֹFastIo
        //
        if (FLT_IS_FASTIO_OPERATION(pfcdCBD)) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PreWrite",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Disallow fast io."));

            pcStatus = FLT_PREOP_DISALLOW_FASTIO ;
            break;
        }

        //
        // ����ƫ������ַ
        //
        pliOffset = &pIoParameterBlock->Parameters.Write.ByteOffset;

        //
        // ֻ����IRP_NOCACHE IRP_PAGING_IO ���� IRP_SYNCHRONOUS_PAGING_IO
        //
        if (!(pIoParameterBlock->IrpFlags & (IRP_NOCACHE | IRP_PAGING_IO | IRP_SYNCHRONOUS_PAGING_IO))) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreWrite",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("The operation we are not interested in."));

            //
            // ��ʱ�ļ�����ͨ�����滺��io��չ,��������Ҫ��¼�ļ���С
            //
            nFileSize.QuadPart = pliOffset->QuadPart + pIoParameterBlock->Parameters.Write.Length;

            if (FctUpdateFileValidSizeIfLonger(pscFileStreamContext, &nFileSize, TRUE)) {
                nFileSize.QuadPart += CONFIDENTIAL_FILE_HEAD_SIZE;

                if (!NT_SUCCESS(FileSetSize(pFltObjects->Instance, pfoFileObject, &nFileSize))) {
                    DebugTraceFileAndProcess(
                        DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
                        "PreWrite",
                        FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                        ("Error: Cannot set file size."));
                }
                else {
                    DebugTraceFileAndProcess(
                        DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                        "PreWrite",
                        FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                        ("Set size to %d.", nFileSize.QuadPart));
                }
            }

            pcStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
            break;
        }

        if ((pliOffset->LowPart == FILE_USE_FILE_POINTER_POSITION) && (pliOffset->HighPart == -1)) {
            //
            // ��ʱ���԰��յ�ǰƫ���������
            //
            DebugTraceFileAndProcess(
                DEBUG_TRACE_ALL_IO|DEBUG_TRACE_CONFIDENTIAL,
                "PreWrite",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Ignore %s tries to read by current postion.",
                CURRENT_PROCESS_NAME_BUFFER));
            ASSERT(FALSE);
        }

        DebugTraceFileAndProcess(
            DEBUG_TRACE_ALL_IO|DEBUG_TRACE_CONFIDENTIAL,
            "PreWrite",
            FILE_OBJECT_NAME_BUFFER(pfoFileObject),
            ("Confidential enter: Length %d offset %d.",
            pIoParameterBlock->Parameters.Write.Length,
            pIoParameterBlock->Parameters.Write.ByteOffset));

        //
        // �޸�ƫ�����ڶ���ʱ���Ѿ��޸Ĺ�ƫ���� ����ҲҪ�޸�
        //
        pliOffset->QuadPart += CONFIDENTIAL_FILE_HEAD_SIZE;

        //
        // �����Ҫ���л������
        //
        if (loDesiredAcces != IoModifyAccess) {
            //
            // �����»��� ���ʧ�ܾͷ���
            //
            bReturn = AllocateAndSwapToNewMdlBuffer(
                pIoParameterBlock,
                pvcVolumeContext,
                (PULONG)lpCompletionContext,
                &pMemoryDescribeList,
                NULL,   // &ulBuffer,
                &ulDataLength,
                Allocate_BufferWrite);

            ulBuffer = *(PULONG)lpCompletionContext;
            //
            // �����ں�����ص����滻���ɵĻ����MDL.
            // ���˹������Զ�ִ����Щ������ʵ����΢��
            // �����ں�����ص��е�Iopb�м�������ռ�
            // ��MDL���滻ǰ�ġ�΢����
            // �������Լ����������м�¼�µĻ�������
            //
            if (!bReturn) {
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
                    "PreWrite",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("Error: Cannot allocate new mdl buffer."));

                pcStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
                break;
            }

            pcStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
        } else {
            //
            // ��ȡԭʼ��ַ
            //
            if (pIoParameterBlock->Parameters.Write.MdlAddress) {
                ulOriginalBuffer = (ULONG)MmGetSystemAddressForMdlSafe(
                        pIoParameterBlock->Parameters.Write.MdlAddress,
                        NormalPagePriority);
            } else {
                ulOriginalBuffer = (ULONG)pIoParameterBlock->Parameters.Write.WriteBuffer;
            }

            ulDataLength = pIoParameterBlock->Parameters.Write.Length;

            //
            // ֱ��ʹ��ԭʼ��ַ
            //
            ulBuffer = ulOriginalBuffer;
        }

        //
        // ִ�мӼ��ܲ���
        //
        DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
            "PreWrite",
            FILE_OBJECT_NAME_BUFFER(pfoFileObject),
            ("Start encrypt. Length: %d", ulDataLength));

        pcStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;

        //
        // ����ʹ�� ֱ�����0x77
        //
        __try {

            for (ULONG i = 0; i < ulDataLength; i++) {
                *((char *)(ulBuffer + i)) = *((char *)(ulBuffer + i)) ^ 0x77;
            }

        } __finally {
            //
            // �����Ѿ����޸� ���ñ�־λ ������
            //
            // FltSetCallbackDataDirty(pfcdCBD);
        }

        DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
            "PreWrite",
            FILE_OBJECT_NAME_BUFFER(pfoFileObject),
            ("Filt finished. Length: %d, offset: %d",
            pIoParameterBlock->Parameters.Write.Length,
            pIoParameterBlock->Parameters.Write.ByteOffset));
    } while (0);

    //
    // �����Ѿ����޸� ���ñ�־λ ������
    //
    FltSetCallbackDataDirty(pfcdCBD);

    //
    // �ͷ�������
    //
    if (pscFileStreamContext != NULL) {
        FctReleaseStreamContext(pscFileStreamContext) ;
    }

    if (pvcVolumeContext != NULL) {
        FltReleaseContext(pvcVolumeContext) ;
    }

    return pcStatus;
}

/*---------------------------------------------------------
��������:   Antinvader_PostWrite
��������:   ������ص�IRP_MJ_WRITE
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
            Flags               �ص������ı�־(ԭ��)
�������:
����ֵ:

����:
����ά��:   2011.3.20    ����汾
            2011.4.3     ����˳�������,��δ�ָ�Mdl��ַ
            2011.7.10    �޸������Ĵ�������,�ָ���Mdl
            2011.7.15    �����Ķ����ĵ� ȥ����mdl����
            2011.7.22    ���������ü�������
---------------------------------------------------------*/
FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostWrite (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    // I/O������,����IRP�����Ϣ
    PFLT_IO_PARAMETER_BLOCK  pIoParameterBlock;

    // �ļ���������
    PFILE_STREAM_CONTEXT pscFileStreamContext;

    // ��ʼд���ƫ����
    LARGE_INTEGER nByteOffset;

    // ���ļ���С
    LARGE_INTEGER nFileNewSize;

    // �ɹ�д��ĳ���
    ULONG ulWrittenBytes;

    // ���ӵĳ���
    LONGLONG llAddedBytes;

    // �µĻ���
    ULONG ulSwappedBuffer;

    // ����״̬
    NTSTATUS status ;
    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    //
    // ��ȡ�ļ���������
    //
    status = FctGetSpecifiedFileStreamContext(
        pFltObjects->Instance,
        pFltObjects->FileObject,
        &pscFileStreamContext);

    if (!NT_SUCCESS(status)) {
        DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO,
            "PostWrite",
            FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
            ("No file context find. Reguarded as not confidential file."));
        pscFileStreamContext = NULL;
        ASSERT(FALSE);
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    //
    // ���lpCompletionContextΪ0��ʾû��������ڴ�,�����ͷ�
    // �ص��Ѿ����Ż�,��������ڴ��Ż��д˻ص�.��һ��ʵ����
    // ж����ʱ��,���˹��������ܵ��ú������ص�,���Ǵ�ʱ����
    // ��δ������.��ʱ,��־FLTFL_POST_OPERATION_DRAINING
    // ������.��ʱ�ṩ�˾����ٵ���Ϣ.����΢������Ӧ��������
    // �еĴ�Ԥ�����д����Ĳ���������,������FLT_POSTOP_FINISHED_PROCESSING.
    //
    if (Flags & FLTFL_POST_OPERATION_DRAINING) {
        if (lpCompletionContext) {
            FctReleaseStreamContext(pscFileStreamContext) ;
        }
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    //
    // ����Iopb ƫ���� д�볤��
    //
    pIoParameterBlock = pfcdCBD->Iopb;
    nByteOffset       = pIoParameterBlock->Parameters.Write.ByteOffset ;
    ulWrittenBytes    = pfcdCBD->IoStatus.Information ;
    ulSwappedBuffer   = (ULONG)lpCompletionContext;

    DebugTraceFileAndProcess(
        DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
        "PostWrite",
        FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
        ("Confidential entered. status: 0x%X, Original valid: %d, Offset %d, Bytes wrriten: %d.",
        pfcdCBD->IoStatus.Status, (int)pscFileStreamContext->nFileValidLength.QuadPart,
        (int)nByteOffset.QuadPart, ulWrittenBytes));

//  nFileNewSize.QuadPart = pscFileStreamContext->nFileValidLength.QuadPart + CONFIDENTIAL_FILE_HEAD_SIZE;

/*  if (!NT_SUCCESS(FileSetSize(pFltObjects->Instance,pFltObjects->FileObject,&nFileNewSize))) {
        DebugTraceFileAndProcess(
            DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
            "PostWrite",
            FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
            ("Cannot update file length to %d.",nFileNewSize.QuadPart)
  );
    }*/
    //
    // ���д��ĳ��ȳ������ļ�ԭ����ValidLength ��ô˵�����ȼӳ���
    // �����޸��ļ���Ч����
    //
/*
    FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);

    if ((llAddedBytes =
        (nByteOffset.QuadPart + (LONGLONG)ulWrittenBytes) -
        pscFileStreamContext->nFileValidLength.QuadPart) > 0) {

            pscFileStreamContext->nFileValidLength.QuadPart =
                nByteOffset.QuadPart + (LONGLONG)ulWrittenBytes;

            pscFileStreamContext->nFileSize.QuadPart += llAddedBytes;

            pscFileStreamContext->bUpdateWhenClose = TRUE;

    //      nFileNewSize.QuadPart = pscFileStreamContext->nFileValidLength.QuadPart + CONFIDENTIAL_FILE_HEAD_SIZE;

        DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
            "PostWrite",
            FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
            ("Update file valid size to %d.", pscFileStreamContext->nFileValidLength.QuadPart)
  );
    }

    FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);
*/
    /*
    if (pscFileStreamContext->bUpdateWhenClose) {
        //
        // ��Ҫ��ˢ����˵���ļ���С�䶯.������������.
        //
        if (!NT_SUCCESS(FileSetSize(pFltObjects->Instance,pFltObjects->FileObject,&nFileNewSize))) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
                "PostWrite",
                FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                ("Cannot update file length to %d.", nFileNewSize.QuadPart)
      );
        }
    }
    */
    if (ulSwappedBuffer) {
        FreeAllocatedMdlBuffer(ulSwappedBuffer,Allocate_BufferWrite);
    }

    // �鿴�Ƿ���Ҫ���»���
    if (lpCompletionContext) {
        FctReleaseStreamContext(pscFileStreamContext) ;
    }

    return FLT_POSTOP_FINISHED_PROCESSING;// STATUS_SUCCESS;
}

/*---------------------------------------------------------
��������:   Antinvader_PreSetInformation
��������:   Ԥ����ص�IRP_MJ_SET_INFORMATION
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
�������:
����ֵ:

����:
����ά��:   2011.3.20    ����汾
            2011.7.12    �޸��˴�С ��֤�˼���ͷ����
---------------------------------------------------------*/
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreSetInformation (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    )
{
    // �ļ�����
    PFILE_OBJECT pfoFileObject;

    // I/O������,����IRP�����Ϣ
    PFLT_IO_PARAMETER_BLOCK  pIoParameterBlock;

    // �ļ���Ϣ���
    FILE_INFORMATION_CLASS  ficFileInformation;

    // �ļ���Ϣ
    PVOID pFileInformation;

    // ���ݴ�С
    ULONG ulLength;

    // ����ֵ
    BOOLEAN bReturn;

    // ����ֵ
    NTSTATUS status;

    // �ļ���������
    PFILE_STREAM_CONTEXT pscFileStreamContext = NULL;

    // ��������
    PVOLUME_CONTEXT pvcVolumeContext = NULL;

    // �Ƿ��޸���Ҫ����д�����ͷ
    BOOLEAN bUpdateFileEncryptionHeader = TRUE;

    // ����ֵ
    FLT_PREOP_CALLBACK_STATUS pcsStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;

    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    //
    // ����Iopb���ļ���Ϣָ��
    //
    pIoParameterBlock = pfcdCBD->Iopb;

    ficFileInformation = pIoParameterBlock->
        Parameters.SetFileInformation.FileInformationClass;

    pFileInformation = pIoParameterBlock->
        Parameters.SetFileInformation.InfoBuffer;

    ulLength = pIoParameterBlock->
        Parameters.SetFileInformation.Length;

    pfoFileObject = pFltObjects->FileObject;

    //
    // ����Ƿ��ǻ��ܽ���
    //
    do {
//      if (!IsCurrentProcessConfidential()) {
//          return FLT_PREOP_SUCCESS_NO_CALLBACK;
//      }

        //
        // ��ȡ�ļ���������
        //
        status = FctGetSpecifiedFileStreamContext(
            pFltObjects->Instance,
            pfoFileObject,
            &pscFileStreamContext);

        if (!NT_SUCCESS(status)) {
            pscFileStreamContext = NULL;
            bUpdateFileEncryptionHeader = FALSE;
            break;
        }

        //
        // ��ȡ��������
        //
        status = FltGetVolumeContext(
            pFltObjects->Filter,
            pFltObjects->Volume,
            (PFLT_CONTEXT *)&pvcVolumeContext);

        if (!NT_SUCCESS(status)) {
            pvcVolumeContext = NULL;
            bUpdateFileEncryptionHeader = FALSE;
            break;
        }

        //
        // ���δ���� ֱ�ӷ���
        //
        if (FctGetFileConfidentialCondition(pscFileStreamContext) != ENCRYPTED_TYPE_CONFIDENTIAL) {
            bUpdateFileEncryptionHeader = FALSE;
            break;
        }

        if (!IsCurrentProcessConfidential()) {
            /*
            if (!IsCurrentProcessSystem()) {
                bUpdateFileEncryptionHeader = FALSE;
                break;
            }
            */
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PreSetInformation",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("An non confidential process operate on a confidential file. FCB: 0x%X",
                pfoFileObject->FsContext));
/*
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PreSetInformation",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("System operate on confidential file. Access granted. FCB: 0x%X",
                pfoFileObject->FsContext));
*/
            break;
        }

        //
        // ��ֹ FAST IO
        //
        if (FLT_IS_FASTIO_OPERATION(pfcdCBD)) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PreSetInformation",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Disallow fast io."));
            pcsStatus = FLT_PREOP_DISALLOW_FASTIO ;
            break;
        }

        DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
            "PreSetInformation",
            FILE_OBJECT_NAME_BUFFER(pfoFileObject),
            ("Confidential entered, Valid file size %d, file size %d.",
            pscFileStreamContext->nFileValidLength,
            pscFileStreamContext->nFileSize));

        //
        // ��ȡ�ļ�����
        //
        pfoFileObject = pFltObjects->FileObject;

        //
        // ��ӡ������Ϣ
        //

        // DebugPrintFileObject("Antinvader_PreSetInformation",pfoFileObject,CALLBACK_IS_CACHED(pIoParameterBlock));
        //
        // ��������,�������
        //
        switch (ficFileInformation) {
        case FileAllInformation:
            //
            //
            // FileAllInformation,�������½ṹ���.��ʹ���Ȳ���,
            // ��Ȼ���Է���ǰ����ֽ�.
            // typedef struct _FILE_ALL_INFORMATION {
            //    FILE_BASIC_INFORMATION BasicInformation;
            //    FILE_STANDARD_INFORMATION StandardInformation;
            //    FILE_INTERNAL_INFORMATION InternalInformation;
            //    FILE_EA_INFORMATION EaInformation;
            //    FILE_ACCESS_INFORMATION AccessInformation;
            //    FILE_POSITION_INFORMATION PositionInformation;
            //    FILE_MODE_INFORMATION ModeInformation;
            //    FILE_ALIGNMENT_INFORMATION AlignmentInformation;
            //    FILE_NAME_INFORMATION NameInformation;
            // } FILE_ALL_INFORMATION, *PFILE_ALL_INFORMATION;
            // ������Ҫע����Ƿ��ص��ֽ����Ƿ������StandardInformation
            // �������Ӱ���ļ��Ĵ�С����Ϣ.
            //
            //
            {
                PFILE_ALL_INFORMATION paiFileInformation = (PFILE_ALL_INFORMATION)pFileInformation;
                //
                // ���������StandardInformation��ô�����޸�
                //
                if (ulLength >= sizeof(FILE_BASIC_INFORMATION) + sizeof(FILE_STANDARD_INFORMATION)) {
                    DebugTraceFileAndProcess(
                        DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                        "PreSetInformation",
                        FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                        ("FileAllInformation Entered, EndOfFile original %d",
                        paiFileInformation->StandardInformation.EndOfFile.QuadPart));

                    //
                    // ������Ч���� ʵ�ʳ���
                    //
                    FctUpdateFileValidSize(pscFileStreamContext,&paiFileInformation->StandardInformation.EndOfFile,TRUE);

                    //
                    // �޸ĳ���
                    //
                    paiFileInformation->StandardInformation.AllocationSize.QuadPart =
                        ROUND_TO_SIZE(paiFileInformation->StandardInformation.AllocationSize.QuadPart + CONFIDENTIAL_FILE_HEAD_SIZE,
                        pvcVolumeContext->ulSectorSize);

                    paiFileInformation->StandardInformation.EndOfFile.QuadPart += CONFIDENTIAL_FILE_HEAD_SIZE;

                    //
                    // �������PositionInformation��ô�޸�
                    //
                    if (ulLength >= sizeof(FILE_BASIC_INFORMATION) +
                        sizeof(FILE_STANDARD_INFORMATION) +
                        sizeof(FILE_INTERNAL_INFORMATION) +
                        sizeof(FILE_EA_INFORMATION) +
                        sizeof(FILE_ACCESS_INFORMATION) +
                        sizeof(FILE_POSITION_INFORMATION)) {
                        paiFileInformation->PositionInformation.CurrentByteOffset.QuadPart
                            += CONFIDENTIAL_FILE_HEAD_SIZE;
                    }
                }
                break;
            }

        case FileAllocationInformation:
            {
                PFILE_ALLOCATION_INFORMATION palloiFileInformation = (PFILE_ALLOCATION_INFORMATION)pFileInformation;

                DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                    "PreSetInformation",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("FileAllocationInformation Entered, AllocationSize original %d.",
                    palloiFileInformation->AllocationSize.QuadPart));

                //
                // �޸ĳ���
                //
                palloiFileInformation->AllocationSize.QuadPart =
                    ROUND_TO_SIZE(palloiFileInformation->AllocationSize.QuadPart + CONFIDENTIAL_FILE_HEAD_SIZE,
                    pvcVolumeContext->ulSectorSize);

                break;
            }
            //
            // ����ͬ�� �޸�֮
            //
        case FileValidDataLengthInformation:
            {
                PFILE_VALID_DATA_LENGTH_INFORMATION pvliInformation = (PFILE_VALID_DATA_LENGTH_INFORMATION)pFileInformation;

                DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                    "PreSetInformation",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("FileValidDataLengthInformation Entered, AllocationSize original %d.",
                    pvliInformation->ValidDataLength.QuadPart));

                //
                // ������Ч���� ʵ�ʳ���
                //
                FctUpdateFileValidSize(pscFileStreamContext, &pvliInformation->ValidDataLength, TRUE);

                pvliInformation->ValidDataLength.QuadPart += CONFIDENTIAL_FILE_HEAD_SIZE;

                break;
            }
        case FileStandardInformation:
            {
                PFILE_STANDARD_INFORMATION psiFileInformation = (PFILE_STANDARD_INFORMATION)pFileInformation;

                DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                    "PreSetInformation",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("FileStandardInformation Entered, AllocationSize EndOfFile original: %d.",
                    psiFileInformation->EndOfFile.QuadPart));

                //
                // ������Ч���� ʵ�ʳ���
                //
                FctUpdateFileValidSize(pscFileStreamContext,&psiFileInformation->EndOfFile, TRUE);

                //
                // �޸ĳ���
                //
                psiFileInformation->AllocationSize.QuadPart =
                    ROUND_TO_SIZE(psiFileInformation->AllocationSize.QuadPart + CONFIDENTIAL_FILE_HEAD_SIZE,
                    pvcVolumeContext->ulSectorSize);

                psiFileInformation->EndOfFile.QuadPart += CONFIDENTIAL_FILE_HEAD_SIZE;

                break;
            }
        case FileEndOfFileInformation:
            {
                PFILE_END_OF_FILE_INFORMATION peofInformation = (PFILE_END_OF_FILE_INFORMATION)pFileInformation;

                DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                    "PreSetInformation",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("FileEndOfFileInformation Entered, EndOfFile original: %d.",
                    peofInformation->EndOfFile.QuadPart));

                //
                // ������Ч���� ʵ�ʳ���
                //
                FctUpdateFileValidSize(pscFileStreamContext,&peofInformation->EndOfFile, TRUE);

                //
                // �޸ĳ���
                //
                peofInformation->EndOfFile.QuadPart += CONFIDENTIAL_FILE_HEAD_SIZE;

                break;
            }
        case FilePositionInformation:
            {
                PFILE_POSITION_INFORMATION ppiInformation = (PFILE_POSITION_INFORMATION)pFileInformation;
                ppiInformation->CurrentByteOffset.QuadPart += CONFIDENTIAL_FILE_HEAD_SIZE;

                DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                    "PreSetInformation",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("FilePositionInformation Entered, EndOfFile original: %d.",
                    ppiInformation->CurrentByteOffset.QuadPart));

                break;
            }

        case FileRenameInformation:
        case FileNameInformation:
        case FileNamesInformation:
            {
                //
                // ʹ�ú�ص����»���ļ���
                //
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                    "PreSetInformation",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("Name operation entered, using post callback to update name."));

                pcsStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
                *lpCompletionContext = pscFileStreamContext;
                pscFileStreamContext = NULL;

                bUpdateFileEncryptionHeader = FALSE;

                break;
            }
       default:
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PreSetInformation",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("default Entered, encouterd something we do not concern. Type code: %d",
                ficFileInformation));

            bUpdateFileEncryptionHeader = FALSE;
            //
            // ��֪����ʲô
            //
            // ASSERT(FALSE);
        }
    } while (0);

    if (bUpdateFileEncryptionHeader) {
        FctSetUpdateWhenCloseFlag(pscFileStreamContext,TRUE);
    }

    if (pscFileStreamContext != NULL) {
        FctReleaseStreamContext(pscFileStreamContext) ;
    }

    if (pvcVolumeContext != NULL) {
        FltReleaseContext(pvcVolumeContext) ;
    }

    return pcsStatus;
}

/*---------------------------------------------------------
��������:   Antinvader_PostSetInformation
��������:   ������ص�IRP_MJ_SET_INFORMATION
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
            Flags               �ص������ı�־(ԭ��)
�������:
����ֵ:

����:
����ά��:   2011.3.20    ����汾
            2011.7.21    ������FctDereferenceFileStreamContextObject
            2011.8.
---------------------------------------------------------*/
FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostSetInformation (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    // �ļ���������
    PFILE_STREAM_CONTEXT pscFileStreamContext = (PFILE_STREAM_CONTEXT)lpCompletionContext;

    // �ļ�����Ϣ
    PFLT_FILE_NAME_INFORMATION pfniFileNameInformation = NULL;

    // ״̬
    NTSTATUS status;

    do {
        //
        // ��ȡ�ļ�������Ϣ
        //
        status = FltGetFileNameInformation(pfcdCBD,
            FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_DEFAULT,
            &pfniFileNameInformation);

        if (!NT_SUCCESS(status)) {
            //
            // û�õ��ļ���Ϣ
            //
            DebugTraceFileAndProcess(
                DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
                "PostSetInformation",
                FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                ("Error: Cannot get file information, pass now."));

            pfniFileNameInformation = NULL;
            break;
        }

        DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
            "PostSetInformation",
            FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
            ("Successfully get new name. Updating now."));

        FctUpdateStreamContextFileName(&pfniFileNameInformation->Name,pscFileStreamContext);
    } while (0);

    if (pfniFileNameInformation != NULL) {
        FltReleaseFileNameInformation(pfniFileNameInformation);
    }

    if (pscFileStreamContext != NULL) {
        FctReleaseStreamContext(pscFileStreamContext) ;
    }

    return FLT_POSTOP_FINISHED_PROCESSING;  // STATUS_SUCCESS;
}

/*---------------------------------------------------------
��������:   Antinvader_PreQueryInformation
��������:   Ԥ����ص�IRP_MJ_QUERY_INFORMATION
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
�������:
����ֵ:

����:
����ά��:   2011.3.20    ����汾
            2011.7.12    ����˹��� �ж��Ƿ��޸Ĵ�С
---------------------------------------------------------*/
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreQueryInformation (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    )
{
    // ����ֵ
    NTSTATUS status;

    // �ļ���������
    PFILE_STREAM_CONTEXT pscFileStreamContext = NULL;

    // ��������
    PVOLUME_CONTEXT pvcVolumeContext = NULL;

    // ��������
    FLT_PREOP_CALLBACK_STATUS pcStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;

    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    //
    // ��ͬ������ �Ź�
    //
//  if (FltIsOperationSynchronous(pfcdCBD)) {
//      return FLT_PREOP_SUCCESS_WITH_CALLBACK;
//  }

    do {
        //
        // ����Ƿ��ǻ��ܽ���
        //
        if (!IsCurrentProcessConfidential()) {
            pcStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
            break;
        }
        //
        // ��ȡ�ļ���������
        //
        status = FctGetSpecifiedFileStreamContext(
            pFltObjects->Instance,
            pFltObjects->FileObject,
            &pscFileStreamContext);

        if (!NT_SUCCESS(status)) {
            pcStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
            pscFileStreamContext = NULL;
            break;
        }

        //
        // ��ȡ��������
        //
        status = FltGetVolumeContext(
            pFltObjects->Filter,
            pFltObjects->Volume,
            (PFLT_CONTEXT *)&pvcVolumeContext);

        if (!NT_SUCCESS(status)) {
            pcStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
            pvcVolumeContext = NULL;
            break;
        }

        //
        // ���δ���� ֱ�ӷ���
        //
        if (FctGetFileConfidentialCondition(pscFileStreamContext) != ENCRYPTED_TYPE_CONFIDENTIAL) {
            pcStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreSetInformation",
                FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                ("not confidential file, pass now."));
            break;
        }

        //
        // ��ֹ FAST IO
        //
        if (FLT_IS_FASTIO_OPERATION(pfcdCBD))
        {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PreQueryInformation",
                FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                ("Disallow fast io."));
            pcStatus = FLT_PREOP_DISALLOW_FASTIO ;
            break;
        }

        //
        // ���ļ��������Ĵ���ȥ ��ΪNULL��ʾ�����ͷ�
        //
        *lpCompletionContext = pscFileStreamContext;
        pscFileStreamContext = NULL;
    } while (0);

    if (pscFileStreamContext != NULL) {
        FctReleaseStreamContext(pscFileStreamContext) ;
    }

    if (pvcVolumeContext != NULL) {
        FltReleaseContext(pvcVolumeContext) ;
    }

    return pcStatus;
}

/*---------------------------------------------------------
��������:   Antinvader_PostQueryInformation
��������:   ������ص�IRP_MJ_QUERY_INFORMATION
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
            Flags               �ص������ı�־(ԭ��)
�������:
����ֵ:

����:
����ά��:   2011.3.20    ����汾
            2011.7.10    ������޸Ĵ�С��Ϣ
---------------------------------------------------------*/
FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostQueryInformation (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    // I/O������,����IRP�����Ϣ
    PFLT_IO_PARAMETER_BLOCK  pIoParameterBlock;

    // �ļ���Ϣ���
    FILE_INFORMATION_CLASS  ficFileInformation;

    // �ļ���Ϣ
    PVOID   pFileInformation;

    // ���ݳ���
    ULONG ulLength;

    // ���ļ�������
    PFILE_STREAM_CONTEXT pscFileStreamContext;

    // �ļ�����
    PFILE_OBJECT    pfoFileObject;

    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    //
    // ����Iopb���ļ���Ϣָ�� �����ĵ�
    //
    pIoParameterBlock = pfcdCBD->Iopb;

    ficFileInformation = pIoParameterBlock->
        Parameters.QueryFileInformation.FileInformationClass;

    pFileInformation = pIoParameterBlock->
        Parameters.QueryFileInformation.InfoBuffer;

    ulLength = pIoParameterBlock->
        Parameters.SetFileInformation.Length;

    pscFileStreamContext = (PFILE_STREAM_CONTEXT)lpCompletionContext;

    pfoFileObject = pFltObjects->FileObject;

    DebugTraceFileAndProcess(
        DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
        "PostQueryInformation",
        FILE_OBJECT_NAME_BUFFER(pfoFileObject),
        ("Confidential entered, Valid file size: %d",
        pscFileStreamContext->nFileValidLength));

    //
    // ��������,�������
    //
    switch (ficFileInformation) {
    case FileAllInformation:
        //
        //
        // FileAllInformation,�������½ṹ���.��ʹ���Ȳ���,
        // ��Ȼ���Է���ǰ����ֽ�.
        // typedef struct _FILE_ALL_INFORMATION {
        //    FILE_BASIC_INFORMATION BasicInformation;
        //    FILE_STANDARD_INFORMATION StandardInformation;
        //    FILE_INTERNAL_INFORMATION InternalInformation;
        //    FILE_EA_INFORMATION EaInformation;
        //    FILE_ACCESS_INFORMATION AccessInformation;
        //    FILE_POSITION_INFORMATION PositionInformation;
        //    FILE_MODE_INFORMATION ModeInformation;
        //    FILE_ALIGNMENT_INFORMATION AlignmentInformation;
        //    FILE_NAME_INFORMATION NameInformation;
        // } FILE_ALL_INFORMATION, *PFILE_ALL_INFORMATION;
        // ������Ҫע����Ƿ��ص��ֽ����Ƿ������StandardInformation
        // �������Ӱ���ļ��Ĵ�С����Ϣ.
        //
        //
        {
            PFILE_ALL_INFORMATION paiFileInformation = (PFILE_ALL_INFORMATION)pFileInformation;

            //
            // ���������StandardInformation��ô�����޸�
            //
            if (ulLength >= sizeof(FILE_BASIC_INFORMATION) + sizeof(FILE_STANDARD_INFORMATION)) {
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                    "PostQueryInformation",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("FileAllInformation Entered, EndOfFile original: %d, valid length: %d.",
                    paiFileInformation->StandardInformation.EndOfFile.QuadPart,
                    pscFileStreamContext->nFileValidLength.QuadPart));

                //
                // ���Դ�Сһ���ᳬ�������һ������ͷ
                //
                ASSERT(paiFileInformation->StandardInformation.EndOfFile.QuadPart >= CONFIDENTIAL_FILE_HEAD_SIZE
                    || paiFileInformation->StandardInformation.EndOfFile.QuadPart == 0);

#ifdef DBG
                //
                // �������Ǽ�¼���ļ���Ϣû����
                //
                // FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);
                // ASSERT(pscFileStreamContext->nFileValidLength.QuadPart == paiFileInformation->StandardInformation.EndOfFile.QuadPart);
                // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);
#endif

//              paiFileInformation->StandardInformation.AllocationSize.QuadPart-= CONFIDENTIAL_FILE_HEAD_SIZE;

                FctGetFileValidSize(pscFileStreamContext,&paiFileInformation->StandardInformation.EndOfFile);
                // paiFileInformation->StandardInformation.EndOfFile.QuadPart = pscFileStreamContext->nFileValidLength.QuadPart;
                // paiFileInformation->StandardInformation.EndOfFile.QuadPart-= CONFIDENTIAL_FILE_HEAD_SIZE;

                //
                // �������PositionInformation��ô�޸�
                //
                if (ulLength >= sizeof(FILE_BASIC_INFORMATION) +
                    sizeof(FILE_STANDARD_INFORMATION) +
                    sizeof(FILE_INTERNAL_INFORMATION) +
                    sizeof(FILE_EA_INFORMATION) +
                    sizeof(FILE_ACCESS_INFORMATION) +
                    sizeof(FILE_POSITION_INFORMATION)) {
                    //
                    // �����ǰλ�ô���һ������ͷͷ��ô�޸�
                    //
                    if (paiFileInformation->PositionInformation.CurrentByteOffset.QuadPart
                        >= CONFIDENTIAL_FILE_HEAD_SIZE) {
                        paiFileInformation->PositionInformation.CurrentByteOffset.QuadPart
                            -= CONFIDENTIAL_FILE_HEAD_SIZE;
                    }
                }
            }
            break;
        }

    case FileAllocationInformation:
        {
            PFILE_ALLOCATION_INFORMATION palloiFileInformation
                =(PFILE_ALLOCATION_INFORMATION)pFileInformation;

            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PostQueryInformation",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("FileAllocationInformation Entered, AllocationSize original: %d.",
                palloiFileInformation->AllocationSize.QuadPart));
            //
            // ���Դ�Сһ���ᳬ�������һ������ͷ
            //
            ASSERT(palloiFileInformation->AllocationSize.QuadPart>=CONFIDENTIAL_FILE_HEAD_SIZE);

            // palloiFileInformation->AllocationSize.QuadPart-= CONFIDENTIAL_FILE_HEAD_SIZE;
            break;
        }
        //
        // ����ͬ�� �޸�֮
        //
    case FileValidDataLengthInformation:
        {
            PFILE_VALID_DATA_LENGTH_INFORMATION pvliInformation = (PFILE_VALID_DATA_LENGTH_INFORMATION)pFileInformation;

            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PostQueryInformation",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("FileValidDataLengthInformation Entered, ValidDataLength original %d",
                pvliInformation->ValidDataLength.QuadPart));

            ASSERT(pvliInformation->ValidDataLength.QuadPart >= CONFIDENTIAL_FILE_HEAD_SIZE);
#ifdef DBG
                //
                // �������Ǽ�¼���ļ���Ϣû����
                //
                // FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);
                // ASSERT(pscFileStreamContext->nFileValidLength.QuadPart == pvliInformation->ValidDataLength.QuadPart);
                // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);
#endif
            // pvliInformation->ValidDataLength.QuadPart -= CONFIDENTIAL_FILE_HEAD_SIZE;
            // pvliInformation->ValidDataLength.QuadPart = pscFileStreamContext->nFileValidLength.QuadPart;

            FctGetFileValidSize(pscFileStreamContext, &pvliInformation->ValidDataLength);
            break;
        }
    case FileStandardInformation:
        {
            PFILE_STANDARD_INFORMATION psiFileInformation
                = (PFILE_STANDARD_INFORMATION)pFileInformation;

            DebugTraceFileAndProcess(DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PostQueryInformation",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("FileStandardInformation Entered, EndOfFile original: %d.",
                psiFileInformation->EndOfFile.QuadPart));

            ASSERT(psiFileInformation->AllocationSize.QuadPart >= CONFIDENTIAL_FILE_HEAD_SIZE);
            ASSERT((psiFileInformation->EndOfFile.QuadPart >= CONFIDENTIAL_FILE_HEAD_SIZE)
                || psiFileInformation->EndOfFile.QuadPart == 0);
#ifdef DBG
                //
                // �������Ǽ�¼���ļ���Ϣû����
                //
                // FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);
                // ASSERT(pscFileStreamContext->nFileValidLength.QuadPart == psiFileInformation->EndOfFile.QuadPart);
                // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);
#endif
//            psiFileInformation->AllocationSize.QuadPart -= CONFIDENTIAL_FILE_HEAD_SIZE;

            // psiFileInformation->EndOfFile.QuadPart -= CONFIDENTIAL_FILE_HEAD_SIZE;
            // psiFileInformation->EndOfFile.QuadPart = pscFileStreamContext->nFileValidLength.QuadPart;

            FctGetFileValidSize(pscFileStreamContext, &psiFileInformation->EndOfFile);
            break;
        }
    case FileEndOfFileInformation:
        {
            PFILE_END_OF_FILE_INFORMATION peofInformation = (PFILE_END_OF_FILE_INFORMATION)pFileInformation;

            DebugTraceFileAndProcess(DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PostQueryInformation",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("FileEndOfFileInformation Entered, EndOfFile original: %d.",
                peofInformation->EndOfFile.QuadPart));

            ASSERT(peofInformation->EndOfFile.QuadPart >= CONFIDENTIAL_FILE_HEAD_SIZE);
#ifdef DBG
                //
                // �������Ǽ�¼���ļ���Ϣû����
                //
                // FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);
                // ASSERT(pscFileStreamContext->nFileValidLength.QuadPart == peofInformation->EndOfFile.QuadPart);
                // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);
#endif
            // peofInformation->EndOfFile.QuadPart -= CONFIDENTIAL_FILE_HEAD_SIZE;
            // peofInformation->EndOfFile.QuadPart = pscFileStreamContext->nFileValidLength.QuadPart;

            FctGetFileValidSize(pscFileStreamContext,&peofInformation->EndOfFile);
            break;
        }
    case FilePositionInformation:
        {
            PFILE_POSITION_INFORMATION ppiInformation = (PFILE_POSITION_INFORMATION)pFileInformation;

            DebugTraceFileAndProcess(DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PostQueryInformation",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("FilePositionInformation Entered, EndOfFile original: %d.",
                ppiInformation->CurrentByteOffset.QuadPart));

            if (ppiInformation->CurrentByteOffset.QuadPart > CONFIDENTIAL_FILE_HEAD_SIZE)
                ppiInformation->CurrentByteOffset.QuadPart -= CONFIDENTIAL_FILE_HEAD_SIZE;

            break;
        }

    default:
        DebugTraceFileAndProcess(DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
            "PostQueryInformation",
            FILE_OBJECT_NAME_BUFFER(pfoFileObject),
            ("default Entered, encouterd something we do not concern. Type code: %d.",
            ficFileInformation)
  );
    }

    //
    // ȡ������
    //
/*  if (lpCompletionContext)
    {
//      FctDereferenceFileStreamContextObject((PFILE_STREAM_CONTEXT)lpCompletionContext);
    }*/

    //
    // �����޸�
    //
    FltSetCallbackDataDirty(pfcdCBD);

    //
    // �ͷ�������
    //
    FctReleaseStreamContext(pscFileStreamContext) ;

    return FLT_POSTOP_FINISHED_PROCESSING;
}

/*---------------------------------------------------------
��������:   Antinvader_PreDirectoryControl
��������:   Ԥ����ص�IRP_MJ_DIRECTORY_CONTROL
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
�������:
����ֵ:

����:
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreDirectoryControl (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    )
{
    // Io������
    PFLT_IO_PARAMETER_BLOCK pIoParameterBlock;

    // �������ķ���ֵ
    FLT_PREOP_CALLBACK_STATUS fcsReturn = FLT_PREOP_SUCCESS_NO_CALLBACK;

    // �����ݵĻ�����
    PVOID pNewBuffer = NULL;

    // �ڴ������б�
    PMDL pmMemoryDiscriptionList = NULL;

    // ��������
    PVOLUME_CONTEXT pvcVolumeContext = NULL;

    // ����ֵ
    NTSTATUS status;

    // ����ֵ
    BOOLEAN bReturn;

    // ���ļ�������
    PFILE_STREAM_CONTEXT pscFileStreamContext = NULL;

    // �ļ�����
    PFILE_OBJECT pfoFileObject;

    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    //
    // ����һЩָ��
    //
    pIoParameterBlock = pfcdCBD->Iopb;
    pfoFileObject = pFltObjects->FileObject;

    //
    // �������������ΪNULL ��������ֱ���Ϊ�µ�����,˵������������
    //
    *lpCompletionContext = NULL;

    do {
        // �������IRP_MN_QUERY_DIRECTORY ��ô�Ͳ�����
        if ((pIoParameterBlock->MinorFunction != IRP_MN_QUERY_DIRECTORY) ||
            (pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.Length == 0)) {
            break;
        }

        //
        // ����Ƿ��ǻ��ܽ���
        //
        if (!IsCurrentProcessConfidential()) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreDirectoryControl",
                FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                ("Not confidential process. Pass now."));
            break;
        }
/*
        //
        // ��ȡ�ļ���������
        //
        status = FctGetSpecifiedFileStreamContext(
            pFltObjects->Instance,
            pFltObjects->FileObject,
            &pscFileStreamContext);

        if (!NT_SUCCESS(status)) {

            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreDirectoryControl",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("No file context find. Reguarded as not confidential file."));
            break;
        }
*/
/*      //
        // ��ȡ��������
        //
        status = FltGetVolumeContext(
            pFltObjects->Filter,
            pFltObjects->Volume,
            (PFLT_CONTEXT *)&pvcVolumeContext
  );

        if (!NT_SUCCESS(status)) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreDirectoryControl",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("No volume context find. Reguarded as not confidential file."));
            break;
        }

        //
        // ���δ���� ֱ�ӷ���
        //
        if (FctGetFileConfidentialCondition(pscFileStreamContext) != ENCRYPTED_TYPE_CONFIDENTIAL) {

            DebugTraceFileAndProcess(
                DEBUG_TRACE_IMPORTANT_INFO,
                "PreDirectoryControl",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Confidential proccess read an not confidential file. File enctype: %d",
                    FctGetFileConfidentialCondition(pscFileStreamContext)));

            break;
        }*/
/*
        //
        // ��ֹ FAST IO
        //
        if (FLT_IS_FASTIO_OPERATION(pfcdCBD)) {

            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreDirectoryControl",
                FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                ("Disallow fast io."));
            break;
        }

        //
        // �����IRP���� ��ô����������
        //
        if (FlagOn(pfcdCBD->Flags,FLTFL_CALLBACK_DATA_IRP_OPERATION)) {

            bReturn = AllocateAndSwapToNewMdlBuffer(
                pIoParameterBlock,
                pvcVolumeContext,
                (PULONG)lpCompletionContext,
                NULL,
                NULL,
                NULL,
                Allocate_BufferDirectoryControl);

            if (!bReturn) {
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
                    "PreDirectoryControl",
                    FILE_OBJECT_NAME_BUFFER(pfoFileObject),
                    ("Error: Cannot swap buffer."));
                break;
            }*/
        // }
        DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
            "PreDirectoryControl",
            FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
            ("An directory control has been lunched."));

        fcsReturn = FLT_PREOP_SUCCESS_NO_CALLBACK;
    } while (0);

    if (pscFileStreamContext != NULL) {
        FctReleaseStreamContext(pscFileStreamContext) ;
    }

    if (pvcVolumeContext != NULL) {
        FltReleaseContext(pvcVolumeContext) ;
    }

    return fcsReturn ;
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

/*---------------------------------------------------------
��������:   Antinvader_PostDirectoryControl
��������:   ������ص�IRP_MJ_DIRECTORY_CONTROL
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
            Flags               �ص������ı�־(ԭ��)
�������:
����ֵ:

����:
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostDirectoryControl (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    // ����״̬
    NTSTATUS status;

    // I/O������,����IRP�����Ϣ
    PFLT_IO_PARAMETER_BLOCK  pIoParameterBlock;

    // �ļ���Ϣ���
    FILE_INFORMATION_CLASS  ficFileInformation;

    // �ļ���Ϣ
    PVOID   pFileInformation;

    // ���ݳ���
    ULONG ulLength;

    // ���ļ�������
    PFILE_STREAM_CONTEXT pscFileStreamContext = NULL;

    // �ļ�����
    PFILE_OBJECT    pfoFileObject;

    // �����˵Ļ���
    ULONG ulSwappedBuffer = NULL;

    // ԭ���Ļ���(��Ҫ���ǰ����ݿ���ȥ��)
    ULONG ulBuffer;

    // ���ڱ���WhenSafe��״̬
    FLT_POSTOP_CALLBACK_STATUS fcsStatus = FLT_POSTOP_FINISHED_PROCESSING;

    //
    // ����Iopb���ļ���Ϣָ�� �����ĵ�
    //
    pIoParameterBlock = pfcdCBD->Iopb;
    ficFileInformation = pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.FileInformationClass;
    pFileInformation = pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer;
    ulLength = pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.Length;
    ulSwappedBuffer = (ULONG)lpCompletionContext;
    pfoFileObject = pFltObjects->FileObject;

    DebugTraceFileAndProcess(DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
        "PostDirectoryControl",
        FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
        ("PostDirectoryControl enterd. FileInformation %d, Swapped buffer: 0x%X.",
        ficFileInformation,lpCompletionContext));

    //
    // ��ȡ�ļ���������
    //
    /*status = FctGetSpecifiedFileStreamContext(
        pFltObjects->Instance,
        pFltObjects->FileObject,
        &pscFileStreamContext);

    if (!NT_SUCCESS(status)) {
        DebugTraceFileAndProcess(
            DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
            "PostDirectoryControl",
            FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
            ("Error: Cannot get file context in post opration."));
        pscFileStreamContext = NULL;
        ASSERT(FALSE);
        return FLT_POSTOP_FINISHED_PROCESSING;
    }
*/
    do {
        if (!NT_SUCCESS(pfcdCBD->IoStatus.Status) || (pfcdCBD->IoStatus.Information == 0)) {
            break;
        }

        //
        // ��Read��ʱ��һ��,������Ҫ�����ݿ�����ԭ���Ļ���
        // ��������Ļ����ַ��ԭ���Ļ���
        //
        if (pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.MdlAddress != NULL) {

            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PostDirectoryControl",
                FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                ("Using mdl buffer."));

            ulBuffer = (ULONG)MmGetSystemAddressForMdlSafe(
                pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.MdlAddress,
                NormalPagePriority);

            if (!ulBuffer) {
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
                    "PostDirectoryControl",
                    FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                    ("Error: Cannot get mdl buffer."));

                pfcdCBD->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                pfcdCBD->IoStatus.Information = 0;
                break;
            }
        }
        else if (FlagOn(pfcdCBD->Flags, FLTFL_CALLBACK_DATA_SYSTEM_BUFFER)
            || FlagOn(pfcdCBD->Flags, FLTFL_CALLBACK_DATA_FAST_IO_OPERATION)) {

            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "PostDirectoryControl",
                FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                ("Using system buffer."));

            ulBuffer = (ULONG)pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer;
        }
        else {
            //
            // ����WhenSafe������ٴλ�ȡ�ļ���������
            // �������ͷŵ���ֹ����
            //
//          FctReleaseStreamContext(pscFileStreamContext) ;
//          pscFileStreamContext = NULL;

            if (FltDoCompletionProcessingWhenSafe(
                pfcdCBD,
                pFltObjects,
                lpCompletionContext,Flags,
                Antinvader_PostDirectoryControlWhenSafe,
                &fcsStatus )) {
                //
                // �����������ͷ�SwappedBuffer
                //
                ulSwappedBuffer = NULL;
            } else {
                pfcdCBD->IoStatus.Status = STATUS_UNSUCCESSFUL;
                pfcdCBD->IoStatus.Information = 0;
            }
            break;
        }

        //
        //  ����������һ��ϵͳ���������FastIo(�Ѿ���ֹ��),���ڿ����ݲ��Ҵ����쳣
        //
        // ע��:����һ��FASTFAT��Bug,�᷵��һ������ĳ���,����������
        // Parameters.DirectoryControl.QueryDirectory.Length����ĳ���
        //
        if (ulSwappedBuffer) {
            __try {
                RtlCopyMemory((PVOID)ulBuffer,
                              (PVOID)ulSwappedBuffer,
                               /*Data->IoStatus.Information*/
                               pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.Length);
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                pfcdCBD->IoStatus.Status = GetExceptionCode();
                pfcdCBD->IoStatus.Information = 0;
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
                    "PostDirectoryControl",
                    FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                    ("Error: Error occurred when copy data back. IoStatus: 0x%X.",
                    pfcdCBD->IoStatus.Status));
                break;
            }
        }
        /*
        DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
            "PostDirectoryControl",
            FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
            ("Swap finished FileInformationClass: %d.",
            ficFileInformation));
        */
    } while (0);

    //
    // ��β����
    //
    if (ulSwappedBuffer) {
        FreeAllocatedMdlBuffer(ulSwappedBuffer, Allocate_BufferDirectoryControl);
    }

    if (pscFileStreamContext) {
        FctReleaseStreamContext(pscFileStreamContext) ;
    }

    return fcsStatus;   // STATUS_SUCCESS;
}

/*---------------------------------------------------------
��������:   Antinvader_PostDirectoryControl
��������:   ��ȫʱDirectoryControl��ص�
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
            Flags               �ص������ı�־(ԭ��)
�������:
����ֵ:

����:
����ά��:   2011.7.20    ����汾
---------------------------------------------------------*/
FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostDirectoryControlWhenSafe (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    // ����ֵ
    NTSTATUS status;

    // I/O������,����IRP�����Ϣ
    PFLT_IO_PARAMETER_BLOCK  pIoParameterBlock;

    // �ļ���Ϣ���
    FILE_INFORMATION_CLASS  ficFileInformation;

    // �ļ���Ϣ
    PVOID   pFileInformation;

    // ���ݳ���
    ULONG ulLength;

    // ���ļ�������
    PFILE_STREAM_CONTEXT pscFileStreamContext = NULL;

    // �ļ�����
    PFILE_OBJECT    pfoFileObject;

    // �����˵Ļ���
    ULONG ulSwappedBuffer = NULL;

    // ԭ���Ļ���(��Ҫ���ǰ����ݿ���ȥ��)
    ULONG ulBuffer;

    //
    // ����(����)MDL,�������ǲ��ܷ���
    //
    status = FltLockUserBuffer( pfcdCBD);

    //
    // ����Iopb���ļ���Ϣָ�� �����ĵ�
    //
    pIoParameterBlock = pfcdCBD->Iopb;

    ficFileInformation = pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.FileInformationClass;
    pFileInformation = pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer;
    ulLength = pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.Length;

    ulSwappedBuffer = (ULONG)lpCompletionContext;

    pfoFileObject = pFltObjects->FileObject;

    DebugTraceFileAndProcess(
        DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
        "PostDirectoryControlWhenSafe",
        FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
        ("PostDirectoryControlWhenSafe enterd. FileInformation: %d, Swapped buffer: 0x%X.",
        ficFileInformation, lpCompletionContext));

    //
    // ��ȡ�ļ���������
    //
    /*
    status = FctGetSpecifiedFileStreamContext(
        pFltObjects->Instance,
        pFltObjects->FileObject,
        &pscFileStreamContext);

    if (!NT_SUCCESS(status)) {
        DebugTraceFileAndProcess(
            DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
            "PostDirectoryControlWhenSafe",
            FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
            ("Error: Cannot get file context in post opration."));
        pscFileStreamContext = NULL;
        ASSERT(FALSE);
        return FLT_POSTOP_FINISHED_PROCESSING;
    }
    */

    do {
        if (!NT_SUCCESS(pfcdCBD->IoStatus.Status) ||
            (pfcdCBD->IoStatus.Information == 0)) {
            break;
        }

        //
        // ��ȡԭʼ���ݵ�ַ
        //
        ulBuffer = (ULONG)MmGetSystemAddressForMdlSafe(
            pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.MdlAddress,
            NormalPagePriority);

        if (!ulBuffer) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
                "PostDirectoryControlWhenSafe",
                FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                ("Error: Cannot get mdl buffer."));

            pfcdCBD->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            pfcdCBD->IoStatus.Information = 0;
            break;
        }

        if (ulSwappedBuffer) {
            __try {
                //
                // ͬPostDirectoryControl �������ݳ�����
                // Parameters.DirectoryControl.QueryDirectory.Length
                //
                RtlCopyMemory((PVOID)ulBuffer,
                              (PVOID)ulSwappedBuffer,
                               /*Data->IoStatus.Information*/
                               pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.Length);
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                pfcdCBD->IoStatus.Status = GetExceptionCode();
                pfcdCBD->IoStatus.Information = 0;
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_ERROR | DEBUG_TRACE_CONFIDENTIAL,
                    "PostDirectoryControl",
                    FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                    ("Error: Error occurred when copy data back. IoStatus: 0x%X.",
                    pfcdCBD->IoStatus.Status));
                break;
            }
        }
        /*
        DebugTraceFileAndProcess(
            DEBUG_TRACE_NORMAL_INFO | DEBUG_TRACE_CONFIDENTIAL,
            "PostDirectoryControlWhenSafe",
            FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
            ("Swap finished FileInformationClass %d.",ficFileInformation));
        */
    } while (0);

    //
    // ��β����
    //
    if (ulSwappedBuffer) {
        FreeAllocatedMdlBuffer(ulSwappedBuffer, Allocate_BufferDirectoryControl);
    }

    if (pscFileStreamContext) {
        FctReleaseStreamContext(pscFileStreamContext) ;
    }

    return FLT_POSTOP_FINISHED_PROCESSING;
}


/*---------------------------------------------------------
��������:   Antinvader_PreCleanUp
��������:   Ԥ����ص�IRP_MJ_CLEANUP
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
�������:
����ֵ:

����:
����ά��:   2011.3.20    ����汾
            2012.1.4     ������������ˢ����Ĳ��� ����Ҫ
                               ��ˢWrite�Ķ����Ͳ��ܱ�����������
---------------------------------------------------------*/
FLT_PREOP_CALLBACK_STATUS
Antinvader_PreCleanUp (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __deref_out_opt PVOID *lpCompletionContext
    )
{
    //
    // ȷ�� IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    // ����ֵ
    NTSTATUS status;

    // ��������
    PVOLUME_CONTEXT pvcVolumeContext;

    // �ļ�����Ϣ
    PFLT_FILE_NAME_INFORMATION pfniFileNameInformation = NULL;

    // �Ƿ����ļ���
    BOOLEAN bDirectory;

    // �ļ���������
    PFILE_STREAM_CONTEXT pscFileStreamContext = NULL;

    //
    // ����Ƿ��ǻ��ܽ���
    //
    do {
        //
        // ��ȡ��������
        //
        status = FltGetVolumeContext(
            pFltObjects->Filter,
            pFltObjects->Volume,
            (PFLT_CONTEXT *)&pvcVolumeContext);

        if (!NT_SUCCESS(status) || (NULL == pvcVolumeContext)) {
            pscFileStreamContext = NULL;

            DebugTraceFileAndProcess(
                DEBUG_TRACE_NORMAL_INFO,
                "PreCleanUp",
                FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                ("No volume context was found."));
            break ;
        }
/*
        //
        // ��ȡ�ļ���������
        //
        status = FctGetSpecifiedFileStreamContext(
            pFltObjects->Instance,
            pFltObjects->FileObject,
            &pscFileStreamContext
  );

        if (!NT_SUCCESS(status)) {
            pscFileStreamContext = NULL;
            __leave ;
        }

        if (pscFileStreamContext->fctEncrypted == ENCRYPTED_TYPE_NOT_CONFIDENTIAL) {
            __leave;
        }
*/
        //
        // ��ȡ�ļ�������Ϣ
        //
        status = FltGetFileNameInformation(pfcdCBD,
            FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT,
            &pfniFileNameInformation);

        if (!NT_SUCCESS(status)) {
            DebugTraceFileAndProcess(
                DEBUG_TRACE_ERROR,
                "PreCleanUp",
                FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                ("Error: Cannot get file name information."));
            pfniFileNameInformation = NULL;
            break ;
        }

        if (pfniFileNameInformation->Name.Length != 0) {
            //
            // ��ȡ������Ϣ �����ǲ����ļ���
            //
            status = FileGetStandardInformation(
                pFltObjects->Instance,
                pFltObjects->FileObject,
                NULL,
                NULL,
                &bDirectory);

            if (!NT_SUCCESS(status)) {
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO,
                    "PreCleanUp",
                    FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                    ("Cannot get file information."));

                break ;
            }

            if (bDirectory) {
                DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO,
                    "PreCleanUp",
                    FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                    ("Dictory just pass."));
                break ;
            }

            //
            // ������CleanUp, ��Ҫˢ����
            //
            FileClearCache(pFltObjects->FileObject);
        }
    } while (0);

    if (pvcVolumeContext != NULL) {
        FltReleaseContext(pvcVolumeContext) ;
    }

    if (pfniFileNameInformation != NULL) {
        FltReleaseFileNameInformation(pfniFileNameInformation);
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

/*---------------------------------------------------------
��������:   Antinvader_PostCleanUp
��������:   ������ص�IRP_MJ_CLEANUP
�������:
            pfcdCBD             �ص�����
            pFltObjects         �ļ�����
            lpCompletionContext ��������������
            Flags               �ص������ı�־(ԭ��)
�������:
����ֵ:

����:
����ά��:   2011.3.20    ����汾
            2011.7.17    �����˻����ͷ�
---------------------------------------------------------*/
FLT_POSTOP_CALLBACK_STATUS
Antinvader_PostCleanUp (
    __inout PFLT_CALLBACK_DATA pfcdCBD,
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in_opt PVOID lpCompletionContext,
    __in FLT_POST_OPERATION_FLAGS Flags
    )
{
    KdPrint(("[Antinvader]Antinvader_PostCleanUp: Entered\n"));

    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    // Io����������
    PFLT_IO_PARAMETER_BLOCK pIoParameterBlock;

    // �򿪵��ļ�����
    PFILE_OBJECT pfoFileObject;

    // Hash���е�ַ
    PHASH_NOTE_DESCRIPTOR pndFileNoteDescriptor;

    // ����ֵ
    BOOLEAN bReturn;

    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();
/*
    //
    // ��ȡ��Ҫ�Ĳ���
    //
    pIoParameterBlock = pfcdCBD->Iopb;
    pfoFileObject = pFltObjects->FileObject;


    // ����ֵ
    NTSTATUS status;

    // ��������
    PVOLUME_CONTEXT pvcVolumeContext;

    // �ļ�����Ϣ
    PFLT_FILE_NAME_INFORMATION pfniFileNameInformation = NULL;

    // �Ƿ����ļ���
    BOOLEAN bDirectory;

    // �ļ���������
    PFILE_STREAM_CONTEXT pscFileStreamContext = NULL;

    //
    // ����Ƿ��ǻ��ܽ���
    //
    do {
        //
        // ��ȡ��������
        //
        status = FltGetVolumeContext(
            pFltObjects->Filter,
            pFltObjects->Volume,
            (PFLT_CONTEXT *)&pvcVolumeContext);

        if (!NT_SUCCESS(status) || (NULL == pvcVolumeContext)) {

            pscFileStreamContext = NULL;

            DebugTraceFileAndProcess(
                    DEBUG_TRACE_NORMAL_INFO,
                    "PreCleanUp",
                    FILE_OBJECT_NAME_BUFFER(pFltObjects->FileObject),
                    ("No volume context was found."));

            break ;
        }

        //
        // ��ȡ�ļ���������
        //
        status = FctGetSpecifiedFileStreamContext(
            pFltObjects->Instance,
            pFltObjects->FileObject,
            &pscFileStreamContext);

        if (!NT_SUCCESS(status)) {
            pscFileStreamContext = NULL;
            break ;
        }

        if (pscFileStreamContext->fctEncrypted == ENCRYPTED_TYPE_NOT_CONFIDENTIAL) {
            break;
        }

    } while (0);

    if (pvcVolumeContext != NULL) {
        FltReleaseContext(pvcVolumeContext) ;
    }

    if (pscFileStreamContext != NULL) {
        FctReleaseStreamContext(pscFileStreamContext) ;
    }
*/
    return FLT_POSTOP_FINISHED_PROCESSING;// STATUS_SUCCESS;
}

/////////////////////////////////////
//     һ�������ص�����
////////////////////////////////////
/*---------------------------------------------------------
��������:   Antinvader_InstanceSetup
��������:   ��������ʵ��
�������:
            pFltObjects             ������������
            Flags                   ������־
            VolumeDeviceType        ���̾�����
            VolumeFilesystemType    ���ļ�ϵͳ����
�������:
����ֵ:
            STATUS_SUCCESS �ɹ�
����:
����ά��:   2011.3.20    ����汾
            2011.7.27    �����˾�����������
---------------------------------------------------------*/
NTSTATUS
Antinvader_InstanceSetup (
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in FLT_INSTANCE_SETUP_FLAGS Flags,
    __in DEVICE_TYPE VolumeDeviceType,
    __in FLT_FILESYSTEM_TYPE VolumeFilesystemType
    )
{
//    UNREFERENCED_PARAMETER(pFltObjects);
//    UNREFERENCED_PARAMETER(Flags);
//    UNREFERENCED_PARAMETER(VolumeDeviceType);
//    UNREFERENCED_PARAMETER(VolumeFilesystemType);

    PAGED_CODE();

    KdPrint(("[Antinvader]Antinvader_InstanceSetup: Entered\n"));

    // ���ڴ洢����Ϣ�Ļ�����
    UCHAR szVolumePropertiesBuffer[sizeof(FLT_VOLUME_PROPERTIES)+512];

    // ����Ϣָ��
    PFLT_VOLUME_PROPERTIES pvpVolumeProperties = (PFLT_VOLUME_PROPERTIES)szVolumePropertiesBuffer;

    // ����ֵ
    NTSTATUS status;

    // ��������
    PVOLUME_CONTEXT pvcVolumeContext = NULL;

    // �豸����
    PDEVICE_OBJECT pdoDeviceObject = NULL;

    // ���صĳ���
    ULONG ulReturn;

    // ���õ�����ָ��
    PUNICODE_STRING pusWorkingName;

    do {
        //
        // ��ҪAttach��һ��������,����һ��������
        //
        status = FltAllocateContext(
            pFltObjects->Filter,
            FLT_VOLUME_CONTEXT,
            sizeof(VOLUME_CONTEXT),
            NonPagedPool,
            (PFLT_CONTEXT *)&pvcVolumeContext);

        if (!NT_SUCCESS(status)) {
            break;
        }

        //
        // ��ȡ������ �ҵ���Ҫ������
        //
        status = FltGetVolumeProperties(
            pFltObjects->Volume,
            pvpVolumeProperties,
            sizeof(szVolumePropertiesBuffer),
            &ulReturn);

        if (!NT_SUCCESS(status)) {
            break;
        }

        //
        // ����������С
        //
        pvcVolumeContext->ulSectorSize = pvpVolumeProperties->SectorSize;
        pvcVolumeContext->uniName.Buffer = NULL;
        pvcVolumeContext->pnliReadEncryptedSignLookasideList = NULL;

        //
        // ��ȡ�豸����
        //
        status = FltGetDiskDeviceObject(pFltObjects->Volume,
            &pdoDeviceObject);

        //
        // ����ɹ��� ���Ի�ȡDos����
        //
        if (NT_SUCCESS(status)) {
            status = RtlVolumeDeviceToDosName(pdoDeviceObject,
                &pvcVolumeContext->uniName);
        }

        //
        // �ò���Dos���� �Ǿ���NT���ư�
        //
        if (!NT_SUCCESS(status)) {
            //
            // �����ĸ���������
            //
            if (pvpVolumeProperties->RealDeviceName.Length > 0) {
                pusWorkingName = &pvpVolumeProperties->RealDeviceName;
            }
            else if (pvpVolumeProperties->FileSystemDeviceName.Length > 0) {
                pusWorkingName = &pvpVolumeProperties->FileSystemDeviceName;
            }
            else {
                //
                // û�п������� �����豸�Ͳ��ҽ���
                //
                status = STATUS_FLT_DO_NOT_ATTACH;
                break;
            }

            pvcVolumeContext->uniName.Buffer = (PWCH)ExAllocatePoolWithTag(
                NonPagedPool,
                pusWorkingName->Length + sizeof(WCHAR),     // ����һ�����ӷ� ":" �ĳ���
                MEM_CALLBACK_TAG);

            //
            // ʧ����.....
            //
            if (pvcVolumeContext->uniName.Buffer == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            pvcVolumeContext->uniName.Length = 0;
            pvcVolumeContext->uniName.MaximumLength = pusWorkingName->Length + sizeof(WCHAR);

            //
            // �����ָ��ƹ��� ���Ҽ���":"
            //
            RtlCopyUnicodeString( &pvcVolumeContext->uniName, pusWorkingName);
            RtlAppendUnicodeToString( &pvcVolumeContext->uniName, L":");
        }

        //
        // �ڶ�ȡ�ļ��ж��Ƿ��ǻ����ļ�ʱ��Ҫʹ��NonCache��ȡ,������һ��SectorSize
        // ���ڿ����ϴ��Ǿ�������������
        //
        pvcVolumeContext->pnliReadEncryptedSignLookasideList =
            (PNPAGED_LOOKASIDE_LIST)ExAllocatePoolWithTag(
                    NonPagedPool,
                    sizeof(NPAGED_LOOKASIDE_LIST),
                    MEM_FILE_TAG);

        ExInitializeNPagedLookasideList(
            pvcVolumeContext->pnliReadEncryptedSignLookasideList,
            NULL,
            NULL,
            0,
            sizeof(pvcVolumeContext->ulSectorSize),
            MEM_FILE_TAG,
            0);

        //
        // �����������Ҫ���Գ�ʼ��һЩ��������
        //

        //
        // ����������
        //
        status = FltSetVolumeContext(
            pFltObjects->Volume,
            FLT_SET_CONTEXT_KEEP_IF_EXISTS,
            pvcVolumeContext,
            NULL);

        if (status == STATUS_FLT_CONTEXT_ALREADY_DEFINED) {
            //
            // ����Ѿ����ù��������� û��ϵ
            //
            status = STATUS_SUCCESS;
        }

    } while (0);

    if (pvcVolumeContext) {
        //
        // �ͷ������� ������ͷ�ϵͳ�����
        //
        FltReleaseContext(pvcVolumeContext);
    }

    if (pdoDeviceObject) {
        //
        // �ͷ��豸����
        //
        ObDereferenceObject(pdoDeviceObject);
    }

    return status;
}

/*---------------------------------------------------------
��������:   Antinvader_InstanceQueryTeardown
��������:   ���ٲ�ѯʵ��
�������:
            pFltObjects             ������������
            Flags                   ��ѯʵ����־
�������:
����ֵ:
            STATUS_SUCCESS �ɹ�
����:       ��ʱδ����κι���
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
NTSTATUS
Antinvader_InstanceQueryTeardown (
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER(pFltObjects);
    UNREFERENCED_PARAMETER(Flags);

    PAGED_CODE();

    KdPrint(("[Antinvader]Antinvader_InstanceQueryTeardown: Entered\n"));

    return STATUS_SUCCESS;
}

/*---------------------------------------------------------
��������:   Antinvader_InstanceTeardownStart
��������:   ��ʼ���ٹ���ʵ���ص�
�������:
            pFltObjects             ������������
            Flags                   ��ѯʵ����־
�������:
����ֵ:

����:       ��ʱδ����κι���
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
VOID
Antinvader_InstanceTeardownStart (
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER(pFltObjects);
    UNREFERENCED_PARAMETER(Flags);

    PAGED_CODE();

    KdPrint(("[Antinvader]Antinvader_InstanceTeardownStart: Entered\n"));
}

/*---------------------------------------------------------
��������:   Antinvader_InstanceTeardownComplete
��������:   ������ٹ���ʵ���ص�
�������:
            pFltObjects ������������
            Flags       ʵ�����ٱ�־
�������:
����ֵ:

����:       ��ʱδ����κι���
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
VOID
Antinvader_InstanceTeardownComplete (
    __in PCFLT_RELATED_OBJECTS pFltObjects,
    __in FLT_INSTANCE_TEARDOWN_FLAGS Flags
    )
{
    UNREFERENCED_PARAMETER(pFltObjects);
    UNREFERENCED_PARAMETER(Flags);

    PAGED_CODE();

    KdPrint(("[Antinvader]Antinvader_InstanceTeardownComplete: Entered\n"));
}

/*---------------------------------------------------------
��������:   Antinvader_CleanupContext
��������:   ����������
�������:
            pcContext       ����������
            pctContextType  ����������
�������:
����ֵ:

����:
����ά��:   2011.7.27    ����汾
            2011.7.29    Bug:ָ�봫�� �Ѿ��޸�
---------------------------------------------------------*/
VOID
Antinvader_CleanupContext(
    __in PFLT_CONTEXT pcContext,
    __in FLT_CONTEXT_TYPE pctContextType
    )
{
    PVOLUME_CONTEXT pvcVolumeContext = NULL;
    PFILE_STREAM_CONTEXT pscFileStreamContext = NULL;

    PAGED_CODE();

    switch (pctContextType) {
    case FLT_VOLUME_CONTEXT:
        {
            pvcVolumeContext = (PVOLUME_CONTEXT)pcContext;

            if (pvcVolumeContext->uniName.Buffer != NULL) {
                ExFreePool(pvcVolumeContext->uniName.Buffer);
                pvcVolumeContext->uniName.Buffer = NULL;
            }

            if (pvcVolumeContext->pnliReadEncryptedSignLookasideList) {
                ExDeleteNPagedLookasideList(pvcVolumeContext->pnliReadEncryptedSignLookasideList);
                ExFreePool(pvcVolumeContext->pnliReadEncryptedSignLookasideList);
                pvcVolumeContext->pnliReadEncryptedSignLookasideList = NULL;
            }
        }
        break ;
    case FLT_STREAM_CONTEXT:
        {
            KIRQL OldIrql;
            pscFileStreamContext = (PFILE_STREAM_CONTEXT)pcContext;

            //
            // �ͷ�������
            //
            FctFreeStreamContext(pscFileStreamContext);
        }
        break ;
    }
}

///////////////////////////////
//      ͨ�Żص�����
///////////////////////////////

/*---------------------------------------------------------
��������:   Antinvader_Connect
��������:   Ring3����ʱ�ص��ú���
�������:
            ClientPort          �ͻ��˶˿�
            ServerPortCookie    ����˿�������
            ConnectionContext   ���Ӷ�������
            SizeOfContext       ���Ӷ������Ĵ�С
            ConnectionCookie    ����Cookie
�������:
����ֵ:
            STATUS_SUCCESS �ɹ�
����:
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/

NTSTATUS
Antinvader_Connect(
    __in PFLT_PORT ClientPort,
    __in PVOID ServerPortCookie,
    __in_bcount(SizeOfContext) PVOID ConnectionContext,
    __in ULONG SizeOfContext,
    __deref_out_opt PVOID *ConnectionCookie
    )
{
    KdPrint(("[Antinvader]Antinvader_Connect: Entered.\n"));
    PAGED_CODE();

    UNREFERENCED_PARAMETER(ServerPortCookie);
    UNREFERENCED_PARAMETER(ConnectionContext);
    UNREFERENCED_PARAMETER(SizeOfContext);
    UNREFERENCED_PARAMETER(ConnectionCookie);

//  ASSERT(gClientPort == NULL);

    pfpGlobalClientPort = ClientPort;
    return STATUS_SUCCESS;
}

/*---------------------------------------------------------
��������:   Antinvader_Disconnect
��������:   Ring3�Ͽ�����ʱ�ص��ú���
�������:
            ConnectionCookie    ����Cookie
�������:
����ֵ:

����:
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
VOID Antinvader_Disconnect(__in_opt PVOID ConnectionCookie)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(ConnectionCookie);
    KdPrint(("[Antinvader]Antinvader_Disconnect: Entered\n"));

    // �رվ��
    FltCloseClientPort( pfltGlobalFilterHandle, &pfpGlobalClientPort);

    pfpGlobalClientPort = NULL;
}

/*---------------------------------------------------------
��������:   Antinvader_Disconnect
��������:   Ring3��Ϣ������
�������:
            ConnectionCookie    ����Cookie
            InputBuffer         ���������
            InputBufferSize     �������ݴ�С
            OutputBufferSize    �������ݻ����С

�������:
            OutputBuffer                ����������
            ReturnOutputBufferLength    ʵ�ʴ������ݴ�С
����ֵ:
            STATUS_SUCCESS �ɹ�
����:
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
NTSTATUS
Antinvader_Message (
    __in PVOID ConnectionCookie,
    __in_bcount_opt(InputBufferSize) PVOID InputBuffer,
    __in ULONG InputBufferSize,
    __out_bcount_part_opt(OutputBufferSize, *ReturnOutputBufferLength) PVOID OutputBuffer,
    __in ULONG OutputBufferSize,
    __out PULONG ReturnOutputBufferLength
    )
{
    // ����
    ANTINVADER_COMMAND acCommand;

    // ����ֵ
    NTSTATUS status;

    // ������Ϣ
    CONFIDENTIAL_PROCESS_DATA cpdProcessData;

    // �������ʱָ��
    PCWSTR  pcwString;

    // �ظ�������
    COMMAND_MESSAGE replyMessage;

    // ����ֵ
    BOOLEAN bReturn;

    replyMessage.lSize = sizeof(COMMAND_MESSAGE);
    replyMessage.acCommand = ENUM_OPERATION_FAILED;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(ConnectionCookie);

    //
    //  ���������Ļ��������û�ģʽ��ԭʼ��ַ.
    //  ���˹������Ѿ�����ProbedForRead(InputBuffer)
    //  ��ProbedForWrite(OutputBuffer),�Ӷ���֤����
    //  ���ڵ�ַ��������Ч��(�û�ģʽ���ں�ģʽ),΢
    //  �������������ü��.�����˹�����û�����κε�
    //  ����ָ����.���Ҫʹ��ָ������Ļ�,΢������
    //  �������ʹ�ýṹ���쳣���������ʻ�����.
    //

    // ������������Ƿ���ȷ
    /*
    if ((InputBuffer != NULL) &&
        (InputBufferSize >= (FIELD_OFFSET(COMMAND_MESSAGE,Command) +
        sizeof(ANTINVADER_COMMAND)))) {
    */
     if (((InputBuffer != NULL)
          && (InputBufferSize == ((PCOMMAND_MESSAGE) InputBuffer)->lSize))
          && (OutputBufferSize>=sizeof(COMMAND_MESSAGE))) {
        __try {
            // ��Ϣ���û�ģʽ������,������Ҫ�ṹ���쳣������

            // ��������
            acCommand = ((PCOMMAND_MESSAGE) InputBuffer)->acCommand;

            // ��������ִ��
            switch (acCommand) {
                case ENUM_COMMAND_PASS:
                    break;
                case ENUM_BLOCK:
                    break;
                case ENUM_ADD_PROCESS:
                case ENUM_DELETE_PROCESS:
                    //
                    // �Ȳ�� length�Ѿ����ֽڳ���,����Ҫ��sizeof����
                    //
                    pcwString = (PCWSTR)((ULONG)InputBuffer + sizeof(COMMAND_MESSAGE));
                    RtlInitUnicodeString(&cpdProcessData.usName, pcwString);

                    pcwString = (PCWSTR)((ULONG)pcwString + (cpdProcessData.usName.Length + sizeof(WCHAR)));
                    RtlInitUnicodeString(&cpdProcessData.usPath, pcwString);

                    pcwString = (PCWSTR)((ULONG)pcwString + (cpdProcessData.usPath.Length + sizeof(WCHAR)));
                    RtlInitUnicodeString(&cpdProcessData.usMd5Digest, pcwString);

                    KdPrint(("[Antinvader]Process Name: %ws\n\t\tProcess Path: %ws\n\t\tProcess MD5: %ws\n",
                        cpdProcessData.usName.Buffer, cpdProcessData.usPath.Buffer, cpdProcessData.usMd5Digest.Buffer));

                    //
                    // �ж���ɾ�����Ǳ�� ִ�в���
                    //
                    bReturn = ((acCommand == ENUM_ADD_PROCESS) ?
                        PctAddProcess(&cpdProcessData) : PctDeleteProcess(&cpdProcessData));

                    if (bReturn) {
                        status = STATUS_SUCCESS;
                        replyMessage.acCommand = ENUM_OPERATION_SUCCESSFUL;
                    } else {
                        status = STATUS_UNSUCCESSFUL;
                    }
                    break;
                default:
                    KdPrint(("[Antinvader]Antinvader_Message: default\n"));
                    status = STATUS_INVALID_PARAMETER;
                    break;
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            // ��������� ���ش���ֵ
            return GetExceptionCode();
        }
    } else {
        // ������ݳ��Ȳ���
        status = STATUS_INVALID_PARAMETER;
    }

    //
    // ��������
    //
    RtlCopyMemory(OutputBuffer, &replyMessage, sizeof(replyMessage.lSize));
    *ReturnOutputBufferLength = replyMessage.lSize;

    return status;
}
