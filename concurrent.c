#include "concurrent.h"

/* Allocazione / deallocazione / copia messaggio */
//Creazione di un messaggio
msg_t * msg_init_string(void* content) {

    msg_t * msg = (msg_t*)malloc(sizeof(msg_t));
    char* string = (char*)content;
    char* new_content = (char*)malloc(strlen(string)+1); //+1 per lo /0 finale
    strcpy(new_content, string);

    msg->content = new_content;
    msg->msg_init = msg_init_string;
    msg->msg_copy = msg_copy_string;
    msg->msg_destroy = msg_destroy_string;
    return msg;
}

//Deallocazione di un messaggio
void msg_destroy_string(msg_t* msg) {
    free(msg->content);
    free(msg);
}

//Copia di un messaggio
msg_t * msg_copy_string(msg_t* msg) {
    return msg->msg_init(msg->content);
}


/* allocazione / deallocazione buffer */
// creazione di un buffer vuoto di dim. max nota
buffer_t* buffer_init(unsigned int maxsize){

	buffer_t* buffer = (buffer_t*)malloc(sizeof(buffer_t));
	buffer->message = (msg_t*)calloc(maxsize, sizeof(msg_t));

	buffer->produce = 0;
	buffer->consume = 0;
	buffer->size = maxsize;
    buffer->freeSlots = maxsize;

	buffer->buffer_init = buffer_init;
	buffer->buffer_destroy = buffer_destroy;

	pthread_mutex_init(&(buffer->mutexProd), NULL);
	pthread_mutex_init(&(buffer->mutexCons), NULL);
    pthread_mutex_init(&(buffer->mutexIndex), NULL);
	pthread_cond_init(&(buffer->notFull), NULL);
	pthread_cond_init(&(buffer->notEmpty), NULL);

	return buffer;
}

// deallocazione di un buffer
void buffer_destroy(buffer_t * buffer){

	pthread_mutex_destroy(&(buffer->mutexProd));
	pthread_mutex_destroy(&(buffer->mutexCons));
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

    if(msg == NULL)
        return BUFFER_ERROR;

    struct timespec timeToWait;
    struct timeval now;

    gettimeofday(&now,NULL);
    timeToWait.tv_sec = now.tv_sec+5;
    timeToWait.tv_nsec = (now.tv_usec+1000UL*1000)*1000UL;
    
    pthread_mutex_lock(&buffer->mutexProd);
    while(buffer->freeSlots == 0) {
            pthread_cond_timedwait(&buffer->notFull, &buffer->mutexProd, &timeToWait);
    }
    /* Operazioni sugli indici */
    pthread_mutex_lock(&buffer->mutexIndex);
    int i = buffer->produce;
    buffer->message[i] = *msg;
    buffer->produce = (i+1) % buffer->size;
    buffer->freeSlots = buffer->freeSlots-1;
    /* Invio la signal e sblocco i semafori */
    pthread_mutex_unlock(&buffer->mutexIndex);
    pthread_cond_signal(&buffer->notEmpty);
    pthread_mutex_unlock(&buffer->mutexProd);
    return msg;
}


void* args_put_bloccante(void* arguments) {
    arg_t *args = arguments;
    msg_t* msg = put_bloccante(args->buffer, args->msg);
    pthread_exit(msg);
}

// inserimento non bloccante: restituisce BUFFER_ERROR se pieno,
// altrimenti effettua l’inserimento e restituisce il messaggio
// inserito; N.B.: msg!=null
msg_t* put_non_bloccante(buffer_t* buffer, msg_t* msg){
    pthread_mutex_lock(&(buffer->mutexProd));
    if(msg != NULL) {
        if(buffer->freeSlots > 0) {

            /* Operazioni sugli indici */
            pthread_mutex_lock(&(buffer->mutexIndex));
            int i = buffer->produce;
            buffer->message[i] = *msg;
            buffer->produce = (i+1) % buffer->size;
            buffer->freeSlots = buffer->freeSlots-1;

            pthread_cond_signal(&(buffer->notEmpty));
            pthread_mutex_unlock(&buffer->mutexIndex);
            pthread_mutex_unlock(&(buffer->mutexProd));
            return msg;
        }
    }
    pthread_mutex_unlock(&(buffer->mutexProd));
    return BUFFER_ERROR;
}

void* args_put_non_bloccante(void* arguments) {
    arg_t *args = arguments;
    msg_t* msg = put_non_bloccante(args->buffer, args->msg);
    pthread_exit(msg);
}


// estrazione bloccante: sospende se vuoto, quindi
// restituisce il valore estratto non appena disponibile
msg_t* get_bloccante(buffer_t* buffer) {

    struct timespec timeToWait;
    struct timeval now;

    gettimeofday(&now,NULL);
    timeToWait.tv_sec = now.tv_sec+5;
    timeToWait.tv_nsec = (now.tv_usec+1000UL*1000)*1000UL;

    pthread_mutex_lock(&(buffer->mutexCons));
    while(buffer->size - buffer->freeSlots == 0) {
            pthread_cond_timedwait(&buffer->notEmpty, &buffer->mutexCons, &timeToWait);
    }
    /* Operazioni sugli indici */
    pthread_mutex_lock(&buffer->mutexIndex);
    int i = buffer->consume;
    msg_t* msg = msg_init_string(buffer->message[i].content); //Copio il contenuto del messaggio estratto
    buffer->message[i].msg_destroy;
    buffer->consume = (i+1) % buffer->size;
    buffer->freeSlots = buffer->freeSlots + 1;
    pthread_mutex_unlock(&buffer->mutexIndex);
    pthread_cond_signal(&buffer->notFull);
    pthread_mutex_unlock(&(buffer->mutexCons));
    return msg;

}

void* args_get_bloccante(void* buffer){
    msg_t* msg = get_bloccante((buffer_t*) buffer);
    pthread_exit(msg);
}

// estrazione non bloccante: restituisce BUFFER_ERROR se vuoto
// ed il valore estratto in caso contrario
msg_t* get_non_bloccante(buffer_t* buffer) {

    pthread_mutex_lock(&(buffer->mutexCons));

    if(buffer->size - buffer->freeSlots == 0) { //se il buffer è vuoto
        pthread_mutex_unlock(&(buffer->mutexCons));
        return BUFFER_ERROR;
    }

    /* Operazioni sugli indici */
    pthread_mutex_lock(&buffer->mutexIndex);
    int i = buffer->consume;
    msg_t* msg = msg_init_string(buffer->message[i].content); //Copio il contenuto del messaggio estratto
    buffer->message[i].msg_destroy; //Distruggo il messaggio nel buffer
    buffer->consume = (i+1) % buffer->size;
    buffer->freeSlots = buffer->freeSlots + 1;

    pthread_cond_signal(&(buffer->notFull));
    pthread_mutex_unlock(&buffer->mutexIndex);
    pthread_mutex_unlock(&(buffer->mutexCons));
    return msg;
}

void* args_get_non_bloccante(void* buffer){
    
    msg_t* msg = get_non_bloccante((buffer_t*) buffer);
    
    pthread_exit(msg);
}

// Verifico che il buffer contenga una data stringa, ritorna 1 se riesce
int contains(buffer_t* buffer, msg_t* string) {
    int i;
    if(buffer != NULL && string != NULL) {
        for(i = 0; i < buffer->size; i++) {
            if(strcmp(buffer->message[i].content, string->content) == 0)
                return 1;
        }
    }
    return 0;
}