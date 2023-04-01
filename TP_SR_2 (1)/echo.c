#include <errno.h>
#include "csapp.h"
#include <unistd.h>

int inter = 0;

void signal_handler(int signum) {
    inter = 1;
}

void echo(int connfd) {
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while (1) {
        ssize_t n = Rio_readlineb(&rio, buf, MAXLINE);
         if (n <= 0) {
         if (n < 0) {
            perror("Erreur de lecture du nom de fichier depuis le client\n");
        } else {
            printf("Client déconnecté\n");
        }
        close(connfd);
        break;
    }
        buf[n - 1] = '\0'; // Enlever le caractère de fin de ligne
        if (strcmp(buf, "bye") == 0) {
            break;
        }
        printf("Nom de fichier reçu : %s\n", buf);

        int fd = Open(buf, O_RDONLY, 0);
        if (fd < 0) {
            perror("Erreur lors de l'ouverture du fichier , Erreur :\n");
            close(connfd);
            return;
        }

        struct stat file_stat;
        if (fstat(fd, &file_stat) == -1) {
            perror("Erreur lors de l'obtention de la taille du fichier \n");
            Close(fd);
            close(connfd);
            return;
        }
        size_t file_size = file_stat.st_size;

        // Envoyer la taille du fichier au client
        Rio_writen(connfd, &file_size, sizeof(file_size));

        // Lire combien le client a reçu des données
        off_t received;
        Rio_readn(connfd, &received, sizeof(received));

        // Déplacer le pointeur
        lseek(fd, received, SEEK_SET);

        char file_buf[MAXBUF];
        ssize_t nb;
        while ((nb = read(fd, file_buf, MAXBUF)) > 0) {
            // fprintf(stderr, "nb = %zd", nb);
            if (inter) {
                printf("Transfert interrompu.\n");
                break;
            }
            int interrupted;

            Rio_readn(connfd, &interrupted, sizeof(interrupted));
            // printf("Interruped : %i\n", interrupted);
            if (interrupted == 1)
                break;
            if (send(connfd, file_buf, nb, 0) == -1) {
                perror("Erreur lors de l'envoi du contenu du fichier : %s\n");
                Close(fd);
                return;
            }
            // sleep(1);
        }

        if (nb == -1) {
            perror("Erreur lors de la lecture du contenu du fichier : \n");
        }
    }
}
