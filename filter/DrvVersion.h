///////////////////////////////////////////////////////////////////////////////
//
// ��Ȩ���� (c) 2011 - 2012
//
// ������Դ�汾��Ϣ�Ķ���
//
// (File was in the PUBLIC DOMAIN  - Created by: ddkwizard\.assarbad\.net)
//
///////////////////////////////////////////////////////////////////////////////

// $Id$

#ifndef __DRVVERSION_H_VERSION__
#define __DRVVERSION_H_VERSION__ 100

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "buildnumber.h"

// ---------------------------------------------------------------------------
// һЩ��������ڰ������ļ�ʱ����
// ---------------------------------------------------------------------------
#define TEXT_AUTHOR            Coxious Hall (Shuxiang Cao)
#define PRD_MAJVER             1                        // �������汾��
#define PRD_MINVER             0                        // ���̸��汾��
#define PRD_BUILD              _FILE_VERSION_BUILD      // �������
#define FILE_MAJVER            1                        // ���ļ�
#define FILE_MINVER            0                        // minor file version
#define FILE_BUILD             _FILE_VERSION_BUILD      // file build number
#define DRV_YEAR               2010-2012                // ��ǰ���ʱ���� (e.g. 2003-2009)
#define TEXT_WEBSITE           none                     // ��ҳ
#define TEXT_PRODUCTNAME       Antinvader ����ģ��      // ��������
#define TEXT_FILEDESC          Antinvader ΢�����������ں˱���ģ�� biuld _FILE_VERSION_BUILD    // ��������
#define TEXT_COMPANY           // ��˾
#define TEXT_MODULE            AntinvaderDriver         // ģ������
#define TEXT_COPYRIGHT         ��Ȩ����(c)DRV_YEAR TEXT_AUTHOR  // ��Ȩ��Ϣ
// #define TEXT_SPECIALBUILD    // optional comment for special builds
#define TEXT_INTERNALNAME      AntinvaderDriver.sys     // ��Ȩ��Ϣ
// #define TEXT_COMMENTS        // optional comments
// ---------------------------------------------------------------------------
// ... well, that's it. Pretty self-explanatory ;)
// ---------------------------------------------------------------------------

#endif // __DRVVERSION_H_VERSION__
