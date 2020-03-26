/* Program : Simple HTTP Proxy Server
   Author  : Utsav Chokshi
   Version : v2.0
   Date    : 10/10/2016
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>   //For atoi
#include <netdb.h> 
#include <fcntl.h>

//Important constants
#define BACKLOG 10
#define BUFFERSIZE 4096
#define RECV_BUFFERSIZE 1000000
#define HTTP_PORT 80
#define HTTP_PORT_PROXY 8080

//Important responses
static char* bad_request_response = 
  "HTTP/1.0 400 Bad Request\n"
  "Content-type: text/html\n"
  "\n"
  "<html>\n"
  " <body>\n"
  "  <h1>Bad Request</h1>\n"
  "  <p>This server did not understand your request.</p>\n"
  " </body>\n"
  "</html>\n";

static char* bad_method_response = 
  "HTTP/1.0 501 Method Not Implemented\n"
  "Content-type: text/html\n"
  "\n"
  "<html>\n"
  " <body>\n"
  "  <h1>Method Not Implemented</h1>\n"
  "  <p>The method is not implemented by this server.</p>\n"
  " </body>\n"
  "</html>\n";

static char* valid_request_response_template = 
  "HTTP/1.0 200 OK\n"
  "Content-type: text/html\n"
  "\n"
  "<html>\n"
  " <body>\n"
  "  <h1>You have hit Utsav's Proxy Server.</h1>\n"
  "  <p>Requested url : %s</p>\n"
  " </body>\n"
  "</html>\n";

static char* error_template = "ERROR : %s\n";

void die(char* msg){	
  char error_msg[1024];
  snprintf(error_msg,sizeof(error_msg),error_template,msg);
  write(1,error_msg,strlen(error_msg));
  exit(1);
}

char* getFileName(char* url, char* dirpath){

      int n = strlen(url);
      int m = strlen(dirpath);
      char*  fileName = (char*)malloc(sizeof(char)*(m+n+1));
      int i = 0, j = 0;

      for(i=0; i<m; i++){
          fileName[i] = dirpath[i]; 
      }

      for(j=0; j<n; j++){
         if(url[j] == '/'){
            fileName[i] = '#';
         }
         else{
            fileName[i] = url[j];
         }
         i++; 
      }
      fileName[i] = '\0';

      return fileName;
}

char* readFromFile(char* fileName){

      int fd = open(fileName,O_RDONLY);
      char* read_buffer = (char*)malloc(RECV_BUFFERSIZE*sizeof(char));
      int offset = 0;

      int bytes_read  = 0;
      while((bytes_read = read(fd,read_buffer+offset,RECV_BUFFERSIZE)) > 0){
          offset += bytes_read;
      }

      read_buffer[offset] = '\0';
      return read_buffer;
}

char* connectRemoteServer(char* request, char* hostname,char* protocol,char* url){

      int sockfd;
      struct hostent *server;
      struct sockaddr_in serv_addr;
      char* recv_buffer = (char*)malloc(RECV_BUFFERSIZE*sizeof(char));
      char* send_buffer = (char*)malloc(BUFFERSIZE*sizeof(char));

      //Create new socket to connect to remote server
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      if (sockfd < 0) {
          die("error while opening socket");
      }

      //Checking whether local-site is requested ?
      char temp[100];
      int server_port;
      if(sscanf(hostname,"%[^.]s.iiit.ac.in",temp) == 0){
           server = gethostbyname(hostname);
           server_port = HTTP_PORT;
           //sprintf(send_buffer,"%s",request);
      }
      else{
          server = gethostbyname("proxy.iiit.ac.in");
          server_port = HTTP_PORT_PROXY;
      }

      //Resolve remote server's IP address
      if (server == NULL) {
          char error_msg[1024];
          char* temp = "error no  host : %s";
          snprintf(error_msg, sizeof(error_msg), temp, hostname);
          die(error_msg);
      }

      //Fill server structure
      bzero((char *) &serv_addr, sizeof(serv_addr));
      serv_addr.sin_family = AF_INET;
      bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
      serv_addr.sin_port = htons(HTTP_PORT_PROXY);
      //serv_addr.sin_port = htons(HTTP_PORT);

      //Connect with remote server
      int n = connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr));
      if(n < 0){
          die("error while connecting");
      } 

      //Sends HTTP Request
      bzero((char*)send_buffer,sizeof(send_buffer));
      sprintf(send_buffer,"GET %s %s\r\nHost: %s\r\nConnection: close\r\n\r\n",url,protocol,hostname);
      int bytes_sent = send(sockfd,send_buffer,strlen(send_buffer),0);

      if(bytes_sent < 0){
          close (sockfd);
          die("error while writing to socket"); 
      }

      //Reads HTTP Response
      int offset = 0;
      int bytes_read  = 0;
      while((bytes_read = recv(sockfd,recv_buffer+offset,RECV_BUFFERSIZE,0)) > 0){
          offset += bytes_read;
      }

      //Even last read should be successful.    
      if (bytes_read < 0) {
          close(sockfd);
          die("error while reading from socket");
      }

      recv_buffer[offset] = '\0';

      return recv_buffer;
}


// Following code handles each client connections and communicates with it
void handleClientConnection (int sockfd)
{ 
   char buffer[BUFFERSIZE];
   int bytes_read;
   int bytes_sent;

   //Waiting for client's message
   //Flusing buffer     
   bzero(buffer,BUFFERSIZE);
   
   bytes_read = read(sockfd,buffer,BUFFERSIZE-1);
 
   if (bytes_read > 0){
	 	
	 	//Received some data. NULL-terminate buffer.So it can be used as string
	 	buffer[bytes_read] = '\0';	

	 	char method[sizeof(buffer)];
	 	char url[sizeof(buffer)];
	 	char protocol[sizeof(buffer)];
	 	char hostname[sizeof(buffer)];
	 	char filepath[sizeof(buffer)];
   
	 	
	 	/*First line of HTTP request is three space seperated strings : 
	 	  1) Method : GET/POST/HEAD/CONNECT
	 	  2) URL : url requested
	 	  3) Protocol Version : HTTP/1.0 , HTTP/1.1 
	 	*/

	 	//sscanf(buffer, "%s %s %s\rHost: %s",method,url,protocol,hostname);      
	 	sscanf(buffer,"%s %s %s\n",method,url,protocol);
	 	sscanf(url,"http://%[^/]s/%s",hostname,filepath);

	 	/*For connecting to remote server, we need only three mentioned parameters.
	 	  But client may send much more information.
	 	  Currently, this information is of no use. So just read it and ignore it.
	 	*/

	 	/*According to HTTP Sepcification, Every HTTP Valid Request ends with CR\LF.*/   
	 	while (strstr (buffer, "\r\n\r\n") == NULL){
	 		bytes_read = read(sockfd,buffer,BUFFERSIZE-1);
	 	}

	 	/*Last read failed. It indicates some problem in connection. Close the connection.*/
	 	if(bytes_read == -1){
  	 		close (sockfd);
  	    die("connection terminated from client side!\n");
	 	}

	 	/*Now verify HTTP Request*/
	 	
	 	//If protocol version do not match. Report as bad request */
	 	if (strcmp (protocol, "HTTP/1.0") != 0 && strcmp (protocol, "HTTP/1.1") != 0) {
  	 		bytes_sent = send(sockfd,bad_request_response,sizeof(bad_request_response),0);
  	 		if(bytes_sent < 0){
  	 			close (sockfd);
  	 			die("error while writing to socket");  
  	 		}
	 	}

	 	//Current implementation only handles GET method. Report as bad request */
	 	else if(strcmp(method, "GET") != 0){
  	 		bytes_sent = send(sockfd,bad_method_response,sizeof(bad_method_response),0);
  	 		if(bytes_sent <  0){
  	 			close(sockfd);
  	 			die("error while writing to socket");  
  	 		}	
	 	}

	 	//Valid HTTP Request!!
	 	else{
            printf("INFO : Valid HTTP Request received.\n");
            printf("INFO : %s\n",buffer);

            char* fileName = getFileName(url,"cache/");
            printf("Filename : %s\n",fileName);
            FILE* fp; 
            fp = fopen(fileName,"r");

            char* response;
            if(fp == NULL){
                printf("Cache Miss !! \n");
                response  = connectRemoteServer(buffer,hostname,protocol,url);
                fp = fopen(fileName,"w");
                bytes_sent = fputs(response,fp);
                if(bytes_sent == EOF){
                    die("error while writing to cache file");
                }
                fclose(fp);    
            }
            else{
                fclose(fp);
                printf("Cache Hit !! \n");
                response = readFromFile(fileName);
            }

            // char* msg1 = "INFO : No cached response found!\n"; 
            // write(1,msg1,strlen(msg1));
            //char* msg2 = "INFO :Cached response found!\n"; 
            //write(1,msg2,strlen(msg2));

            bytes_sent = send(sockfd, response, strlen(response), 0);
    		    if (bytes_sent < 0){
    		  	     close(sockfd);
     			       die("error while writing to socket");   
     		    }
	 	    }
	 } 
	 else if(bytes_read == 0){
	      die("connection terminated from client side!\n");
	 }
	 else{
	 	    die("error reading from socket\n");
	 }
}

