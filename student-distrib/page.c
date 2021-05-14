

// #include "page_assembly.h"
#include "x86_desc.h"
#include "page.h"
#include "lib.h"

void PAGE_ON();
PDE PD[entry_num];// __attribute__((aligned (four_k)));
PTE PT[entry_num];
PTE PT_for_video[entry_num];
PTE PT_for_heap [entry_num];

/*
init_page:
initial page directory entry for the first 4MB with kb form and second 4MB with MB form for kernel
initial page table which contain vedio memory, and only initialize the entry contain vedio memory, it should locate at the first 4 mb PDE
input: None
output: None
*/
void init_page(void){
    printf("Init PAGE...\n");
    int i=0;
    //fill first 4MB to be KB
    PD[0].k.p=1; // present set to present
    PD[0].k.r_w=1;// enable write and read
    PD[0].k.u_s=0; //set supervisor priviledge level
    PD[0].k.pwt=0; // make write cache enable
    PD[0].k.pcd=0; // make page table to be cached
    PD[0].k.a=0; //initially set to 0, wait user to visit for first time
    PD[0].k.reserved=0; //basically set to 0
    PD[0].k.ps=0; // clear for page size 4kb
    PD[0].k.g=0;//not frequently use 
    PD[0].k.avail=0; //all 32 bits are available to software
    PD[0].k.ptb_add=(int) PT>>shift_twelve; //get the most significant 20 bit
    //finish fill first 4MB
    
    //for the second 4MB to set MB for kernel
    PD[1].M.p=1 ;// present set to present
    PD[1].M.r_w=1;// enable write and read
    PD[1].M.u_s=0; //set supervisor priviledge level
    PD[1].M.pwt=0; // make write cache enable
    PD[1].M.pcd=0;// make page table to be cached
    PD[1].M.a=0; //initially set to 0, wait user to visit for first time
    PD[1].M.d=0; //clear when point to directory table
    PD[1].M.ps=1;// set for page size 4M
    PD[1].M.g=1;//frequently use, global page
    PD[1].M.avail=0;//all 32 bits are available to software
    PD[1].M.pat=0; //set 0 fro processor not support
    PD[1].M.reserved=0;//basically set to 0
    PD[1].M.ptb_add=1; // x400000>>22 for the ten significant bit
     //finish fill second 4MB

    for(i=0;i<entry_num;i++){
        if (i!=0 && i!=1){
            PD[i].k.p=0; // present set to present for K M share the same add
        }
    }

    //initialize the page table entry
    for(i=0;i<entry_num;i++){
        if (i==video_memory>>shift_twelve){
            PT[i].p=1;// present set to present
            PT[i].r_w=1;//// enable write and read
            PT[i].u_s=0; //set supervisor priviledge level
            PT[i].pwt=0; // make write cache enable
            PT[i].pcd=0;// make page table to be cached
            PT[i].a=0; //initially set to 0, wait user to visit for first time
            PT[i].d=1; //set when point to page 
            PT[i].pat=0;//set 0 fro processor not support
            PT[i].g=0;//frequently use, global page
            PT[i].avail=0;//all 32 bits are available to software
            PT[i].ptb_add=video_memory>>shift_twelve;
        }
        else if (i==video_buffer_terminal_1>>shift_twelve){
            PT[i].p=1;// present set to present
            PT[i].r_w=1;//// enable write and read
            PT[i].u_s=1; //set supervisor priviledge level
            PT[i].pwt=0; // make write cache enable
            PT[i].pcd=0;// make page table to be cached
            PT[i].a=0; //initially set to 0, wait user to visit for first time
            PT[i].d=1; //set when point to page 
            PT[i].pat=0;//set 0 fro processor not support
            PT[i].g=0;//frequently use, global page
            PT[i].avail=0;//all 32 bits are available to software
            PT[i].ptb_add=video_buffer_terminal_1>>shift_twelve;
        }
        
        else if (i==video_buffer_terminal_2>>shift_twelve){
            PT[i].p=1;// present set to present
            PT[i].r_w=1;//// enable write and read
            PT[i].u_s=1; //set supervisor priviledge level
            PT[i].pwt=0; // make write cache enable
            PT[i].pcd=0;// make page table to be cached
            PT[i].a=0; //initially set to 0, wait user to visit for first time
            PT[i].d=1; //set when point to page 
            PT[i].pat=0;//set 0 fro processor not support
            PT[i].g=0;//frequently use, global page
            PT[i].avail=0;//all 32 bits are available to software
            PT[i].ptb_add=video_buffer_terminal_2>>shift_twelve;
        }
        else if (i==video_buffer_terminal_3>>shift_twelve){
            PT[i].p=1;// present set to present
            PT[i].r_w=1;//// enable write and read
            PT[i].u_s=0; //set supervisor priviledge level
            PT[i].pwt=0; // make write cache enable
            PT[i].pcd=0;// make page table to be cached
            PT[i].a=0; //initially set to 0, wait user to visit for first time
            PT[i].d=1; //set when point to page 
            PT[i].pat=0;//set 0 fro processor not support
            PT[i].g=0;//frequently use, global page
            PT[i].avail=0;//all 32 bits are available to software
            PT[i].ptb_add=video_buffer_terminal_3>>shift_twelve;
        }
        else if (i==backdoor>>shift_twelve){
            PT[i].p=1;// present set to present
            PT[i].r_w=1;//// enable write and read
            PT[i].u_s=1; //set supervisor priviledge level
            PT[i].pwt=0; // make write cache enable
            PT[i].pcd=0;// make page table to be cached
            PT[i].a=0; //initially set to 0, wait user to visit for first time
            PT[i].d=1; //set when point to page 
            PT[i].pat=0;//set 0 fro processor not support
            PT[i].g=0;//frequently use, global page
            PT[i].avail=0;//all 32 bits are available to software
            PT[i].ptb_add=video_memory>>shift_twelve;
        }

        else{
            PT[i].p=0;
        }
    }
    PAGE_ON();


}
/*
PAGE_ON
*Enabling paging is actually very simple. All that is needed is to load CR3 with the address 
*of the page directory and to set the paging (PG) and protection (PE) bits of CR0. Note: setting
*the paging flag when the protection flag is clear causes a general-protection exception.
*/

void PAGE_ON(){
    asm volatile(
	"movl %0, %%eax             ;"
	"movl %%eax, %%cr3          ;"

	"movl %%cr4, %%eax          ;"
	"orl $0x00000010, %%eax     ;"
	"movl %%eax, %%cr4          ;"

	"movl %%cr0, %%eax          ;"
	"orl $0x80000000, %%eax     ;"
	"movl %%eax, %%cr0          ;"

	:  : "r"(PD): "eax" );
}



