#include "csapp.h"
#include <time.h>

int main(int argc, char **argv)
{
    char cwd[1024];
    // Récupere le repertoire courant
    getcwd(cwd, sizeof(cwd));
    // Le nom de répertoire à ajouter
    char new_dir[] = "/repclient"; 
    strcat(cwd, new_dir);
    chdir(cwd);
    int clientfd, port;
    char *host, buf[MAXLINE];
    rio_t rio;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    // Fixer le port à 2121
    port = 2121;

    clientfd = Open_clientfd(host, port);
    printf("client connected to server OS\n"); 
    Rio_readinitb(&rio, clientfd);

    // Envoyer le nom du fichier au serveur
    printf("ftp > ");
    Fgets(buf, MAXLINE, stdin);
    
    char copy[MAXLINE];
    strcpy(copy, buf);
    // Extraction du première token
    char * token = strtok(copy, " ");
    if(strcmp(token,buf)==0)
    {
        if(strncmp(buf,"bye",3)==0)
        {
            // Si le mot est bye on coupe la connexion
            printf("fermeture de connexion\n");
            // Le programme s'arrête
            exit(0);
        }
        else
        {
            // Si c'est ni get ni bye alors on demande de reécrire une commande
            printf("je ne comprends pas la commande\nget pour envoyer un fichier\nbye pour deconnecter\n");
            // Le programme s'arrête 
            exit(0);
        }
    }
    if(strcmp(token,"get")==0)
    {   
        // on récupere le nom du fichier
        token = strtok(NULL, " ");
    }
    // On envoie le nom de fichier
    Rio_writen(clientfd, token, strlen(token));

    // Lire la taille du fichier envoyé par le serveur
size_t file_size;
ssize_t read_size = Rio_readn(clientfd, &file_size, sizeof(file_size));

// Check if the server has closed the connection
if (read_size == 0)
{
    printf("La connexion avec le serveur a été interrompue prématurément.\n");
    exit(EXIT_FAILURE);
}
size_t data_size = file_size;


    //creer le fichier pour écrire dedans 
    char * file = token;
    FILE *f;
    if ((f = fopen(file, "a")) == NULL) 
    {
        perror("Erreur lors de l'ouverture du fichier %s :\n");
        exit(EXIT_FAILURE);
    }
    // Record the start time
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Lire le contenu du fichier
    while (data_size > 0) {
        ssize_t n = recv(clientfd, token, MAXLINE, 0);
         if (n == 0) 
        {
            // Connexion interrompu
            printf("La connexion avec le serveur a été interrompue prématurément.\n");
            exit(EXIT_FAILURE);
        }
        else if (n < 0) 
        {
            // Erreur de lecture depuis le serveur
            perror("Erreur de lecture depuis le serveur : \n");
            exit(EXIT_FAILURE);
        }
        // Ecrire dans le fichier
        fwrite(token, 1, n, f);
        data_size -= n;
    }
    
    // Record the end time
    clock_gettime(CLOCK_MONOTONIC, &end);

    fclose(f);

    // Calcule des statistiques
    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Transfer successfully complete.\n");
    printf("%zd bytes received in %.2f seconds (%.2f Kbytes/s).\n", 
    file_size, elapsed_time, 
    ((double)file_size / 1024) / elapsed_time);


    if (close(clientfd) < 0)
    {
        // Erreur de fermeture de connexion
        perror("Erreur lors de la fermeture de la connexion : \n");
        exit(EXIT_FAILURE);
    }
    exit(0);
}