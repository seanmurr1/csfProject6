/*
 * CSF Assignment 6 
 * Shelby Coe : scoe4
 * Sean Murray : smurra42
 */

// Includes:
#include <stdio.h>      /* for snprintf */
#include "csapp.h"
#include "calc.h"

/* buffer size for reading lines of input from user */
#define LINEBUF_SIZE 1024

// Struct encapsulating data needed for a client connection
struct Client_conn {
	// Client socket file descriptor
	int client_fd;
	// Shared Calc object
	struct Calc *calc;
	// Pointer to server status
	int *status;
}; 

// Thread start function
void *thread_start(void *vargp);

// Function to chat with client
int chat_with_client(struct Calc *calc, int client_fd);

// Main function
int main(int argc, char **argv) {
	// Checking for correct number of CL args
	if (argc != 2) {
		return -1;
  	}

	// Attempting to open server socket
  	int server_fd = Open_listenfd(argv[1]);
	// Checking for success
  	if (server_fd < 0) {
   		return -1;
  	}
	
	// Creating calc struct for server
  	struct Calc *calc = calc_create();

	// Thread ID
	pthread_t tid;

	// Loop to accept client requests
  	int keep_going = 1;
  	while (keep_going) {
		struct Client_conn *conn = malloc(sizeof(struct Client_conn));
		if (!conn) {
			// TODO error check instead? or exit?
			continue;
		}
		// Link to shared calculator
		conn->calc = calc;
		// Connect to server status
		conn->status = &keep_going;
		// Try to accept connection
    		conn->client_fd = Accept(server_fd, NULL, NULL);

    		// Case: successful connection
		if (conn->client_fd > 0) {
			// Open new thread to chat with client
			pthread_create(&tid, NULL, thread_start, conn);

		} else {
			// Free connection storage in case of failed connection
			free(conn);
		}
  	}
	// Close server and destroy calc object
  	close(server_fd);
  	calc_destroy(calc);
  	return 0;
}


void *thread_start(void *vargp) {
	// Obtaining connection info
	struct Client_conn *conn = (struct Client_conn *)vargp;
	int client_fd = conn->client_fd;
	struct Calc *calc = conn->calc;
	int *status = conn->status;
	pthread_detach(pthread_self());
	free(vargp);
	
	// Process connection
	rio_t in;
	// Wrap to client
	rio_readinitb(&in, client_fd);
	// Buffer for input
	char linebuf[LINEBUF_SIZE];
	
	// Loop to obtain input
	int done = 0;
	while (!done) {
		// Read input
		ssize_t n = rio_readlineb(&in, linebuf, LINEBUF_SIZE);
		if (n <= 0) {
			// error or end of input 
			close(client_fd);
			return NULL;
		} 
		else if (strcmp(linebuf, "quit\n") == 0 || strcmp(linebuf, "quit\r\n") == 0) {
			// quit command 
			close(client_fd);
			return NULL;
		} 
		else if (strcmp(linebuf, "shutdown\n") == 0 || strcmp(linebuf, "shutdown\r\n") == 0) {
      			// shutdown command 
      			*status = 0;
      			close(client_fd);
      			return NULL;
    		} 
		else {
			// process input line 
			int result;
			if (calc_eval(calc, linebuf, &result) == 0) {
				// expression couldn't be evaluated 
				rio_writen(client_fd, "Error\n", 6);
			} else {
				// output result to client 
				int len = snprintf(linebuf, LINEBUF_SIZE, "%d\n", result);
				if (len < LINEBUF_SIZE) {
					rio_writen(client_fd, linebuf, len);
				}
			}
		}
	}
	close(client_fd);
	return NULL;
}

// Function to chat with client
// Evaluates expressions with calc object
// Return 1: done with request (either fulfilled or error), keep server up
// Return 0: shutdown server
int chat_with_client(struct Calc *calc, int client_fd) {
	rio_t in;
	// Wrap to client
	rio_readinitb(&in, client_fd);
	// Buffer for input
	char linebuf[LINEBUF_SIZE];
	
	// Loop to obtain input
	int done = 0;
	while (!done) {
		// Read input
		ssize_t n = rio_readlineb(&in, linebuf, LINEBUF_SIZE);
		if (n <= 0) {
			// error or end of input 
			return 1;
		} 
		else if (strcmp(linebuf, "quit\n") == 0 || strcmp(linebuf, "quit\r\n") == 0) {
			// quit command 
			return 1;
		} 
		else if (strcmp(linebuf, "shutdown\n") == 0 || strcmp(linebuf, "shutdown\r\n") == 0) {
      			// shutdown command 
      			return 0;
    		} 
		else {
			// process input line 
			int result;
			if (calc_eval(calc, linebuf, &result) == 0) {
				// expression couldn't be evaluated 
				rio_writen(client_fd, "Error\n", 6);
			} else {
				// output result to client 
				int len = snprintf(linebuf, LINEBUF_SIZE, "%d\n", result);
				if (len < LINEBUF_SIZE) {
					rio_writen(client_fd, linebuf, len);
				}
			}
		}
	}
	return 1;
}

