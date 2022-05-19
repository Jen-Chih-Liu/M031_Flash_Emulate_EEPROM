#ifndef __MALLOC_H
#define __MALLOC_H
#include "NuMicro.h"


#ifndef NULL
    #define NULL 0
#endif

//定義三個內存池
#define SRAMIN   0      //內部內存池



#define SRAMBANK    1   //定義支持的SRAM塊數.   


//mem1內存參數設定.mem1完全處於內部SRAM裡面.
#define MEM1_BLOCK_SIZE         32                              //內存塊大小為32字節
#define MEM1_MAX_SIZE           64*1024                         //最大管理內存 64K
#define MEM1_ALLOC_TABLE_SIZE   MEM1_MAX_SIZE/MEM1_BLOCK_SIZE   //內存表大小

////mem2內存參數設定.mem2的內存池處於外部SRAM裡面
//#define MEM2_BLOCK_SIZE           32                              //內存塊大小為32字節
//#define MEM2_MAX_SIZE         960 *1024                       //最大管理內存960K
//#define MEM2_ALLOC_TABLE_SIZE MEM2_MAX_SIZE/MEM2_BLOCK_SIZE   //內存表大小
//
////mem3內存參數設定.mem3處於CCM,用於管理CCM(特別注意,這部分SRAM,僅CPU可以訪問!!)
//#define MEM3_BLOCK_SIZE           32                              //內存塊大小為32字節
//#define MEM3_MAX_SIZE         60 *1024                        //最大管理內存60K
//#define MEM3_ALLOC_TABLE_SIZE MEM3_MAX_SIZE/MEM3_BLOCK_SIZE   //內存表大小
//


//內存管理控制器
struct _m_mallco_dev
{
    void (*init)(uint8_t);                  //初始化
    uint16_t (*perused)(uint8_t);                   //內存使用率
    uint8_t     *membase[SRAMBANK];             //內存池 管理SRAMBANK個區域的內存
    uint32_t *memmap[SRAMBANK];                 //內存管理狀態表
    uint8_t  memrdy[SRAMBANK];              //內存管理是否就緒
};
extern struct _m_mallco_dev mallco_dev;  //在mallco.c裡面定義

void mymemset(void *s, uint8_t c, uint32_t count);  //設置內存
void mymemcpy(void *des, void *src, uint32_t n); //複製內存
void my_mem_init(uint8_t memx);             //內存管理初始化函數(外/內部調用)
uint32_t my_mem_malloc(uint8_t memx, uint32_t size); //內存分配(內部調用)
uint8_t my_mem_free(uint8_t memx, uint32_t offset);     //內存釋放(內部調用)
uint16_t my_mem_perused(uint8_t memx);              //獲得內存使用率(外/內部調用)
////////////////////////////////////////////////////////////////////////////////
//用戶調用函數
void myfree(uint8_t memx, void *ptr);           //內存釋放(外部調用)
void *mymalloc(uint8_t memx, uint32_t size);        //內存分配(外部調用)
void *myrealloc(uint8_t memx, void *ptr, uint32_t size); //重新分配內存(外部調用)
#endif













