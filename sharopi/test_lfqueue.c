#include <string.h>
#include <stdio.h>

#define LFQ_IMPLEMENT
#include "lfqueue.h"

#define MAKE_QUEUE(sz) \
	unsigned char queue_buffer[sz]; \
	struct LFQueue queue; \
	lfqInit(&queue, sizeof queue_buffer, queue_buffer)

#define TEST_EQUAL(T, a, b, FMT) { \
		const T a_val = (a); \
		const T b_val = (b); \
		if (a_val != b_val) { \
			++errcnt; \
			printf("%s:%d: ("#a") "FMT" != "FMT" ("#b")\n", __FILE__, __LINE__, a_val, b_val); \
		} \
	}

#if 0
#define TEST_WRITE(arr, size, retval) { \
	const unsigned char wbuf[] = arr;	\
	TEST_EQUAL(LFQSize, retval, lfqWrite(&queue, size, wbuf), "%u"); \
	}
#endif

#define TEST_WRITE(size, retval, ...) { \
	const unsigned char wbuf[] = { __VA_ARGS__ };	\
	TEST_EQUAL(LFQSize, retval, lfqWrite(&queue, size, wbuf), "%u"); \
	}

#define TEST_READ(size, retval, ...) { \
		const unsigned char expect[size] = { __VA_ARGS__ }; \
		unsigned char rbuf[size] = { __VA_ARGS__ }; \
		TEST_EQUAL(LFQSize, retval, lfqRead(&queue, size, rbuf), "%u"); \
		TEST_EQUAL(int, 0, memcmp(rbuf, expect, size), "%d"); \
	}

int trivial_st() {
	int errcnt = 0;
	unsigned char buffer[8];
	MAKE_QUEUE(8);

	TEST_EQUAL(LFQSize, 0, lfqRead(&queue, 0, buffer), "%u");
	TEST_EQUAL(LFQSize, 0, lfqRead(&queue, 1, buffer), "%u");
	TEST_EQUAL(LFQSize, 0, lfqRead(&queue, 7, buffer), "%u");
	TEST_EQUAL(LFQSize, 0, lfqRead(&queue, 8, buffer), "%u");
	TEST_EQUAL(LFQSize, 0, lfqRead(&queue, 9, buffer), "%u");

	TEST_WRITE(1, 0, 23);
	TEST_WRITE(7, 1, 10, 20, 30, 40, 50, 60, 70);
	TEST_WRITE(6, 0, 10, 20, 30, 40, 50, 60);
	TEST_WRITE(1, 1, 10);

	TEST_READ(8, 7, 23, 10, 20, 30, 40, 50, 60);

	/*
	TEST_EQUAL(LFQSize, 0, lfqWrite(&queue, 1, buffer), "%u");
	TEST_EQUAL(LFQSize, 2, lfqWrite(&queue, 8, buffer), "%u");
	TEST_EQUAL(LFQSize, 0, lfqWrite(&queue, 6, buffer), "%u");
	*/

	return errcnt;
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	int errcnt = trivial_st();
	printf("Errors: %d\n", errcnt);
	return errcnt;
}
