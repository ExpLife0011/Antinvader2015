//
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

// Windows ͷ�ļ�
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN     // �� Windows ͷ���ų�����ʹ�õ�����
#endif
#include <windows.h>
//#include <ntdef.h>
#include <tchar.h>
#include <stdio.h>

// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�

#pragma comment(lib, "fltLib.lib")
#pragma comment(lib, "user32.lib")

#if defined(_WIN64)
#pragma comment(lib, "KernelControler64.lib")
#else
#pragma comment(lib, "KernelControler32.lib")
#endif
