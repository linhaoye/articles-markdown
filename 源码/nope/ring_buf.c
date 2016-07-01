#include "ring_buf.h"

struct ring_buf {
	void **data;
	int length;
	int start;
	int end;
};
