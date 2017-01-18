///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2011 - 2012
//
// ԭʼ�ļ�����     : FileFunction.cpp
// ��������         : AntinvaderDriver
// ����ʱ��         : 2011-03-26
//
//
// ����             : �����ļ���Ϣ�Ĺ���ʵ��
//
// ����ά��:
//  0000 [2011-03-26] ����汾.
//
///////////////////////////////////////////////////////////////////////////////

#include "FileFunction.h"
#include "AntinvaderDriver.h"
#include "ConfidentialFile.h"
#include "ProcessFunction.h"

// ���ڴ�ż���ͷ�ڴ����������
NPAGED_LOOKASIDE_LIST   nliNewFileHeaderLookasideList;

/*---------------------------------------------------------
��������:   FileSetSize
��������:   �����������ļ���С
�������:
            pfiInstance         ������ʵ��
            pfoFileObject       �ļ�����
            pnFileSize          �ļ���С
�������:
����ֵ:
            STATUS_SUCCESS �ɹ� ���򷵻���Ӧ״̬

����:

����ά��:   2011.4.3    ����汾 ʹ��IRP
            2011.4.9      �޸�Ϊʹ��FltXXX�汾
---------------------------------------------------------*/
NTSTATUS
FileSetSize(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT pfoFileObject,
    __in PLARGE_INTEGER pnFileSize
    )
{
    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    //
    // ֱ�������ļ���Ϣ
    //
    FILE_END_OF_FILE_INFORMATION eofInformation;
    eofInformation.EndOfFile.QuadPart = pnFileSize->QuadPart;

    return FltSetInformationFile(
                        pfiInstance,
                        pfoFileObject,
                        (PVOID)&eofInformation,
                        sizeof(FILE_END_OF_FILE_INFORMATION),
                        FileEndOfFileInformation);
}

/*---------------------------------------------------------
��������:   FileSetOffset
��������:   ���������ļ�ƫ�� ���ڶ�ȡ���ݺ��޸�
�������:
            pfiInstance         ������ʵ��
            pfoFileObject       �ļ�����
            pnFileOffset        �ļ�ƫ��
�������:
����ֵ:
����:
����ά��:   2011.7.17     ����汾
---------------------------------------------------------*/
NTSTATUS FileSetOffset(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT pfoFileObject,
    __in PLARGE_INTEGER pnFileOffset
    )
{
    FILE_POSITION_INFORMATION fpiPosition;

    LARGE_INTEGER nOffset;

    //
    // �Ȱ�ƫ������������ ������
    //

    nOffset.QuadPart = pnFileOffset->QuadPart;
    nOffset.LowPart = pnFileOffset->LowPart;

    fpiPosition.CurrentByteOffset = nOffset;

    return FltSetInformationFile(pfiInstance,
                                 pfoFileObject,
                                 &fpiPosition,
                                 sizeof(FILE_POSITION_INFORMATION),
                                 FilePositionInformation);
}
/*---------------------------------------------------------
��������:   FileGetStandardInformation
��������:   �������ȡ�ļ�������Ϣ
�������:
            pfiInstance         ������ʵ��
            pfoFileObject       �ļ�����
            pnAllocateSize      �����С
            pnFileSize          �ļ���С
            pbDirectory         �Ƿ���Ŀ¼
�������:
����ֵ:
            STATUS_SUCCESS �ɹ� ���򷵻���Ӧ״̬

����:       ���������Ϊ��ѡ,����Ҫ��ѯ��ΪNULL����

����ά��:   2011.4.3    ����汾
            2011.4.9    �޸�Ϊʹ��FltXXX�汾
---------------------------------------------------------*/
NTSTATUS
FileGetStandardInformation(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT pfoFileObject,
    __inout_opt PLARGE_INTEGER pnAllocateSize,
    __inout_opt PLARGE_INTEGER pnFileSize,
    __inout_opt BOOLEAN *pbDirectory
    )
{
    // ����ֵ
    NTSTATUS status;
    PFILE_STANDARD_INFORMATION psiFileStandardInformation;

    //
    // ȷ��IRQL <= APC_LEVEL
    //
    PAGED_CODE();

    //
    // ���ȷ����ڴ�, ׼����ѯ, ���ʧ��, ������Դ����.
    //
    psiFileStandardInformation = (PFILE_STANDARD_INFORMATION)
        FltAllocatePoolAlignedWithTag(pfiInstance, NonPagedPool, sizeof(FILE_STANDARD_INFORMATION), MEM_FILE_TAG);

    if (!psiFileStandardInformation) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // ��ѯ��Ϣ, ����ɹ���ɸѡ��Ϣ, ����ֱ�ӷ���.
    //
    status = FltQueryInformationFile(
        pfiInstance,    // ʵ��, ��ֹ����
        pfoFileObject,
        (PVOID)psiFileStandardInformation,
        sizeof(FILE_STANDARD_INFORMATION),
        FileStandardInformation,
        NULL            // ����Ҫ�˽ⷵ���˶�������
        );

    if (NT_SUCCESS(status)) {
        if (pnAllocateSize) {
            *pnAllocateSize = psiFileStandardInformation->AllocationSize;
        }
        if (pnFileSize) {
            *pnFileSize = psiFileStandardInformation->EndOfFile;
        }
        if (pbDirectory != NULL) {
            *pbDirectory = psiFileStandardInformation->Directory;
        }
    }

    FltFreePoolAlignedWithTag(pfiInstance, psiFileStandardInformation, MEM_FILE_TAG);
    return status;
}

