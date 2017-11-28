#include "concurrent.h"
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"


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

/* TEST STACK */

void test_unitaryDimension(void) {
    if(bufferUnitary != NULL)
        CU_ASSERT(1 == bufferUnitary->size);
}

/* MAIN */
int main() {

    CU_pSuite cUnitBufferUnitary = NULL;
    CU_pSuite cUnitBuffer = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    cUnitBufferUnitary = CU_add_suite("Adding Unitary Buffer Suite Creation:\n", init_createBufferUnitary, clean_destroyBufferUnitary);
    if(NULL == cUnitBufferUnitary) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /*ADDING FIRST STACK*/
    if(NULL == CU_add_test(cUnitBufferUnitary, "Test[0] | Unitary Dimension:\t", test_unitaryDimension)) {
        CU_cleanup_registry();
        return CU_get_error();
       }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
