
#ifndef RTC__
#define RTC__

#include    "types.h"

#define RTC_register_number_port 0x70
#define RTC_register_W_R_port    0x71
#define RTC_register_A           0x8A // with disable_NMI
#define RTC_register_B           0x8B // with disable_NMI
#define RTC_register_C           0x8C // with disable_NMI
#define init_num                 0x40
#define rtc_irq_number           8
#define RTC_register_C_with_out_disable_NMI 0x0C

/*Version 2 ML*/
#define SUCCESS                  0
#define NOT_SUCCESS              -1
#define RTC_RAM                  0x20
#define TWO_POWER_FIFTEEN        32768
#define THE_F                    0x0F
#define REVERSE_F                0xF0
#define two_HZ                   2
#define max_HZ                   1024


void init_rtc();
void __rtc_interrupt_handler__();
int32_t freq_2_rate(uint32_t freq);
void set_rtc_rate(uint32_t rate);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);
int32_t rtc_read(int32_t fd, void* buf, uint32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);


#endif



