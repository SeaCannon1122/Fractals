#include <stdio.h>
#include <CL/cl.h>
#include "platform.h"

#define CL_CALL(call)\
do {\
cl_int err = (call);\
if (err != CL_SUCCESS) {\
fprintf(stderr, "OpenCL error in \n     %s \n  at %s:%d: %d\n^\n",#call, __FILE__, __LINE__, err);\
while ((get_key_state(KEY_ESCAPE) & 1) == 0) sleep_for_ms(100);\
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
while ((get_key_state(KEY_ESCAPE) & 1) == 0) sleep_for_ms(100);\
exit(1);\
}\
} while(0)

cl_program create_program(cl_context context, cl_device_id device, const char* source);

cl_kernel create_kernel(cl_program program, const char* kernel_name);