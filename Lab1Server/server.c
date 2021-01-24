/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   server.c
 * Author: khanha75
 *
 * Created on January 20, 2021, 12:17 PM
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
#define SERVERPORT "50000"  // the port users will be connecting to
#define MAXBUFLEN 1000
#define CLIENTPORT "51000"
/*
 REFERENCE:
 Code here taken from Beej's Guide to Network Programming
 */
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
    struct sockaddr_in their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    
    if ((rv = getaddrinfo(NULL, SERVERPORT, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    //loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next){
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1){
            perror("server: socket");
            continue;
        }
        
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);
            perror("server: bind");
            continue;
        }
        
        break;
    }
    
    if(p == NULL){
        fprintf(stderr, "server: failed to bind socket\n");
        return 2;
    }
    
    
    printf("server: waiting to recvfrom...\n");
    addr_len = sizeof their_addr;
    
    if((numbytes = recvfrom(sockfd,buf, MAXBUFLEN-1,0,(struct sockaddr *)&their_addr,&addr_len)) == -1){
        perror("recvfrom");
        exit(1);
    }
    
    //printf("server: got a packet from %s\n", inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s,sizeof s));
    printf("server: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("server: packet contains \"%s\"\n",buf);
    
    //Code for the response
    char* client_addr = s;
    char* response;
    char* rec = buf;
    const char* expected_response = "ftp";
    if(!strcmp(expected_response,rec)){
        response = "yes";
    }
    else{
        response = "no";
    }
    
    
    
    int numbytes_c;
   
    
    
    size_t length = sizeof their_addr;
    if((numbytes_c = sendto(sockfd,response,strlen(response),0,(struct sockaddr *)&their_addr, length)) == -1){
        perror("server: sendto");
        exit(1);
    }
    
    printf("server: sent %d bytes to %s\n",numbytes_c,client_addr);
    
    
    freeaddrinfo(servinfo);
    
    close(sockfd);
    
    return (EXIT_SUCCESS);
}

