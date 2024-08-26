#include "headers.h"

struct fractal_render_resources setup_fractal(const char* source, const char* kernel_name) {
    struct fractal_render_resources setup;



    cl_platform_id platform;
    CL_CALL(clGetPlatformIDs(1, &platform, NULL));

    CL_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &setup.device, NULL));

    CL_OBJECT_CALL( , setup.context, clCreateContext(NULL, 1, &setup.device, NULL, NULL, &err));

    CL_OBJECT_CALL( , setup.queue, clCreateCommandQueue(setup.context, setup.device, 0, &err));

    CL_OBJECT_CALL( , setup.program, clCreateProgramWithSource(setup.context, 1, &source, NULL, &err));

    CL_CALL(clBuildProgram(setup.program, 1, &setup.device, NULL, NULL, NULL));

    CL_OBJECT_CALL( , setup.kernel, clCreateKernel(setup.program, kernel_name, &err));

    return setup;
}

unsigned int* render_fractal(struct fractal_render_resources* resources, struct v2d_double position, double pixel_size, int width, int height, int max_iterations, int divergence_radius) {

    unsigned int* image = (unsigned int*)malloc(width * height * sizeof(unsigned int));

    CL_OBJECT_CALL(cl_mem, imageMemObj, clCreateBuffer(resources->context, CL_MEM_WRITE_ONLY, width * height * sizeof(unsigned int), NULL, &err));
    
    CL_CALL(clSetKernelArg(resources->kernel, 0, sizeof(cl_mem), (void*)&imageMemObj));
    CL_CALL(clSetKernelArg(resources->kernel, 1, sizeof(int), &width));
    CL_CALL(clSetKernelArg(resources->kernel, 2, sizeof(int), &height));
    CL_CALL(clSetKernelArg(resources->kernel, 3, sizeof(double), &pixel_size));
    CL_CALL(clSetKernelArg(resources->kernel, 4, sizeof(double), &position.x));
    CL_CALL(clSetKernelArg(resources->kernel, 5, sizeof(double), &position.y));
    CL_CALL(clSetKernelArg(resources->kernel, 6, sizeof(int), &max_iterations));
    CL_CALL(clSetKernelArg(resources->kernel, 7, sizeof(int), &divergence_radius));

    size_t globalItemSize[2] = { width, height };
    CL_CALL(clEnqueueNDRangeKernel(resources->queue, resources->kernel, 2, NULL, globalItemSize, NULL, 0, NULL, NULL));
    
    CL_CALL(clFlush(resources->queue));
    CL_CALL(clFinish(resources->queue));
    
    CL_CALL(clEnqueueReadBuffer(resources->queue, imageMemObj, CL_TRUE, 0, width * height * sizeof(unsigned int), image, 0, NULL, NULL));

    CL_CALL(clReleaseMemObject(imageMemObj));

    return image;
};

void cleanup_fractal(struct fractal_render_resources* setup) {
    clReleaseKernel(setup->kernel);
    clReleaseProgram(setup->program);
    clReleaseCommandQueue(setup->queue);
    clReleaseContext(setup->context);
}