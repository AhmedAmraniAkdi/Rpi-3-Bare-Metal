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
// bmce2837 is cortex-a53 , it has neon and not scalable vector extension sve

/*
x = deltaX/width * (Xpixel + pan_factor * Xpan + Xstart) * Scale
Y = deltaY/heigth * (Ypixel + pan_factor * Ypan + Ystart) * Scale
*/
#define width 1024
#define height 768

//static int pan_factor = 10;
static int Xpan = 0;
static int Ypan = 0;
static double scale = 1;

//static double Xstart = -2.0;
//static double Ystart = -2.0;

static double deltaX = 4.0;
static double deltaY = 4.0;

static uint32_t *bf;

#define line_per_thread 24 // 32 threads total (not counting mains) and 768 heights -> 24 lines per thread;
#define total_threads 32
#define iterations 1024

void colour_fractal(uint64_t n1, uint64_t n2, uint32_t *buffer_temp){
    *buffer_temp = ((int) (127.5 * sin(0.1 * n1) + 127.5)) 
        | ((int) (127.5 * sin(0.1 * n1 + 2.094) + 127.5) << 8)
        | ((int) (127.5 * sin(0.1 * n1 + 4.188) + 127.5) << 16);

    buffer_temp++;

    *buffer_temp = ((int) (127.5 * sin(0.1 * n2) + 127.5)) 
        | ((int) (127.5 * sin(0.1 * n2 + 2.094) + 127.5) << 8)
        | ((int) (127.5 * sin(0.1 * n2 + 4.188) + 127.5) << 16);

    buffer_temp++;
    
}

// big inspiration from
// https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/OneLoneCoder_VIDEO_IntrinsicFunctions.cpp
void draw_fractal_mandelbrot(void *arg, void *ret){
    int *ith_line = (int*)arg;
    uint32_t *buffer_temp = bf;

    double xcoord = 0.0, ycoord = 0.0;
    double xcoords[2] = {0,0};

    // https://gist.github.com/csarron/3191b401ec545f78022881f1805cae9a
    // https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/intrinsics

    float64x2_t _a, _b, _zr, _zi, _zr2, _zi2, _cr, _ci;
    float64x2_t _two = {2.0, 2.0};
    float64x2_t _four = {4.0, 4.0};

    uint64x2_t _n, _mask0, _mask1, _temp;
    uint64x2_t _iterations = {iterations, iterations};
    uint64x2_t _one = {1,1};

    uint64_t n1 = 0, n2 = 0;

    for(int y = *ith_line; y < line_per_thread; y += total_threads, buffer_temp += (total_threads * width)){

        ycoord = deltaY/height * (y + Ypan) * scale;
        _ci    = vld1q_dup_f64(&ycoord);

        for(int x = 0; x < width; x += 2/*, buffer_temp += 2*/){ // x, y are pixel space
            
            xcoord     = deltaX/width * (x + Xpan) * scale;
            xcoords[0] = xcoord;
            xcoords[1] = xcoord + deltaX/width * scale;
            _cr        = vld1q_f64(xcoords);

            _zi = vmovq_n_f64(0.0);
            _zr = vmovq_n_f64(0.0);
            _n  = vmovq_n_u64(0);
            
            r:
            // z = a + ib = z * z + c
            // a = zr * zr - zi * zi + cr = zr2 - zi2 + cr
            // b = zi * zi * 2.0 + ci

            _zr2 = vmulq_f64(_zr, _zr);
            _zi2 = vmulq_f64(_zi, _zi);

            _a   = vsubq_f64(_zr2, _zi2);
            _a   = vaddq_f64(_a, _cr);

            _b   = vmulq_f64(_zi, _zi);
            _b   = vmlaq_f64(_ci, _b, _two);

            _zr  = _a;
            _zi  = _b;

            // while( abs(z) < 2 && n < iterations)
            // while((zi * zi + zr * zr) < 4 && n < iterations)

            _a     = vaddq_f64(_zr2, _zi2);
            _mask0 = vcltq_f64(_a, _four);
            _mask1 = vcltq_u64(_n, _iterations);
            _mask1 = vandq_u64(_mask1, _mask0);

            // we increment n in places where the condition holds

            _temp = vandq_u64(_mask1, _one);
            _n    = vaddq_u64(_n , _temp);

            // https://stackoverflow.com/questions/41005281/testing-neon-simd-registers-for-equality-over-all-lanes
            // https://stackoverflow.com/questions/34504266/comparision-with-zero-using-neon-instruction

            // if any element in narrowed mask1 is 1 then we still iterate
            //if(vqmovn_u64(_mask1) > 0) // in the compiler we trust (need to pick maximum of the 32x2?)
            if(vaddvq_u64(_mask1) > 0){ // add the vectors together
                goto r;
            }

            n1 = vgetq_lane_u64(_n, 0);
            n2 = vgetq_lane_u64(_n, 1);
            colour_fractal(n1, n2, buffer_temp);
        }
    }

}

void notmain(){

    set_max_freq();
    uart_init();
    print_info_mem_freq();
    interrupt_init();
    populate_tables();
    mmu_enable();
    fb_init();
    bf = buffer();

    register_irq_handler(bTIMER_CORE0, CORE0, &scheduler_tick, &core_timer_clearer);
    register_irq_handler(bTIMER_CORE1, CORE1, &scheduler_tick, &core_timer_clearer);
    register_irq_handler(bTIMER_CORE2, CORE2, &scheduler_tick, &core_timer_clearer);
    register_irq_handler(bTIMER_CORE3, CORE3, &scheduler_tick, &core_timer_clearer);

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
    int k = 0;
    fork_task(CORE0, &draw_fractal_mandelbrot, &k, NULL);
    int j = 1;
    fork_task(CORE1, &draw_fractal_mandelbrot, &j, NULL);
    printk("starting threading\n");

    /*threading_init();
    join_all();
    printk("Input:");
    printk("\nleft:q\tright:d\tup:z\tdown:s\tzoomIN:o\tzoomOut:p\tquit:l\n");

    do{
        input = uart_getc();
        switch(input){
            case 'q':
                Xpan-=10;
                break;
            case 'd':
                Xpan+=10;
                break;
            case 'z':
                Ypan++;
                break;
            case 's':
                Ypan--;
                break;
            case 'p':
                scale *= 1.001;
                break;
            case 'o':
                scale *= 0.999;
                break;
            case 'l':
                break;
            default:
                printk("Wrong key:\nleft:q\tright:d\tup:z\tdown:s\tzoomIN:o\tzoomOut:p\tquit:l\n");
        }

        time1 = READ_TIMER();
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
        printk("Time consumed for drawing the fractal: %f",(double)(time1 - time2)/freq);
    } while(input != 'l');

    printk("\nAnother chapter closes\n");*/

    while(1){}
}