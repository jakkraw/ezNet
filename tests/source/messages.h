#pragma once
#include <cstdio>
#include <cstring>

struct Greet {
	int i = 5;
	explicit Greet(int i) : i(i){}
	void print() const { printf("Hello from nr: %d\n", i); }
};

struct Text100 {
	char text[100]{};

	Text100(const char* c) { memcpy_s(text, sizeof(text), c, strlen(c) + 1); }

	void print() const { printf("%s\n", text); }
};
