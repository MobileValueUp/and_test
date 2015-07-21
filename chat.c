#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAX 512

char quit[] = "exit";
int numClient =0;
int clientSock[MAX];

int getMaxfd(int);

int main(int argc, char* argv[])
{
	int connSock, listenSock;
	struct sockaddr_in s_addr, c_addr;
	int len, i , n, j;
	char rcvBuffer[BUFSIZ], sbuf[BUFSIZ];
	char name[MAX] ;
	int maxfd;

	fd_set read_fds;
	
	if(argc != 2 )
	{
		printf("Usage : %s <port>  \n", argv[0]);
		exit(1);
	}

	listenSock = socket(PF_INET, SOCK_STREAM, 0 );
	
	memset(&s_addr,0,sizeof(s_addr));
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(atoi(argv[1]));

	if(bind(listenSock, (struct sockaddr *) &s_addr, sizeof(s_addr)) == -1 )
	{
		printf("can not bind\n");
		return -1;
	}

	if( listen(listenSock, 5) == -1 )
	{
		printf("listen fail\n");
		return -1;
	}

	printf("Please enter your name: ");
	scanf("%s", name);
	printf("your name is %s\n", name);
	while(1)
	{
		maxfd = getMaxfd(listenSock) + 1;

		FD_ZERO(&read_fds);
		FD_SET(0,&read_fds);
		FD_SET(listenSock, &read_fds);
	
	
		for(i = 0; i< numClient; i++)
		{
			FD_SET(clientSock[i], &read_fds);
		}
			
		if(select(maxfd, &read_fds, NULL, NULL, NULL) < 0 )
		{
			printf("select error\n");
			exit(-1);
		}

		printf(">%s :\n ", name);

		if( FD_ISSET(listenSock, &read_fds))
		{
			connSock = accept(listenSock, (struct sockaddr *) &c_addr, &len);
		 	clientSock[numClient++] = connSock;
		}
		
		if( FD_ISSET(0, &read_fds))
		{
			if( ( n = read(0, sbuf, BUFSIZ )) > 0 )
			{
				if( strncmp( sbuf, "@talk", 5) == 0 )
				{
					char *ptr = strtok(sbuf, " ");
					char *ip = strtok(NULL," ");
					char *port = strtok(NULL," ");
					struct sockaddr_in c_addr;

					connSock = socket(PF_INET, SOCK_STREAM, 0);
					memset(&c_addr, 0, sizeof(c_addr));
					c_addr.sin_addr.s_addr = inet_addr(ip);
					c_addr.sin_family = AF_INET;
					c_addr.sin_port= htons(atoi(port));
					
					if( connect(connSock, (struct sockaddr*) &c_addr, sizeof(c_addr) ) == -1 )
					{
						printf("Can not connect\n");
						return -1;
					}
					clientSock[numClient++] = connSock;
				}
				else if ( strncmp(sbuf, quit, 4) == 0 )
				{
					break;
				}
				else
				{
					char temp[1024] ;
					strcat(temp,name);
					strcat(temp,": ");
					strcat(temp,sbuf);
					for( j=0; j < numClient; j++)
					{
						write(clientSock[j], temp,strlen(temp));
					}	
					temp[0] = '\0';
				}
			}
		}


		for( i=0; i< numClient; i++)
		{
			if(FD_ISSET(clientSock[i], &read_fds))
			{
				if( (n = read(clientSock[i] , rcvBuffer, sizeof(rcvBuffer))) != 0 )
				{
					if ( strncmp(rcvBuffer, quit, 4) == 0)
					{
						close(clientSock[i]);
						if( i != numClient-1)
							clientSock[i] = clientSock[numClient-1];
						numClient--;
						continue;
					}
					else
					{
//						for( j=0; j < numClient; j++)
//						{
//							write(clientSock[j], rcvBuffer, sizeof(rcvBuffer));
//							printf("%s",rcvBuffer);
//						}	
						
						printf("%s",rcvBuffer);
						rcvBuffer[0] = '\0';
					}
				}
				else
				{
					close(clientSock[i]);
					if( i != numClient-1)
						clientSock[i] = clientSock[numClient-1];

					continue;
				}
			}
		}	
	}
}

int getMaxfd(int n)
{
	int max = n;
	int i;
	
	for(i =0 ; i<numClient; i++)
	{
		if(clientSock[i] > max )
			max = clientSock[i];
	}

	return max;
}
