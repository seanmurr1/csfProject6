/*
 * CSF Assignment 5
 * Shelby Coe : scoe4
 * Sean Murray : smurra42
 */

#include <stdio.h>      /* for snprintf */
#include "csapp.h"
#include "calc.h"

/* buffer size for reading lines of input from user */
#define LINEBUF_SIZE 1024

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

	// Loop to accept client requests
  	int keep_going = 1;
  	while (keep_going) {
		// Try to accept connection
    		int client_fd = Accept(server_fd, NULL, NULL);
    		if (client_fd > 0) {
			// Chat with client and then close connection
			keep_going = chat_with_client(calc, client_fd);
			close(client_fd);
		}
  	}
	// Close server and destroy calc object
  	close(server_fd);
  	calc_destroy(calc);
  	return 0;
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

