#include <errno.h>
#include "csapp.h"

void echo(int connfd)
{
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    ssize_t n = Rio_readlineb(&rio, buf, MAXLINE);
    if (n <= 0) {
        perror("Erreur de lecture du nom de fichier depuis le client\n");
        return;
    }
    // Enlever le caractère de fin de ligne
    buf[n-1] = '\0';
    printf("Nom de fichier reçu : %s\n", buf);

    // Ouverture de fichier 
    int fd = Open(buf, O_RDONLY, 0);
    if (fd < 0) {
        perror("Erreur lors de l'ouverture du fichier , Erreur : \n");
        return;
    }

    // Récupere la taille du fichier
    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        printf("Erreur lors de l'obtention de la taille du fichier : %s\n", strerror(errno));
        Close(fd);
        return;
    }
    size_t file_size = file_stat.st_size;


    // Envoyer la taille du fichier au client
    Rio_writen(connfd, &file_size, sizeof(file_size));

    // allocation de la memoire pour la taille du fichier
    char *buffer = malloc(file_size * sizeof(char));

    // lire le fichier dans la memoire
    ssize_t nb = read(fd, buffer, file_size);
    if (nb < 0)
    {
        perror("Erreur lors de la lecture du contenu du fichier : \n");
        Free(buffer);
        Close(fd);
        return;
    }

    // Envoyer le fichier au client
    if (send(connfd, buffer, file_size, 0) == -1)
    {
        perror("Erreur lors de l'envoi du contenu du fichier : \n");
        Free(buffer);
        Close(fd);
        return;
    }


    // liberer la mémoire 
    Free(buffer);
    Close(fd);
}