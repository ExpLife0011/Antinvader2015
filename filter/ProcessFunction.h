///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2011 - 2012
//
// ԭʼ�ļ�����     : ProcessFunction.h
// ��������         : AntinvaderDriver
// ����ʱ��         : 2011-03-20
//
//
// ����             : ���ڽ�����Ϣ�Ĺ�������
//
// ����ά��:
//  0000 [2011-03-20] ����汾.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

//#include <ntdef.h>
#include <ntifs.h>

//////////////////////
//  ��������
//////////////////////

// EPROCESS������СȡΪ12K
#define MAX_EPROCESS_SIZE   3 * 4 * 1024

// �ڴ��־
#define MEM_PROCESS_FUNCTION_TAG        'pfun'

//////////////////////
//  ƫ��������
//////////////////////

//
// �˴�����PEB(Process Environment Block)
// ��ַ���λ�������EPROCESS��ʼλ��ƫ��
// ��Windbg�����õ�
// nt!_EPROCESS
// ...
//   +0x1b0 Peb              : Ptr32 _PEB
// ...
// ����ֱ��Ӳ���뽫PEBƫ��������Ϊ0x1b0
//
#define PEB_STRUCTURE_OFFSET    0x1b0

//
// �˴�����PEB�ṹ��Parameter��Աƫ��
// ��Windbg������
// nt!_PEB
// ...
//   +0x010 ProcessParameters : Ptr32 _RTL_USER_PROCESS_PARAMETERS
// ...
// ����ֱ��Ӳ���뽫Parameter��Աƫ������Ϊ0x010
//
#define PARAMETERS_STRUCTURE_OFFSET     0x010

//
// �˴�������̾�������NT·�������ƫ��
// ��Windbg������
// nt!_RTL_USER_PROCESS_PARAMETERS
// ...
//   +0x038 ImagePathName    : _UNICODE_STRING
// ...
// ����ֱ��Ӳ���뽫ImagePathNameƫ������Ϊ0x038
//
#define IMAGE_PATH_STRUCTURE_OFFSET     0x038
//
// �˴�������̾�������SectionObject���ƫ��
// ��Windbg������
// nt!_EPROCESS
// ...
//   +0x138 SectionObject    : Ptr32 Void
// ...
// ����ֱ��Ӳ���뽫SectionObjectƫ������Ϊ0x038
//
#define IMAGE_SECTION_OBJECT_STRUCTURE_OFFSET   0x038

//
// �˴�������̾�������Segment���ƫ��
// ��Windbg������
// nt!_SECTION_OBJECT
// ...
//   +0x014 Segment          : Ptr32 _SEGMENT
// ...
// ����ֱ��Ӳ���뽫Segmentƫ������Ϊ0x014
//
#define IMAGE_SEGMENT_STRUCTURE_OFFSET          0x014

//
// �˴�������̾�������ControlArea���ƫ��
// ��Windbg������
// nt!_SEGMENT
// ...
//   +0x000 ControlArea      : Ptr32 _CONTROL_AREA
// ...
// ����ֱ��Ӳ���뽫ControlAreaƫ������Ϊ0x000
//
#define IMAGE_CONTROL_AREA_STRUCTURE_OFFSET     0x000

//
// �˴�������̾�������FilePointer���ƫ��
// ��Windbg������
// nt!_CONTROL_AREA
// ...
//   +0x024 FilePointer      : Ptr32 _FILE_OBJECT
// ...
// ����ֱ��Ӳ���뽫FilePointerƫ������Ϊ0x024
//
#define IMAGE_FILE_POINTER_STRUCTURE_OFFSET     +0x024

/////////////////////
//  ��������
/////////////////////

// ����������Ƶ�ַ���� EPROCESS ��ƫ����
static size_t stGlobalProcessNameOffset = 0;

// ���� System ���̵� EPROCESS ��ַ, �����ж�
static PEPROCESS peGlobalProcessSystem = NULL;

/////////////////////
//  �궨��
/////////////////////

// �ж��Ƿ�ƥ��ʱ�ķ���ֵ
#define PROCESS_NAME_NOT_CONFIDENTIAL   0x00000001
#define PROCESS_PATH_NOT_CONFIDENTIAL   0x00000002
#define PROCESS_MD5_NOT_CONFIDENTIAL    0x00000004

/////////////////////
//  ��������
/////////////////////

void InitProcessNameOffset();

ULONG GetCurrentProcessName(PUNICODE_STRING usCurrentProcessName);

BOOLEAN IsCurrentProcessConfidential();
