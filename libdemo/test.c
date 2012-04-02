#include <stdio.h>
#include <stdint.h>

uint32_t myrand(uint32_t v)
{
	v *= 1664525;
	v += 1013904223;
	return v;
}

int main()
{
	uint32_t r = 0;
	int i = 0;
	for (i = 0; i < 30; ++i) {
		r = myrand(r);
		printf("%08x\n", r);
	}
	return 0;
}
