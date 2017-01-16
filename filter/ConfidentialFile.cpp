///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2011 - 2012
//
// ԭʼ�ļ�����     : ConfidentialFile.cpp
// ��������         : AntinvaderDriver
// ����ʱ��         : 2011-03-26
//
//
// ����             : �����ļ������ĵ�ʵ��
//
// ����ά��:
//  0001 [2011-07-28] ����汾.������һ�汾ȫ������
//
///////////////////////////////////////////////////////////////////////////////

#include "ConfidentialFile.h"
#include "AntinvaderDriver.h"
#include "CallbackRoutine.h"
#include "FileFunction.h"

/*---------------------------------------------------------
��������:   FctCreateContextForSpecifiedFileStream
��������:   Ϊָ�����ļ�������������
�������:   pfiInstance             ������ʵ��
            pfoFileObject           �ļ�����
            dpscFileStreamContext   ����������ָ��Ŀռ�
�������:
����ֵ:     STATUS_SUCCESS Ϊ�ɹ�

����:       ����һ���Ѿ����ڵ�������ʱ,���ü�����+1
            ������ͬ��
����ά��:   2011.7.28    ����汾
            2012.1.3     �����˶����ü������޸�
---------------------------------------------------------*/
NTSTATUS
FctCreateCustFileStreamContextForFileObject(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT pfoFileObject,
	__in PFLT_CALLBACK_DATA pfcdCBD,
	__in PFLT_FILE_NAME_INFORMATION pfniFileNameInformation,
    __inout PCUST_FILE_STREAM_CONTEXT * dpscFileStreamContext
   )
{
    // ����ֵ
    NTSTATUS status;

    // ���뵽����������
    PCUST_FILE_STREAM_CONTEXT pscFileStreamContext;

    // ����ɵ�������
    PCUST_FILE_STREAM_CONTEXT pscOldStreamContext;

    //
    // �Ȱѷ���ֵ��NULL
    //
    *dpscFileStreamContext = NULL;

    //
    // ���� IRQL <= APC Level
    //
    PAGED_CODE();

    //
    // �ȳ��Ի�ȡ������
    //
    status = FctGetCustFileStreamContextByFileObject(
        pfiInstance,
        pfoFileObject,
        &pscFileStreamContext);

    if (NT_SUCCESS(status))
	{
        *dpscFileStreamContext = pscFileStreamContext;
        pscFileStreamContext->ulReferenceTimes++;

        return STATUS_FLT_CONTEXT_ALREADY_DEFINED;
    }
	else if (status == STATUS_NOT_FOUND)
	{
		//
		// ����������
		//
		status = FltAllocateContext(pfltGlobalFilterHandle,
			FLT_STREAM_CONTEXT,
			FILE_STREAM_CONTEXT_SIZE,
			NonPagedPool,
			(PFLT_CONTEXT *)&pscFileStreamContext);
		if (!NT_SUCCESS(status)) {
			return status;
		}

		//
		// ��ʼ��������
		//
		RtlZeroMemory(pscFileStreamContext, FILE_STREAM_CONTEXT_SIZE);

		//
		// �����ڴ�
		//
		pscFileStreamContext->prResource = (PERESOURCE)ExAllocatePoolWithTag(
			NonPagedPool,
			sizeof(ERESOURCE),
			MEM_TAG_FILE_TABLE);

		if (!pscFileStreamContext->prResource) {
			//
			// û���ڴ���
			//
			FltReleaseContext(pscFileStreamContext);
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		ExInitializeResourceLite(pscFileStreamContext->prResource);

		//
		// ����������
		//
		status = FltSetStreamContext(
			pfiInstance,
			pfoFileObject,
			FLT_SET_CONTEXT_KEEP_IF_EXISTS,
			pscFileStreamContext,
			(PFLT_CONTEXT *)&pscOldStreamContext);

		if (!NT_SUCCESS(status)) {
			FctFreeCustFileStreamContext(pscFileStreamContext);
			FltReleaseContext(pscFileStreamContext);
			return status;
		}

		status = FctInitializeCustFileStreamContext(pscFileStreamContext, pfcdCBD, pfniFileNameInformation);
		if (!NT_SUCCESS(status))
		{
			FctFreeCustFileStreamContext(pscFileStreamContext);
			FltReleaseContext(pscFileStreamContext);
			return status;
		}

		//
		// ͬ��
		//
		FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);

		//
		// ���淵��ֵ
		//
		*dpscFileStreamContext = pscFileStreamContext;

		return STATUS_SUCCESS;
	}
	return status;
}

