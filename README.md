# concorrente
#HWC
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
