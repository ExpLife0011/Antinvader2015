///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2011 - 2012
//
// ԭʼ�ļ�����     : SystemHook.cpp
// ��������         : AntinvaderDriver
// ����ʱ��         : 2011-04-2
//
//
// ����             : HOOK�ҹ����ܵ�ʵ���ļ�
//
// ����ά��:
//  0000 [2011-04-2]  ����汾.
//
///////////////////////////////////////////////////////////////////////////////

#include "SystemHook.h"

#include <ntdef.h>
#include <intrin.h>   // For __readmsr
#include <fltKernel.h>

#pragma intrinsic(__readmsr)

#if defined(_WIN64)

// 64λϵͳ�£�ֻ���Լ����� SSDT ��
SERVICE_DESCRIPTOR_ENTRY    KeServiceDescriptorTable;

#endif

ZW_CREATE_PROCESS   ZwCreateProcessOriginal;

//////////////////////////////////////////////////////////////////////////
//
// See: http://czy0538.lofter.com/post/1ccbdcd5_4079090
//
// Modified by Guozi(wokss@163.com)
//
//////////////////////////////////////////////////////////////////////////
UINT64 GetKeServiceDescirptorTableShadow64()
{
    UINT64 KeServiceDescirptorTable = 0;                                    // ���� KeServiceDescirptorTable ��ַ
    PUCHAR startAddressSearch = (PUCHAR)__readmsr((ULONG)(0xC0000082));     // ��ȡ KiSystemCall64 ��ַ
    PUCHAR endAddressSearch = startAddressSearch + 0x500;                   // �����Ľ�����ַ
    ULONG tmpAddress = 0;                                                   // ���ڱ�����ʱ��ַ

    // �ȼ��ǰ�������ֽ��Ƿ�����Ч�ĵ�ַ�ռ�?
    if (!MmIsAddressValid(startAddressSearch) || !MmIsAddressValid(startAddressSearch + 1))
        return 0;

    // �� KiSystemCall64 ��ʼ�����亯�����ڹ��� KeServiceDescriptorTable �ṹ����Ϣ
    for (PUCHAR now = startAddressSearch; now < endAddressSearch; ++now) {
        // ֻ���� 3 ���ֽڼ��ɣ���Ϊǰ�������ֽ��Ѿ�������
        if (MmIsAddressValid(now + 2)) {
            // ������ 0x4c, 0x8d, 0x15
            if (now[0] == 0x4c && now[1] == 0x8d && now[2] == 0x15) {
                // ����� 4 ���ֽ�
                RtlCopyMemory(&tmpAddress, now + 3, 4);
                // �õ� KeServiceDescirptorTable �����ʵ��ַ
                KeServiceDescirptorTable = (UINT64)tmpAddress + (UINT64)now + 7;
            }
        }
        else {
            // ���� 3 ���ֽ�
            now += 3;
            while (now < endAddressSearch) {
                // ��ǰ����, �ҵ�����������Ч�ĵ�ַ�ռ����ʼλ��
                if (MmIsAddressValid(now) && MmIsAddressValid(now + 1) && MmIsAddressValid(now + 2)) {
                    // ��Ϊ������һ��ѭ��, now ���1, ���������ȼ�ȥ 1.
                    now--;
                    break;
                }
                // ͬ��, ���� 3 ���ֽ�
                now += 3;
            }
        }
    }
    return KeServiceDescirptorTable;
}

//
// See: http://czy0538.lofter.com/post/1ccbdcd5_4079090
// See: http://bbs.csdn.net/topics/391491205 (The another implement routine)
//
UINT64 GetKeServiceDescirptorTableShadow64_Original()
{
    UINT64 KeServiceDescirptorTable = 0;                                    // ���� KeServiceDescirptorTable ��ַ
    PUCHAR startAddressSearch = (PUCHAR)__readmsr((ULONG)(0xC0000082));     // ��ȡ KiSystemCall64 ��ַ
    PUCHAR endAddressSearch = startAddressSearch + 0x500;                   // �����Ľ�����ַ
    ULONG tmpAddress = 0;                                           // ���ڱ�����ʱ��ַ
    int j = 0;                                                      // ���ڽ�������

    // �� KiSystemCall64 ��ʼ�����亯�����ڹ��� KeServiceDescriptorTable �ṹ����Ϣ
    for (PUCHAR i = startAddressSearch; i < endAddressSearch; i++, j++) {
        if (MmIsAddressValid(i) && MmIsAddressValid(i + 1) && MmIsAddressValid(i + 2)) {
            // ������ 0x4c 0x8d 0x15
            if (startAddressSearch[j] == 0x4c &&
                startAddressSearch[j + 1] == 0x8d &&
                startAddressSearch[j + 2] == 0x15) {
                // �����4��������
                RtlCopyMemory(&tmpAddress, i + 3, 4);
                // �õ�KeServiceDescirptorTable����ʵ��ַ
                KeServiceDescirptorTable = (UINT64)tmpAddress + (UINT64)i + 7;
            }
        }
    }
    return KeServiceDescirptorTable;
}

