#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "platform.h"
#include "parallel_computing.h"
#include "utils.h"

#define fractal_from_equation(name, Re, Im) {\
"                                                 __kernel void " name "(__global unsigned int* image, int width, int height, float  pixelSize, float  centerX, float  centerY, int maxIter, int divRadius) {\n\n int x = get_global_id(0);\nint y = get_global_id(1);\n\nfloat Re = 0.f;\nfloat Im = 0.f;\n\nfloat cRe = centerX + (x - width / 2.0f) * pixelSize;\nfloat cIm = (centerY + (height / 2.0f - y - 1.f) * pixelSize);\n\nint n = 0;\nwhile (Re * Re + Im * Im < divRadius * divRadius && n < maxIter) { \nfloat Re_new = " Re ";\nfloat Im_new = " Im ";\n\nRe = Re_new;\nIm = Im_new;\nn++;\n}\n\nunsigned int color;\nif (n == maxIter) {color = 0xFF000000;}\nelse {\nfloat t = (float)n / maxIter;\nunsigned int r = (unsigned int)(9 * (1 - t) * t * t * t * 255);\nunsigned int g = (unsigned int)(15 * (1 - t) * (1 - t) * t * t * 255);\nunsigned int b = (unsigned int)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);\ncolor = (0xFF << 24) | (r << 16) | (g << 8) | b;\n}\nimage[y * width + x] = color;\n}",\
"#pragma OPENCL EXTENSION cl_khr_fp64 : enable \n __kernel void " name "(__global unsigned int* image, int width, int height, double pixelSize, double centerX, double centerY, int maxIter, int divRadius) {\n\n int x = get_global_id(0);\nint y = get_global_id(1);\n\ndouble Re = 0;\ndouble Im = 0;\n\ndouble cRe = centerX + (x - width / 2.0) * pixelSize;\ndouble cIm = (centerY + (height / 2.0 - y - 1) * pixelSize);\n\nint n = 0;\nwhile (Re * Re + Im * Im < divRadius * divRadius && n < maxIter) { \ndouble Re_new = " Re ";\ndouble Im_new = " Im ";\n\nRe = Re_new;\nIm = Im_new;\nn++;\n}\n\nunsigned int color;\nif (n == maxIter) {color = 0xFF000000;}\nelse {\ndouble t = (double)n / maxIter;\nunsigned int r = (unsigned int)(9 * (1 - t) * t * t * t * 255);\nunsigned int g = (unsigned int)(15 * (1 - t) * (1 - t) * t * t * 255);\nunsigned int b = (unsigned int)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);\ncolor = (0xFF << 24) | (r << 16) | (g << 8) | b;\n}\nimage[y * width + x] = color;\n}"}


struct control_thread_args {
    int window;
    struct v2d_double* pos;
    double* pixel_size;
    int* iterations;
    int* radius;
    int* fractal_id;
    char* change_flag;
    char* double_mode;
    char double_support;
};

