/**
  ******************************************************************************
  * @file     fatfs.c
  * @author   LEO
	* @date     2018/10/12
	* @version  0.0.1
  * @brief    FatFs�ļ�����ϵͳ�ӿ�
	******************************************************************************
**/
#include "fatfs.h"

FATFS *fatfs[1];        /*�߼����̹�����*/
FILINFO fileInfo;       /*�ļ���Ϣ*/
DIR dir;                /*Ŀ¼*/

/*�ļ������б�*/
const char *FileType_Table[FILE_MAX_TYPE_NUM][FILE_MAX_SUBT_NUM] =
{
{"BIN"},
{"LRC"},
{"NES"},
{"TXT", "C", "H"},
{"MP1", "MP2", "MP3", "MP4", "M4A", "3GP", "3G2", "OGG", "ACC", "WMA", "WAV", "MID", "FLAC"},
{"BMP", "JPG", "JPEG", "GIF"},
{"GCODE"}
};																			

/*�ļ�ϵͳ��ʼ��*/
FRESULT Exf_Configuration(void)
{
	fatfs[0] = (FATFS*)Memory_Malloc(MEMORY_IN_BANK, sizeof(FATFS));
	if(fatfs[0]) return FR_OK;
	return FR_INT_ERR;
}

/*��ĸСдת��д*/
unsigned char char_upper(unsigned char c)
{
	if(c < 'A') return c;
	else if(c >= 'a') return (c - ' ');
	return c;
}

/*��ȡ�ļ����� *fName:�ļ��� ����ֵ:0xFF,��ʾ�޷�ʶ����ļ����ͱ��;����,����λ��ʾ��������,����λ��ʾ����С��*/
unsigned char Exf_GetType(unsigned char *fname)
{
	unsigned char tempBuf[6];
	unsigned char *attr = (unsigned char *)'\0';
	unsigned char counter_i=0 , counter_j;
	while(counter_i < 250)
	{
		counter_i++;
		if(*fname == '\0') break;
		fname++;
	}
	if(counter_i == 250) return 0xFF;
 	for(counter_i=0; counter_i<6; counter_i++)
	{
		fname--;
		if(*fname == '.')
		{
			fname++;
			attr = fname;
			break;
		}
	}
	strcpy((char *)tempBuf, (const char*)attr);
 	for(counter_i=0; counter_i<5; counter_i++) tempBuf[counter_i] = char_upper(tempBuf[counter_i]);
	for(counter_i=0; counter_i<FILE_MAX_TYPE_NUM; counter_i++)
	{
		for(counter_j=0; counter_j<FILE_MAX_SUBT_NUM; counter_j++)
		{
			if(*FileType_Table[counter_i][counter_j] == 0) break;
			if(strcmp((const char *)FileType_Table[counter_i][counter_j],(const char *)tempBuf) == 0) return (counter_i<<4) | counter_j;
		}
	}
	return 0xFF;
}

/*��ȡ����ʣ������ drv:���̱�� total:������(KB) free:ʣ������(KB)*/
unsigned char Exf_GetFree(unsigned char *drv,unsigned int *total, unsigned int *free)
{
	FATFS *fatfs;
	unsigned char res;
	unsigned int fre_clust=0, fre_sect=0, tot_sect=0;
	res = f_getfree((const TCHAR*)drv, (DWORD*)&fre_clust, &fatfs);
	if(res == 0)
	{
		tot_sect = (fatfs->n_fatent-2) * fatfs->csize;
		fre_sect = fre_clust * fatfs->csize;
#if FF_MAX_SS != 512
		tot_sect *= fatfs->ssize / 512;
		fre_sect *= fatfs->ssize / 512;
#endif	  
		*total = tot_sect >> 1;
		*free = fre_sect >> 1;
 	}
	return res;
}

/*�ļ����� *fcpymsg,ʵ����Ϣ(*pname:�ļ�/�ļ����� pct:�ٷֱ� mode:0,�����ļ���;1,���°ٷֱ�;2,�����ļ���)
           *psrc:Դ�ļ� *pdst:Ŀ���ļ� totsize:�ܴ�С(totsize=0Ϊ�����ļ�����) cpdsize:�Ѹ��ƴ�С fwmode:0,������ԭ�ļ�;1,����ԭ�ļ�*/
