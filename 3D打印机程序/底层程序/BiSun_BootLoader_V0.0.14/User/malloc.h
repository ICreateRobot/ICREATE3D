#ifndef __MALLOC_H
#define __MALLOC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"

/*内存块数量定义*/
#define MEMORY_BANK_NUM  1

/*内部内存分配*/
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

/*内存管理初始化 memory:所属内存池*/
void Memory_Configuration(unsigned char memory);

/*获取内存使用率 memory:所属内存池 返回值:使用率(0~100)*/
unsigned char Memory_GetUseRate(unsigned char memory);

/*内存分配 memory:所属内存池 length:要分配的内存长度(字节)*/
void *Memory_Malloc(unsigned char memory, unsigned int length);

/*内存释放 memory:所属内存池 startAddress:内存首地址*/
ErrorStatus Memory_Free(unsigned char memory, void *startAddress);

/*内存重新分配 memory:所属内存池 startAddress:内存首地址 length:要分配的内存长度(字节)*/
void *Memory_Remalloc(unsigned char memory, void *startAddress, unsigned int length);

#ifdef __cplusplus
}
#endif

#endif