/*---------------------------------------------------------
��������:   FctGetSpecifiedFileStreamContext
��������:   ��ȡ�ļ��������� ������ͬ��
�������:   pfiInstance             ������ʵ��
            pfoFileObject           �ļ�����
            dpscFileStreamContext   �����ļ���ָ��Ŀռ�
�������:
����ֵ:     STATUS_SUCCESS Ϊ�ɹ�
����:
����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
NTSTATUS
FctGetCustFileStreamContextByFileObject(
    __in PFLT_INSTANCE pfiInstance,
    __in PFILE_OBJECT pfoFileObject,
    __inout PCUST_FILE_STREAM_CONTEXT * dpscFileStreamContext
   )
{
    NTSTATUS status;

    status =  FltGetStreamContext(
        pfiInstance,
        pfoFileObject,
        (PFLT_CONTEXT *)dpscFileStreamContext);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    FILE_STREAM_CONTEXT_LOCK_ON((*dpscFileStreamContext));

    return status;
}

/*---------------------------------------------------------
��������:   FctInitializeContext
��������:   Ϊָ�����ļ������������� ������ͬ��
�������:   pscFileStreamContext    �����ļ���ָ��Ŀռ�
�������:
����ֵ:     STATUS_SUCCESS Ϊ�ɹ�
����:
����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
NTSTATUS
FctInitializeCustFileStreamContext(
    __inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext,
    __in PFLT_CALLBACK_DATA pfcdCBD,
    __in PFLT_FILE_NAME_INFORMATION pfniFileNameInformation
   )
{
    // ����ֵ
    NTSTATUS status;

    status = FctUpdateCustFileStreamContextFileName(
        &pfniFileNameInformation->Name,
        pscFileStreamContext);

    if (!NT_SUCCESS(status)) {
        // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);
        return status;
    }

    //
    // ��������� �� ��ʼ����������
    //
    RtlCopyMemory(pscFileStreamContext->wszVolumeName,
        pfniFileNameInformation->Volume.Buffer,
        pfniFileNameInformation->Volume.Length) ;

    pscFileStreamContext->ulReferenceTimes = 1;

    pscFileStreamContext->bCached = CALLBACK_IS_CACHED(pfcdCBD->Iopb);
    pscFileStreamContext->fctEncrypted = ENCRYPTED_TYPE_UNKNOWN;
    pscFileStreamContext->fosOpenStatus = OPEN_STATUS_UNKNOWN;
    pscFileStreamContext->bIsNeedRewriteFileEncryptedHeadWhenClose = FALSE;

    //
    // ���ﻹ��֪��������û�м��� ��������Ϊ��δ�����ļ�
    //
    status = FileGetStandardInformation(
        pfcdCBD->Iopb->TargetInstance,
        pfcdCBD->Iopb->TargetFileObject,
        NULL,
        &pscFileStreamContext->nFileValidLength,
        NULL);

    if (!NT_SUCCESS(status)) {
        // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);
        return status;
    }

    //
    // ����
    //
    // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);

    return STATUS_SUCCESS;
}

/*---------------------------------------------------------
��������:   FctUpdateStreamContextFileName
��������:   �������ļ��������е�����
�������:   pusName                 �µ�����
            pscFileStreamContext    �ļ���������ָ��
�������:
����ֵ:     STATUS_SUCCESS Ϊ�ɹ�
����:
����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
NTSTATUS
FctUpdateCustFileStreamContextFileName(
    __in PUNICODE_STRING pusName,
    __inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext
    )
{
    // ����ֵ
    NTSTATUS status = STATUS_SUCCESS ;

    // ��׺������
    USHORT usPostFixLength;

    //
    // ����Ѿ��������� ���ͷŵ�
    //
    if (pscFileStreamContext->usName.Buffer != NULL) {
        ExFreePoolWithTag( pscFileStreamContext->usName.Buffer,MEM_TAG_FILE_TABLE);
        pscFileStreamContext->usName.Length = 0;
        pscFileStreamContext->usName.MaximumLength = 0;
        pscFileStreamContext->usName.Buffer = NULL;
    }

    //
    // ���벢����������
    //
    pscFileStreamContext->usName.MaximumLength = pusName->Length;
    pscFileStreamContext->usName.Buffer = (PWCH)ExAllocatePoolWithTag(
                                            NonPagedPool,
                                            pscFileStreamContext->usName.MaximumLength,
                                            MEM_TAG_FILE_TABLE);

    if (pscFileStreamContext->usName.Buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyUnicodeString(&pscFileStreamContext->usName, pusName);

    usPostFixLength = FileGetFilePostfixName(&pscFileStreamContext->usName,NULL);

    if (usPostFixLength) {
        pscFileStreamContext->usPostFix.Buffer = (PWCH)ExAllocatePoolWithTag(
                                            NonPagedPool,
                                            usPostFixLength,
                                            MEM_TAG_FILE_TABLE);

        if (pscFileStreamContext->usPostFix.Buffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        pscFileStreamContext->usPostFix.Length = 0;
        pscFileStreamContext->usPostFix.MaximumLength = usPostFixLength;

        FileGetFilePostfixName(&pscFileStreamContext->usName,&pscFileStreamContext->usPostFix);
    }
    else {
        pscFileStreamContext->usPostFix.Length = 0;
        pscFileStreamContext->usPostFix.MaximumLength = 0;
        pscFileStreamContext->usPostFix.Buffer = NULL;
    }

    return status;
}

/*---------------------------------------------------------
��������:   FctFreeStreamContext
��������:   �ͷ��ļ���������
            pscFileStreamContext    �ļ���������ָ��
�������:
����ֵ:     STATUS_SUCCESS Ϊ�ɹ�
����:
����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
NTSTATUS
FctFreeCustFileStreamContext(
    __inout PCUST_FILE_STREAM_CONTEXT  pscFileStreamContext
    )
{
    NTSTATUS status;

    if (pscFileStreamContext == NULL) {
        return STATUS_SUCCESS;
    }

    if (pscFileStreamContext->usName.Buffer != NULL) {
        ExFreePoolWithTag(pscFileStreamContext->usName.Buffer, MEM_TAG_FILE_TABLE);

        pscFileStreamContext->usName.Length = 0;
        pscFileStreamContext->usName.MaximumLength = 0;
        pscFileStreamContext->usName.Buffer = NULL;
    }

    if (pscFileStreamContext->usPostFix.Buffer != NULL) {
        ExFreePoolWithTag(pscFileStreamContext->usPostFix.Buffer, MEM_TAG_FILE_TABLE);

        pscFileStreamContext->usPostFix.Length = 0;
        pscFileStreamContext->usPostFix.MaximumLength = 0;
        pscFileStreamContext->usPostFix.Buffer = NULL;
    }

    if (pscFileStreamContext->prResource != NULL) {
        status = ExDeleteResourceLite(pscFileStreamContext->prResource);

        if (!NT_SUCCESS(status)) {
            return status;
        }

        ExFreePoolWithTag(pscFileStreamContext->prResource, MEM_TAG_FILE_TABLE);
    }
    return STATUS_SUCCESS;
}

/*---------------------------------------------------------
��������:   FctConstructFileHead
��������:   �ͷ��ļ���������
            pscFileStreamContext    �ļ���������ָ��
            pFileHead               ����ͷ�ĵ�ַ
�������:   pFileHead               ��д�õ�ͷ

����ֵ:     STATUS_SUCCESS Ϊ�ɹ�
����:       ��Ҫ�����������ڴ�
����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
NTSTATUS
FctEncodeCustFileStreamContextEncrytedHead(
    __in    PCUST_FILE_STREAM_CONTEXT pscFileStreamContext,
    __inout PVOID pFileHead
    )
{
    WCHAR wHeaderLogo_begin[ENCRYPTION_HEAD_LOGO_SIZE] = ENCRYPTION_HEADER_BEGIN;
	WCHAR wHeaderLogo_end[ENCRYPTION_HEAD_LOGO_SIZE] = ENCRYPTION_HEADER_END;

    PCUST_FILE_ENCRYPTION_HEAD pfehFileEncryptionHead = (PCUST_FILE_ENCRYPTION_HEAD)pFileHead;
    RtlZeroMemory(pfehFileEncryptionHead, CONFIDENTIAL_FILE_HEAD_SIZE);
    RtlCopyMemory(pfehFileEncryptionHead, wHeaderLogo_begin, ENCRYPTION_HEAD_LOGO_SIZE);

    pfehFileEncryptionHead->nFileValidLength = pscFileStreamContext->nFileValidLength.QuadPart;
    pfehFileEncryptionHead->nFileRealSize    = pscFileStreamContext->nFileSize.QuadPart;

	RtlCopyMemory(pfehFileEncryptionHead+ CONFIDENTIAL_FILE_HEAD_SIZE- ENCRYPTION_HEAD_LOGO_SIZE, wHeaderLogo_end, ENCRYPTION_HEAD_LOGO_SIZE);

    return STATUS_SUCCESS;
}

/*---------------------------------------------------------
��������:   FctDeconstructFileHead
��������:   �ͷ��ļ���������
            pscFileStreamContext    �ļ���������ָ��
            pFileHead               ����ͷ�ĵ�ַ
�������:   pscFileStreamContext    ����õ�������

����ֵ:     STATUS_SUCCESS Ϊ�ɹ�
����:       ��Ҫ�����߶�ȡ���ļ�ͷ
����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
NTSTATUS
FctDecodeCustFileStreamContextEncrytedHead(
    __inout PCUST_FILE_STREAM_CONTEXT  pscFileStreamContext,
    __in PVOID  pFileHead
    )
{
    PCUST_FILE_ENCRYPTION_HEAD pfehFileEncryptionHead = (PCUST_FILE_ENCRYPTION_HEAD)pFileHead;

    pscFileStreamContext->nFileValidLength.QuadPart = pfehFileEncryptionHead->nFileValidLength;
    pscFileStreamContext->nFileSize.QuadPart        = pfehFileEncryptionHead->nFileRealSize;

    pscFileStreamContext->bIsNeedRewriteFileEncryptedHeadWhenClose          = FALSE;

    return STATUS_SUCCESS;
}

/*---------------------------------------------------------
��������:   FctUpdateFileValidSize
��������:   �����ļ���Ч��С
            pscFileStreamContext    �ļ���������ָ��
            pnFileValidSize         �ļ���Ч��С
            bSetUpdateWhenClose     �Ƿ�������UpdateWhenClose
                                    ��־
�������:   pscFileStreamContext    ���úõ�������

����ֵ:     ��
����:       ��������

            ���bSetUpdateWhenCloseΪTRUE ���Զ�����
            UpdateWhenClose��־

����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
// FORCEINLINE
VOID
FctUpdateCustFileStreamContextValidSize(
    __inout PCUST_FILE_STREAM_CONTEXT  pscFileStreamContext,
    __in    PLARGE_INTEGER        pnFileValidSize,
    __in    BOOLEAN               bSetUpdateWhenClose
    )
{
    pscFileStreamContext->nFileValidLength.QuadPart = pnFileValidSize->QuadPart;

    if (bSetUpdateWhenClose) {
        pscFileStreamContext->bIsNeedRewriteFileEncryptedHeadWhenClose = TRUE;
    }
}

/*---------------------------------------------------------
��������:   FctGetFileValidSize
��������:   �����ļ���Ч��С
            pscFileStreamContext    �ļ���������ָ��
            pnFileValidSize         ����ļ���Ч��С��ָ��

�������:   pnFileValidSize         �ļ���Ч��С

����ֵ:     ��
����:       ��������
            pnFileValidSize ��Ҫָ����Ч��ַ
����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
// FORCEINLINE
VOID
FctGetCustFileStreamContextValidSize(
    __in    PCUST_FILE_STREAM_CONTEXT  pscFileStreamContext,
    __inout PLARGE_INTEGER        pnFileValidSize
    )
{
    // FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);

    pnFileValidSize->QuadPart = pscFileStreamContext->nFileValidLength.QuadPart;

    // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);
}


/*---------------------------------------------------------
��������:   FctUpdateFileValidSizeIfLonger
��������:   ��������С�����Ļ�,�����ļ���Ч��С
            pscFileStreamContext    �ļ���������ָ��
            pnFileValidSize         �ļ���Ч��С
            bSetUpdateWhenClose     �Ƿ�������UpdateWhenClose
                                    ��־
�������:   pscFileStreamContext    ���úõ�������

����ֵ:     TRUE Ϊ�ɹ�����,FALSEΪû������
����:       ��������

            ���bSetUpdateWhenCloseΪTRUE ���Զ�����
            UpdateWhenClose��־,�����û�и����ļ���Ч��С
            ��־���ᱻ����.

����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
// FORCEINLINE
BOOLEAN
FctUpdateCustFileStreamContextValidSizeIfLonger(
    __inout PCUST_FILE_STREAM_CONTEXT  pscFileStreamContext,
    __in    PLARGE_INTEGER        pnFileValidSize,
    __in    BOOLEAN               bSetUpdateWhenClose
    )
{
    BOOLEAN bReturn = FALSE;

    // FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);

    if (pnFileValidSize->QuadPart > pscFileStreamContext->nFileValidLength.QuadPart) {
        pscFileStreamContext->nFileValidLength.QuadPart = pnFileValidSize->QuadPart;
        if (bSetUpdateWhenClose) {
            pscFileStreamContext->bIsNeedRewriteFileEncryptedHeadWhenClose = TRUE;
        }
        bReturn = TRUE;
    }
    // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);

    return bReturn;
}

/*---------------------------------------------------------
��������:   FctUpdateFileConfidentialCondition
��������:   ���õ�ǰ�ļ��Ƿ�Ϊ�����ļ�
            pscFileStreamContext    �ļ���������ָ��
            fetFileEncryptedType    �ļ�����

�������:   pscFileStreamContext    ���úõ�������

����ֵ:     ��
����:       ��������

            fetFileEncryptedType��ѡֵ��

            ENCRYPTED_TYPE_UNKNOWN          ��֪���������
            ENCRYPTED_TYPE_CONFIDENTIAL     �ļ��ǻ��ܵ�
            ENCRYPTED_TYPE_NOT_CONFIDENTIAL �ļ��Ƿǻ��ܵ�

����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
// FORCEINLINE
VOID
FctSetCustFileStreamContextEncryptedType(
    __inout PCUST_FILE_STREAM_CONTEXT  pscFileStreamContext,
    __in    FILE_ENCRYPTED_TYPE   fetFileEncryptedType
    )
{
    // FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);

    pscFileStreamContext->fctEncrypted = fetFileEncryptedType;

    // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);
}

/*---------------------------------------------------------
��������:   FctGetFileConfidentialCondition
��������:   ��ȡ��ǰ�ļ��Ƿ�Ϊ�����ļ�
            pscFileStreamContext    �ļ���������ָ��

�������:   ��

����ֵ:     �ļ���ǰ��� ȡֵ��

            ENCRYPTED_TYPE_UNKNOWN          ��֪���������
            ENCRYPTED_TYPE_CONFIDENTIAL     �ļ��ǻ��ܵ�
            ENCRYPTED_TYPE_NOT_CONFIDENTIAL �ļ��Ƿǻ��ܵ�

����:       ��������

����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
// FORCEINLINE
FILE_ENCRYPTED_TYPE
FctGetCustFileStreamContextEncryptedType(
    __in    PCUST_FILE_STREAM_CONTEXT  pscFileStreamContext
    )
{
    //FILE_ENCRYPTED_TYPE fetFileEncryptedType;

    //// FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);

    //fetFileEncryptedType = pscFileStreamContext->fctEncrypted;

    //// FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);

    return pscFileStreamContext->fctEncrypted;
}

/*---------------------------------------------------------
��������:   FctDereferenceFileContext
��������:   ���ü�����һ
            pscFileStreamContext    �ļ���������ָ��

�������:   pscFileStreamContext    ���úõ�������

����ֵ:     ��
����:       ��������
            ����ȷ���Ƿ��ļ���ȫ���ر�,�����д����ͷʱ��

����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
// FORCEINLINE
VOID
FctDecCustFileStreamContextReferenceCount(
    __inout PCUST_FILE_STREAM_CONTEXT  pscFileStreamContext
    )
{
    // FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);

	if (pscFileStreamContext->ulReferenceTimes > 0)
	{
		--(pscFileStreamContext->ulReferenceTimes);
	}

    // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);
}
/*---------------------------------------------------------
��������:   FctReferenceFileContext
��������:   ���ü�����һ
            pscFileStreamContext    �ļ���������ָ��

�������:   pscFileStreamContext    ���úõ�������

����ֵ:     ��
����:       ��������
            ����ȷ���Ƿ��ļ���ȫ���ر�,�����д����ͷʱ��

����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
// FORCEINLINE
//VOID
//FctIncCustFileStreamContextReferenceCount(
//    __inout PCUST_FILE_STREAM_CONTEXT  pscFileStreamContext
//    )
//{
//    // FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);
//
//    ASSERT(pscFileStreamContext->ulReferenceTimes != 0);
//
//    ++(pscFileStreamContext->ulReferenceTimes);
//
//    // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);
//}

/*---------------------------------------------------------
��������:   FctIsReferenceCountZero
��������:   ���ü����Ƿ񽵵�0
            pscFileStreamContext    �ļ���������ָ��

�������:   ��

����ֵ:     ��
����:       ��������
            ����ȷ���Ƿ��ļ���ȫ���ر�,�����д����ͷʱ��

����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
// FORCEINLINE
BOOLEAN
FctIsCustFileStreamContextReferenceCountZero(
    __in    PCUST_FILE_STREAM_CONTEXT  pscFileStreamContext
    )
{
    //BOOLEAN bIsZero;

    // FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);

    //bIsZero = !pscFileStreamContext->ulReferenceTimes;

    // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);

    return 0 == pscFileStreamContext->ulReferenceTimes;
}

/*---------------------------------------------------------
��������:   FctSetUpdateWhenCloseFlag
��������:   ����UpdateWhenCloseFlag��־
            pscFileStreamContext    �ļ���������ָ��
            bSet                    ��Ҫ��д���ǲ���Ҫ

�������:   pscFileStreamContext    ���úõ�������

����ֵ:     ��
����:       ��������

            bSet ΪTrue��ʾ��Ҫ��ˢ False��ʾ����

����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
// FORCEINLINE
VOID
FctSetIsNeedRewriteFileEncryptedHeadWhenClose(
    __inout PCUST_FILE_STREAM_CONTEXT    pscFileStreamContext,
    __in    BOOLEAN                 bSet
    )
{
    // FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);

    pscFileStreamContext->bIsNeedRewriteFileEncryptedHeadWhenClose = bSet;

    // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);
}

/*---------------------------------------------------------
��������:   FctIsUpdateWhenCloseFlag
��������:   �Ƿ�������UpdateWhenCloseFlag��־

            pscFileStreamContext    �ļ���������ָ��

�������:   ��

����ֵ:     ��
����:       ��������

            ����ֵΪTrue��ʾ��Ҫˢ False ��ʾ����Ҫ

����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
// FORCEINLINE
BOOLEAN
FctIsNeedRewriteFileEncryptedHeadWhenClose(__inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext)
{
    //BOOLEAN bUpdateWhenClose;

    // FILE_STREAM_CONTEXT_LOCK_ON(pscFileStreamContext);

    //bUpdateWhenClose = pscFileStreamContext->bIsNeedRewriteFileEncryptedHeadWhenClose;

    // FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);

    return pscFileStreamContext->bIsNeedRewriteFileEncryptedHeadWhenClose;
}

/*---------------------------------------------------------
��������:   FctReleaseStreamContext
��������:   �ͷ�������,���м�����ͬ��
            �����滻FltReleaseContext

            pscFileStreamContext    �ļ���������ָ��

�������:   ��

����ֵ:     ��
����:       ��������

����ά��:   2011.7.28    ����汾
---------------------------------------------------------*/
// FORCEINLINE
VOID FctReleaseCustFileStreamContext(__inout PCUST_FILE_STREAM_CONTEXT pscFileStreamContext)
{
	FctFreeCustFileStreamContext(pscFileStreamContext);

    FILE_STREAM_CONTEXT_LOCK_OFF(pscFileStreamContext);
    FltReleaseContext(pscFileStreamContext);
}
