#include "tcp_server.h"
#include <stdio.h>
#include <string.h>

#ifdef USE_WINDOWS_OS

#include <winsock2.h>
#endif


#ifdef __linux__

#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <stdbool.h>

#include "csm_array.h"
#include "transports.h"

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#endif

#define CRLF		"\r\n"
#define MAX_CLIENTS 10

#ifndef TCP_BUF_SIZE
#define TCP_BUF_SIZE    2048
#endif


typedef struct
{
   SOCKET sock;
   bool connected; // 0 = not connected
   int8_t channel_id;

   uint8_t buffer[TCP_BUF_SIZE];
   
} peer;

/* an array for all clients */
static peer peers[MAX_CLIENTS];

static void init(void)
{
#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if(err < 0)
   {
      puts("WSAStartup failed !");
      exit(EXIT_FAILURE);
   }
#endif
}

static void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}



/*
static void broadcast(peer *clients, peer sender, int actual, const char *buffer, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for(i = 0; i < actual; i++)
   {
      // we don't send message to the sender
      if(sender.sock != clients[i].sock)
      {
         if(from_server == 0)
         {
         //   strncpy(message, sender.name, BUF_SIZE - 1);
        //    strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
       //  strncat(message, buffer, sizeof message - strlen(message) - 1);
      //   write_peer(clients[i].sock, message);
      }
   }
}
*/

static int init_connection(int tcp_port)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(tcp_port);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}


static void write_peer(SOCKET sock, const char *buffer, size_t size)
{
   if(send(sock, buffer, size, 0) < 0)
   {
      perror("send()");
    //  exit(errno);
   }
}

static int read_peer(SOCKET sock, char *buffer, int max_size)
{
   int n = 0;

   if((n = recv(sock, buffer, max_size, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disconnect the client */
      n = 0;
   }
   return n;
}

void tcp_server_send(int8_t channel_id, const char *buffer, size_t size)
{
   // look for peer by channel id
   for (int i = 0; i < MAX_CLIENTS; i++)
   {
       if (peers[i].connected && peers[i].channel_id == channel_id)
       {
           write_peer(peers[i].sock, buffer, size);
           break;
       }
   }
}

static void app(data_handler data_func, connection_handler connection_func, disconnection_handler disconnection_func, int tcp_port)
{
   SOCKET sock = init_connection(tcp_port);


   unsigned int max = sock;

   // Init properly
   for (int i = 0; i < MAX_CLIENTS; i++)
   {
      peers[i].connected = false;
      peers[i].channel_id = -1;
      peers[i].sock = INVALID_SOCKET;
   }

   fd_set working_set;
   fd_set master_set;

   FD_ZERO(&master_set);

   /* add the connection socket */
   FD_SET(sock, &master_set);

   printf("[TCP Server] TCP Server started on TCP port: %d\r\n", tcp_port);

   while(1)
   {
        // update max
        max = sock;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
           if (peers[i].connected)
           {
               /* what is the new maximum fd ? */
               max = peers[i].sock > max ? peers[i].sock : max;
           }
        }

        memcpy(&working_set, &master_set, sizeof(master_set));

        if(select(max + 1, &working_set, NULL, NULL, NULL) == -1)
        {
            perror("select()");
            exit(errno);
        }


        if(FD_ISSET(sock, &working_set))
        {
            /* new client */
            SOCKADDR_IN csin = { 0 };
            int sinsize = sizeof csin;
            SOCKET csock = accept(sock, (SOCKADDR *)&csin, &sinsize);

            if(csock == INVALID_SOCKET)
            {
                perror("accept()");
                continue;
            }

            printf("[TCP] Accept socket %d\r\n", csock);

            // Grant access to the application layer
            int8_t channel_id = connection_func();
            if (channel_id >= 0)
            {
                FD_SET(csock, &master_set);
                
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (!peers[i].connected)
                    {
                        peers[i].sock = csock;
                        peers[i].connected = true;
                        peers[i].channel_id = channel_id;
                        puts("[TCP server] New connection!");
                        break;
                    }
                }
            }
            else
            {
                // Reject connection
                end_connection(csock);
                puts("[TCP server] New connection rejected");
            }
        }

        for(int i = 0; i < MAX_CLIENTS; i++)
        {
            if (peers[i].connected)
            {
                /* a client is talking */
                if(FD_ISSET(peers[i].sock, &working_set))
                {
                   int size = read_peer(peers[i].sock, &peers[i].buffer[0], sizeof(peers[i].buffer));
                   /* client disconnected */
                   if(size == 0)
                   {
                     FD_CLR(peers[i].sock, &master_set);
                     end_connection(peers[i].sock);

                     puts("Client disconnected !");
                     disconnection_func(peers[i].channel_id);

                     // Make sure structure elements are cleared
                     peers[i].sock = INVALID_SOCKET;
                     peers[i].connected = false;
                     peers[i].channel_id = -1;
                   }
                   else
                   {
                       puts("[TCP server] New data received!");
                       if (data_func != NULL)
                       {
                           int ret = data_func(peers[i].channel_id, &peers[i].buffer[0], size, sizeof(peers[i].buffer));
                           if (ret > 0)
                           {
                              printf("[TCP] Send to seocket %d\r\n", peers[i].sock);
                              write_peer(peers[i].sock, &peers[i].buffer[0], ret);
                           }
                       }

                       //broadcast(peers, client, actual, buffer, 0);
                   }
                } 
            }
        }
   }

   // Clear peers
   for(int i = 0; i < MAX_CLIENTS; i++)
   {
       if (peers[i].connected)
       {
            closesocket(peers[i].sock);
       }
   }
   // End server
   end_connection(sock);
}


int tcp_server_init(data_handler data_func, connection_handler conn_func, disconnection_handler discon_func, int tcp_port)
{
   init();

   app(data_func, conn_func, discon_func, tcp_port);

   end();

   return EXIT_SUCCESS;
}
