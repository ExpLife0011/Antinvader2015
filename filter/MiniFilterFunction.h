///////////////////////////////////////////////////////////////////////////////
///
/// ��Ȩ���� (c) 2011 - 2012
///
/// ԭʼ�ļ�����     : MiniFilterFunction.h
/// ��������         : AntinvaderDriver
/// ����ʱ��         : 2011-04-2
///
///
/// ����             : ����΢����������ͨ�ú���ͷ�ļ�
///
/// ����ά��:
///  0000 [2011-04-2]  ����汾.
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ntdef.h>
#include <ntifs.h>
#include <fltKernel.h>

#include "CallbackRoutine.h"

////////////////////////
//      �궨��
////////////////////////

// �����µĻ������ڴ��־
#define MEM_TAG_READ_BUFFER                     'read'
#define MEM_TAG_WRITE_BUFFER                    'writ'
#define MEM_TAG_DIRECTORY_CONTROL_BUFFER        'dirc'

// �����»���Ļ�����;
typedef enum _ALLOCATE_BUFFER_TYPE
{
    Allocate_BufferRead = 1,        // ������
    Allocate_BufferWrite,           // д����
    Allocate_BufferDirectoryControl
} ALLOCATE_BUFFER_TYPE;

BOOLEAN AllocateAndSwapToNewMdlBuffer(
    __in PFLT_IO_PARAMETER_BLOCK pIoParameterBlock,
    __in PVOLUME_CONTEXT pvcVolumeContext,
    __inout PULONG pulNewBuffer,
    __inout_opt PMDL *dpMemoryDescribeList,
    __inout_opt PULONG pulOriginalBuffer,
    __inout_opt PULONG pulDataLength,
    __in ALLOCATE_BUFFER_TYPE ulFlag
    );

VOID FreeAllocatedMdlBuffer(
    __in ULONG ulBuffer,
    __in ALLOCATE_BUFFER_TYPE ulFlag
    );
