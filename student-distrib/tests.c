


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

#include "systemfile.h"
#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "idt.h"
#include "rtc.h"
#include "terminal.h"
#include "buddy_sys.h"
#define four_kb 0
#define PASS 1
// #define FAIL 0
#define KEYBOARD_IRQ 1

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S, idt.c
 * 
 * Changed by us, add the coverage for the 0x00-0x19 for out idt except 0x15
 */


int idt_test(){
	TEST_HEADER;
	int i;
	int result = PASS;
	for (i = 0; i < 20; i++){
		if(i == 15){
			continue;
		}
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
}


/* division Test
 * 
 * Assert the exception of division will happen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S, idt.c
 * 
 */

int division_test(){
	TEST_HEADER;
	int i;
    int a=0;
    int b=4;
    for(i = 0;i<20;i++){
	b=b/a;

	}

	return FAIL; // if the exception happened, we can never get here
}

/* Any exception test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S, idt.c
 * 
 */
int test_any_excp(){
	/* change the following vector between 0-19(0x00-0x13) */ 
	/* to test each corresponding exception handlers */
	asm("int $0x08");
	return PASS;
}

/* test_i8259_disable_irq_garbage
 * Inputs: None
 * Outputs: This test should not mask any interrupts when a garbage input is given to disable_irq 
 * Side Effects: None
 * Coverage: garbage input to disable_irq
 * Files: i8259.c
 */
int test_i8259_disable_irq_garbage(){
	/*send a invalid irq_num to disable_irq, nothing should happen*/
	/*The interrupt from keyboard and rtc should still be aceepted*/
	disable_irq(19);
	disable_irq(1234);
	return PASS;
}


/* test_i8259_disable_irq
 * Inputs: None
 * Outputs: This test should mask interrupts from keyboard and rtc
 * Side Effects: None
 * Coverage: disable_irq
 * Files: i8259.c
 */
int test_i8259_disable_irq(){
	/*send irq_num of keyboard and rtc and check whether it is masked*/
	disable_irq(rtc_irq_number);
	disable_irq(KEYBOARD_IRQ);
	return PASS;
}

/* test_i8259_enable_irq_garbage
 * Inputs: None
 * Outputs: This test should not enable any interrupts when a garbage input is given to enable_irq 
 * Side Effects: None
 * Coverage: garbage input to enable_irq
 * Files: i8259.c
 */
int test_i8259_enable_irq_garbage(){
	/*send garbage input to enable_irq nothing should happend*/
	enable_irq(19);
	enable_irq(1234);
	return PASS;
}

/* test_i8259_enable_irq
 * Inputs: None
 * Outputs: This test should enable interrupts from keyboard and rtc
 * Side Effects: None
 * Coverage: enable_irq
 * Files: i8259.c
 */
int test_i8259_enable_irq(){
	/*it should enable the interrupt from the keyboard and rtc after disable_irq is called*/
	enable_irq(rtc_irq_number);
	enable_irq(KEYBOARD_IRQ);
	return PASS;
}





/* page_test
 * Inputs: None
 * Outputs: This test should check if page mechanism is work or not.
 * Side Effects: None
 * Files: page.c
 */
int page_test(){
	TEST_HEADER;
	int i;
	int result = PASS;

	/* test margin of page */
	int test;
	int* ptr = (int*)0x400000;	/* the margin of PD */
	ptr[0] = 2;//just a random number
	test = ptr[0];
    int index= 0xB8000 >> 12;//  0xB8000 >> 12 is ithe index of page table 

	
	//check if the first two exist 
	if (PD[0].k.p == 0 || PD[1].M.p == 0 ){
		result = FAIL;
	}
	//check if the other present at the video memory
	for(i = 0; i < 1024; i++){// 1024 is the entry number
		if(PD[i].M.p == 1 && i > 1){
			result = FAIL;
		}
		if(PT[i].p == 1 && i !=index ){		
			result = FAIL;
		}
		if (PT[i].p == 0 && i == index){ 		 
			result = FAIL;
		} 
	}
	return result;
}




/* Checkpoint 2 tests */
/* rtc_freq_test
 * Inputs: None
 * Outputs: This test should change the freq of the rtc and print "1" on the screen at different freqs
 * Side Effects: Change the freq of the rtc
 * Files: rtc.c
 */
// void rtc_freq_test(){
// 	int i; //loop index
// 	int freq = 2;
// 	int fd = rtc_open(NULL);
// 	for (freq=2; freq<1024; freq*=2){
// 		clear();
// 		printf("freq changes! freq is: %d \n", freq);
// 		rtc_write(fd,(void *)freq,4);
// 		for(i=0; i<freq+1; i++){
// 			rtc_read(fd,(void *)freq,4);
// 			printf("1");
// 		}
// 	}
// }

// int terminal_test()
// {
// 	TEST_HEADER;
// 	uint8_t buf[128];
// 	int32_t num_char;
// 	while(1){
// 		terminal_write(1, (uint8_t *)"Hi, what's your name? ", 22);
// 		num_char = terminal_read(0, buf, 128);
// 		terminal_write(1, (uint8_t *)"Hello, ", 7);
// 		terminal_write(1, buf, num_char);
// 	}
// 	return PASS;
// }

// int read_dentry_by_index_and_read_data_test(){
// 	clear();
// 	dentry dentry_buffer;
// 	int32_t is_fail;
// 	uint32_t size=50000; 
// 	uint8_t buf[size];
// 	int32_t num_read;
// 	int i;
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 	//test function:read_dentry_by_index
// 	is_fail = read_dentry_by_index(10, &dentry_buffer);// index 10 is: frame0.txt
// 	if(is_fail == -1){
// 		return FAIL;
// 	}
// 	printf("File name: ");
// 	for(i=0; i<FILENAME_LEN; i++){ //filename may not have '\0'. DO NOT USE puts.
// 		putc(dentry_buffer.filename[i]);
// 	}
// 	printf("\n");
// 	printf("File type: %d\n", dentry_buffer.filetype);
// 	printf("Inode #: %d\n", dentry_buffer.inode_num);
// 	printf("\nend of read_dentry_by_index test\n");
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 	//test function：read_data. print the content of the file
// 	num_read = read_data(dentry_buffer.inode_num, 0, buf, size);
	
// 	for(i=0; i<num_read; i++){
// 		if(buf[i]!='\0'){
// 			putc(buf[i]);
// 		}
// 	}
// 	printf("\n");
// 	printf("File size : %d\n",num_read);
// 	// printf("File size: %d\n", (int32_t)num_read);
// 	// if(num_read>4096){//file too large. 
// 	// 	printf("print first 500 chars:\n");
// 	// 	for(i=0; i<500; i++){
// 	// 		putc(buf[i]);
// 	// 	}		
// 	// }
// 	// else{
// 	// 	for(i=0; i<num_read; i++){
// 	// 		putc(buf[i]);
// 	// 	}	
// 	// }

// 	printf("\nend of read_data\n");
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 	return PASS;
// }




/* 

 *Filesystem test 
 * 
 * Description: test all function related to Filesystem, if any of them fails, will return FAIL.
 * Inputs: NONE
 * Outputs: None
 * Side Effects: PASS for success, FAIL for failure
 * Files: filesys.c/filesys.h
 */
// int read_dentry_by_name_and_read_data_test(){
// 	clear();
// 	dentry dentry_buffer;
// 	int32_t is_fail;
// 	uint32_t size=50000; 
// 	uint8_t buf[size];
// 	int32_t num_read;
// 	int i;//used as index


// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 	//test function：read_dentry_by_name. print dentry.
// 	is_fail = read_dentry_by_name((uint8_t*)"fish", &dentry_buffer);	  //change filename here
// 	/*
// 	LIST:  
// 		"frame0.txt"
// 		"verylargetextwithverylongname.txt"
	
	
// 	*/
// 	if(is_fail == -1){
// 		return FAIL;
// 	}
	
// 	printf("File name: ");
// 	for(i=0; i<FILENAME_LEN; i++){ //filename may not have '\0'. DO NOT USE puts.
// 		putc(dentry_buffer.filename[i]);
// 	}
// 	printf("\n");
// 	printf("File type: %d\n", dentry_buffer.filetype);
// 	printf("Inode #: %d\n", dentry_buffer.inode_num);
// 	printf("\nend of read_dentry_by_name test\n");
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 	//test function：read_data. print the content of the file
// 	num_read = read_data(dentry_buffer.inode_num, 0, buf, size);
// 	//printf("File size : %d\n",num_read);
// 	for(i=0; i<num_read; i++){
// 		if(buf[i]!='\0'){
// 			putc(buf[i]);
// 		}
// 	}	
// 	printf("\n");
// 	printf("File size : %d\n",num_read);
// 	printf("\nend of read_data\n");
// 	// printf("File size: %d\n", (int32_t)num_read);
// 	// if(num_read>4096){//file too large. 
// 	// 	printf("print first 500 chars:\n");
// 	// 	for(i=0; i<500; i++){
// 	// 		if (buf[i]!='\0'){
// 	// 			putc(buf[i]);
// 	// 		}
		
// 	// 	}		
// 	// }
// 	// else{
// 	// 	for(i=0; i<num_read; i++){
// 	// 		putc(buf[i]);
// 	// 	}	
// 	// }

// 	// printf("\nend of read_data\n");
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// return PASS;
// }





// int list_all_files(){



// 	clear();
// 	int32_t fd=0;//we do not use fd for cp2.
// 	int32_t nbytes=0;//we do not need nbytes for sysdir_read.
// 	int offset;//offset of dentry
// 	int i;//index
// 	uint8_t name_buffer[FILENAME_LEN+1];


// 	dentry dentry_buffer;
// 	int32_t is_fail;


// 	for(offset=0;offset<17;offset++){//we have 17 files.
// 		sysdir_read( fd,  offset, name_buffer,  nbytes);
// 		printf("File name:");
// 		for(i=0; i<FILENAME_LEN; i++){ //filename may not have '\0'. DO NOT USE puts.
// 			putc(name_buffer[i]);
// 		}


// 		is_fail = read_dentry_by_index(offset, &dentry_buffer);// index 1 is: sigtest
// 		if(is_fail == -1){
// 			return FAIL;
// 		}
// 		printf(" File type:%d ", dentry_buffer.filetype);
// 		printf("  File size:%d ", (inode*)(inode_men_start+dentry_buffer.inode_num)->length);
// 		printf("\n");
	

// 	}
	
// 	return PASS;
// }

// int test_file_read(){
	
// 	uint32_t size=50000; 
// 	uint8_t buf[size];
// 	int32_t num_read;
// 	uint32_t off=0;
// 	char* txt="verylargetextwithverylongname.txt";
// 	int i;//index
// 	clear();
// 	num_read=sysfile_read((int32_t)txt,off,(void*)buf, size);

// 	//printf("File size : %d\n",num_read);
// 	for(i=0; i<num_read; i++){
// 		if(buf[i]!='\0'){
// 			putc(buf[i]);
// 		}
// 	}	
// 	printf("\n");
// 	printf("File size : %d\n",num_read);
// 	printf("\nend of read_data\n");
// 	return PASS;

// }


/*check function of file open
input: filename
*output: 0 for vaild filename, -1 otherwise
*/
// int test_file_open()
// {
// 	char* txt="frame0.txt";
// 	if (sysfile_open((const uint8_t*) txt)==0){
// 		return PASS;
// 	}
// 	return FAIL;
// }

/*test_file_write
* input: None
output: 0 for pass
*function test for file write, for read-only file, sysfile_write should return false*/
// int test_file_write()
// {
// 	int32_t fd=0;
// 	uint8_t buf[1];
// 	int32_t nbytes=0;
// 	if(sysfile_write(fd,  buf, nbytes)==FAIL){
// 		return PASS;
// 	}
// 	else{
// 		return FAIL;
// 	}
// }
/*
*test_file_close
*test file close
*input: none
*output: none
*/
// int test_file_close(){
// 	int32_t fd=0;
// 	if(sysfile_close(fd)==PASS){
// 		return PASS;
// 	}
// 	return FAIL;
// }

// int test_dir_open_write_close(){
// 	int32_t fd=0;
// 	uint8_t buf[1];
// 	int32_t nbytes=0;
// 	if(sysdir_open((const uint8_t *)"frame0.txt")==0 && sysdir_write(fd, buf, nbytes)==FAIL && sysdir_close(fd)==PASS){
// 		return PASS;
// 	}
// 	return FAIL;
// }
int32_t malloc(uint32_t order){
	int32_t current =mem_allco(order);
	if(current == -1){
		printf("fail to malloc " );
		return FAIL;
	}else{
		demo_show_linked_list();
	}
	return PASS;
}
/* Test suite entry point */
void launch_tests(){
	int32_t address = malloc(four_kb);//malloc 4kb 
	free(address);
	
	// TEST_OUTPUT("idt_test", idt_test());
    
	// TEST_OUTPUT("disable_rtc_and_keyboard",test_i8259_disable_irq());

	// TEST_OUTPUT("enable_rtc_and_keyboard",test_i8259_enable_irq());

	// TEST_OUTPUT("division_test", division_test());

	// TEST_OUTPUT("any exception",test_any_excp());
	
	// TEST_OUTPUT("disable_garbage",test_i8259_disable_irq_garbage());
	
	// TEST_OUTPUT("enable_garbage",test_i8259_enable_irq_garbage());
    
	// TEST_OUTPUT("page test", page_test());

	//TEST_OUTPUT("file system test0",read_dentry_by_name_and_read_data_test());
	//TEST_OUTPUT("file system test1",read_dentry_by_index_and_read_data_test());
	//TEST_OUTPUT("list all files",list_all_files());
	//TEST_OUTPUT("test file read",test_file_read());
	//TEST_OUTPUT("test file open",test_file_open());
	//TEST_OUTPUT("test file write",test_file_write());
	//TEST_OUTPUT("test file close",test_file_close());
	//TEST_OUTPUT("test dir open write close",test_dir_open_write_close());
	


	// rtc_freq_test();
	// terminal_test();



	
/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

}
