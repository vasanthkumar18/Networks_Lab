#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <arpa/inet.h>
#define MAX 80 
#define SA struct sockaddr

int sendmail(int connfd, char name[])
{
	char msg[2048],model[80];
	
	bzero(msg,2048);
	bzero(model,80);
	name[0]+=32;
	//printf("%s",name);
	snprintf(model, 80, "From: %s@localhost\n", name);
	
	char flush;
	printf("\n\nEnter the email in the following format:\n\nFrom: <username>@<domain name>\nTo: <username>@<domain name>\nSubject: <subject string, max 50 characters>\n<Message body – one or more lines, terminated by a final line with only a fullstop character>\n\nEnter your email:\n\n");
	char *line = NULL;
  	size_t len = 0;
 	ssize_t read = 0;
 	int l = 0;
  	read = getline(&line, &len, stdin);
  	l += snprintf(msg + l,2048,"%s",line);
  	//printf(line);
  	//char * token = strtok(line, " ");
  	if(strcmp(line,model))
  	{
  		//fflush(stdin);
  		printf("\nIncorrect format\n");
  		return -1;
  	}
  	read = getline(&line, &len, stdin);
  	l += snprintf(msg + l,2048,"%s",line);
  	char * token = strtok(line, " ");
  	if(token==NULL)
  	{
  		printf("\nIncorrect format\n");
  		return -1;
  	}
  	if(strcmp(token,"To:"))
  	{
  		printf("\nIncorrect format\n");
  		return -1;
  	}
  	token = strtok(NULL, "@");
  	if(token==NULL)
  	{
  		printf("\nIncorrect format\n");
  		return -1;
  	}
  	//printf(token);
  	token = strtok(NULL, "\n");
  	if(token==NULL)
  	{
  		printf("\nIncorrect format\n");
  		return -1;
  	}
  	//printf(token);
  	if(strcmp(token,"localhost"))
  	{
  		printf("\nIncorrect format\n");
  		return -1;
  	}
  	read = getline(&line, &len, stdin);
  	l += snprintf(msg + l,2048,"%s",line);
  	token = strtok(line, " ");
  	if(token==NULL)
  	{
  		printf("\nIncorrect format\n");
  		return -1;
  	}
  	if(strcmp(token,"Subject:"))
  	{
  		printf("\nIncorrect format\n");
  		return -1;
  	}
  	read = getline(&line, &len, stdin);
  	if(strcmp(line,".\n")==0)
  	{
  		printf("\nIncorrect format\n");
  		return -1;
  	}
  	l += snprintf(msg + l,2048,"%s",line);
  	do
  	{
  		getline(&line, &len, stdin);
  		l += snprintf(msg + l,2048,"%s",line);	
  	}while(strcmp(line,".\n"));
  	//printf("%s",msg);
  	write(connfd,msg,2048);
  	return 0;
}

void func(int connfd,char *name)
{
	//printf("\n**************************************************************************************************************");
	while(1)
	{
		printf("**************************************************************************************************************\n\nEnter 'Send Mail' for sending mail\nEnter 'Quit' for ending the connection\n\n");
		char ch[20];
		char flush;
		printf("Enter: ");
		scanf("%[^\n]",ch);
		scanf("%c",&flush);
		
        	if(flush!=10)
        		printf("NOT THE WHOLE LINE\n");
		if(strcmp(ch,"Send Mail")==0)
		{
			int ret = sendmail(connfd,name);
			name[0]-=32;
			if(ret!=-1)
			{
				char buff[80];
				bzero(buff,sizeof(buff));
       				int s = read(connfd,buff, sizeof(buff));	
     		  		printf("\n%s\n\n",buff);
				//printf("\nProcedure to send mail here\n\n**************************************************************************************************************");
			}
		}
		else if(strcmp(ch,"Quit")==0)
		{
			printf("\n[-]Disconnecting Client\n\n");
        		close(connfd);
			break;
		}	
		else
			printf("\nEnter a valid command\n");
	}
}

int main(int argc, char *argv[]) 
{ 
	if(argc<2)
	{
		printf("[-]Enter port number\n");
		return 1;
	}
	
	int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli; 


	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("[-]Socket creation failed\n"); 
		exit(0); 
	} 
	else
		printf("[+]Socket is created\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	int port_no = atoi(argv[1]);
	
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port_no);
	
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("[-]Connection with server failed\n"); 
		exit(0); 
	} 
	else
		printf("[+]Connected to server\n");
	
	char name[MAX];
	char pass[MAX];
	char buff[MAX];
	while(1)
	{
		printf("\nEnter name: ");
        	char flush;
       		scanf("%[^\n]",name);
        	scanf("%c",&flush);
        	if(flush!=10)
        		printf("NOT THE WHOLE LINE\n");
        	printf("Enter password: ");
        	scanf("%[^\n]",pass);
        	scanf("%c",&flush);
        	if(flush!=10)
        		printf("NOT THE WHOLE LINE\n");
        
       		write(sockfd, name, sizeof(name));
       		write(sockfd, pass, sizeof(pass));
       		
       		bzero(buff,sizeof(buff));
       		int s = read(sockfd,buff, sizeof(buff));
       		
       		printf("\n%s\n\n",buff);
       		
       		if(strncmp(buff,"101",3)==0)
       			break;
       		
        }
        
        func(sockfd,name);	
}
