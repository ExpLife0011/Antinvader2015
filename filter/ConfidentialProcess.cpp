///////////////////////////////////////////////////////////////////////////////
///
/// ��Ȩ���� (c) 2011 - 2012
///
/// ԭʼ�ļ�����     : ConfidentialProcess.cpp
/// ��������         : AntinvaderDriver
/// ����ʱ��         : 2011-03-20
///
///
/// ����             : ���ܽ�������ά��ʵ���ļ� ����:
///                    ���ܽ���Hash��
///                    ���ܽ����������
///
/// ����ά��:
///  0000 [2011-03-20] ����汾.
///
///////////////////////////////////////////////////////////////////////////////

#include "ConfidentialProcess.h"
#include "BasicAlgorithm.h"

// ���ܽ���Hash��
PHASH_TABLE_DESCRIPTOR phtProcessHashTableDescriptor;

// ���̼�¼��������
NPAGED_LOOKASIDE_LIST  nliProcessContextLookasideList;

/*---------------------------------------------------------
��������:   PctInitializeHashTable
��������:   ��ʼ��Hash��
�������:
�������:
����ֵ:
            Hash���Ƿ��ʼ���ɹ� TRUEΪ�ɹ� ����ΪFALSE
����:
����ά��:   2011.3.23    ����汾
            2011.4.5     �ع�ʹ��ͨ���㷨
---------------------------------------------------------*/
BOOLEAN  PctInitializeHashTable()
{
    //
    // ֱ�ӵ���HashInitialize��ʼ��
    //
    return HashInitialize(
        &phtProcessHashTableDescriptor,
        CONFIDENTIAL_PROCESS_TABLE_POINT_NUMBER);
    return TRUE;
}

/*---------------------------------------------------------
��������:   PctGetProcessHash
��������:   ����ָ���������ݵ�Hashֵ
�������:   ppdProcessData  ������Ϣ
�������:
����ֵ:     Hashֵ
����:       Hashʹ��·������
����ά��:   2011.3.25    ����汾
---------------------------------------------------------*/
ULONG PctGetProcessHash(__in PCONFIDENTIAL_PROCESS_DATA ppdProcessData)
{
    //
    // Ӧ��ʹ��·������Hash ����Ŀ��ʹ�þ���������
    //
    return ELFhashUnicode(&ppdProcessData->usName,
                          CONFIDENTIAL_PROCESS_TABLE_POINT_NUMBER);
}

