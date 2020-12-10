#include "rpi.h"
#include "interrupt.h"
#include "thread.h"
#include "mini_uart.h"
#include "kassert.h"
#include <stdint.h>
#include "mmu.h"
#include "VCmailbox.h"
#include <arm_neon.h>
#include "fb.h"
#include <math.h>

// let's do this

#define width 1024
#define height 768
#define depth 4

#define Xstart -2.5
#define Ystart -1.5
#define Xend 1.5
#define Yend 1.5

static float Xpan = 0;
static float Ypan = 0;
static float scale = 1.0;

static uint32_t *bf = NULL;

#define line_per_thread 24 // 32 threads total (not counting mains) and 768 heights -> 24 lines per thread;
#define total_threads 32
#define iterations 32


// big inspiration from
// https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/OneLoneCoder_VIDEO_IntrinsicFunctions.cpp
void draw_fractal_mandelbrot(void *arg, void *ret){
    //float *ith_line = (float*)arg;
    uint32_t *buffer_temp = bf;

    float32x4_t _a, _b, _zr, _zi, _zr2, _zi2, _cr, _ci;
    float32x4_t _two = vdupq_n_f32(2.0);
    float32x4_t _four = vdupq_n_f32(4.0);
    float32x4_t _x_offset123 = {0.0, 1.0, 2.0, 3.0};

    uint32x4_t _n, _mask0, _mask1;
    uint32x4_t _iterations =  vdupq_n_u32(iterations);
    uint32x4_t _one = vdupq_n_u32(1);

    float32x4_t _xmin = vdupq_n_f32(Xstart + Xpan);
    float32x4_t _ymin = vdupq_n_f32(Ystart + Ypan);
    float32x4_t _xscale = vdupq_n_f32((Xend - Xstart)/width * scale);
    float32x4_t _yscale = vdupq_n_f32((Yend - Ystart)/height * scale);

    uint32_t n_result[4] = {0, 0, 0, 0};


    //for(int y = *ith_line; y < line_per_thread; y += total_threads, buffer_temp += (total_threads * width)){
    for(int y = 0.0/**ith_line*/; y < height; y++){
        _a  = vdupq_n_f32((float)y);
        _ci = vmlaq_f32(_ymin, _yscale, _a);

        for(int x = 0.0; x < width; x += 4/*, buffer_temp += 4*/){ // x, y are pixel space
            
            _a  = vdupq_n_f32((float)x);
            _a  = vaddq_f32(_a, _x_offset123);
            _cr = vmlaq_f32(_xmin, _xscale, _a);


            _zi = vdupq_n_f32(0.0);
            _zr = vdupq_n_f32(0.0);
            _n  = vdupq_n_u32(0);
            
            r:
            // z = a + ib = z * z + c
            // a = zr * zr - zi * zi + cr = zr2 - zi2 + cr
            // b = zr * zi * 2.0 + ci

            _zr2 = vmulq_f32(_zr, _zr);
            _zi2 = vmulq_f32(_zi, _zi);

            _a   = vsubq_f32(_zr2, _zi2);
            _a   = vaddq_f32(_a, _cr);

            _b   = vmulq_f32(_zr, _zi);
            _b   = vmlaq_f32(_ci, _b, _two);

            _zr  = _a;
            _zi  = _b;

            // while( abs(z) < 2 && n < iterations)
            // while((zi * zi + zr * zr) < 4 && n < iterations)

            _a     = vaddq_f32(_zr2, _zi2);
            _mask0 = vcltq_f32(_a, _four);

            if(vaddvq_u32(_mask0) == 0){ // add the vectors together
                goto colour;
            }

            _mask1 = vcltq_u32(_n, _iterations);
            _mask1 = vandq_u32(_mask1, _mask0);

            // we increment n in places where the condition holds

            _n    = vaddq_u32(_n , vandq_u32(_mask1, _one));

            if(vaddvq_u32(_mask1) > 0){ // add the vectors together
                goto r;
            }

            colour:
            vst1q_u32(n_result, _n);
            for(int i = 0; i < 4; i++){
                *buffer_temp = (int) (127.5 * sin(0.1 * n_result[i]) + 127.5)
                             | ((int) (127.5 * sin(0.1 * n_result[i] + 2.094) + 127.5)) << 8
                             | ((int) (127.5 * sin(0.1 * n_result[i] + 4.188) + 127.5)) << 16;
                buffer_temp++;
            }
        }
    }

}

void notmain(){

    uart_init();
    interrupt_init();
    populate_tables();
    mmu_enable();
    bf = fb_init();


    //egister_irq_handler(bTIMER_CORE0, CORE0, &scheduler_tick, &core_timer_clearer);
    //register_irq_handler(bTIMER_CORE1, CORE1, &scheduler_tick, &core_timer_clearer);
    //register_irq_handler(bTIMER_CORE2, CORE2, &scheduler_tick, &core_timer_clearer);
    //register_irq_handler(bTIMER_CORE3, CORE3, &scheduler_tick, &core_timer_clearer);

    printk("configuration complete\n");

    char input = 'w';
    uint64_t time1 = 0;
    uint64_t time2 = 0;
    uint64_t freq = READ_TIMER_FREQ();
    
    /*int ith_line = 0;
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 8; j++){
            int k = ith_line;
            fork_task(CORE0 + i, &draw_fractal_mandelbrot, &k, NULL);
            ith_line++;
        }
    }*/
    /*int k = 0;
    fork_task(CORE0, &draw_fractal_mandelbrot, &k, NULL);
    int j = 1;
    fork_task(CORE1, &draw_fractal_mandelbrot, &j, NULL);*/
    int k = 0;
    //fork_task(CORE0, &draw_fractal_mandelbrot, &k, NULL);
    printk("starting threading\n");

    //threading_init();
    //join_all();
    printk("Input:");
    printk("\nleft:q\tright:d\tup:z\tdown:s\tzoomIN:o\tzoomOut:p\tquit:l\n");
    draw_fractal_mandelbrot(&k, NULL);
    do{
        input = uart_getc();
        switch(input){
            case 'q':
                Xpan -= 0.05;
                break;
            case 'd':
                Xpan += 0.05;
                break;
            case 'z':
                Ypan += 0.05;
                break;
            case 's':
                Ypan -= 0.05;
                break;
            case 'p':
                scale *= 1.01;
                break;
            case 'o':
                scale *= 0.99;
                break;
            case 'l':
                break;
            default:
                printk("Wrong key:\nleft:q\tright:d\tup:z\tdown:s\tzoomIN:o\tzoomOut:p\tquit:l\n");
        }
        draw_fractal_mandelbrot(&k, NULL);
        /*time1 = READ_TIMER();
        ith_line = 0;
        // since reusing for the same task, we can not exit and not fork threads?
        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 8; j++){
                fork_task(CORE0 + i, &draw_fractal_mandelbrot, &ith_line, NULL);
                ith_line++;
            }
        }
        threading_init();
        join_all();

        time2 = READ_TIMER();
        printk("Time consumed for drawing the fractal: %f",(double)(time1 - time2)/freq);*/
    } while(input != 'l');

    printk("\nAnother chapter closes\n");

    while(1){}
}