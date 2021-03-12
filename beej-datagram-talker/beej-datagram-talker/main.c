//
//  main.c
//  beej-datagram-talker
//
//  Created by wan zhongwen on 2021/3/11.
//

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define SERVERPORT "4950"    // the port users will be connecting to

int main(int argc, const char * argv[]) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    ssize_t numbytes;
    
    if (argc != 3) {
        fprintf(stderr, "usage: talker hostname message\n");
        exit(EXIT_FAILURE);
    }
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    
    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }
    
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        exit(EXIT_FAILURE);
    }
    
    if ((numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(EXIT_FAILURE);
    }
    
    freeaddrinfo(servinfo);
    
    printf("talker: sent %zd bytes to %s\n", numbytes, argv[1]);
    close(sockfd);
    
    return EXIT_SUCCESS;
}
