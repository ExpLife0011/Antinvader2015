///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2011 - 2012
//
// ԭʼ�ļ�����     : ConfidentialProcess.h
// ��������         : AntinvaderDriver
// ����ʱ��         : 2011-03-20
//
//
// ����             : ���ܽ�������ά��ͷ�ļ� ����:
//                    ���ܽ���Hash��
//                    ���ܽ����������
//
// ����ά��:
//  0000 [2011-03-23] ����汾.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "BasicAlgorithm.h"
#include "AntinvaderDef.h"

////////////////////////
//      ��������
////////////////////////

// ���ܽ���Hash���ܴ����ָ���� �ݶ�Ϊ256
#define CONFIDENTIAL_PROCESS_TABLE_POINT_NUMBER 256

// ָ���С
#define CONFIDENTIAL_PROCESS_POINT_SIZE sizeof(int *)

// ���ܽ���Hash��ʵ�ʴ�С�ݶ�Ϊ256��ָ���С ����32λ64λ������
#define CONFIDENTIAL_PROCESS_TABLE_SIZE CONFIDENTIAL_PROCESS_TABLE_POINT_NUMBER * \
                                            sizeof(int *)

// ����PctIsProcessDataAccordance��Flags�ͷ���ֵȡֵ ��Ҫ�Ƚ��û�����
#define CONFIDENTIAL_PROCESS_COMPARISON_MATCHED 0X00000000  // ȫ�����
#define CONFIDENTIAL_PROCESS_COMPARISON_NAME    0x00000001  // �Ƚ�����(���Ʋ�ͬ)
//#define CONFIDENTIAL_PROCESS_COMPARISON_PATH    0x00000002  // �Ƚ�·��(·����ͬ)
#define CONFIDENTIAL_PROCESS_COMPARISON_MD5     0x00000002  // �Ƚ�Md5ժҪ(Md5ժҪ��ͬ)
#define CONFIDENTIAL_PROCESS_COMPARISON_NO_MATCHED    0xFFFFFFFF  // ��ƥ��

////////////////////////
//      ��������
////////////////////////

// ���ܽ���Hash��
extern PHASH_TABLE_DESCRIPTOR phtProcessHashTableDescriptor;

// ���̼�¼��������
extern NPAGED_LOOKASIDE_LIST  nliProcessContextLookasideList;

#ifdef TEST_DRIVER_NOTEPAD
extern BOOLEAN		TEST_driver_notepad_switch;
extern BOOLEAN		TEST_driver_word_switch;
extern BOOLEAN		TEST_driver_excel_switch;
extern BOOLEAN		TEST_driver_ppt_switch;
#endif

////////////////////////
//      �궨��
////////////////////////

// �ڴ��־ �����ڴ�ʱʹ��
#define MEM_TAG_PROCESS_TABLE   'cptd'

/*
// ���Ե�ַһ��ΪHash��������, ��ʱ������
#define ASSERT_HASH_TABLE_ADDRESS(addr)     FLT_ASSERT((addr >= ulGlobalProcessDataTableAddress) && \
                                                       (addr < CONFIDENTIAL_PROCESS_TABLE_SIZE + \
                                                       ulGlobalProcessDataTableAddress))
*/

////////////////////////
//      �ṹ����
////////////////////////

// ���ܽ�����Ϣ�ṹ��

//TO BE CONTINUE
//ͨ���������ơ�·����MD5ֵ�ж��Ƿ��ǿ��Ž���Ӧ�޸�Ϊͨ�����ҽ������ơ�MD5ֵ�ķ�ʽ
//��ͬ�Ļ�����װ��ͬ�����䰲װ·�����ܲ�һ�£�·��û�����塣
//��ͬ���̿��ܴ��ڶ���汾����ͬ�汾��md5ֵ��ͬ��������һ��hash�ڵ�������У��ҵ�
//����һ����ͬ�Ľ�����+md5���ǿ��Ž���
typedef struct _CONFIDENTIAL_PROCESS_DATA
{
    UNICODE_STRING usName;                      // ��������
//    UNICODE_STRING usPath;                      // ����·��
    UNICODE_STRING usMd5Digest;                 // ����MD5У��ֵ
} CONFIDENTIAL_PROCESS_DATA, * PCONFIDENTIAL_PROCESS_DATA;

/////////////////////////
//      ��������
/////////////////////////

BOOLEAN  PctInitializeHashTable();

//��ȡ��������hashֵ
ULONG PctGetProcessNameHashValue(
    __in PCONFIDENTIAL_PROCESS_DATA ppdProcessData
);

BOOLEAN PctNewProcessDataHashNode(
    __in PCONFIDENTIAL_PROCESS_DATA ppdProcessData,
    __inout  PCONFIDENTIAL_PROCESS_DATA * dppdNewProcessData
);

BOOLEAN PctAddProcess(
    __in PCONFIDENTIAL_PROCESS_DATA ppdProcessData
);

ULONG  PctIsProcessDataAccordance(
    __in PCONFIDENTIAL_PROCESS_DATA ppdProcessDataOne,
    __in PCONFIDENTIAL_PROCESS_DATA ppdProcessDataAnother,
    __in ULONG ulFlags
);

BOOLEAN PctIsProcessDataInConfidentialHashTable(
    __in  PCONFIDENTIAL_PROCESS_DATA ppdProcessDataSource,
    __inout PCONFIDENTIAL_PROCESS_DATA * dppdProcessDataInTable
);
/*
VOID PctUpdateProcessMd5(
    __in  PCONFIDENTIAL_PROCESS_DATA ppdProcessDataInTable,
    __in  PCONFIDENTIAL_PROCESS_DATA ppdProcessDataSource
);
*/
VOID PctFreeProcessDataHashNode(
    __in  PCONFIDENTIAL_PROCESS_DATA ppdProcessData,
    __in  BOOLEAN bFreeDataBase
);

BOOLEAN PctFreeHashTable();

BOOLEAN PctDeleteProcessDataHashNode(__in  PCONFIDENTIAL_PROCESS_DATA ppdProcessData);

static
BOOLEAN PctIsDataMachedCallback(
    __in PVOID lpContext,
    __in PVOID lpNoteData
);

static VOID PctFreeHashMemoryCallback (__in PVOID lpNoteData);

#ifdef TEST_DRIVER_NOTEPAD
BOOLEAN PctAddDeleteProcess(
	__in PCONFIDENTIAL_PROCESS_DATA ppdProcessData,
	__in BOOLEAN isAddOrDelete
	);
#endif
