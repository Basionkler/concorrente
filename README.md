# concorrente
#HWC

V 1.6
- Improvements to test-concurrency

V 1.5
- Major bug fix for args_put_bloccante
- Major bug fix for args_put_non_bloccante
- Major bug fix for args_get_bloccante
- Major bug fix for args_get_non_bloccante
- Test-Case improvements
- Starting Multi-Thread Suite test-cases

V 1.0
- Terminati tutti i metodi richiesti sugli inserimenti e sulle estrazioni
- test.c inizializzato
- Inseriti i Suite su buffer unitario e k-sized
- Inserita la prima batteria di test su buffer unitario

V 0.6
- Implementate Get bloccante e non bloccante
- Riorganizzati gli import
- Implementate le primitive msg_init/copy/destroy

V 0.2
- Modificata la struttura del buffer, aggiunto un secondo mutex. Ora si usa un mutex per i produttori, uno per i consumatori
- Implementate le funzioni put bloccante e non bloccante. Da applicare migliorie
- Implementata funzione di supporto "slotLiberi(buffer_t* buffer)"

V 0.1
- struttura del buffer completata
- Il buffer viene allocato e deallocato con apparente successo!
