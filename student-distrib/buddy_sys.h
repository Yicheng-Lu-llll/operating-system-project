#ifndef BUDDY_STSTEM
#define BUDDY_STSTEM
#include "lib.h"
#include "system_call.h"
#include "x86_desc.h"

#define heap_start_address 0x08800000
#define total_page_num 8


typedef union busy_pair_bitmaps
{
    uint8_t val; // 32bit val, share same add with struct below it
    struct{

        uint32_t zero_0 :1;
        uint32_t zero_1 :1;
        uint32_t zero_2 :1;
        uint32_t zero_3 :1;// bit_map[0] need 4 bits. 

        uint32_t one_0 : 1;
        uint32_t one_1 : 1;// bit_map[1] need 2 bits. 

        uint32_t two_0 : 1;// bit_map[2] need 1 bits. 

        uint32_t unused0 :1;
        uint32_t unused1 :1;//unused bit_map[3] need 0 bits. 

    } __attribute__ ((packed));                              
}busy_pair_bitmaps;

 


typedef struct free_area      	
{
    uint32_t free_area_list[4][8];//linux have ten elements.And for simplicity, we do not use linked list.
    busy_pair_bitmaps map;
}free_area;//linked list to manage the free pages 


int32_t init_heap(void);
int32_t demo_show_linked_list();
int32_t demo_show_bitmap();
int32_t mem_allco(uint32_t order);
int32_t fill_first_free_entry_for_one_layer(uint32_t order,uint32_t fill_address);
int32_t get_index_of_first_free_entry_for_one_layer(uint32_t order);
int32_t get_index_of_first_filled_entry_for_one_layer(uint32_t order);
int32_t split_down(uint32_t order);
int32_t power(int32_t a,int32_t b);
void free(int32_t address);



#endif