/*---------------------------------------------------------
��������:   FileCompleteCallback
��������:   ��ɻص�,�����¼�
�������:
            CallbackData        ��������
            Context             ������
�������:
����ֵ:
����:
����ά��:   2011.4.9    ����汾
---------------------------------------------------------*/
static
VOID
FileCompleteCallback(
    __in PFLT_CALLBACK_DATA CallbackData,
    __in PFLT_CONTEXT Context
    )
{
    //
    // ������ɱ�־
    //
    KeSetEvent((PRKEVENT)Context, 0, FALSE);
}

/*---------------------------------------------------------
��������:   FileWriteEncryptionHeader
��������:   ������д����ͷ��Ϣ
�������:
            pfiInstance         ������ʵ��
            pfoFileObject       �ļ�����
            pvcVolumeContext    ��������
            pscFileStreamContext�ļ���������
�������:
����ֵ:
            STATUS_SUCCESS �ɹ� ���򷵻���Ӧ״̬

����:       ����ͷ���ݶ�����ο�FileFunction.h ENCRYPTION_HEADER

����ά��:   2011.4.9    �޸�Ϊʹ��FltXXX�汾
---------------------------------------------------------*/
NTSTATUS
FileWriteEncryptionHeader(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT  pfoFileObject,
    __in PVOLUME_CONTEXT pvcVolumeContext,
    __in PCUST_FILE_STREAM_CONTEXT pscFileStreamContext
    )
{
    // �ļ�ͷ����
//    static WCHAR wHeader[CONFIDENTIAL_FILE_HEAD_SIZE/sizeof(WCHAR)] = ENCRYPTION_HEADER;

    // ����¼�
    KEVENT keEventComplete;

    // �ļ���С
    LARGE_INTEGER nFileSize;

    // ƫ����
    LARGE_INTEGER nOffset;

    // ���� ����Ϊ��׼����ͷ����
    ULONG ulLength = CONFIDENTIAL_FILE_HEAD_SIZE;

    // ����ֵ
    NTSTATUS status;

    // ����ͷ��ַ
    PVOID pHeader;

    // �Ƿ���Ҫ�����ļ���С
    BOOLEAN bSetSize = FALSE;

    //
    // ��ʼǰ�ȳ�ʼ���¼�����
    //
    KeInitializeEvent(
        &keEventComplete,
        SynchronizationEvent,   // ͬ���¼�
        FALSE                   // �¼���ʼ��־ΪFALSE
        );

    //
    // �����ļ���С
    //
    //FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);

    //
    // �µ��ļ� �Ѽ���ͷ��С�ӵ�FileLength����
    //
    if (pscFileStreamContext->nFileValidLength.QuadPart == 0) {
        pscFileStreamContext->nFileSize.QuadPart = CONFIDENTIAL_FILE_HEAD_SIZE;
        bSetSize = TRUE;
    } 
	//else {
        //FLT_ASSERT(pscFileStreamContext->nFileSize.QuadPart >= CONFIDENTIAL_FILE_HEAD_SIZE);
    //}

    //FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);

    if (bSetSize) {
        status = FileSetSize(
            pfiInstance,
            pfoFileObject,
            &nFileSize);

        if (!NT_SUCCESS(status))
            return status;
    }

    //
    // ����������ļ� ����Ҫд����ͷ һ���Ǹ��¼���ͷ ���ȱش���һ������ͷ
    //
/*
    nFileSize.QuadPart = pscFileStreamContext->nFileValidLength.QuadPart + CONFIDENTIAL_FILE_HEAD_SIZE;
*/
    //
    // д����ܱ�ʶͷ
    //
    nOffset.QuadPart = 0;

//  ulLength = ROUND_TO_SIZE(ulLength,pvcVolumeContext->ulSectorSize);

    pHeader = ExAllocateFromNPagedLookasideList(&nliNewFileHeaderLookasideList);

    FctEncodeCustFileStreamContextEncrytedHead(pscFileStreamContext, pHeader);

    status =  FltWriteFile(
               pfiInstance,             // ��ʼʵ��,���ڷ�ֹ����
               pfoFileObject,           // �ļ�����
               &nOffset,                // ƫ����, ��ͷд��
               CONFIDENTIAL_FILE_HEAD_SIZE,     // ulLength, // һ��ͷ�Ĵ�С
               pHeader,                 // ͷ����
               FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET | FLTFL_IO_OPERATION_NON_CACHED,  // �ǻ���д��
               NULL,                    // ����Ҫ����д����ֽ���
               FileCompleteCallback,    // �ص�, ȷ��ִ�����
               &keEventComplete         // �ص�������, ��������¼�
        );
    //
    // �ȴ� FltWriteFile ���.
    //
    KeWaitForSingleObject(&keEventComplete, Executive, KernelMode, TRUE, 0);
    ExFreeToNPagedLookasideList(&nliNewFileHeaderLookasideList, pHeader);

    return status;
}

