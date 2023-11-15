# progetto labII BIGFARM
progetto valido per esame di Laboratorio II - UniPisa (informatica)

# farm
Nel farm.c inizio con l'esaminare gli argomenti passati da linea di comando, inizialmente controllando le flags con uno switch: con la funzione strtol controllo che i parametri passati alle flags siano corretti e inizializzo numero thread, dimensione del buffer e delay dell'usleep. Nel caso di default utilizzo le variabili già inizializzate in precedenza. Creo due semafori per la gestione del buffer utili a inserire gli elementi all'interno, preparo l'array di struct il quale contiene una struct per ogni thread che partirà processando il corpo del tbody. Sempre nel main, faccio partire un for che, ignorando le flags con relativi parametri grazie a optind, controlla i nomi dei file passati su linea di comando, inserendo nel buffer solo quelli esistenti. Inoltre controllo che la variabile volatile condsignal che gestisce il segnale SIGINT sia stata modificata dalla funzione handler. Se no, continuo a inserire i nomi all'interno del file grazie ai due semafori. Tra un inserimento e l'altro faccio una usleep che dipende dal delay passato. Per far terminare i thread inserisco nel buffer dopo l'ultimo elemento inserito la stringa "-1", utile al tbody per terminare il while e fare la pthread_exit. Nel while prendo in mutua esclusione il nome del file, lo apro e mediante fseek, ftell e rewind riesco a calcolare il numero di long all'interno, i quali vengono inseriti nell'array numerifile per poi essere sommati all'interno della variabile somma presente nella struct del thread. Per far comunicare il farm con il server collector.py apro il socket, invio il primo byte che identifica il tipo di richiesta che voglio venga svolta, subito dopo invio lunghezza del nome, il nome del file e la somma: quest'ultima essendo un long viene passata come due interi da 4 byte e ricevuta dal server come un unico intero a 8 byte. 

# collector
Il collector.py, quindi, riceve il primo byte per stabilire quale richiesta soddisfare mediante una lettera. Nel caso del farm, viene inviata la "c", la quale fa partire la funzione gestisci_worker. Riceve 12 byte, i primi 4 per la lunghezza del nome del file, gli altri 8 per ricomporre la somma, scomposta in due interi dal farm. Ricevo lenfilename byte per il nome del file, e inserisco la coppia somma nomefile nel dizionario. Le altre due richieste sono dedicate al client. La funzione allcouple gestisce la chiamata del client senza argomenti, inviando: il numero di coppie all'interno del dizionario, e per ogni chiave la lunghezza sia della stringa che identifica la somma, la somma stessa, la lunghezza del nome del file e il nome del file (valore del dizionario corrispondente alla somma). La funzione couplecheck invece viene processata se viene chiamato il client passando come argomenti dei long. Inizializzo un array dove andrò a inserire per ogni long passato, i nomi dei file che sono legati a quella somma all'interno del dizionario. Ricevo dal client: lunghezza della stringa corrispondente alla somma e la somma stessa. Scorro il dizionario, quando trovo la somma passata, salvo il nome del file associato. Faccio una sorted sull'array e invio al client i nomi per ogni somma.

# client
Il client.c stabilisce la connessione col server, e a seconda del numero di argomenti passati invia un tipo diverso di richiesta. Se argc = 1, allora invio la lettera che identifica la richiesta, leggo il numero di chiavi presenti all'interno del dizionario, e per ognuna leggo la coppia somma nomefile e li stampo su stdout. Se vengono passati argomenti, invio sempre il byte per identificare la richiesta, invio il numero di argomenti passati al client, e per ognuno invio al server come stringa i long passati. Leggo il numero di elementi presenti nell'array contenente i nomi dei file associati a quel long passato e ne stampo il contenuto collegandoli alla somma. 
