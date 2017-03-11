/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <hostname>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <string.h>

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;

void end_prg(int socket_descriptor){
    close(socket_descriptor);
    printf("connexion avec le serveur fermee, fin du programme.\n");
    exit(0); 
}

void analyse(char msg[],int socket){
    if(strcmp(msg,"/q")==0){
        end_prg(socket);
    }
}

void * envoi(void * socket_descriptor){
	char msg[256];
	int socket = (int) socket_descriptor;
	while(1){
		memset(msg,0,256);
		fgets(msg,256,stdin);
        msg[strcspn(msg, "\n")] = '\0';//remplace /n par \0
		if((write(socket,msg,strlen(msg)))<0)
		{
			perror("erreur : impossible d'ecrire le message destine au serveur.");
    	}
    	analyse(msg,socket);
	}
}

void * reception(void * socket_descriptor){
	int longueur;
	char buffer[256];
	int socket = (int) socket_descriptor;
	while(1){
		longueur = 0;
		memset(buffer,0,256);
		while((longueur = read(socket, buffer, sizeof(buffer))) > 0) {
            buffer[strcspn(buffer, "\0")] = '\n';
			write(1,buffer,longueur);
		}
	}		
}

int main(int argc, char **argv) {
    int     		socket_descriptor, 	/* descripteur de socket */
		    		longueur; 		    /* longueur d'un buffer utilisé */
    sockaddr_in 	adresse_locale; 	/* adresse de socket local */
    hostent *		ptr_host; 		    /* info sur une machine hote */
    servent *		ptr_service; 		/* info sur service */
    char 			buffer[256];
    char *			prog; 			    /* nom du programme */
    char *			host; 			    /* nom de la machine distante */

    char			pseudo[50];

    pthread_t thread_envoi, thread_reception;
    
    if (argc != 2) {
		perror("usage : client <adresse-serveur>");
		exit(1);
    }
    prog = argv[0];
    host = argv[1];
    printf("nom de l'executable : %s \n", prog);
    printf("adresse du serveur  : %s \n", host);
    if ((ptr_host = gethostbyname(host)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son adresse.");
		exit(1);
    }
    
    /* copie caractere par caractere des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; 
    

    
    if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
		perror("erreur : impossible de recuperer le numero de port du service desire.");
		exit(1);
    }
    adresse_locale.sin_port = htons(ptr_service->s_port);
    
    /*-----------------------------------------------------------*/
    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le serveur.");
		exit(1);
    }
    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de se connecter au serveur.");
		exit(1);
    }
    printf("connexion etablie avec le serveur. \n");
    
    /*-----------------------------------------------------------*/
    
    /** Création du pseudo **/
    
    
    while(strcmp(buffer,"OK")!=0){
        printf("Entrez votre pseudo: \n");
        fgets(pseudo, sizeof pseudo, stdin);
        pseudo[strcspn(pseudo, "\n")] = '\0'; //enlève le caractère de saut de ligne 
        if ((write(socket_descriptor, pseudo, strlen(pseudo))) < 0) {
            perror("erreur : impossible d'ecrire le message destine au serveur.");
            exit(1);
        }
        longueur = read(socket_descriptor, buffer, sizeof(buffer));
        buffer[longueur] = '\0';
        if(strcmp(buffer,"ERROR_30") == 0)
            printf("Pseudo déjà utilisé, choisissez en un autre\n");
    }
    
    
    printf("__________________________\n");
    printf("                          \n");
    printf("/l          - Lister les utilisateurs connectés\n");
    printf("/h          - Lister les commandes\n");
    printf("/q          - Quitter le serveur\n");
    printf("__________________________\n\n");
    printf("Bonjour %s .\n", pseudo);
    
    
    
    
    
    /* envoi du message vers le serveur */
    if(pthread_create(&thread_envoi,NULL,envoi,(void *)socket_descriptor) != 0){
    	perror("erreur création du thread d'envoi des messages");
    	exit(1);	
    }
    /* lecture de la reponse en provenance du serveur */
    if(pthread_create(&thread_reception,NULL,reception,(void *)socket_descriptor) != 0){
    	perror("erreur création du thread d'envoi des messages");
    	exit(1);	
    } 

    pthread_join(thread_envoi,NULL);
    pthread_join(thread_reception,NULL);
}

