//
//  main.c
//  beej-poll
//
//  Created by wan zhongwen on 2021/3/11.
//

/*
** pollserver.c -- a cheezy multiperson chat server
*/

#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT "9034"   // Port we're listening on
#define BACKLOG 10

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_litener_socket(void)
{
    int listener;
    int yes = 1;
    int rv;
    char s[INET6_ADDRSTRLEN];
    
    struct addrinfo hints, *ai, *p;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }
    
    for (p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }
        
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        
        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }
        break;
    }
    
    if (p == NULL) {
        return -1;
    }
    
    printf("server at %s:%s\n", inet_ntop(p->ai_family, get_in_addr(p->ai_addr), s, INET6_ADDRSTRLEN), PORT);
    
    freeaddrinfo(ai);
    
    if (listen(listener, BACKLOG) == -1) {
        return -1;
    }
    
    return listener;
}

void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
    if (*fd_count == *fd_size) {
        *fd_size *= 2;
        *pfds = reallocf(*pfds, sizeof(**pfds) * (*fd_size));
    }
    
    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN;
    (*pfds)[*fd_count].revents = 0;

    (*fd_count)++;
}

void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    pfds[i] = pfds[*fd_count - 1];
    pfds[i].revents = 0;
    (*fd_count)--;
}

int main(void)
{
    int listener;
    
    int newfd;
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;
    
    char buf[256];
    
    char remoteIP[INET6_ADDRSTRLEN];
    
    int fd_count = 0;
    int fd_size = 5;
    struct pollfd *pfds = malloc(sizeof(*pfds) * fd_size);
    
    listener = get_litener_socket();
    
    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(EXIT_FAILURE);
    }
    
    pfds[0].fd = listener;
    pfds[0].events = POLLIN;
    
    fd_count = 1;
    
    for (;;) {
        int poll_count = poll(pfds, fd_count, -1);
        
        if (poll_count == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }
        
        for (int i = 0; i < fd_count && poll_count > 0; i++) {
            if (pfds[i].revents & POLLIN) {
                poll_count--;
                if (pfds[i].fd == listener) {
                    addrlen = sizeof(remoteaddr);
                    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
                    
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        add_to_pfds(&pfds, newfd, &fd_count, &fd_size);
                        
                        printf("pollserver: new connection from %s on socket %d\n", inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remoteIP, INET6_ADDRSTRLEN), newfd);
                    }
                } else {
                    ssize_t nbytes = recv(pfds[i].fd, buf, sizeof(buf), 0);
                    int sender_fd = pfds[i].fd;
                    if (nbytes <= 0) {
                        if (nbytes == 0) {
                            printf("pollserver: socket %d hung up\n", sender_fd);
                        } else {
                            perror("recv");
                        }
                        close(pfds[i].fd);
                        
                        del_from_pfds(pfds, i, &fd_count);
                    } else {
                        for (int j = 0; j < fd_count; j++) {
                            int dest_fd = pfds[j].fd;
                            
                            if (dest_fd != listener && dest_fd != sender_fd) {
                                if (send(dest_fd, buf, nbytes, 0) == -1) {
                                    perror("send");
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return 0;
}
