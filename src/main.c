#include "headers.h"

struct control_thread_args {
    struct window_state* window;
    struct v2d_double* pos;
    double* pixel_size;
    int* iterations;
    int* radius;
    int* fractal_id;
};

void control_thread_function(struct control_thread_args* args) {

    bool dragging = false;

    struct point2d_int mouse_cords;

    bool console_shown = false;

    while (is_window_active(args->window)) {

        if (get_key_state(KEY_MOUSE_LEFT) & 0b1) {
            struct point2d_int test_mouse_cords = get_mouse_cursor_position(args->window);
            
            if (test_mouse_cords.x > 1 && test_mouse_cords.x < args->window->window_width - 2 && test_mouse_cords.y > 1 && test_mouse_cords.y < args->window->window_height - 2 && is_window_selected(args->window)) {
                if (dragging == false) {
                    dragging = true;
                    mouse_cords = test_mouse_cords;
                }
            }
            else { dragging = false; }
        }
        else dragging = false;

        if (dragging) {
            struct point2d_int new_mouse_cords = get_mouse_cursor_position(args->window);

            *args->pos = (struct v2d_double){
                args->pos->x + (double)(mouse_cords.x - new_mouse_cords.x) * *args->pixel_size ,
                args->pos->y - (double)(mouse_cords.y - new_mouse_cords.y) * *args->pixel_size ,
            };

            mouse_cords = new_mouse_cords;
        }

        int scroll = get_last_mouse_scroll();

        if (scroll) {
            struct point2d_int mouse_pos = get_mouse_cursor_position(args->window);

            args->pos->x += (mouse_pos.x - args->window->window_width / 2) * (*args->pixel_size - *args->pixel_size * powf(1.2f, -(double)scroll));
            args->pos->y += -(mouse_pos.y - args->window->window_height / 2) * (*args->pixel_size - *args->pixel_size * powf(1.2f, -(double)scroll));

            *args->pixel_size *= powf(1.2f, -(double)scroll);
        }

        

        if ((get_key_state(KEY_PLUS) & 0b11) == 0b11) *args->iterations += 64;
        if (((get_key_state(KEY_MINUS) & 0b11) == 0b11) && *args->iterations > 64) *args->iterations -= 64;

        if ((get_key_state('W') & 0b11) == 0b11) *args->radius += 1;
        if (((get_key_state('S') & 0b11) == 0b11) && *args->radius > 1) *args->radius -= 1;

        if ((get_key_state('C') & 0b11) == 0b11) {
            if(console_shown) {console_shown = false; hide_console_window(); }
            else {console_shown = true; show_console_window(); }
        }

        if ((get_key_state('0') & 0b11) == 0b11) *args->fractal_id = 0;
        if ((get_key_state('1') & 0b11) == 0b11) *args->fractal_id = 1;
        if ((get_key_state('2') & 0b11) == 0b11) *args->fractal_id = 2;
        if ((get_key_state('3') & 0b11) == 0b11) *args->fractal_id = 3;
        if ((get_key_state('4') & 0b11) == 0b11) *args->fractal_id = 4;
        if ((get_key_state('5') & 0b11) == 0b11) *args->fractal_id = 5;
        if ((get_key_state('6') & 0b11) == 0b11) *args->fractal_id = 6;
        if ((get_key_state('7') & 0b11) == 0b11) *args->fractal_id = 7;
        if ((get_key_state('8') & 0b11) == 0b11) *args->fractal_id = 8;
        if ((get_key_state('9') & 0b11) == 0b11) *args->fractal_id = 9;

        printf("iterations: %d                                \n", *args->iterations);
        printf("radius: %d                                    \n", *args->radius);
        printf("position: %f %f                               \n", args->pos->x, args->pos->y);
        set_console_cursor_position(0, 0);

        sleep_for_ms(20);
    }

    

}

#define cmul(Re, Im) ()