/*---------------------------------------------------------
��������:   FileReadEncryptionHeaderAndDeconstruct
��������:   �������ȡ��������ͷ ͬʱ���
�������:
            pfiInstance         ������ʵ��
            pfoFileObject       �ļ�����
            pvcVolumeContext    ��������
            pscFileStreamContext�ļ���������
�������:
����ֵ:
            STATUS_SUCCESS �ɹ� ���򷵻���Ӧ״̬

����:

����ά��:   2011.4.9    �޸�Ϊʹ��FltXXX�汾
---------------------------------------------------------*/
NTSTATUS
FileReadEncryptionHeaderAndDecode(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT  pfoFileObject,
    __in PVOLUME_CONTEXT pvcVolumeContext,
    __in PCUST_FILE_STREAM_CONTEXT  pscFileStreamContext
    )
{
    // ����¼�
    KEVENT keEventComplete;

    // �ļ���С
    LARGE_INTEGER nFileSize;

    // ƫ����
    LARGE_INTEGER nOffset;

    // ���� ����Ϊ��׼����ͷ����
    ULONG ulLength = CONFIDENTIAL_FILE_HEAD_SIZE;

    // ����������״̬
    NTSTATUS status;

    // ������Ҫ���ص�״̬
    NTSTATUS statusRet;

    // ����ͷ��ַ
    PVOID pHeader;

    //
    // ��ʼǰ�ȳ�ʼ���¼�����
    //
    KeInitializeEvent(
        &keEventComplete,
        SynchronizationEvent,// ͬ���¼�
        FALSE// �¼���ʼ��־ΪFALSE
);

    //
    // ��ȡ���ܱ�ʶͷ
    //
    nOffset.QuadPart = 0;

//  ulLength = ROUND_TO_SIZE(ulLength,pvcVolumeContext->ulSectorSize);
    pHeader = ExAllocateFromNPagedLookasideList(&nliNewFileHeaderLookasideList);

    statusRet = FltReadFile(
                pfiInstance,            // ��ʼʵ��,���ڷ�ֹ����
                pfoFileObject,          // �ļ�����
                &nOffset,               // ƫ���� ��ͷд��
                CONFIDENTIAL_FILE_HEAD_SIZE,    // ulLength,// һ��ͷ�Ĵ�С
                pHeader,
                FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET | FLTFL_IO_OPERATION_NON_CACHED, // �ǻ���д��
                NULL,                   // ����Ҫ���ض�����ֽ���
                FileCompleteCallback,   // �ص�,ȷ��ִ�����
                &keEventComplete        // �ص�������, ��������¼�
        );

    //
    // �ȴ����
    //
    KeWaitForSingleObject(&keEventComplete, Executive, KernelMode, TRUE, 0);

    //
    // ���
    //
    FctDecodeCustFileStreamContextEncrytedHead(pscFileStreamContext,pHeader);
    ExFreeToNPagedLookasideList(&nliNewFileHeaderLookasideList,pHeader);

    //
    // �ָ�ƫ������0
    //
    status = FileSetOffset(pfiInstance,pfoFileObject,&nOffset);

    if (!NT_SUCCESS(status)) {
        statusRet = status;
    }

    return statusRet;
}

