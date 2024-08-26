#include "openclutils.h"

cl_program create_program(cl_context context, cl_device_id device, const char* source) {
    CL_OBJECT_CALL(cl_program, program, clCreateProgramWithSource(context, 1, &source, NULL, &err));

    CL_CALL(clBuildProgram(program, 1, &device, NULL, NULL, NULL));
    return program;
}

cl_kernel create_kernel(cl_program program, const char* kernel_name) {
    CL_OBJECT_CALL(cl_kernel, kernel, clCreateKernel(program, kernel_name, &err));
    return kernel;
}

cl_context create_context(cl_device_id* device) {
    cl_platform_id platform;
    CL_CALL(clGetPlatformIDs(1, &platform, NULL));

    cl_device_id devices[10];
    cl_uint num_devices;
    CL_CALL(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 10, devices, &num_devices));

    *device = devices[0];

    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0
    };

    CL_OBJECT_CALL(cl_context, context, clCreateContext(properties, 1, device, NULL, NULL, &err));
    return context;
}