/*---------------------------------------------------------
��������:   PctConvertProcessDataToStaticAddress
��������:   ��ʼ��һ����Ҫ����Hash���е����ݽڵ�
�������:   ppdProcessData      ������Ϣ
�������:   dppdNewProcessData  �½ڵ��ַ
����ֵ:     �Ƿ��ʼ���ɹ� TRUEΪ�ɹ� FALSEΪʧ��
����:
����ά��:   2011.3.26    ����汾
---------------------------------------------------------*/
BOOLEAN PctConvertProcessDataToStaticAddress(
    __in PCONFIDENTIAL_PROCESS_DATA ppdProcessData,
    __inout  PCONFIDENTIAL_PROCESS_DATA * dppdNewProcessData
    )
{
    // �����½ṹ�ڴ�
    PCONFIDENTIAL_PROCESS_DATA ppdNewProcessData
        = (PCONFIDENTIAL_PROCESS_DATA)ExAllocatePoolWithTag(
                    NonPagedPool,   // �Ƿ�ҳ�ڴ�
                    sizeof(CONFIDENTIAL_PROCESS_DATA),  // һ�����ݽṹ��С
                    MEM_TAG_PROCESS_TABLE);
    // ����
    PWCHAR pwName =
        (PWCHAR)ExAllocatePoolWithTag(
                    NonPagedPool,   // �Ƿ�ҳ�ڴ�
                    ppdProcessData->usName.MaximumLength,   // һ�����ݽṹ��С
                    MEM_TAG_PROCESS_TABLE);

    // ·��
    PWCHAR pwPath =
        (PWCHAR)ExAllocatePoolWithTag(
                    NonPagedPool,   // �Ƿ�ҳ�ڴ�
                    ppdProcessData->usPath.MaximumLength,   // һ�����ݽṹ��С
                    MEM_TAG_PROCESS_TABLE);

    // Md5ժҪ
    PWCHAR pwMd5Digest =
        (PWCHAR)ExAllocatePoolWithTag(
                    NonPagedPool,   // �Ƿ�ҳ�ڴ�
                    ppdProcessData->usMd5Digest.MaximumLength,  // һ�����ݽṹ��С
                    MEM_TAG_PROCESS_TABLE);

    //
    // �����Դ���� ����ʧ�� �������ǲ�����ܵ�
    //
    if (!((ULONG)ppdNewProcessData &
         (ULONG)pwName &
         (ULONG)pwPath &
         (ULONG)pwMd5Digest)) {
        return FALSE;
    }

    //
    // ��ʼ���ַ���
    //
    RtlInitEmptyUnicodeString(
        &ppdNewProcessData->usMd5Digest,
        pwMd5Digest,
        ppdProcessData->usMd5Digest.MaximumLength);

    RtlInitEmptyUnicodeString(
        &ppdNewProcessData->usPath,
        pwPath,
        ppdProcessData->usName.MaximumLength);

    RtlInitEmptyUnicodeString(
        &ppdNewProcessData->usName,
        pwName,
        ppdProcessData->usName.MaximumLength);

    //
    // �����ַ���
    //
    RtlCopyUnicodeString(&ppdNewProcessData->usMd5Digest,&ppdProcessData->usMd5Digest);
    RtlCopyUnicodeString(&ppdNewProcessData->usPath,&ppdProcessData->usPath);
    RtlCopyUnicodeString(&ppdNewProcessData->usName,&ppdProcessData->usName);

    //
    // ����
    //
    *dppdNewProcessData = ppdNewProcessData;

    return TRUE;
}

/*---------------------------------------------------------
��������:   PctAddProcess
��������:   ����ܽ��̱��м���һ������
�������:   ppdProcessData  ������Ϣ
�������:
����ֵ:
            TRUEΪ�ɹ� ����ΪFALSE
����:       ppdProcessData���Զ���ת��Ϊ��̬��ַ,������
            ֻ��Ҫ�ṩ���ݼ���
����ά��:   2011.3.23  ����汾
            2011.4.26  ppdProcessData���س�ʼ��
---------------------------------------------------------*/
BOOLEAN PctAddProcess(__in PCONFIDENTIAL_PROCESS_DATA ppdProcessData)
{
    //
    // ��������ǰ��ַ�Ѿ�����ʼ����
    //
    BOOLEAN bReturn;

    PCONFIDENTIAL_PROCESS_DATA ppdNewProcessData;

    //
    // ת���ɾ�̬��ַ,��ջ�е�����ȫ���û�����
    //
    bReturn = PctConvertProcessDataToStaticAddress(
        ppdProcessData,
        &ppdNewProcessData);

    //
    // �����ʼ��ʧ��
    //
    if (!bReturn) {
        return FALSE;
    }

    //
    // ����Hash
    //
    ULONG ulHash = PctGetProcessHash(ppdProcessData);

    bReturn = HashInsertByHash(phtProcessHashTableDescriptor,
        ulHash,
        ppdNewProcessData,
        sizeof(CONFIDENTIAL_PROCESS_DATA));

    //
    // ���ʧ����, �ͷŵ��ղ�ת���ɾ�̬��ַ������ڴ�
    //
    if (!bReturn) {
        PctFreeProcessDataStatic(ppdNewProcessData, TRUE);
    }

    return bReturn;
}

