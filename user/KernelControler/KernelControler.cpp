//
// KernelControler.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "KernelControler.h"

CFilterDriverObject::CFilterDriverObject()
{
    //
    // ����ʱ�Զ�������������
    //
    DWORD dwResult = FilterConnectCommunicationPort(
        _T(COMMUNICATE_PORT_NAME),
        0,
        NULL,
        0,
        NULL,
        &hConnectionPort);

    if (dwResult != S_OK) {
        throw _T("Cannot establish driver connection.");
        hConnectionPort = NULL;
    }
}

CFilterDriverObject::~CFilterDriverObject()
{
    //
    // ����ʱ�ر�����
    //
    if (hConnectionPort) {
        CloseHandle(hConnectionPort);
    }

    // MessageBox(NULL, _T("Free successful"), _T("FREE"), MB_OK);
}

BOOL CFilterDriverObject::AddConfidentialProcess(
    LPCWSTR wProcessName,
    /*LPCWSTR wProcessPath,*/
    LPCWSTR wProcessMD5
    )
{
    DWORD dwBytesReturned;              // ���ص����ݴ�С
    DWORD dwPackSize;                   // ����С
    PCOMMAND_MESSAGE pCommondMessage;   // ���ݰ�

    // ��������
    PCOMMAND_MESSAGE pReplyMessage = (PCOMMAND_MESSAGE)malloc(sizeof(COMMAND_MESSAGE));
    if (pReplyMessage == NULL)
        return FALSE;

    if (!PackProcessData(wProcessName, /*wProcessPath,*/ wProcessMD5,
            ENUM_ADD_PROCESS, &pCommondMessage, &dwPackSize)) {
        return FALSE;
    }
    if (pCommondMessage == NULL)
        return FALSE;

    DWORD dwResult = FilterSendMessage(
        hConnectionPort,
        pCommondMessage,
        dwPackSize,
        pReplyMessage,
        sizeof(COMMAND_MESSAGE),
        &dwBytesReturned);

    UNREFERENCED_PARAMETER(dwResult);

    /*
    if (dwResult != S_OK ||
        dwBytesReturned != sizeof(COMMAND_MESSAGE) ||
        pReplyMessage->acCommand != ENUM_OPERATION_SUCCESSFUL) {
        throw _T("Add process failed.");
        return FALSE;
    }

    if (dwResult != S_OK) {
        throw _T("Add process failed indicated by return value.");
        return FALSE;
    }

    if (dwBytesReturned != sizeof(COMMAND_MESSAGE)) {
        throw _T("Add process failed indicated by bytes returned.");
        return FALSE;
    }

    if (pReplyMessage->acCommand != ENUM_OPERATION_SUCCESSFUL) {
        throw _T("Add process failed indicated reply message.");
        return FALSE;
    }
    */

    //
    // ��β����
    //
    if (pCommondMessage)
        free(pCommondMessage);

    if (pReplyMessage)
        free(pReplyMessage);

    return TRUE;
}

BOOL CFilterDriverObject::DeleteConfidentialProcess(
    LPCWSTR wProcessName,
    /*LPCWSTR wProcessPath,*/
    LPCWSTR wProcessMD5
    )
{
    DWORD dwBytesReturned;              // ���ص����ݴ�С
    DWORD dwPackSize;                   // ����С
    PCOMMAND_MESSAGE pCommondMessage;   // ���ݰ�

    // ��������
    PCOMMAND_MESSAGE pReplyMessage = (PCOMMAND_MESSAGE)malloc(sizeof(COMMAND_MESSAGE));
    if (pReplyMessage == NULL)
        return FALSE;

    if (!PackProcessData(wProcessName,/* wProcessPath,*/ wProcessMD5,
        ENUM_DELETE_PROCESS, &pCommondMessage, &dwPackSize)) {
        return FALSE;
    }
    if (pCommondMessage == NULL)
        return FALSE;

    DWORD dwResult = FilterSendMessage(
        hConnectionPort,
        pCommondMessage,
        dwPackSize,
        pReplyMessage,
        sizeof(COMMAND_MESSAGE),
        &dwBytesReturned);

    UNREFERENCED_PARAMETER(dwResult);

    /*
    if (dwResult != S_OK ||
        dwBytesReturned != sizeof(COMMAND_MESSAGE) ||
        pReplyMessage->acCommand != ENUM_OPERATION_SUCCESSFUL) {
        throw _T("Add process failed.");
        return FALSE;
    }

    if (dwResult != S_OK) {
        throw _T("Add process failed indicated by return value.");
        return FALSE;
    }

    if (dwBytesReturned != sizeof(COMMAND_MESSAGE)) {
        throw _T("Add process failed indicated by bytes returned.");
        return FALSE;
    }

    if (pReplyMessage->acCommand != ENUM_OPERATION_SUCCESSFUL) {
        throw _T("Add process failed indicated reply message.");
        return FALSE;
    }
    */

    //
    // ��β����
    //
    if (pCommondMessage)
        free(pCommondMessage);

    if (pReplyMessage)
        free(pReplyMessage);

    return TRUE;
}

// �����˼ǵ��ͷ��ڴ�
BOOL CFilterDriverObject::PackProcessData(
    LPCWSTR wProcessName,           // ������
    //LPCWSTR wProcessPath,           // ·��
    LPCWSTR wProcessMD5,            // MD5
    ANTINVADER_COMMAND acCommond,   // ����
    PCOMMAND_MESSAGE * ppMessage,   // �����ð������ݵ�ַ
    DWORD * pdwMessageSize          // �������С
    )
{
    // ������ַ, ���ڼ��㿽����������λ��
    LPVOID pAddress;

    //
    // �������ݽṹ��С
    //
    size_t stProcessName = (_tcslen(wProcessName) + 1) * sizeof(TCHAR);
    //size_t stProcessPath = (_tcslen(wProcessPath) + 1) * sizeof(TCHAR);
    size_t stProcessMD5 = (_tcslen(wProcessMD5) + 1) * sizeof(TCHAR);

    size_t stSize = stProcessName + /*stProcessPath + */stProcessMD5 + sizeof(COMMAND_MESSAGE);

    //
    // �����ڴ�
    //
    PCOMMAND_MESSAGE pCommondMessage = (PCOMMAND_MESSAGE)malloc(stSize);
    if (pCommondMessage == NULL) {
        throw _T("Insufficient memory");
        return FALSE;
    }

    //
    // �������
    //
    pCommondMessage->lSize = stSize;
    pCommondMessage->acCommand = acCommond;

    pAddress = (LPVOID)((char *)pCommondMessage + sizeof(COMMAND_MESSAGE));
    memcpy(pAddress, wProcessName, stProcessName);

    pAddress = (LPVOID)((char *)pAddress + stProcessName);
    //memcpy(pAddress, wProcessPath, stProcessPath);

    //pAddress = (LPVOID)((DWORD)pAddress + stProcessPath);
    memcpy(pAddress, wProcessMD5, stProcessMD5);

    if (ppMessage)
        *ppMessage = pCommondMessage;

    if (pdwMessageSize)
        *pdwMessageSize = stSize;

    return TRUE;
}
