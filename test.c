#include "concurrent.h"
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"


buffer_t* bufferUnitary;
buffer_t* buffer;

/* ============================ STARTING SUITE CASES ============================*/
/*SUITE K-SIZED BUFFER*/
int init_createBuffer(void) {
    buffer = buffer_init(4);
    if(buffer != NULL) return 0;
    else return 1;
}

int clean_destroyBuffer(void) {
    buffer_destroy(buffer);
    if(buffer != NULL) return 0;
    else return 1;
}
/* END K-SIZED BUFFER SUITE */

/*  SUITE UNITARY BUFFER */
int init_createBufferUnitary(void) {
    bufferUnitary = buffer_init(1);
    if(bufferUnitary != NULL) return 0;
    else return 1;
}

int clean_destroyBufferUnitary(void) {
    buffer_destroy(bufferUnitary);
    if(bufferUnitary != NULL) return 0;
    else return 1;
}
/* END UNITARY BUFFER SUITE */

/* ============================ STARTING TEST CASES ============================*/

/* STACK TEST UNITARY BUFFER*/

// un buffer unitario ha dimensione 1
void test_unitaryDimension(void) {
    if(bufferUnitary != NULL)
        CU_ASSERT(1 == bufferUnitary->size);
}

// un buffer appena creato ha esattamente uno slot libero
void test_emptyNewUnitaryBufferFreeSlot(void) {
    if(bufferUnitary != NULL)
        CU_ASSERT(1 == slotLiberi(bufferUnitary));
}

/* END UNITARY BUFFER STACK TEST */

/* ============================ ENDING TEST CASES ============================*/

/* MAIN */
int main() {

    CU_pSuite cUnitBufferUnitary = NULL;
    CU_pSuite cUnitBuffer = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    cUnitBufferUnitary = CU_add_suite("Adding Unitary Buffer Suite Creation.", init_createBufferUnitary, clean_destroyBufferUnitary);
    if(NULL == cUnitBufferUnitary) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /*ADDING UNITARY STACK TEST*/
    if( CU_add_test(cUnitBufferUnitary, "Test[0] | Unitary Dimension\t", test_unitaryDimension) == NULL ||
        CU_add_test(cUnitBufferUnitary, "Test[1] | Free Slots (Buffer just created)\t", test_emptyNewUnitaryBufferFreeSlot) == NULL
        ) {
        CU_cleanup_registry();
        return CU_get_error();
       }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
