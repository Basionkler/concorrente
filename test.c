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

// Test[0] - un buffer unitario ha dimensione 1
void test_unitaryDimension(void) {
    if(bufferUnitary != NULL)
        CU_ASSERT(1 == bufferUnitary->size);
}

// Test[1] - un buffer appena creato ha esattamente uno slot libero
void test_emptyNewUnitaryBufferFreeSlot(void) {
    if(bufferUnitary != NULL) {
        CU_ASSERT(1 == bufferUnitary->freeSlots);
        CU_ASSERT(0 == bufferUnitary->produce);
        CU_ASSERT(0 == bufferUnitary->consume);
    }
}

// Test[2] - (P=1; C=0; N=1) Produzione di un solo messaggio in un buffer vuoto
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

// Test[3] - (P=1; C=0; N=1) Produzione in un buffer pieno
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

// Test[4] - (P=0; C=1; N=1) Consumazione di un solo messaggio da un buffer pieno
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

// Test[5] - (P=0; C=1; N=1) Consumazione da un buffer vuoto
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

// Test [6] - (P=1; C=1; N=1) Consumazione e produzione concorrente di un messaggio in un buffer unitario; prima il consumatore
void test_getAndPutUnitaryBuffer(void) {
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

		pthread_create(&cons, NULL, args_get_bloccante, bufferUnitary);
		pthread_create(&prod, NULL, args_put_bloccante, &args);
		
		pthread_join(cons, (void*)&expectedGet);
		pthread_join(prod, (void*)&expectedPut);
		
		CU_ASSERT(1 == bufferUnitary->freeSlots); //Verifico che è vuoto
		CU_ASSERT(0 == strcmp(expectedGet->content, expectedPut->content)); //Verifico che il messaggio estratto è effettivamente quello inserito
	}
}

// Test[7] - (P>1; C=0; N=1) Produzione concorrente di molteplici messaggi in un buffer unitario vuoto
void test_multiplePutUnitaryBuffer(void) {
	msg_t* msg_a = msg_init_string("EXPECTED_MESSAGE_A");
	msg_t* msg_b = msg_init_string("EXPECTED_MESSAGE_B");
	msg_t* msg_c = msg_init_string("EXPECTED_MESSAGE_C");
	msg_t* expected_a;
	msg_t* expected_b;
	msg_t* expected_c;

	arg_t args_a;
	args_a.buffer = bufferUnitary;
	args_a.msg = msg_a;

	arg_t args_b;
	args_b.buffer = bufferUnitary;
	args_b.msg = msg_b;

	arg_t args_c;
	args_c.buffer = bufferUnitary;
	args_c.msg = msg_c;

	if(bufferUnitary != NULL) {
		pthread_t prod_a;
		pthread_t prod_b;
		pthread_t prod_c;
		
		CU_ASSERT(1 == bufferUnitary->freeSlots); //Verifico che è vuoto
		pthread_create(&prod_a, NULL, args_put_non_bloccante, &args_a);
		pthread_create(&prod_b, NULL, args_put_non_bloccante, &args_b);
		pthread_create(&prod_c, NULL, args_put_non_bloccante, &args_c);

		pthread_join(prod_a, (void*)&expected_a);
		pthread_join(prod_b, (void*)&expected_b);
		pthread_join(prod_c, (void*)&expected_c);

		CU_ASSERT(0 == bufferUnitary->freeSlots); //Verifico che è pieno

		if(expected_c != NULL && 0 == strcmp(expected_c->content, msg_c->content)) {
			printf("\tC_THREAD HA INSERITO\t");
			CU_ASSERT_EQUAL(expected_b, BUFFER_ERROR); //Verifico che non c'è inserimento
			CU_ASSERT_EQUAL(expected_a, BUFFER_ERROR); //Verifico che non c'è inserimento
			CU_ASSERT(0 == strcmp(expected_c->content, (char*)bufferUnitary->message[0].content)); //Verifico che il contenuto del buffer è esattamente quello inserito dal thread specifico
		} else if (expected_b != NULL && 0 == strcmp(expected_b->content, msg_b->content)){
			printf("\tB_THREAD HA INSERITO\t");
			CU_ASSERT_EQUAL(expected_a, BUFFER_ERROR); //Verifico che non c'è inserimento
			CU_ASSERT_EQUAL(expected_c, BUFFER_ERROR); //Verifico che non c'è inserimento
			CU_ASSERT(0 == strcmp(expected_b->content, (char*)bufferUnitary->message[0].content)); //Verifico che il contenuto del buffer è esattamente quello inserito dal thread specifico
		} else if (expected_a != NULL && 0 == strcmp(expected_a->content, msg_a->content)) {
			printf("\tA_THREAD HA INSERITO\t");
			CU_ASSERT_EQUAL(expected_c, BUFFER_ERROR); //Verifico che non c'è inserimento
			CU_ASSERT_EQUAL(expected_b, BUFFER_ERROR); //Verifico che non c'è inserimento
			CU_ASSERT(0 == strcmp(expected_a->content, (char*)bufferUnitary->message[0].content)); //Verifico che il contenuto del buffer è esattamente quello inserito dal thread specifico
		}

		CU_ASSERT(0 == bufferUnitary->freeSlots); //Buffer pieno
	}
}

