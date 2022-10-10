#ifndef __FATFS_H
#define __FATFS_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <string.h>
#include "system.h"
#include "ff.h"
#include "malloc.h"

#define FILE_MAX_TYPE_NUM    7      /*文件类型大类数量*/
#define FILE_MAX_SUBT_NUM    13     /*文件类型小类数量*/
#define MAX_PATHNAME_DEPTH	(512+1) /*最大文件深度*/

extern FATFS *fatfs[1];
extern FILINFO fileInfo;
extern DIR dir;

/*文件系统初始化*/
FRESULT Exf_Configuration(void);

/*获取文件类型 *fName:文件名 返回值:0xFF,表示无法识别的文件类型编号;其他,高四位表示所属大类,低四位表示所属小类*/
unsigned char Exf_GetType(unsigned char *fname);

/*获取磁盘剩余容量 drv:磁盘编号 total:总容量(KB) free:剩余容量(KB)*/
unsigned char Exf_GetFree(unsigned char *drv,unsigned int *total, unsigned int *free);

/*文件复制 *fcpymsg,实现信息(*pname:文件/文件夹名 pct:百分比 mode:0,更新文件名;1,更新百分比;2,更新文件夹)
           *psrc:源文件 *pdst:目标文件 totsize:总大小(totsize=0为单个文件拷贝) cpdsize:已复制大小 fwmode:0,不覆盖原文件;1,覆盖原文件*/
unsigned char Exf_CopyFile(unsigned char(*fcpymsg)(unsigned char *pname,unsigned char pct,unsigned char mode), \
										 unsigned char *psrc, unsigned char *pdst, unsigned int totsize, unsigned int cpdsize, unsigned char fwmode);

/*获取路径下的文件夹 返回值:0,路径即为卷标号;其他,文件夹名字首地址*/
unsigned char *Exf_GetFolderName(unsigned char *dpfn);

/*获取文件夹大小 返回值:0,文件夹大小为0,或者读取过程中发生了错误;其他,文件夹大小*/
unsigned int Exf_GetFolderSize(unsigned char *fdname);

/*文件夹复制 *fcpymsg,实现信息(*pname:文件/文件夹名 pct:百分比 mode:0,更新文件名;1,更新百分比;2,更新文件夹)
						 *psrc:源文件夹 *pdst:目标文件夹 totsize:总大小(totsize=0为单个文件拷贝) cpdsize:已复制大小 fwmode:0,不覆盖原文件;1,覆盖原文件*/
unsigned char Exf_CopyFolder(unsigned char(*fcpymsg)(unsigned char*pname,unsigned char pct,unsigned char mode), \
														 unsigned char *psrc, unsigned char *pdst, unsigned int *totsize, unsigned int *cpdsize, unsigned char fwmode);

#ifdef __cplusplus
}
#endif

#endif
