#include "csapp.h"

#define MAX_NAME_LEN 256
#define N_PROC 1

void echo(int connfd);

void SIGCHLD_handler(int sig)
{
    pid_t result;
    do
    {
        result = waitpid(-1, NULL, WNOHANG);
    } while (result > 0);

    if (errno != ECHILD && errno != EXIT_SUCCESS)
    {
        perror("Error on waiting: ");
        printf("%i", errno);
        exit(-1);
    }
}

pid_t childs[N_PROC];

void SIGINT_handler(int sig)
{
    for (int i = 0; i < N_PROC; ++i)
    {
        Kill(childs[i], SIGINT);
    }

    exit(0);
}

void sendData(struct sockaddr_in clientaddr, socklen_t clientlen, int connfd)
{
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    Getnameinfo((SA *) &clientaddr, clientlen,
                client_hostname, MAX_NAME_LEN, 0, 0, 0);

    /* determine the textual representation of the client's IP address */
    Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
              INET_ADDRSTRLEN);

    printf("server connected to %s (%s)\n", client_hostname,
           client_ip_string);
   // getchar();
    echo(connfd);
    Close(connfd);
}

/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main(int argc, char **argv)
{

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char new_dir[] = "/repserveur"; // the name of the new directory to add
    strcat(cwd, new_dir);
    chdir(cwd);
    Signal(SIGCHLD, SIGCHLD_handler);

    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;

    if (argc != 1)
    {
        fprintf(stderr, "usage: %s\n", argv[0]);
        exit(0);
    }
    port = 2121;

    clientlen = (socklen_t) sizeof(clientaddr);

    listenfd = Open_listenfd(port);

    for (int i = 0; i < N_PROC; i++)
    {
        if ((childs[i] = Fork()) == 0)
        {
            while (1)
            {
                while ((connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen)) < 0);

                sendData(clientaddr, clientlen , connfd);
                //getchar();
            }

            exit(0);
        }
    }

    Signal(SIGINT, SIGINT_handler);

    while(1)
        Pause();
}