// Test[8] - (P=1; C=1; N=1) Consumazione e produzione concorrente di un messaggio da un buffer unitario; prima il produttore
void test_putAndGetUnitaryBuffer(void) {

	bufferUnitary->message[0].content = "EXPECTED_MESSAGE"; //Elimino '_X' finale del test precedente

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

		pthread_create(&prod, NULL, args_put_bloccante, &args);
		pthread_create(&cons, NULL, args_get_bloccante, bufferUnitary);

		pthread_join(cons, (void*)&expectedGet);
		pthread_join(prod, (void*)&expectedPut);

		CU_ASSERT(0 == strcmp(msg->content, expectedGet->content)); //Verifico che il messaggio estratto sia quello atteso
		CU_ASSERT(0 == strcmp(msg->content, expectedPut->content)); //Verifico che il messaggio inserito sia quello atteso

		CU_ASSERT(0 == bufferUnitary->freeSlots); //Verifico che il buffer è pieno
	}
}

// Test[9] - (P=0; C>1; N=1) Consumazione concorrente di molteplici messaggi da un buffer unitario pieno
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
		pthread_create(&cons_b, NULL, args_get_non_bloccante, bufferUnitary);
		pthread_create(&cons_c, NULL, args_get_non_bloccante, bufferUnitary);

		pthread_join(cons_a, (void*)&expected_a);
		pthread_join(cons_b, (void*)&expected_b);
		pthread_join(cons_c, (void*)&expected_c);

		if(expected_a != NULL && 0 == strcmp(expected_a->content, msg->content)) {
			printf("\tTHREAD_A HA CONSUMATO\t");
			CU_ASSERT_EQUAL(expected_b, BUFFER_ERROR);
			CU_ASSERT_EQUAL(expected_c, BUFFER_ERROR);
		} else if (expected_b != NULL && 0 == strcmp(expected_b->content, msg->content)) {
			printf("\tTHREAD_B HA CONSUMATO\t");
			CU_ASSERT_EQUAL(expected_a, BUFFER_ERROR);
			CU_ASSERT_EQUAL(expected_c, BUFFER_ERROR);
		} else if (expected_c != NULL && 0 == strcmp(expected_c->content, msg->content)) {
			printf("\tTHREAD_C HA CONSUMATO\t");
			CU_ASSERT_EQUAL(expected_b, BUFFER_ERROR);
			CU_ASSERT_EQUAL(expected_a, BUFFER_ERROR);
		}

		CU_ASSERT(1 == bufferUnitary->freeSlots); //Il buffer continua a essere vuoto
	} 
}

/*END UNITARY BUFFER (MULTI THREAD)*/

/* START BUFFER 4-SIZED TEST-CASES */

