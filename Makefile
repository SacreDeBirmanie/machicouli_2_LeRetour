all: clean client serveur

clean:
	rm -f bin/*

client:
	gcc -pthread -o bin/client src/client.c 

serveur :
	gcc -pthread -o bin/serveur src/serveur.c 

