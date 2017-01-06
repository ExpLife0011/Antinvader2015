///////////////////////////////////////////////////////////////////////////////
///
/// ��Ȩ���� (c) 2011 - 2012
///
/// ԭʼ�ļ�����     : SystemHook.h
/// ��������         : AntinvaderDriver
/// ����ʱ��         : 2011-04-2
///
///
/// ����             : HOOK�ҹ����ܵ�ͷ�ļ�
///
/// ����ά��:
///  0000 [2011-04-2]  ����汾.
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ntdef.h>
#include <ntifs.h>

////////////////////////////
//      ���ݽṹ
////////////////////////////

// SSDT
typedef struct _SERVICE_DESCRIPTOR_ENTRY {
        unsigned int *  ServiceTableBase;
        unsigned int *  ServiceCounterTableBase;
        unsigned int    NumberOfServices;
        unsigned char * ParamTableBase;
} SERVICE_DESCRIPTOR_ENTRY, * PSERVICE_DESCRIPTOR_ENTRY;

// ZwCreateProcess����ָ��
typedef NTSTATUS (*ZW_CREATE_PROCESS) (
    __out PHANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in HANDLE InheritFromProcessHandle,
    __in BOOLEAN InheritHandles,
    __in_opt HANDLE SectionHandle ,
    __in_opt HANDLE DebugPort ,
    __in_opt HANDLE ExceptionPort
    );

////////////////////////////
//      ���ݵ���
////////////////////////////
#if 0
// ԭʼ��ZwCreateProcess
extern "C"
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateProcess(
    __out PHANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in HANDLE InheritFromProcessHandle,
    __in BOOLEAN InheritHandles,
    __in_opt HANDLE SectionHandle,
    __in_opt HANDLE DebugPort,
    __in_opt HANDLE ExceptionPort
    );
#endif

#if defined(_WIN64)

// 64λϵͳ�£�ֻ���Լ����� SSDT ��
#ifdef __cplusplus
extern "C"
#else
extern
#endif
SERVICE_DESCRIPTOR_ENTRY    KeServiceDescriptorTable;

#else

// ���� SSDT ��
#ifdef __cplusplus
extern "C"
#else
extern
#endif
__declspec(dllimport) SERVICE_DESCRIPTOR_ENTRY KeServiceDescriptorTable;

#endif

////////////////////////////
//      �궨��
////////////////////////////

// SSDT��ַ
#define SSDT_ADDRESS_OF_FUNCTION(_function) \
            KeServiceDescriptorTable.ServiceTableBase[*(PULONG)((PUCHAR)_function + 1)]

// һ����ļ�·������ ���ڲ²�򿪽��̵�·������,��������Ľ�Ϊ׼ȷ
// �������Ч��,���õ�Խ��ʱ�临�Ӷ�ԽС,�ռ临�Ӷ�Խ��
#define HOOK_NORMAL_PROCESS_PATH    NORMAL_PATH_LENGTH

// �ڴ��־
#define MEM_HOOK_TAG    'hook'

////////////////////////////
//      ��������
////////////////////////////

// ����ԭʼ��ZwCreateOprocess��ַ
extern ZW_CREATE_PROCESS    ZwCreateProcessOriginal;

////////////////////////////
//      ��������
////////////////////////////

// ׼���ҹ�����ĺ���
#ifdef __cplusplus
extern "C"
#else
extern
#endif
NTSTATUS
AntinvaderNewCreateProcess(
    __out PHANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in HANDLE InheritFromProcessHandle,
    __in BOOLEAN InheritHandles,
    __in_opt HANDLE SectionHandle ,
    __in_opt HANDLE DebugPort ,
    __in_opt HANDLE ExceptionPort
    );

VOID WriteProtectionOn();

VOID WriteProtectionOff();

BOOLEAN InitKeServiceDescirptorTable();