// Test[10] - (P>1; C=0; N>1) Produzione concorrente di molteplici messaggi in un buffer vuoto; il buffer si satura in corso
void test_fullfillingEmptyBuffer(void) {
	msg_t* msg_0 = msg_init_string("EXPECTED_MESSAGE_0");
	msg_t* msg_1 = msg_init_string("EXPECTED_MESSAGE_1");
	msg_t* msg_2 = msg_init_string("EXPECTED_MESSAGE_2");
	msg_t* msg_3 = msg_init_string("EXPECTED_MESSAGE_3");
	msg_t* msg_4 = msg_init_string("EXPECTED_MESSAGE_4");

	msg_t* expected_0;
	msg_t* expected_1;
	msg_t* expected_2;
	msg_t* expected_3;
	msg_t* expected_4;

	arg_t args_0;
	args_0.buffer = buffer;
	args_0.msg = msg_0;

	arg_t args_1;
	args_1.buffer = buffer;
	args_1.msg = msg_1;

	arg_t args_2;
	args_2.buffer = buffer;
	args_2.msg = msg_2;

	arg_t args_3;
	args_3.buffer = buffer;
	args_3.msg = msg_3;

	arg_t args_4;
	args_4.buffer = buffer;
	args_4.msg = msg_4;

	if(buffer != NULL) {
		pthread_t prod_0;
		pthread_t prod_1;
		pthread_t prod_2;
		pthread_t prod_3;
		pthread_t prod_4;

		CU_ASSERT(4 == buffer->freeSlots) // Verifico che è vuoto

		pthread_create(&prod_0, NULL, args_put_non_bloccante, &args_0);
		pthread_create(&prod_1, NULL, args_put_non_bloccante, &args_1);
		pthread_create(&prod_2, NULL, args_put_non_bloccante, &args_2);
		pthread_create(&prod_3, NULL, args_put_non_bloccante, &args_3);
		pthread_create(&prod_4, NULL, args_put_non_bloccante, &args_4);

		pthread_join(prod_0, (void*)&expected_0);
		pthread_join(prod_1, (void*)&expected_1);
		pthread_join(prod_2, (void*)&expected_2);
		pthread_join(prod_3, (void*)&expected_3);
		pthread_join(prod_4, (void*)&expected_4);

		CU_ASSERT(0 == buffer->freeSlots); // Verifico che è pieno

		/*thread 4 non inserisce */
		if( expected_0 != NULL && 0 == strcmp(expected_0->content, msg_0->content) && //Verifico che non hanno BUFFER_ERROR
			expected_1 != NULL && 0 == strcmp(expected_1->content, msg_1->content) &&
			expected_2 != NULL && 0 == strcmp(expected_2->content, msg_2->content) &&
			expected_3 != NULL && 0 == strcmp(expected_3->content, msg_3->content)
			) {
			CU_ASSERT_EQUAL(expected_4, BUFFER_ERROR);
			printf(" THREAD_4 NON INSERISCE\t");
			/*
			CU_ASSERT(1 == contains(buffer, expected_0)); // è presente nel buffer
			CU_ASSERT(1 == contains(buffer, expected_1));
			CU_ASSERT(1 == contains(buffer, expected_2));
			CU_ASSERT(1 == contains(buffer, expected_3));
			CU_ASSERT(0 == contains(buffer, expected_4)); // message_4 non presente nel buffer
			*/
		} else if ( //thread_0 non inserisce
			expected_1 != NULL && 0 == strcmp(expected_1->content, msg_1->content) &&
			expected_2 != NULL && 0 == strcmp(expected_2->content, msg_2->content) &&
			expected_3 != NULL && 0 == strcmp(expected_3->content, msg_3->content) &&
			expected_4 != NULL && 0 == strcmp(expected_4->content, msg_4->content)
			) {
				CU_ASSERT_EQUAL(expected_0, BUFFER_ERROR);
				printf(" THREAD_0 NON INSERISCE\t");
				/*
				CU_ASSERT(1 == contains(buffer, expected_1)); // è presente nel buffer
				CU_ASSERT(1 == contains(buffer, expected_2));
				CU_ASSERT(1 == contains(buffer, expected_3));
				CU_ASSERT(1 == contains(buffer, expected_4));
				CU_ASSERT(0 == contains(buffer, expected_0)); // message_0 non presente nel buffer
				*/
		} else if ( //thread_1 non inserisce
			expected_2 != NULL && 0 == strcmp(expected_2->content, msg_2->content) &&
			expected_3 != NULL && 0 == strcmp(expected_3->content, msg_3->content) &&
			expected_4 != NULL && 0 == strcmp(expected_4->content, msg_4->content) &&
			expected_0 != NULL && 0 == strcmp(expected_0->content, msg_0->content)
			) {
				CU_ASSERT_EQUAL(expected_1, BUFFER_ERROR);
				printf(" THREAD_1 NON INSERISCE\t");
				/*
				CU_ASSERT(1 == contains(buffer, expected_0)); // è presente nel buffer
				CU_ASSERT(1 == contains(buffer, expected_2));
				CU_ASSERT(1 == contains(buffer, expected_3));
				CU_ASSERT(1 == contains(buffer, expected_4));
				CU_ASSERT(0 == contains(buffer, expected_1)); // message_1 non presente nel buffer
				*/
		} else if ( //thread_2 non inserisce
			expected_3 != NULL && 0 == strcmp(expected_3->content, msg_3->content) &&
			expected_4 != NULL && 0 == strcmp(expected_4->content, msg_4->content) &&
			expected_0 != NULL && 0 == strcmp(expected_0->content, msg_0->content) &&
			expected_1 != NULL && 0 == strcmp(expected_1->content, msg_1->content)
			) {
				CU_ASSERT_EQUAL(expected_2, BUFFER_ERROR);
				printf(" THREAD_2 NON INSERISCE\t");
				/*
				CU_ASSERT(1 == contains(buffer, expected_1)); // è presente nel buffer
				CU_ASSERT(1 == contains(buffer, expected_3));
				CU_ASSERT(1 == contains(buffer, expected_4));
				CU_ASSERT(1 == contains(buffer, expected_0));
				CU_ASSERT(0 == contains(buffer, expected_2)); // message_2 non presente nel buffer
				*/
		} else if ( //thread_3 non inserisce
			expected_4 != NULL && 0 == strcmp(expected_4->content, msg_4->content) &&
			expected_0 != NULL && 0 == strcmp(expected_0->content, msg_0->content) &&
			expected_1 != NULL && 0 == strcmp(expected_1->content, msg_1->content) &&
			expected_2 != NULL && 0 == strcmp(expected_2->content, msg_2->content)
			) {
				CU_ASSERT_EQUAL(expected_3, BUFFER_ERROR);
				printf(" THREAD_3 NON INSERISCE\t");
				/*
				CU_ASSERT(1 == contains(buffer, expected_1)); // è presente nel buffer
				CU_ASSERT(1 == contains(buffer, expected_2));
				CU_ASSERT(1 == contains(buffer, expected_4));
				CU_ASSERT(1 == contains(buffer, expected_0));
				CU_ASSERT(0 == contains(buffer, expected_3)); // message_3 non presente nel buffer
				*/
		} 
	}
}

