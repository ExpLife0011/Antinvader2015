///////////////////////////////////////////////////////////////////////////////
///
/// ��Ȩ���� (c) 2011 - 2012
///
/// ԭʼ�ļ�����     : Common.h
/// ��������         : AntinvaderDriver
/// ����ʱ��         : 2011-03-20
///
///
/// ����             : ����Ring0��Ring3֮���ͨ��Լ��
///                    ����һЩ����
///
/// ����ά��:
///  0000 [2011-03-20] ����汾.
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

//#include <ntdef.h>

/////////////////////////////////////////
//      ͨ��Լ��
/////////////////////////////////////////

// �ܵ�����
#define COMMUNICATE_PORT_NAME   "\\AntinvaderPort"

// ����Ring3��Ring0ͨ�ŵ�����
typedef enum _ANTINVADER_COMMAND {
    ENUM_UNDEFINED = 0,
    ENUM_COMMAND_PASS,
    ENUM_BLOCK,
    ENUM_ADD_PROCESS,
    ENUM_DELETE_PROCESS,

    ENUM_OPERATION_SUCCESSFUL,
    ENUM_OPERATION_FAILED
} ANTINVADER_COMMAND;

//
// ����Ring3��Ring0ͨ�ŵ�����ṹ
//
// ���ݽṹΪ
//
// ��Ϣͷ   lSize       ������Ϣ��С
//          acCommand   ����
// ��Ϣ���ݽ����ں�������� ����û��д��
//          �ڲ��ʱ�����д���
//
// ���� ENUM_COMMAND_PASS �޺�׺����
//      ENUM_BLOCK        �޺�׺����
//
//      ENUM_ADD_PROCESS  ��׺Ϊ��������
//          �����ַ���,��\0��β
//      �������� ����·�� �ļ�MD5
//
//      ENUM_DELETE_PROCESS��׺Ϊ��������
//          �����ַ���,��\0��β
//      �������� ����·�� �ļ�MD5
//
//      ENUM_OPERATION_SUCCESSFUL �޺�׺����
//      ENUM_OPERATION_FAILED     �޺�׺����
//
typedef struct _ANTINVADER_MESSAGE {
    LONG lSize;
    ANTINVADER_COMMAND acCommand;
} COMMAND_MESSAGE, * PCOMMAND_MESSAGE;

////////////////////////////////////////
//      �궨��
////////////////////////////////////////

// һ����ļ�·������ ���ڲ²��·������,��������Ľ�Ϊ׼ȷ
// �������Ч��,���õ�Խ��ʱ�临�Ӷ�ԽС,�ռ临�Ӷ�Խ��
#define NORMAL_PATH_LENGTH      128

////////////////////////////////////////
//      ��������
////////////////////////////////////////
