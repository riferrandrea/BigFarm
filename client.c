#include "xerrori.h"

#define QUI __LINE__,__FILE__
#define HOST "127.0.0.1"
#define PORT 57765


// questa versione non permette di scegliere 
// host e port dalla linea di comando
// per cambiarli è necessario ricompilare
int main(int argc, char const* argv[])
{
   
  int fd_skt = 0;     // file descriptor associato al socket
  struct sockaddr_in serv_addr;
  size_t e;
  char *clientnoarg; // richiesta client senza argomenti
  char *clientwarg; // richesta client con sequenza di long
  int nkeys, dimstringa; 
  int argomenti = htonl(argc);
  
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
    
  if (argc == 1){

    clientnoarg = "a"; // invio un byte per dire al collector di eseguire la richiesta ./client senza argomenti
    e = writen(fd_skt,clientnoarg,sizeof(char)); 
    if(e!=sizeof(char)) termina("Errore write richiesta a");

    e = readn(fd_skt,&nkeys,sizeof(nkeys)); // ricevo il numero di elementi all'interno del dizionario di coppie
    if(e!=sizeof(int)) termina("Errore read numero chiavi");

    for(int i=0;i<ntohl(nkeys);i++) { // ricevo le coppie dal collector

    
    e = readn(fd_skt,&dimstringa,sizeof(int));  // ricevo la dimensione della stringa corrispondente alla somma dei long (chiave dizionario)
    if(e!=sizeof(int)) termina("Errore read dimsommacoll");

    char *sommacoll = malloc((dimstringa)+1);
    e = readn(fd_skt,sommacoll,ntohl(dimstringa));  // ricevo la somma (chiave dizionario)
    if(e!=ntohl(dimstringa)) termina("Errore read sommacoll");
    sommacoll[dimstringa] = '\0';
    
    
    e = readn(fd_skt,&dimstringa,sizeof(int)); // ricevo il numero di lettere del nome del file
    if(e!=sizeof(int)) termina("Errore read dimnomefile");


    char *nomefilecoll = malloc(dimstringa+1);
    e = readn(fd_skt,nomefilecoll,ntohl(dimstringa)); // ricevo il nome del file 
    if(e!=ntohl(dimstringa)) termina("Errore read nomefilecoll");
    nomefilecoll[dimstringa] = '\0';
    
    fprintf(stdout, "%s %s\n", sommacoll, nomefilecoll);
    free(sommacoll);
    free(nomefilecoll);
    }

  } else {

    clientwarg = "b"; // richiesta del client con ./client args 
    e = writen(fd_skt,clientwarg,sizeof(char)); 
    if(e!=sizeof(char)) termina("Errore write richiesta b");

    e = writen(fd_skt,&argomenti,sizeof(int)); // passo il nunmero di argomenti
    if(e!=sizeof(int)) termina("Errore write argc");

    for(int i = 1; i < argc; i++){
      
      char *sommacheck = malloc(strlen(argv[i]));
      strcpy(sommacheck, argv[i]);

      dimstringa = htonl(strlen(argv[i]));
      e = writen(fd_skt, &dimstringa, sizeof(int));
      if(e!=sizeof(int)) termina("Errore write dim sommacheck");

      
      e = writen(fd_skt, sommacheck, strlen(argv[i])); // passo argomenti dati su linea di comando (somme)
      if(e!=strlen(argv[i])) termina("Errore write sommacheck");  


      e = readn(fd_skt,&nkeys,sizeof(nkeys));  // leggo il numero di chiavi corrispondenti alla somma passata
      if(ntohl(nkeys) == 0) fprintf(stdout, "Nessun file\n");

      for(int j=0;j<ntohl(nkeys);j++) {

      e = readn(fd_skt,&dimstringa,sizeof(int)); // ricevo il numero di lettere del nome del file
      if(e!=sizeof(int)) termina("Errore read dimnomefile");

      char *nomefilecoll = malloc(dimstringa+1);
      e = readn(fd_skt,nomefilecoll,ntohl(dimstringa)); // ricevo il nome del file 
      if(e!=ntohl(dimstringa)) termina("Errore read nomefilecoll");
      nomefilecoll[dimstringa] = '\0';
    
      fprintf(stdout, "%s %s\n", sommacheck, nomefilecoll);
      free(nomefilecoll);
      }     
      free(sommacheck);     
    }
  
  }
  if(close(fd_skt)<0)
    perror("Errore chiusura socket");
}