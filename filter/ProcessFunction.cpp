///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2011 - 2012
//
// ԭʼ�ļ�����     : ProcessFunction.cpp
// ��������         : AntinvaderDriver
// ����ʱ��         : 2011-03-20
//
//
// ����             : ���ڽ�����Ϣ�Ĺ���ʵ��
//
// ����ά��:
//  0000 [2011-03-20] ����汾.
//
///////////////////////////////////////////////////////////////////////////////

#include "ProcessFunction.h"
#include "AntinvaderDriver.h"
#include "ConfidentialProcess.h"

#include <ntddk.h>

/*---------------------------------------------------------
��������:   InitProcessNameOffset
��������:   ��ʼ��EPROCESS�ṹ�н������ĵ�ַƫ��������DriverEntry������system����ʱ�ҵ�system��������ƫ������
			��DriverEntry�����̵���ʱ����EPROCESS+ƫ����ֱ�ӻ�ȡ������
�������:
�������:
����ֵ:
����:       �ο��˳�����(̷��)��˼·
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
BOOLEAN InitProcessNameOffset()
{
    PEPROCESS peCurrentProcess = PsGetCurrentProcess();

    //
    // ���� EPROCESS �ṹ,�������ҵ� System �ַ�����ַ,
    // ��õ�ַΪ EPROCESS �н������Ƶ�ַ.
    //
    for (ULONG i = 0; i < MAX_EPROCESS_SIZE; i++) {
        if (!strncmp("System", (PCHAR)peCurrentProcess + i, strlen("System"))) {
            s_stGlobalProcessNameOffset = i;
            return TRUE;
        }
    }

	return FALSE;
}

/*---------------------------------------------------------
��������:   GetCurrentProcessNameA
��������:   ��ʼ��EPROCESS�ṹ�н������ĵ�ַ
�������:   ansiCurrentProcessName    ����������Ļ�����
�������:   pSucceed    �Ƿ�ɹ�
����ֵ:     ����������
����:       �ο��˳�����(̷��)��˼·
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
ULONG FltGetCurrentProcessNameA(
    __in PANSI_STRING ansiCurrentProcessName,
    __out PBOOLEAN pSucceed
    )
{
    PEPROCESS peCurrentProcess;
    ULONG ulLenth;

    //
    // ȷ�� (IRQL <= APC_LEVEL), ���� Debug ģʽ�·�������.
    //
    PAGED_CODE();

    if (s_stGlobalProcessNameOffset == 0) {
        if (pSucceed)
            *pSucceed = FALSE;
        return 0;
    }

    //
    // ��õ�ǰ���� EPROCESS, Ȼ���ƶ�һ��ƫ�Ƶõ�����������λ��.
    //
    peCurrentProcess = PsGetCurrentProcess();
    if (peCurrentProcess == NULL) {
        if (pSucceed)
            *pSucceed = FALSE;
        return 0;
    }

    //
    // ֱ�ӽ�����ַ���� ansiCurrentProcessName ����.
    //
    RtlInitAnsiString(ansiCurrentProcessName,
                      ((PCHAR)peCurrentProcess + s_stGlobalProcessNameOffset));

    if (pSucceed)
        *pSucceed = TRUE;

    return ansiCurrentProcessName->Length;
}

/*---------------------------------------------------------
��������:   GetCurrentProcessName
��������:   ��ʼ��EPROCESS�ṹ�н������ĵ�ַ
�������:   usCurrentProcessName    ����������Ļ�����
�������:   pSucceed    �Ƿ�ɹ�
����ֵ:     ����������
����:       �ο��˳�����(̷��)��˼·
����ά��:   2011.3.20    ����汾
---------------------------------------------------------*/
ULONG FltGetCurrentProcessName(
    __in PUNICODE_STRING usCurrentProcessName,
    __out PBOOLEAN pSucceed
    )
{
	if (!pSucceed)
	{
		return 0;
	}

	ANSI_STRING ansiCurrentProcessName = { 0 };

	ULONG ulLenth = FltGetCurrentProcessNameA(&ansiCurrentProcessName, pSucceed);
	if (!(*pSucceed) || ulLenth <= 0)
	{
		return 0;
	}

    ulLenth = RtlAnsiStringToUnicodeSize(&ansiCurrentProcessName);
    if (ulLenth > usCurrentProcessName->MaximumLength) 
	{
        *pSucceed = FALSE;
        return ulLenth;
    }

    //
    // ת��ΪUnicode
    //
    RtlAnsiStringToUnicodeString(usCurrentProcessName, &ansiCurrentProcessName, FALSE);

    *pSucceed = TRUE;
    return ulLenth;
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
    WCHAR wProcessNameBuffer[NORMAL_NAME_LENGTH] = { 0 };
	WCHAR wProcessMD5Buffer[NORMAL_MD5_LENGTH] = { 0 };
    CONFIDENTIAL_PROCESS_DATA cpdCurrentProcessData = { 0 };    
    UNICODE_STRING usProcessName = { 0 };
    ULONG ulLength;
    BOOLEAN bSucceed = FALSE;

    RtlInitEmptyUnicodeString(
        &cpdCurrentProcessData.usName,
        wProcessNameBuffer,
		NORMAL_NAME_LENGTH * sizeof(WCHAR));
	RtlInitEmptyUnicodeString(
		&cpdCurrentProcessData.usMd5Digest,
		wProcessMD5Buffer,
		NORMAL_MD5_LENGTH * sizeof(WCHAR));

    ulLength = FltGetCurrentProcessName(&cpdCurrentProcessData.usName, &bSucceed);
    if (!bSucceed) {
        KdDebugPrint("[Antinvader] IsCurrentProcessConfidential(): call GetCurrentProcessName() failed."
            " ulLength = %u\n", ulLength);
        return FALSE;
    }
    KdDebugPrint("[Antinvader] IsCurrentProcessConfidential() ProcessName: %ws, ulLength = %u\n",
        cpdCurrentProcessData.usName.Buffer, ulLength);

#ifdef TEST_DRIVER_NOTEPAD
	// ����notepad
	UNICODE_STRING usProcessConfidential_notepad = { 0 };
	UNICODE_STRING usProcessConfidential_word = { 0 };
	UNICODE_STRING usProcessConfidential_excel = { 0 };
	UNICODE_STRING usProcessConfidential_ppt = { 0 };

    RtlInitUnicodeString(&usProcessConfidential_notepad, L"notepad.exe");
	RtlInitUnicodeString(&usProcessConfidential_word, L"winword.exe");
	RtlInitUnicodeString(&usProcessConfidential_excel, L"excel.exe");
	RtlInitUnicodeString(&usProcessConfidential_ppt, L"powerpnt.exe");

    RtlInitUnicodeString(&usProcessName, cpdCurrentProcessData.usName.Buffer);

	BOOLEAN ret = FALSE;
	if (TEST_driver_notepad_switch)
	{
		ret = ret || (RtlCompareUnicodeString(&usProcessName, &usProcessConfidential_notepad, TRUE) == 0);
	}
	if (TEST_driver_word_switch)
	{
		ret = ret || (RtlCompareUnicodeString(&usProcessName, &usProcessConfidential_word, TRUE) == 0);
	}
	if (TEST_driver_excel_switch)
	{
		ret = ret || (RtlCompareUnicodeString(&usProcessName, &usProcessConfidential_excel, TRUE) == 0);
	}
	if (TEST_driver_ppt_switch)
	{
		ret = ret || (RtlCompareUnicodeString(&usProcessName, &usProcessConfidential_ppt, TRUE) == 0);
	}
	return ret;
#else
	__try {
		// �ڿ��Ž���hash���в��ұ����̣�����ҵ����ǿ��Ž��̣���������md5��һ��)����ʵ��
		bSucceed = ComputeCurrentProcessMD5(&cpdCurrentProcessData.usName,&cpdCurrentProcessData.usMd5Digest);
		if (!bSucceed) {
			KdDebugPrint("[Antinvader] IsCurrentProcessConfidential(): call GetCurrentProcessMD5() failed.");
			return FALSE;
		}
		// TO BE CONTINUE
		return PctIsProcessDataInConfidentialHashTable(&cpdCurrentProcessData, NULL);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return FALSE;
	}
#endif
}

