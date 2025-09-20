#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>

#define CHUNK_SIZE 1000

// Usage:
// ./iPerfer -s -p <listen_port>
// ./iPerfer -c -h <server_hostname> -p <server_port> -t <time>

int main(int argc, char** argv) {
  int s, new_s;
  struct sockaddr_storage their_addr;
  socklen_t addr_len = sizeof their_addr;
  int t;
  struct addrinfo hints, *serverAddrInfo;


  char close_chunk[CHUNK_SIZE];  // CHUNK_SIZE = 1000
  char ack_chunk[CHUNK_SIZE];
  memset(close_chunk, 9, CHUNK_SIZE);
  memset(ack_chunk, 1, CHUNK_SIZE);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  s = socket(PF_INET, SOCK_STREAM, 0);
  printf("Program started\n");

    // SERVER MODE
    if (strcmp(argv[1],"-s")==0) {
      printf("Server started\n");

      // Parsing and checking arguments
      mode = 's';
      if (argc != 4 || strcmp(argv[2],"-p")!=0) {
        printf("Error: missing or extra arguments\n");
        exit(1);
      }
      char *listen_port = argv[3];
      if (atoi(listen_port) < 1024 || atoi(listen_port) > 65535) {
        printf("Error: port number must be in the range of [1024, 65535]\n");
        exit(1);
      }

      // Bind, Listen and Accept

      // Allow reuse of the port number for testing
      time_t server_start_time, server_end_time;
      int optval = 1;
      setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

      hints.ai_flags = AI_PASSIVE;
      getaddrinfo(NULL, listen_port, &hints, &serverAddrInfo);
      bind(s, serverAddrInfo->ai_addr, serverAddrInfo->ai_addrlen);
      listen(s, 1);
      new_s = accept(s, (struct sockaddr *)&their_addr, &addr_len);
      printf("connection accepted\n");
      close(s); //just one connection, we don't need to listen for others
      int kb_received = 0;
      char buff[CHUNK_SIZE];
      server_start_time = time(NULL);

      while(1){
        int received = recv(new_s, buff,CHUNK_SIZE, 0 );
        if (received==0) break; //connection closed
        if (memcmp(buff, close_chunk, CHUNK_SIZE) == 0){
          break; //FIN message
        }
        printf("Received 1 chunk\n");
        kb_received++;
      }
      server_end_time = time(NULL);
      send(new_s, ack_chunk, CHUNK_SIZE, 0);
      printf("ACK send\n");
      recv(new_s, NULL, CHUNK_SIZE, 0); //waiting the client to close first
      close(new_s);

      double elapsed_time = difftime(server_end_time, server_start_time);
      printf("Received=%d KB, ",kb_received);
      int mb_received = kb_received/1000;
      double bandwidth_mbps = mb_received/elapsed_time;
      //TODO format with 3 decimal points
      printf("Rate=%f Mbps\n",bandwidth_mbps );
  }

    // CLIENT MODE
    else if (strcmp(argv[1],"-c")==0) {
      mode = 'c';
      if (argc != 8 || strcmp(argv[2],"-h")!=0 || strcmp(argv[4],"-p")!=0 || strcmp(argv[6],"-t")!=0) {
        printf("Error: missing or extra arguments\n");
        exit(1);
      }
      char *server_hostname = argv[3];
      char *server_port = argv[5];
      if (atoi(server_port) < 1024 || atoi(server_port) > 65535) {
        printf("Error: port number must be in the range of [1024, 65535]\n");
        exit(1);
      }
      t = atoi(argv[7]);
      if (t < 1) {
        printf("Error: time argument must be greater than 0\n ");
        exit(1);
      }

      // Connect
      hints.ai_flags = 0;
      getaddrinfo(server_hostname, server_port, &hints, &serverAddrInfo);
      connect(s, serverAddrInfo->ai_addr, serverAddrInfo-> ai_addrlen);

      //timeout
      time_t client_start_time = time(NULL);
      time_t client_end_time = client_start_time + t;
      char send_chunk[CHUNK_SIZE];
      memset(send_chunk, 0, CHUNK_SIZE);

      printf("Client started sending\n");
      int sent_kb=0;
      while(client_end_time > time(NULL)){
        printf("Client send one chunk\n");
        int bytes_sent = send(s,send_chunk,CHUNK_SIZE,0);
        if (bytes_sent > 0) {
          sent_kb++;
        }
      }

      char buff[CHUNK_SIZE];
      while(1){
        recv(s,buff,CHUNK_SIZE,0);
        if (memcmp(buff, ack_chunk, CHUNK_SIZE) == 0){
          close(s);
          break;
        }
      }
      time_t client_final_time = time(NULL);
      time_t elapsed_time = client_final_time - client_start_time;

      double bandwidth_mbps = (sent_kb * 8.0) / (elapsed_time * 1000.0);
      printf("Sent=%d KB, Rate=%.3f Mbps\n", sent_kb, bandwidth_mbps);
    }
}
