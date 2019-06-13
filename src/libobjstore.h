#ifndef OBJSTORE_H
#define OBJSTORE_H

#include <stdlib.h>

#define OS_OK 0
#define OS_ERR 1
#define OS_NOCONN 2
#define OS_EXISTS 3
#define OS_NEXISTS 4

#define REGISTER_STR "REGISTER %s\n"
#define STORE_STR "STORE %s %lu\n"
#define OK_STR "OK \n"
#define RETRIEVE_STR "RETRIEVE %s\n"
#define DATA_STR "DATA %lu \n%s"
#define DELETE_STR "DELETE %s \n"
#define LEAVE_STR "LEAVE \n"
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
 * 	OS_EXISTS se esiste un oggetto di nome <name> nell'object store del client;
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
 * 	OS_NEXISTS se l'oggetto non esiste per il client;
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
