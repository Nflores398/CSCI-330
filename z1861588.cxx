/*
 * TCPServerReadDir.cxx
 * 
 * TCP server
 * 
 * 	loops/forks to serve request from client 
 * 	      opens directory, sends back lines of file names to client
 * 
 * 	command line arguments:
 * 		argv[1] port number to receive requests on
 * 
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

void processClientRequest(int connSock) {
	int received;
	const char *readingfile;
	int fd;
	struct stat st;
	char content[20] = "content of ";
	string token;
	string errorinput = "Wrong input entered!\n Formate: \"GET /....\"";
	int i = 0;
	string command, pathname;
	char path[1024], buffer[1024],buffer2[3072];

	// read a message from the client
	if ((received = read(connSock, path, sizeof(path))) < 0) {
		perror("receive");
		exit(EXIT_FAILURE);
	}
	cout << "Client request: " << path << endl;
	istringstream temp(path);
	while(i != 2)
	{
		getline(temp,token,' ');
		if(i == 0)
		{
			command = token;
			cout << command << endl;
		}
		if(i == 1)
		{
			pathname = token;
		}
		i++;
	}
	if(command == "GET")
	{
		if( stat(pathname.c_str(),&st) == 0 )
		{
			if(st.st_mode & S_IFDIR)
			{
				cout << "Directory!\n";
				if(pathname.empty())
				{
					pathname = ".";
				}
	// open directory	
			DIR *dirp = opendir(pathname.c_str());
			if (dirp == 0) {
			// tell client that an error occurred
			strcpy(buffer, pathname.c_str());
			strcat(buffer, ": could not open directory\n");
			if (write(connSock, buffer, strlen(buffer)) < 0) {
				perror("write");
				exit(EXIT_FAILURE);
			}
			exit(EXIT_SUCCESS);
			}
	
		// read directory entries
		struct dirent *dirEntry;
		while ((dirEntry = readdir(dirp)) != NULL) {
			if (dirEntry->d_name[0] != '.') 
			{
				strcpy(buffer, dirEntry->d_name);
				strcat(buffer, "\n");
			}
			else
			{
				continue;
			}
			strcpy(buffer, dirEntry->d_name);
			strcat(buffer, "\n");
			if (write(connSock, buffer, strlen(buffer)) < 0) {
				perror("write");
				exit(EXIT_FAILURE);
			}
			cout << "sent: " << buffer;		
		}	
		closedir(dirp);
		cout << "done with client request\n";
		close(connSock);
		exit(EXIT_SUCCESS);
		}
		else if(st.st_mode & S_IFREG)
		{	
			readingfile = pathname.c_str();
			if((fd = open(readingfile, O_RDONLY)) < 0)
			{
				cout << "error";
				exit(EXIT_SUCCESS);
			}
			else
			{
				while(read(fd,buffer2, sizeof(buffer2)-1) > 0)
				{
					strcat(content,readingfile);
					strcat(content,"\n");
					write(connSock, content, strlen(content));
					if(buffer2 == NULL || buffer2[0] == '\0')
					{
						close(fd);
						exit(EXIT_SUCCESS);
					}
					cout << "sent: " << buffer2 << endl;
					strcat(buffer2, "\n");
			if (write(connSock, buffer2, strlen(buffer2)) < 0) {
				perror("write");
				exit(EXIT_FAILURE);
			}
					
				}
				close(fd);
				exit(EXIT_SUCCESS);
			}
		}
		
	}
	}
	else
	{
		cout << "ERROR";
		strcpy(buffer, errorinput.c_str());
		if (write(connSock, buffer, strlen(buffer)) < 0) {
			perror("write");
		 	exit(EXIT_FAILURE);
		}
		exit(EXIT_FAILURE);
	}
}
        
int main(int argc, char *argv[]) {

	if (argc != 2) {
		cerr << "USAGE: z1861588 port\n";
		exit(EXIT_FAILURE);
	}
	
	// Create the TCP socket 
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}	
	// create address structures
	struct sockaddr_in server_address;  // structure for address of server
	struct sockaddr_in client_address;  // structure for address of client
	unsigned int addrlen = sizeof(client_address);	

	// Construct the server sockaddr_in structure 
	memset(&server_address, 0, sizeof(server_address));   /* Clear struct */
	server_address.sin_family = AF_INET;                  /* Internet/IP */
	server_address.sin_addr.s_addr = INADDR_ANY;          /* Any IP address */
	server_address.sin_port = htons(atoi(argv[1]));       /* server port */

	// Bind the socket
	if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}	
	
	// listen: make socket passive and set length of queue
	if (listen(sock, 64) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	
	cout << "z1861588 listening on port: " << argv[1] << endl;

	// Run until cancelled 
	while (true) {
		int connSock=accept(sock, (struct sockaddr *) &client_address, &addrlen);
		if (connSock < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		// fork
		if (fork()) { 	    // parent process
			close(connSock);
		} else { 			// child process
			processClientRequest(connSock);
		}
	}	
	close(sock);
	return 0;
}
