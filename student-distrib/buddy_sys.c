#include "buddy_sys.h"
//four_k
free_area heap_manager;

// block* block_constructor(uint32_t address=NULL,uint32_t size=NULL,uint32_t next=NULL){

// }

int32_t init_heap(void){
    //open all pages
    int32_t PD_entry_index = heap_start_address >>offset_22; 
    PD[PD_entry_index].k.p=1; // present set to present
    PD[PD_entry_index].k.r_w=1;// enable write and read
    PD[PD_entry_index].k.u_s=1; //set supervisor priviledge level
    PD[PD_entry_index].k.pwt=0; // make write cache enable
    PD[PD_entry_index].k.pcd=0; // make page table to be cached
    PD[PD_entry_index].k.a=0; //initially set to 0, wait user to visit for first time
    PD[PD_entry_index].k.reserved=0; //basically set to 0
    PD[PD_entry_index].k.ps=0; // clear for page size 4kb
    PD[PD_entry_index].k.g=0;//not frequently use 
    PD[PD_entry_index].k.avail=0; //all 32 bits are available to software
    PD[PD_entry_index].k.ptb_add=(int) PT_for_heap>>shift_twelve; //get the most significant 20 bit

    int index = 0;
    for(; index<total_page_num; index++){
        PT_for_heap[index].p=1;// present set to present
        PT_for_heap[index].r_w=1;//// enable write and read
        PT_for_heap[index].u_s=1; //set supervisor priviledge level
        PT_for_heap[index].pwt=0; // make write cache enable
        PT_for_heap[index].pcd=0;// make page table to be cached
        PT_for_heap[index].a=0; //initially set to 0, wait user to visit for first time
        PT_for_heap[index].d=1; //set when point to page 
        PT_for_heap[index].pat=0;//set 0 fro processor not support
        PT_for_heap[index].g=0;//frequently use, global page
        PT_for_heap[index].avail=0;//all 32 bits are available to software
    }

    //init heap_manager
    index = 0;
    int index2 = 0;
    for(index =0 ; index<4; index++){
        for(index2 =0 ; index2<8; index2++)
        heap_manager.free_area_list[index][index2] = NULL;
    }
    heap_manager.free_area_list[3][0] =200;// heap_start_address;
    heap_manager.map.val = 0;

    //test

    // fill_first_free_entry_for_one_layer(3,2);
    // fill_first_free_entry_for_one_layer(3,3);
    // fill_first_free_entry_for_one_layer(3,4);
    //mem_allco(0);
    // demo_show_linked_list();
   
    
    // demo_show_bitmap();



   
    return 0;
}
int32_t demo_show_linked_list(){
    printf("linked list layer 0:  %d %d %d %d\n",heap_manager.free_area_list[0][0],heap_manager.free_area_list[0][1],heap_manager.free_area_list[0][2],heap_manager.free_area_list[0][3],heap_manager.free_area_list[0][4]);
    printf("linked list layer 1:  %d %d %d %d\n",heap_manager.free_area_list[1][0],heap_manager.free_area_list[1][1],heap_manager.free_area_list[1][2],heap_manager.free_area_list[1][3],heap_manager.free_area_list[1][4]);
    printf("linked list layer 2:  %d %d %d %d\n",heap_manager.free_area_list[2][0],heap_manager.free_area_list[2][1],heap_manager.free_area_list[2][2],heap_manager.free_area_list[2][3],heap_manager.free_area_list[2][4]);
    printf("linked list layer 3:  %d %d %d %d\n",heap_manager.free_area_list[3][0],heap_manager.free_area_list[3][1],heap_manager.free_area_list[3][2],heap_manager.free_area_list[3][3],heap_manager.free_area_list[3][4]);
    return 0;
}


int32_t demo_show_bitmap(){
    printf("1-page chunk first  layer:   %d %d %d %d\n",heap_manager.map.zero_0,heap_manager.map.zero_1,heap_manager.map.zero_2,heap_manager.map.zero_3);
    printf("2-page chunk second layer:    %d   %d  \n",heap_manager.map.one_0,heap_manager.map.one_1);
    printf("4-page chunkthird  layer:      %d    \n",heap_manager.map.one_0,heap_manager.map.one_1);
    return 0;
}

int32_t mem_allco(uint32_t order){

    if (order>3){return -1;}
    int32_t index = 0;
    int32_t layer = order;
    int32_t desired_layer = order;
    int32_t choen_layer = -1;
    int32_t choen_index = -1;
    int32_t is_over = -1;


    for(layer = order;layer<4; layer++){
        if(is_over == 1){break;}
        for(index = 0; index<8; index ++){
            if(is_over == 1){break;}
            if(heap_manager.free_area_list[layer][index] != NULL){
                
                choen_layer = layer;
                choen_index = index;
                is_over = 1;
            }
        }
    }

    //can not find enough page
    if(choen_layer == -1 || choen_index ==-1){return -1;}

    while(desired_layer < choen_layer){
        split_down( choen_layer);
        choen_layer--;
    }

    index = get_index_of_first_filled_entry_for_one_layer(desired_layer);
    int32_t address = heap_manager.free_area_list[desired_layer][index];

    //case: desired_layer == choen_layer   =1:
    fill_first_free_entry_for_one_layer(desired_layer,NULL);

return address;
}

int32_t fill_first_free_entry_for_one_layer(uint32_t order,uint32_t fill_address){
    int index = 0;
    for(index = 0; index<8; index ++){
        if(heap_manager.free_area_list[order][index] == NULL){
            heap_manager.free_area_list[order][index] = fill_address;
            return 0;
        }
        
    }

    return -1;
}

int32_t get_index_of_first_free_entry_for_one_layer(uint32_t order){
    int index = 0;
    for(index = 0; index<8; index ++){
        if(heap_manager.free_area_list[order][index] == NULL){
            return index;
        }
    }
    return -1;
}

int32_t get_index_of_first_filled_entry_for_one_layer(uint32_t order){
    int index = 0;
    for(index = 0; index<8; index ++){
        if(heap_manager.free_area_list[order][index] != NULL){
            return index;
        }
    }
    return -1;
}

int32_t split_down(uint32_t order){
    if(order == 0){return -1;}
    uint32_t index = get_index_of_first_filled_entry_for_one_layer(order);
    uint32_t address = heap_manager.free_area_list[order][index];
    heap_manager.free_area_list[order][index] = 0;

  

    index = get_index_of_first_free_entry_for_one_layer( order -1);
    heap_manager.free_area_list[order-1][index] = (uint32_t) (address + (100*(power(2,order)))/2);//four_k*(2^order)

    index = get_index_of_first_free_entry_for_one_layer( order -1);
    heap_manager.free_area_list[order-1][index] = (uint32_t) address;

    return 0;
//
}
void free(int32_t address){}
int32_t power(int32_t a,int32_t b){
    int32_t count = 1;
    int32_t result = a;
    for(count = 1; count<b;count++){
        result = result * a;
    }
    return result;

}

// 200   1800








