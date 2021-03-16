//
//  main.c
//  sender-client
//
//  Created by wan zhongwen on 2021/3/11.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

int main(int argc, const char * argv[]) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv, send_bytes;

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

    if ((send_bytes = send(sockfd , argv[3] , strlen(argv[3]) , 0)) == -1)
    {
        perror("send");
        exit(EXIT_FAILURE);
    }

    printf("send_bytes: %d\n", send_bytes);

    close(sockfd);
    
    return 0;
}
