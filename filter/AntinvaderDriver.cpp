///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2011 - 2012
//
// ԭʼ�ļ�����     : AntinvaderDriver.cpp
// ��������         : AntinvaderDriver
// ����ʱ��         : 2011-03-20
//
//
// ����             : Antinvader��������������
//
// ����ά��:
//  0000 [2011-03-20] ����汾.
//
///////////////////////////////////////////////////////////////////////////////

// $Id$

#ifdef __cplusplus
extern "C" {
#endif

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <ntddscsi.h>

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

#ifdef __cplusplus
}   // extern "C"
#endif

#include "AntinvaderDriver.h"
#include "ProcessFunction.h"
#include "ConfidentialProcess.h"
#include "FileFunction.h"
#include "CallbackRoutine.h"
#include "ConfidentialFile.h"
#include "SystemHook.h"
#include "KeLog.h"

////////////////////////////
// ȫ�ֱ���
///////////////////////////
PDRIVER_OBJECT pdoGlobalDrvObj = NULL;      // ��������
PFLT_FILTER pfltGlobalFilterHandle = NULL;  // ���˾��

PFLT_PORT pfpGlobalServerPort;              // �������˿�, ��Ring3ͨ��
PFLT_PORT pfpGlobalClientPort;              // �ͻ��˶˿�, ��Ring3ͨ��

// COMMUNICATE_PORT_NAME     �ܵ�����: "\\AntinvaderPort"
PRESET_UNICODE_STRING(usCommunicatePortName, COMMUNICATE_PORT_NAME);

#ifdef __cplusplus
extern "C" {
#endif
/*---------------------------------------------------------
��������:   DriverEntry
��������:   �����������,ע��΢��������,ͨ�Ŷ˿�
�������:
            DriverObject ��������
            RegistryPath ����·��
�������:
            DriverObject ��������
����ֵ:
            STATUS_SUCCESSFUL Ϊ�ɹ����򷵻�ʧ��״ֵ̬
����:
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
NTSTATUS DriverEntry(
    __inout PDRIVER_OBJECT DriverObject,
    __in PUNICODE_STRING   RegistryPath
    )
{
    PAGED_CODE();

    // ��ʼ�� KeLog
    KeLog_Init();

    DebugTraceEx(DEBUG_TRACE_LOAD_UNLOAD, "DriverEntry", "[Antinvader] DriverEntry Enter.");

    // ����ֵ
    NTSTATUS status = STATUS_SUCCESS;

    // ��ʼ�����ݷ���ֵ
    BOOLEAN bReturn = 0;

    // ��ȫ��������
    PSECURITY_DESCRIPTOR psdSecurityDescriptor;

    // ����Ȩ�޽ṹ
    OBJECT_ATTRIBUTES   oaObjectAttributes;

    BOOLEAN success = InitKeServiceDescirptorTable();

    // ����ȫ����������
    pdoGlobalDrvObj = DriverObject;

    //
    // ע���������
    //
    if (NT_SUCCESS(status = FltRegisterFilter(DriverObject,               // ��������
                                              &FilterRegistration,        // ����ע����Ϣ
                                              &pfltGlobalFilterHandle     // �������, ���浽ȫ�ֱ���
                                              ))) {
        //
        // ����ɹ��� ��������
        //
        DebugTraceEx(DEBUG_TRACE_NORMAL_INFO, "DriverEntry", "[Antinvader] Register succeed!");

        if (!NT_SUCCESS(status = FltStartFiltering(pfltGlobalFilterHandle))) {
            //
            // �������ʧ�� ж������
            //
            DebugTraceEx(DEBUG_TRACE_ERROR, "DriverEntry", "[Antinvader] Starting filter failed.");

            FltUnregisterFilter(pfltGlobalFilterHandle);
            return status;
        }

    } else {
        //
        // �����ע�ᶼû�гɹ� ���ش�����
        //
        DebugTraceEx(DEBUG_TRACE_ERROR, "DriverEntry", "[Antinvader] Register failed.");
        return status;
    }

    //
    // ����ͨ�Ŷ˿�
    //

    //
    // ��ʼ����ȫ��������,Ȩ��ΪFLT_PORT_ALL_ACCESS
    //
    status  = FltBuildDefaultSecurityDescriptor(
                            &psdSecurityDescriptor,
                            FLT_PORT_ALL_ACCESS);

    if (!NT_SUCCESS(status)) {
        //
        // �����ʼ��ʧ�� ,ж������
        //
        DebugTraceEx(DEBUG_TRACE_ERROR, "DriverEntry", "[Antinvader] Built security descriptor failed.");

        FltUnregisterFilter(pfltGlobalFilterHandle);

        //
        // ������Ϣ
        //
        return status;
    }

    // ��ʼ������Ȩ�޽ṹ
    InitializeObjectAttributes(&oaObjectAttributes,                        // �ṹ
                               &usCommunicatePortName,                     // ��������
                               OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,   // �ں˾�� ��Сд������
                               NULL,
                               psdSecurityDescriptor);

    // ����ͨ�Ŷ˿�
    status = FltCreateCommunicationPort( pfltGlobalFilterHandle,    // �����������
                                         &pfpGlobalServerPort,      // ����˶˿�
                                         &oaObjectAttributes,       // Ȩ��
                                         NULL,
                                         Antinvader_Connect,
                                         Antinvader_Disconnect,
                                         Antinvader_Message,
                                         1                          // ���������
                                        );

    // �ͷŰ�ȫ��������
    FltFreeSecurityDescriptor(psdSecurityDescriptor);

    if (!NT_SUCCESS(status)) {
        //
        // ������յĲ���ʧ�� �ͷ��Ѿ��������Դ
        //
        DebugTraceEx(DEBUG_TRACE_ERROR, "DriverEntry", "[Antinvader] Creating communication port failed.");

        if (NULL != pfpGlobalServerPort) {
            FltCloseCommunicationPort(pfpGlobalServerPort);
        }

        if (NULL != pfltGlobalFilterHandle) {
            FltUnregisterFilter(pfltGlobalFilterHandle);
        }
    }

    //
    // ��ʼ��������ƫ��
    //
    InitProcessNameOffset();

//  FctInitializeHashTable();

    //
    // ��ʼ�����ܽ��̱�
    //
    PctInitializeHashTable();

    //
    // ��ʼ����������
    //
    ExInitializeNPagedLookasideList(
        &nliNewFileHeaderLookasideList,
        NULL,
        NULL,
        0,
        CONFIDENTIAL_FILE_HEAD_SIZE,
        MEM_FILE_TAG,
        0);

    /*
    ExInitializeNPagedLookasideList(
        &nliCallbackContextLookasideList,
        NULL,
        NULL,
        0,
        sizeof(_POST_CALLBACK_CONTEXT),
        MEM_CALLBACK_TAG,
        0);

    ExInitializeNPagedLookasideList(
        &nliFileStreamContextLookasideList,
        NULL,
        NULL,
        0,
        FILE_STREAM_CONTEXT_SIZE,
        MEM_TAG_FILE_TABLE,
        0);
    */

    //
    // ����
    //
    DebugTraceEx(DEBUG_TRACE_LOAD_UNLOAD, "DriverEntry", "[Antinvader] DriverEntry all succeed, leave now.");

    return status;
}

