#include <CUnit/CUnit.h>
#include "concurrent.h"

buffer_t* bufferUnitary;
buffer_t* buffer;

/* SERVICE FUNCTIONS */
int init_createBuffer(void) {
    buffer = buffer_init(4);
    return buffer != NULL;
}

int init_createBufferUnitary(void) {
    bufferUnitary = buffer_init(1);
    return bufferUnitary != NULL;
}

int clean_destroyBuffer(void) {
    buffer_destroy(buffer);
    return buffer == NULL;
}

int clean_destroyBufferUnitary(void) {
    buffer_destroy(bufferUnitary);
    return bufferUnitary == NULL;
}
