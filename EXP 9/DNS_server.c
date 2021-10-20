#include <sys/types.h>
#include <sys/socket.h>	
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>

		
#define SIZE 2048	// Size of Buffers
#define QSIZE 100	// Size of Queries

// DNS Header

int StartsWith(const char *a, const char *b){
   if(strncmp(a, b, strlen(b)) == 0) 
	   return 1;
   return 0;
}
int ends_with(char *a,char *b){
    for(int i=0;i<strlen(a);i++)
    {
        if(a[i]==b[0])
        {
            for(int j=0;j<strlen(b);j++)
                if(a[i+j]!=b[j])
                    return 0;
            return 1;
        }
    }
    return 0;    
}
void getRoot(char root[100]){
    char line[100];
    system("nslookup -type=ns . > root.txt");
    FILE *fp = fopen("root.txt", "r");
    while (fscanf(fp,"%s",line)!=EOF)
    {
        if(ends_with(line,".root-servers.net."))
        {
            strcpy(root,line);
            break;
        }
    }
}
void getDomain(char domain[100], char website[230],char root[100]){
    char cmd[169] = "nslookup -type=ns ",cmd2[100] = "", path[1035]="", *token;
    
    token = strtok(website, ".");
    int i = strncmp(token, "www", 3);
    if (i == 0)
        token = strtok(NULL, ".");
    char *lastpart = strtok(NULL, "\0");
    
    strcat(cmd,lastpart);  
    strcat(cmd," ");  
    strcat(cmd, root);
    strcat(cmd, " > ");
    
    strcat(cmd, "domain.txt");

    system(cmd);

    FILE *fp = fopen("domain.txt", "r");
    while (fgets(path, sizeof(path), fp) != NULL) {
        if(strstr(path, "nameserver"))
        {
            token=strtok(path, " = ");
            token=strtok(NULL, " = ");
            token[strcspn(token, "\n")] = 0;
            strcpy(domain, token);
            break;                    
        }
    }
}
void get_name_server(char domain[100], char website[100], char nameserver[100]){
    char cmd[169] = "nslookup -type=ns ",cmd2[100] = "", path[1035]="", *token, w[100];
    
    strcpy(w, website);
    token = strtok(website, ".");
    int i = strncmp(token, "www", 3);
    if (i == 0)
        token = strtok(NULL, ".");
    
    strcat(cmd,w);  
    strcat(cmd," ");  
    strcat(cmd, domain);
    strcat(cmd, " > ");
    
    strcat(cmd, "nameserver.txt");
    system(cmd);

    FILE *fp = fopen("nameserver.txt", "r");
    while (fgets(path, sizeof(path), fp) != NULL) {
        if(strstr(path, "nameserver"))
        {
            token=strtok(path, " = ");
            token=strtok(NULL, " = ");
            token[strcspn(token, "\n")] = 0;
            strcpy(nameserver, token);
            break;                    
        }
    }
}
void get_main_server(char result[100],char domain[100], char website[100], int type){
    char cmd[169] = "nslookup -type=",cmd2[100] = "", path[1035]="", root[100]="", *token,w[100], nameserver[100];
    

    if(type == 1)
        strcat(cmd, "aaaa ");
    else if(type == 2)
        strcat(cmd, "a ");
    else if(type == 3)
        strcat(cmd, "cname ");
    else
        strcat(cmd, "ns ");
    strcpy(w, website);
    w[strlen(w)-1] = '\0';
    website[strlen(website)-1] = '\0';
    

    strcat(cmd,w);  
    strcat(cmd," ");  


    get_name_server(domain,w, nameserver);
    strcat(cmd, nameserver);
    strcat(cmd, " > ");
    strcat(cmd, "cache/");
    strcat(cmd, website);

    strcat(cmd2, "cache/"); 
    strcat(cmd2, website);
    
    if(type == 1)
    {
        strcat(cmd, "-aaaa");
        strcat(cmd2, "-aaaa");
    }
        
    if(type == 2)
    {
        strcat(cmd, "-a");
        strcat(cmd2, "-a");
    }
    if (type == 3)
    {
        strcat(cmd, "-cname");
        strcat(cmd2, "-cname");
    }
    if (type == 4)
    {
        strcat(cmd, "-ns");
        strcat(cmd2, "-ns");
    }
    strcat(cmd, ".txt");
    strcat(cmd2, ".txt");
    printf("Command Invoked: %s\n",cmd);
    system(cmd);
    FILE *fp = fopen(cmd2,"r");

    if(type == 1 || type == 2)
    {
        int i = 0;
        while (fgets(path, sizeof(path), fp) != NULL) {
            if(strstr(path, "Address"))
            {
                if(i == 1)
                {
                    token=strtok(path, " = ");
                    token=strtok(NULL, " = ");
                    token[strcspn(token, "\n")] = 0;
                    strcpy(result, token);
                    i++;
                    break;
                }
                else
                    i++;
                                    
            }
        }
        if(i == 1)
            strcat(result, "not found");
    }
    if(type == 4)
    {
        
        while (fgets(path, sizeof(path), fp) != NULL) {
            if(strstr(path, "nameserver"))
            {
                token=strtok(path, " = ");
                token=strtok(NULL, " = ");
                token[strcspn(token, "\n")] = '\0';
                strcat(result, token);
                strcat(result, ";");         
            }
        }
    }
    if(type == 3)
    {
        int flag = 0;
        while (fgets(path, sizeof(path), fp) != NULL) {
            if(strstr(path, "canonical name"))
            {
            printf("%s\n", path);

                token=strtok(path, " = ");
                token=strtok(NULL, " = ");
                token=strtok(NULL, " = ");
                token[strcspn(token, "\n")] = 0;
                strcat(result, token);   
                flag = 1;   
            }
        }
        if(flag == 0)
        {
            strcat(result, "not found");
        }
    }
    
}

