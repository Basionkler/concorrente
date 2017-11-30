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
    if(bufferUnitary != NULL) {
        CU_ASSERT(1 == bufferUnitary->freeSlots);
        CU_ASSERT(0 == bufferUnitary->produce);
        CU_ASSERT(0 == bufferUnitary->consume);
    }
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

// (P=1; C=0; N=1) Produzione in un buffer pieno
void test_singlePutUnitaryBuffer(void) {
	msg_t* msg = msg_init_string("EXPECTED_MESSAGE");
	msg_t* expected;

	arg_t args;
	args.buffer = bufferUnitary;
	args.msg = msg;

	if(bufferUnitary != NULL) {
		pthread_t thread;
		CU_ASSERT (0 == bufferUnitary->freeSlots); //Verifico che è già pieno
		pthread_create(&thread, NULL, args_put_non_bloccante, &args);
		pthread_join(thread, (void*)&expected);
		CU_ASSERT_EQUAL(expected, BUFFER_ERROR);
		CU_ASSERT (0 == bufferUnitary->freeSlots); //Verifico che è ancora pieno
	}
}

// (P=0; C=1; N=1) Consumazione di un solo messaggio da un buffer pieno
void test_singleGetFullUnitaryBuffer(void) {
	msg_t* msg = msg_init_string("EXPECTED_MESSAGE");
	msg_t* expected;

	if(bufferUnitary != NULL) {
		pthread_t thread;
		CU_ASSERT (0 == bufferUnitary->freeSlots); //Verifico che è già pieno
		pthread_create(&thread, NULL, args_get_bloccante, bufferUnitary);
		pthread_join(thread, (void*)&expected);
		CU_ASSERT(0 == strcmp(expected->content, msg->content)); //Verifico che il messaggio estratto sia quello atteso
		CU_ASSERT (1 == bufferUnitary->freeSlots); //Verifico che è vuoto
	}
}

// (P=0; C=1; N=1) Consumazione da un buffer vuoto
void test_singleGetEmptyUnitaryBuffer(void) {
	msg_t* msg;

	if(bufferUnitary != NULL) {
		pthread_t thread;
		CU_ASSERT (1 == bufferUnitary->freeSlots); // Verifico che è vuoto
		pthread_create(&thread, NULL, args_get_non_bloccante, bufferUnitary);
		pthread_join(thread, (void*)&msg);
		CU_ASSERT_EQUAL(msg, BUFFER_ERROR); //Verifico buffer error
	}
}

/* END UNITARY BUFFER STACK TEST */

/* STACK TEST UNITARY BUFFER (MULTI THREAD)*/

// (P=1; C=1; N=1) Consumazione e produzione concorrente di un messaggio in un buffer unitario; prima il produttore
void test_putAndGetUnitaryBuffer(void) {
	msg_t* msg = msg_init_string("EXPECTED_MESSAGE");
	msg_t* expectedPut;
	msg_t* expectedGet;

	arg_t args;
	args.buffer = bufferUnitary;
	args.msg = msg;

	if(bufferUnitary != NULL) {
		pthread_t prod;
		pthread_t cons;

		CU_ASSERT(1 == bufferUnitary->freeSlots); //Verifico che è vuoto
		pthread_create(&prod, NULL, args_put_non_bloccante, &args);
		pthread_join(prod, (void*)&expectedPut);
		CU_ASSERT(0 == bufferUnitary->freeSlots); //Verifico che è pieno

		pthread_create(&cons, NULL, args_get_bloccante, bufferUnitary);
		pthread_join(cons, (void*)&expectedGet);
		CU_ASSERT(1 == bufferUnitary->freeSlots); //Verifico che è vuoto
		CU_ASSERT(0 == strcmp(expectedGet->content, expectedPut->content)); //Verifico che il messaggio estratto è effettivamente quello inserito

	}
}

