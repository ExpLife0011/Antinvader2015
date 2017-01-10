///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2011 - 2012
//
// ԭʼ�ļ�����     : AntinvaderDriver.h
// ��������         : AntinvaderDriver
// ����ʱ��         : 2011-03-20
//
//
// ����             : Antinvader����������Ҫͷ�ļ�,���������;�̬���Ͷ���
//
// ����ά��:
//  0000 [2011-03-20] ����汾.
//
///////////////////////////////////////////////////////////////////////////////

// $Id$

#ifndef __ANTINVADERDRIVER_DRIVER_H__
#define __ANTINVADERDRIVER_DRIVER_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "AntinvaderDef.h"

#if 0
///////////////////////////
//  ��������ͷ�ļ�����
//////////////////////////

// �����㷨
#include "BasicAlgorithm.h"

// ��������ص�
#include "CallbackRoutine.h"

// ������Ϣ����
#include "ProcessFunction.h"

// �����ļ�����
#include "ConfidentialFile.h"

// �����ļ�����
#include "FileFunction.h"

// ���ܽ�������
#include "ConfidentialProcess.h"

// ΢��������ͨ�ú���
#include "MiniFilterFunction.h"

// �ҹ�
#include "SystemHook.h"

#endif

// ��������ص�
#include "CallbackRoutine.h"

// �����ļ�����
#include "ConfidentialFile.h"

#include <fltKernel.h>

// ����ж��
NTSTATUS Antinvader_Unload (__in FLT_FILTER_UNLOAD_FLAGS Flags);

// ��������Ҫ���˵�IRP����
const FLT_OPERATION_REGISTRATION Callbacks[] = {
    // ����
    { IRP_MJ_CREATE, 0, Antinvader_PreCreate, Antinvader_PostCreate },
    // �ر�
    { IRP_MJ_CLOSE, 0, Antinvader_PreClose, Antinvader_PostClose },
    // ����
    { IRP_MJ_CLEANUP, 0, Antinvader_PreCleanUp, Antinvader_PostCleanUp },
    // ���ļ�
    { IRP_MJ_READ, 0, Antinvader_PreRead, Antinvader_PostRead },
    // д�ļ� ��������
    { IRP_MJ_WRITE, 0, Antinvader_PreWrite, Antinvader_PostWrite },
    // �����ļ���Ϣ,������������ͷ
    { IRP_MJ_SET_INFORMATION, 0, Antinvader_PreSetInformation, Antinvader_PostSetInformation },
    // Ŀ¼
    { IRP_MJ_DIRECTORY_CONTROL, 0, Antinvader_PreDirectoryControl, Antinvader_PostDirectoryControl },
    // ��ȡ�ļ���Ϣ
    { IRP_MJ_QUERY_INFORMATION, 0, Antinvader_PreQueryInformation, Antinvader_PostQueryInformation },
    // End of IRP_MJ
    { IRP_MJ_OPERATION_END }
};

CONST FLT_CONTEXT_REGISTRATION Contexts[] = {
    { FLT_VOLUME_CONTEXT, 0, Antinvader_CleanupContext, sizeof(VOLUME_CONTEXT), MEM_CALLBACK_TAG },
    { FLT_STREAM_CONTEXT, 0, Antinvader_CleanupContext, FILE_STREAM_CONTEXT_SIZE, MEM_TAG_FILE_TABLE },
    { FLT_CONTEXT_END }
};

// ����ע�����
const FLT_REGISTRATION FilterRegistration = {
    sizeof(FLT_REGISTRATION),           //  ��С
    FLT_REGISTRATION_VERSION,           //  �汾
    0,                                  //  ��־

    Contexts,                           //  ������
    Callbacks,                          //  �����ص����� ����FLT_OPERATION_REGISTRATION

    Antinvader_Unload,                  //  ж�ص���

    Antinvader_InstanceSetup,           //  ��������ʵ��
    NULL,   // Antinvader_InstanceQueryTeardown,    //  ���ٲ�ѯʵ��
    NULL,   // Antinvader_InstanceTeardownStart,    //  ��ʼʵ�����ٻص�
    NULL,   // Antinvader_InstanceTeardownComplete, //  ���ʵ�����ٻص�

    NULL,                               //  �����ļ����ƻص� (δʹ�� NULL)
    NULL,                               //  ����Ŀ���ļ����ƻص�(δʹ�� NULL)
    NULL                                //  ��׼�����������ƻ���(δʹ�� NULL)
};

////////////////////////////
// ȫ�ֱ���
///////////////////////////
extern PDRIVER_OBJECT pdoGlobalDrvObj;             // ��������
extern PFLT_FILTER pfltGlobalFilterHandle;         // ���˾��

extern PFLT_PORT pfpGlobalServerPort;              // �������˿�, ��Ring3ͨ��
extern PFLT_PORT pfpGlobalClientPort;              // �ͻ��˶˿�, ��Ring3ͨ��

#endif // __ANTINVADERDRIVER_DRIVER_H__