// TEST[11] - (P>1; C=0; N>1) Produzione concorrente di molteplici messaggi in un buffer pieno; il buffer è già saturo
void test_multiplePutFullBuffer(void) {
	msg_t* msg_0 = msg_init_string("MESSAGE");
	msg_t* msg_1 = msg_init_string("MESSAGE");
	msg_t* msg_2 = msg_init_string("MESSAGE");

	msg_t* expected_0;
	msg_t* expected_1;
	msg_t* expected_2;

	arg_t args_0;
	args_0.buffer = buffer;
	args_0.msg = msg_0;

	arg_t args_1;
	args_1.buffer = buffer;
	args_1.msg = msg_1;

	arg_t args_2;
	args_2.buffer = buffer;
	args_2.msg = msg_2;

	if(buffer != NULL) {
		pthread_t prod_0;
		pthread_t prod_1;
		pthread_t prod_2;

		CU_ASSERT(0 == buffer->freeSlots); //Verifico che il buffer è gia pieno

		pthread_create(&prod_0, NULL, args_put_non_bloccante, &args_0);
		pthread_create(&prod_1, NULL, args_put_non_bloccante, &args_1);
		pthread_create(&prod_2, NULL, args_put_non_bloccante, &args_2);

		pthread_join(prod_0, (void*)&expected_0);
		pthread_join(prod_1, (void*)&expected_1);
		pthread_join(prod_2, (void*)&expected_2);

		CU_ASSERT(0 == buffer->freeSlots); //verifico che è ancora pieno
		CU_ASSERT_EQUAL(expected_0, BUFFER_ERROR);
		CU_ASSERT_EQUAL(expected_1, BUFFER_ERROR);
		CU_ASSERT_EQUAL(expected_2, BUFFER_ERROR);
	}
}

