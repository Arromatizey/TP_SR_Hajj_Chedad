#include "csapp.h"
#include <string.h>

int interrupted = 0;

// handler de sigint
void handler(int sig) {
    interrupted = 1;
}

void receive(int sockfd, const char *filename, size_t file_size) {
    // On crée un fichier tmp.txt pour afficher les donnees 
    char filetmp[MAXLINE];
    strcpy(filetmp, "tmp.txt");
    FILE *tmpfile = fopen(filetmp, "a");
    if (tmpfile == NULL) {
        perror("Erreur lors de l'ouverture du fichier temporaire: \n");
        return;
    }

    fseek(tmpfile, 0, SEEK_END);
    off_t data_received = ftell(tmpfile);
    fseek(tmpfile, 0, SEEK_SET);

    // Envoyer combien de data j'ai reçu au serveur
    Rio_writen(sockfd, &data_received, sizeof(data_received));

    char buf[MAXBUF];
    // Calculer combien de donnée il reste à envoyer
    size_t rest = file_size - data_received;
    while (rest > 0) {
        //printf("\nfile_size : %d\n ", file_size);
        //printf("\ndata_received : %d\n ", data_received);
        //printf("\rrest : %d\n ", rest);

        // Envoyer au serveur si on a une interruption
        Rio_writen(sockfd, &interrupted, sizeof(interrupted));
        if (interrupted == 1) {
            // on arrete à cause d'une interruption
            printf("Transfert interrompu.\n");
            break;
        }
        size_t data_rest = (rest > MAXBUF) ? MAXBUF : rest;
         usleep(1000000); // pause de 0.1 seconde
        ssize_t data_rec = Rio_readn(sockfd, buf, data_rest);

        if (data_rec <= 0) {
            perror("Erreur lors de la réception du fichier: \n");
            break;
        }
        // On ecrit les données reçus
        fwrite(buf, 1, data_rec, tmpfile);
        rest -= data_rec;
    }

    fclose(tmpfile);

    if (rest == 0) {
        // si il y a plus des données qui restent alors on nomme le tmp.txt par le nom du fichier ( transfert fini )
        if (rename(filetmp, filename) == -1) {
            perror("Erreur lors du renommage du fichier temporaire: \n");
        }
    }
}

int main(int argc, char **argv) {
    signal(SIGINT, handler);
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char new_dir[] = "/repclient"; // the name of the new directory to add
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
    port = 2121;

    clientfd = Open_clientfd(host, port);
    printf("client connected to server OS\n");
    Rio_readinitb(&rio, clientfd);

    while (1) {
        // Envoyer le nom du fichier au serveur
        printf("ftp> ");
        Fgets(buf, MAXLINE, stdin);

        char copy[MAXLINE];
        strcpy(copy, buf);
        // Extract the first token
        char *token = strtok(copy, " ");
        if (strcmp(token, buf) == 0) {
            if (strncmp(buf, "bye", 3) == 0) {
                //deconnexion
                printf("fermeture de connexion\n");
                break;
            } else {
                printf("je ne comprends pas la commande\nget pour envoyer un fichier\nbye pour deconnecter\n");
                continue;
            }
        }
        if (strcmp(token, "get") == 0) {

            token = strtok(NULL, " ");
        }
        //on envoie le nom de fichier
        Rio_writen(clientfd, token, strlen(token));

        // Lire la taille du fichier envoyé par le serveur
        size_t file_size;
        size_t read_size = Rio_readn(clientfd, &file_size, sizeof(file_size));
        // Check if the server has closed the connection
        if (read_size == 0)
        {
              printf("La connexion avec le serveur a été interrompue prématurément.\n");
            exit(EXIT_FAILURE);
        }
       

        //creer le fichier pour écrire dedans
        char *file = token;
        FILE *f;
        if ((f = fopen(file, "a")) == NULL) {
            perror("Erreur lors de l'ouverture du fichier  : \n");
            exit(EXIT_FAILURE);
        }
        // Record the start time
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        // Lire le contenu du fichier
        receive(clientfd, file, file_size);
        if (interrupted == 1) {
            interrupted = 0;
            continue;
        }

        // Record the end time
        clock_gettime(CLOCK_MONOTONIC, &end);

        fclose(f);

        double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

        printf("Transfer successfully complete.\n");
        printf("%zd bytes received in %.2f seconds (%.2f Kbytes/s).\n",
               file_size, elapsed_time,
               ((double) file_size / 1024) / elapsed_time);

    }
    if (close(clientfd) < 0) {
        perror("Erreur lors de la fermeture de la connexion : \n");
        exit(EXIT_FAILURE);
    }
    exit(0);
}
