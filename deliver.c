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
 *////

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
#define MAXBUFLEN 1050
#define MAXLEN 1050
void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

}

//structure to hold packet data
struct packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char filedata[1000];

};

//returns the number of digits in an integer, helps with serialize
int digits (int n){
    int result = 0;
    while(n != 0){
        result += 1;
        n = n /10;
    }
    return result;
    
}


int current_packet_size;//holds the size of the next packet to be sent, updated in serialize

//function that turns a packet into a string ready to be sent, and updates the current packet size
//note that the += digits and then -= 1 in the while loops is because the integer digits are taken from
//least significant to most, so they must be written into the string backwards
//also note 58 is ascii for colon
char* serialize(struct packet pack){
    char result[1050];
    char col = ':';
    int buf_index = 0;
    int total_frag = pack.total_frag;
    int frag_no = pack.frag_no;
    int size = pack.size;
    int file_size = pack.size;
    char* filename = pack.filename;
    
    buf_index += digits(total_frag) - 1;
    while(total_frag != 0){
        int digit = total_frag%10;
        char dig = digit + '0';
        result[buf_index] = dig;
        total_frag = total_frag / 10;
        buf_index -= 1;
    }
    buf_index += digits(pack.total_frag) + 1;
    result[buf_index] = col;
    buf_index += 1;
    
    buf_index += digits(frag_no) - 1;
    while(frag_no != 0){
        int digit = frag_no%10;
        char dig = digit + '0';
        result[buf_index] = dig;
        frag_no = frag_no / 10;
        buf_index -= 1;
    }
    buf_index += digits(pack.frag_no) + 1;
    result[buf_index] = col;
    buf_index += 1;
    
    
    buf_index += digits(size) - 1;
    while(size != 0){
        int digit = size%10;
        char dig = digit + '0';
        result[buf_index] = dig;
        size = size / 10;
        buf_index -= 1;
    }
    buf_index += digits(pack.size) + 1;
    result[buf_index] = col;
    buf_index += 1;
    
    for(int j = 0; j < strlen(filename); j++){
        char letter = filename[j];
        result[buf_index] = letter;
        buf_index += 1;
    }
    result[buf_index] = '\0';
    buf_index += 1;
    result[buf_index] = col;
    buf_index += 1;
    
    for(int i = 0; i < file_size; i++){
        char letter = pack.filedata[i];
        result[buf_index] = letter;
        buf_index += 1;
    }
    current_packet_size = buf_index;
    
    char* res = result;
    
    return res;
}


int main(int argc, char** argv) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    char* message;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    char* file_name;
    char in_1[MAXLEN], in_2[MAXLEN];
    scanf("%s %s", &in_1, &in_2);
    message = "ftp";
    file_name = in_2;
    
   
     
    //start up code to send the ftp message
    if(!(access(file_name,F_OK) == 0)) return 0;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    
    const char* host = "ug142.eecg.utoronto.ca";
    const char* SERVERPORT = "51001";
    
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
    start = clock();
    
    
    if((numbytes = sendto(sockfd,message,strlen(message),0,p->ai_addr,p->ai_addrlen)) == -1){
        perror("client: sendto");
        exit(1);
    }
    
    addr_len = sizeof their_addr;
    
    if((numbytes = recvfrom(sockfd,buf, MAXBUFLEN-1,0,(struct sockaddr *)&their_addr,&addr_len)) == -1){
        perror("recvfrom");
        exit(1);
    }
    
    printf("A file transfer can start\n");
    //calculating the round trip time
    end = clock();
    rt_time = ((double)end - start) / CLOCKS_PER_SEC;
    printf("Total Round Trip Time: %f s\n", rt_time) / CLOCKS_PER_SEC;
 
    
    //reading the file data into a buffer
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
        packets[i].filename = file_name;
        packets[i].total_frag = frag_count;
        packets[i].frag_no = i + 1;
        packets[i].size = remaining_data_count < 1000 ? remaining_data_count: 1000;
        remaining_data_count -= packets[i].size;
        
        for(int j = 0; j < packets[i].size; j++){
            packets[i].filedata[j] = file_buf[buf_index];
            buf_index += 1;
        }
        
    }

    
    //SEND LOOP
    for(int i = 0; i < frag_count; i++){
        message = serialize(packets[i]);
        
    
        if((numbytes = sendto(sockfd,message,current_packet_size,0,p->ai_addr,p->ai_addrlen)) == -1){
            perror("client: sendto");
            exit(1);
        }
        addr_len = sizeof their_addr;

        if((numbytes = recvfrom(sockfd,buf, MAXBUFLEN-1,0,(struct sockaddr *)&their_addr,&addr_len)) == -1){
            perror("recvfrom");
            exit(1);
        }

      
    }
    
    //clean up code
    freeaddrinfo(servinfo);
    free(file_buf);
    free(packets);
    
    close(sockfd);
    return (EXIT_SUCCESS);
}