void control_thread_function(struct control_thread_args* args) {

    bool dragging = false;

    struct point2d_int mouse_cords;

    bool console_shown = false;

    int width = get_window_width(args->window);
    int height = get_window_height(args->window);

    while (is_window_active(args->window)) {

        int new_width = get_window_width(args->window);
        int new_height = get_window_height(args->window);

        if(new_width != width || new_height != height) {
            width = new_width;
            height = new_height;
            *args->change_flag |= 2;
        }

        struct point2d_int new_mouse_cords = get_mouse_cursor_position(args->window);

        if (get_key_state(KEY_MOUSE_LEFT) & 0b1) {
            
            
            if (new_mouse_cords.x > 1 && new_mouse_cords.x < width - 2 && new_mouse_cords.y > 1 && new_mouse_cords.y < height - 2 && is_window_selected(args->window)) {
                if (dragging == false) {
                    dragging = true;
                    mouse_cords = new_mouse_cords;
                }
            }
            else { dragging = false; }
        }
        else dragging = false;

        if (dragging) {

            *args->pos = (struct v2d_double){
                args->pos->x + (double)(mouse_cords.x - new_mouse_cords.x) * *args->pixel_size ,
                args->pos->y - (double)(mouse_cords.y - new_mouse_cords.y) * *args->pixel_size ,
            };

            *args->change_flag |= 1;

            mouse_cords = new_mouse_cords;
        }

        int scroll = get_last_mouse_scroll();

        if (scroll) {

            args->pos->x += (new_mouse_cords.x - width / 2) * (*args->pixel_size - *args->pixel_size * pow(1.2f, -(double)scroll));
            args->pos->y += -(new_mouse_cords.y - height / 2) * (*args->pixel_size - *args->pixel_size * pow(1.2f, -(double)scroll));

            *args->pixel_size *= pow(1.2, -(double)scroll);
            *args->change_flag |= 1;
        }

        

        if ((get_key_state(KEY_PLUS) & 0b11) == 0b11) { 
            *args->iterations += 256; 
            *args->change_flag |= 1;
        }
        if (((get_key_state(KEY_MINUS) & 0b11) == 0b11) && *args->iterations > 256) {
            *args->iterations -= 256;
            *args->change_flag |= 1;
        }

        if ((get_key_state('W') & 0b11) == 0b11) { 
            *args->radius += 1;
            *args->change_flag |= 1;
        }
        if (((get_key_state('S') & 0b11) == 0b11) && *args->radius > 1) {
            *args->radius -= 1;
            *args->change_flag |= 1;
        }

        if ((get_key_state('C') & 0b11) == 0b11) { 
            if(console_shown) {console_shown = false; hide_console_window(); }
            else {console_shown = true; show_console_window(); }
        }

        if ((get_key_state('D') & 0b11) == 0b11 && args->double_support) {
            if (*args->double_mode == 1) { *args->double_mode = 0; *args->change_flag |= 1;}
            else { *args->double_mode = 1; *args->change_flag |= 1;}
        }

        if ((get_key_state('0') & 0b11) == 0b11) { *args->fractal_id = 0; *args->change_flag |= 1; }
        if ((get_key_state('1') & 0b11) == 0b11) { *args->fractal_id = 1; *args->change_flag |= 1; }
        if ((get_key_state('2') & 0b11) == 0b11) { *args->fractal_id = 2; *args->change_flag |= 1; }
        if ((get_key_state('3') & 0b11) == 0b11) { *args->fractal_id = 3; *args->change_flag |= 1; }
        if ((get_key_state('4') & 0b11) == 0b11) { *args->fractal_id = 4; *args->change_flag |= 1; }
        if ((get_key_state('5') & 0b11) == 0b11) { *args->fractal_id = 5; *args->change_flag |= 1; }
        if ((get_key_state('6') & 0b11) == 0b11) { *args->fractal_id = 6; *args->change_flag |= 1; }
        if ((get_key_state('7') & 0b11) == 0b11) { *args->fractal_id = 7; *args->change_flag |= 1; }
        if ((get_key_state('8') & 0b11) == 0b11) { *args->fractal_id = 8; *args->change_flag |= 1; }
        if ((get_key_state('9') & 0b11) == 0b11) { *args->fractal_id = 9; *args->change_flag |= 1; }

        sleep_for_ms(20);
    }

}

