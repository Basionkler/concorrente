#include "concurrent.h"
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include <pthread.h>


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
        CU_ASSERT(1 == bufferUnitary->freeSlots);
}

// (P=1; C=0; N=1) Produzione di un solo messaggio in un buffer vuoto
void test_singleProductionEmptyUnitaryBuffer(void) {
	msg_t* msg = msg_init_string("EXPECTED_MESSAGE");
	msg_t* expected;

	arg_t args;
	args.buffer = bufferUnitary;
	args.msg = msg;

	if(bufferUnitary != NULL) {
		pthread_t thread;
		
		CU_ASSERT (1 == bufferUnitary->freeSlots);

		pthread_create(&thread, NULL, args_put_bloccante, &args);
		pthread_join(thread, (void*)&expected);

		CU_ASSERT_EQUAL(msg, expected);
		CU_ASSERT (0 == bufferUnitary->freeSlots); //Verifico che l'inserimento sia andato a buon fine
	}
}

//(P=0; C=1; N=1) Consumazione di un solo messaggio da un buffer pieno
void test_singleGetEmptyUnitaryBuffer(void) {

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
    if( CU_add_test(cUnitBufferUnitary, "Test[0] | Unitary Dimension", test_unitaryDimension) == NULL ||
        CU_add_test(cUnitBufferUnitary, "Test[1] | Free Slots (Buffer just created)", test_emptyNewUnitaryBufferFreeSlot) == NULL ||
        CU_add_test(cUnitBufferUnitary, "Test[2] | Produzione di un solo messaggio in un buffer vuoto", test_singleProductionEmptyUnitaryBuffer) == NULL
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
