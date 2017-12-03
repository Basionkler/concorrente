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
			CU_ASSERT_EQUAL(expected_b, BUFFER_ERROR); //Verifico che non c'è inserimento
			CU_ASSERT_EQUAL(expected_a, BUFFER_ERROR); //Verifico che non c'è inserimento
			CU_ASSERT(0 == strcmp(expected_c->content, (char*)bufferUnitary->message[0].content)); //Verifico che il contenuto del buffer è esattamente quello inserito dal thread specifico
		} else if (expected_b != NULL && 0 == strcmp(expected_b->content, msg_b->content)){
			CU_ASSERT_EQUAL(expected_a, BUFFER_ERROR); //Verifico che non c'è inserimento
			CU_ASSERT_EQUAL(expected_c, BUFFER_ERROR); //Verifico che non c'è inserimento
			CU_ASSERT(0 == strcmp(expected_b->content, (char*)bufferUnitary->message[0].content)); //Verifico che il contenuto del buffer è esattamente quello inserito dal thread specifico
		} else if (expected_a != NULL && 0 == strcmp(expected_a->content, msg_a->content)) {
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
	msg_t* verify = msg_init_string("EXPECTED_MESSAGE");

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

		CU_ASSERT(0 == strcmp(verify->content, expectedGet->content)); //Verifico che il messaggio estratto sia quello atteso
		CU_ASSERT(0 == strcmp(verify->content, expectedPut->content)); //Verifico che il messaggio inserito sia quello atteso

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
			CU_ASSERT_EQUAL(expected_b, BUFFER_ERROR);
			CU_ASSERT_EQUAL(expected_c, BUFFER_ERROR);
		} else if (expected_b != NULL && 0 == strcmp(expected_b->content, msg->content)) {
			CU_ASSERT_EQUAL(expected_a, BUFFER_ERROR);
			CU_ASSERT_EQUAL(expected_c, BUFFER_ERROR);
		} else if (expected_c != NULL && 0 == strcmp(expected_c->content, msg->content)) {
			CU_ASSERT_EQUAL(expected_b, BUFFER_ERROR);
			CU_ASSERT_EQUAL(expected_a, BUFFER_ERROR);
		}

		CU_ASSERT(1 == bufferUnitary->freeSlots); //Il buffer è vuoto
	} 
}

