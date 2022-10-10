/**
  ******************************************************************************
  * @file     malloc.c
  * @author   LEO
  * @date     2018/10/12
  * @version  0.0.1
  * @brief    内存管理操作
  ******************************************************************************
**/

#include "malloc.h"

__align(32) unsigned char MemoryIn_Pool[MEMORY_IN_MAX_SIZE];

unsigned short int MemoryIn_Table[MEMORY_IN_TABLE_SIZE];

const unsigned int MemoryBlock_Size[MEMORY_BANK_NUM] = {MEMORY_IN_BLOCK_SIZE};
const unsigned int MemoryPool_Size[MEMORY_BANK_NUM] = {MEMORY_IN_MAX_SIZE};
const unsigned int MemoryTable_Size[MEMORY_BANK_NUM] = {MEMORY_IN_TABLE_SIZE};

Memory_TypeDef Memory_Struct =
{
  Memory_Configuration,
  Memory_GetUseRate,
  MemoryIn_Pool,
  MemoryIn_Table,
  DISABLE
};

/*复制内存 *desAddress:目的地址 *srcAddress:源地址 length:需要设置的内存长度(字节)*/
void Memory_Copy(void *desAddress, void *srcAddress, unsigned int length)
{
  unsigned char *des = desAddress;
  unsigned char *src = srcAddress;
  while(length--) *des++ = *src++;
}

/*设置内存 *startAddress:内存首地址 value:要设置的值 length:需要设置的内存长度(字节)*/
void Memory_Write(void *startAddress, unsigned char value, unsigned int length)
{
  unsigned char *tempAddress = startAddress;
  while(length--) *tempAddress++ = value;
}

/*内存管理初始化 memory:所属内存池*/
void Memory_Configuration(unsigned char memory)
{
  Memory_Write(Memory_Struct.Table[memory], 0, MemoryTable_Size[memory]*2);
  Memory_Write(Memory_Struct.Pool[memory], 0, MemoryPool_Size[memory]);
  Memory_Struct.State[memory] = ENABLE;
}

/*获取内存使用率 memory:所属内存池 返回值:使用率(0~100)*/
unsigned char Memory_GetUseRate(unsigned char memory)
{
  unsigned int tempRate = 0;
  unsigned int tempCounter;
  for(tempCounter=0; tempCounter<MemoryTable_Size[memory]; tempCounter++)
  {
    if(Memory_Struct.Table[memory][tempCounter]) tempRate++;
  }
  return tempRate * 100 / MemoryTable_Size[memory];
}

/*内存分配检查 memory:所属内存池 length:要分配的内存长度(字节)*/
unsigned int Memory_MallocCheck(unsigned char memory, unsigned int length)
{
  signed long tempOffset = 0;
  unsigned int tempBlockNum;
  unsigned int tempContinuousNum = 0;
  unsigned int tempCounter;
  if(!Memory_Struct.State[memory]) Memory_Struct.Configuration(memory);
  if(length == 0) return 0xFFFFFFFF;
  tempBlockNum = length / MemoryBlock_Size[memory];
  if(length % MemoryBlock_Size[memory]) tempBlockNum++;
  for(tempOffset=MemoryTable_Size[memory]-1; tempOffset>=0; tempOffset--)
  {
    if(!Memory_Struct.Table[memory][tempOffset])tempContinuousNum++;
    else tempContinuousNum = 0;
    if(tempContinuousNum == tempBlockNum)
    {
      for(tempCounter=0; tempCounter<tempBlockNum; tempCounter++) Memory_Struct.Table[memory][tempOffset+tempCounter] = tempBlockNum;
      return (tempOffset * MemoryBlock_Size[memory]);
    }
  }
  return 0xFFFFFFFF;
}

/*内存释放检查 memory:所属内存池 offsetAddress:偏移地址*/
ErrorStatus Memory_FreeCheck(unsigned char memory, unsigned int offsetAddress)
{
  int tempCounter;
  unsigned int tempBlockNum;
  unsigned int tempBlockCount;
  if(!Memory_Struct.State[memory])
  {
    Memory_Struct.Configuration(memory);
    return ERROR;
  }
  if(offsetAddress < MemoryPool_Size[memory])
  {
    tempBlockNum = offsetAddress / MemoryBlock_Size[memory];
    tempBlockCount = Memory_Struct.Table[memory][tempBlockNum];
    for(tempCounter=0; tempCounter<tempBlockCount; tempCounter++) Memory_Struct.Table[memory][tempBlockNum+tempCounter] = 0;
    return SUCCESS;
  }
  else return ERROR;
}

/*内存分配 memory:所属内存池 length:要分配的内存长度(字节)*/
void *Memory_Malloc(unsigned char memory, unsigned int length)
{
  unsigned int tempOffset;
  tempOffset = Memory_MallocCheck(memory, length);
  if(tempOffset == 0xFFFFFFFF) return NULL;
  else return (void*)((unsigned int)Memory_Struct.Pool[memory] + tempOffset);
}

/*内存释放 memory:所属内存池 startAddress:内存首地址*/
ErrorStatus Memory_Free(unsigned char memory, void *startAddress)
{
  unsigned int tempOffset;
  if(startAddress == NULL) return ERROR;
  tempOffset = (unsigned int)startAddress - (unsigned int)Memory_Struct.Pool[memory];
  return Memory_FreeCheck(memory, tempOffset);
}

/*内存重新分配 memory:所属内存池 startAddress:内存首地址 length:要分配的内存长度(字节)*/
void *Memory_Remalloc(unsigned char memory, void *startAddress, unsigned int length)
{
  unsigned int tempOffset;
  tempOffset = Memory_MallocCheck(memory, length);
  if(tempOffset == 0xFFFFFFFF) return NULL;
  else
  {
    Memory_Copy((void*)((unsigned int)Memory_Struct.Pool[memory]+tempOffset), startAddress, length);
    Memory_Free(memory, startAddress);
    return (void*)((unsigned int)Memory_Struct.Pool[memory] + tempOffset);
  }
}
