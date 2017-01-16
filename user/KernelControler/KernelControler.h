//
// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� KERNELCONTROLER_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// KERNELCONTROLER_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
//

#ifndef KERNELCONTROLER_INCLUDED
#define KERNELCONTROLER_INCLUDED

#pragma once

#ifdef KERNELCONTROLER_EXPORTS
#define KERNELCONTROLER_API     __declspec(dllexport)
#else
#define KERNELCONTROLER_API     __declspec(dllimport)
#endif

#include "../../filter/Common.h"

//
// �����Ǵ� KernelControler.dll ������
//
class KERNELCONTROLER_API CFilterDriverObject
{
public:
    CFilterDriverObject();
    ~CFilterDriverObject();

    BOOL Start();       // ��ʼ���� �������غ���Զ�����
    BOOL Stop();        // ֹͣ����

    BOOL AddConfidentialProcess(
        LPCWSTR wProcessName,   // ������
//        LPCWSTR wProcessPath,   // ·��
        LPCWSTR wProcessMD5     // md5
    );

    BOOL DeleteConfidentialProcess(
        LPCWSTR wProcessName,   // ������
//        LPCWSTR wProcessPath,   // ·��
        LPCWSTR wProcessMD5     // MD5
    );

private:
    BOOL PackProcessData(
        LPCWSTR wProcessName,           // ������
//        LPCWSTR wProcessPath,           // ·��
        LPCWSTR wProcessMD5,            // MD5
        ANTINVADER_COMMAND acCommond,   // ����
        PCOMMAND_MESSAGE * dpMessage,   // �����ð������ݵ�ַ
        DWORD * ddwMessageSize          // �������С
    );

    HANDLE hConnectionPort;
};

#endif // !KERNELCONTROLER_INCLUDED
