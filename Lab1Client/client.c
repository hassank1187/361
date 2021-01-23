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
#define SERVERPORT "50000"
#define CLIENTPORT "51000"
#define MAXBUFLEN 1000
void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

}

int main(int argc, char** argv) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    char* message = "ftp";
    
    //
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    //
    
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
    
    //Code to setup binding
    struct addrinfo *clientinfo;
    if ((rv = getaddrinfo(NULL, CLIENTPORT, &hints, &clientinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    if (bind(sockfd, clientinfo->ai_addr, clientinfo->ai_addrlen) == -1){
            close(sockfd);
            perror("listener: bind");
            
        }
    
    //
    
    if((numbytes = sendto(sockfd,message,strlen(message),0,p->ai_addr,p->ai_addrlen)) == -1){
        perror("talker: sendto");
        exit(1);
    }
    printf("talker: sent %d bytes to %s\n",numbytes,host);
    
    //Code to start receiving
    if(clientinfo == NULL){
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }
    
    
    printf("listener: waiting to recvfrom...\n");
    addr_len = sizeof their_addr;
    
    if((numbytes = recvfrom(sockfd,buf, MAXBUFLEN-1,0,(struct sockaddr *)&their_addr,&addr_len)) == -1){
        perror("recvfrom");
        exit(1);
    }
    
    printf("listener: got a packet from %s\n", inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s,sizeof s));
    printf("listener: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("listener: packet contains \"%s\"\n",buf);
    
    
    //
    freeaddrinfo(clientinfo);
    freeaddrinfo(servinfo);
    
    close(sockfd);
    return (EXIT_SUCCESS);
}

