
/*----------------------------------------------
Serveur à lancer avant le client
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h> 
/* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> 
/* pour hostent, servent */
#include <string.h> 
/* pour bcopy, ... */  
#include <pthread.h>

#define TAILLE_MAX_NOM 256
#define MAX_CLIENT 10

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

struct arg_connexion
{
    sockaddr_in adresse_client_courant;
    int socket;
    int (*clients);
    int* nb_client;
};

/*------------------------------------------------------*/
void renvoi (int sock,int nb_client,int (*clients)) {
    char buffer[256];
    int longueur;
    while(1){
        longueur = 0;
        memset(buffer,0,256);
        if ((longueur = read(sock, buffer, sizeof(buffer))) <= 0) 
            return;
        printf("message lu : %s \n", buffer);
        printf("renvoi du message.\n");
        /*for(int i=0;i<nb_client;i++){
            write(&clients[i],buffer,strlen(buffer)+1);    
        }*/
        write(sock,buffer,strlen(buffer)+1); 
        printf("message envoye. \n");       
        return;
    }
}
/*------------------------------------------------------*/

void * connexion(void * args){
    struct arg_connexion *co_args = (struct arg_connexion *)args;
    int nouv_socket_descriptor;
    int socket_descriptor = (int)co_args->socket;
    int longueur_adresse_courante = sizeof(co_args->adresse_client_courant);
    /* adresse_client_courant sera renseignée par accept via les infos du connect */
    if (
        (nouv_socket_descriptor = accept(socket_descriptor, 
        (sockaddr*)(&co_args->adresse_client_courant),
        &longueur_adresse_courante))
    < 0) {
        perror("erreur : impossible d'accepter la connexion avec le client.");
        exit(1);
    }
    /* traitement du message */
    int (*clients) = co_args->clients;
    int* nb_client = co_args->nb_client;

    //(*clients)[*nb_client] = nouv_socket_descriptor;
    *nb_client = *nb_client + 1;
    
    printf("reception d'un message.\n");
    renvoi(nouv_socket_descriptor,*nb_client,clients);
}


/*------------------------------------------------------*/
main(int argc, char **argv) {
    int (*clients)[MAX_CLIENT];
    int* nb_client;
    *nb_client = 0;
    pthread_t thread_connexion; 

    int 
socket_descriptor, 
/* descripteur de socket */
nouv_socket_descriptor, 
/* [nouveau] descripteur de socket */
longueur_adresse_courante; 
/* longueur d'adresse courante d'un client */
    sockaddr_in 
adresse_locale, 
/* structure d'adresse locale*/
       adresse_client_courant; 
/* adresse client courant */
    hostent*
ptr_hote; 
/* les infos recuperees sur la machine hote */
    servent*
ptr_service; 
/* les infos recuperees sur le service de la machine */
    char 
machine[TAILLE_MAX_NOM+1]; 
/* nom de la machine locale */
    gethostname(machine,TAILLE_MAX_NOM);
/* recuperation du nom de la machine */
    /* recuperation de la structure d'adresse en utilisant le nom */
    if ((ptr_hote = gethostbyname(machine)) == NULL) {
        perror("erreur : impossible de trouver le serveur a partir de son nom.");
        exit(1);
    }    
    /* initialisation de la structure adresse_locale avec les infos recuperees */
    /* copie de ptr_hote vers adresse_locale */
    bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
    adresse_locale.sin_family
= ptr_hote->h_addrtype; 
/* ou AF_INET */
    adresse_locale.sin_addr.s_addr
= INADDR_ANY; 
/* ou AF_INET */
    /* 2 facons de definir le service que l'on va utiliser a distance */
    /* (commenter l'une ou l'autre des solutions) */
    /*-----------------------------------------------------------*/
    /* SOLUTION 1 : utiliser un service existant, par ex. "irc" */
    
    if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
        perror("erreur : impossible de recuperer le numero de port du service desire.");
        exit(1);
    }
    adresse_locale.sin_port = htons(ptr_service->s_port);
    /*-----------------------------------------------------------*/
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    /*
    adresse_locale.sin_port = htons(5000);
    /*-----------------------------------------------------------*/

    printf("numero de port pour la connexion au serveur : %d \n", 
            ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);
    /* creation de la socket */
    printf("create socket !");    
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("erreur : impossible de creer la socket de connexion avec le client.");
        exit(1);
    }
    printf("association socket !"); 
    /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
    if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
        perror("erreur : impossible de lier la socket a l'adresse de connexion.");
        exit(1);
    }
    printf("init !");
    /* initialisation de la file d'ecoute */
    listen(socket_descriptor,5);
    printf("listen !");
    /* attente des connexions et traitement des donnees recues */
    for(;;) {

        struct arg_connexion *args;
        args->adresse_client_courant = adresse_client_courant;
        args->socket = socket_descriptor;
        args->clients = *clients;
        args->nb_client = nb_client;
        if(pthread_create(&thread_connexion,NULL,connexion,(void *)args) != 0){
            perror("erreur création du thread de connexion");   
        } 
        pthread_join(thread_connexion,NULL);
    }    
}