// Test[10] | Consumazioni e produzioni concorrenti di molteplici messaggi in un buffer unitario
void test_multipleGetAndPutUnitaryBuffer(void) {
	/* 6 produttori */
	msg_t* msg_a = msg_init_string("EXPECTED_MESSAGE_A");
	msg_t* msg_b = msg_init_string("EXPECTED_MESSAGE_B");
	msg_t* msg_c = msg_init_string("EXPECTED_MESSAGE_C");
	msg_t* msg_d = msg_init_string("EXPECTED_MESSAGE_D");
	msg_t* msg_e = msg_init_string("EXPECTED_MESSAGE_E");
	msg_t* msg_f = msg_init_string("EXPECTED_MESSAGE_F");
	
	msg_t* expected_a_put;
	msg_t* expected_b_put;
	msg_t* expected_c_put;
	msg_t* expected_d_put;
	msg_t* expected_e_put;
	msg_t* expected_f_put;

	/* 5 consumatori */
	msg_t* expected_a_get;
	msg_t* expected_b_get;
	msg_t* expected_c_get;
	msg_t* expected_d_get;
	msg_t* expected_e_get;

	arg_t args_a;
	args_a.buffer = bufferUnitary;
	args_a.msg = msg_a;

	arg_t args_b;
	args_b.buffer = bufferUnitary;
	args_b.msg = msg_b;

	arg_t args_c;
	args_c.buffer = bufferUnitary;
	args_c.msg = msg_c;

	arg_t args_d;
	args_d.buffer = bufferUnitary;
	args_d.msg = msg_d;

	arg_t args_e;
	args_e.buffer = bufferUnitary;
	args_e.msg = msg_e;

	arg_t args_f;
	args_f.buffer = bufferUnitary;
	args_f.msg = msg_f;

	if(bufferUnitary != NULL) {
		/* dichiaro i thread dei produttori */
		pthread_t prod_a;
		pthread_t prod_b;
		pthread_t prod_c;
		pthread_t prod_d;
		pthread_t prod_e;
		pthread_t prod_f;

		/* dichiaro i thread dei consumatori */
		pthread_t cons_a;
		pthread_t cons_b;
		pthread_t cons_c;
		pthread_t cons_d;
		pthread_t cons_e;

		CU_ASSERT(1 == bufferUnitary->freeSlots); //Il buffer è vuoto

		/* I thread iniziano in maniera sparsa */
		pthread_create(&cons_a, NULL, args_get_bloccante, bufferUnitary);
		pthread_create(&cons_b, NULL, args_get_bloccante, bufferUnitary);

		pthread_create(&prod_a, NULL, args_put_bloccante, &args_a);
		pthread_create(&prod_b, NULL, args_put_bloccante, &args_b);
		pthread_create(&prod_c, NULL, args_put_bloccante, &args_c);

		pthread_create(&cons_c, NULL, args_get_bloccante, bufferUnitary);

		pthread_create(&prod_d, NULL, args_put_bloccante, &args_d);

		pthread_create(&cons_d, NULL, args_get_bloccante, bufferUnitary);

		pthread_create(&prod_e, NULL, args_put_bloccante, &args_e);
		pthread_create(&prod_f, NULL, args_put_bloccante, &args_f);

		pthread_create(&cons_e, NULL, args_get_bloccante, bufferUnitary);

		pthread_join(prod_a, (void*)&expected_a_put);
		pthread_join(prod_b, (void*)&expected_b_put);
		pthread_join(prod_c, (void*)&expected_c_put);
		pthread_join(prod_d, (void*)&expected_d_put);
		pthread_join(prod_e, (void*)&expected_e_put);
		pthread_join(prod_f, (void*)&expected_f_put);

		pthread_join(cons_a, (void*)&expected_a_get);
		pthread_join(cons_b, (void*)&expected_b_get);
		pthread_join(cons_c, (void*)&expected_c_get);
		pthread_join(cons_d, (void*)&expected_d_get);
		pthread_join(cons_e, (void*)&expected_e_get);

		/* Tutti i thread scrivono */
		CU_ASSERT_NOT_EQUAL(expected_a_put, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_b_put, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_c_put, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_d_put, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_e_put, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_f_put, BUFFER_ERROR);

		/* Tutti i thread leggono */
		CU_ASSERT_NOT_EQUAL(expected_a_get, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_b_get, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_c_get, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_d_get, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_e_get, BUFFER_ERROR);

		/*Il buffer è pieno */
		CU_ASSERT(0 == bufferUnitary->freeSlots);
	}
}

/*END UNITARY BUFFER (MULTI THREAD)*/

/* START BUFFER 4-SIZED TEST-CASES */