/*---------------------------------------------------------
��������:   FileIsEncrypted
��������:   �ж��ļ��Ƿ��ǻ����ļ�
�������:
            pfiInstance                 ������ʵ��
            pfoFileObject               �ļ�����
            pfcdCBD                     �ص�����
            ulFlags                     ��־λ

�������:
            pbIsFileEncrypted �ļ��Ƿ񱻼���
����ֵ:
            STATUS_SUCCESS                  �ɹ�(δд����ͷ)
            STATUS_FILE_NOT_ENCRYPTED       ���ǻ����ļ�
            STATUS_REPARSE_OBJECT           ������½����ļ� �ڱ�����
                                            �б�д���˼���ͷ

����:       ���������ƵĲ��Ǻܺ�,��д�ļ�ͷ���޸�IRP����
            Ҳ������������.��ʱ���޸���.

            �������ļ��Ѿ������ڻ��ܱ���,����������
            STATUS_IMAGE_ALREADY_LOADED

            �����ѯ����ʧ��(����������)������
            STATUS_OBJECT_NAME_NOT_FOUND

            pbAutoWriteEncryptedHeader����״̬ΪTRUEʱ���Զ�
            �������ͷ,ͬʱ���޸�FLT_PARAMETER��������,�����
            TRUE˵���Ѿ�����,��Ҫ�ⲿ����FltSetCallbackDataDirty

����ά��:   2011.4.9     ����汾
            2011.4.13    �������ж��Ƿ��Ѿ������ڱ���
            2011.4.19    Bug:δִ��FltCreateFileҲ��
                                   ����FltClose,������.
            2011.4.30    �޸Ĳ�ѯ��������ʱ�ķ���ֵ.
            2011.5.1     Bug:δ��pbIsFileEncrypted
                                   ��ֵ������жϴ���.������.
            2011.5.2     �����ļ��Ĳ���������ȥ.
            2011.7.8     Bug:�޸���FLT_PARAMETERȴû��
                                   ֪ͨFLTMGR.֪ͨ�ⲿ�Ƿ���
                                   ҪDirty.������.
            2011.7.20    �޸������ݽṹ�����˶���
            2012.1.1     Bug:���ִ˺�������һЩͼ�����
                                   �쳣,FltReadFile���,ʹ��
                                   �Լ����´򿪵��ļ����󼴿�.
                                   ������.
---------------------------------------------------------*/
NTSTATUS
FileIsEncrypted(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT pfoFileObjectOpened,
    __in PFLT_CALLBACK_DATA pfcdCBD,
    __in PVOLUME_CONTEXT pvcVolumeContext,
    __in PCUST_FILE_STREAM_CONTEXT  pscFileStreamContext,
    __in ULONG  ulFlags
    )
{
    // �Ƿ���Ŀ¼
    BOOLEAN bDirectory;

    // �ļ�����
    LARGE_INTEGER nFileSize;

    // ״̬
    NTSTATUS status;

    // ����¼�
    KEVENT keEventComplete;

    // ƫ����
    LARGE_INTEGER nOffset;

    // ���ܱ�ʶ
    WCHAR wEncryptedLogo_begin[ENCRYPTION_HEAD_LOGO_SIZE] = ENCRYPTION_HEADER_BEGIN;
	WCHAR wEncryptedLogo_end[ENCRYPTION_HEAD_LOGO_SIZE] = ENCRYPTION_HEADER_END;

    // ��ȡ����������, ����ͬ���ܱ�ʶ�Ƚ�, ��������һ����� \0 �Ŀռ�.
    WCHAR wBufferRead[CONFIDENTIAL_FILE_HEAD_SIZE] = { 0 };

    // �ļ����
    HANDLE hFile = NULL;

    // ��������,���ڴ��ļ�ʱʹ��
    OBJECT_ATTRIBUTES oaObjectAttributes;

    // �ļ�·��
    UNICODE_STRING usPath;

    // �ļ�·��������ݵ�ַ
    WCHAR wPath[NORMAL_FILE_PATH_LENGTH];

    // �ļ�·��ָ��
    PWSTR pwPath = wPath;

    // �ļ�·������
    ULONG ulPathLength;

    // ��Ҫ��Ȩ��
    ULONG ulDesiredAccess;

    // ׼�����صķ���ֵ
    NTSTATUS statusRet = STATUS_FILE_NOT_ENCRYPTED;

    // ��������ֵ
    BOOLEAN bReturn;

    // ������IO����
    PFLT_PARAMETERS  pfpParameters;

    // ���ڴ�Ŷ�ȡ���ļ���ͷ���ֵ��ڴ�
    PWCHAR pwFileHead;

    //
    // �������Ȩ�޵�ֵ, ������.
    //
    FltDecodeParameters(
        pfcdCBD,
        NULL,
        NULL,
        NULL,
        (LOCK_OPERATION *)&ulDesiredAccess);

    pfpParameters = &pfcdCBD->Iopb->Parameters;

    do {
        //
        // ��ѯ�ļ�������Ϣ
        //
        status = FileGetStandardInformation(
                pfiInstance,
                pfoFileObjectOpened,
                NULL,
                &nFileSize,
                &bDirectory);

        if (!NT_SUCCESS(status)) {
            statusRet = status;
            break;
        }

        //
        // �����Ŀ¼,ֱ�ӷ��ز���Ҫ����
        //
        if (bDirectory) {
            statusRet = STATUS_FILE_NOT_ENCRYPTED;
            break;
        }

        //
        // ����ļ���СΪ0(�½����ļ�), ����׼��д����, �����ǻ��ܽ���, ��ô����.
        //
        if ((nFileSize.QuadPart == 0)                                       // �ļ���СΪ0
            && (ulDesiredAccess & (FILE_WRITE_DATA | FILE_APPEND_DATA))     // ׼��д�ļ�
            && IsCurrentProcessConfidential()) {
            //
            // ���Ҫ���Զ��������ͷ, ��ô����д��, д��ǰ���������ļ���С.
            //
            if (!(ulFlags & FILE_IS_ENCRYPTED_DO_NOT_WRITE_LOGO)) {//��Ҫ����

                statusRet = FileWriteEncryptionHeader(
                    pfiInstance,
                    pfoFileObjectOpened,
                    pvcVolumeContext,
                    pscFileStreamContext);

                if (NT_SUCCESS(statusRet)) {
                    FileClearCache(pfoFileObjectOpened);
                    //
                    // �ָ�ƫ������ 0
                    //
                    FltDebugTraceFileAndProcess(pfiInstance,
                        DEBUG_TRACE_IMPORTANT_INFO | DEBUG_TRACE_CONFIDENTIAL,
                        "FileIsEncrypted",
                        FILE_OBJECT_NAME_BUFFER(pfoFileObjectOpened),
                        "Header has been written. Set offset. High:%d,Low: %d, Quard: %d",
                            nOffset.HighPart, nOffset.LowPart, nOffset.QuadPart);

                    nOffset.QuadPart = 0;

                    status = FileSetOffset(pfiInstance, pfoFileObjectOpened, &nOffset);

                    if (!NT_SUCCESS(status)) {
                        statusRet = status;
                        break;
                    }
                    statusRet = STATUS_REPARSE_OBJECT;
                } else {
                    FltDebugTraceFileAndProcess(pfiInstance,
                        DEBUG_TRACE_ERROR,
                        "FileIsEncrypted",
                        FILE_OBJECT_NAME_BUFFER(pfoFileObjectOpened),
                        "Cannot write header.");
                }
            }
			else//����Ҫ����
			{
				statusRet = STATUS_SUCCESS;
			}
            break;
        }

        //
        // ����ļ���СС�ڼ���ͷ,��ô�϶����Ǽ����ļ�
        //
        if (nFileSize.QuadPart < CONFIDENTIAL_FILE_HEAD_SIZE) {
            statusRet = STATUS_FILE_NOT_ENCRYPTED;
            break;
        }

        //
        // ���ڶ���ǰ�����ַ��ж��Ƿ��Ǽ����ļ�,�ַ���
        // ȡ����FileFunction.h��ENCRYPTION_HEAD_LOGO_SIZE
        //

        //
        // ��ʼǰ�ȳ�ʼ���¼�����
        //
        KeInitializeEvent(
            &keEventComplete,
            SynchronizationEvent,   // ͬ���¼�
            FALSE                   // �¼���ʼ��־ΪFALSE
            );

        nOffset.QuadPart = 0;

        //
        // ��ȡһ���ڴ�����ڴ��
        //
//      pwFileHead = (PWCHAR)ExAllocateFromNPagedLookasideList(
//          pvcVolumeContext->pnliReadEncryptedSignLookasideList);

        __try {

            status = FltReadFile(
                pfiInstance,
                pfoFileObjectOpened,
                &nOffset,
				CONFIDENTIAL_FILE_HEAD_SIZE,  // pvcVolumeContext->ulSectorSize, // ���ڷǻ������һ���Զ�һ����һ��SectorSize, ��������Ͷ�һ��ENCRYPTION_HEAD_LOGO_SIZE, //,ulLengthToRead, // ����һ����ʶ���ȵ�����
                wBufferRead,                // pwFileHead, // ������pwFileHead
                FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET,   // FLTFL_IO_OPERATION_NON_CACHED|FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET,
                NULL,
                FileCompleteCallback,
                (PVOID)&keEventComplete);

            KeWaitForSingleObject(&keEventComplete, Executive, KernelMode, TRUE, 0);

        } __except(EXCEPTION_EXECUTE_HANDLER) {

//          ExFreeToNPagedLookasideList(
//              pvcVolumeContext->pnliReadEncryptedSignLookasideList, pwFileHead);

            return STATUS_UNSUCCESSFUL;
        }

        if (!NT_SUCCESS(status)) {
            statusRet = status;
            break;
        }

        //
        // �ָ�ƫ������ 0
        //
        status = FileSetOffset(pfiInstance, pfoFileObjectOpened, &nOffset);
        if (!NT_SUCCESS(status)) {
            statusRet = status;
            break;
        }

        //
        // �Ƚϱ�־�Ƿ����
        //
        // FltDebugPrintFileObject("Read file check", pfoFileObjectOpened, FALSE);
        // DbgPrint(("\t\tRead file %ws\n", wBufferRead));

       if((RtlCompareMemory(wBufferRead, wEncryptedLogo_begin, ENCRYPTION_HEAD_LOGO_SIZE*sizeof(WCHAR))== ENCRYPTION_HEAD_LOGO_SIZE*sizeof(WCHAR))
		&& (RtlCompareMemory(((CHAR*)wBufferRead) + CONFIDENTIAL_FILE_HEAD_SIZE- ENCRYPTION_HEAD_LOGO_SIZE*sizeof(WCHAR), wEncryptedLogo_end, ENCRYPTION_HEAD_LOGO_SIZE*sizeof(WCHAR)) == ENCRYPTION_HEAD_LOGO_SIZE*sizeof(WCHAR)))
		{
            statusRet = STATUS_SUCCESS;

            FltDebugTraceFileAndProcess(pfiInstance,
                DEBUG_TRACE_IMPORTANT_INFO | DEBUG_TRACE_CONFIDENTIAL,
                "FileIsEncrypted",
                FILE_OBJECT_NAME_BUFFER(pfoFileObjectOpened),
                "Confidential file detected.");
//          ExFreeToNPagedLookasideList(
//              pvcVolumeContext->pnliReadEncryptedSignLookasideList,pwFileHead);
            break;
        }

//      ExFreeToNPagedLookasideList(
//          pvcVolumeContext->pnliReadEncryptedSignLookasideList,pwFileHead);

    } while (0);

    //
    // �ƺ���
    //
/*
    //
    // Ҫע��:
    // 1.�ļ���CREATE��ΪOPEN.
    // 2.�ļ���OVERWRITEȥ��.�����ǲ���Ҫ���ܵ��ļ�,
    // ������������.����Ļ�,��������ͼ�����ļ���,
    // ��������ļ��Ѿ�������.������ͼ�����ļ���,��
    // ����һ�λ�ȥ������ͷ.
    //

    //
    // ��������ֶ�û�� ʲô��û�� �Ͳ����޸���
    //
    if (!(ulFlags&FILE_IS_ENCRYPTED_DO_NOT_CHANGE_OPEN_WAY)) {
        ULONG ulDisp = FILE_OPEN;
        pfpParameters->Create.Options &= 0x00ffffff;
        pfpParameters->Create.Options |= (ulDisp << 24);
    }
*/

    return statusRet;
}