// Test[12] - (P=0; C>1; N>1) Consumazione concorrente di molteplici messaggi da un buffer pieno
void test_multipleGetFullBuffer(void) {
	msg_t expected_0;
	msg_t expected_1;
	msg_t expected_2;
	msg_t expected_3;

	if(buffer != NULL) {
		pthread_t cons_0;
		pthread_t cons_1;
		pthread_t cons_2;
		pthread_t cons_3;

		printf("\n\t\t%s\t", (char*)buffer->message[0].content);
		printf("\n\t\t%s\t", (char*)buffer->message[1].content);
		printf("\n\t\t%s\t", (char*)buffer->message[2].content);
		printf("\n\t\t%s\t", (char*)buffer->message[3].content);

		CU_ASSERT(0 == buffer->freeSlots); //Verifico che il buffer è pieno

		
		pthread_create(&cons_0, NULL, args_get_non_bloccante, buffer);
		pthread_create(&cons_1, NULL, args_get_non_bloccante, buffer);
		pthread_create(&cons_2, NULL, args_get_non_bloccante, buffer);
		pthread_create(&cons_3, NULL, args_get_non_bloccante, buffer);

		pthread_join(cons_0, (void*)&expected_0);
		pthread_join(cons_1, (void*)&expected_1);
		pthread_join(cons_2, (void*)&expected_2);
		pthread_join(cons_3, (void*)&expected_3);

		printf("\n\t\t%s\t", (char*)expected_0.content);
		printf("\n\t\t%s\t", (char*)expected_1.content);
		printf("\n\t\t%s\t", (char*)expected_2.content);
		printf("\n\t\t%s\t", (char*)expected_3.content);
		CU_ASSERT(4 == buffer->freeSlots) //Verifico che ora il buffer è vuoto
	}
}

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

    /* SUITE DI CREAZIONE BUFFER 4-SIZED MULTI THREAD */
    cUnitBuffer = CU_add_suite("Adding 4-SIZED buffer suite creation...", init_createBuffer, clean_destroyBuffer);
    if(NULL == cUnitBuffer) {
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

    /*ADDING UNITARY-MULTI-THREAD STACK TEST*/
    if( CU_add_test(cUnitBufferUnitary_multi, "Test[6] | Consumazione e produzione concorrente di un messaggio in un buffer unitario; prima il consumatore", test_getAndPutUnitaryBuffer) == NULL ||
    	CU_add_test(cUnitBufferUnitary_multi, "Test[7] | Produzione concorrente di molteplici messaggi in un buffer unitario vuoto", test_multiplePutUnitaryBuffer) == NULL ||
    	CU_add_test(cUnitBufferUnitary_multi, "Test[8] | Consumazione e produzione concorrente di un messaggio in un buffer unitario; prima il produttore", test_putAndGetUnitaryBuffer) == NULL ||
    	CU_add_test(cUnitBufferUnitary_multi, "Test[9] | Consumazione concorrente di molteplici messaggi da un buffer unitario pieno", test_multipleGetUnitaryFullBuffer) == NULL
    	) {
    		CU_cleanup_registry();
	        return CU_get_error();
    }

    /* ADDING 4-SIZED-MULTI-THREAD STACK TEST */
    if( CU_add_test(cUnitBuffer, "Test[10] | Produzione concorrente di molteplici messaggi in un buffer vuoto; il buffer si satura in corso", test_fullfillingEmptyBuffer) == NULL ||
    	CU_add_test(cUnitBuffer, "Test[11] | Produzione concorrente di molteplici messaggi in un buffer pieno; il buffer è già saturo", test_multiplePutFullBuffer) == NULL ||
    	CU_add_test(cUnitBuffer, "Test[12] | Consumazione concorrente di molteplici messaggi da un buffer pieno", test_multipleGetFullBuffer) == NULL
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
