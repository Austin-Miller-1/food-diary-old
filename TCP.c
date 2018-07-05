
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/tcp.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** args){
  //Fake message
  char message[100];
  snprintf(message, 100, "hello world?");

  //Initialise; make socket
  int socket;
  if( (socket = nn_socket(AF_SP, NN_REP)) == -1){perror("Could not make socket"); exit(0);}


  //Set connection
  char* transport = "tcp";
  char* address = "fd-website.herokuapp.com/api/test-session";
  char* port = "80";
  char addr[100];
  snprintf(addr, 100, "%s://%s:%s", transport, address, port);
//  printf("%s", addr);

  int endpointID;
  if( (endpointID = nn_connect(socket ,"tcp://127.0.0.1:5555")) == -1){perror("Could not connect"); exit(0);}


  //Send data
  int bytesSent;
  if( (bytesSent = nn_send(socket, message, 100, 0)) ){perror("Could not send message");}
  printf("Bytes sent: %d", bytesSent);

  //End connection
  nn_shutdown(socket , endpointID);
  nn_close(socket);
  return 1;
}
