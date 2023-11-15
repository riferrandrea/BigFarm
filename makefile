# definizione del compilatore e dei flag di compilazione
# che vengono usate dalle regole implicite
CC=gcc
CFLAGS=-g -Wall -O -std=gnu99
LDLIBS=-lm -lrt -pthread 

# eseguibili da costruire
EXECS=farm client

.PHONY: clean

# di default make cerca di realizzare il primo target 
all: $(EXECS)

# non devo scrivere il comando associato ad ogni target 
# perché il default di make in questo caso va bene

farm: farm.o xerrori.o
client: client.o xerrori.o


# target che cancella eseguibili e file oggetto
clean:
	rm -f $(EXECS) *.o  

# target che crea l'archivio dei sorgenti
zip:
	zip bigfarm.zip makefile *.c *.h *.py *.md
