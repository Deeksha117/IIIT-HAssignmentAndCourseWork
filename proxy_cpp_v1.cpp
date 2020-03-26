/* Program : Simple HTTP Proxy Server
   Author  : Utsav Chokshi
   Version : v1.0
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
#include <string>
#include <utility>   
#include <map>   
#include <iostream>

//Important constants
#define BACKLOG 10
#define BUFFERSIZE 4096
#define RECV_BUFFERSIZE 1000000
#define HTTP_PORT 8080

using namespace std;

map<string,pair<string,int> > cache;

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

void connectRemoteServer(char* request, char* hostname){

  int sockfd;
  struct hostent *server;
  struct sockaddr_in serv_addr;
  char recv_buffer[RECV_BUFFERSIZE];

	  //Create new socket to connect to remote server
	  sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        die("error while opening socket");
    }

    //Resolve remote server's IP address
    //server = gethostbyname(hostname);
    server = gethostbyname("proxy.iiit.ac.in");
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
    serv_addr.sin_port = htons(HTTP_PORT);

    //Connect with remote server
    int n = connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(n < 0){
        die("error while connecting");
    } 

    //Sends HTTP Request
    int bytes_sent = write(sockfd,request,strlen(request));

    if(bytes_sent < 0){
    	close (sockfd);
	 	  die("error while writing to socket"); 
    }

    //Reads HTTP Response
    int offset = 0;
    int bytes_read  = 0;
    while((bytes_read = read(sockfd,recv_buffer+offset,RECV_BUFFERSIZE)) > 0){
    	offset += bytes_read;
    }
	
	  //Even last read should be successful.    
    if (bytes_read < 0) {
     	close(sockfd);
        die("error while reading from socket");
    }

    string key(request,strlen(request));
    string response(recv_buffer,offset);

    //Cache the response against request
    cache[key] = make_pair(response,offset);
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

	 	printf("INFO : HTTP Request received.\n");
	 	printf("INFO : %s\n",buffer);

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
	 	  sscanf(buffer, "%s %s %s\nHost: %s",method,url,protocol,hostname);      

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
	 		bytes_sent = write(sockfd,bad_request_response,sizeof(bad_request_response));
	 		if(bytes_sent < 0){
	 			close (sockfd);
	 			die("error while writing to socket");  
	 		}
	 	}

	 	//Current implementation only handles GET method. Report as bad request */
	 	else if(strcmp(method, "GET") != 0 && strcmp(method, "CONNECT") != 0){
	 		bytes_sent = write(sockfd,bad_method_response,sizeof(bad_method_response));
	 		if(bytes_sent <  0){
	 			close(sockfd);
	 			die("error while writing to socket");  
	 		}	
	 	}

	 	//Valid HTTP Request!!
	 	else{
          
          string key(buffer,strlen(buffer));

          if(cache.find(key) == cache.end()){
             connectRemoteServer(buffer,hostname);
             char* msg1 = "INFO : No cached response found!\n"; 
             write(1,msg1,strlen(msg1));
          }
          else{
             char* msg2 = "INFO :Cached response found!\n"; 
             write(1,msg2,strlen(msg2));
          }

          const char* response = cache[key].first.c_str(); 
          bytes_sent = write (sockfd, response, cache[key].second);
		      if (bytes_sent < 0){
		  	     close(sockfd);
 			       die("error while writing to socket");   
 		      }
            //Default response
          //   char response[1024];
          //   snprintf (response, sizeof (response), valid_request_response_template, url);
          //   bytes_sent = write (sockfd, response, strlen (response));
    		    // if (bytes_sent < 0){
    		  	 //   close(sockfd);
     			  //    die("error while writing to socket");   
     		   //  }
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

	if(argc <= 1){
		char* err_msg = "Incorrect Usage.\nUsage : ./proxy port_number";
		die(err_msg);
		return 0;
	}

	int port_number = atoi(argv[1]);
	init(port_number);

	return 0;
}
