///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2011 - 2012
//
// ԭʼ�ļ�����     : BasicAlgorithm.cpp
// ��������         : AntinvaderDriver
// ����ʱ��         : 2011-03-20
//
//
// ����             : һЩ�������㷨����ʵ��,��������Hash,���������
//
// ����ά��:
//  0000 [2011-03-20] ����汾.
//
///////////////////////////////////////////////////////////////////////////////

#include "BasicAlgorithm.h"
#include "AntinvaderDef.h"

/*---------------------------------------------------------
��������:   ELFhashAnsi
��������:   ELF�㷨�����ַ���Hash Ansi�汾
�������:   pansiKey    ��ҪHash���ַ���
            ulMod       ���Hashֵ��Χ
�������:
����ֵ:     Hashֵ
����:
        ELF hash�Ƕ��ַ�������hash����ʱ�ĳ��ú�����һ����
        �����������е�ÿ��Ԫ�����ΰ�ǰ��λ����һ��Ԫ�ص�
        ����λ����,���һ��������,��������ĸ���λ������,
        ��ô�ͽ����ۻ����볤���ĵ���λ�����,�������õ�
        �ĳ�����HASH��ȡ��,�õ���HASH�е�λ��.
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
ULONG ELFhashAnsi(PANSI_STRING pansiKey, ULONG ulMod)
{
    ULONG ulHash = 0;
    ULONG ulTemp;

    PCHAR pchBuffer;

    pchBuffer = pansiKey->Buffer;

    while (*pchBuffer) {
        ulHash = (ulHash << 4) + *pchBuffer++;
        ulTemp = ulHash & 0xf0000000L;
        if (ulTemp)
            ulHash ^= ulTemp >> 24;
        ulHash &= ~ulTemp;
    }

    return ulHash % ulMod;
}

/*---------------------------------------------------------
��������:   ELFhashUnicode
��������:   ELF�㷨�����ַ���Hash Unicode�汾, ת����ʹ����
            Ansi�汾.
�������:   pansiKey    ��ҪHash���ַ���
            ulMod       ���Hashֵ��Χ
�������:
����ֵ:     Hashֵ
����:

����ά��:   2011.3.20    ����汾
            2011.7.16    �����Ansi�����ڴ�
---------------------------------------------------------*/
ULONG ELFhashUnicode(PUNICODE_STRING pusKey, ULONG ulMod)
{
    // ת��ansi�ַ���
    ANSI_STRING ansiKey;

    // ��ȡ��Hashֵ
    ULONG ulHash;

    //
    // ת����Ansi
    //
    RtlUnicodeStringToAnsiString(
        &ansiKey,
        pusKey,
        TRUE        // ��RtlUnicodeStringToAnsiString�����ڴ�
        );

    //
    // ֱ����ansi�汾
    //
    ulHash = ELFhashAnsi(&ansiKey, ulMod);

    //
    // �ͷ��ڴ�
    //
    RtlFreeAnsiString(&ansiKey);

    return ulHash;
}

/*---------------------------------------------------------
��������:   HashInitialize
��������:   ��ʼ���µ�Hash��

�������:   dpHashTable                 ����Hash������
            ulMaximumPointNumber        ���Hashֵ��Χ

�������:   dpHashTable                 Hash������

����ֵ:     TRUEΪ�ɹ� FALSEʧ��
����:
����ά��:   2011.4.2     ����汾
            2011.7.16    ����������Ϊ�˻�����
---------------------------------------------------------*/
BOOLEAN
HashInitialize(
    PHASH_TABLE_DESCRIPTOR * dpHashTable,
    ULONG ulMaximumPointNumber
    )
{
    // �������һ����ȷ
    ASSERT(dpHashTable);
    ASSERT(ulMaximumPointNumber);

    //
    // �ֱ����������ڴ�, �ֱ��Ǳ��ڴ�ͱ������ṹ,
    // ���ʧ���򷵻�, �������ṹ, ��������Ȼ�󷵻�.
    //
    PHASH_TABLE_DESCRIPTOR pHashTable;
    pHashTable = (PHASH_TABLE_DESCRIPTOR)ExAllocatePoolWithTag(
                        NonPagedPool,
                        sizeof(HASH_TABLE_DESCRIPTOR),
                        MEM_HASH_TAG);

    if (!pHashTable) {
        return FALSE;
    }

    pHashTable->ulHashTableBaseAddress = (ULONG)ExAllocatePoolWithTag(
                        NonPagedPool,
                        HASH_POINT_SIZE * ulMaximumPointNumber,
                        MEM_HASH_TAG);

    if (!pHashTable) {
        ExFreePool(pHashTable);
        return FALSE;
    }

    RtlZeroMemory((PVOID)pHashTable->ulHashTableBaseAddress,
        HASH_POINT_SIZE * ulMaximumPointNumber);

    //
    // ��ʼ�������� ������Ϣ
    //
    KeInitializeMutex(&pHashTable->irmMutex, 0);

    pHashTable->ulMaximumPointNumber = ulMaximumPointNumber;

    //
    // ����
    //
    *dpHashTable = pHashTable;

    return TRUE;
}

