
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

void* handle_request(void*);

// pthread_mutex_t mutex;
int main(int argc, char **argv)
{
  if (argc != 2)
  {
    printf("error");
    return 1;
  }
  // pthread_mutex_init(&mutex, nullptr);

  int sock = socket(AF_INET, SOCK_STREAM, 0),
    clientSock;
  if (sock == -1)
  {
    perror("error");
    return 1;
  }

  int sockReuseOpt = 1;
  socklen_t  sockReuseOptLen = sizeof(sockReuseOpt);
  if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &sockReuseOpt, sockReuseOptLen) == -1) {
    perror("error");
    close(sock);
    return 1;
  }

  struct sockaddr_in sockAddr,
    clntAddr;
  memset(&sockAddr, 0, sizeof(sockAddr));
  sockAddr.sin_family = AF_INET;
  sockAddr.sin_port = (in_port_t)(htons(atoi(argv[1])));
  sockAddr.sin_addr.s_addr = ntohl(INADDR_ANY);

  if(bind(sock, (struct sockaddr *)(&sockAddr), sizeof(sockAddr)) == -1) {
    perror("error");
    close(sock);
    return 1;
  }

  if(listen(sock, 5) == -1) {
    perror("error");
    close(sock);
    return 1;
  }

  pthread_t thread;
  while(1) {
    socklen_t clntLen = sizeof(clntAddr);
    clientSock = accept(sock,(struct sockaddr *)(&clntAddr), &clntLen);
    if(clientSock == -1) {
      perror("error");
      continue;
    }
    if(pthread_create(&thread, NULL, handle_request, &clientSock))  {
      perror("error");
      close(clientSock);
      close(sock);
      return 1;
    }
    if(pthread_detach(thread)) {
      perror("error");
      close(clientSock);
      close(sock);
      return 1;
    }
  }
  close(sock);
  // if(pthread_mutex_destroy(&mutex)) {
  //   cout << strerror(errno) << endl;
  //   return 1;
  // }
  return 0;
}


void* handle_request(void* arg) {
  int clntSock = *(int*)arg;
  FILE *readFile = fdopen(clntSock, "r"),
    *writeFile = fdopen(dup(clntSock), "w");
  // if(pthread_mutex_lock(&mutex)) {
  //   cout << strerror(errno) << endl;
  //   return NULL;
  // }
  char readBuf[50];
  memset(readBuf, 0, 50);

  fgets(readBuf, 20, readFile);
  if(strstr(readBuf, "GET") == NULL) {
    fputs("error", writeFile);
    fflush(writeFile);
    fclose(readFile);
    fclose(writeFile);
    // pthread_mutex_unlock(&mutex);
    return 0;
    // pthread_exit(nullptr);
  }  
  
  FILE* readHtml = fopen("./public/index.html", "r");
  if(!readHtml) {
    perror("error");
    fclose(readFile);
    fclose(writeFile);
    // pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
    return NULL;
  }
  fputs("HTTP/1.0 200 OK\r\n", writeFile);
  fputs("Content-type: text/html\r\n", writeFile);
  fputs("Content-length: 5\r\n\r\n", writeFile);
  char htmlBuf[50];

  while(fgets(htmlBuf,50, readHtml) != NULL) {
    fputs(htmlBuf, writeFile);
    fflush(writeFile);
    memset(htmlBuf, 0, 50);
  }

  fflush(writeFile);
  shutdown(fileno(writeFile), SHUT_WR);
  fclose(writeFile);
  fclose(readHtml);
  fclose(readFile);
  // if(pthread_mutex_unlock(&mutex)) {
  //   cout << strerror(errno) << endl;
  //   return NULL;
  // }
  // pthread_exit(nullptr);
  return NULL;
}