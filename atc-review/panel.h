#ifndef NEBULAR_UNIFIED_PANEL_H
#define NEBULAR_UNIFIED_PANEL_H
#include <stdint.h>
#define NEBULAR_WAVEFORM_SIZE 535u
enum nebular_pixel { NEBULAR_ORANGE=1, NEBULAR_WHITE=5, NEBULAR_YELLOW=9, NEBULAR_RED=13, NEBULAR_BLACK=4, NEBULAR_GRAY=6, NEBULAR_DARK_GRAY=7 };
enum nebular_state { NEBULAR_IDLE, NEBULAR_RESET_WAIT, NEBULAR_POWER_WAIT, NEBULAR_REFRESH_WAIT, NEBULAR_OFF_WAIT, NEBULAR_DONE, NEBULAR_ERROR };
struct nebular_io {
 void (*power)(void *,uint8_t); void (*reset)(void *,uint8_t);
 void (*command)(void *,uint8_t); void (*data)(void *,uint8_t);
 void (*delay_ms)(void *,unsigned); uint8_t (*ready)(void *);
 uint32_t (*millis)(void *); uint8_t (*pixel)(void *,uint32_t);
 uint8_t (*wave)(void *,unsigned,unsigned);
};
struct nebular_panel {
 const struct nebular_io *io; void *context;
 uint32_t since; enum nebular_state state;
 uint16_t width,height; uint8_t component,model;
};
uint8_t nebular_pack4(const uint8_t [4],unsigned);
int nebular_start(struct nebular_panel *,const struct nebular_io *,void *,unsigned);
enum nebular_state nebular_poll(struct nebular_panel *);
void nebular_abort(struct nebular_panel *);
#endif