#ifdef __cplusplus
}   // extern "C"
#endif

/*---------------------------------------------------------
��������:   Antinvader_Unload
��������:   ж����������,�ر�ͨ�Ŷ˿�
�������:
            Flags   ж�ر�־
�������:
����ֵ:
            STATUS_SUCCESS �ɹ�
����:
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
NTSTATUS Antinvader_Unload(__in FLT_FILTER_UNLOAD_FLAGS Flags)
{
    UNREFERENCED_PARAMETER(Flags);

    PAGED_CODE();

    DebugTraceEx(DEBUG_TRACE_LOAD_UNLOAD, "Unload", "Entered.");

    //
    // �ر�ͨ�Ŷ˿�
    //
    if (pfpGlobalServerPort) {
        DebugTraceEx(DEBUG_TRACE_LOAD_UNLOAD, "Unload", "Closing communication port .... 0x%X", pfpGlobalServerPort);
        FltCloseCommunicationPort( pfpGlobalServerPort);
    }

    //
    // ж�ع���
    //
    if (pfltGlobalFilterHandle) {
        DebugTraceEx(DEBUG_TRACE_LOAD_UNLOAD, "Unload", "Unregistering filter .... 0x%X", pfltGlobalFilterHandle);
        FltUnregisterFilter( pfltGlobalFilterHandle);
    }

    //
    // �����ͷ����б��, ��Դ, ���������.
    //
    PctFreeTable();

//  ExDeleteNPagedLookasideList(&nliCallbackContextLookasideList);
//  ExDeleteNPagedLookasideList(&nliFileStreamContextLookasideList);

    ExDeleteNPagedLookasideList(&nliNewFileHeaderLookasideList);

    DebugTraceEx(DEBUG_TRACE_LOAD_UNLOAD, "Unload", "All succeed. Leave now.");

    //
    // ж�� KeLog
    //
    KeLog_Unload();

    return STATUS_SUCCESS;
}