unsigned char Exf_CopyFile(unsigned char(*fcpymsg)(unsigned char *pname,unsigned char pct,unsigned char mode), \
										 unsigned char *psrc, unsigned char *pdst, unsigned int totsize, unsigned int cpdsize, unsigned char fwmode)
{
	unsigned char res;
	unsigned short int br=0, bw=0;
	FIL *fsrc=0, *fdst=0;
	unsigned char *fbuf = 0;
	unsigned char curpct = 0;
 	fsrc = (FIL*)Memory_Malloc(MEMORY_IN_BANK, sizeof(FIL));
 	fdst = (FIL*)Memory_Malloc(MEMORY_IN_BANK, sizeof(FIL));
	fbuf = (unsigned char*)Memory_Malloc(MEMORY_IN_BANK, 8192);
	if(fsrc==NULL || fdst==NULL || fbuf==NULL) res = FR_NOT_ENOUGH_CORE;
	else
	{
		if(fwmode == 0) fwmode = FA_CREATE_NEW;
		else fwmode = FA_CREATE_ALWAYS;
	 	res = f_open(fsrc, (const TCHAR*)psrc, FA_READ|FA_OPEN_EXISTING);
	 	if(res == 0) res = f_open(fdst, (const TCHAR*)pdst, FA_WRITE|fwmode);
		if(res == 0)
		{
			if(totsize == 0)
			{
				totsize = fsrc->obj.objsize;
				cpdsize = 0;
				curpct = 0;
		 	}
			else curpct = (cpdsize*100) / totsize;
			fcpymsg(psrc, curpct, 0x02);
			while(res == 0)
			{
				res = f_read(fsrc, fbuf, 8192, (UINT*)&br);
				if(res || br==0) break;
				res = f_write(fdst, fbuf, (UINT)br, (UINT*)&bw);
				cpdsize += bw;
				if(curpct != (cpdsize*100)/totsize)
				{
					curpct = (cpdsize*100) / totsize;
					if(fcpymsg(psrc, curpct, 0x02))
					{
						res = 0xFF;
						break;
					}
				}
				if(res || bw<br) break;
			}
			f_close(fsrc);
			f_close(fdst);
		}
	}
	Memory_Free(MEMORY_IN_BANK, fsrc);
	Memory_Free(MEMORY_IN_BANK, fdst);
	Memory_Free(MEMORY_IN_BANK, fbuf);
	return res;
}

/*��ȡ·���µ��ļ��� ����ֵ:0,·����Ϊ����;����,�ļ��������׵�ַ*/
unsigned char *Exf_GetFolderName(unsigned char *dpfn)
{
	unsigned short int temp = 0;
 	while(*dpfn != 0)
	{
		dpfn++;
		temp++;
	}
	if(temp < 4) return 0;
	while((*dpfn!=0x5C) && (*dpfn!=0x2F)) dpfn--;
	return ++dpfn;
}

/*��ȡ�ļ��д�С ����ֵ:0,�ļ��д�СΪ0,���߶�ȡ�����з����˴���;����,�ļ��д�С*/
unsigned int Exf_GetFolderSize(unsigned char *fdname)
{
	unsigned char res = 0;
	DIR *fddir = 0;
	FILINFO *finfo = 0;
	unsigned char *pathname = 0;
 	unsigned short int pathlen = 0;
	unsigned int fdsize = 0;
	fddir = (DIR*)Memory_Malloc(MEMORY_IN_BANK, sizeof(DIR));
 	finfo = (FILINFO*)Memory_Malloc(MEMORY_IN_BANK, sizeof(FILINFO));
	if(fddir==NULL || finfo==NULL) res = 100;
	if(res == 0)
	{ 
 		pathname = Memory_Malloc(MEMORY_IN_BANK, MAX_PATHNAME_DEPTH);
 		if(pathname == NULL) res = 101;
 		if(res == 0)
		{
			pathname[0] = 0;
			strcat((char*)pathname, (const char*)fdname);
			res = f_opendir(fddir, (const TCHAR*)fdname);
			if(res == 0)
			{
				while(res == 0)
				{
					res = f_readdir(fddir, finfo);
					if(res!=FR_OK || finfo->fname[0]==0) break;
					if(finfo->fname[0] == '.') continue;
					if(finfo->fattrib & 0x10)
					{
						pathlen = strlen((const char*)pathname);
						strcat((char*)pathname, (const char*)"/");
						strcat((char*)pathname, (const char*)finfo->fname);
						fdsize += Exf_GetFolderSize(pathname);
						pathname[pathlen] = 0;
					}
					else fdsize += finfo->fsize;
				}
			}
			Memory_Free(MEMORY_IN_BANK, pathname);
		}
 	}
	Memory_Free(MEMORY_IN_BANK, fddir);
	Memory_Free(MEMORY_IN_BANK, finfo);
	if(res) return 0;
	else return fdsize;
}

