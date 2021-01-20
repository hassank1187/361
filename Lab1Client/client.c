/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   client.c
 * Author: khanha75
 *
 * Created on January 20, 2021, 1:02 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
/*
 * 
 */
/*
 REFERENCE:
 Code here taken from Beej's Guide to Network Programming
 */
#define SERVERPORT "4950"
int main(int argc, char** argv) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    char* message = "test";
   
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    
    const char* host = "ug141.eecg.utoronto.ca";
    
    if((rv = getaddrinfo(host,SERVERPORT, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    for(p = servinfo; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1){
            perror("talker: socket");
            continue;
        }
        break;
    }
    
    if(p == NULL){
        fprintf(stderr, "talker: failed to create socket \n");
        return 2;
    }
    
    if((numbytes = sendto(sockfd,message,strlen(message),0,p->ai_addr,p->ai_addrlen)) == -1){
        perror("talker: sendto");
        exit(1);
    }
    
    freeaddrinfo(servinfo);
    printf("talker: sent %d bytes to %s\n",numbytes,host);
    close(sockfd);
    return (EXIT_SUCCESS);
}

