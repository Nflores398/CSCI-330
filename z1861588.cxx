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
#include <ctime>
using namespace std;
/**********************************************
This is a server always the client to get the
contents of a directoy and the contents of a
file. Using the command "GET". Or the user can
use "INFO" to get the server to send the Client
current time from the server.
************************************************
Commands "GET" and "INFO"
************************************************/
void processClientRequest(int connSock)
{
	//All varibles that helps with the server
	int fd, received;
	int i = 0;
	struct stat st;
	char content[1024] = "content of ";
	char notfounderror[1024] = "ERROR: ";
	char *currenttime;
	const char *readingfile;
	string timeis = "Current time: \n";
	string errorinput = "Wrong input entered!\n Formate: \"GET /....\" OR \"INFO\"";
	string command, pathname, token;
	char path[1024], buffer[3072];

	// read a message from the client
	if ((received = read(connSock, path, sizeof(path))) < 0)
	{
		perror("receive");
		exit(EXIT_FAILURE);
	}
	cout << "Client request: " << path << endl;
	//Reads the path and splits the path into a command and path
	istringstream temp(path);
	while (i != 2)
	{
		getline(temp, token, ' ');
		if (i == 0)
		{
			command = token;
		}
		if (i == 1)
		{
			pathname = token;
		}
		i++;
	}
	//If the user enters GET
	if (command == "GET")
	{
		if (stat(pathname.c_str(), &st) == 0)
		{
			//If the path was a Directory
			if (st.st_mode & S_IFDIR)
			{
				cout << "Directory!\n";
				// open directory
				DIR *dirp = opendir(pathname.c_str());
				if (dirp == 0)
				{
					// tell client that an error occurred
					strcpy(buffer, pathname.c_str());
					strcat(buffer, ": could not open directory\n");
					if (write(connSock, buffer, strlen(buffer)) < 0)
					{
						perror("write");
						cout << "\n------------------------------------\n";
						exit(EXIT_FAILURE);
					}
					cout << "\n------------------------------------\n";
					exit(EXIT_SUCCESS);
				}
				// read directory entries
				struct dirent *dirEntry;
				while ((dirEntry = readdir(dirp)) != NULL)
				{
					//used to remove the ... and .. from sending to client
					if (dirEntry->d_name[0] != '.')
					{
						strcpy(buffer, dirEntry->d_name);
						strcat(buffer, "\n");
					}
					else
					{
						continue;
					}
					//Sends all the items of the directory
					strcpy(buffer, dirEntry->d_name);
					strcat(buffer, "\n");
					if (write(connSock, buffer, strlen(buffer)) < 0)
					{
						perror("write");
						exit(EXIT_FAILURE);
					}
					cout << "sent: " << buffer;
				}
				closedir(dirp);
				cout << "done with client request\n";
				close(connSock);
				cout << "\n------------------------------------\n";
				exit(EXIT_SUCCESS);
			}
			//If the path is a FILE
			else if (st.st_mode & S_IFREG)
			{
				cout << "File!\n";
				readingfile = pathname.c_str();
				//Opens the file
				if ((fd = open(readingfile, O_RDONLY)) < 0)
				{
					cout << "error";
					cout << "\n------------------------------------\n";
					exit(EXIT_SUCCESS);
				}
				else
				{
					//Reads the contents of the file and send it to the client
					while (read(fd, buffer, sizeof(buffer) - 1) > 0)
					{
						strcat(content, readingfile);
						strcat(content, "\n");
						write(connSock, content, strlen(content));
						cout << "sent: \"" << buffer << "\"\n";
						strcat(buffer, "\n");
						if (write(connSock, buffer, strlen(buffer)) < 0)
						{
							perror("write");
							cout << "\n------------------------------------\n";
							exit(EXIT_FAILURE);
						}
					}
					cout << "done with client request\n";
					close(fd);
					cout << "\n------------------------------------\n";
					exit(EXIT_SUCCESS);
				}
			}
		}
		else
		{
			//If path doesnt go anywhere prints out this error message
			readingfile = pathname.c_str();
			cout << "ERROR\n Nothing FOUND!\n";
			strcat(notfounderror, readingfile);
			strcat(notfounderror, " not found\n");
			if (write(connSock, notfounderror, strlen(notfounderror)) < 0)
			{
				perror("write");
				cout << "\n------------------------------------\n";
				exit(EXIT_FAILURE);
			}
			cout << "\n------------------------------------\n";
			exit(EXIT_FAILURE);
		}
	}
	//If the users types INFO instead of GET
	else if (command == "INFO")
	{
		//Gets the current time and sends it to the client
		time_t timex = time(0);
		currenttime = ctime(&timex);
		cout << "Sent: " << timeis << endl;
		cout << "Sent: " << currenttime << endl;
		cout << "done with client request\n";
		strcpy(buffer, timeis.c_str());
		if (write(connSock, buffer, strlen(buffer)) < 0)
		{
			perror("write");
			cout << "\n------------------------------------\n";
			exit(EXIT_FAILURE);
		}
		if (write(connSock, currenttime, strlen(currenttime)) < 0)
		{
			perror("write");
			cout << "\n------------------------------------\n";
			exit(EXIT_FAILURE);
		}
		cout << "\n------------------------------------\n";
		exit(EXIT_SUCCESS);
	}
	else
	{
		//If the user doesnt enter GET or INFO prints error
		cout << "ERROR";
		strcpy(buffer, errorinput.c_str());
		if (write(connSock, buffer, strlen(buffer)) < 0)
		{
			perror("write");
			cout << "\n------------------------------------\n";
			exit(EXIT_FAILURE);
		}
		cout << "\n------------------------------------\n";
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{

	if (argc != 2)
	{
		cerr << "USAGE: z1861588 port\n";
		exit(EXIT_FAILURE);
	}

	// Create the TCP socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	// create address structures
	struct sockaddr_in server_address; // structure for address of server
	struct sockaddr_in client_address; // structure for address of client
	unsigned int addrlen = sizeof(client_address);

	// Construct the server sockaddr_in structure
	memset(&server_address, 0, sizeof(server_address)); /* Clear struct */
	server_address.sin_family = AF_INET;				/* Internet/IP */
	server_address.sin_addr.s_addr = INADDR_ANY;		/* Any IP address */
	server_address.sin_port = htons(atoi(argv[1]));		/* server port */

	// Bind the socket
	if (bind(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}

	// listen: make socket passive and set length of queue
	if (listen(sock, 64) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	cout << "z1861588 listening on port: " << argv[1] << endl;

	// Run until cancelled
	while (true)
	{
		int connSock = accept(sock, (struct sockaddr *)&client_address, &addrlen);
		if (connSock < 0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}
		// fork
		if (fork())
		{ // parent process
			close(connSock);
		}
		else
		{ // child process
			processClientRequest(connSock);
		}
	}
	close(sock);
	return 0;
}