// (P>1; C=0; N=1) Produzione concorrente di molteplici messaggi in un buffer unitario vuoto
void test_multiplePutUnitaryBuffer(void) {
	msg_t* msg = msg_init_string("EXPECTED_MESSAGE");
	msg_t* expected_a;
	msg_t* expected_b;
	msg_t* expected_c;

	arg_t args;
	args.buffer = bufferUnitary;
	args.msg = msg;

	if(bufferUnitary != NULL) {
		pthread_t prod_a;
		pthread_t prod_b;
		pthread_t prod_c;
		
		CU_ASSERT(1 == bufferUnitary->freeSlots); //Verifico che è vuoto
		pthread_create(&prod_a, NULL, args_put_non_bloccante, &args);
		pthread_create(&prod_b, NULL, args_put_non_bloccante, &args);
		pthread_create(&prod_c, NULL, args_put_non_bloccante, &args);

		CU_ASSERT(0 == bufferUnitary->freeSlots); //Verifico che è pieno

		pthread_join(prod_a, (void*)&expected_a);
		pthread_join(prod_b, (void*)&expected_b);
		pthread_join(prod_c, (void*)&expected_c);

		if(expected_c != NULL && 0 == strcmp(expected_c->content, msg->content)) {
			CU_ASSERT_EQUAL(expected_b, BUFFER_ERROR); //Verifico che non c'è inserimento
			CU_ASSERT_EQUAL(expected_a, BUFFER_ERROR); //Verifico che non c'è inserimento
		} else if (expected_b != NULL && 0 == strcmp(expected_b->content, msg->content)){
			CU_ASSERT_EQUAL(expected_a, BUFFER_ERROR); //Verifico che non c'è inserimento
			CU_ASSERT_EQUAL(expected_c, BUFFER_ERROR); //Verifico che non c'è inserimento
		} else if (expected_a != NULL && 0 == strcmp(expected_a->content, msg->content)) {
			CU_ASSERT_EQUAL(expected_c, BUFFER_ERROR); //Verifico che non c'è inserimento
			CU_ASSERT_EQUAL(expected_b, BUFFER_ERROR); //Verifico che non c'è inserimento
		}
		CU_ASSERT(0 == bufferUnitary->freeSlots); //Buffer pieno
	}
}

// (P=1; C=1; N=1) Consumazione e produzione concorrente di un messaggio da un buffer unitario; prima il consumatore
void test_getAndPutUnitaryBuffer(void) {
	msg_t* msg = msg_init_string("EXPECTED_MESSAGE");
	msg_t* expectedPut;
	msg_t* expectedGet;

	arg_t args;
	args.buffer = bufferUnitary;
	args.msg = msg;
	if(bufferUnitary != NULL) {
		pthread_t cons;
		pthread_t prod;

		CU_ASSERT(0 == bufferUnitary->freeSlots); //Verifico che il buffer è pieno
		pthread_create(&cons, NULL, args_get_bloccante, bufferUnitary);
		pthread_join(cons, (void*)&expectedGet);
		CU_ASSERT(1 == bufferUnitary->freeSlots) //Verifico che è vuoto
		CU_ASSERT(0 == strcmp(msg->content, expectedGet->content)); //Verifico che il messaggio estratto sia quello atteso

		pthread_create(&prod, NULL, args_put_bloccante, &args);
		pthread_join(prod, (void*)&expectedPut);
		CU_ASSERT(0 == bufferUnitary->freeSlots); //Verifico che il buffer è pieno
		CU_ASSERT(0 == strcmp(msg->content, expectedPut->content)); //Verifico che il messaggio inserito sia quello atteso

	}
}

// (P=0; C>1; N=1) Consumazione concorrente di molteplici messaggi da un buffer unitario pieno
void test_multipleGetUnitaryFullBuffer(void) {
	msg_t* msg = msg_init_string("EXPECTED_MESSAGE");
	msg_t* expected_a;
	msg_t* expected_b;
	msg_t* expected_c;

	if(bufferUnitary != NULL) {
		pthread_t cons_a;
		pthread_t cons_b;
		pthread_t cons_c;

		CU_ASSERT(0 == bufferUnitary->freeSlots); //Verifico che il buffer è pieno
		pthread_create(&cons_a, NULL, args_get_non_bloccante, bufferUnitary);
		pthread_join(cons_a, (void*)&expected_a);
		CU_ASSERT(0 == strcmp(msg->content, expected_a->content)); //Verifico che il messaggio estratto sia quello atteso
		CU_ASSERT(1 == bufferUnitary->freeSlots); //Il buffer è vuoto

		pthread_create(&cons_b, NULL, args_get_non_bloccante, bufferUnitary);
		pthread_join(cons_b, (void*)&expected_b);
		CU_ASSERT_EQUAL(expected_b, BUFFER_ERROR);

		pthread_create(&cons_c, NULL, args_get_non_bloccante, bufferUnitary);
		pthread_join(cons_c, (void*)&expected_c);
		CU_ASSERT_EQUAL(expected_c, BUFFER_ERROR);
		CU_ASSERT(1 == bufferUnitary->freeSlots); //Il buffer continua a essere vuoto
	}
}

