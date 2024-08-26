__kernel void mandelbrot(__global unsigned int* image, int width, int height, double pixelSize, double centerX, double centerY, int maxIter, int divRadius) {

	int x = get_global_id(0);
	int y = get_global_id(1);

	double Re = 0;
	double Im = 0;

	double cRe = centerX + (x - width / 2.0) * pixelSize;
	double cIm = (centerY + (height / 2.0 - y - 1) * pixelSize);

	int n = 0;

	while (Re * Re + Im * Im < divRadius * divRadius && n < maxIter) {

		double Re_new = Re * Re - Im * Im + cRe;
		double Im_new = 2.0 * Re * Im + cIm;

		Re = Re_new;
		Im = Im_new;
		n++;
	}

	unsigned int color;

	if (n == maxIter) { color = 0xFF000000; }

	else {
		double t = (double)n / maxIter;

		unsigned int r = (unsigned int)(9 * (1 - t) * t * t * t * 255);
		unsigned int g = (unsigned int)(15 * (1 - t) * (1 - t) * t * t * 255);
		unsigned int b = (unsigned int)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);

		color = (0xFF << 24) | (r << 16) | (g << 8) | b;
	}

	image[y * width + x] = color;
}