char *nslookup_handle(char result[100], char args[100], int type)
{
    int i, n;
    char website[230]="", domain[100]="",root[100]="";
    char website_name[100]="";
    
    strcpy(website, args);
    strcpy(website_name, args);

	printf("\n[ NSLOOKUP OUTPUT ]\n");
    getRoot(root);
    printf("Root: %s\n", root);
    getDomain(domain, website,root);
    printf("Domain:%s\n", domain);
    get_main_server(result, domain, website_name, type);
}



typedef struct {
	char ID[2];			// ID number
	unsigned short RD :1;		// Recursion Desired
	unsigned short TC :1;		// Truncate
	unsigned short AA :1;		// Authoritive Answer
	unsigned short OPCODE :1;	// Purpose of msg
	unsigned short QR :1;		// Query/Response

	unsigned short RCODE :4;	// Response Code
	unsigned short CD :1;		// Checking Disabled
	unsigned short AD :1;		// Authenticated Data
	unsigned short Z :1;		// Reserved
	unsigned short RA :1;		// Recursion Available

	unsigned short QDCOUNT;		// Question Count
	unsigned short ANCOUNT;		// Answer Count
	unsigned short NSCOUNT;		// Name Server Count
	unsigned short ARCOUNT;		// Additional Info Count
} DNS_HEADER;

// Constant sized fields of query structure
typedef struct {
	char QNAME[QSIZE];
	short unsigned qsize;
	char QTYPE[2];
	char QCLASS[2];
} DNS_QUESTION;

typedef struct {
	unsigned short TTL;
	unsigned short RDLENGTH;
	char RDATA[QSIZE];
} DNS_ANS;


typedef struct DNS_RECORD{
	DNS_QUESTION Q;
	DNS_ANS A;

	struct DNS_RECORD *next;
	struct DNS_RECORD *prev;
} DNS_RECORD;


typedef struct {
	int sock;
	char buf[SIZE];
	struct sockaddr_in clientAddr;
} HL_ARG;

DNS_RECORD *Cache;
pthread_mutex_t lock_cache;

