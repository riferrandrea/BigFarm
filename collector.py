#! /usr/bin/env python3
# server che fornisce l'elenco dei primi in un dato intervallo 
# gestisce più clienti contemporaneamente usando i thread
# Parte premendo il tasto Run 
# può essere usato con il client pclient.py 
import sys, struct, socket, threading


# host e porta di default
HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 57765  # Port to listen on (non-privileged ports are > 1023)
dizwork = {}




def main(host=HOST,port=PORT):
  # creiamo il server socket
  with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
      s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
      s.bind((host, port))
      s.listen()
      while True:
        # mi metto in attesa di una connessione
        conn, addr = s.accept()
        
        
        data = recv_all(conn, 1)
        if (data.decode() == "a"):
              t = threading.Thread(target=allcouple, args=(conn,addr)) 
              t.start()
        elif (data.decode() == "b"):
               t = threading.Thread(target=couplecheck, args=(conn,addr))
               t.start()
        elif (data.decode() == "c"):
              t = threading.Thread(target=gestisci_worker, args=(conn,addr))
              t.start()   

  s.shutdown(socket.SHUT_RDWR)
  s.close()
    
    




def gestisci_worker(conn,addr): 
  #gestisce somma nomefile con worker
  with conn:  
    # prendo lunghezza nomefile e la somma
    data = recv_all(conn,12)
    assert len(data)==12
    lenfilename = struct.unpack("!i",data[:4])[0]
    somma = struct.unpack("!q",data[4:])[0]
    # prendo nome file
    data = recv_all(conn,lenfilename)
    assert len(data)==lenfilename
    nomefile = data.decode()

    dizwork.update({somma: nomefile}) # inserisco nel dizionario la coppia


    
def allcouple(conn,addr): 
  # gestisce ./client senza argomenti
  with conn:  
    conn.sendall(struct.pack("!i",len(dizwork))) #numero di elementi all'interno del dizionario
    sorted(dizwork.items())
    for key in dizwork:
      conn.sendall(struct.pack("!i",len(str(key)))) #lunghezza stringa che rappresenta la somma    
      conn.sendall(str(key).encode()) # somma
      conn.sendall(struct.pack("!i",len(dizwork.get(key)))) #lunghezza nomefile 
      conn.sendall(dizwork.get(key).encode()) # nome del file

    
def couplecheck(conn,addr): 
  # gestisce ./client args
  with conn:  
    dizworkwsum = []
    data = recv_all(conn,4)
    assert len(data)==4
    argc = struct.unpack("!i",data)[0]
    for i in range(argc-1): 
          data = recv_all(conn,4)
          assert len(data)==4
          luntestsomma = struct.unpack("!i", data)[0]   
          data = recv_all(conn,luntestsomma)
          assert len(data)==luntestsomma
          testsomma = data.decode()       
          for key in dizwork:   #inserisco in dizworkwsum tutti i file con testsomma
                if testsomma == str(key):
                  dizworkwsum.append(dizwork.get(key))
          conn.sendall(struct.pack("!i",len(dizworkwsum)))
          sorted(dizworkwsum)
          for e in dizworkwsum:
            conn.sendall(struct.pack("!i", len(e))) #invio lunghezza del nome del file
            conn.sendall(e.encode()) # invio elementi dell' array di nomifile con testsomma 
          dizworkwsum = []
 
    



# riceve esattamente n byte e li restituisce in un array di byte
# il tipo restituto è "bytes": una sequenza immutabile di valori 0-255
# analoga alla readn che abbiamo visto nel C
def recv_all(conn,n):
  chunks = b''
  bytes_recd = 0
  while bytes_recd < n:
    chunk = conn.recv(min(n - bytes_recd, 1024))
    if len(chunk) == 0:
      raise RuntimeError("socket connection broken")
    chunks += chunk
    bytes_recd = bytes_recd + len(chunk)
  return chunks
 
 

if len(sys.argv)==1:
  main()
elif len(sys.argv)==2:
  main(sys.argv[1])
elif len(sys.argv)==3:
  main(sys.argv[1], int(sys.argv[2]))
else:
  print("Uso:\n\t %s [host] [port]" % sys.argv[0])



