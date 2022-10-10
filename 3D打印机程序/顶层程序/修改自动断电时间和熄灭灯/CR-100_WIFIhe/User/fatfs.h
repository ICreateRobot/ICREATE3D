#ifndef __FATFS_H
#define __FATFS_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <string.h>
#include "system.h"
#include "ff.h"
#include "malloc.h"

#define FILE_MAX_TYPE_NUM    7      /*�ļ����ʹ�������*/
#define FILE_MAX_SUBT_NUM    13     /*�ļ�����С������*/
#define MAX_PATHNAME_DEPTH	(512+1) /*����ļ����*/

extern FATFS *fatfs[1];
extern FILINFO fileInfo;
extern DIR dir;

/*�ļ�ϵͳ��ʼ��*/
FRESULT Exf_Configuration(void);

/*��ȡ�ļ����� *fName:�ļ��� ����ֵ:0xFF,��ʾ�޷�ʶ����ļ����ͱ��;����,����λ��ʾ��������,����λ��ʾ����С��*/
unsigned char Exf_GetType(unsigned char *fname);

/*��ȡ����ʣ������ drv:���̱�� total:������(KB) free:ʣ������(KB)*/
unsigned char Exf_GetFree(unsigned char *drv,unsigned int *total, unsigned int *free);

/*�ļ����� *fcpymsg,ʵ����Ϣ(*pname:�ļ�/�ļ����� pct:�ٷֱ� mode:0,�����ļ���;1,���°ٷֱ�;2,�����ļ���)
           *psrc:Դ�ļ� *pdst:Ŀ���ļ� totsize:�ܴ�С(totsize=0Ϊ�����ļ�����) cpdsize:�Ѹ��ƴ�С fwmode:0,������ԭ�ļ�;1,����ԭ�ļ�*/
unsigned char Exf_CopyFile(unsigned char(*fcpymsg)(unsigned char *pname,unsigned char pct,unsigned char mode), \
										 unsigned char *psrc, unsigned char *pdst, unsigned int totsize, unsigned int cpdsize, unsigned char fwmode);

/*��ȡ·���µ��ļ��� ����ֵ:0,·����Ϊ����;����,�ļ��������׵�ַ*/
unsigned char *Exf_GetFolderName(unsigned char *dpfn);

/*��ȡ�ļ��д�С ����ֵ:0,�ļ��д�СΪ0,���߶�ȡ�����з����˴���;����,�ļ��д�С*/
unsigned int Exf_GetFolderSize(unsigned char *fdname);

/*�ļ��и��� *fcpymsg,ʵ����Ϣ(*pname:�ļ�/�ļ����� pct:�ٷֱ� mode:0,�����ļ���;1,���°ٷֱ�;2,�����ļ���)
						 *psrc:Դ�ļ��� *pdst:Ŀ���ļ��� totsize:�ܴ�С(totsize=0Ϊ�����ļ�����) cpdsize:�Ѹ��ƴ�С fwmode:0,������ԭ�ļ�;1,����ԭ�ļ�*/
unsigned char Exf_CopyFolder(unsigned char(*fcpymsg)(unsigned char*pname,unsigned char pct,unsigned char mode), \
														 unsigned char *psrc, unsigned char *pdst, unsigned int *totsize, unsigned int *cpdsize, unsigned char fwmode);

#ifdef __cplusplus
}
#endif

#endif
