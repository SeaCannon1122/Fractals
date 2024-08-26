#ifndef UTILS_H
#define UTILS_H

struct v2d_double {
	double x;
	double y;
};

struct v2d_int {
	int x;
	int y;
};

int string_length(char* str);

char* parse_file(const char* filename);

#endif // !UTILS_H
