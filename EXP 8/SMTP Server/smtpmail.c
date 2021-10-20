#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>
#define MAX 80 
#define SA struct sockaddr


void mail(int connfd)
{
	char buff[MAX],msg[2048];
	char name[MAX];
	char pass[MAX];
	int flag = 0;
	int ignore=0;
	
	FILE *fp;
	
	while(flag!=1)
	{
		bzero(name,sizeof(name));
		int s = read(connfd,name, sizeof(name));
		
		if(s==0)
		{
			//printf("[-]Connection with client terminated unexpectedly");
			return;
		}
		//printf("%s\n",name);
		bzero(pass,sizeof(pass));
		s = read(connfd,pass, sizeof(pass));
		if(s==0)
		{
			//printf("[-]Connection with client terminated unexpectedly");
			return;
		}
		//printf("%s\n",pass);
	
        
    		size_t len = 0;
    		ssize_t read;
    		
    		fp = fopen("logincred.txt","rb");
        	if(fp==NULL)
        	{
            		printf("[-]Logincred file open error\n");
            		snprintf(buff, MAX, "103 Internal authentication error");
            		write(connfd, buff, sizeof(buff));
            		return;   
        	}
    	
    		char * line = NULL;
		while ((read = getline(&line, &len, fp)) != -1) 
        	{
        		char * token = strtok(line, ",");
        		//printf("%s\n",token);
        	
        		if(strncmp(name,token,strlen(token))==0)
        		{	
        		
        			token = strtok(NULL, "\n");
        			//printf("%s\n",token);
        			if(strncmp(token,pass,strlen(token))==0)
        				flag=1;
        			else
        				flag=2;
        			break;
        		}
    		}
    		
    		if (line)
        		free(line);
    	
    		bzero(buff,sizeof(buff));
    		if(flag==0)
    		{
    			snprintf(buff, MAX, "100 User does not exist");
    			printf("%s as %s\n",buff,name);
    		}
    		if(flag==1)
    		{	
    			snprintf(buff, MAX, "101 Login Successful");
    			printf("%s\n",buff);
    		}
    		if(flag==2)
    		{
    			snprintf(buff, MAX, "102 Incorrect Password");
    			printf("%s\n",buff);
    		}
    		
    		write(connfd, buff, sizeof(buff));
    	}
    	
   	fclose(fp);
   	while(1)
   	{
   		bzero(msg,2048);
   		int s = read(connfd,msg,2048);
   		if(s==0)
   			return;
   		
   		else
   		{
   			size_t len = 0;
    			ssize_t read;
    			char * line = NULL;
   			//printf("**\n%s**\n",msg);
   			char copy[2048];
   			strcpy(copy,msg);
   			char rname[20];
   			flag=0;
   			bzero(buff,MAX);
   			char * token = strtok(msg, "\n");
   			token = strtok(NULL, " ");
   			token = strtok(NULL, "@");
   			strcpy(rname,token);
   			rname[0]-=32;
   			//printf("%s\n",rname);
   			fflush(stdin);
   			fp = fopen("logincred.txt","rb");
   			while ((read = getline(&line, &len, fp)) != -1) 
        		{
        			char * token = strtok(line, ",");
        			if(strncmp(rname,token,strlen(token))==0)
        			{	
        				flag=1;
        				break;
        			}
    			}
    			fclose(fp);
    			bzero(buff,sizeof(buff));
    			if(flag==0)
    			{
    				//printf("No such user\n");
    				snprintf(buff, MAX, "200 Reciever does not exist");
    			}
    			else
    			{
    				//printf("Yes such user\n");
    				
    				//rname[0]+=32;
    				char loc[38];
    				snprintf(loc, 38, "./%s/mymailbox.mail",rname);
    				//printf("After snprintf\n");
    				//printf("%s\n",loc);
    				
    				fp = fopen(loc,"a");
    				if(fp == NULL)
    				{
    					//printf("Cannot open the file\n");
    					snprintf(buff, MAX, "202 Sending failed");
    				}
    				else
    				{
    					
    					
    					char copy_[2048];
    					int m=0;
    					char * token = strtok(copy, "\n");
    					m += snprintf(copy_ + m,2048,"%s\n",token);
    					token = strtok(NULL, "\n");
    					m += snprintf(copy_ + m,2048,"%s\n",token);
    					token = strtok(NULL, "\n");
    					m += snprintf(copy_ + m,2048,"%s\n",token);
    					
    					time_t t = time(NULL);
  					struct tm tm = *localtime(&t);
  					m += snprintf(copy_+m,2048,"Received: %d-%02d-%02d : %02d : %02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
  					
  					token = strtok(NULL, "\n");
 					
 					while(token!=NULL)
  					{
  						m += snprintf(copy_ + m,2048,"%s\n",token);
  						token = strtok(NULL, "\n");
    					}
    					//printf("%s",copy_);
    					fflush(stdin);
    					fprintf(fp,"%s",copy_);
    					snprintf(buff, MAX, "201 Message successfully sent");
    					fclose(fp);
    				}
    			}
    			//fflush(stdin);
    			printf("%s\n",buff);
    			//fflush(stdin);
    			write(connfd,buff,MAX);
   		}
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
	
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("[-]Socket binding failed\n"); 
		exit(0); 
	} 
	else
		printf("[+]Socket is bound\n"); 


	if ((listen(sockfd, 5)) != 0) { 
		printf("[-]Listening failed\n"); 
		exit(0); 
	} 
	else
		printf("[+]Server is listening\n"); 
		
	
	len = sizeof(cli); 

	while(1)
	{
		connfd = accept(sockfd, (SA*)&cli, &len); 
		if (connfd < 0) { 
			printf("[-]Server could not accept\n"); 
			exit(0); 
		} 
		else
			printf("[+]Server has accepted the client\n");
			
		mail(connfd);
		
		char buff[MAX];
		bzero(buff,MAX);
		if(read(connfd,buff,MAX)==0)
		{
			printf("[-]Client has been disconnected\n");
			close(connfd);
		}
	}
		
	close(sockfd);
	
	
	return 0;
		
}
