#include <CL/cl.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "openclutils.h"
#include "fractal.h"



struct fractal_render_resources {
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_device_id device;
    bool double_support;
};

void* setup_fractal(char** source, char* kernel_name) {
    struct fractal_render_resources* setup = malloc(sizeof(struct fractal_render_resources));

    cl_platform_id platform;
    CL_CALL(clGetPlatformIDs(1, &platform, NULL));

    CL_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &setup->device, NULL));

    CL_OBJECT_CALL( , setup->context, clCreateContext(NULL, 1, &setup->device, NULL, NULL, &err));

    CL_OBJECT_CALL( , setup->queue, clCreateCommandQueue(setup->context, setup->device, 0, &err));

    char extensions[1024];
    clGetDeviceInfo(setup->device, CL_DEVICE_EXTENSIONS, sizeof(extensions), extensions, NULL);

    if (strstr(extensions, "cl_khr_fp64") == NULL) {
        setup->double_support = false;
        CL_OBJECT_CALL(, setup->program, clCreateProgramWithSource(setup->context, 1, (const char**) &source[0], NULL, &err));
    }
    else {
        setup->double_support = true;
        CL_OBJECT_CALL(, setup->program, clCreateProgramWithSource(setup->context, 1, (const char**) &source[1], NULL, &err));
    }

    CL_CALL(clBuildProgram(setup->program, 1, &setup->device, NULL, NULL, NULL));

    CL_OBJECT_CALL( , setup->kernel, clCreateKernel(setup->program, kernel_name, &err));

    return setup;
}

unsigned int* render_fractal(void* resources, struct v2d_double position, double pixel_size, int width, int height, int max_iterations, int divergence_radius) {

    unsigned int* image = (unsigned int*)malloc(width * height * sizeof(unsigned int));

    CL_OBJECT_CALL(cl_mem, imageMemObj, clCreateBuffer(((struct fractal_render_resources*)resources)->context, CL_MEM_WRITE_ONLY, width * height * sizeof(unsigned int), NULL, &err));
    
    if (((struct fractal_render_resources*)resources)->double_support) {
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 0, sizeof(cl_mem), (void*)&imageMemObj));
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 1, sizeof(int), &width));
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 2, sizeof(int), &height));
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 3, sizeof(double), &pixel_size));
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 4, sizeof(double), &position.x));
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 5, sizeof(double), &position.y));
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 6, sizeof(int), &max_iterations));
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 7, sizeof(int), &divergence_radius));
    }

    else {

        float posx = position.x;
        float posy = position.y;
        float ps = pixel_size;

        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 0, sizeof(cl_mem), (void*)&imageMemObj));
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 1, sizeof(int), &width));
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 2, sizeof(int), &height));
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 3, sizeof(float), &ps));
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 4, sizeof(float), &posx));
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 5, sizeof(float), &posy));
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 6, sizeof(int), &max_iterations));
        CL_CALL(clSetKernelArg(((struct fractal_render_resources*)resources)->kernel, 7, sizeof(int), &divergence_radius));
    }

    size_t globalItemSize[2] = { width, height };
    CL_CALL(clEnqueueNDRangeKernel(((struct fractal_render_resources*)resources)->queue, ((struct fractal_render_resources*)resources)->kernel, 2, NULL, globalItemSize, NULL, 0, NULL, NULL));
    
    CL_CALL(clFlush(((struct fractal_render_resources*)resources)->queue));
    CL_CALL(clFinish(((struct fractal_render_resources*)resources)->queue));
    
    CL_CALL(clEnqueueReadBuffer(((struct fractal_render_resources*)resources)->queue, imageMemObj, CL_TRUE, 0, width * height * sizeof(unsigned int), image, 0, NULL, NULL));

    CL_CALL(clReleaseMemObject(imageMemObj));

    return image;
};

void cleanup_fractal(void* setup) {
    clReleaseKernel(((struct fractal_render_resources*)setup)->kernel);
    clReleaseProgram(((struct fractal_render_resources*)setup)->program);
    clReleaseCommandQueue(((struct fractal_render_resources*)setup)->queue);
    clReleaseContext(((struct fractal_render_resources*)setup)->context);
}