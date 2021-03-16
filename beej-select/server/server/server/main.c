//
//  main.c
//  server
//
//  Created by wan zhongwen on 2021/3/12.
//

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT "9034"  // file descriptor for standard input

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, const char * argv[])
{
    fd_set master;
    fd_set read_fds;
    int fdmax;
    
    int listener;
    int newfd;
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;
    
    char buf[256];
    ssize_t nbytes;
    
    char remoteIP[INET6_ADDRSTRLEN];
    int yes = 1;
    int i, j, rv;
    
    struct addrinfo hints, *ai, *p;
    
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }
    
    for (p = ai; p != NULL; p = p->ai_next) {
        if ((listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        
        if (bind(listener, p->ai_addr, p->ai_addrlen) == -1) {
            close(listener);
            continue;
        }
        
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(EXIT_FAILURE);
    }
    
    freeaddrinfo(ai); // all done with this
    
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    // add the listener to the master set
    FD_SET(listener, &master);
//    FD_SET(0, &master);
    
    fdmax = listener;
    
    for (;;) {
        FD_COPY(&master, &read_fds);
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }
        
        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == listener) {
                    addrlen = sizeof(remoteaddr);
                    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master);
                        if (newfd > fdmax) {
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on socket %d\n", inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remoteIP, INET6_ADDRSTRLEN), newfd);
                    }
                } else {
                    if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
                        if (nbytes == 0) {
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i);
                        FD_CLR(i, &master);
                    } else {
                        for (j = 0; j <= fdmax; j++) {
                            if (FD_ISSET(j, &master)) {
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        perror("send");
                                    }
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
