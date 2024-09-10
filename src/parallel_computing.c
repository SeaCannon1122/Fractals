#include "parallel_computing.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <CL/cl.h>

#define MAX_KERNEL_ARGS 128

enum arg_types {
    ARG_CHAR = 0,
    ARG_SHORT = 1,
    ARG_INT = 2,
    ARG_LONG = 3,
    ARG_FLOAT = 4,
    ARG_DOUBLE = 5,
    ARG_POINTER = 6,
};

cl_context context;
cl_device_id device;
cl_command_queue queue;

bool double_support = false;

struct program_resources {
    cl_program program;
    cl_kernel kernel;

    int arg_count;
    char arg_types[MAX_KERNEL_ARGS];
};

#define CL_CALL(call)\
do {\
cl_int err = (call);\
if (err != CL_SUCCESS) {\
fprintf(stderr, "OpenCL error in\n     %s\n  at %s:%d: %d\n", #call, __FILE__, __LINE__, err);\
exit(1);\
}\
} while(0)

#define CL_OBJECT_CALL(type, object, call)\
type object;\
do {\
cl_int err;\
object = (call);\
if (err != CL_SUCCESS) {\
fprintf(stderr, "OpenCL error in \n     %s \n  at %s:%d: %d\n^\n",#call, __FILE__, __LINE__, err);\
exit(1);\
}\
} while(0)



void parallel_computing_init() {
    cl_platform_id platform;
    CL_CALL(clGetPlatformIDs(1, &platform, NULL));

    CL_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL));

    CL_OBJECT_CALL(, context, clCreateContext(NULL, 1, &device, NULL, NULL, &err));

    CL_OBJECT_CALL(, queue, clCreateCommandQueue(context, device, 0, &err));

    char extensions[1024];
    clGetDeviceInfo(device, CL_DEVICE_EXTENSIONS, sizeof(extensions), extensions, NULL);
    if (strstr(extensions, "cl_khr_fp64") != NULL) double_support = true;
}

void parallel_computing_exit() {
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}

bool is_double_supported() {
    return double_support;
}


void* create_kernel(char* src) {
    struct program_resources* resources = malloc(sizeof(struct program_resources));
    if (resources == NULL) return NULL;
    resources->arg_count = 0;

    int i = 0;
    for (; src[i] != '('; i++);
    for (; src[i] == ' '; i++);
    for (; src[i] != ')'; resources->arg_count++) {

        if (src[i] == 'c' && src[i + 1] == 'h' && src[i + 2] == 'a' && src[i + 3] == 'r') {
            i += 4;
            resources->arg_types[resources->arg_count] = ARG_CHAR;
        }
        else if (src[i] == 's' && src[i + 1] == 'h' && src[i + 2] == 'o' && src[i + 3] == 'r' && src[i + 4] == 't') {
            i += 5;
            resources->arg_types[resources->arg_count] = ARG_SHORT;
        }
        else if (src[i] == 'i' && src[i + 1] == 'n' && src[i + 2] == 't') {
            i += 3;
            resources->arg_types[resources->arg_count] = ARG_INT;
        }
        else if (src[i] == 'l' && src[i + 1] == 'o' && src[i + 2] == 'n' && src[i + 3] == 'g') {
            i += 4;
            resources->arg_types[resources->arg_count] = ARG_LONG;
        }
        else if (src[i] == 'f' && src[i + 1] == 'l' && src[i + 2] == 'o' && src[i + 3] == 'a' && src[i + 4] == 't') {
            i += 5;
            resources->arg_types[resources->arg_count] = ARG_FLOAT;
        }
        else if (src[i] == 'd' && src[i + 1] == 'o' && src[i + 2] == 'u' && src[i + 3] == 'b' && src[i + 4] == 'l' && src[i + 5] == 'e') {
            i += 6;
            resources->arg_types[resources->arg_count] = ARG_DOUBLE;
        }
        else {
            for (; src[i] != '*'; i++);
            i++;
            resources->arg_types[resources->arg_count] = ARG_POINTER;
        }

        for (; src[i] == ' '; i++);
        for (; src[i] != ' ' && src[i] != ')'; i++);
        for (; src[i] == ' '; i++);
    }

    i = 0;
    for (; src[i] != '_'; i++);
    for (; src[i] != '_'; i++);
    for (; src[i] != 'k'; i++);
    for (; src[i] != 'e'; i++);
    for (; src[i] != 'r'; i++);
    for (; src[i] != 'n'; i++);
    for (; src[i] != 'e'; i++);
    for (; src[i] != 'l'; i++);

    for (; src[i] != 'd'; i++);
    i++;
    for (; src[i] == ' '; i++);

    int kernel_name_start = i;

    for (; src[i] != ' ' && src[i] != '('; i++);

    char* kernel_name = malloc(i - (long long)kernel_name_start + 1);
    if (kernel_name == NULL) {
        free(resources);
        return NULL;
    }

    CL_OBJECT_CALL(, resources->program, clCreateProgramWithSource(context, 1, (const char**)&src, NULL, &err));

    CL_CALL(clBuildProgram(resources->program, 1, &device, NULL, NULL, NULL));
    
    for (int j = kernel_name_start; j < i; j++) kernel_name[j - kernel_name_start] = src[j];
    kernel_name[i - kernel_name_start] = '\0';

    CL_OBJECT_CALL(, resources->kernel, clCreateKernel(resources->program, kernel_name, &err));

    free(kernel_name);
    return resources;
}