BOOLEAN InitKeServiceDescirptorTable()
{
#if defined(_WIN64)
    SERVICE_DESCRIPTOR_ENTRY * lpKeServiceDescirptorTable = (SERVICE_DESCRIPTOR_ENTRY *)GetKeServiceDescirptorTableShadow64();
    if (lpKeServiceDescirptorTable != NULL) {
        RtlCopyMemory((void *)&KeServiceDescriptorTable, lpKeServiceDescirptorTable, sizeof(SERVICE_DESCRIPTOR_ENTRY));
        return TRUE;
    }
#endif
    return FALSE;
}

//
// See: http://czy0538.lofter.com/post/1ccbdcd5_4079090
//
UINT64 GetSSDTFunctionAddress64(INT32 index)
{
    INT64 address = 0;
    SERVICE_DESCRIPTOR_ENTRY * lpServiceDescriptorTable = (SERVICE_DESCRIPTOR_ENTRY *)GetKeServiceDescirptorTableShadow64();
    PULONG ssdt = (PULONG)lpServiceDescriptorTable->ServiceTableBase;
    ULONG dwOffset = ssdt[index];
    dwOffset >>= 4;                     // get real offset
    address = (UINT64)ssdt + dwOffset;  // get real address of function in ssdt
    KdPrint(("GetSSDTFunctionAddress(%d): 0x%llX\n", index, address));
    return address;
}

/*---------------------------------------------------------
��������:   WriteProtectionOn
��������:   ����д�������ж�
�������:
�������:
����ֵ:
����:
����ά��:   2011.4.3     ����汾
---------------------------------------------------------*/
inline
VOID WriteProtectionOn()
{
#if !defined(_WIN64)
    __asm {
        MOV     EAX, CR0
        OR      EAX, 10000H     // �ر�д����
        MOV     CR0, EAX
        STI                     // �����ж�
    }
#endif
}

/*---------------------------------------------------------
��������:   WriteProtectionOff
��������:   �ر�д�������ж�
�������:
�������:
����ֵ:
����:
����ά��:   2011.4.3     ����汾
---------------------------------------------------------*/
inline
VOID WriteProtectionOff()
{
#if !defined(_WIN64)
    __asm {
        CLI                         // �������ж�
        MOV     EAX, CR0
        AND     EAX, NOT 10000H     // ȡ��д����
        MOV     CR0, EAX
    }
#endif
}

#if !defined(_WIN64)
#ifdef __cplusplus
extern "C"
#endif
_declspec(naked)
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
    __in_opt HANDLE ExceptionPort)
{
   __asm {
       MOV      EAX, 1Fh
       LEA      EDX, [ESP + 04]
       INT      2Eh
       RET      20h
   }
}
#elif 1
#ifdef __cplusplus
extern "C"
#endif
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
    __in_opt HANDLE ExceptionPort)
{
    return 0;
}
#else
_declspec(naked)
NTSTATUS
NTAPI
ZwCreateProcess(
         PHANDLE phProcess,
         ACCESS_MASK DesiredAccess,
         POBJECT_ATTRIBUTES ObjectAttributes,
         HANDLE hParentProcess,
         BOOLEAN bInheritParentHandles,
         HANDLE hSection OPTIONAL,
         HANDLE hDebugPort OPTIONAL,
         HANDLE hExceptionPort OPTIONAL)
{
   __asm {
       MOV      EAX, 1Fh
       LEA      EDX, [ESP + 04]
       INT      2Eh
       RET      20h
   }
}
#endif

VOID HookInitializeFunctionAddress()
{
    // ����ԭʼ��ַ
    ZwCreateProcessOriginal = (ZW_CREATE_PROCESS)SSDT_ADDRESS_OF_FUNCTION(ZwCreateProcess);
}