// Parse the DNS Header from received buffer
void parseHeader(char *buffer, DNS_HEADER *header){
	char temp;

	header->ID[0] = buffer[0];
	header->ID[1] = buffer[1];

	// Taking the 3rd byte and parsing it
	temp = buffer[2];
	header->QR = (temp & 128) >> 7;			
	header->OPCODE = (temp & 120) >> 3;	 
	header->AA = (temp & 4) >> 2;				
	header->TC = (temp & 2) >> 1;
	header->RD = temp & 1;

	// Taking the 4th byte and parsing
	temp = buffer[3];
	header->RA = (temp & 128) >> 7;
	header->Z = (temp & 64) >> 6;
	header->AD = (temp & 32) >> 5;
	header->CD = (temp & 16) >> 4;
	header->RCODE = temp & 15;

	// Taking 5th and 6th byte and parsing Question Entry count
	header->QDCOUNT = buffer[4] << 8;
	header->QDCOUNT += buffer[5];

	// Taking 7th and 8th byte and parsing Answer Entry count
	header->ANCOUNT = buffer[6] << 8;
	header->ANCOUNT += buffer[7];

	// Taking 9th and 10th byte and parsing Name Server count
	header->NSCOUNT = buffer[8] << 8;
	header->NSCOUNT += buffer[9];

	// Taking 11th and 12th byte and parsing Additional Info count
	header->ARCOUNT = buffer[10] << 8;
	header->ARCOUNT += buffer[11];
}


// Replace a substring in a string
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}
// Parse Question from Question buffer
void parseQuestion(char *qs, DNS_QUESTION *q){

	// Fetch the Question
	int i = 0;
	while (qs[i])
		i += qs[i] + 1;
	q->qsize = i + 1;

	for (i = 0; i < q->qsize; i++)
		q->QNAME[i] = qs[i];

	q->QTYPE[0] = qs[q->qsize];
	q->QTYPE[1] = qs[q->qsize + 1];
	
	q->QCLASS[0] = qs[q->qsize + 2];
	q->QCLASS[1] = qs[q->qsize + 3];
}

void check_4(char final[4], char *octet){
	strcpy(final, "");

	if (strlen(octet) == 1){
		strcat(final, "000");
		strcat(final, octet);
	}
	else if (strlen(octet) == 2){
		strcat(final, "00");
		strcat(final, octet);
	}
	else if (strlen(octet) == 3){
		strcat(final, "0");
		strcat(final, octet);
	}
	else
		strcat(final, octet);
}

char *substr(const char *src, int m, int n){
	int len = n - m;
	char *dest = (char*)malloc(sizeof(char) * (len + 1));
	for (int i = m; i < n && ((*src + i) != '\0'); i++){
		*dest = *(src + i);
		dest++;
	}
	*dest = '\0';
	return dest-len;
}

int cname( char *RDATA, char re[100], char website[100]){
	char r[100], *token;
	strcpy(r, re);
	int i = 0, loop = 0;
	token = strtok(r, ".");
	while (token != NULL){
		short m = strlen(token);
		RDATA[i] = m;
		i++;
		loop = 0;
		while (token[loop] != '\0'){
			RDATA[i] = token[loop];
			loop++;
			i++;
		}
		token = strtok(NULL, ".");
	}

	RDATA[i++] = 0;
	return i;
}
// Parse IPv4 to RDATA
void parseIPv4(char *RDATA, char *ip){
	char *temp;

	// Break string into IP fields
	temp = strtok(ip,".");
	RDATA[0] = atoi(temp);

	temp = strtok(NULL, ".");
	RDATA[1] = atoi(temp);

	temp = strtok(NULL, ".");
	RDATA[2] = atoi(temp);

	temp = strtok(NULL, ".");
	RDATA[3] = atoi(temp);
}