void* create_device_buffer(size_t size) {
    CL_OBJECT_CALL(cl_mem, device_buffer, clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, &err));
    return device_buffer;
}

void write_to_device_buffer(void* device_buffer, void* host_buffer, size_t size, size_t offset) {
    clEnqueueWriteBuffer(queue, device_buffer, CL_FALSE, offset, size, host_buffer, 0, NULL, NULL);
}

void read_from_device_buffer(void* device_buffer, void* host_buffer, size_t size, size_t offset) {
    clEnqueueReadBuffer(queue, device_buffer, CL_FALSE, offset, size, host_buffer, 0, NULL, NULL);
}

void destroy_device_buffer(void* memory_buffer) {
    clReleaseMemObject(memory_buffer);
}

void run_kernel(void* kernel, char dimension, int dim_x, int dim_y, int dim_z, ...) {
    struct program_resources* resources = (struct program_resources*)kernel;

    va_list args;
    va_start(args, dim_z);

    for (int i = 0; i < resources->arg_count; i++) {
        
        switch (resources->arg_types[i]) {
            
        case ARG_CHAR: {
            char char_arg = va_arg(args, int);
            CL_CALL(clSetKernelArg(resources->kernel, i, sizeof(char), &char_arg));
            break;  
        }
            
        case ARG_SHORT: {
            short short_arg = va_arg(args, int);
            CL_CALL(clSetKernelArg(resources->kernel, i, sizeof(short), &short_arg));
            break;
        }

        case ARG_INT: {
            int int_arg = va_arg(args, int);
            CL_CALL(clSetKernelArg(resources->kernel, i, sizeof(int), &int_arg));
            break;
        }

        case ARG_LONG: {
            long long long_arg = va_arg(args, long long);
            CL_CALL(clSetKernelArg(resources->kernel, i, sizeof(long long), &long_arg));
            break;
        }

        case ARG_FLOAT: {
            float float_arg = va_arg(args, double);
            CL_CALL(clSetKernelArg(resources->kernel, i, sizeof(float), &float_arg));
            break;
        }

        case ARG_DOUBLE: {
            double double_arg = va_arg(args, double);
            CL_CALL(clSetKernelArg(resources->kernel, i, sizeof(double), &double_arg));
            break;
        }

        case ARG_POINTER: {
            void* pointer_arg = va_arg(args, void*);
            CL_CALL(clSetKernelArg(resources->kernel, i, sizeof(cl_mem), &pointer_arg));
            break;
        }

        }
    }

    size_t globalItemSize[3] = { dim_x, dim_y, dim_z};
    CL_CALL(clEnqueueNDRangeKernel(queue, resources->kernel, dimension, NULL, globalItemSize, NULL, 0, NULL, NULL));

    return;
}

void wait_for_parallel_execution_completion() {
    clFinish(queue);
}

void destroy_kernel(void* kernel) {
    struct program_resources* resources = (struct program_resources*)kernel;

    clReleaseKernel(resources->kernel);
    clReleaseProgram(resources->program);
}