VOID HookOnSSDT()
{
    // �ر�д���� �������ж�
    WriteProtectionOff();

    // �޸��µ�ַ
    SSDT_ADDRESS_OF_FUNCTION(ZwCreateProcess) = (unsigned int)(void *)&AntinvaderNewCreateProcess;

    // �ر�д���� �������ж�
    WriteProtectionOn();
}

VOID HookOffSSDT()
{
    // ԭʼ��ַһ��Ϊ��
    //FLT_ASSERT(ZwCreateProcessOriginal == NULL);

    // �ر�д���� �������ж�
    WriteProtectionOff();

    // �޸Ļ�ԭ���ĵ�ַ
    SSDT_ADDRESS_OF_FUNCTION(ZwCreateProcess) = (unsigned int)(void *)&ZwCreateProcessOriginal;

    // �ر�д���� �������ж�
    WriteProtectionOn();
}

/*---------------------------------------------------------
��������:   AntinvaderNewCreateProcess
��������:   �����滻ZwCreateProcess�Ĺҹ�����
�������:   ͬZwCreateProcess
�������:   ͬZwCreateProcess
����ֵ:     ͬZwCreateProcess
����:
����ά��:   2011.4.5     ����汾
---------------------------------------------------------*/

NTSTATUS
AntinvaderNewCreateProcess(
    __out PHANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in HANDLE InheritFromProcessHandle,
    __in BOOLEAN InheritHandles,
    __in_opt HANDLE SectionHandle,
    __in_opt HANDLE DebugPort,
    __in_opt HANDLE ExceptionPort
    )
{
/*
    // ����ֵ
    NTSTATUS status;

    // ��Ҫ�򿪽��̵�Ŀ¼���ļ�����
    PFILE_OBJECT pFileRootObject;

    // ���ɴ򿪽��̵��ļ�����
    PFILE_OBJECT pFileObject;

    // �����ļ�·���Ŀռ� �Ȳ�һ����С.
    WCHAR  wPathString[HOOK_NORMAL_PROCESS_PATH];

    // �����ļ�·���ռ�ĵ�ַ
    PWSTR pPathString = wPathString;

    // �ļ�·��
    PUNICODE_STRING usFilePath = {0};

    // ��������ļ�·�� ��ʱʹ��
    PUNICODE_STRING usFilePathNewAllocated = {0};

    // �ļ�·������
    ULONG ulPathLength;
    USHORT ustPathLength;

    //
    // �ȵ���ԭ���ZwCreateProcess��������
    //
    status = ZwCreateProcessOriginal(
            ProcessHandle,
            DesiredAccess,
            ObjectAttributes,
            InheritFromProcessHandle,
            InheritHandles,
            SectionHandle ,
            DebugPort ,
            ExceptionPort);

    //
    // �������ʧ�ܾͲ���Ҫ�ж���
    //
    if (!NT_SUCCESS(status)) {
        return status;
    }

    //
    // ���жϽ��������Ƿ�ƥ�� ��ƥ��ͷ���
    //
    if (IsProcessConfidential(ObjectAttributes->ObjectName, NULL, NULL)) {
        return STATUS_SUCCESS;
    }

    //
    // ��ʼ���ַ���
    //
    RtlInitEmptyUnicodeString(
        usFilePath,
        pPathString,
        HOOK_NORMAL_PROCESS_PATH);

    //
    // ��ȡ��������·�����ļ�����,ʧ�ܾ�ֱ�ӷ���
    //
    status = ObReferenceObjectByHandle(
        ObjectAttributes->RootDirectory,// ·�����
        GENERIC_READ,// ֻ��
        *IoFileObjectType,// �ļ�����
        KernelMode,// �ں�ģʽ
        (PVOID *)&pFileRootObject,// ��������ַ�Ŀռ�
        NULL// ��������ʹ��NULL
        );

    if (!NT_SUCCESS(status)) {
        return STATUS_SUCCESS;
    }

    //
    // ʹ���ļ������ȡ·��
    //
    ulPathLength = FctGetFilePath(
        pFileRootObject,
        usFilePath,
        CONFIDENTIAL_FILE_NAME_FILE_OBJECT);

    //
    // �жϲ²���ڴ��Ƿ��С,�����ѯʧ����ulPathLength = 0,����ִ���������
    //
    if (ulPathLength > HOOK_NORMAL_PROCESS_PATH) {
        //
        // �����ڴ�,���Ž�������Ҳ������
        //
        pPathString = (PWSTR)ExAllocatePoolWithTag(
                        NonPagedPool,
                        ulPathLength + ObjectAttributes->ObjectName->Length + 1,
                        MEM_HOOK_TAG
              );

        //
        // ���³�ʼ���ַ���
        //
        RtlInitEmptyUnicodeString(
                usFilePath,
                pPathString,
                (USHORT)(ulPathLength + ObjectAttributes->ObjectName->Length +1 )
                // ·������ + һ��б�ܳ��� + �ļ�������
                );

        //
        // ���²�ѯ
        //
        ulPathLength = FctGetFilePath(
                pFileRootObject,
                usFilePath,
                CONFIDENTIAL_FILE_NAME_FILE_OBJECT);
    }

    if (!ulPathLength) {
        //
        // �����ȡʧ��ʹ��QUERY_NAME_STRING��ѯ
        //
        ulPathLength = FctGetFilePath(
            pFileRootObject,
            usFilePath,
            CONFIDENTIAL_FILE_NAME_QUERY_NAME_STRING);

        if (ulPathLength > HOOK_NORMAL_PROCESS_PATH) {
            //
            // �����ڴ�,���Ž�������Ҳ������
            //
            pPathString = (PWSTR)ExAllocatePoolWithTag(
                            NonPagedPool,
                            ulPathLength + ObjectAttributes->ObjectName->Length + 1 ,
                            MEM_HOOK_TAG);

            //
            // ���³�ʼ���ַ���
            //
            RtlInitEmptyUnicodeString(
                    usFilePath,
                    pPathString,
                    (USHORT)(ulPathLength + ObjectAttributes->ObjectName->Length +1 )
                    // ·������ + һ��б�ܳ��� + �ļ�������
                    );

            //
            // ���²�ѯ
            //
            ulPathLength = FctGetFilePath(
                    pFileRootObject,
                    usFilePath,
                    CONFIDENTIAL_FILE_NAME_QUERY_NAME_STRING);
        }

        if (!ulPathLength) {
            //
            // ����ʧ��,û����,ֱ�ӷ���
            //
            return STATUS_SUCCESS;
        }
    }

    if (ulPathLength + 1 > usFilePath->MaximumLength) {
        //
        // �����ڴ�,���Ž�������Ҳ������
        //
        pPathString = (PWSTR)ExAllocatePoolWithTag(
                        NonPagedPool,
                        ulPathLength + ObjectAttributes->ObjectName->Length + 1,
                        MEM_HOOK_TAG);

        //
        // ���³�ʼ���ַ���
        //
        RtlInitEmptyUnicodeString(
                usFilePathNewAllocated,
                pPathString,
                (USHORT)(ulPathLength + ObjectAttributes->ObjectName->Length + 1)
                // ·������ + һ��б�ܳ��� + �ļ�������
                );

        //
        // �����ַ���
        //
        RtlCopyUnicodeString(usFilePathNewAllocated , usFilePath);

        usFilePath = usFilePathNewAllocated;
    }

    //
    // �ж�·�������Ƿ���"\",������Ǿͼ���
    //

    if (usFilePath->Buffer[ulPathLength - 1] != L'\\') {
        RtlAppendUnicodeToString(usFilePath , L"\\");
    }

    //
    // ������Ͻ������ƴ�С�ֲ�����
    //
    if (ulPathLength + 1 + ObjectAttributes->ObjectName->Length > usFilePath->MaximumLength) {
        //
        // �����ڴ�
        //
        pPathString = (PWSTR)ExAllocatePoolWithTag(
                        NonPagedPool,
                        ulPathLength + ObjectAttributes->ObjectName->Length + 1 ,
                        MEM_HOOK_TAG);

        //
        // ���³�ʼ���ַ���
        //
        RtlInitEmptyUnicodeString(
                usFilePathNewAllocated,
                pPathString,
                (USHORT)(ulPathLength + ObjectAttributes->ObjectName->Length +1 )
                // ·������ + һ��б�ܳ��� + �ļ�������
                );

        //
        // �����ַ���
        //
        RtlCopyUnicodeString( usFilePathNewAllocated, usFilePath);

        usFilePath = usFilePathNewAllocated;
    }

    //
    // ���ļ���������ȥ
    //
    RtlAppendUnicodeStringToString(usFilePath, ObjectAttributes->ObjectName);

    //
    // �����жϽ���·���Ƿ�ƥ�� ��ƥ��ͷ���
    //
    if (IsProcessConfidential(NULL ,usFilePath , NULL)) {
        return STATUS_SUCCESS;
    }
*/
    return STATUS_SUCCESS;
}
