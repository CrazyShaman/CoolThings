//
//  serverFromClass.c
//  
//
//  Created by Christopher Tobias on 10/24/13.
//
//

#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>


int main(){
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    //what we want to talk to
    addr.sin_family = AF_INET;
    //which port we wnat to talk to
    addr.sin_port = htons(8080);
    //who we want to talk to
    addr.sin_addr.s_addr = INADDR_ANY;
    int on = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    
    if(bind(server_sock,(struct sockaddr*)&addr, sizeof(addr))){
        perror("Couldnt connect!!!!!");
        exit(1);
    }
    
    
    int err = listen(server_sock, 10); //will hold onto 10 requests for you until you come back
    //int client_sock = accept(server_sock,0,0);
    
    
    //char *req = "GET / HTTP/1.0\r\nhost:www.google.com\r\n\r\n";
    //send(s, req, strlen(req));
    char buf[1000];
    fd_set readset, rdyset;
    FD_ZERO(&readset); //set the set to NULL
    //FD_SET( client_sock, &readset);  //add client scocket to set
    FD_SET(0,&readset); //add the stdin to readset
    FD_SET(server_sock, &readset); // the server_sock to the readset
    rdyset = readset;
    
    int max_client = 0;
    // select(max # of clients, set of file descriptors that you're reading from, set you're writing to, set that tells you any errors
    while( select(32, &rdyset,0,0,0) ){
        //if theres something on standard in then read it and send it out to all the clients
        if(FD_ISSET(0,&rdyset) == 1){
            char stdin_buf[1024];
            int len = read(0,stdin_buf, 1024);
            stdin_buf[len] = 0;
            // sending to all the clients
            for(int c = 4; c<=max_client; c++){
                if(FD_ISSET(c, &readset)){  // FD_ISSET(file descriptor, 
                    write(c,stdin_buf,len);
                }
            }
        }
        //otherwise check if any of the clients are set and read from them
        for(int c = 4; c<=max_client; c++){
            if(FD_ISSET(c, &rdyset)){
                int len = recv(c, buf, 1000,0);
                //if a client is set but they send nothing that means they closed the connection
                //so you clear it from the sead set and close it
                if (len == 0 ){
                    FD_CLR(c, &readset);
                    close(c);
                }
                write(1,buf,strlen(buf));
                memset(buf, 0, 1000);
            }
        }
        //if serversocket is set then that means you gained a new client
        //therefore add them to the list of clients
        if(FD_ISSET(server_sock, &rdyset)){
            int newclient=accept(server_sock, 0,0);
            printf("got a new client %d\n", newclient);
            FD_SET(newclient, &readset);
            if(newclient>max_client) max_client = newclient;
        }
        //select changes the readset so if we dont do this, the next time
        //around well only be listeing for whatever we got before
        rdyset = readset;
    }
}