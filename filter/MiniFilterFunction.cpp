///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2011 - 2012
//
// ԭʼ�ļ�����     : MiniFilterFunction.cpp
// ��������         : AntinvaderDriver
// ����ʱ��         : 2011-04-2
//
//
// ����             : ����΢��������ͨ�ú���
//
// ����ά��:
//  0000 [2011-04-2]  ����汾.
//
///////////////////////////////////////////////////////////////////////////////

#include "MiniFilterFunction.h"

////////////////////////////////////
//    ΢������������
////////////////////////////////////

/*---------------------------------------------------------
��������:   AllocateAndSwapToNewMdlBuffer
��������:   �����µĻ�����,������ԭ������
�������:
            pIoParameterBlock       I/O������,����IRP�����Ϣ
            pulNewBuffer            ����»���ָ��Ŀռ�
            dpMemoryDescribeList    ���Mdlָ��ռ�
            pulOriginalBuffer       ���ԭʼ�����ַ�Ŀռ�
            pulDataLength           ������ݳ��ȵĿռ�
            ulFlag                  �������;
�������:
            pulNewBuffer            �»����ָ��
            dpMemoryDescribeList    Mdlָ��
            pulDataLength           ���ݳ���
            pulOriginalBuffer       ԭ��ַָ��
����ֵ:
            TRUE    �ɹ�
            FALSE   ʧ��
����:
            ����ɹ��󽫻��ԭ�ȵ����ݿ������µĻ�����ȥ

            dpMemoryDescribeList
            pulOriginalBuffer
            pulDataLength
            ������ΪNULL

����ά��:   2011.4.2     ����汾
            2011.4.3     ��������ݳ��ȷ���
---------------------------------------------------------*/
BOOLEAN AllocateAndSwapToNewMdlBuffer(
    __in PFLT_IO_PARAMETER_BLOCK pIoParameterBlock,
    __in PVOLUME_CONTEXT pvcVolumeContext,
    __inout PVOID * ppvNewBuffer,
    __inout_opt PMDL * dpMemoryDescribeList,
    __inout_opt PVOID * ppvOriginalBuffer,
    __inout_opt PULONG pulDataLength,
    __in ALLOCATE_BUFFER_TYPE ulFlag
    )
{
    FLT_ASSERT(pIoParameterBlock != NULL);

    // ���ݳ���
    ULONG ulDataLength;

    // �����ݻ���
    PVOID pvBuffer = NULL;

    // �����ݻ���
    PVOID pvOriginalBuffer = NULL;

    // ��Mdl
    PMDL pMemoryDescribeList;

    // �������ݳ���
    switch (ulFlag) {
        //
        // ����Ƿǻ����д,��ô��д���ȱ������
        //
        case Allocate_BufferRead:
            ulDataLength = (pIoParameterBlock->IrpFlags & IRP_NOCACHE) ?
                (ULONG)ROUND_TO_SIZE(pIoParameterBlock->Parameters.Read.Length, pvcVolumeContext->ulSectorSize) :
                pIoParameterBlock->Parameters.Read.Length;
            break;

        case Allocate_BufferWrite:
            ulDataLength = (pIoParameterBlock->IrpFlags & IRP_NOCACHE) ?
                (ULONG)ROUND_TO_SIZE(pIoParameterBlock->Parameters.Write.Length, pvcVolumeContext->ulSectorSize) :
                pIoParameterBlock->Parameters.Write.Length;
            break;

        case Allocate_BufferDirectoryControl:
            ulDataLength = pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.Length;
            break;

        default:
            return FALSE;
    }

    //
    // ����Ҫ���н��ܲ���,��û��Ȩ���޸Ļ���������ʱ��Ҫ����
    // ׼��ӵ��Ȩ�޵Ļ���.���Ȩ��ΪIoReadAccess����Ϊ��Ҫ׼
    // ������.��������׼������
    //
    switch (ulFlag) {
        case Allocate_BufferRead:
            pvBuffer = ExAllocatePoolWithTag(
                NonPagedPool,
                (SIZE_T)ulDataLength,
                MEM_TAG_READ_BUFFER);
            break;

        case Allocate_BufferWrite:
            pvBuffer = ExAllocatePoolWithTag(
                NonPagedPool,
                (SIZE_T)ulDataLength,
                MEM_TAG_WRITE_BUFFER);

        case Allocate_BufferDirectoryControl:
            pvBuffer = ExAllocatePoolWithTag(
                NonPagedPool,
                (SIZE_T)ulDataLength,
                MEM_TAG_DIRECTORY_CONTROL_BUFFER);
            break;
    }


    if (!pvBuffer) {
        // �������ʧ���򷵻�,��������ĩβ
        return FALSE;
    }

    pMemoryDescribeList = IoAllocateMdl(
        (PVOID)pvBuffer,
        ulDataLength,
        FALSE,      // û��IRP����
        FALSE,      // �м�����
        NULL        // û��IRP
        );

    if (!pMemoryDescribeList) {
        ExFreePool(pvBuffer);
        return FALSE;
    }

    //
    //
    // ���΢������ʹ�÷Ƿ�ҳ���еĻ�������һ��û������
    // FLTFL_CALLBACK_DATA_SYSTEM_BUFFER�Ĳ���,��ô��Ҳ
    // ������MuBuildMdlForNonpagedPool()���ѵ�ַ��д��
    // MdlAddress����.������Ϊ��ôһ��,������κι�����
    // �����ļ�ϵͳ�������ٳ���ȥ�����Ƿ�ҳ��(�����ڹ���
    // ��ʱ��ʹ�ö���,���Ƕ�Ч�ʲ���),����ṩ��һ��MDL,
    // ���������ļ�ϵͳ���ǿ���ͨ��MDL���ʻ���(���Ի��
    // һ��ϵͳ�ڴ��ַ��������).
    //
    //
    MmBuildMdlForNonPagedPool(pMemoryDescribeList);

    //
    // �����Mdl��ʹ��Mdl��ȡ��ַ,����ֱ��ʹ��IRP,ͬʱ
    // �޸�pIoParameterBlock���µĻ���
    //
    switch (ulFlag) {
        case Allocate_BufferRead:
            if (pIoParameterBlock->Parameters.Read.MdlAddress) {
                pvOriginalBuffer = MmGetSystemAddressForMdlSafe(
                    pIoParameterBlock->Parameters.Read.MdlAddress,
                    NormalPagePriority);
            } else {
                pvOriginalBuffer = pIoParameterBlock->Parameters.Read.ReadBuffer;
            }

            pIoParameterBlock->Parameters.Read.ReadBuffer = pvBuffer;
            pIoParameterBlock->Parameters.Read.MdlAddress = pMemoryDescribeList;

            break;

        case Allocate_BufferWrite:
            if (pIoParameterBlock->Parameters.Write.MdlAddress) {
                pvOriginalBuffer = MmGetSystemAddressForMdlSafe(
                    pIoParameterBlock->Parameters.Write.MdlAddress,
                    NormalPagePriority);
            } else {
                pvOriginalBuffer = pIoParameterBlock->Parameters.Write.WriteBuffer;
            }

            pIoParameterBlock->Parameters.Write.WriteBuffer = pvBuffer;
            pIoParameterBlock->Parameters.Write.MdlAddress = pMemoryDescribeList;

            //
            // �����ڴ浽�µ�ַ
            //
            RtlCopyMemory(pvBuffer, pvOriginalBuffer, ulDataLength);
            break;

        case Allocate_BufferDirectoryControl:
            pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.DirectoryBuffer = pvBuffer;
            pIoParameterBlock->Parameters.DirectoryControl.QueryDirectory.MdlAddress = pMemoryDescribeList;
            pvOriginalBuffer = NULL;
            break;
    }

    //
    // ���淵�ز���
    //
    if (ppvNewBuffer) {
        *ppvNewBuffer = pvBuffer;
    }
    if (dpMemoryDescribeList) {
        *dpMemoryDescribeList = pMemoryDescribeList;
    }
    if (ppvOriginalBuffer) {
        *ppvOriginalBuffer = pvOriginalBuffer;
    }
    if (pulDataLength) {
        *pulDataLength = ulDataLength;
    }
    return TRUE;
}

/*---------------------------------------------------------
��������:   FreeAllocatedMdlBuffer
��������:   �ͷ�֮ǰ����Ļ�����
�������:
            ulNewBuffer             ����»���ָ��Ŀռ�
            ulFlag                  ���������
�������:
����ֵ:
����:
����ά��:   2011.4.2     ����汾
            2011.7.9     �������ޱ���ж��ͷ�
---------------------------------------------------------*/
VOID FreeAllocatedMdlBuffer(
    __in PVOID pvBuffer,
    __in ALLOCATE_BUFFER_TYPE ulFlag
    )
{
    switch (ulFlag) {
        case Allocate_BufferRead:
            ExFreePoolWithTag(pvBuffer, MEM_TAG_READ_BUFFER);
            break;

        case Allocate_BufferWrite:
            ExFreePoolWithTag(pvBuffer, MEM_TAG_WRITE_BUFFER);
            break;

        case Allocate_BufferDirectoryControl:
            ExFreePoolWithTag(pvBuffer, MEM_TAG_DIRECTORY_CONTROL_BUFFER);
            break;

        default:
            ExFreePool(pvBuffer);
    }
}
