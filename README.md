### Progetto di Sistemi Operativi e Laboratorio dell'anno 2018/2019

## Note:
Realizzato da [Giovanni Solimeno](https://www.facebook.com/CraxGf).

Per testare tutto scompattare il file in una cartella ed eseguire ```make```, lanciare il server ed eseguire ```make test```

Il progetto, revisionato dal prof. Prencipe, è stato criticato in due modi:
1) Le read dell'header vengono effettuate byte per byte, quando **il modo corretto sarebbe di leggere l'header a chunk, fino a quando non si incontra un newline in uno dei chunk**;
2) Il flag ```server_info_s::server_running``` dovrebbe essere un ```atomic_t```, dato che è una variabile condivisa da più thread (in realtà, come fatto notare dal prof, problemi non ce ne sono, in quanto c`è un unico scrittore e più lettori il flag).

## L'object store
L'object store **è un eseguibile** il cui scopo è quello di **ricevere dai client delle richieste di memorizzare, recuperare, cancellare blocchi di dati dotati di nome**, detti "*oggetti*". L'object store
- **gestisce uno spazio di memorizzazione separato per ogni cliente**
- i **nomi degli oggetti** sono **garantiti essere univoci all'interno dello spazio di memorizzazione di un cliente**
- i **nomi di clienti** sono **garantiti essere tutti distinti**

Tutti i nomi rispettano il formato dei nomi di file POSIX.

L'object store è un server che attende il collegamento di un client su un socket (locale) di nome noto `objstore.sock`. Per collegarsi all'object store, un client invia al server il un messaggio di **registrazione** nel formato indicato sotto; in risposta, l'object store crea un thread destinato a servire le richieste di quel particolare cliente. Il thread servente termina quando il client invia un esplicito comando di **deregistrazione** oppure se si verifica un errore nella comunicazione. Le **altre richieste** che possono essere inviate riguardano lo *store* di un blocco di dati, il *retrieve* di un blocco di dati e il *delete* di un blocco di dati.

Internamente, l'object store **memorizza gli oggetti** che gli vengono affidati (e altri eventuali dati che si rendessero necessari) **nel file system**, all'interno di **file che hanno per nome il nome dell'oggetto**. Questi file sono poi **contenuti in directory che hanno per nome il nome del cliente** a cui l'oggetto appartiene. **Tutte le directory dei clienti sono contenute in una directory `data`** all'interno della working directory del server dell'object store.

Il server **quando riceve un segnale termina** il prima possibile **lasciando l'object store in uno stato consistente**, cioè non vengono mai lasciati nel file system oggetti parziali. Quando riceve il **segnale SIGUSR1, vengono stampate sullo standard output alcune informazioni di stato** del server; tra queste, almeno le seguenti: numero di client connessi, numero di oggetti nello store, size totale dello store.
