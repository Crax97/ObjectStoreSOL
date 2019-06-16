#ifndef OBJSTORE_H
#define OBJSTORE_H

#include <stdlib.h>

/*
 * Inizia la connessione all'object store, registrando il client con il <name> dato.
 * Restituisce:
 * 	OS_OK se la connessione è avvenuta correttamente;
 * 	OS_ERR in caso di errore.
 * La connessione all'object store è globale per il client.*/
int os_connect(char* name);

/*
 * Richiede all'object store di memorizzare l'oggetto puntato da <block> di dimensione <len> 
 * con il nome <name>.
 * Restituisce: 
 * 	OS_OK se la memorizzazione è avvenuta;
 * 	OS_NOCONN se non è stata chiamato os_connect(...) in precedenza;
 * 	OS_ERR in caso di errore generico.
 * */
int os_store(char* name, void* block, size_t len);

/*
 * Recupera dall'object store l'oggetto di nome <name>.
 * Restituisce:
 * 	Un puntatore all'oggetto recuperato, se il recupero è andato a buon fine;
 * 	NULL altrimenti.*/
void* os_retrieve(char* name);

/* Cancella l'oggetto di nome <name> dall'object store.
 * Restituisce:
 * 	OS_OK se la cancellazione è avvenuta;
 * 	OS_NOCONN se non è stato chiamato os_connect(...) in precedenza;
 * 	OS_ERR in caso di errore generico.
 */
int os_delete(char* name);

/* Chiude la connessione all'object store.
 * Restituisce:
 * 	OS_OK se la disconnessione è avvenuta;
 * 	OS_NOCONN se non è stato chiamato os_connect(...) in precedenza;
 */
int os_disconnect();

#endif