/*---------------------------------------------------------
��������:   HashInsertByHash
��������:   ͨ���ύHashֵ����Hash��ڵ�

�������:   pHashTable          Hash��������
            ulHash              Hashֵ
            lpData              �������������
            ulLength            �������ݳ���

�������:
����ֵ:     TRUEΪ�ɹ� FALSEʧ��
����:       �������ݽ��ᱻ����,���ڽڵ�ɾ��ʱ���ͷ�
����ά��:   2011.4.4     ����汾
---------------------------------------------------------*/
BOOLEAN
HashInsertByHash(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in ULONG ulHash,
    __in PVOID lpData,
    __in ULONG ulLength
    )
{
    // Hash��Ӧ�ĵ�һ����ַ
    ULONG ulPointAddress;

    // �µĽڵ��ַ
    PHASH_NOTE_DESCRIPTOR pHashNote;

    // ��һ������Ľڵ��ַ, ����ȷ����ͻ.
    PHASH_NOTE_DESCRIPTOR pFirstNote;

    // �������ݿռ��ַ
    PVOID lpBuffer;

    //
    // ��ȡHash��Ӧ�ĵ�ַ, ��ʼ��һ���µĽڵ�, ʧ�ܾͷ���.
    //
    pHashNote = (PHASH_NOTE_DESCRIPTOR)ExAllocatePoolWithTag(
                        NonPagedPool,
                        sizeof(HASH_NOTE_DESCRIPTOR),
                        MEM_HASH_TAG);

    if (!pHashNote) {
        return FALSE;
    }

    ulPointAddress = HASH_NOTE_POINT_ADDRESS(pHashTable, ulHash);

    //
    // �������������ڴ沢����
    //
    lpBuffer = ExAllocatePoolWithTag(
                    NonPagedPool,
                    ulLength,
                    MEM_HASH_TAG);

    if (!lpBuffer) {
        ExFreePool(pHashNote);
        return FALSE;
    }

    RtlCopyMemory(lpBuffer, lpData, ulLength);

    //
    // �漰���������, �����￪ʼ����.
    //
    HASH_LOCK_ON(pHashTable);

    pFirstNote = *(PHASH_NOTE_DESCRIPTOR *)ulPointAddress;
    if (!pFirstNote) {
        //
        // �����û�д�Ź�����,��ʼ������,ֱ��д�� ����ʼ������ͷ
        //
        *(PULONG)ulPointAddress = (ULONG)pHashNote;

        InitializeListHead((PLIST_ENTRY)pHashNote);
    } else {
        //
        // ������ڳ�ͻ, ���������
        //
        InsertTailList((PLIST_ENTRY)pFirstNote, (PLIST_ENTRY)pHashNote);
    }

    //
    // ���� Hash ɾ��ʱʹ��
    //
    pHashNote->ulHash = ulHash;

    //
    // �����������ݵ�ַ
    //
    pHashNote->lpData = lpBuffer;

    HASH_LOCK_OFF(pHashTable);

    return TRUE;
}

/*---------------------------------------------------------
��������:   HashInsertByNumber
��������:   ͨ���ύ���ֲ���Hash��ڵ�

�������:   pHashTable          Hash��������
            ulNumber            ���ڼ���Hash������
            lpData              �������������
            ulLength            �������ݳ���

�������:
����ֵ:     TRUEΪ�ɹ� FALSEʧ��
����:       �������ݽ��ᱻ����,���ڽڵ�ɾ��ʱ���ͷ�
����ά��:   2011.4.4     ����汾
---------------------------------------------------------*/
BOOLEAN
HashInsertByNumber(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in ULONG ulNumber,
    __in PVOID lpData,
    __in ULONG ulLength
    )
{
    return HashInsertByHash(
                pHashTable,
                ulNumber % (pHashTable->ulMaximumPointNumber),
                lpData,
                ulLength);
}

