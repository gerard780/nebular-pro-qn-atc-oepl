#include <stdint.h>
extern uint8_t host_pb,host_pd,host_pa,host_temp,host_config[176];
extern uint32_t host_tick,host_outer;
void host_flash(uint32_t,uint32_t,void *);
void host_start(unsigned);
void host_end(void);
void host_log(const char *);
void host_command(unsigned);
void host_data(unsigned);
void host_delay(unsigned);
#define FLASH_READ host_flash
#define ATC_START host_start
#define ATC_END host_end
#define LOG host_log
#define COMMAND host_command
#define DATA host_data
#define DELAY_US host_delay
#define DIVIDE(a,b) ((a)/(b))
#define PB_OUT host_pb
#define PD_OUT host_pd
#define PA_IN host_pa
#define TICK host_tick
#define OUTER_TIMER host_outer
#define TEMPERATURE host_temp
#define CONFIG host_config
void host_load(void); void host_set_panel(unsigned); void host_save(void);
