#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <concurrent.h>
#include <CUnit/CUnit.h>

#define BUFFER_ERROR (msg_t *) NULL


/* allocazione / deallocazione buffer */
// creazione di un buffer vuoto di dim. max nota
buffer_t* buffer_init(unsigned int maxsize){

	buffer_t* buffer = (buffer_t*)malloc(sizeof(buffer_t));
	buffer->message = (msg_t*)calloc(maxsize, sizeof(msg_t));

	buffer->produce = 0;
	buffer->consume = 0;
	buffer->size = maxsize;

	buffer->buffer_init = buffer_init;
	buffer->buffer_destroy = buffer_destroy;

	pthread_mutex_init(&(buffer->mutex), NULL);
	pthread_cond_init(&(buffer->notFull), NULL);
	pthread_cond_init(&(buffer->notEmpty), NULL);

	return buffer;
}

// deallocazione di un buffer
void buffer_destroy(buffer_t* buffer){

	pthread_mutex_destroy(&(buffer->mutex));
	pthread_cond_destroy(&(buffer->notFull));
	pthread_cond_destroy(&(buffer->notEmpty));
	int i = 0;

	while(i < buffer->size) {
		buffer->message[i].msg_destroy;
		i++;
	}

	free(buffer->message);
	free(buffer);
}

/* operazioni sul buffer */
// inserimento bloccante: sospende se pieno, quindi
// effettua l’inserimento non appena si libera dello spazio
// restituisce il messaggio inserito; N.B.: msg!=null
msg_t* put_bloccante(buffer_t* buffer, msg_t* msg){

}

// inserimento non bloccante: restituisce BUFFER_ERROR se pieno,
// altrimenti effettua l’inserimento e restituisce il messaggio
// inserito; N.B.: msg!=null
msg_t* put_non_bloccante(buffer_t* buffer, msg_t* msg){

}

// estrazione bloccante: sospende se vuoto, quindi
// restituisce il valore estratto non appena disponibile
msg_t* get_bloccante(buffer_t* buffer) {

}

// estrazione non bloccante: restituisce BUFFER_ERROR se vuoto
// ed il valore estratto in caso contrario
msg_t* get_non_bloccante(buffer_t* buffer) {

}

int main() {
    buffer_t* buffer = buffer_init(8);
    printf("%d\n", &buffer);
    printf("%d\n", &buffer->message);
    buffer_destroy(&buffer);
    return 0;
}
