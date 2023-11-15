#include "xerrori.h"

#define QUI __LINE__,__FILE__
#define HOST "127.0.0.1"
#define PORT 57765

extern char *optarg;
extern int optind;
int lunbuff = 8;
volatile bool condsignal = false;

typedef struct {
	long somma;
  int *cindex;  // indice nel buffer
  char **buffer; 
  pthread_mutex_t *cmutex;
  sem_t *sem_free_slots;
  sem_t *sem_data_items;  
} dati;


void *tbody(void *arg)
{  
	dati *a = (dati *)arg; 
	long *numerifile; // array long elementi file
	char *nomefile; // nome del file su cui lavora thread 
	do {
  
	xsem_wait(a->sem_data_items,QUI);
	xpthread_mutex_lock(a->cmutex,QUI);
	
	nomefile = a->buffer[*(a->cindex) % lunbuff]; // inserisco file nel buffer
	*(a->cindex) += 1;

	xpthread_mutex_unlock(a->cmutex,QUI);
  xsem_post(a->sem_free_slots,QUI);
		
	FILE *f = fopen(nomefile, "rb");
	if (f == NULL) continue;
   
  fseek(f,0,SEEK_END);
	int elementi = ftell(f)/sizeof(long);
  if (elementi == -1) termina("errore ftell");
	numerifile = malloc(elementi*sizeof(long));
	if(numerifile == NULL) termina("Errore malloc");
	rewind(f);
	if(fread(numerifile, sizeof(*numerifile), elementi, f) != elementi) termina("Errore fread");
	fclose(f);

  a->somma = 0;
	for(int i = 0; i < elementi; i++){
		a->somma += i * numerifile[i];
	}
  
  int fd_skt = 0;      // file descriptor associato al socket
  struct sockaddr_in serv_addr;
  int sommaprimi4, sommaultimi4;
  int tmplungnomefile, tmpprimi4, tmpultimi4;
  char *requestworker;
  size_t e;

  
  // crea socket
  if ((fd_skt = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    termina("Errore creazione socket");
  // assegna indirizzo
  serv_addr.sin_family = AF_INET;
  // il numero della porta deve essere convertito 
  // in network order 
  serv_addr.sin_port = htons(PORT);
  serv_addr.sin_addr.s_addr = inet_addr(HOST);
  // apre connessione
  if (connect(fd_skt, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    termina("Errore apertura connessione");
    
  requestworker = "c";  // per gestire richiesta nel collector
  e = writen(fd_skt,requestworker,sizeof(char));
  if(e!=sizeof(char)) termina("Errore write richiesta c");

  // invio lunghezza stringa per allocare spazio nel collector
  tmplungnomefile = htonl(strlen(nomefile));
  e = writen(fd_skt,&tmplungnomefile,sizeof(tmplungnomefile));
  if(e!=sizeof(int)) termina("Errore write");

  
  sommaultimi4 = a->somma;
  sommaprimi4 = a->somma >> 32;  // separo il long in due int e li passo al collector
  tmpprimi4 = htonl(sommaprimi4);
  e = writen(fd_skt,&tmpprimi4,sizeof(tmpprimi4));
  if(e!=sizeof(int)) termina("Errore write byte piu significativi");
  tmpultimi4 = htonl(sommaultimi4);
  e = writen(fd_skt,&tmpultimi4,sizeof(tmpultimi4));
  if(e!=sizeof(int)) termina("Errore write");

  //invio nomefile
  e = writen(fd_skt,nomefile,ntohl(tmplungnomefile));
  if(e!=ntohl(tmplungnomefile)) termina("Errore write byte meno significativi");
	
	close(fd_skt);
	free(numerifile);
		
	} while(strcmp(nomefile, "-1")!=0);

  pthread_exit(NULL); 
}  

void handler(int s)  // per gestire il SIGINT
{
  if(s==SIGINT) {
    condsignal = true;
  }
}

int main(int argc, char *argv[])
{
  // controlla numero argomenti
  if(argc<2) {
      printf("Uso: %s file [file ...] \n",argv[0]);
      return 1;
  }
	int opt, delay = 0, nthread = 4;
  char *test;

	while ((opt = getopt(argc, argv, "n:q:t:")) != -1) 
	{
   switch (opt) 
   {
    case 'n':
      if(strtol(optarg, &test, 10) > 0) nthread = atoi(optarg);
      else termina("erroreflag numero thread");
      break;
    case 'q':
      if(strtol(optarg, &test, 10) > 0) lunbuff = atoi(optarg);
      else termina("errore flag dimensione buffer");
      break;
    case 't':
      if(strtol(optarg, &test, 10) >= 0 && strcmp(test, "") == 0) delay = atoi(optarg);
      else termina("errore flag delay");
      break; 
    default:
      termina("errore passaggio flags");
   }
	}

  struct sigaction sa;
  sa.sa_handler = &handler;
	// setta a "insieme vuoto" sa.sa_mask che è la
	// maschera di segnali da bloccare: non una buona idea 
	// quando si usa lo stesso handler per più funzioni
  sigemptyset(&sa.sa_mask);     
  sa.sa_flags = SA_RESTART;     // restart system calls 
  sigaction(SIGINT,&sa,NULL);   // stesso handler per Control-C


  // threads related
  char *buffer[lunbuff];
  int pindex=0,cindex=0;
	pthread_mutex_t cmutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_t t[nthread];
  dati a[nthread];
  sem_t sem_free_slots, sem_data_items;
  
  xsem_init(&sem_free_slots,0,lunbuff,QUI);
  xsem_init(&sem_data_items,0,0,QUI);
  for(int i=0;i<nthread;i++) {
    // faccio partire il thread i
		a[i].somma = 0;
    a[i].buffer = buffer;
		a[i].cindex = &cindex;
		a[i].cmutex = &cmutex;
    a[i].sem_data_items = &sem_data_items;
    a[i].sem_free_slots = &sem_free_slots;
    
    xpthread_create(&t[i],NULL,tbody,a+i,QUI);
  }


	for(int i = optind; i < argc; i++){ 
    if(condsignal) break;  // quando arriva il segnale SIGINT finisce di processare gli elementi che ci sono nel buffer e non scrive piu niente
    FILE *f = fopen(argv[i], "rb");
    if (f == NULL) 
    {
      fprintf(stderr, "errore passaggio file: %s non valido\n", argv[i]);  //metto solo file esistenti nel buffer
      continue;
    }  
    fclose(f);
		xsem_wait(&sem_free_slots,QUI);
    buffer[pindex++ % lunbuff] = argv[i];
    xsem_post(&sem_data_items,QUI);
    usleep(delay*1000);
	}

  // terminazione threads
 	for(int i=0;i<nthread;i++) {
    xsem_wait(&sem_free_slots,QUI);
    buffer[pindex++ % lunbuff] = "-1";
    xsem_post(&sem_data_items,QUI);
  }

	
  // join dei thread e calcolo risulato
	for(int i=0;i<nthread;i++){
    xpthread_join(t[i],NULL,QUI);
  }

  
  sem_destroy(&sem_data_items);
  sem_destroy(&sem_free_slots);
  pthread_mutex_destroy(&cmutex);
	return 0;
}