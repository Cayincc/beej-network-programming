//
//  main.c
//  sender
//
//  Created by wan zhongwen on 2021/3/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

int sendall(int s, char *buf, int *len)
{
    int total = 0;
    int bytesleft = *len;
    int n = -1;
    
    while (total < *len) {
        n = send(s, buf, bytesleft, 0);
        if (n == -1) {
            break;
        }
        total += n;
        bytesleft -= n;
    }
    
    *len = total;
    
    return n == -1?-1:0;
}

int main(int argc, const char * argv[]) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int send_buff_size = 1024;
    int actually_buff_size;
//    socklen_t alen = sizeof(int);
//    ssize_t send_bytes;

    if (argc != 4)
    {
        fprintf(stderr, "参数错误\n");
        exit(EXIT_FAILURE);
    }
    

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }
    
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("socket");
            continue;
        }
        
        setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &send_buff_size, sizeof(send_buff_size));
//        getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &actually_buff_size, &alen);
        
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("connect");
            continue;
        }

        break;
        
    }

    if (p == NULL)
    {
        fprintf(stderr, "failed to connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(servinfo);

//    if ((send_bytes = send(sockfd , argv[3] , strlen(argv[3]) , 0)) == -1)
//    {
//        close(sockfd);
//        perror("send");
//        exit(EXIT_FAILURE);
//    }

    int len = (int)strlen(argv[3]);
    
    if (sendall(sockfd, (char *)&argv[3], &len) == -1) {
        close(sockfd);
        fprintf(stderr, "sendall failed\n");
        exit(EXIT_FAILURE);
    }
    
    printf("send_bytes: %d\n", len);

    close(sockfd);
    
    return 0;
}
