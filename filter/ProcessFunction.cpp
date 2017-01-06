///////////////////////////////////////////////////////////////////////////////
///
/// ��Ȩ���� (c) 2011 - 2012
///
/// ԭʼ�ļ�����     : ProcessFunction.cpp
/// ��������         : AntinvaderDriver
/// ����ʱ��         : 2011-03-20
///
///
/// ����             : ���ڽ�����Ϣ�Ĺ���ʵ��
///
/// ����ά��:
///  0000 [2011-03-20] ����汾.
///
///////////////////////////////////////////////////////////////////////////////

#include "ProcessFunction.h"
#include "ConfidentialProcess.h"

#include <ntddk.h>

/*---------------------------------------------------------
��������:   InitProcessNameOffset
��������:   ��ʼ��EPROCESS�ṹ�н������ĵ�ַ
�������:
�������:
����ֵ:
����:       �ο��˳�����(̷��)��˼·
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
void InitProcessNameOffset()
{
    ULONG i;
    PEPROCESS peCurrentProcess;
    peGlobalProcessSystem = PsGetCurrentProcess();

    peCurrentProcess = peGlobalProcessSystem;

    //
    // ����EPROCESS�ṹ,�������ҵ�System�ַ�����ַ,
    // ��õ�ַΪEPROCESS�н������Ƶ�ַ
    //
    for (i = 0; i < MAX_EPROCESS_SIZE; i++) {
        if (!strncmp("System",
            (PCHAR)peCurrentProcess + i,
            strlen("System"))) {
            stGlobalProcessNameOffset = i;
            break;
        }
    }
}

/*---------------------------------------------------------
��������:   GetCurrentProcessName
��������:   ��ʼ��EPROCESS�ṹ�н������ĵ�ַ
�������:   usCurrentProcessName    ����������Ļ�����
�������:   usCurrentProcessName    ����Ľ�������ַ
����ֵ:     ����������
����:       �ο��˳�����(̷��)��˼·
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
ULONG GetCurrentProcessName(
    __in PUNICODE_STRING usCurrentProcessName
    )
{
    PEPROCESS peCurrentProcess;
    ULONG i, ulLenth;
    ANSI_STRING ansiCurrentProcessName;
    if(stGlobalProcessNameOffset == 0)
        return 0;

    //
    // ��õ�ǰ����EPROCESS,Ȼ���ƶ�һ��ƫ�Ƶõ�����������λ��
    //
    peCurrentProcess = PsGetCurrentProcess();

    //
    // ֱ�ӽ�����ַ����ansiCurrentProcessName����
    //
    RtlInitAnsiString(&ansiCurrentProcessName,
                        ((PCHAR)peCurrentProcess + stGlobalProcessNameOffset));

    //
    // ���������ansi�ַ���,����ת��Ϊunicode�ַ���
    //

    //
    // ��ȡ��Ҫ�ĳ���
    //
    ulLenth = RtlAnsiStringToUnicodeSize(&ansiCurrentProcessName);

    if(ulLenth > usCurrentProcessName->MaximumLength)  {
        //
        // ������Ȳ����򷵻���Ҫ�ĳ���
        //
        return ulLenth;
    }

    //
    // ת��ΪUnicode
    //
    RtlAnsiStringToUnicodeString(usCurrentProcessName,
                                    &ansiCurrentProcessName,FALSE);
    return ulLenth;
}

/*---------------------------------------------------------
��������:   IsProcessConfidential
��������:   �жϽ����Ƿ��ǻ��ܽ���
�������:
            usProcessName   ��������
            usProcessPath   ����·��
            usProcessMd5    ����MD5У��ֵ
�������:
����ֵ:     ��ȫƥ��Ϊ0,����Ϊ����ֵ�Ļ���

            PROCESS_NAME_NOT_CONFIDENTIAL   ���Ʋ�ƥ��
            PROCESS_PATH_NOT_CONFIDENTIAL   ·����ƥ��
            PROCESS_MD5_NOT_CONFIDENTIAL    MD5У�鲻ƥ��

����:       ����Ҫ������Ϣֱ����ΪNULL����

����ά��:   2011.4.5     ����汾  ������notepad.exe
            2011.7.25    ����Pctģ��
---------------------------------------------------------
ULONG IsProcessConfidential(
    PUNICODE_STRING usProcessName,
    PUNICODE_STRING usProcessPath,
    PUNICODE_STRING usProcessMd5
    )
{
    // ����ֵ ��¼��ƥ����Ϣ
    ULONG ulRet;

    // ��������
    CONFIDENTIAL_PROCESS_DATA cpdProcessData;

    if (!)

    UNICODE_STRING usProcessConfidential = { 0 };
    if ( usProcessName ) {
        RtlInitUnicodeString(&usProcessConfidential,L"notepad.exe");
        if(RtlCompareUnicodeString(usProcessName,&usProcessConfidential,TRUE) == 0)
            return 0;
        return PROCESS_NAME_NOT_CONFIDENTIAL;
    }

    return 0;
}*/
/*---------------------------------------------------------
��������:   IsCurrentProcessSystem
��������:   �жϵ�ǰ�����Ƿ��ǻ��ܽ���
�������:
�������:
����ֵ:     ��ǰ�����Ƿ���System
����:       ���Զ�WPS����ʱ������ʱ����System���̶߳Ի���
            �ļ����в���.������ǻ����ļ�����System����
            ͬ�����.
����ά��:   2011.7.27    ����汾
---------------------------------------------------------*/
inline BOOLEAN IsCurrentProcessSystem(){
    return peGlobalProcessSystem == PsGetCurrentProcess();
}

