void memcpy(void* src, void* dst, unsigned long length) {
	unsigned char* c_src = (unsigned char*)src;
	unsigned char* c_dst = (unsigned char*)dst;
	unsigned long i;
	for (i = 0; i < length; i++) {
		*(c_dst+i) = *(c_src+i);
	}
}
