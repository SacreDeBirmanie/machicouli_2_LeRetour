/*----------------------------------------------
Serveur à lancer avant le client
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <linux/types.h> /* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> /* pour hostent, servent */
#include <string.h> /* pour bcopy, ... */  
#include <pthread.h>

#define TAILLE_MAX_NOM 256
#define MAX_CLIENT 10

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

//Définition d'un client
typedef struct{
    int socket;               //socket
    char pseudo[50];          //pseudo
    pthread_t thread;         //thread dans lequel les actions du client sont effectué
    bool is_connected;        //true si le client est connecté
} Client;

/*------------------------------------------------------*/
//Variables Globales//
Client tab_clients[MAX_CLIENT];           /*Tableau contenant les clients connectés ou non*/
int taille_tab_clients = 0;                   /*Nombre de clients */


/*------------------------------------------------------*/

/**Envoie un message à tous les clients
@param client émetteur du message
@param msg le message
*/
void envoyer_message(Client * client, char msg[]){
    char *answer = malloc (sizeof (*answer) * 256);
    strcpy(answer, (*client).pseudo);
    strcat(answer,": ");
    strcat(answer,msg);
    printf("%s\n", answer);
    int i;
    for (i=0;i<taille_tab_clients;i++){
        if(strcmp((*client).pseudo,tab_clients[i].pseudo)!=0){
            if((write(tab_clients[i].socket,answer,strlen(answer)+1)) < 0){
                perror("erreur : impossible d'ecrire le message destine au serveur.");
                exit(1);
            } 
        }     
    }    
}
/*------------------------------------------------------*/
//Affiche les informations relatives au clients connectés
void listerInfo(){
    int i = 0;
    for(i; i<taille_tab_clients; i++){
        printf("\nUtilisateur %i\n", i);
        printf("Pseudo: %s\n", tab_clients[i].pseudo);
        printf("Socket: %i\n", tab_clients[i].socket);
        printf("Connecté: %i\n\n", tab_clients[i].is_connected);
    }
}
/*------------------------------------------------------*/

char* listeClient(){
    char* res = malloc(taille_tab_clients*50*sizeof(char));
    strcpy(res,"Liste des clients connectés:");
    int i = 0;
    for(i; i<taille_tab_clients; i++){
        strcat(res, "\n - ");
        strcat(res, tab_clients[i].pseudo);
    }
    printf("\n");

    return res;
}
/*------------------------------------------------------*/

void supprimerUtilisateur(Client *client_supprime){
    Client tmp[MAX_CLIENT];
    int i,j = 0;
    for(i; i<taille_tab_clients; i++){
        if(tab_clients[i].socket != (*client_supprime).socket){
            tmp[j] = tab_clients[i];
            j++;
        }
        else{
            close(tab_clients[i].socket);
        }
    }
    taille_tab_clients--;
}
/*------------------------------------------------------*/


//Traite les commandes reçues par le client
static void * commande (void * c){
    Client * client = (Client *) c;
	char buffer[256];
	char *answer = malloc (sizeof (*answer) * 256);
	int longueur;

    //Si le client n'a pas de pseudo
    while(strlen((*client).pseudo)<=1){
        longueur = read((*client).socket, buffer, sizeof(buffer));
        sleep(3);
        buffer[longueur]='\0';
        int i;
        bool valid=true;
        for(i=0;i<taille_tab_clients;i++){
            if(strcmp(tab_clients[i].pseudo,buffer) == 0)
                valid = false;
        }
        if(valid){
            strcpy((*client).pseudo, buffer);
            write(1,buffer,longueur);//affichage coté serveur
            buffer[0]='O';
            buffer[1]='K';
            buffer[2]='\0';
            write((*client).socket,buffer,2);
            write(1,buffer,2);
        }else{
            write((*client).socket,"ERROR",5);
        }
    }
	
	
    while(1){
    	longueur = read((*client).socket, buffer, sizeof(buffer));

        buffer[longueur]='\0';

    	sleep(3);
    	// Quitter le serveur
    	if(strcmp(buffer,"/q")==0){
    		printf("%s a entré la commande /q\n", (*client).pseudo);
            strcpy(answer, (*client).pseudo);
            strcat(answer," a quitté le serveur.\n");
           	//coloriser(answer, 'm');
           	envoyer_message(client, answer);
           	supprimerUtilisateur(client);
            pthread_exit(NULL);
    	}
    	// Lister les utilisateurs connectés
    	else if(strcmp(buffer,"/l")==0){
    		printf("%s a entré la commande /l\n", (*client).pseudo);
    		strcpy(answer, listeClient());
    		//coloriser(answer, 'v');
    		write((*client).socket,answer,strlen(answer)+1); 
    	}
        else if(strcmp(buffer,"/h")==0){
            printf("%s a entré la commande /h\n", (*client).pseudo);
            strcpy(answer, "__________________________\n                          \n/q          - Quitter le serveur\n/l          - Lister les utilisateurs connectés\n/h          - Afficher les commandes\n__________________________\n\n");
            //coloriser(answer, 'v');
            write((*client).socket,answer,strlen(answer)+1);  
        }         
        //Cas d'un message normale
        else if(longueur > 0){
            envoyer_message(client, buffer);	
    	}
    }
}
/*------------------------------------------------------*/