void init(int port_number){

	   int sockfd, newsockfd, pid;
     struct sockaddr_in serv_addr, cli_addr;

     // Creating socket 
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0){
        die("ERROR in opening socket");
     }

     // Filling server structure
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(port_number); 

     //binding socket to sever structure
     int n = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
     if(n<0){
        die("ERROR in binding socket");
     }

     //Making socket to listen to incoming connections
     listen(sockfd,BACKLOG);
     socklen_t clilen = sizeof(cli_addr);

     printf("Proxy server started at 127.0.0.1 : %d\n",port_number); 

     while (1) {

      	 // Acceppting an incoming connection and filling client structure
         newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0){
            die("ERROR on accepting");
         }

         pid = fork();
         if (pid < 0){
            die("ERROR on forking");
         }

         // Creating child process for each client connection    
         if (pid == 0)  {
             close(sockfd);
             handleClientConnection(newsockfd);
             exit(0);
         }
         // Parent process will be actively listening to incoming connection
         else{
           close(newsockfd); 
         } 
     }
     close(sockfd); 
}


int main(int argc, char* const argv[]){

  printf("\n*****WELCOME TO UTSAV'S PROXY SERVER*****\n\n");

	if(argc <= 1){
		char* err_msg = "Incorrect Usage.\nUsage : ./proxy port_number";
		die(err_msg);
		return 0;
	}

	int port_number = atoi(argv[1]);
	init(port_number);

	return 0;
}
