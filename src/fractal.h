#ifndef MANDELBROTSET_H
#define MANDELBROTSET_H

#define fractal_from_equation(name, Re, Im) "__kernel void " name "(__global unsigned int* image, int width, int height, double pixelSize, double centerX, double centerY, int maxIter, int divRadius) {\n\n int x = get_global_id(0);\nint y = get_global_id(1);\n\ndouble Re = 0;\ndouble Im = 0;\n\ndouble cRe = centerX + (x - width / 2.0) * pixelSize;\ndouble cIm = (centerY + (height / 2.0 - y - 1) * pixelSize);\n\nint n = 0;\nwhile (Re * Re + Im * Im < divRadius * divRadius && n < maxIter) { \ndouble Re_new = " Re ";\ndouble Im_new = " Im ";\n\nRe = Re_new;\nIm = Im_new;\nn++;\n}\n\nunsigned int color;\nif (n == maxIter) {color = 0xFF000000;}\nelse {\ndouble t = (double)n / maxIter;\nunsigned int r = (unsigned int)(9 * (1 - t) * t * t * t * 255);\nunsigned int g = (unsigned int)(15 * (1 - t) * (1 - t) * t * t * 255);\nunsigned int b = (unsigned int)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);\ncolor = (0xFF << 24) | (r << 16) | (g << 8) | b;\n}\nimage[y * width + x] = color;\n}"

void* setup_fractal(const char* source, const char* kernel_name);

unsigned int* render_fractal(void* resources, struct v2d_double position, double pixel_size, int width, int height, int max_iterations, int divergence_radius);

void cleanup_fractal(void* setup);

#endif // !MANDELBROTSET_H