/*END UNITARY BUFFER (MULTI THREAD)*/

/* START BUFFER K-SIZED TEST-CASES */

// (P>1; C=0; N>1) Produzione concorrente di molteplici messaggi in un buffer vuoto; il buffer si satura in corso
void test_fullfillingEmptyBuffer(void) {
	msg_t* msg = msg_init_string("EXPECTED_MESSAGE");

	arg_t args;
	args.buffer = bufferUnitary;
	args.msg = msg;

	if(buffer != NULL) {

	}
}

// (P>1; C=0; N>1) Produzione concorrente di molteplici messaggi in un buffer pieno; il buffer è già saturo
void test_multiplePutFullBuffer(void) {

}

//(P=0; C>1; N>1) Consumazione concorrente di molteplici messaggi da un buffer pieno

//(P>1; C=0; N>1) Produzione concorrente di molteplici messaggi in un buffer vuoto; il buffer non si riempe
void test_multiplePutEmptyBuffer(void) {

}

/* END BUFFER K-SIZED TEST-CASES */

/* ============================ ENDING ALL TEST CASES ============================*/

/* MAIN */
int main() {

    CU_pSuite cUnitBufferUnitary = NULL;
    CU_pSuite cUnitBufferUnitary_multi = NULL;

    CU_pSuite cUnitBuffer = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* SUITE DI CREAZIONE BUFFER UNITARIO SINGLE THREAD */
    cUnitBufferUnitary = CU_add_suite("Adding unitary buffer suite creation...", init_createBufferUnitary, clean_destroyBufferUnitary);
    if(NULL == cUnitBufferUnitary) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* SUITE DI CREAZIONE BUFFER UNITARIO MULTI THREAD */
    cUnitBufferUnitary_multi = CU_add_suite("Adding Unitary Buffer Multi-Thread suite creation...", init_createBufferUnitary, clean_destroyBufferUnitary);
    if(NULL == cUnitBufferUnitary_multi) {
    	CU_cleanup_registry();
        return CU_get_error();
    }

    /*ADDING UNITARY-SINGLE-THREAD STACK TEST*/
    if( CU_add_test(cUnitBufferUnitary, "Test[0] | Dimensione unitaria", test_unitaryDimension) == NULL ||
        CU_add_test(cUnitBufferUnitary, "Test[1] | Slot liberi (Buffer unitario appena creato)", test_emptyNewUnitaryBufferFreeSlot) == NULL ||
        CU_add_test(cUnitBufferUnitary, "Test[2] | Produzione di un solo messaggio in un buffer vuoto", test_singleProductionEmptyUnitaryBuffer) == NULL ||
        CU_add_test(cUnitBufferUnitary, "Test[3] | Produzione in un buffer pieno", test_singlePutUnitaryBuffer) == NULL ||
        CU_add_test(cUnitBufferUnitary, "Test[4] | Consumazione di un singolo messaggio in un buffer pieno", test_singleGetFullUnitaryBuffer) == NULL ||
        CU_add_test(cUnitBufferUnitary, "Test[5] | Consumazione di un singolo messaggio in un buffer vuoto", test_singleGetEmptyUnitaryBuffer) == NULL ) 
    	{
	        CU_cleanup_registry();
	        return CU_get_error();
       }

    printf("\n\n");

    /*ADDING UNITARY-SINGLE-THREAD STACK TEST*/
    if( CU_add_test(cUnitBufferUnitary_multi, "Test[6] | Consumazione e produzione concorrente di un messaggio in un buffer unitario; prima il produttore", test_putAndGetUnitaryBuffer) == NULL ||
    	CU_add_test(cUnitBufferUnitary_multi, "Test[7] | Produzione concorrente di molteplici messaggi in un buffer unitario vuoto", test_multiplePutUnitaryBuffer) == NULL ||
    	CU_add_test(cUnitBufferUnitary_multi, "Test[8] | Consumazione e produzione concorrente di un messaggio in un buffer unitario; prima il consumatore", test_getAndPutUnitaryBuffer) == NULL ||
    	CU_add_test(cUnitBufferUnitary_multi, "Test[9] | Consumazione concorrente di molteplici messaggi da un buffer unitario pieno", test_multipleGetUnitaryFullBuffer) == NULL
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