int main(int argc, char **argv) {
  
    int             socket_descriptor, 			/* descripteur de socket */
	               	nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
			     	longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */
    sockaddr_in 	adresse_locale, 			/* structure d'adresse locale*/
			        adresse_client_courant; 	/* adresse client courant */
    hostent*		ptr_hote; 					/* les infos recuperees sur la machine hote */
    servent*		ptr_service; 				/* les infos recuperees sur le service de la machine */
    char 			machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */


    
    


    gethostname(machine,TAILLE_MAX_NOM);		/* recuperation du nom de la machine */
    
    /* recuperation de la structure d'adresse en utilisant le nom */
    if ((ptr_hote = gethostbyname(machine)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son nom.");
		exit(1);
    }
    
    /* initialisation de la structure adresse_locale avec les infos recuperees */			
    
    /* copie de ptr_hote vers adresse_locale */
    bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
    adresse_locale.sin_family		= ptr_hote->h_addrtype; 	/* ou AF_INET */
    adresse_locale.sin_addr.s_addr	= INADDR_ANY; 			/* ou AF_INET */


    //attribution d'un port
    if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
        perror("erreur : impossible de recuperer le numero de port du service desire.");
        exit(1);
    }
    adresse_locale.sin_port = htons(ptr_service->s_port);
    
    printf("numero de port pour la connexion au serveur : %d \n", 
		   ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);
    
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le client.");
		exit(1);
    }

    /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
    if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de lier la socket a l'adresse de connexion.");
		exit(1);
    }

    /* initialisation de la file d'ecoute */
    listen(socket_descriptor,5);

    /* attente des connexions et traitement des donnees recues */
    while(1) {
    
		longueur_adresse_courante = sizeof(adresse_client_courant);
		
        /*vérification des slots dispo sur le serveur*/
		if (taille_tab_clients >= MAX_CLIENT) {
            perror("Echec de la connexion, aucune place restante sur le serveur.");
			exit(1);
		}
        else{
            //si le client n'est pas encore connecté
            if(!tab_clients[taille_tab_clients].is_connected){
                if ((nouv_socket_descriptor = accept(socket_descriptor,(sockaddr*)(&adresse_client_courant),&longueur_adresse_courante))< 0) {
                    perror("erreur : impossible d'accepter la connexion avec le client.");
                    exit(1);
                }
                //création d'un thread pour le client
                else{
                    tab_clients[taille_tab_clients].pseudo[0] = '\0';
                    tab_clients[taille_tab_clients].is_connected = 1;                    
                    tab_clients[taille_tab_clients].socket = nouv_socket_descriptor;
                    if(pthread_create(&tab_clients[taille_tab_clients].thread, NULL, commande, &tab_clients[taille_tab_clients]) != 0){
                        perror("erreur dans la création du thread client");   
                    }
                    else{
                        taille_tab_clients++;
                    }
                }                
            }
        }
    }

    return 0;  
}
