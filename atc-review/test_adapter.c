#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define NEBULAR_ADAPTER_HOST
#include "adapter.c"
uint8_t host_pb,host_pd,host_pa=2,host_temp=24,host_config[176];
uint32_t host_tick=0xffff0000u,host_outer;
static uint8_t flash[0x80000],saved[176],expected_wave[2][535];
static unsigned starts,ends,saves,frames,frame_bytes,cmd,n,errors,model,raw_two;
static uint8_t sent[12000];
void host_flash(uint32_t a,uint32_t count,void *p) {
 assert((a==0x7e000 && count==92) || (a>=0x40000 && a+count<=0x73000));
 memcpy(p,flash+a,count);
}
void host_load(void) { memcpy(host_config,saved,176); }
void host_set_panel(unsigned preset) {
 assert(preset==0 || preset==17 || preset==38);
 host_config[12]=preset==17 ? 0x72 : preset==38 ? 0x73 : 0xfa; host_config[13]=0;
 host_config[15]=preset;host_config[16]=0;host_config[17]=preset ? 5 : 0;host_config[18]=0;
 host_config[22]=preset==17 ? 40 : preset==38 ? 200 : 0;host_config[23]=preset==17;
 host_config[24]=preset==17 ? 152 : preset==38 ? 200 : 0;host_config[25]=0;
}
void host_save(void) { memcpy(saved,host_config,176); ++saves; }
void host_start(unsigned x) { assert(x==1);++starts; }
void host_end(void) { ++ends; adapter_refresh(); }
void host_log(const char *s) { (void)s;++errors; }
static void finish_command(void) {
 unsigned phase=(frames ? frames-1 : 0)%2;
 if(cmd==0x20) { assert(n==535);assert(memcmp(sent,expected_wave[phase],535)==0); }
 if(cmd==0x61) { const uint8_t a[4]={0,152,1,40},b[4]={0,200,0,200};assert(n==4 && memcmp(sent,model?b:a,4)==0); }
 if(cmd==0x06) { const uint8_t a[7]={15,10,47,37,34,46,33},b[7]={15,10,47,37,34,43,33};assert(n==7 && memcmp(sent,model?b:a,7)==0); }
 if(cmd==0xf0) assert(n==1 && sent[0]==(model ? 0x7d : 0x5f));
 if(cmd==0x10) {
  unsigned p;
  uint8_t expected=phase ? (raw_two ? 0x45 : 0x44) : (raw_two ? 0x5e : 0x55);
  assert(n==(model ? 10000u : 11248u));
  for(p=0;p<n;++p) assert(sent[p]==expected);
  frame_bytes+=n;
 }
}
void host_command(unsigned c) { finish_command();cmd=c;n=0;if(c==0x20) ++frames; }
void host_data(unsigned d) { assert(n<sizeof sent);sent[n++]=(uint8_t)d; }
void host_delay(unsigned us) { host_tick+=us*16; }
static void fixture(unsigned selected,unsigned two) {
 unsigned size;
 model=selected;raw_two=two;
 memcpy(flash+0x7e000,product_records[model],92);
 host_set_panel(model ? 38 : 17);
 size=(model ? 5000 : 5624)*(two+1);
 memset(flash+0x40000,0,21);memcpy(flash+0x4000c,&size,4);flash[0x40010]=two ? 0x21 : 0x20;
 memset(flash+0x40015,0x55,size/(two+1));
 memset(flash+0x40015+size/(two+1),0x33,size/(two+1));
 memset(S,0,sizeof(*S));cmd=0xff;n=0;frames=0;frame_bytes=0;host_pa=2;
}
static void load_expected(unsigned selected,unsigned band,const char *directory) {
 char name[1024];unsigned phase;
 (void)selected;
 for(phase=0;phase<2;++phase) {
  FILE *f;snprintf(name,sizeof name,"%s/band-%02u-mode-%u.bin",directory,band,phase+1);
  f=fopen(name,"rb");assert(f);assert(fread(expected_wave[phase],1,535,f)==535);fclose(f);
 }
}
int main(int argc,char **argv) {
 static const int temps[10]={3,6,9,12,15,20,25,30,35,36};
 unsigned m,band,two,i,old,old_saves;
 assert(argc==3);
 /* Exact factory record recognition and early settings selection. */
 for(m=0;m<3;++m) {
  unsigned selected=product_models[m];
  memcpy(flash+0x7e000,product_records[m],92);assert(detect_model()==selected);
  for(i=0;i<176;++i) saved[i]=(uint8_t)(i*7+3);
  old_saves=saves;nebular_boot();assert(saves==old_saves+1 && saved[12]==(selected?0x73:0x72));
  for(i=47;i<102;++i) assert(saved[i]==(uint8_t)(i*7+3));
  nebular_boot();assert(saves==old_saves+1);
  flash[0x7e02f]^=1;assert(detect_model()==2);nebular_boot();assert(saved[12]==0xfa && saved[17]==0);
 }
 for(m=0;m<2;++m) for(band=0;band<10;++band) for(two=0;two<2;++two) {
  fixture(m,two);host_temp=(uint8_t)temps[band];load_expected(m,band,argv[m+1]);
  old=starts;adapter_image(0x40000,0);assert(starts==old+1 && S->band==band && S->model==m);
  adapter_image(0x40000,0);assert(starts==old+1);
  for(i=0;i<20;++i) { host_tick+=320000;if(adapter_ready()) break; }
  finish_command();assert(i<20 && S->panel.state==NEBULAR_DONE && !S->failed && frames==2);
  assert(frame_bytes==(m ? 20000u : 22496u));assert(host_pb&0x20);
 }
 /* Temperature edges use signed values and inclusive upper bounds. */
 for(m=0;m<2;++m) {
  fixture(m,1);host_temp=128;old=starts;adapter_image(0x40000,0);assert(starts==old);
  host_temp=129;adapter_image(0x40000,0);assert(starts==old+1 && S->band==0);
  fixture(m,1);host_temp=127;adapter_image(0x40000,0);assert(S->band==9);
  for(i=0;i<9;++i) { fixture(m,1);host_temp=(uint8_t)(temps[i]+1);adapter_image(0x40000,0);assert(S->band==i+1); }
 }
 /* Every source pixel of an asymmetric image maps to the correct native row. */
 for(m=0;m<2;++m) {
  unsigned pixels=m ? 40000 : 44992,width=m ? 200 : 152,height=m ? 200 : 296,plane=pixels/8;
  fixture(m,1);S->model=m;S->address=0x40015;S->two_planes=1;S->cache_index=~0u;
  for(i=0;i<plane;++i) { flash[0x40015+i]=(uint8_t)(i*37+(i>>3));flash[0x40015+plane+i]=(uint8_t)(i*71+(i>>5)); }
  for(i=0;i<pixels;++i) {
   unsigned mapped=(height-1-i/width)*width+i%width,mask=0x80u>>(mapped&7);
   static const unsigned codes[4]={5,4,13,9};
   unsigned color=!!(flash[0x40015+mapped/8]&mask)+2*!!(flash[0x40015+plane+mapped/8]&mask);
   assert(pixel(S,i)==codes[color]);
  }
 }
 /* Rejection paths and BUSY timeouts must not trigger or hang a refresh. */
 for(m=0;m<2;++m) {
  fixture(m,1);host_temp=24;old=starts;flash[0x40010]=0x30;adapter_image(0x40000,0);assert(starts==old);
  fixture(m,1);flash[0x4000c]=1;adapter_image(0x40000,0);assert(starts==old);
  fixture(m,1);host_config[12]^=1;adapter_image(0x40000,0);assert(starts==old);
  fixture(m,1);flash[0x7e000]^=1;adapter_image(0x40000,0);assert(starts==old);
  fixture(m,1);adapter_image(0x3ffff,0);adapter_image(0x72fff,0);assert(starts==old);
  fixture(m,1);host_pa=0;adapter_image(0x40000,0);host_tick+=16000*401;assert(adapter_ready() && S->panel.state==NEBULAR_ERROR);
 }
 S->last_tick=0xfffffff0u;S->milliseconds=100;S->remainder=0;host_tick=15984;assert(millis(S)==101);
 printf("PASS: 2 models x 10 temperature bands x RAW1/RAW2; all 40 waveforms byte-exact, 40 paired refreshes / 80 component refreshes, geometry/booster/setup, all asymmetric pixels, boot identity preservation, boundaries, unknown-model rejection, timer wrap and BUSY timeout. Host state %zu bytes.\n",sizeof(*S));
 return 0;
}
