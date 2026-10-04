#include "panel.h"

uint8_t nebular_pack4(const uint8_t pixels[4], unsigned component) {
    unsigned i, result=0, shift=component ? 0 : 2;
    for (i=0; i<4; ++i) result=(result<<2)|((pixels[i]>>shift)&3);
    return (uint8_t)result;
}

static void command(struct nebular_panel *p, uint8_t c) {
    p->io->command(p->context,c);
}
static void data(struct nebular_panel *p, uint8_t d) {
    p->io->data(p->context,d);
}
static void reg(struct nebular_panel *p, uint8_t c, uint8_t d) {
    command(p,c); data(p,d);
}
static void state(struct nebular_panel *p, enum nebular_state s) {
    p->state=s; p->since=p->io->millis(p->context);
}

void nebular_abort(struct nebular_panel *p) {
    if (p && p->io) p->io->power(p->context,0);
    if (p) p->state=NEBULAR_ERROR;
}

static void reset_component(struct nebular_panel *p) {
    const struct nebular_io *io=p->io;
    void *ctx=p->context;
    io->power(ctx,1); io->delay_ms(ctx,5);
    io->reset(ctx,1); io->delay_ms(ctx,2);
    io->reset(ctx,0); io->delay_ms(ctx,20);
    io->reset(ctx,1); io->delay_ms(ctx,2);
    io->reset(ctx,0); io->delay_ms(ctx,20);
    io->reset(ctx,1); io->delay_ms(ctx,2);
    state(p,NEBULAR_RESET_WAIT);
}

int nebular_start(struct nebular_panel *p,const struct nebular_io *io,void *ctx,unsigned model) {
    if (!p || !io || model>1 || !io->wave) return 0;
    if (p->state!=NEBULAR_IDLE && p->state!=NEBULAR_DONE && p->state!=NEBULAR_ERROR) return 0;
    p->io=io; p->context=ctx; p->model=model; p->component=0;
    p->width=model ? 200 : 152; p->height=model ? 200 : 296;
    reset_component(p); return 1;
}
static uint8_t wave(struct nebular_panel *p,unsigned i) {
    return p->io->wave(p->context,p->component,i);
}

static int initialize_image(struct nebular_panel *p) {
    static const uint8_t booster[7]={15,10,47,37,34,46,33};
    uint32_t i;
    unsigned j;
    uint8_t pixels[4];
    if (!p->model) {
        reg(p,0x4d,0x78); reg(p,0xae,0x0f);
        reg(p,0xb6,0x0f); reg(p,0xba,0x2a);
    }
    command(p,0); data(p,7); data(p,0xa8);
    command(p,1); data(p,7);
    for (i=0;i<5;++i) data(p,wave(p,i));
    command(p,3); data(p,0x10); data(p,0x54); data(p,0x44);
    command(p,6); for (i=0;i<7;++i) data(p,(p->model && i==5) ? 43 : booster[i]);
    reg(p,0x30,wave(p,6)); reg(p,0x50,0x37); reg(p,0xf0,p->model ? 0x7d : 0x5f);
    command(p,0x60); data(p,2); data(p,2);
    command(p,0x61); data(p,p->width>>8); data(p,p->width); data(p,p->height>>8); data(p,p->height);
    command(p,0x65); for (i=0;i<4;++i) data(p,0);
    reg(p,0x82,wave(p,5)^0x80);
    reg(p,0xe3,0x22); reg(p,0xe7,0x1c); reg(p,0xe9,1);
    command(p,0x20);
    for (i=0;i<NEBULAR_WAVEFORM_SIZE;++i) data(p,wave(p,i));
    command(p,0x10);
    for (i=0;i<(uint32_t)p->width*p->height;i+=4) {
        for (j=0;j<4;++j) {
            pixels[j]=p->io->pixel(p->context,i+j);
            switch (pixels[j]) {
            case 1: case 4: case 5: case 6: case 7: case 9: case 13: break;
            default: return 0;
            }
        }
        data(p,nebular_pack4(pixels,p->component));
    }
    command(p,4);
    state(p,NEBULAR_POWER_WAIT);
    return 1;
}

enum nebular_state nebular_poll(struct nebular_panel *p) {
    uint32_t elapsed;
    if (!p) return NEBULAR_ERROR;
    if (p->state==NEBULAR_IDLE || p->state==NEBULAR_DONE ||
        p->state==NEBULAR_ERROR) return p->state;
    elapsed=p->io->millis(p->context)-p->since;
    switch (p->state) {
    case NEBULAR_RESET_WAIT:
        if (elapsed<1) break;
        if (p->io->ready(p->context)) {
            if (!initialize_image(p)) nebular_abort(p);
        } else if (elapsed>=400) nebular_abort(p);
        break;
    case NEBULAR_POWER_WAIT:
        if (elapsed<1) break;
        if (p->io->ready(p->context)) {
            reg(p,0x12,0); state(p,NEBULAR_REFRESH_WAIT);
        } else if (elapsed>=2000) nebular_abort(p);
        break;
    case NEBULAR_REFRESH_WAIT:
        if (elapsed<10) break;
        if (p->io->ready(p->context)) {
            reg(p,2,0); state(p,NEBULAR_OFF_WAIT);
        } else if (elapsed>=200000) nebular_abort(p);
        break;
    case NEBULAR_OFF_WAIT:
        if (elapsed<1) break;
        if (p->io->ready(p->context)) {
            reg(p,7,0xa5); p->io->power(p->context,0);
            if (!p->component) { p->component=1; reset_component(p); }
            else state(p,NEBULAR_DONE);
        } else if (elapsed>=400) nebular_abort(p);
        break;
    default: nebular_abort(p); break;
    }
    return p->state;
}