/*---------------------------------------------------------
��������:   PctIsProcessDataAccordance
��������:   �ж������������Ƿ�����
�������:   ppdProcessDataOne       һ����������
            ppdProcessDataAnother   ��һ����������
            dwFlags                 ��־ ��ѡֵ���� ���û�����

            CONFIDENTIAL_PROCESS_COMPARISON_NAME �Ƚ�����
            CONFIDENTIAL_PROCESS_COMPARISON_PATH �Ƚ�·��
            CONFIDENTIAL_PROCESS_COMPARISON_MD5  �Ƚ�Md5

�������:
����ֵ:
            ��������Ϊ0,�����򷵻�ֵΪ����ֵ�Ļ���

            CONFIDENTIAL_PROCESS_COMPARISON_NAME ���Ʋ�ͬ
            CONFIDENTIAL_PROCESS_COMPARISON_PATH ·����ͬ
            CONFIDENTIAL_PROCESS_COMPARISON_MD5  Md5��ͬ

����:       �������ҪУ��Md5 PCONFIDENTIAL_PROCESS_DATA->
            usMd5Digest�ɲ���

����ά��:   2011.3.25    ����汾
---------------------------------------------------------*/
ULONG  PctIsProcessDataAccordance(
    __in PCONFIDENTIAL_PROCESS_DATA ppdProcessDataOne,
    __in PCONFIDENTIAL_PROCESS_DATA ppdProcessDataAnother,
    __in ULONG ulFlags
    )
{
    LONG lRet;      // ���������õĲ������ؼ�¼
    ULONG ulRet;    // �������ķ��ؼ�¼

    //
    // ����ĵ�ַһ������
    //
    ASSERT(ppdProcessDataOne);
    ASSERT(ppdProcessDataAnother);

    // ����ֵ
    ulRet = 0;

    if (ulFlags & CONFIDENTIAL_PROCESS_COMPARISON_MD5) {
        //
        // �����ҪУ��Md5ֵ, ����бȽ�, ��Сд����
        //
        lRet = RtlCompareUnicodeString(&ppdProcessDataOne->usMd5Digest,         // ��һ��md5
                                       &ppdProcessDataAnother->usMd5Digest,     // �ڶ���md5
                                       TRUE);                                   // ��Сд����
        // ���������򷵻�
        if (lRet) {
            ulRet |= CONFIDENTIAL_PROCESS_COMPARISON_MD5;
        }
    }

    if (ulFlags & CONFIDENTIAL_PROCESS_COMPARISON_NAME) {
        //
        // �����ҪУ������, ����бȽ�, ��Сд������
        //
        lRet = RtlCompareUnicodeString(&ppdProcessDataOne->usName,          // ��һ������
                                       &ppdProcessDataAnother->usName,      // �ڶ�������
                                       FALSE);                              // ��Сд������
        // ���������򷵻�
        if (lRet) {
            ulRet |= CONFIDENTIAL_PROCESS_COMPARISON_NAME;
        }
    }

    if (ulFlags & CONFIDENTIAL_PROCESS_COMPARISON_PATH) {
        //
        // �����ҪУ��·������,����бȽ�,��Сд������
        //
        lRet = RtlCompareUnicodeString(&ppdProcessDataOne->usPath,          // ��һ��·��
                                       &ppdProcessDataAnother->usPath,      // �ڶ���·��
                                       FALSE);                              // ��Сд������
        // ���������򷵻�
        if (lRet) {
            ulRet |= CONFIDENTIAL_PROCESS_COMPARISON_PATH;
        }
    }

    return ulRet;
}

/*---------------------------------------------------------
��������:   PctIsDataMachedCallback
��������:   �жϼ�¼�Ƿ���ͨ,��ΪHashSearchByHash�ص�
�������:   lpContext       ��Ϊ���յ��ļ�����
�������:   lpNoteData      Hash���е�����(�ļ�����)
����ֵ:
            �������Ƿ���� TRUEΪ��� ����ΪFALSE
����:

����ά��:   2011.4.5     ����汾
            2011.7.16    �޸����жϷ�ʽ
---------------------------------------------------------*/
static
BOOLEAN PctIsDataMachedCallback(
    __in PVOID lpContext,
    __in PVOID lpNoteData
    )
{
    //
    // ʹ��PctIsProcessDataAccordance�ж�,MD5��·����ʱ������
    //
    return !PctIsProcessDataAccordance(
        (PCONFIDENTIAL_PROCESS_DATA)lpContext,
        (PCONFIDENTIAL_PROCESS_DATA)lpNoteData,
        CONFIDENTIAL_PROCESS_COMPARISON_NAME    // У������
        );
}

