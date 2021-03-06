/* Standard C Headers */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/* Thread pool header file */

#include "threadpool.h"

char response[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html><html><head><title>Mon serveur web</title>"
"<body bgcolor=lightgreen><h1>Hello from my own web server !</h1></body></html>\r\n";

/******************************************************************************/
/* processing the request                                                     */
/******************************************************************************/
void process_request (void *requete ) {
  int *client_fd = (int *) requete;
  int bufsize = 1024;
  char *buffer = malloc(bufsize);

  if (*client_fd == -1)
    perror("accept");

  recv(*client_fd, buffer, bufsize, 0);
  printf ("%s\n", buffer);
  send(*client_fd, response, sizeof(response), 0);
  close(*client_fd);
  free (buffer);
}

/******************************************************************************/
/* dispatching the requests                                                   */
/******************************************************************************/
void dispatch_requests (pthread_pool_t *thread_pool, void *requete) {
  pthread_pool_exec(thread_pool,process_request,requete);
}

/******************************************************************************/
/* function main                                                              */
/******************************************************************************/

int main(int argc, char **argv) {
  
  printf  ("\x1B[32m %s \x1B[0m%s", "[MAIN]", "Start processing\n");

  /*--------------------------------------------------------------------------*/
  /* Initialize a thread pool of size 10                                       */
  /*--------------------------------------------------------------------------*/
  pthread_pool_t *thread_pool = pthread_pool_init (10);  
  if (thread_pool == NULL)
  perror ("ERROR: failed to create the thread pool, ");
  printf ("\x1B[32m %s \x1B[0m%s", "[MAIN]", "Start processing\n");

    /* Leave some time for the threadpool to finish its initialization */
  sleep (1);


  /*--------------------------------------------------------------------------*/
  /* Initialize web server                                                    */
  /*--------------------------------------------------------------------------*/
  int one = 1, client_fd;
  struct sockaddr_in svr_addr, from;
  socklen_t sin_len = sizeof(from);

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    perror("socket");

  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

  int port = 1234;
  svr_addr.sin_family = AF_INET;
  svr_addr.sin_addr.s_addr = INADDR_ANY;
  svr_addr.sin_port = htons(port);

  if (bind(sock, (struct sockaddr *) &svr_addr, sizeof(svr_addr)) == -1) {
    close(sock);
    perror("bind");
  }

  listen(sock, 5);

  /*--------------------------------------------------------------------------*/
  /* Process incoming requests                                                */
  /*--------------------------------------------------------------------------*/
  while (1) {
    client_fd = accept(sock, (struct sockaddr *) &from, &sin_len);
    dispatch_requests(thread_pool,(void *)& client_fd);
  }

  return EXIT_SUCCESS;
}