// Parse IPv6 to RDATA
void parseIPv6(char *RDATA, char *IP2){
	char *IP, *octet;
	char final[4], format[20];

	if (strstr(IP2,"::")){
		IP = str_replace(IP2, "::", ":0000:0000:");
		octet = strtok(IP, ":");
	}
	else {
		octet = strtok(IP2, ":");
	}
	check_4(final, octet);
	char byte[2];
	char *ptr;
	RDATA[0] = strtoumax(substr(final, 0, 2), &ptr, 16); 
	RDATA[1] = strtoumax(substr(final, 2, 4), &ptr, 16);

	short i = 1;
	for (short i = 1; i < 6; i++){
		octet = strtok(NULL,":");
		check_4(final, octet);
		RDATA[i*2] = strtoumax(substr(final, 0, 2), &ptr, 16);
		RDATA[i*2 + 1] = strtoumax(substr(final, 2, 4), &ptr, 16);
	}

	octet = strtok(NULL, "\0");
	check_4(final, octet);

	printf("%s\n", octet);
	RDATA[14] = strtoumax(substr(final, 0, 2), &ptr, 16);
	RDATA[15] = strtoumax(substr(final, 2, 4), &ptr, 16);

}

// Check if Cache has the Query Request's Response
int fetchFromCache(DNS_QUESTION *q, DNS_ANS *ans){
	printf("\n[ CHECKING CACHE ]\n");
	DNS_RECORD *entry = Cache;
	int flag = 0;

	while (entry != NULL){
		if (q->qsize == entry->Q.qsize){
			if (q->QTYPE[0] == entry->Q.QTYPE[0] && q->QTYPE[1] == entry->Q.QTYPE[1]){
				short matching = 1;
				for (unsigned short i = 0; i < q->qsize; i++) {

					// Check if Domain Names are the same
					if (q->QNAME[i] != entry->Q.QNAME[i]){
						matching = 0;
						break;
					}

					if (matching){
						*ans = entry->A;
						flag = 1;
						break;
					}
				}
			}
		}
		entry = entry->next;		// Goto next cache entry
	}

	if (!flag)
		printf("\n[ CACHE MISS ]\n");
	else
		printf("\n[ CACHE HIT ]\n");
	
	return flag;
}

// Converts query name to URL
void NameToString(char *str, DNS_QUESTION *q){
	unsigned short i = 0, j = 0;
	while (q->QNAME[i]){
		j = i + 1;
		i += q->QNAME[i] + 1;
		while (j < i){
			str[j - 1] = q->QNAME[j];
			++j;
		}
		str[j - 1] = '.';
	}
	str[j] = '\0';
}

// Add a new entry to the cache
void addCache(DNS_QUESTION *q, DNS_ANS *ans){
	DNS_RECORD *entry = (DNS_RECORD*)malloc(sizeof(DNS_RECORD));

	entry->Q = *q;
	entry->A = *ans;
	entry->next = NULL;
	entry->prev = NULL;

	if (Cache != NULL){
		entry->next = Cache;
		Cache->prev = entry;
	}
	Cache = entry;
	printf("\n[ NEW ENTRY ADDED TO CACHE ]\n");
}