/*---------------------------------------------------------
��������:   PctGetSpecifiedProcessDataAddress
��������:   ��ȡָ���Ľ�����Hash���е�ַ
�������:   ppdProcessDataSource    ���ڶ��յ�����
            dppdProcessDataInTable  ����Hash��ַ�Ļ�����
�������:   dppdProcessDataInTable  ����Hash���ַ
����ֵ:
            �������Ƿ���� TRUEΪ��� ����ΪFALSE
����:
            ��û���ҵ�,dppdProcessDataInTable������0

����ά��:   2011.3.25    ����汾
---------------------------------------------------------*/
BOOLEAN PctGetSpecifiedProcessDataAddress(
    __in  PCONFIDENTIAL_PROCESS_DATA ppdProcessDataSource,
    __inout_opt PCONFIDENTIAL_PROCESS_DATA * dppdProcessDataInTable
    )
{
    // Hash
    ULONG ulHash;

    // ����ֵ
    BOOLEAN bReturn;

    // �ҵ��ĵ�ַ
    PHASH_NOTE_DESCRIPTOR pndNoteDescriptor;

    //
    // ��ȡ����Hash
    //
    ulHash = PctGetProcessHash(ppdProcessDataSource);

    //
    // ֱ����Hash����
    //
    bReturn = HashSearchByHash(
        phtProcessHashTableDescriptor,
        ulHash,
        PctIsDataMachedCallback,
        ppdProcessDataSource,
        &pndNoteDescriptor);

    //
    // �ж��Ƿ�ɹ�,���Ƿ���Ҫ��������
    //
    if (bReturn) {
        if (dppdProcessDataInTable) {
            //
            // �ɹ��˾ͱ�������
            //
            *dppdProcessDataInTable = (PCONFIDENTIAL_PROCESS_DATA)(pndNoteDescriptor->lpData);
        }
        return TRUE;
    }
    return FALSE;
}

/*---------------------------------------------------------
��������:   PctUpdateProcessMd5
��������:   ����ָ�����̵�����
�������:   ppdProcessDataInTable   Hash���е�ַ
            ppdProcessDataSource    ��Md5������
�������:
����ֵ:
����:       ppdProcessDataSource��
            PCONFIDENTIAL_PROCESS_DATA->usName��usPath�ɲ���

����ά��:   2011.3.25    ����汾
---------------------------------------------------------*/

#if 0
VOID PctUpdateProcessMd5(
        __in  PCONFIDENTIAL_PROCESS_DATA ppdProcessDataInTable,
        __in  PCONFIDENTIAL_PROCESS_DATA ppdProcessDataSource
    )
{
    //
    // ����ĵ�ַһ����ȷ
    //
    ASSERT(ppdProcessDataSource);
    ASSERT(ppdProcessDataInTable);
    PROCESS_TABLE_LOCK_ON;

    //
    // ����Md5ֵ ֱ�ӿ���
    //
    RtlCopyMemory(
        &ppdProcessDataInTable->usMd5Digest,
        &ppdProcessDataSource->usMd5Digest,
        sizeof(UNICODE_STRING)
);

    PROCESS_TABLE_LOCK_OFF;
}
#endif