/*---------------------------------------------------------
��������:   FileCreateByObjectNotCreated
��������:   ���ļ�����ǰ��ͨ���ö�����ļ�
�������:
            pfiInstance                 ������ʵ��
            pfoFileObject               �ļ�����
            pfpParameters               IRP����
            phFileHandle                �����ļ�����Ŀռ�
            ulDesiredAccess             ������Ȩ��

�������:
            phFileHandle �ļ����
����ֵ:
            STATUS_SUCCESS                  �ɹ�
            STATUS_OBJECT_NAME_NOT_FOUND    δ��ѯ������

����:       �����ѯ����ʧ��(����������)������
            STATUS_OBJECT_NAME_NOT_FOUND

            ���Ҫͬ������ļ�����Ȩ��һ��,ulDesiredAccess
            ������ΪNULL

            �򿪺���Ҫʹ��FltClose�ر��ļ�

����ά��:   2011.5.2     ����汾
            2011.7.28    �޸��˲��ֲ���
            2012.1.2     Bug:����ֵΪ�ɹ���,�������Ч
---------------------------------------------------------*/
NTSTATUS
FileCreateByObjectNotCreated(
    __in PFLT_INSTANCE pfiInstance,
    __in PFLT_FILE_NAME_INFORMATION pfniFileNameInformation,
    __in PFLT_PARAMETERS pfpParameters,
    __in_opt ULONG ulDesiredAccess,
    __out HANDLE * phFileHandle
    )
{
    // ��ȡ���������� ����ͬ���ܱ�ʶ�Ƚ�
    WCHAR wBufferRead[ENCRYPTION_HEAD_LOGO_SIZE] = { 0 };

    // �ļ�����
    ULONG  FileAttributes;

    // ����Ȩ��
    ULONG  ShareAccess;

    // �򿪴���
    ULONG  CreateDisposition;

    // ��ѡ��
    ULONG  CreateOptions;

    // ��������,���ڴ��ļ�ʱʹ��
    OBJECT_ATTRIBUTES oaObjectAttributes;

    // �ļ�·������
    ULONG ulPathLength;

    // ���ص�Io״̬
    IO_STATUS_BLOCK ioStatusBlock;

    // ���ص�״̬
    NTSTATUS statusRet;

    //
    // �������
    //
    InitializeObjectAttributes(
        &oaObjectAttributes,
        &pfniFileNameInformation->Name,
        OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    CreateDisposition   = pfpParameters->Create.Options >> 24;
    CreateOptions       = pfpParameters->Create.Options & 0x00ffffff;
    ShareAccess         = pfpParameters->Create.ShareAccess;
    FileAttributes      = pfpParameters->Create.FileAttributes;

    //
    // �������Ȩ�޵�ֵ ���û������
    //
    if (!ulDesiredAccess) {
        ulDesiredAccess = pfpParameters->Create.SecurityContext->DesiredAccess;
    }

    //
    // Ϊ�˼���Windows Xp, ʹ�� FltCreateFile.
    //
    return FltCreateFile(
        pfltGlobalFilterHandle,
        pfiInstance,
        phFileHandle,
        ulDesiredAccess,
        &oaObjectAttributes,
        &ioStatusBlock,
        NULL,
        FileAttributes,
        ShareAccess,
        CreateDisposition,
        CreateOptions,
        NULL,
        NULL,
        IO_IGNORE_SHARE_ACCESS_CHECK);
}

/*---------------------------------------------------------
��������:   FileCreateForHeaderWriting
��������:   ���ļ�����ǰ��ͨ���ö�����ļ�
�������:
            pfiInstance                 ������ʵ��
            pfniFileNameInformation     �ļ�����Ϣ
            phFileHandle                �����ļ�����Ŀռ�

�������:
            phFileHandle �ļ����
����ֵ:
            STATUS_SUCCESS                  �ɹ�
            STATUS_OBJECT_NAME_NOT_FOUND    δ��ѯ������

����:       �����ѯ����ʧ��(����������)������
            STATUS_OBJECT_NAME_NOT_FOUND


            �򿪺���Ҫʹ��FltClose�ر��ļ�

����ά��:   2011.5.2     ����汾
---------------------------------------------------------*/
NTSTATUS
FileCreateForHeaderWriting(
    __in PFLT_INSTANCE pfiInstance,
    __in PUNICODE_STRING puniFileName,
    __out HANDLE * phFileHandle
    )
{
    // ��������,���ڴ��ļ�ʱʹ��
    OBJECT_ATTRIBUTES oaObjectAttributes;

    // �ļ�·������
    ULONG ulPathLength;

    // ���ص�Io״̬
    IO_STATUS_BLOCK ioStatusBlock;

    // ���ص�״̬
    NTSTATUS statusRet;

    //
    // �������
    //
    InitializeObjectAttributes(
        &oaObjectAttributes,
        puniFileName,
        OBJ_KERNEL_HANDLE|OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    //
    // Ϊ�˼���Windows Xp, ʹ��FltCreateFile.
    //
    return FltCreateFile(
        pfltGlobalFilterHandle,
        pfiInstance,
        phFileHandle,
        FILE_READ_DATA|FILE_WRITE_DATA,
        &oaObjectAttributes,
        &ioStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ,
        FILE_OPEN,
        FILE_NON_DIRECTORY_FILE,
        NULL,
        0,
        IO_IGNORE_SHARE_ACCESS_CHECK);
}

/*---------------------------------------------------------
��������:   FileClearCache
��������:   �ͷŻ��� ���ڻ��ܺͷǻ��ܽ��̶�ȡ���������
�������:   pFileObject                 �ļ�����
�������:
����ֵ:
����:
����ά��:   2011.7.17     ����汾
---------------------------------------------------------*/
// ������
void FileClearCache(PFILE_OBJECT pFileObject)
{
    // FCB
    PFSRTL_COMMON_FCB_HEADER pFcb;

    // ˯��ʱ�� ����KeDelayExecutionThread
    LARGE_INTEGER liInterval;

    // �Ƿ���Ҫ�ͷ���Դ
    BOOLEAN bNeedReleaseResource = FALSE;

    // �Ƿ���Ҫ�ͷŷ�ҳ��Դ
    BOOLEAN bNeedReleasePagingIoResource = FALSE;

    // IRQL
    KIRQL irql;

    // ѭ��ʱ�Ƿ�����
    BOOLEAN bBreak = TRUE;

    // �Ƿ���Դ������
    BOOLEAN bLockedResource = FALSE;

    // �Ƿ��Ƿ�ҳ��Դ������
    BOOLEAN bLockedPagingIoResource = FALSE;

    //
    // ��ȡFCB
    //
    pFcb = (PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext;

    //
    // ���û��FCB ֱ�ӷ���
    //
    if (pFcb == NULL) {
/*
#ifdef DBG
        __asm int 3
#endif*/
        return;
    }

    //
    // ��֤��ǰIRQL <= APC_LEVEL
    //
    if (irql = KeGetCurrentIrql() > APC_LEVEL) {
#if defined(DBG) && !defined(_WIN64)
        __asm int 3
#endif
        return;
    }

    //
    // ����˯��ʱ��
    //
    liInterval.QuadPart = -1 * (LONGLONG)50;

    //
    // �����ļ�ϵͳ
    //
    FsRtlEnterFileSystem();

    //
    // ѭ������ һ��Ҫ���� �������������
    //
    for (;;) {
        //
        // ��ʼ������
        //
        bBreak                          = TRUE;
        bNeedReleaseResource            = FALSE;
        bNeedReleasePagingIoResource    = FALSE;
        bLockedResource                 = FALSE;
        bLockedPagingIoResource         = FALSE;

        //
        // �����ļ�ϵͳ
        //
//      FsRtlEnterFileSystem();

        //
        // ��FCB������
        //
        if (pFcb->PagingIoResource) {
            bLockedPagingIoResource = ExIsResourceAcquiredExclusiveLite(pFcb->PagingIoResource);
        }

        //
        // ʹ���� ������ һ����.....
        //
        if (pFcb->Resource) {
            bLockedResource = TRUE;
            //
            // �ȳ�����һ����Դ
            //
            if (ExIsResourceAcquiredExclusiveLite(pFcb->Resource) == FALSE) {
                //
                // û�õ���Դ ����һ��
                //
                bNeedReleaseResource = TRUE;
                if (bLockedPagingIoResource) {
                    if (ExAcquireResourceExclusiveLite(pFcb->Resource, FALSE) == FALSE) {
                        bBreak = FALSE;
                        bNeedReleaseResource = FALSE;
                        bLockedResource = FALSE;
                    }
                } else {
                    ExAcquireResourceExclusiveLite(pFcb->Resource, TRUE);
                }
            }
        }

        if (bLockedPagingIoResource == FALSE) {
            if (pFcb->PagingIoResource) {

                bLockedPagingIoResource = TRUE;
                bNeedReleasePagingIoResource = TRUE;

                if (bLockedResource) {

                    if (ExAcquireResourceExclusiveLite(pFcb->PagingIoResource, FALSE) == FALSE) {

                        bBreak = FALSE;
                        bLockedPagingIoResource = FALSE;
                        bNeedReleasePagingIoResource = FALSE;
                    }
                } else {
                    ExAcquireResourceExclusiveLite(pFcb->PagingIoResource, TRUE);
                }
            }
        }

        if (bBreak) {
            break;
        }

        if (bNeedReleasePagingIoResource) {
            ExReleaseResourceLite(pFcb->PagingIoResource);
        }
        if (bNeedReleaseResource) {
            ExReleaseResourceLite(pFcb->Resource);
        }

        /*
        if (irql == PASSIVE_LEVEL) {
//          FsRtlExitFileSystem();
            KeDelayExecutionThread(KernelMode, FALSE, &liInterval);
        }
        else {
            KEVENT waitEvent;
            KeInitializeEvent(&waitEvent, NotificationEvent, FALSE);
            KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, &liInterval);
        }
        */
    }

    //
    // �����õ�����
    //
    if (pFileObject->SectionObjectPointer) {

        IO_STATUS_BLOCK ioStatus;
        IoSetTopLevelIrp( (PIRP)FSRTL_FSP_TOP_LEVEL_IRP);
        CcFlushCache(pFileObject->SectionObjectPointer, NULL, 0, &ioStatus);

        if (pFileObject->SectionObjectPointer->ImageSectionObject) {
            MmFlushImageSection(pFileObject->SectionObjectPointer, MmFlushForWrite); // MmFlushForDelete
        }

        CcPurgeCacheSection(pFileObject->SectionObjectPointer, NULL, 0, FALSE);
        IoSetTopLevelIrp(NULL);
    }

    if (bNeedReleasePagingIoResource) {
        ExReleaseResourceLite(pFcb->PagingIoResource);
    }
    if (bNeedReleaseResource) {
        ExReleaseResourceLite(pFcb->Resource);
    }

    FsRtlExitFileSystem();
/*
Acquire:
    FsRtlEnterFileSystem();

    if (Fcb->Resource)
        ResourceAcquired = ExAcquireResourceExclusiveLite(Fcb->Resource, TRUE);
    if (Fcb->PagingIoResource)
        PagingIoResourceAcquired = ExAcquireResourceExclusive(Fcb->PagingIoResource, FALSE);
    else
        PagingIoResourceAcquired = TRUE ;
    if (!PagingIoResourceAcquired) {
        if (Fcb->Resource)  ExReleaseResource(Fcb->Resource);
        FsRtlExitFileSystem();
        KeDelayExecutionThread(KernelMode,FALSE,&Delay50Milliseconds);
        goto Acquire;
    }

    if (FileObject->SectionObjectPointer) {
        IoSetTopLevelIrp( (PIRP)FSRTL_FSP_TOP_LEVEL_IRP);

        if (bIsFlushCache) {
            CcFlushCache( FileObject->SectionObjectPointer, FileOffset, Length, &IoStatus);
        }

        if (FileObject->SectionObjectPointer->ImageSectionObject) {
            MmFlushImageSection(
                FileObject->SectionObjectPointer,
                MmFlushForWrite);
        }

        if (FileObject->SectionObjectPointer->DataSectionObject) {
            PurgeRes = CcPurgeCacheSection(FileObject->SectionObjectPointer,
                NULL,
                0,
                FALSE);
        }

        IoSetTopLevelIrp(NULL);
    }

    if (Fcb->PagingIoResource)
        ExReleaseResourceLite(Fcb->PagingIoResource);

    if (Fcb->Resource)
        ExReleaseResourceLite(Fcb->Resource);

    FsRtlExitFileSystem();
    */
}
/*---------------------------------------------------------
��������:   FileGetFilePostfixName
��������:   ��ȡ�ļ���׺��
�������:
            puniFileName        �ļ�����
            puniPostfixName     �����׺����UNICODE_STRING�ṹ
                                ��������ڴ� ��ΪNULL�򷵻���Ҫ
                                ����ĳ���
�������:
            puniPostfixName     ��׺���ַ���

����ֵ:     �ļ���׺������

            �������0˵����ȡʧ��

����:
����ά��:   2011.8.9    ����汾
---------------------------------------------------------*/
USHORT
FileGetFilePostfixName(
    __in PUNICODE_STRING  puniFileName,
    __inout_opt PUNICODE_STRING puniPostfixName
    )
{
    UNICODE_STRING uniPostFix;
    USHORT usLength;

    for (usLength = puniFileName->Length / sizeof(WCHAR); usLength > 0; --usLength) {

        if (puniFileName->Buffer[usLength] == L'\\') {
            return 0;
        }

        if (puniFileName->Buffer[usLength] == L'.') {
            RtlInitUnicodeString(&uniPostFix, &puniFileName->Buffer[usLength]);

            if (puniPostfixName) {
                if (puniPostfixName->MaximumLength < uniPostFix.Length) {
                    return uniPostFix.Length;
                }

                RtlCopyUnicodeString(puniPostfixName, &uniPostFix);
                puniPostfixName->Length = puniFileName->Length - usLength * sizeof(WCHAR);

                return puniPostfixName->Length;
            } else {
                return uniPostFix.MaximumLength;
            }
        }
    }

    return 0;
}