// Test[11] - (P>1; C=0; N>1) Produzione concorrente di molteplici messaggi in un buffer vuoto; il buffer si satura in corso
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
			CU_ASSERT(0 == contains(buffer, expected_4)); // 4 non è nel buffer
			CU_ASSERT(1 == contains(buffer, expected_0)); // è presente nel buffer
			CU_ASSERT(1 == contains(buffer, expected_1));
			CU_ASSERT(1 == contains(buffer, expected_2));
			CU_ASSERT(1 == contains(buffer, expected_3));

		} else if ( //thread_0 non inserisce
			expected_1 != NULL && 0 == strcmp(expected_1->content, msg_1->content) &&
			expected_2 != NULL && 0 == strcmp(expected_2->content, msg_2->content) &&
			expected_3 != NULL && 0 == strcmp(expected_3->content, msg_3->content) &&
			expected_4 != NULL && 0 == strcmp(expected_4->content, msg_4->content)
			) {
				CU_ASSERT_EQUAL(expected_0, BUFFER_ERROR);
				CU_ASSERT(0 == contains(buffer, expected_0)); // 0 non è nel buffer
				CU_ASSERT(1 == contains(buffer, expected_1)); // è presente nel buffer
				CU_ASSERT(1 == contains(buffer, expected_2));
				CU_ASSERT(1 == contains(buffer, expected_3));
				CU_ASSERT(1 == contains(buffer, expected_4));
				
		} else if ( //thread_1 non inserisce
			expected_2 != NULL && 0 == strcmp(expected_2->content, msg_2->content) &&
			expected_3 != NULL && 0 == strcmp(expected_3->content, msg_3->content) &&
			expected_4 != NULL && 0 == strcmp(expected_4->content, msg_4->content) &&
			expected_0 != NULL && 0 == strcmp(expected_0->content, msg_0->content)
			) {
				CU_ASSERT_EQUAL(expected_1, BUFFER_ERROR);
				CU_ASSERT(0 == contains(buffer, expected_1)); // 1 non è nel buffer
				CU_ASSERT(1 == contains(buffer, expected_0)); // è presente nel buffer
				CU_ASSERT(1 == contains(buffer, expected_2));
				CU_ASSERT(1 == contains(buffer, expected_3));
				CU_ASSERT(1 == contains(buffer, expected_4));
				
		} else if ( //thread_2 non inserisce
			expected_3 != NULL && 0 == strcmp(expected_3->content, msg_3->content) &&
			expected_4 != NULL && 0 == strcmp(expected_4->content, msg_4->content) &&
			expected_0 != NULL && 0 == strcmp(expected_0->content, msg_0->content) &&
			expected_1 != NULL && 0 == strcmp(expected_1->content, msg_1->content)
			) {
				CU_ASSERT_EQUAL(expected_2, BUFFER_ERROR);
				CU_ASSERT(0 == contains(buffer, expected_2)); // 2 non è nel buffer
				CU_ASSERT(1 == contains(buffer, expected_1)); // è presente nel buffer
				CU_ASSERT(1 == contains(buffer, expected_3));
				CU_ASSERT(1 == contains(buffer, expected_4));
				CU_ASSERT(1 == contains(buffer, expected_0));
				
		} else if ( //thread_3 non inserisce
			expected_4 != NULL && 0 == strcmp(expected_4->content, msg_4->content) &&
			expected_0 != NULL && 0 == strcmp(expected_0->content, msg_0->content) &&
			expected_1 != NULL && 0 == strcmp(expected_1->content, msg_1->content) &&
			expected_2 != NULL && 0 == strcmp(expected_2->content, msg_2->content)
			) {
				CU_ASSERT_EQUAL(expected_3, BUFFER_ERROR);
				CU_ASSERT(0 == contains(buffer, expected_3)); // 3 non è nel buffer
				CU_ASSERT(1 == contains(buffer, expected_1)); // è presente nel buffer
				CU_ASSERT(1 == contains(buffer, expected_2));
				CU_ASSERT(1 == contains(buffer, expected_4));
				CU_ASSERT(1 == contains(buffer, expected_0));
				
		} 
	}
}

// TEST[12] - (P>1; C=0; N>1) Produzione concorrente di molteplici messaggi in un buffer pieno; il buffer è già saturo
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

// Test[13] - (P=0; C>1; N>1) Consumazione concorrente di molteplici messaggi da un buffer pieno
void test_multipleGetFullBuffer(void) {

	msg_t* expected_0;
	msg_t* expected_1;
	msg_t* expected_2;
	msg_t* expected_3;

	if(buffer != NULL) {
		pthread_t cons_0;
		pthread_t cons_1;
		pthread_t cons_2;
		pthread_t cons_3;

		CU_ASSERT(0 == buffer->freeSlots); //Verifico che il buffer è pieno
		
		pthread_create(&cons_0, NULL, args_get_non_bloccante, buffer);
		pthread_create(&cons_1, NULL, args_get_non_bloccante, buffer);
		pthread_create(&cons_2, NULL, args_get_non_bloccante, buffer);
		pthread_create(&cons_3, NULL, args_get_non_bloccante, buffer);

		pthread_join(cons_0, (void*)&expected_0);
		pthread_join(cons_1, (void*)&expected_1);
		pthread_join(cons_2, (void*)&expected_2);
		pthread_join(cons_3, (void*)&expected_3);

		CU_ASSERT_NOT_EQUAL(expected_0, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_1, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_2, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_3, BUFFER_ERROR);
		CU_ASSERT(4 == buffer->freeSlots) //Verifico che ora il buffer è vuoto
	}
}

