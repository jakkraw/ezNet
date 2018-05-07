#pragma once
#include <cstdio>
#include <cstring>

struct Greet {
	int i = 5;
	void print() const { printf("Greets with number: %d\n", i); }
};

struct Goodbye {
	char text[20]{0};
	Goodbye(const char* c) { memcpy_s(text, 20, c, strlen(c)+1); }
	void print() const { printf("message with: %s\n", &text); }
};
