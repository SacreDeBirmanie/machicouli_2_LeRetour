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
#include <gtk/gtk.h>

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;

typedef struct{
    int socket;               //socket
    GtkTextBuffer *buffer;
} Message;


Message msg;

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

void envoi(int socket, char msg[]){
    //msg[strcspn(msg, "\n")] = '\0';//remplace /n par \0
	if((write(socket,msg,strlen(msg)))<0)
	{
		perror("erreur : impossible d'ecrire le message destine au serveur.");
	}
	analyse(msg,socket);
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


void * build_ui(){
    gtk_main ();
}

void
on_window_destroy (GtkWidget *widget, gpointer data)
{
  gtk_main_quit ();
}

/* Callback for close button */
void
on_button_clicked (GtkWidget *button, gpointer data)
{
  GtkTextIter start;
  GtkTextIter end;

  gchar *text;

  Message message;
  message.buffer = data->buffer;
  message.socket = data->socket;

  /* Obtain iters for the start and end of points of the buffer */
  gtk_text_buffer_get_start_iter (message.buffer, &start);
  gtk_text_buffer_get_end_iter (message.buffer, &end);

  /* Get the entire buffer text. */
  text = gtk_text_buffer_get_text (message.buffer, &start, &end, FALSE);

  /* Print the text */
  //g_print ("%s", text);
  envoi(message.socket,text);
  g_free (text);
}

/*
void choisir_pseudo(GtkWidget *buffer_receive,int socket_descriptor, char buffer[]){

    while(strcmp(buffer,"OK")!=0){
        gtk_text_buffer_set_text (buffer_receive, "Entrez votre pseudo", -1);
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
    

}*/

int main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *text_view_send;
  GtkWidget *text_view_receive;
  GtkWidget *button;
  GtkTextBuffer *buffer_send;
  GtkTextBuffer *buffer_receive;
  
  int socket_descriptor,  /* descripteur de socket */
      longueur;         /* longueur d'un buffer utilisé */
      sockaddr_in   adresse_locale;   /* adresse de socket local */
      hostent *   ptr_host;         /* info sur une machine hote */
      servent *   ptr_service;    /* info sur service */
  char buffer[256];
  char * prog;           /* nom du programme */
  char * host;           /* nom de la machine distante */
  char pseudo[50];
  pthread_t thread_ui, thread_reception;
  
  gtk_init (&argc, &argv);

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
  








  /* Create a Window. */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Messagerie Instantanée En Ligne");

  /* Set a decent default size for the window. */
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
  g_signal_connect (G_OBJECT (window), "destroy", 
                    G_CALLBACK (on_window_destroy),
                    NULL);

  vbox = gtk_box_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  /* Create a multiline text widget. */
  text_view_send = gtk_text_view_new ();
  gtk_box_pack_start (GTK_BOX (vbox), text_view_send, 1, 1, 0);

    /* Create a multiline text widget. */
  text_view_receive = gtk_text_view_new ();
  gtk_box_pack_start (GTK_BOX (vbox), text_view_receive, 1, 1, 0);

  /* Obtaining the buffer associated with the widget. */
  buffer_send = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view_send));
  /* Set the default buffer text. */ 
  gtk_text_buffer_set_text (buffer_send, "Envoyer un message", -1);
  
  /* Obtaining the buffer associated with the widget. */
  buffer_receive = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view_receive));
  /* Set the default buffer text. */ 
  gtk_text_buffer_set_text (buffer_receive, "Bienvenue dans le chat", -1);

  msg.socket = socket_descriptor;
  msg.buffer = buffer_send;

  /* Create a close button. */
  button = gtk_button_new_with_label ("Envoyer");
  gtk_box_pack_start (GTK_BOX (vbox), button, 0, 0, 0);
  g_signal_connect (G_OBJECT (button), "clicked", 
                    G_CALLBACK (on_button_clicked),
                    &msg);
  
  gtk_widget_show_all (window);
    
    /* thread ui */
    if(pthread_create(&thread_ui,NULL,build_ui,NULL)){
      perror("erreur création du thread d'ui");
      exit(1);  
    }
    /* lecture de la reponse en provenance du serveur */
   /* if(pthread_create(&thread_reception,NULL,reception,(void *)socket_descriptor) != 0){
      perror("erreur création du thread de reception des messages");
      exit(1);  
    } */
    pthread_join(thread_ui,NULL);
    //pthread_join(thread_reception,NULL);
  return 0;
}


/*
int main(int argc, char **argv) {
    int     		socket_descriptor, 	// descripteur de socket 
		    		longueur; 		    // longueur d'un buffer utilisé 
    sockaddr_in 	adresse_locale; 	// adresse de socket local 
    hostent *		ptr_host; 		    // info sur une machine hote 
    servent *		ptr_service; 		// info sur service 
    char 			buffer[256];
    char *			prog; 			    // nom du programme 
    char *			host; 			    // nom de la machine distante 

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
    
    // copie caractere par caractere des infos de ptr_host vers adresse_locale 
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; 
    

    
    if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
		perror("erreur : impossible de recuperer le numero de port du service desire.");
		exit(1);
    }
    adresse_locale.sin_port = htons(ptr_service->s_port);
    
    //*-----------------------------------------------------------
    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));
    //* creation de la socket 
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le serveur.");
		exit(1);
    }
    //* tentative de connexion au serveur dont les infos sont dans adresse_locale 
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de se connecter au serveur.");
		exit(1);
    }
    printf("connexion etablie avec le serveur. \n");
    
    //*-----------------------------------------------------------
    
    //** Création du pseudo 
    
    
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
    
    
    
    
    
    //* envoi du message vers le serveur 
    if(pthread_create(&thread_envoi,NULL,envoi,(void *)socket_descriptor) != 0){
    	perror("erreur création du thread d'envoi des messages");
    	exit(1);	
    }
    //* lecture de la reponse en provenance du serveur 
    if(pthread_create(&thread_reception,NULL,reception,(void *)socket_descriptor) != 0){
    	perror("erreur création du thread d'envoi des messages");
    	exit(1);	
    } 

    pthread_join(thread_envoi,NULL);
    pthread_join(thread_reception,NULL);
}
*/