/*---------------------------------------------------------
��������:   HashInsertByUnicodeString
��������:   ͨ���ύUnicode�ַ�������Hash��ڵ�

�������:   pHashTable          Hash��������
            pusString           ���ڼ���Hash��Unicode�ַ���
            lpData              �������������
            ulLength            �������ݳ���

�������:
����ֵ:     TRUEΪ�ɹ� FALSEʧ��
����:       �������ݽ��ᱻ����,���ڽڵ�ɾ��ʱ���ͷ�
����ά��:   2011.4.4     ����汾
---------------------------------------------------------*/
BOOLEAN
HashInsertByUnicodeString(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in PUNICODE_STRING pusString,
    __in PVOID lpData,
    __in ULONG ulLength
    )
{
    return HashInsertByHash(
                pHashTable,
                ELFhashUnicode(
                    pusString,
                    pHashTable->ulMaximumPointNumber),
                lpData,
                ulLength);
}

/*---------------------------------------------------------
��������:   HashSearchByHash
��������:   ͨ���ύHashֵ�ķ�ʽ����Hash��ڵ�

�������:   pHashTable          Hash��������

            ulHash              Hashֵ

            CallBack            �ص�����,�����ж��Ƿ��ҵ�,
                                ��ϸ��Ϣ��μ�BasicAlgorithm.h

            lpContext           �ص�������������,����Ϊ��������
                                ���ص�����

            dpData              �����ҵ��ڵ��ַ�Ŀռ�

�������:   dpData              �ҵ��Ľڵ��ַ

����ֵ:     TRUEΪ�ҵ� FALSEʧ��

����:       ���ֻ�����ѯ�Ƿ��ڱ��� �ɽ�dpData��ΪNULL

����ά��:   2011.4.4     ����汾
            2011.7.17    ������dpData�ж�
---------------------------------------------------------*/
BOOLEAN
HashSearchByHash(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in ULONG ulHash,
    __in HASH_IS_NOTE_MACHED_CALLBACK CallBack,
    __in PVOID lpContext,
    __inout PHASH_NOTE_DESCRIPTOR * dpData
    )
{
    // ��ǰ�ڵ�
    PHASH_NOTE_DESCRIPTOR pCurrentHashNote;

    // ��һ���ڵ�
    PHASH_NOTE_DESCRIPTOR pFirstHashNote;

    // �Ƿ��Ѿ��ҵ�
    BOOLEAN bFind = FALSE;

    //
    // �����˷�ҳ�ڴ� һ��Ҫ��PASSIVE_LEVEL��
    //
    // ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
    PAGED_CODE();

    //
    // ֱ�ӷ��ص�ַ
    // �漰��������� ʹ��������
    //
    HASH_LOCK_ON(pHashTable);

    //
    // ��ȡ��һ���ڵ�,���û�����ݾͷ���
    //
    pFirstHashNote = *(PHASH_NOTE_DESCRIPTOR *)HASH_NOTE_POINT_ADDRESS(pHashTable, ulHash);

    if (!pFirstHashNote) {
        HASH_LOCK_OFF(pHashTable);
        return FALSE;
    }

    //
    // �����ڵ�,���ûص��ж������Ƿ���ȷ����
    //
    pCurrentHashNote = pFirstHashNote;

    do {
        bFind = CallBack(lpContext, pCurrentHashNote->lpData);

        if (bFind) {
            break;
        }

        pCurrentHashNote = (PHASH_NOTE_DESCRIPTOR)pCurrentHashNote->lListEntry.Flink;
    } while (pCurrentHashNote != pFirstHashNote);

    HASH_LOCK_OFF(pHashTable);

    if (dpData) {
        *dpData = pCurrentHashNote;
    }

    return bFind;
}

/*---------------------------------------------------------
��������:   HashSearchByNumber
��������:   ͨ���ύHashֵ�ķ�ʽ����Hash��ڵ�

�������:   pHashTable          Hash��������

            ulNumber            ���ڼ���Hash������

            CallBack            �ص�����,�����ж��Ƿ��ҵ�,
                                ��ϸ��Ϣ��μ�BasicAlgorithm.h

            lpContext           �ص�������������,����Ϊ��������
                                ���ص�����

            dpData              �����ҵ��ڵ��ַ�Ŀռ�

�������:   dpData              �ҵ��Ľڵ��ַ

����ֵ:     TRUEΪ�ҵ� FALSEʧ��
����:
����ά��:   2011.4.4     ����汾
---------------------------------------------------------*/
BOOLEAN
HashSearchByNumber(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in ULONG ulNumber,
    __in HASH_IS_NOTE_MACHED_CALLBACK CallBack,
    __in PVOID lpContext,
    __inout PHASH_NOTE_DESCRIPTOR * dpData
    )
{
    return HashSearchByHash(
                pHashTable,
                ulNumber%(pHashTable->ulMaximumPointNumber),
                CallBack,
                lpContext,
                dpData);
}