// Fetch the Answer Iteratively
int fetchIterative(DNS_QUESTION *q, DNS_ANS *ans){
	ans->TTL = 30;
	int n = 1;
	char web[100], website[100], result[100] = "", fname[100]="", *token, w[100], path[100];
	NameToString(web, q);			// Get website name
	strcpy(w, web);
	strcat(fname, "cache/");
	strcat(fname, web);				// Make a cache entry for website 

	// For AAAA Queries
	if (q->QTYPE[1] == 0x1C){
		nslookup_handle(result, web, 1);
		printf("Results: %s\n", result);
		if (strncmp(result, "not found", 9) == 0)
			ans->RDLENGTH = 0;
		else {
			ans->RDLENGTH = 16;
			parseIPv6(ans->RDATA, result);
		}

		addCache(q, ans);
	}

	// For A Queries
	else if(q->QTYPE[1] == 0x1){
		nslookup_handle(result, web, 2);
		printf("Result: %s\n", result);
		if (strncmp(result, "not found", 9) == 0)
			ans->RDLENGTH = 0;
		else {
			ans->RDLENGTH = 4;
			parseIPv4(ans->RDATA, result);
		}
		addCache(q, ans);
	}

	// For CNAME Queries
	else if(q->QTYPE[1] == 0x5){
		strcpy(website, web);
		token = strtok(w, ".");
		int i = strncmp(token, "www", 3);
		if (!i){
			token = strtok(NULL, "\0");
			strcpy(website, token);
		}

		nslookup_handle(result, web, 3);
		if (strncmp(result, "not found", 9) == 0)
			ans->RDLENGTH = 0;
		else{
			int r = cname(ans->RDATA, result, website);
			ans->RDLENGTH = r;
			printf("Result : %s %i\n", website, r);
			return 666;
		}
	}

	// For NS Queries
	else if(q->QTYPE[1] == 0x2){
		strcpy(website, web);
		token = strtok(w, ".");
		int i = strncmp(token, "www", 3);
		if (!i){
			token = strtok(NULL, "\0");
			strcpy(website, token);
		}


		nslookup_handle(result, web, 4);
		if (strncmp(result, "not found", 9) == 0)
			ans->RDLENGTH = 0;
		else{
			int r = cname(ans->RDATA, result, website);
			ans->RDLENGTH = r;
			printf("Result : %s %i\n", website, r);
			return 666;
		}

	}
	return n;

}

int resolveQuery(DNS_QUESTION *q, DNS_ANS *ans){
	ans->TTL = 30;
	int n = 1;	
	// Check if its not in the cache
	if (fetchFromCache(q, ans) == 0)
		n = fetchIterative(q, ans);
	return n;
}

// Packs a DNS Packet Header
void assignHeader(char *buffer, DNS_HEADER *header, unsigned short RDLENGTH){
	
	// Assign ID
	buffer[0] = header->ID[0];
	buffer[1] = header->ID[1];

	// Pack the 3rd byte
	char temp = 1;	//QR = 1 as it is a Query Response

	temp = temp << 4;
	temp = temp | (header->OPCODE);	// Assign OPCODE

	temp = temp << 1;
	temp = temp | (header->AA);		// Assign AA

	temp = temp << 1;
	temp = temp | (header->TC);		// Assign TC

	temp = temp << 1;
	temp = temp | (header->RD);		// Assign RD

	buffer[2] = temp;

	temp = 0;						// Assign RA as 0
	
	temp = temp << 7;
	temp = temp | (header->RCODE);	// ASSIGN RCODE

	buffer[3] = temp;

	buffer[5] = header->QDCOUNT;
	buffer[7] = (RDLENGTH == 0) ? 0 : 1;
	buffer[9] = header->NSCOUNT;
	buffer[11] = header->ARCOUNT;
}

void assignQuestion(char *qField, DNS_QUESTION *q){
	unsigned i = 0;
	// Get the Question Field
	while (i < q->qsize){
		qField[i] = q->QNAME[i];
		++i;
	}

	qField[i++] = q->QTYPE[0];
	qField[i++] = q->QTYPE[1];
	
	qField[i++] = q->QCLASS[0];
	qField[i++] = q->QCLASS[1];
}

void assignAnswer(char *ansField, DNS_QUESTION *q, DNS_ANS *ans){
	unsigned i = 0;
	while (i < q->qsize){
		ansField[i] = q->QNAME[i];
		++i;
	}

	ansField[i++] = q->QTYPE[0];
	ansField[i++] = q->QTYPE[1];

	ansField[i++] = q->QCLASS[0];
	ansField[i++] = q->QCLASS[1];

	ansField[i++] = 0;
	ansField[i++] = 0;
	ansField[i++] = 0;
	ansField[i++] = ans->TTL;

	ansField[i++] = 0;
	ansField[i++] = ans->RDLENGTH;

	for (unsigned j = 0; j < ans->RDLENGTH; j++)
		ansField[i++] = ans->RDATA[j];
}