/*---------------------------------------------------------
��������:   GetCurrentProcessPath
��������:   ��ȡ��ǰ����·����Ϣ
�������:   puniFilePath    ָ����Ч�ڴ���ַ���ָ��
�������:   puniFilePath    ������������·�����ַ���
����ֵ:     TRUE, ����ɹ��ҵ�.
            FALSE, ʧ��.
����:       ���س�ʼ������ʱ��Buffer��ַ
            ԭ�����ȥ���̵�PEB�а������ٳ���
            һ��ע����������������ڴ����ڴ��·��
            �ǵ��������ͷ�

����ά��:   2011.4.3     ����汾
---------------------------------------------------------*/
//BOOLEAN GetCurrentProcessPath(__inout PUNICODE_STRING puniFilePath)
//{
//    // PEB�ṹ��ַ
//    ULONG ulPeb;
//
//    // Parameter�ṹ��ַ
//    ULONG ulParameters;
//
//    // ��ǰ����
//    PEPROCESS  peCurrentProcess;
//
//    // �ҵ��ĵ�ַ,�ȴ�����
//    PUNICODE_STRING puniFilePathLocated;
//
//    //
//    // ��õ�ǰ����EPROCESS
//    //
//    peCurrentProcess = PsGetCurrentProcess();
//	if (NULL == peCurrentProcess)
//	{
//		return FALSE;
//	}
//
//    //
//    // �Բ�ȷ�����ڴ���з���,��������,�ṹ���쳣��������
//    //
//    __try {
//        //
//        // ��ȡ��ǰ����PEB��ַ
//        //
//        ulPeb = *(ULONG*)((ULONG)peCurrentProcess + PEB_STRUCTURE_OFFSET);
//
//        //
//        // ��ָ��˵�����ں˽���, �϶�û��PEB�ṹ.
//        //
//        if (!ulPeb) {
//            return FALSE;
//        }
//
//        //
//        // ����ַ�Ƿ���Ч, ��Ч�϶�Ҳ����.
//        //
//        if (!MmIsAddressValid((PVOID)ulPeb)) {
//            return FALSE;
//        }
//
//        //
//        // ����Parameter��ַ, ���ڲ�����ָ���
//        // ֱ���ǽ��ṹ�屾�����������, �ʲ���
//        // Ҫ�ٴν��е�ַ��Ч�Լ��.
//        //
//        ulParameters = *(PULONG)((ULONG)ulPeb+PARAMETERS_STRUCTURE_OFFSET);
//
//        //
//        // ����Path��ַ
//        //
//        puniFilePathLocated = (PUNICODE_STRING)(ulParameters+IMAGE_PATH_STRUCTURE_OFFSET);
//
//        //
//        // �����ڴ�
//        //
//        puniFilePath->Buffer = (PWCH)ExAllocatePoolWithTag(
//            NonPagedPool,
//            puniFilePathLocated->MaximumLength + 2,
//            MEM_PROCESS_FUNCTION_TAG);
//
//        //
//        // ��������
//        //
//        RtlCopyUnicodeString(puniFilePath, puniFilePathLocated);
//
//        return TRUE;
//
//    } __except(EXCEPTION_EXECUTE_HANDLER) {
//        KdDebugPrint("[Antinvader] Severe error occured when getting current process path.\r\n");
//#if defined(DBG) && !defined(_WIN64)
//        __asm int 3
//#endif
//    }
//
//    return FALSE;
//}

/*---------------------------------------------------------
��������:   GetCurrentProcessMD5
��������:   ��ȡ��ǰ����MD5ֵ
�������:   ����������������·��
�������:   ����md5
����ֵ:     �Ƿ�ɹ�
����:
����ά��:   2017.1.11
---------------------------------------------------------*/
BOOLEAN ComputeCurrentProcessMD5(__in PUNICODE_STRING punistrCurrentProcessName,
	__inout PUNICODE_STRING punistrCurrentProcessMD5)
{
	// TO BE CONTINUE
	return FALSE;
}

