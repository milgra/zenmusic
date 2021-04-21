#ifndef remote_h
#define remote_h

#include "mtchannel.c"

void remote_listen(ch_t* channel);

#endif

#if __INCLUDE_LEVEL__ == 0

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 23723
#define MAXLINE 10

int remote_alive = 0;

void* remote_listen_ins(void* p)
{
  ch_t* channel = (ch_t*)p;

  char               buffer[MAXLINE];
  char*              hello = "Hello from server";
  struct sockaddr_in servaddr, cliaddr;

  // Creating socket file descriptor
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) printf("socket creation failed");

  // zero out addresses
  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // Filling server information
  servaddr.sin_family      = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port        = htons(PORT);

  // Bind the socket with the server address

  int res = bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr));
  if (res < 0) printf("socket bind failed");

  socklen_t len = sizeof(cliaddr); //len is value/resuslt

  while (remote_alive)
  {
    // blocking listen
    int num = recvfrom(sockfd,
                       (char*)buffer,
                       MAXLINE,
                       MSG_WAITALL,
                       (struct sockaddr*)&cliaddr,
                       &len);

    buffer[num] = '\0';

    ch_send(channel, buffer);

    printf("data received : %s\n", buffer);
  }

  return NULL;
}

void remote_listen(ch_t* channel)
{
  pthread_t threadId;

  if (remote_alive == 0)
  {
    remote_alive = 1;

    int err = pthread_create(&threadId, NULL, &remote_listen_ins, channel);

    if (err) printf("Thread creation failed : %s", strerror(err));
  }
}

#endif