unsigned createResponse(DNS_HEADER *header, DNS_QUESTION *q, DNS_ANS *ans, char *buffer){
	memset(buffer, 0, SIZE);

	unsigned pos = 0;

	assignHeader(buffer, header, ans->RDLENGTH);
	pos = 12;		// Header is 12 bytes

	assignQuestion(buffer + pos, q);
	pos += (q->qsize) + 4;

	if (ans->RDLENGTH != 0){
		assignAnswer(buffer + pos, q, ans);
		pos += (q->qsize) + 10 + (ans->RDLENGTH);
	}
	return pos;
}

// Searching the Cache
void *TTLHandler(){
	pthread_mutex_lock(&lock_cache);
	DNS_RECORD *entry, *temp;
	entry = Cache;	// To iterate through the cache

	while (entry != NULL){
		entry->A.TTL -= 1;

		if (entry->A.TTL == 0){
			if (entry->next != NULL)
				(entry->next)->prev = entry->prev;
			
			if (entry->prev == NULL)
				Cache = entry->next;
			else
				(entry->prev)->next = entry->next;
			temp = entry;
			printf("\n[ DELETING CACHE ENTRY : %s ]\n", temp->Q.QNAME);
			entry = entry->next;
			free (temp);
		}
		else
			entry = entry->next;
	}
	pthread_mutex_unlock(&lock_cache);
}

void *cacheHandler(){
	clock_t start, end;
	while (1){
		start = clock() / CLOCKS_PER_SEC;
		while (1){
			end = clock() / CLOCKS_PER_SEC;
			if (end - start >= 1)
				break;
		}
		pthread_t T_ID;
		pthread_create(&T_ID, NULL, TTLHandler, NULL);
	}
}

void *handleLookup(void *ARG){
	HL_ARG *arg = (HL_ARG *)ARG;

	DNS_HEADER requestHeader;
	DNS_QUESTION requestQuestion;
	DNS_ANS ans;
	parseHeader(arg->buf, &requestHeader);
	parseQuestion(arg->buf + 12, &requestQuestion);

	int n = resolveQuery(&requestQuestion, &ans);
	if(requestQuestion.QTYPE[1] == 0x2){

		DNS_ANS ans[n];
	}

	char buffer[SIZE];
	unsigned int packetSize;
	packetSize = createResponse(&requestHeader, &requestQuestion, &ans, buffer);

	if (sendto(arg->sock, buffer, packetSize, 0, (struct sockaddr *)&(arg->clientAddr), sizeof(arg->clientAddr)) < 0 )
		printf("Error in sendto()");
	else
		printf("\n[ RESPONSE SENT ]\n");
	free(arg);
}


int main(int argc, char *argv[]){
	struct sockaddr_in server_addr, client_addr;
	int sock, newsock, opt = 1;
	int client_addr_len = sizeof(client_addr);
	char recvBuff[SIZE];
	int recvLen;

	Cache = NULL;
	pthread_mutex_init(&lock_cache, NULL);

	unsigned short portno;
	// Check if port number was given or not
	if (argc < 2){
		printf("\nPort Number: ");
		scanf("%hu", &portno);
	}
	else 
		portno = atoi(argv[1]);

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("Socket Error");
		return -1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0){
		perror("Error at Setting sock options");
		return -1;
	}

	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portno);

	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("Error at BIND");
		return 1;
	}

	printf("Server is on PORT: %hu\n", portno);

	pthread_t T1;
	pthread_create(&T1, NULL,cacheHandler, NULL); 

	HL_ARG *arg;
	while (1){
		if ((recvLen = recvfrom(sock, recvBuff, SIZE - 1, 0, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
			perror("Error at recvfrom");
		else{
			arg = (HL_ARG *)malloc(sizeof(HL_ARG));
			for (int i = 0; i < SIZE; i++)
				arg->buf[i] = recvBuff[i];
			arg->sock = sock;
			arg->clientAddr = client_addr;
			pthread_t T2;
			pthread_create(&T2, NULL, handleLookup, (void*)arg);

			printf("\n[ REQUEST GENERATED ]\n");
		}
	}

	return 0;
}