void Entry() {

    struct window_state* window = create_window(200, 100, 700, 500, "Fractals");

    char* src_mandelbrot = fractal_from_equation("mandelbrot",   "Re * Re                  -                     Im * Im                                + cRe", "2 * Re *                Im                                                            + cIm");
    char* src_mandelbrot3 = fractal_from_equation("mandelbrot3", "Re * Re * Re             -  3 * Re *           Im * Im                                + cRe", "3 * Re * Re *           Im   -                Im * Im * Im                            + cIm");
    char* src_mandelbrot4 = fractal_from_equation("mandelbrot4", "Re * Re * Re * Re        -  6 * Re * Re *      Im * Im   +     Im * Im * Im * Im      + cRe", "4 * Re * Re * Re *      Im   -  4 * Re *      Im * Im * Im                            + cIm");
    char* src_mandelbrot5 = fractal_from_equation("mandelbrot5", "Re * Re * Re * Re * Re   - 10 * Re * Re * Re * Im * Im   + 5 * Im * Im * Im * Im * Re + cRe", "5 * Re * Re * Re * Re * Im   - 10 * Re * Re * Im * Im * Im   + Im * Im * Im * Im * Im + cIm");
    
    char* src_burning_ship = fractal_from_equation("burning_ship", "Re * Re - Im * Im + cRe", " 2.0 * fabs(Re * Im) + cIm");
    char* src_feather = fractal_from_equation("feather", "((Re * Re * Re - 3 * Re * Im * Im) * (1 + Re * Re) + (3 * Re * Re * Im - Im * Im * Im) * (Im * Im)) / ((1 + Re * Re) * (1 + Re * Re) + (Im * Im) * (Im * Im)) + cRe", "((3 * Re * Re * Im - Im * Im * Im) * (1 + Re * Re) - (Re * Re * Re - 3 * Re * Im * Im) * (Im * Im))  / ((1 + Re * Re) * (1 + Re * Re) + (Im * Im) * (Im * Im)) + cIm");

    struct fractal_render_resources mandelbrot_resources = setup_fractal(src_mandelbrot, "mandelbrot");
    struct fractal_render_resources mandelbrot3_resources = setup_fractal(src_mandelbrot3, "mandelbrot3");
    struct fractal_render_resources mandelbrot4_resources = setup_fractal(src_mandelbrot4, "mandelbrot4");
    struct fractal_render_resources mandelbrot5_resources = setup_fractal(src_mandelbrot5, "mandelbrot5");
    struct fractal_render_resources burning_ship_resources = setup_fractal(src_burning_ship, "burning_ship");
    struct fractal_render_resources feather_resources = setup_fractal(src_feather, "feather");
    
    struct v2d_double pos = { 0.5, 0. };

    double pixel_size = 0.04;
     
    int iterations = 64;

    int radius = 2;

    int id = 0;

    struct control_thread_args args = {window, &pos, &pixel_size, &iterations, &radius, &id};

    void* control_thread = create_thread(control_thread_function, &args);

    while (is_window_active(window)) {
        
        struct fractal_render_resources* resources;

        switch (id) {
            
        case 1:
            resources = &mandelbrot_resources;
            break;

        case 2: 
            resources = &mandelbrot3_resources;
            break;

        case 3:
            resources = &mandelbrot4_resources;
            break;

        case 4:
            resources = &mandelbrot5_resources;
            break;

        case 5:
            resources = &burning_ship_resources;
            break;

        case 6:
            resources = &feather_resources;
            break;

        default:
            resources = &mandelbrot_resources;
            break;
        }
        int width = window->window_width;
        int height = window->window_height;
        unsigned int* data = render_fractal(resources, pos, pixel_size, width, height, iterations, radius);

        struct point2d_int mp = get_mouse_cursor_position(window);

        draw_to_window(window, data, width, height);

        free(data);

        sleep_for_ms(10);

    }
    
    cleanup_fractal(&mandelbrot_resources);
    cleanup_fractal(&mandelbrot3_resources);
    cleanup_fractal(&mandelbrot4_resources);
    cleanup_fractal(&burning_ship_resources);
    cleanup_fractal(&feather_resources);  

    close_window(window);

    join_thread(control_thread);


    return;
}