// TEST[14] - (P>1; C=0; N>1) Produzione concorrente di molteplici messaggi in un buffer vuoto; il buffer non si riempe
void test_multiplePutEmptyBuffer(void) {
	msg_t* msg_0 = msg_init_string("MESSAGE_0");
	msg_t* msg_1 = msg_init_string("MESSAGE_1");
	msg_t* msg_2 = msg_init_string("MESSAGE_2");

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

		CU_ASSERT(4 == buffer->freeSlots); //Verifico che il buffer è vuoto

		pthread_create(&prod_0, NULL, args_put_non_bloccante, &args_0);
		pthread_create(&prod_1, NULL, args_put_non_bloccante, &args_1);
		pthread_create(&prod_2, NULL, args_put_non_bloccante, &args_2);

		pthread_join(prod_0, (void*)&expected_0);
		pthread_join(prod_1, (void*)&expected_1);
		pthread_join(prod_2, (void*)&expected_2);

		CU_ASSERT_NOT_EQUAL(buffer->freeSlots, 0); //Il buffer non è pieno
		CU_ASSERT_NOT_EQUAL(buffer->freeSlots, 4); //il buffer non è vuoto
		CU_ASSERT(1 == buffer->freeSlots); // 3 thread creati in un buffer vuoto di dimensione 4. Rimane 1 solo slot libero.

	}
}

