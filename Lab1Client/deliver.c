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
#include <time.h>
#include <math.h>

/*
 * 
 */
/*
 REFERENCE:
 Code here taken from Beej's Guide to Network Programming
 */
#define MAXBUFLEN 1000
void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

}

struct packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char filedata[1000];

};




int main(int argc, char** argv) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    char* message = "ftp";
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    char* file_name = "sample1.txt";
    
    
    
    if(!(access(file_name,F_OK) == 0)) return 0;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    
    const char* host = argv[1];
    const char* SERVERPORT = argv[2];
    
    if((rv = getaddrinfo(host,SERVERPORT, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    for(p = servinfo; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1){
            perror("client: socket");
            continue;
        }
        break;
    }
    
    if(p == NULL){
        fprintf(stderr, "client: failed to create socket \n");
        return 2;
    }
    
    
    // code to start measuring time
    clock_t start,end;
    double rt_time;
    //start = clock();
    
    
    if((numbytes = sendto(sockfd,message,strlen(message),0,p->ai_addr,p->ai_addrlen)) == -1){
        perror("client: sendto");
        exit(1);
    }
    printf("client: sent %d bytes to %s\n",numbytes,host);
    
   
    
    
    printf("client: waiting to recvfrom...\n");
    addr_len = sizeof their_addr;
    
    if((numbytes = recvfrom(sockfd,buf, MAXBUFLEN-1,0,(struct sockaddr *)&their_addr,&addr_len)) == -1){
        perror("recvfrom");
        exit(1);
    }
    
    printf("client: got a packet from %s\n", inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s,sizeof s));
    printf("client: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("client: packet contains \"%s\"\n",buf);
    //
    char *rec = buf;
    const char* expected_response = "yes";
    if(!strcmp(expected_response,rec)) printf("A file transfer can start\n");
    //calculating the round trip time
    //end = clock();
    //rt_time = ((double)end - start) / CLOCKS_PER_SEC;
    //printf("Total Round Trip Time: %f s\n", rt_time) / CLOCKS_PER_SEC;
    
    //
    /*
    
    FILE *fileptr;
    char* file_buf;
    long filelen;
    fileptr = fopen(file_name,"rb");
    fseek(fileptr,0,SEEK_END);
    filelen = ftell(fileptr);
    rewind(fileptr);
    
    file_buf = (char*)malloc(filelen * sizeof(char));
    fread(file_buf,filelen,1,fileptr);
    fclose(fileptr);
    
   
    
    //converting the data into packets
    int remaining_data_count = filelen;
    double total_fragments = filelen / 1000.0;
    int frag_count = total_fragments;
    if(frag_count < total_fragments) frag_count += 1;
    int buf_index = 0;
    struct packet* packets = (struct packet*)malloc(frag_count * sizeof(struct packet));
    
    for(int i = 0; i < frag_count; i++){
        packets[i].total_frag = frag_count;
        packets[i].frag_no = i + 1;
        packets[i].size = remaining_data_count < 1000 ? remaining_data_count: 1000;
        remaining_data_count -= packets[i].size;
        
        for(int j = 0; j < packets[j].size; j++){
            packets[i].filedata[j] = file_buf[buf_index];
            buf_index += 1;
        }
        
    
    }
    */
    //
    freeaddrinfo(servinfo);
    //free(file_buf);
    //free(packets);
    
    close(sockfd);
    return (EXIT_SUCCESS);
}

/*
 * TODO
 * conver the packets into strings
 * on client/server side do loop with recvfrom and sendto and send packets and acknowledgement
command line arguments
makefile
s3,s4 
 
 */