/*---------------------------------------------------------
��������:   PctFreeProcessDataStatic
��������:   �ͷŽ��̱��нڵ��ڴ�
�������:   ppdProcessData  Ҫ�ͷ�����Hash���ַ
            bFreeDataBase   �Ƿ���Ҫ��ppdProcessData�����ͷ���
�������:
����ֵ:
����:       ppdProcessDataΪHash���ж�Ӧ�ĵ�ַ,���ǹ���
����ά��:   2011.3.26    ����汾
---------------------------------------------------------*/
VOID PctFreeProcessDataStatic(
    __in  PCONFIDENTIAL_PROCESS_DATA ppdProcessData,
    __in  BOOLEAN bFreeDataBase
    )
{
    if (ppdProcessData->usName.Buffer) {
        ExFreePool(ppdProcessData->usName.Buffer);
    }

    if (ppdProcessData->usPath.Buffer) {
        ExFreePool(ppdProcessData->usPath.Buffer);
    }

    if (ppdProcessData-> usMd5Digest.Buffer) {
        ExFreePool(ppdProcessData->usMd5Digest.Buffer);
    }

    if (bFreeDataBase) {
        ExFreePool(ppdProcessData);
    }
}
/*---------------------------------------------------------
��������:   PctFreeHashMemoryCallback
��������:   HashDelete�ص�
�������:   lpNoteData  Ҫɾ���ļ����ݽڵ��ַ
�������:
����ֵ:
����:
����ά��:   2011.4.5     ����汾
---------------------------------------------------------*/
static
VOID
PctFreeHashMemoryCallback (
    __in PVOID lpNoteData
    )
{
    PctFreeProcessDataStatic((PCONFIDENTIAL_PROCESS_DATA)lpNoteData,FALSE);
}

/*---------------------------------------------------------
��������:   PctDeleteProcess
��������:   ɾ�����̵�����
�������:   ppdProcessData  Ҫɾ����������Hash���ַ
�������:
����ֵ:     TRUE Ϊ�ɹ� FALSEΪʧ��
����:
����ά��:   2011.3.25    ����汾
            2011.4.5     �޸�Ϊʹ��ͨ�ÿ�
---------------------------------------------------------*/
BOOLEAN PctDeleteProcess(
    __in  PCONFIDENTIAL_PROCESS_DATA ppdProcessData
    )
{
    // ��ַ����
    ASSERT(ppdProcessData);

    // ��ɾ�����̵�Hashֵ
    ULONG ulHash;

    // �ҵ��ĵ�ַ
    PHASH_NOTE_DESCRIPTOR pndNoteDescriptor;

    // ����Hashֵ
    ulHash = PctGetProcessHash(ppdProcessData);

    // ����ֵ
    BOOLEAN bReturn;

    //
    // ֱ����Hash����
    //
    bReturn = HashSearchByHash(
        phtProcessHashTableDescriptor,
        ulHash,
        PctIsDataMachedCallback,
        ppdProcessData,
        &pndNoteDescriptor);

    if (!bReturn) {
        return FALSE;
    }

    //
    // ɾ���ڵ�
    //
    return HashDelete(
        phtProcessHashTableDescriptor,
        pndNoteDescriptor,
        PctFreeHashMemoryCallback,
        TRUE);
}
/*---------------------------------------------------------
��������:   PctIsPostfixMonitored
��������:   �жϵ�ǰ��׺�Ƿ񱻼���
�������:   puniPostfix ��׺��
�������:
����ֵ:
            TRUE��ʾ����,FALSE��ʾ������
����:
����ά��:   2011.8.23    ����汾 ���ڲ���
---------------------------------------------------------*/
BOOLEAN  PctIsPostfixMonitored(PUNICODE_STRING puniPostfix)
{
    UNICODE_STRING uniPostFix1;
    UNICODE_STRING uniPostFix2;

    RtlInitUnicodeString(&uniPostFix1, L".txt");
    RtlInitUnicodeString(&uniPostFix2, L".doc");

    if (!RtlCompareUnicodeString(puniPostfix, &uniPostFix1, TRUE)) {
        return TRUE;
    }

    if (!RtlCompareUnicodeString(puniPostfix, &uniPostFix2, TRUE)) {
        return TRUE;
    }

    return FALSE;
}

/*---------------------------------------------------------
��������:   PctFreeTable
��������:   �ͷ�����Hash��
�������:
�������:
����ֵ:     TRUE Ϊ�ɹ� FALSEΪʧ��
����:       ����ж��ʱʹ��
����ά��:   2011.3.25    ����汾
            2011.4.5     �޸�Ϊʹ��ͨ�ÿ�
---------------------------------------------------------*/
BOOLEAN PctFreeTable()
{
    //
    // ֱ���ͷ�
    //
    HashFree(phtProcessHashTableDescriptor,
        PctFreeHashMemoryCallback);
    return TRUE;
}