/*---------------------------------------------------------
��������:   HashSearchByString
��������:   ͨ���ύHashֵ�ķ�ʽ����Hash��ڵ�

�������:   pHashTable          Hash��������

            pusString           ���ڼ���Hash��Unicode�ַ���

            CallBack            �ص�����,�����ж��Ƿ��ҵ�,
                                ��ϸ��Ϣ��μ�BasicAlgorithm.h

            lpContext           �ص�������������,����Ϊ��������
                                ���ص�����

            dpData              �����ҵ��ڵ��ַ�Ŀռ�

�������:   dpData              �ҵ��Ľڵ��ַ

����ֵ:     TRUEΪ�ҵ� FALSEʧ��
����:
����ά��:   2011.4.4     ����汾
---------------------------------------------------------*/
BOOLEAN
HashSearchByString(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in PUNICODE_STRING pusString,
    __in HASH_IS_NOTE_MACHED_CALLBACK CallBack,
    __in PVOID lpContext,
    __inout PHASH_NOTE_DESCRIPTOR * dpData
    )
{
    return HashSearchByHash(
                pHashTable,
                ELFhashUnicode(
                    pusString,
                    pHashTable->ulMaximumPointNumber),
                CallBack,
                lpContext,
                dpData);
}

/*---------------------------------------------------------
��������:   HashDelete
��������:   ͨ���ύHashֵ�ķ�ʽ����Hash��ڵ�

�������:   pHashTable  Hash��������
            pHashNote   Hash�ڵ�������
            CallBack    �ص�,����Ҫ������NULL
            bLock       �Ƿ����
�������:

����ֵ:     TRUEɾ���ɹ� ����ΪFALSE
����:       �ص��в���Ҫ�ͷ��������ݱ���,ֻ��Ҫ�ͷ����¼��ռ�
����ά��:   2011.4.4     ����汾
---------------------------------------------------------*/
BOOLEAN
HashDelete(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in PHASH_NOTE_DESCRIPTOR pHashNote,
    __in_opt HASH_DELETE_CALLBACK CallBack,
    __in BOOLEAN bLock
    )
{
    ASSERT(pHashNote);
    ASSERT(pHashTable);

    PHASH_NOTE_DESCRIPTOR * dpNote;
    if (bLock) {
        HASH_LOCK_ON(pHashTable);
    }

    //
    // �ȼ����Hash����ָ��note��ָ���ַ
    //
    dpNote = (PHASH_NOTE_DESCRIPTOR *)HASH_NOTE_POINT_ADDRESS(pHashTable, pHashNote->ulHash);

    if (IsListEmpty((PLIST_ENTRY)pHashNote)) {
        //
        // ���ֻ��һ���ڵ���,��ô���е�����ҲӦ������
        //
        *dpNote = NULL;
    } else {
        //
        // �����ֹһ���ڵ���ҽڵ��ǵ�һ��,��ôҪ��Hash���еĵ�ַ�޸�
        //
        if (*dpNote == pHashNote) {
            *dpNote = (PHASH_NOTE_DESCRIPTOR)pHashNote->lListEntry.Flink;
        }

        //
        // Ȼ��ȫ�Ƴ��ڵ�
        //
        RemoveEntryList((PLIST_ENTRY)pHashNote);
    }

    if (bLock) {
        HASH_LOCK_OFF(pHashTable);
    }

    //
    // �ͷ��ڴ� �ȵ��ûص�
    //
    if (CallBack) {
        CallBack(pHashNote->lpData);
    }

    ExFreePool(pHashNote->lpData);
    ExFreePool(pHashNote);

    return TRUE;
}