/*---------------------------------------------------------
��������:   IsCurrentProcessConfidential
��������:   �жϵ�ǰ�����Ƿ��ǻ��ܽ���
�������:
�������:
����ֵ:     ��ǰ�����Ƿ��ǻ��ܽ���
����:
����ά��:   2011.4.3     ����汾 ������notepad
            2011.7.25    ������ܽ��̱�ģ�� ������
---------------------------------------------------------*/

BOOLEAN IsCurrentProcessConfidential()
{
    WCHAR wProcessNameBuffer[32] = { 0 };
    CONFIDENTIAL_PROCESS_DATA cpdCurrentProcessData = { 0 };
    //UNICODE_STRING usProcessConfidential = { 0 };
    ULONG ulLength;

    RtlInitEmptyUnicodeString(
        &cpdCurrentProcessData.usName,
        wProcessNameBuffer,
        32 * sizeof(WCHAR)
        );

    ulLength = GetCurrentProcessName(&cpdCurrentProcessData.usName);
    // KdPrint(("[Antinvader]IsCurrentProcessConfidential Name:%ws\n", cpdCurrentProcessData.usName.Buffer));

    __try {
        return PctGetSpecifiedProcessDataAddress(&cpdCurrentProcessData, NULL);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        ASSERT(FALSE);
    }

    // RtlInitUnicodeString(&usProcessConfidential,L"notepad.exe");

    //if(RtlCompareUnicodeString(&usProcessName,&usProcessConfidential,TRUE) == 0)
    //  return TRUE;
    return FALSE;
}

/*---------------------------------------------------------
��������:   GetCurrentProcessPath
��������:   ��ȡ��ǰ����·����Ϣ

�������:   puniFilePath    ָ����Ч�ڴ���ַ���ָ��

�������:   puniFilePath    ������������·�����ַ���

����ֵ:     TRUE ����ɹ��ҵ�
            FALSE ʧ��

����:       ���س�ʼ������ʱ��Buffer��ַ

            ԭ�����ȥ���̵�PEB�а������ٳ���

            һ��ע����������������ڴ����ڴ��·��
            �ǵ��������ͷ�

����ά��:   2011.4.3     ����汾
---------------------------------------------------------*/
BOOLEAN GetCurrentProcessPath(__inout PUNICODE_STRING puniFilePath)
{
    // PEB�ṹ��ַ
    ULONG ulPeb;

    // Parameter�ṹ��ַ
    ULONG ulParameters;

    // ��ǰ����
    PEPROCESS  peCurrentProcess;

    // �ҵ��ĵ�ַ,�ȴ�����
    PUNICODE_STRING puniFilePathLocated;

    //
    // ��õ�ǰ����EPROCESS
    //
    peCurrentProcess = PsGetCurrentProcess();

    //
    // �Բ�ȷ�����ڴ���з���,��������,�ṹ���쳣��������
    //
    __try {
        //
        // ��ȡ��ǰ����PEB��ַ
        //
        ulPeb = *(ULONG*)((ULONG)peCurrentProcess + PEB_STRUCTURE_OFFSET);

        //
        // ��ָ��˵�����ں˽���,�϶�û��PEB�ṹ
        //
        if (!ulPeb) {
            return FALSE;
        }

        //
        // ����ַ�Ƿ���Ч,��Ч�϶�Ҳ����
        //
        if (!MmIsAddressValid((PVOID)ulPeb)) {
            return -1;
        }

        //
        // ����Parameter��ַ ���ڲ�����ָ���
        // ֱ���ǽ��ṹ�屾�����������,�ʲ���
        // Ҫ�ٴν��е�ַ��Ч�Լ��
        //
        ulParameters = *(PULONG)((ULONG)ulPeb+PARAMETERS_STRUCTURE_OFFSET);

        //
        // ����Path��ַ
        //
        puniFilePathLocated = (PUNICODE_STRING)(ulParameters+IMAGE_PATH_STRUCTURE_OFFSET);

        //
        // �����ڴ�
        //
        puniFilePath->Buffer = (PWCH)ExAllocatePoolWithTag(
            NonPagedPool,
            puniFilePathLocated->MaximumLength+2,
            MEM_PROCESS_FUNCTION_TAG
            );

        //
        // ��������
        //
        RtlCopyUnicodeString(puniFilePath, puniFilePathLocated);

        return TRUE;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrint(("[Antinvader]Severe error occured when getting current process path.\r\n"));
#if defined(DBG) && !defined(_WIN64)
        __asm int 3
#endif
    }

    return FALSE;
}
