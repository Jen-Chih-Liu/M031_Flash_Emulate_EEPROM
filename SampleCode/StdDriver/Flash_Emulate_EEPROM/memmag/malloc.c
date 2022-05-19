#include "malloc.h"
#include "stdio.h"


//內存池(32字節對齊)
__align(32) uint8_t mem1base[MEM1_MAX_SIZE];                                                    //內部SRAM內存池
//__align(32) uint8_t mem2base[MEM2_MAX_SIZE] __attribute__((at(0X68000000)));                  //外部SRAM內存池
//__align(32) uint8_t mem3base[MEM3_MAX_SIZE] __attribute__((at(0X10000000)));                  //內部CCM內存池
//內存管理表
uint32_t mem1mapbase[MEM1_ALLOC_TABLE_SIZE];                                                    //內部SRAM內存池MAP
//u16 mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((at(0X68000000+MEM2_MAX_SIZE))); //外部SRAM內存池MAP
//u16 mem3mapbase[MEM3_ALLOC_TABLE_SIZE] __attribute__((at(0X10000000+MEM3_MAX_SIZE))); //內部CCM內存池MAP
//內存管理參數
const uint32_t memtblsize[SRAMBANK] = {MEM1_ALLOC_TABLE_SIZE};  //內存表大小
const uint32_t memblksize[SRAMBANK] = {MEM1_BLOCK_SIZE};                //內存分塊大小
const uint32_t memsize[SRAMBANK] = {MEM1_MAX_SIZE};                         //內存總大小


//內存管理控制器
struct _m_mallco_dev mallco_dev =
{
    my_mem_init,                        //內存初始化
    my_mem_perused,                     //內存使用率
    mem1base,           //內存池
    mem1mapbase,//內存管理狀態表
    0,                  //內存管理未就緒
};

//複製內存
//*des:目的地址
//*src:源地址
//n:需要複製的內存長度(字節為單位)
void mymemcpy(void *des, void *src, uint32_t n)
{
    uint8_t *xdes = des;
    uint8_t *xsrc = src;

    while (n--)*xdes++ = *xsrc++;
}
//設置內存
//*s:內存首地址
//c :要設置的值
//count:需要設置的內存大小(字節為單位)
void mymemset(void *s, uint8_t c, uint32_t count)
{
    uint8_t *xs = s;

    while (count--)*xs++ = c;
}
//內存管理初始化
//memx:所屬內存塊
void my_mem_init(uint8_t memx)
{
    mymemset(mallco_dev.memmap[memx], 0, memtblsize[memx] * 2); //內存狀態表數據清零
    mymemset(mallco_dev.membase[memx], 0, memsize[memx]);   //內存池所有數據清零
    mallco_dev.memrdy[memx] = 1;                            //內存管理初始化OK
}
//獲取內存使用率
//memx:所屬內存塊
//返回值:使用率(0~100)
uint16_t my_mem_perused(uint8_t memx)
{
    uint32_t used = 0;
    uint32_t i;

    for (i = 0; i < memtblsize[memx]; i++)
    {
        if (mallco_dev.memmap[memx][i])used++;
    }

    return (used * 100) / (memtblsize[memx]);
}
//內存分配(內部調用)
//memx:所屬內存塊
//size:要分配的內存大小(字節)
//返回值:0XFFFFFFFF,代表錯誤;其他,內存偏移地址
uint32_t my_mem_malloc(uint8_t memx, uint32_t size)
{
    signed long offset = 0;
    uint32_t nmemb; //需要的內存塊數
    uint32_t cmemb = 0; //連續空內存塊數
    uint32_t i;

    if (!mallco_dev.memrdy[memx])mallco_dev.init(memx); //未初始化,先執行初始化

    if (size == 0)return 0XFFFFFFFF; //不需要分配

    nmemb = size / memblksize[memx]; //獲取需要分配的連續內存塊數

    if (size % memblksize[memx])nmemb++;

    for (offset = memtblsize[memx] - 1; offset >= 0; offset--) //搜索整個內存控制區
    {
        if (!mallco_dev.memmap[memx][offset])cmemb++; //連續空內存塊數增加
        else cmemb = 0;                             //連續內存塊清零

        if (cmemb == nmemb)                         //找到了連續nmemb個空內存塊
        {
            for (i = 0; i < nmemb; i++)             //標注內存塊非空
            {
                mallco_dev.memmap[memx][offset + i] = nmemb;
            }

            return (offset * memblksize[memx]); //返回偏移地址
        }
    }

    return 0XFFFFFFFF;//未找到符合分配條件的內存塊
}
//釋放內存(內部調用)
//memx:所屬內存塊
//offset:內存地址偏移
//返回值:0,釋放成功;1,釋放失敗;
uint8_t my_mem_free(uint8_t memx, uint32_t offset)
{
    int i;

    if (!mallco_dev.memrdy[memx]) //未初始化,先執行初始化
    {
        mallco_dev.init(memx);
        return 1;//未初始化
    }

    if (offset < memsize[memx]) //偏移在內存池內.
    {
        int index = offset / memblksize[memx];      //偏移所在內存塊號碼
        int nmemb = mallco_dev.memmap[memx][index]; //內存塊數量

        for (i = 0; i < nmemb; i++)                 //內存塊清零
        {
            mallco_dev.memmap[memx][index + i] = 0;
        }

        return 0;
    }
    else return 2; //偏移超區了.
}
//釋放內存(外部調用)
//memx:所屬內存塊
//ptr:內存首地址
void myfree(uint8_t memx, void *ptr)
{
    uint32_t offset;

    if (ptr == NULL)return; //地址為0.

    offset = (uint32_t)ptr - (uint32_t)mallco_dev.membase[memx];
    my_mem_free(memx, offset);  //釋放內存
	  ptr=(void *)0;
}
//分配內存(外部調用)
//memx:所屬內存塊
//size:內存大小(字節)
//返回值:分配到的內存首地址.
void *mymalloc(uint8_t memx, uint32_t size)
{
    uint32_t offset;
    offset = my_mem_malloc(memx, size);

    if (offset == 0XFFFFFFFF)
        return NULL;
    else return (void *)((uint32_t)mallco_dev.membase[memx] + offset);
}
//重新分配內存(外部調用)
//memx:所屬內存塊
//*ptr:舊內存首地址
//size:要分配的內存大小(字節)
//返回值:新分配到的內存首地址.
void *myrealloc(uint8_t memx, void *ptr, uint32_t size)
{
    uint32_t offset;
    offset = my_mem_malloc(memx, size);

    if (offset == 0XFFFFFFFF)
    {
        printf("\r\nrealloc_Err");
        return NULL;
    }

    else
    {
        mymemcpy((void *)((uint32_t)mallco_dev.membase[memx] + offset), ptr, size); //拷貝舊內存內容到新內存
        myfree(memx, ptr);                                                  //釋放舊內存
        return (void *)((uint32_t)mallco_dev.membase[memx] + offset);               //返回新內存首地址
    }
}