/*---------------------------------------------------------
��������:   HashFree
��������:   �ͷ�����Hash��
�������:   pHashTable  Hash��������
            CallBack    ɾ���ص�
�������:
����ֵ:
����:
����ά��:   2011.4.4     ����汾
---------------------------------------------------------*/
VOID
HashFree(__in PHASH_TABLE_DESCRIPTOR pHashTable,
         __in_opt HASH_DELETE_CALLBACK CallBack)
{
    ASSERT(pHashTable);

    // ��һ��Hash�ڵ��ַ�ĵ�ַ
    PHASH_NOTE_DESCRIPTOR * dpCurrentFirstHashNote;

    HASH_LOCK_ON(pHashTable);

    //
    // �������п��ܵ�Hash���е�ַ
    //
    for (ULONG ulCurrentHash = 0;
         ulCurrentHash < pHashTable->ulMaximumPointNumber;
         ulCurrentHash ++) {
        //
        // һֱɾ����һ���ڵ�,ֱ�������������
        //
        dpCurrentFirstHashNote =
                (PHASH_NOTE_DESCRIPTOR *)HASH_NOTE_POINT_ADDRESS(pHashTable, ulCurrentHash);

        while (*dpCurrentFirstHashNote != NULL) {
            DebugTraceEx(DEBUG_TRACE_NORMAL_INFO, "HashFree", "Release table note 0x%X", *dpCurrentFirstHashNote);

            HashDelete(
                pHashTable,
                *dpCurrentFirstHashNote,
                CallBack,
                FALSE       // ���������Ѿ��ӹ�����, Deleteʱ���ؼ���
                );
        }
    }

    HASH_LOCK_OFF(pHashTable);

    //
    // ���нڵ��Ѿ�ɾ�� �����ͷű��ڴ���������ڴ�
    //
    ExFreePool((PVOID)pHashTable->ulHashTableBaseAddress);
    ExFreePool((PVOID)pHashTable);
}

///////////////////Debug///////////////////////////
/*---------------------------------------------------------
��������:   DbgCheckEntireHashTable
��������:   �������Hash��
�������:   pHashTable  Hash��������
�������:
����ֵ:
����:       ʹ��ʱ���ú�DebugCheckEntireHashTable
����ά��:   2012.12.22     ����汾
---------------------------------------------------------*/

#if 0
VOID DbgCheckEntireHashTable(__in PHASH_TABLE_DESCRIPTOR pHashTable)
{
    KdPrint(("[Antinvader] DbgCheckEntireHashTable entered.\n"));

    PHASH_NOTE_DESCRIPTOR pCurrentHashNote;
    PHASH_NOTE_DESCRIPTOR pHeadNote;

    ASSERT(pHashTable);

    KdPrint(("[Antinvader] pHashTable passed.\n\
            \t\tDiscriptor address: 0x%X\n\
            \t\tBase address: 0x%X\n\
            \t\tMaximum Point Number: 0x%X\n\
            \t\tMutex address: 0x%x\n",
        pHashTablepHashTable,
        pHashTablepHashTable->ulHashTableBaseAddress,
        pHashTablepHashTable->ulMaximumPointNumber,
        pHashTablepHashTable->irmMutex));

    //
    // ֱ�ӷ��ص�ַ
    // �漰��������� ʹ��������
    //
    HASH_LOCK_ON(pHashTable);

    //
    // ����������
    //
    for (ULONG ulHash = 0; ulHash < pHashTable->ulMaximumPointNumber; ulHash++) {

        pCurrentHashNote =
            *(PHASH_NOTE_DESCRIPTOR *)HASH_NOTE_POINT_ADDRESS(pHashTable, ulHash);

        if (pCurrentHashNote) {
            //
            // ������Ч�ڵ�
            //
            KdPrint(("[Antinvader] Node %d found, head address: 0x%X\n", ulHash, pCurrentHashNote));

            if (IsListEmpty((PLIST_ENTRY)pHeadNote)) {
                KdPrint(("[Antinvader] empty:0x%X\n", ulHash, pCurrentHashNote));
            } else {
                //
                // �����ֹһ���ڵ�,��ô���Ƴ�����
                //
                RemoveEntryList((PLIST_ENTRY)pHashNote);
            }
            pHeadNote = pCurrentHashNote;

            while (pCurrentHashNote->lListEntry.Flink);
        }
    }

    KdPrint(("[Antinvader] First node address: 0x%X\n", pFirstHashNote));

    if (!pFirstHashNote) {
        KdPrint(("[Antinvader] First node address: 0x%X\n",pFirstHashNote));
        HASH_LOCK_OFF(pHashTable);
        return FALSE;
    }

    //
    // �����ڵ�,���ûص��ж������Ƿ���ȷ����
    //
    pCurrentHashNote = pFirstHashNote;

    do {
        bFind = CallBack(lpContext, pCurrentHashNote->lpData);

        if (bFind) {
            break;
        }

        pCurrentHashNote = (PHASH_NOTE_DESCRIPTOR)pCurrentHashNote->lListEntry .Flink;
    } while (pCurrentHashNote != pFirstHashNote);

    HASH_LOCK_OFF(pHashTable);

    if (dpData) {
        *dpData = pCurrentHashNote;
    }
}

#endif