/*�ļ��и��� *fcpymsg,ʵ����Ϣ(*pname:�ļ�/�ļ����� pct:�ٷֱ� mode:0,�����ļ���;1,���°ٷֱ�;2,�����ļ���)
						 *psrc:Դ�ļ��� *pdst:Ŀ���ļ��� totsize:�ܴ�С(totsize=0Ϊ�����ļ�����) cpdsize:�Ѹ��ƴ�С fwmode:0,������ԭ�ļ�;1,����ԭ�ļ�*/
unsigned char Exf_CopyFolder(unsigned char(*fcpymsg)(unsigned char*pname,unsigned char pct,unsigned char mode), \
														 unsigned char *psrc, unsigned char *pdst, unsigned int *totsize, unsigned int *cpdsize, unsigned char fwmode)
{
	unsigned char res = 0;
	DIR *srcdir=0, *dstdir=0;
	FILINFO *finfo = 0;
	unsigned char *fn = 0;
	unsigned char *dstpathname=0, *srcpathname=0;
 	unsigned short int dstpathlen=0, srcpathlen=0;
	srcdir = (DIR*)Memory_Malloc(MEMORY_IN_BANK, sizeof(DIR));
 	dstdir = (DIR*)Memory_Malloc(MEMORY_IN_BANK, sizeof(DIR));
	finfo = (FILINFO*)Memory_Malloc(MEMORY_IN_BANK, sizeof(FILINFO));
	if(srcdir==NULL || dstdir==NULL || finfo==NULL) res = 100;
	if(res == 0)
	{
 		dstpathname = Memory_Malloc(MEMORY_IN_BANK, MAX_PATHNAME_DEPTH);
		srcpathname = Memory_Malloc(MEMORY_IN_BANK, MAX_PATHNAME_DEPTH);
 		if(dstpathname==NULL || srcpathname==NULL) res = 101;
 		if(res == 0)
		{
			dstpathname[0] = 0;
			srcpathname[0] = 0;
			strcat((char*)srcpathname, (const char*)psrc);
			strcat((char*)dstpathname, (const char*)pdst);
			res = f_opendir(srcdir, (const TCHAR*)psrc);
			if(res == 0)
			{
				strcat((char*)dstpathname, (const char*)"/");
 				fn = Exf_GetFolderName(psrc);
				if(fn == 0)
				{
					dstpathlen = strlen((const char*)dstpathname);
					dstpathname[dstpathlen] = psrc[0];
					dstpathname[dstpathlen+1] = 0;
				}
				else strcat((char*)dstpathname, (const char*)fn);
 				fcpymsg(fn, 0, 0x04);
				res = f_mkdir((const TCHAR*)dstpathname);
				if(res == FR_EXIST) res = 0;
				while(res == 0)
				{
					res = f_readdir(srcdir, finfo);
					if(res!=FR_OK || finfo->fname[0]==0) break;
					if(finfo->fname[0] == '.') continue;
					fn = (unsigned char*)finfo->fname;
					dstpathlen = strlen((const char*)dstpathname);
					srcpathlen = strlen((const char*)srcpathname);
					strcat((char*)srcpathname, (const char*)"/");
 					if(finfo->fattrib & 0x10)
					{
						strcat((char*)srcpathname, (const char*)fn);
						res = Exf_CopyFolder(fcpymsg, srcpathname, dstpathname, totsize, cpdsize, fwmode);
					}
					else
					{
						strcat((char*)dstpathname, (const char*)"/");
						strcat((char*)dstpathname, (const char*)fn);
						strcat((char*)srcpathname, (const char*)fn);
						fcpymsg(fn, 0, 0x01);
						res = Exf_CopyFile(fcpymsg, srcpathname, dstpathname, *totsize, *cpdsize, fwmode);
						*cpdsize += finfo->fsize;
					}
					srcpathname[srcpathlen] = 0;
					dstpathname[dstpathlen] = 0;
				}
			}
			Memory_Free(MEMORY_IN_BANK, dstpathname);
 			Memory_Free(MEMORY_IN_BANK, srcpathname);
		}
 	}
	Memory_Free(MEMORY_IN_BANK, srcdir);
	Memory_Free(MEMORY_IN_BANK, dstdir);
	Memory_Free(MEMORY_IN_BANK, finfo);
	return res;
}
