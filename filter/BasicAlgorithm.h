///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2011 - 2012
//
// ԭʼ�ļ�����     : BasicAlgorithm.h
// ��������         : AntinvaderDriver
// ����ʱ��         : 2011-03-20
//
//
// ����             : һЩ�������㷨��������,��������Hash,���������
//
// ����ά��:
//  0000 [2011-03-20] ����汾.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ntdef.h>
#include <ntifs.h>

////////////////////////
//      �궨��
////////////////////////

// Hash���ڴ��־
#define MEM_HASH_TAG        'hash'

// һ��ָ��ĳ���
#define HASH_POINT_SIZE     sizeof(int *)

// Hash��Ӧ�ĵ�ַ
#define HASH_NOTE_POINT_ADDRESS(_table, _hash) \
            (_table->ulHashTableBaseAddress+_hash * HASH_POINT_SIZE)

// ������
#define HASH_LOCK_ON(_table)    KeWaitForSingleObject(&_table->irmMutex, Executive, KernelMode, FALSE, NULL)  // KeAcquireSpinLock(&_table->kslLock, &_table->irqlLockIRQL)
#define HASH_LOCK_OFF(_table)   KeReleaseMutex(&_table->irmMutex , FALSE)                                     // KeReleaseSpinLock(&_table->kslLock, _table->irqlLockIRQL)

////////////////////////
//  ��������
////////////////////////

// Hash������
typedef struct _HASH_TABLE_DESCRIPTOR
{
    ULONG ulHashTableBaseAddress;
    ULONG ulMaximumPointNumber;
//  KSPIN_LOCK kslLock;     // ������
//  KIRQL irqlLockIRQL;     // ȡ���ж�
    KMUTEX irmMutex;        // ������
} HASH_TABLE_DESCRIPTOR, * PHASH_TABLE_DESCRIPTOR;

// Hash�ڵ�����
typedef struct _HASH_NOTE_DESCRIPTOR
{
    LIST_ENTRY lListEntry;
    ULONG ulHash;
    PVOID lpData;
} HASH_NOTE_DESCRIPTOR, * PHASH_NOTE_DESCRIPTOR;

////////////////////////
//  ��������
////////////////////////

/*---------------------------------------------------------
��������:   HASH_IS_NOTE_MACHED_CALLBACK
��������:   �ص�����,�����ж��Ƿ��ҵ���ƥ���Hash��ڵ�

�������:   lpContext    ������,������(Search)������ָ��
            lpNoteData   ��ǰ�ڵ����������
�������:
����ֵ:     TRUEΪ��ȫƥ�� FALSEΪ��ƥ��

����:       �ú���Ҫ��ʹ������д,����ȷ��Hash��������
            ���Ľڵ��Ƿ���ȫƥ��,���ֱ�ӷ���TRUE������
            ��Hash��ͬ�ĵ�һ���ڵ�ᱻ����.

����ά��:   2011.4.4    ����汾
---------------------------------------------------------*/
typedef
BOOLEAN ( * HASH_IS_NOTE_MACHED_CALLBACK) (
    __in PVOID lpContext,
    __in PVOID lpNoteData
);
/*---------------------------------------------------------
��������:   HASH_DELETE_CALLBACK
��������:   �ص�����,�ͷ������������ڴ�

�������:   lpNoteData   ��ǰ�ڵ����������
�������:
����ֵ:

����:       �ú���Ҫ��ʹ������д,�ڽ����֮ǰ�ͷ��ڹ���
            �ڵ�����ʱ������ڴ��

����ά��:   2011.4.5    ����汾
---------------------------------------------------------*/
typedef
VOID ( * HASH_DELETE_CALLBACK) (
    __in PVOID lpNoteData
);

ULONG ELFhash(
    __in PANSI_STRING pansiKey,
    __in ULONG ulMod
);

BOOLEAN
HashInitialize(
    __in PHASH_TABLE_DESCRIPTOR * dpHashTable,
    __in ULONG ulMaximumPointNumber
);

BOOLEAN
HashInsertByHash(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in ULONG ulHash,
    __in PVOID lpData,
    __in ULONG ulLength
);

BOOLEAN
HashInsertByNumber(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in ULONG ulNumber,
    __in PVOID lpData
);

BOOLEAN
HashInsertByUnicodeString(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in PUNICODE_STRING pusString,
    __in PVOID lpData
);

BOOLEAN
HashSearchByHash(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in ULONG ulHash,
    __in HASH_IS_NOTE_MACHED_CALLBACK CallBack,
    __in PVOID lpContext,
    __inout PHASH_NOTE_DESCRIPTOR * dpData
);

BOOLEAN
HashSearchByNumber(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in ULONG ulNumber,
    __in HASH_IS_NOTE_MACHED_CALLBACK CallBack,
    __in PVOID lpContext,
    __inout PHASH_NOTE_DESCRIPTOR * dpData
);

BOOLEAN
HashSearchByString(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in PUNICODE_STRING pusString,
    __in HASH_IS_NOTE_MACHED_CALLBACK CallBack,
    __in PVOID lpContext,
    __inout PHASH_NOTE_DESCRIPTOR * dpData
);

BOOLEAN
HashDelete(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in PHASH_NOTE_DESCRIPTOR pHashNote,
    __in_opt HASH_DELETE_CALLBACK CallBack,
    __in BOOLEAN bLock
);

VOID
HashFree(
    __in PHASH_TABLE_DESCRIPTOR pHashTable,
    __in_opt HASH_DELETE_CALLBACK CallBack
);

ULONG ELFhashUnicode(PUNICODE_STRING pusKey, ULONG ulMod);

////////////////////////
//  Debug
////////////////////////
/*
#ifdef DBG

VOID DbgCheckEntireHashTable(
    __in PHASH_TABLE_DESCRIPTOR pHashTable
);

#define DebugCheckEntireHashTable(_x) DbgCheckEntireHashTable(_x)

#else

#define DebugCheckEntireHashTable(_x)

#endif
*/
