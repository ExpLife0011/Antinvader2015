///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2016 - 2017
//
// ԭʼ�ļ�����     : KeLog.h
// ��������         : AntinvaderDriver
// ����ʱ��         : 2017-01-09
//
//
// ����             : �ں������� Log �ļ����ģ��
//
// ����ά��:
//  0000 [2017-01-09] ����汾.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ntifs.h>
#include <fltKernel.h>

#ifndef KeLog_TracePrint
#define KeLog_TracePrint(_x_)     DbgPrint _x_
#endif

NTSTATUS
KeLog_Init();

NTSTATUS
KeLog_Unload();

void KeLog_GetCurrentTime(PTIME_FIELDS timeFileds);

void KeLog_GetCurrentTimeString(LPSTR time);

BOOLEAN
KeLog_Print(LPCSTR lpszLog, ...);

BOOLEAN
KeLog_FltPrint(PFLT_INSTANCE pfiInstance, LPCSTR lpszLog, ...);