// TEST[15] - Consumazioni e produzioni concorrenti di molteplici messaggi in un buffer
void test_multipleGetAndPutBuffer(void) {

	/* Inizializzo i messaggi di 8 produttori */
	msg_t* msg_0 = msg_init_string("EXPECTED_MESSAGE_0");
	msg_t* msg_1 = msg_init_string("EXPECTED_MESSAGE_1");
	msg_t* msg_2 = msg_init_string("EXPECTED_MESSAGE_2");
	msg_t* msg_3 = msg_init_string("EXPECTED_MESSAGE_3");
	msg_t* msg_4 = msg_init_string("EXPECTED_MESSAGE_4");
	msg_t* msg_5 = msg_init_string("EXPECTED_MESSAGE_5");
	msg_t* msg_6 = msg_init_string("EXPECTED_MESSAGE_6");
	msg_t* msg_7 = msg_init_string("EXPECTED_MESSAGE_7");

	msg_t* expected_0_put;
	msg_t* expected_1_put;
	msg_t* expected_2_put;
	msg_t* expected_3_put;
	msg_t* expected_4_put;
	msg_t* expected_5_put;
	msg_t* expected_6_put;
	msg_t* expected_7_put;

	/* 9 messaggi consumati */
	msg_t* expected_0_get;
	msg_t* expected_1_get;
	msg_t* expected_2_get;
	msg_t* expected_3_get;
	msg_t* expected_4_get;
	msg_t* expected_5_get;
	msg_t* expected_6_get;
	msg_t* expected_7_get;
	msg_t* expected_8_get;

	/* Setup arguments*/
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

	arg_t args_5;
	args_5.buffer = buffer;
	args_5.msg = msg_5;

	arg_t args_6;
	args_6.buffer = buffer;
	args_6.msg = msg_6;

	arg_t args_7;
	args_7.buffer = buffer;
	args_7.msg = msg_7;

	if(buffer != NULL) {
		/* creating producer threads */
		pthread_t prod_0;
		pthread_t prod_1;
		pthread_t prod_2;
		pthread_t prod_3;
		pthread_t prod_4;
		pthread_t prod_5;
		pthread_t prod_6;
		pthread_t prod_7;

		/* creating consumer threads */
		pthread_t cons_0;
		pthread_t cons_1;
		pthread_t cons_2;
		pthread_t cons_3;
		pthread_t cons_4;
		pthread_t cons_5;
		pthread_t cons_6;
		pthread_t cons_7;
		pthread_t cons_8;

		CU_ASSERT_NOT_EQUAL(buffer->freeSlots, 0); //Il buffer non è pieno
		CU_ASSERT_NOT_EQUAL(buffer->freeSlots, 4); //il buffer non è vuoto

		/* Vengono creati i thread in ordine sparso */
		pthread_create(&prod_0, NULL, args_put_bloccante, &args_0);
		pthread_create(&prod_1, NULL, args_put_bloccante, &args_1);

		pthread_create(&cons_0, NULL, args_get_bloccante, buffer);

		pthread_create(&prod_2, NULL, args_put_bloccante, &args_2);

		pthread_create(&cons_1, NULL, args_get_bloccante, buffer);
		pthread_create(&cons_2, NULL, args_get_bloccante, buffer);

		pthread_create(&prod_3, NULL, args_put_bloccante, &args_3);

		pthread_create(&cons_3, NULL, args_get_bloccante, buffer);
		pthread_create(&cons_4, NULL, args_get_bloccante, buffer);

		pthread_create(&prod_4, NULL, args_put_bloccante, &args_4);

		pthread_create(&cons_5, NULL, args_get_bloccante, buffer);
		pthread_create(&cons_6, NULL, args_get_bloccante, buffer);
		pthread_create(&cons_7, NULL, args_get_bloccante, buffer);

		pthread_create(&prod_5, NULL, args_put_bloccante, &args_5);
		pthread_create(&prod_6, NULL, args_put_bloccante, &args_6);

		pthread_create(&cons_8, NULL, args_get_bloccante, buffer);
		pthread_create(&prod_7, NULL, args_put_bloccante, &args_7);

		pthread_join(prod_0, (void*)&expected_0_put);
		pthread_join(prod_1, (void*)&expected_1_put);
		pthread_join(prod_2, (void*)&expected_2_put);
		pthread_join(prod_3, (void*)&expected_3_put);
		pthread_join(prod_4, (void*)&expected_4_put);
		pthread_join(prod_5, (void*)&expected_5_put);
		pthread_join(prod_6, (void*)&expected_6_put);
		pthread_join(prod_7, (void*)&expected_7_put);

		pthread_join(cons_0, (void*)&expected_0_get);
		pthread_join(cons_1, (void*)&expected_1_get);
		pthread_join(cons_2, (void*)&expected_2_get);
		pthread_join(cons_3, (void*)&expected_3_get);
		pthread_join(cons_4, (void*)&expected_4_get);
		pthread_join(cons_5, (void*)&expected_5_get);
		pthread_join(cons_6, (void*)&expected_6_get);
		pthread_join(cons_7, (void*)&expected_7_get);
		pthread_join(cons_8, (void*)&expected_8_get);

		/* Tutti i thread scrivono */
		CU_ASSERT_NOT_EQUAL(expected_0_put, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_1_put, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_2_put, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_3_put, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_4_put, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_5_put, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_6_put, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_7_put, BUFFER_ERROR);

		/* Tutti i thread leggono */
		CU_ASSERT_NOT_EQUAL(expected_0_get, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_1_get, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_2_get, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_3_get, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_4_get, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_5_get, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_6_get, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_7_get, BUFFER_ERROR);
		CU_ASSERT_NOT_EQUAL(expected_8_get, BUFFER_ERROR);

		CU_ASSERT_NOT_EQUAL(buffer->freeSlots, 0); //Il buffer non è pieno
		CU_ASSERT_NOT_EQUAL(buffer->freeSlots, 4); //il buffer non è vuoto
		CU_ASSERT(2 == buffer->size - buffer->freeSlots); //Complessivamente mi rimangono due slot liberi
	}
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
    	CU_add_test(cUnitBufferUnitary_multi, "Test[9] | Consumazione concorrente di molteplici messaggi da un buffer unitario pieno", test_multipleGetUnitaryFullBuffer) == NULL ||
    	CU_add_test(cUnitBufferUnitary_multi, "Test[10] | Consumazioni e produzioni concorrenti di molteplici messaggi in un buffer unitario", test_multipleGetAndPutUnitaryBuffer) == NULL
    	) {
    		CU_cleanup_registry();
	        return CU_get_error();
    }

    /* ADDING 4-SIZED-MULTI-THREAD STACK TEST */
    if( CU_add_test(cUnitBuffer, "Test[11] | Produzione concorrente di molteplici messaggi in un buffer vuoto; il buffer si satura in corso", test_fullfillingEmptyBuffer) == NULL ||
    	CU_add_test(cUnitBuffer, "Test[12] | Produzione concorrente di molteplici messaggi in un buffer pieno; il buffer è già saturo", test_multiplePutFullBuffer) == NULL ||
    	CU_add_test(cUnitBuffer, "Test[13] | Consumazione concorrente di molteplici messaggi da un buffer pieno", test_multipleGetFullBuffer) == NULL ||
    	CU_add_test(cUnitBuffer, "Test[14] | Produzione concorrente di molteplici messaggi in un buffer vuoto; il buffer non si riempe", test_multiplePutEmptyBuffer) == NULL ||
    	CU_add_test(cUnitBuffer, "Test[15] | Consumazioni e produzioni concorrenti di molteplici messaggi in un buffer", test_multipleGetAndPutBuffer) == NULL
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
