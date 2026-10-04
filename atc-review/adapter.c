/* One adapter for the two captured revisions. All 40 factory waveforms are
 * embedded in a bounded 128-byte-window stream format. RAW1/RAW2 only.
 * State remains in ATC's existing 300-byte, JD-unused custom LUT buffer.
 */
#include "panel.h"
#ifdef NEBULAR_ADAPTER_HOST
#include "host_io.h"
#define ADC_TEMPERATURE TEMPERATURE
#define LOAD host_load
#define SET_PANEL host_set_panel
#define SAVE host_save
#else
#define FLASH_READ ((void (*)(uint32_t,uint32_t,void *))0x11ee9)
#define ATC_START ((void (*)(unsigned))0x5b9d)
#define ATC_END ((void (*)(void))0x5d25)
#define LOG ((void (*)(const char *))0x1157d)
#define COMMAND ((void (*)(unsigned))0x7d2d)
#define DATA ((void (*)(unsigned))0x7d7d)
#define DELAY_US ((void (*)(unsigned))0x1051)
#define DIVIDE ((uint32_t (*)(uint32_t,uint32_t))0x2ead)
#define PB_OUT (*(volatile uint8_t *)0x80058b)
#define PD_OUT (*(volatile uint8_t *)0x80059b)
#define PA_IN (*(volatile uint8_t *)0x800580)
#define TICK (*(volatile uint32_t *)0x800740)
#define OUTER_TIMER (*(volatile uint32_t *)0x844658)
#define ADC_TEMPERATURE (*(volatile int8_t *)0x8449bc)
#define CONFIG ((volatile uint8_t *)0x8439d0)
#define LOAD ((void (*)(void))0xd84d)
#define SET_PANEL ((void (*)(unsigned))0xd1f9)
#define SAVE ((void (*)(void))0xd7ed)
#endif
#define MAGIC 0x4e42554cu
struct wave_record { uint8_t header[7]; uint16_t offset,length; };
#include "waves.inc"
#include "products.inc"
struct adapter_state {
 uint32_t magic,address,cache_index,last_tick,remainder,milliseconds;
 struct nebular_panel panel;
 uint8_t cache[2][32],history[128];
 const uint8_t *next,*end;
 uint16_t produced;
 uint8_t remaining,distance,two_planes,band,model,failed;
};
#ifndef NEBULAR_ADAPTER_HOST
typedef char state_must_fit_lut[(sizeof(struct adapter_state)<=300)?1:-1];
#define S ((struct adapter_state *)0x844660)
#else
static struct adapter_state host_state;
#define S (&host_state)
#endif
unsigned detect_model(void) {
 uint8_t record[92]; unsigned model,i;
 FLASH_READ(0x7e000,92,record);
 for(model=0;model<sizeof(product_records)/sizeof(product_records[0]);++model) {
  for(i=0;i<92 && record[i]==product_records[model][i];++i) {}
  if(i==92) return product_models[model];
 }
 return 2;
}
void nebular_boot(void) {
 uint8_t before[175]; unsigned i,model;
 LOAD(); model=detect_model();
 for(i=0;i<175;++i) before[i]=CONFIG[i];
 SET_PANEL(model==0 ? 17 : model==1 ? 38 : 0);
 for(i=0;i<175;++i) if(before[i]!=CONFIG[i]) { SAVE(); break; }
}
static void power(void *ctx,uint8_t on) { (void)ctx; if(on) PB_OUT&=(uint8_t)~0x20; else PB_OUT|=0x20; }
static void reset(void *ctx,uint8_t high) { (void)ctx; if(high) PD_OUT|=0x10; else PD_OUT&=(uint8_t)~0x10; }
static void command(void *ctx,uint8_t c) { (void)ctx; COMMAND(c); }
static void data(void *ctx,uint8_t d) { (void)ctx; DATA(d); }
static void delay(void *ctx,unsigned ms) { (void)ctx; DELAY_US(ms*1000); }
static uint8_t ready(void *ctx) { (void)ctx; return !!(PA_IN&2); }
static uint32_t millis(void *ctx) {
 struct adapter_state *s=ctx; uint32_t tick=TICK,total=(tick-s->last_tick)+s->remainder;
 uint32_t ms=DIVIDE(total,16000);
 s->last_tick=tick; s->remainder=total-ms*16000; s->milliseconds+=ms; return s->milliseconds;
}
static uint8_t wave(void *ctx,unsigned component,unsigned index) {
 struct adapter_state *s=ctx;
 const struct wave_record *r=&wave_records[(s->model*10+s->band)*2+component];
 uint8_t value;
 if(index<7) return r->header[index];
 if(index==7) { s->next=wave_bytes+r->offset; s->end=s->next+r->length; s->produced=0; s->remaining=0; }
 if(s->failed || index!=(unsigned)s->produced+7) { s->failed=1; return 0; }
 if(!s->remaining) {
  uint8_t token;
  if(s->next>=s->end) { s->failed=1; return 0; }
  token=*s->next++;
  s->remaining=(token&127)+(token>=128 ? 3 : 1); s->distance=0;
  if(token>=128) {
   if(s->next>=s->end) { s->failed=1; return 0; }
   s->distance=*s->next++;
   if(!s->distance || s->distance>128 || s->distance>s->produced) { s->failed=1; return 0; }
  }
 }
 if(s->distance) value=s->history[(s->produced-s->distance)&127];
 else {
  if(s->next>=s->end) { s->failed=1; return 0; }
  value=*s->next++;
 }
 s->history[s->produced&127]=value; ++s->produced; --s->remaining;
 if(index==534 && (s->remaining || s->next!=s->end)) s->failed=1;
 return value;
}
static uint32_t plane_size(struct adapter_state *s) { return s->model ? 5000 : 5624; }
__attribute__((noinline)) static uint32_t source_byte(struct adapter_state *s,uint32_t byte) {
 unsigned row=s->model ? 25 : 19, height=s->model ? 200 : 296;
 return byte+(height-1)*row-2*row*DIVIDE(byte,row);
}
static uint8_t pixel(void *ctx,uint32_t index) {
 struct adapter_state *s=ctx; uint32_t byte=source_byte(s,index>>3),block=byte&~31u,count;
 unsigned mask=0x80u>>(index&7);
 static const uint8_t codes[4]={5,4,13,9};
 if(s->cache_index!=block) {
  count=plane_size(s)-block; if(count>32) count=32;
  FLASH_READ(s->address+block,count,s->cache[0]);
  if(s->two_planes) FLASH_READ(s->address+plane_size(s)+block,count,s->cache[1]);
  s->cache_index=block;
 }
 return codes[(!!(s->cache[0][byte&31]&mask)) | ((s->two_planes && (s->cache[1][byte&31]&mask)) ? 2 : 0)];
}
static const struct nebular_io io={power,reset,command,data,delay,ready,millis,pixel,wave};
unsigned adapter_init(unsigned unused) { (void)unused; return (uint8_t)ADC_TEMPERATURE; }
void adapter_sleep(void) { power(0,0); }
void adapter_refresh(void) { if(S->magic==MAGIC) nebular_start(&S->panel,&io,S,S->model); }
unsigned adapter_ready(void) {
 uint8_t component; enum nebular_state result;
 if(S->magic!=MAGIC) return 1;
 component=S->panel.component; result=nebular_poll(&S->panel);
 if(S->failed) { nebular_abort(&S->panel); result=NEBULAR_ERROR; }
 if(S->panel.component!=component) OUTER_TIMER=TICK;
 if(result==NEBULAR_ERROR) LOG("panel error\r\n");
 return result==NEBULAR_DONE || result==NEBULAR_ERROR;
}
void adapter_image(uint32_t address,unsigned unused) {
 uint8_t header[21]; unsigned i,model,two,band=0; uint32_t size,plane;
 int temperature=(int8_t)ADC_TEMPERATURE;
 volatile uint8_t *cfg=CONFIG;
 (void)unused;
 if(S->magic==MAGIC && S->panel.state!=NEBULAR_IDLE && S->panel.state!=NEBULAR_DONE && S->panel.state!=NEBULAR_ERROR) { LOG("busy\r\n"); return; }
 model=detect_model(); if(model>1) { LOG("model\r\n"); return; }
 if(address<0x40000 || address>0x73000-21) return;
 FLASH_READ(address,21,header);
 if(header[16]!=0x20 && header[16]!=0x21) { LOG("type\r\n"); return; }
 if(temperature<=-128 || temperature>127) return;
 while(band<9 && temperature>temperature_upper[band]) ++band;
 two=header[16]==0x21; plane=model ? 5000 : 5624;
 size=(uint32_t)header[12]|((uint32_t)header[13]<<8)|((uint32_t)header[14]<<16)|((uint32_t)header[15]<<24);
 if(size!=plane*(two+1) || address+21+size>0x73000) return;
 if(cfg[17]!=5 || cfg[18]!=0 || cfg[12]!=(model ? 0x73 : 0x72) || cfg[13]!=0 ||
    cfg[22]!=(model ? 200 : 40) || cfg[23]!=(model ? 0 : 1) || cfg[24]!=(model ? 200 : 152) || cfg[25]!=0) { LOG("geometry\r\n"); return; }
 for(i=0;i<sizeof(*S);++i) ((uint8_t *)S)[i]=0;
 S->magic=MAGIC; S->address=address+21; S->two_planes=two; S->band=band; S->model=model;
 S->cache_index=~0u; S->last_tick=TICK;
 ATC_START(1); ATC_END();
}
