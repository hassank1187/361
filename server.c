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
#include <math.h>
/*
 * 
 */
#define MAXBUFLEN 1050
/*
 REFERENCE:
 Code here taken from Beej's Guide to Network Programming
 */
struct packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char filedata[1000];

};

char name[30] = {0}; //holds the file name, updated in deserialze

//function that parses the received string and constructs the packet based off of it
//note that 58 is ascii for a colon
struct packet deserialize(char message[], int numbytes){
    struct packet result;
    int buf_index = 0;
    int total_frag = 0;
    int size = 0;
    int frag_no = 0;
    while(message[buf_index] != 58){
        char digit = message[buf_index];
        int dig = digit - '0';
        total_frag = total_frag*10 + dig;
        buf_index += 1;
    }
    result.total_frag = total_frag;
    buf_index += 1;
    while(message[buf_index] != 58){
        char digit = message[buf_index];
        int dig = digit - '0';
        frag_no = frag_no*10 + dig;
        buf_index += 1;
    }
    result.frag_no = frag_no;
    buf_index += 1;
    while(message[buf_index] != 58){
        char digit = message[buf_index];
        int dig = digit - '0';
        size = size*10 + dig;
        buf_index += 1;
    }
    result.size = size;
    buf_index += 1;
        
    int j = 0;
    while(message[buf_index] != 58){
        name[j] = message[buf_index];
        j++;
        buf_index += 1;
    }
    result.filename = name;
    buf_index += 1;
    
    for(int i = 0; i < size; i++){
        result.filedata[i] = message[buf_index];
        buf_index += 1;
    }
    
    
    
    return result;

}

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
    
    //start up code to receive the ftp message and respond yes
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    const char* SERVERPORT = argv[1];
   
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
    
    
    addr_len = sizeof their_addr;
    
    if((numbytes = recvfrom(sockfd,buf, MAXBUFLEN-1,0,(struct sockaddr *)&their_addr,&addr_len)) == -1){
        perror("recvfrom");
        exit(1);
    }

    //code for the response
    char* client_addr = s;
    char* response = "yes";

    int numbytes_c;

    size_t length = sizeof their_addr;
    if((numbytes_c = sendto(sockfd,response,strlen(response),0,(struct sockaddr *)&their_addr, length)) == -1){
        perror("server: sendto");
        exit(1);
    }
    
    
    FILE* fileptr;
    
    char* ack = "ACK";
    int prev_frag = -1;
    //RECEPTION LOOP
    while(1){
      
    addr_len = sizeof their_addr;
    
    if((numbytes = recvfrom(sockfd,buf, MAXBUFLEN,0,(struct sockaddr *)&their_addr,&addr_len)) == -1){
        perror("recvfrom");
        exit(1);
    }
    

    char* rec = buf;

    struct packet pack = deserialize(buf,numbytes);
    int frag_no = pack.frag_no;
    int total_frag = pack.total_frag;
    
    if(frag_no == prev_frag) continue;
    
    int file_size = pack.size;
    if(frag_no == 1){
        fileptr = fopen(pack.filename,"wb");
        
    }
    fwrite(&pack.filedata,pack.size,1,fileptr);

  
    
    //Code for the response
    char* client_addr = s;
    int numbytes_c;
   
    
    size_t length = sizeof their_addr;
    if((numbytes_c = sendto(sockfd,ack,strlen(ack),0,(struct sockaddr *)&their_addr, length)) == -1){
        perror("server: sendto");
        exit(1);
    }
    
    //stop when you receive the final fragment
    prev_frag = frag_no;
    if(frag_no == total_frag) break;
    }
    
    //cleanup code
    freeaddrinfo(servinfo);
    fclose(fileptr);
    close(sockfd);
    
    return (EXIT_SUCCESS);
}