int main() {

    platform_init();
    parallel_computing_init();

    int window = create_window(0, 0, 1000 + 16, 800 + 39, "Fractals");

    struct v2d_double pos = { 0.5, 0. };
    double pixel_size = 0.004;
    int iterations = 1200;
    int radius = 2;
    int id = 0;
    char change_flag = 1;
    char double_mode = 0;
    char double_support = is_double_supported();

    char* src_mandelbrot[] = fractal_from_equation("mandelbrot", "Re * Re                  -                     Im * Im                                + cRe", "2 * Re *                Im                                                            + cIm");
    char* src_mandelbrot3[] = fractal_from_equation("mandelbrot3", "Re * Re * Re             -  3 * Re *           Im * Im                                + cRe", "3 * Re * Re *           Im   -                Im * Im * Im                            + cIm");
    char* src_mandelbrot4[] = fractal_from_equation("mandelbrot4", "Re * Re * Re * Re        -  6 * Re * Re *      Im * Im   +     Im * Im * Im * Im      + cRe", "4 * Re * Re * Re *      Im   -  4 * Re *      Im * Im * Im                            + cIm");
    char* src_mandelbrot5[] = fractal_from_equation("mandelbrot5", "Re * Re * Re * Re * Re   - 10 * Re * Re * Re * Im * Im   + 5 * Im * Im * Im * Im * Re + cRe", "5 * Re * Re * Re * Re * Im   - 10 * Re * Re * Im * Im * Im   + Im * Im * Im * Im * Im + cIm");
    char* src_burning_ship[] = fractal_from_equation("burning_ship", "Re * Re - Im * Im + cRe", " 2.0 * fabs(Re * Im) + cIm");
    char* src_feather[] = fractal_from_equation("feather", "((Re * Re * Re - 3 * Re * Im * Im) * (1 + Re * Re) + (3 * Re * Re * Im - Im * Im * Im) * (Im * Im)) / ((1 + Re * Re) * (1 + Re * Re) + (Im * Im) * (Im * Im)) + cRe", "((3 * Re * Re * Im - Im * Im * Im) * (1 + Re * Re) - (Re * Re * Re - 3 * Re * Im * Im) * (Im * Im))  / ((1 + Re * Re) * (1 + Re * Re) + (Im * Im) * (Im * Im)) + cIm");

    void* mandelbrot_kernels[] = { create_kernel(src_mandelbrot[0]), (double_support ? create_kernel(src_mandelbrot[1]) : NULL) };
    void* mandelbrot3_kernels[] = { create_kernel(src_mandelbrot3[0]), (double_support ? create_kernel(src_mandelbrot3[1]) : NULL) };
    void* mandelbrot4_kernels[] = { create_kernel(src_mandelbrot4[0]), (double_support ? create_kernel(src_mandelbrot4[1]) : NULL) };
    void* mandelbrot5_kernels[] = { create_kernel(src_mandelbrot5[0]), (double_support ? create_kernel(src_mandelbrot5[1]) : NULL) };
    void* burning_ship_kernels[] = { create_kernel(src_burning_ship[0]), (double_support ? create_kernel(src_burning_ship[1]) : NULL) };
    void* feather_kernels[] = { create_kernel(src_feather[0]), (double_support ? create_kernel(src_feather[1]) : NULL) };

    struct control_thread_args args = {window, &pos, &pixel_size, &iterations, &radius, &id, &change_flag, &double_mode, double_support};
    void* control_thread = create_thread(control_thread_function, &args);

    double last_frame_time = 0;

    int width = get_window_width(window);
    int height = get_window_height(window);
    void* device_buffer = create_device_buffer(width * height * sizeof(unsigned int));
    unsigned int* pixel_buffer = malloc(width * height * sizeof(unsigned int));

    while (is_window_active(window)) {



        if (change_flag && get_window_height(window) != 0) {

            void** kernel;

            switch (id) {

            case 1: kernel = mandelbrot_kernels; break;
            case 2: kernel = mandelbrot3_kernels; break;
            case 3: kernel = mandelbrot4_kernels; break;
            case 4: kernel = mandelbrot5_kernels; break;
            case 5: kernel = burning_ship_kernels; break;
            case 6: kernel = feather_kernels; break;
            default: kernel = mandelbrot_kernels; break;

            }

            double start_time = get_time();

            if(change_flag & 2) {

                free(pixel_buffer);
                destroy_device_buffer(device_buffer);

                width = get_window_width(window);
                height = get_window_height(window);

                device_buffer = create_device_buffer(width * height * sizeof(unsigned int));
                pixel_buffer = malloc(width * height * sizeof(unsigned int));
            }

            run_kernel(kernel[double_mode], 2, width, height, 0, device_buffer, width, height, pixel_size, pos.x, pos.y, iterations, radius);
            read_from_device_buffer(device_buffer, pixel_buffer, width * height * sizeof(unsigned int), 0);
            wait_for_parallel_execution_completion();

            draw_to_window(window, pixel_buffer, width, height, 1);
            change_flag = false;

            set_console_cursor_position(0, 0);
            printf("last frame time: %f             \n", get_time() - start_time);
            printf("iterations: %d                                \n", iterations);
            printf("radius: %d                                    \n", radius);
            printf("position: %f %f                               \n", pos.x, pos.y);

            sleep_for_ms(10);
        }



    }
    
    destroy_kernel(mandelbrot_kernels[0]); if(double_support) destroy_kernel(mandelbrot_kernels[1]);
    destroy_kernel(mandelbrot3_kernels[0]); if (double_support) destroy_kernel(mandelbrot3_kernels[1]);
    destroy_kernel(mandelbrot4_kernels[0]); if (double_support) destroy_kernel(mandelbrot4_kernels[1]);
    destroy_kernel(mandelbrot5_kernels[0]); if (double_support) destroy_kernel(mandelbrot5_kernels[1]);
    destroy_kernel(burning_ship_kernels[0]); if (double_support) destroy_kernel(burning_ship_kernels[1]);
    destroy_kernel(feather_kernels[0]); if (double_support) destroy_kernel(feather_kernels[1]);

    wait_for_parallel_execution_completion();

    free(pixel_buffer);
    destroy_device_buffer(device_buffer);

    close_window(window);

    
    join_thread(control_thread);

    parallel_computing_exit();
    platform_exit();

    return 0;
}
