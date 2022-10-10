#ifndef __MALLOC_H
#define __MALLOC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

/*�ڴ����������*/
#define MEMORY_BANK_NUM  1

/*�ڲ��ڴ����*/
#define MEMORY_IN_BANK        0
#define MEMORY_IN_BLOCK_SIZE  32
#define MEMORY_IN_MAX_SIZE    (20 * 1024)
#define MEMORY_IN_TABLE_SIZE  (MEMORY_IN_MAX_SIZE / MEMORY_IN_BLOCK_SIZE)

typedef struct
{
	void (*Configuration)(unsigned char);
	unsigned char (*UseRate)(unsigned char);
	unsigned char *Pool[MEMORY_BANK_NUM];
	unsigned short int *Table[MEMORY_BANK_NUM];
	FunctionalState State[MEMORY_BANK_NUM];
}Memory_TypeDef;

/*�ڴ�����ʼ�� memory:�����ڴ��*/
void Memory_Configuration(unsigned char memory);

/*��ȡ�ڴ�ʹ���� memory:�����ڴ�� ����ֵ:ʹ����(0~100)*/
unsigned char Memory_GetUseRate(unsigned char memory);

/*�ڴ���� memory:�����ڴ�� length:Ҫ������ڴ泤��(�ֽ�)*/
void *Memory_Malloc(unsigned char memory, unsigned int length);

/*�ڴ��ͷ� memory:�����ڴ�� startAddress:�ڴ��׵�ַ*/
ErrorStatus Memory_Free(unsigned char memory, void *startAddress);

/*�ڴ����·��� memory:�����ڴ�� startAddress:�ڴ��׵�ַ length:Ҫ������ڴ泤��(�ֽ�)*/
void *Memory_Remalloc(unsigned char memory, void *startAddress, unsigned int length);

#ifdef __cplusplus
}
#endif

#endif
