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
#include <unistd.h>

#define TAILLE_MAX_NOM 256
#define MAX_CLIENT 10


#define ROUGE  1
#define VERT  2
#define JAUNE  3
#define BLEU  4
#define MAGENTA  5
#define CYAN  6
#define BLANC  7


#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

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


char* coloriser(char* msg, int choix){
    char *colored_msg = malloc(sizeof (*colored_msg)*strlen(msg)+14);

    switch(choix) {
        case ROUGE :
            strcpy(colored_msg, KRED);
            break;
        case VERT :
            strcpy(colored_msg, KGRN);
            break;
        case JAUNE :
            strcpy(colored_msg, KYEL);
            break;
        case BLEU :
            strcpy(colored_msg, KBLU);
            break;
        case MAGENTA :
            strcpy(colored_msg, KMAG);
            break;
        case CYAN :
            strcpy(colored_msg, KCYN);
            break;
        case BLANC :
            strcpy(colored_msg, KWHT);
            break;                                                      
        default :
            printf("Erreur lors de la colorisation\n" );
    }
    strcat(colored_msg, msg);
    strcat(colored_msg, KNRM);
    return colored_msg;
}

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

/**Envoie un message à un client
@param client émetteur du message
@param client destinataire du message
@param msg le message
*/
void chuchoter(Client * client,Client * dest, char msg[]){
    char *answer = malloc (sizeof (*answer) * 256);
    strcpy(answer, (*client).pseudo);
    strcat(answer," vous chuchotte : ");
    strcat(answer,msg);
    if((write((*dest).socket, coloriser(answer,MAGENTA),strlen(answer)+16)) < 0){
        perror("erreur : impossible d'ecrire le message destine au serveur.");
        exit(1);
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
    int i= 0;
    for(i; i<taille_tab_clients; i++){
        if(tab_clients[i].socket == (*client_supprime).socket){
            close(tab_clients[i].socket);//fermeture de la socket
            tab_clients[taille_tab_clients].is_connected = false; 
            int k = i;
            for(k; k<(taille_tab_clients-1); k++){//suppression dans le tableau
                tab_clients[k] = tab_clients[k+1];
            }
        }
    }
    taille_tab_clients--;
}
/*------------------------------------------------------*/
/**Verifie si le pseudo respecte les standards du serveur
cad caractère alphanumérique uniquement
@param pseudo le pseudo choisi
@param longueur la longueur du pseudo
@return vrai si le pseudo est correct faux sinon
**/
bool verification_pseudo(char pseudo[], int longueur){
		int i =0;
        bool resultat = true;
        //if(longueur>49)
          //  resultat = false;
		while(i<longueur && resultat ){
            if(!(pseudo[i]>=48 && pseudo[i]<=57)){
                if(!(pseudo[i]>=65 && pseudo[i]<=90)){
        			if(!(pseudo[i]>=97 && pseudo[i]<=122)){
        				resultat = false;
        			}
                }
            }
			i++;
		}		
		return resultat;
}

//Traite les commandes reçues par le client
static void * commande (void * c){
    Client * client = (Client *) c;
	char buffer[256];
    char substr[10];
	char *answer = malloc (sizeof (*answer) * 256);
	int longueur;
    int i;
    char *error = malloc(sizeof(*error)*57);
    //si whisper
    Client * dest;
    char pseudo[50];
    //
    int nonValid = 0;
    //Si le client n'a pas de pseudo
    while(strlen((*client).pseudo)<=1){
        longueur = read((*client).socket, buffer, sizeof(buffer));
        buffer[longueur]='\0';
        nonValid = 0;
        for(i=0;i<taille_tab_clients;i++){
            if(strcmp(tab_clients[i].pseudo,buffer) == 0)
                nonValid = 29;
            else if(!verification_pseudo(buffer, longueur))
				nonValid = 30;
        }
        if(nonValid == 0){
            strcpy((*client).pseudo, buffer);
            write(1,buffer,longueur);//affichage coté serveur
            buffer[0]='O';
            buffer[1]='K';
            buffer[2]='\0';
            write((*client).socket,buffer,2);
            write(1,buffer,2);
        }else if(nonValid == 29){
            write((*client).socket,"ERROR_29",8);
            write(1,"ERROR_29",8);
        }else{
            write((*client).socket,"ERROR_30",8);
            write(1,"ERROR_30",8);
        }
    }
	
	
    while(1){
    	longueur = read((*client).socket, buffer, sizeof(buffer));

        buffer[longueur]='\0';
        
        //récupération des 3 premiers caractères, utile si la chaine contient /w
        if(longueur>=3){
            memcpy( substr, &buffer[0], 3 );
            substr[3] = '\0';
        }
    	
    	// Quitter le serveur
    	if(strcmp(buffer,"/q")==0){
    		printf("%s a entré la commande /q\n", (*client).pseudo);
            strcpy(answer, (*client).pseudo);
            strcat(answer," a quitté le serveur.\n");
           	//coloriser(answer, 'm');
           	envoyer_message(client, coloriser(answer,ROUGE));
           	supprimerUtilisateur(client);
            pthread_exit(NULL);
    	}
    	// Lister les utilisateurs connectés
    	else if(strcmp(buffer,"/l")==0){
    		printf("%s a entré la commande /l\n", (*client).pseudo);
    		strcpy(answer, coloriser(listeClient(),VERT));
    		//coloriser(answer, 'v');
    		write((*client).socket,answer,strlen(answer)+1); 
    	}
        else if(strcmp(buffer,"/h")==0){
            printf("%s a entré la commande /h\n", (*client).pseudo);
            strcpy(answer, "__________________________\n                          \n/w <utilisateur> <message> - Envoyer un message privé à un utilisateur\n/q - Quitter le serveur\n/l - Lister les utilisateurs connectés\n/h - Afficher les commandes\n__________________________\n\n");
            
            write((*client).socket,answer,strlen(answer)+1);  
        }  
        else if(strcmp(substr,"/w ")==0){
            bool trouve = false;
            printf("%s a entré la commande /w \n", (*client).pseudo);
            //buffer[8]='X';//suppression de l'espace
            
            if(longueur>3){
                char * start = strchr(&buffer,' ');
					start++;//ptr vers 1ere lettre du pseudo
					if(strlen(start)>0){
						char * end = strchr(start,' ');//ptr vers fin du pseudo
						if(end != NULL){
							char * msg = end+1;
							if(strlen(msg)>0){
								*end = '\0';
								printf("destiné à %s ", start);
								printf(" message: %s\n", msg);
						
								for (i=0;i<taille_tab_clients;i++){
									if(strcmp(start,tab_clients[i].pseudo)==0){
										trouve = true;
										dest = &tab_clients[i];
									}     
								}
								if(trouve){
									printf("destinataire trouvé \n");
									chuchoter(client,dest,msg);
								}
								else{
									printf("ERREUR destinataire inconnu \n");
                                    strcpy(error,"Serveur : ERREUR destinataire inconnu.");
                                    write((*client).socket,coloriser(error,JAUNE),52);
								}
								
							}else{
								printf("ERREUR PAS DE MESSAGE \n");
                                strcpy(error,"Serveur : \\w <utilisateur> <message>");
                                write((*client).socket,coloriser(error,JAUNE),56);
                            }
						
						}else{
							printf("ERREUR PAS DE MESSAGE \n");
                            strcpy(error,"Serveur : \\w <utilisateur> <message>");
                            write((*client).socket,coloriser(error,JAUNE),56);
                        }
					}
            }
            

        }
        //cas d'une commande mal écrite
        else if(buffer[0]=='/'){
            strcpy(error,"Serveur : Commande inconnue");
			write((*client).socket,coloriser(error,JAUNE),42);
			write(1,"ERROR_12\n",9);
		}  
        //Cas d'un message normale
        else if(longueur > 0){
            envoyer_message(client, coloriser(buffer,BLEU));	
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
                    tab_clients[taille_tab_clients].is_connected = true;                    
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
