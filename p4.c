
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>


#define DEBUG 1 
#define dbg if(DEBUG)
#define TRUE 1
#define FALSE 0
#define END_POINTS_FILE "endpoints"
#define ETHERNET_INTERFACE "eth0"
#define BUFSIZE 2048

int isFileOwner = FALSE;
int I = -1;
int N,b,c,F,B,P,S,T,n;
int socketId = -1;
char MY_IP_ADDR[INET_ADDRSTRLEN];
int MY_SOCKET_PORT = -1;
int localTime = 0;
int failed = FALSE;

struct node_information{
	int id;
	char IP[INET_ADDRSTRLEN];
	int port;
	int timestamp;
	int heartbeats;
	int isAlive;
} node_info;

typedef struct node_information NODE;

void writeFile(char* IP, int port);
int sendMsgToNode(char* msg, NODE node);
int sendMsgToNodeId(char* msg, int nodeId);
int sendMsgToNeighbors(char* msg);
void createHBMessage(char* message);	
void getMyIP(char* buffer);
void *start_server(void *arg);
void initNeighbors(int *neighbours, int skipSelf);
int waitForInit();
void loadNodeInfo();
void printNode(NODE n);
void printNodes();
void failNode();
void updateNodeStatus();
void writeNodesData();

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

NODE *global_node_info;
int *neighbors;
#define RAND_BUFFER 32

struct random_data *buf1, *buf2;
int *uniqueFailed;

int main(int argc, char *argv[])
{
	if (argc != 9)
	{
		printf("usage ./p4 N b c F B P S T\n");
		printf("N: number of peer nodes\n");
		printf("b: gossip parameter\n");
		printf("c: gossip parameter\n");
		printf("F: number of seconds after which a node is considered dead\n");
		printf("B: number of bad nodes that should fail\n");
		printf("P: number of seconds to wait between failures\n");
		printf("S: the seed of the random number generator\n");
		printf("T: Total number of seconds to run\n");
		exit(1);
	}
	n = N = atoi(argv[1]);
	b = atoi(argv[2]);
	c = atoi(argv[3]);
	F = atoi(argv[4]);
	B = atoi(argv[5]);
	P = atoi(argv[6]);
	S = atoi(argv[7]);
	T = atoi(argv[8]);

	dbg printf("Starting p4 with following parameters\n");
	dbg printf("Peer node/s : %d\n",N);
	dbg printf("Gossip parameter : %d\n",b);
	dbg printf("Gossip parameter : %d\n",c);
	dbg printf("Number of seconds after which a node is considered dead : %d\n",F);
	dbg printf("Number of bad nodes that should fail : %d\n",B);
	dbg printf("Number of seconds to wait between failures : %d\n",P);
	dbg printf("The seed of the random number generator : %d\n",S);

	if(N<2)
	{
		printf("At least 2 nodes are required.\n");
		exit(1);
	}
	if(b>N-1)
	{
		printf("Neighbors can not be more than nodes.\n");
		exit(1);
	}

	//generate some random port no.	
	int pid = getpid();
	int port = pid%20000+30000;

	socketId = socket(AF_INET, SOCK_DGRAM, 0);
	if (socketId < 0) {
		printf("ERROR opening socket\n");
		exit(1);
	}

	struct sockaddr_in myaddr;
	struct sockaddr_in remaddr;
	socklen_t addrlen = sizeof(remaddr);
    
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(port);
	
	/*Bind socket with port*/
	if (bind(socketId, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return -1;
	}

	struct timeval tv;

	tv.tv_sec = 30;  /* 30 Secs Timeout */
	tv.tv_usec = 0;

	/*Set timeout for socket*/
	setsockopt(socketId, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));


	MY_SOCKET_PORT = port;
	char MYIP[INET_ADDRSTRLEN];
	
	/*Find my machine's IP*/
	getMyIP(MYIP);

	struct sockaddr_in myServAddr;
	memset((char *)&myServAddr, 0, sizeof(myServAddr));

	//write the server IP to the file
	writeFile(MYIP, port);

	//wait for the OK signal from last joining process--
	if(I < N-1){
		waitForInit();
		loadNodeInfo();
	}
	else
	{
		dbg printf("I am last one to join\n");
	}
	
	int bad = B;
	
	uniqueFailed = (int*)calloc(N,sizeof(int));
	neighbors = (int*)calloc(b,sizeof(int));
	
	buf1 = (struct random_data*)calloc(1, sizeof(struct random_data));
	buf2 = (struct random_data*)calloc(1, sizeof(struct random_data));
	char *rand_statebufs2 = (char*)calloc(1, RAND_BUFFER);
	char *rand_statebufs1 = (char*)calloc(1, RAND_BUFFER);
	initstate_r(S+I, rand_statebufs1, RAND_BUFFER, buf1);
	initstate_r(S, rand_statebufs2, RAND_BUFFER, buf2);

	neighbors = (int*) malloc(sizeof(int)*b);

	
	pthread_t servThread;
	pthread_create(&servThread, NULL, start_server, NULL);
	
	char message[N*11];
	int compleated = FALSE;
	while(TRUE){
		for(int i =0;i<c;i++){
            if(localTime>=T)
            {
                compleated=TRUE;
                break;
            }
            initNeighbors(neighbors, TRUE);
			createHBMessage(message);
			if(!failed){
				sendMsgToNeighbors(message);
				dbg printf("Iteration %d complete\n",i);
				pthread_mutex_lock(&mutex);
				dbg printf("Heartbeat of %d is %d..\n",I, global_node_info[I].heartbeats);
				global_node_info[I].heartbeats++;
				global_node_info[I].timestamp++;
				pthread_mutex_unlock(&mutex);
				updateNodeStatus();
			}
			else
			{
				dbg printf("Node failed...");
			}
			sleep(1);
			localTime++;

			if(!failed && B>0 && localTime%P==0){
				failNode();
			}
		}
		if(compleated)
			break;
	}

	dbg printf("Waiting for all messages to arrive...\n");
	sleep(2);

	//pthread_join(servThread, NULL);

	close(socketId);

	if(isFileOwner)
		remove(END_POINTS_FILE);
	//printNodes();
	writeNodesData();
	printNodes();
}

// Output file listXX
void writeNodesData()
{
	const char *status = failed?"FAIL\n":"OK\n";
	char fileName[10];
	char line[32];
	sprintf(fileName,"list%d",I);
	FILE *file;
	file = fopen(fileName, "w");
	fputs(status,file);
	for(int i = 0; i<N;i++){
		sprintf(line,"%d %d\n",i,global_node_info[i].timestamp);
		fputs(line,file);
	}
	fclose(file);
}

//create heartbeat message
void createHBMessage(char* message){
	stpcpy(message,"");
	char temp[11];
	for(int i = 0;i<N;i++){
		sprintf(temp,"%d|",global_node_info[i].heartbeats);
		strcat(message,temp);
	}
}

void updateNodeStatus(){
	for(int i = 0;i<N;i++){
		if(localTime - global_node_info[i].timestamp >= F)
			global_node_info[i].isAlive = FALSE;
	}
}


// fetch my IP address
void getMyIP(char* buffer) 
{
	struct ifreq ifr;
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, ETHERNET_INTERFACE, IFNAMSIZ-1);
	ioctl(socketId, SIOCGIFADDR, &ifr);
	strncpy(buffer, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),INET_ADDRSTRLEN);
	strncpy(MY_IP_ADDR,  inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),INET_ADDRSTRLEN);
}

//send message function
int sendMsg(char* msg, char *server, int port ){

	dbg printf("Send \"%s\" to %s:%d via socket %d\n",msg, server, port, socketId);
	struct sockaddr_in remaddr;
	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(port);
	if (inet_aton(server, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	int slen=sizeof(remaddr);
	if (sendto(socketId, msg, strlen(msg), 0, (struct sockaddr *)&remaddr, slen)<0)
		perror("sendto");

}

void printNodes(){
	for(int i = 0;i<N;i++){
		printNode(global_node_info[i]);
	}
}

void printNode(NODE n){
	const char *status = n.isAlive?"ALIVE":"DEAD";
	printf("Node# %d [IP: %s, PORT: %d, TIME: %d, HB: %d, STATUS: %s]\n", n.id, n.IP, n.port, n.timestamp, n.heartbeats,status);
}

int sendMsgToNode(char* msg, NODE node){
	int port = node.port;
	char *server = node.IP;
	sendMsg(msg, server, port);
}

int sendMsgToNodeId(char* msg, int nodeId){
	dbg printf("Send \"%s\" to Node %d from Node %d\n",msg, nodeId, I);
	if(nodeId == I){
		return 1;
	}
	else
		sendMsgToNode(msg, global_node_info[nodeId]);
}


int sendMsgToNeighbors(char* msg){
	for(int i = 0;i<b;i++){
		sendMsgToNodeId(msg, neighbors[i]);
	}
}

void *start_server(void *arg)
{
	char buf[BUFSIZE];
	ssize_t recvlen;
	//int msgs = 0;

	char buffer[10];
	int nodeId = 0;
	int idx = 0;
	int heartbeat = 0;
	while (TRUE) {
		recvlen = recv(socketId, buf, BUFSIZE, 0);
		if (recvlen > 0) {
			buf[recvlen] = 0;
			dbg printf("received message: \"%s\"\n", buf);
			idx = 0;
			nodeId = 0;
			if(failed)
				continue;
			pthread_mutex_lock(&mutex);
			for(int i=0;i<recvlen;i++){
				if(buf[i]!='|')
					buffer[idx++] = buf[i];
				else{
					buffer[idx] = 0;
					//printf("Heartbeat is %s\n", buffer);
					heartbeat = atoi(buffer);
					if(global_node_info[nodeId].heartbeats < heartbeat && global_node_info[nodeId].isAlive)
					{
						global_node_info[nodeId].heartbeats = heartbeat;
						global_node_info[nodeId].timestamp = localTime;
					}		
					idx=0;
					nodeId++;
				}
			}
			pthread_mutex_unlock(&mutex);
		}
	}
}

void initNeighbors(int *neighbors, int skipSelf){
	int unique[N];
	for(int i=0;i<N;i++)
		unique[i] = 0;
	int b1 = b;;
	if(b>n-1)
		b1 = n-1;
	for(int i=0;i<b1;i++){
		int neighbor;
		random_r(buf1, &neighbor);
		neighbor = neighbor%N;
		while((skipSelf && neighbor==I) || unique[neighbor] || !global_node_info[neighbor].isAlive)
		{
			random_r(buf1, &neighbor);
			neighbor = neighbor%N;
		}
		neighbors[i] = neighbor;
		//printNode(global_node_info[neighbor]);
		unique[neighbor] = 1;
	}
}



void failNode(){
		int failedNode;
		random_r(buf2, &failedNode);
		failedNode = failedNode%N;
		while(uniqueFailed[failedNode]){
			random_r(buf2, &failedNode);
			failedNode = failedNode%N;
		}
		uniqueFailed[failedNode] = TRUE;
		//printNode(global_node_info[neighbor]);
		if(failedNode==I){
			failed=TRUE;
			global_node_info[I].isAlive = FALSE;
		}
		B--;
		n--;
}

//wait for OK signal
int waitForInit()
{
	dbg printf("Waiting for %d nodes to join...\n", (N-I-1));
	int recvlen;			/* # bytes received */
	char buf[BUFSIZE];	/* receive buffer */
    while(TRUE) {
		recvlen = recv(socketId, buf, BUFSIZE, 0);
		if (recvlen > 0) {
			buf[recvlen] = 0;
		}
		if(recvlen== 2){
			if(strcmp(buf, "OK")==0)
				break;
		}
	}
	return 0;
}

void loadNodeInfo(){

	global_node_info = (NODE*)malloc(N* sizeof(NODE));

	char *fileName = END_POINTS_FILE;
	FILE *file;
	char line[100];
	char temp[3][30];

	if(access(fileName, F_OK)!=-1){
		//File Exists
		file = fopen(fileName, "r");
	}
	else
	{
		printf("Error in reading file.");
		exit(-1);
	}


	int count = 0;
	while(fgets(line, sizeof(line), file) != NULL)
	{
		int index = 0;
		int tempArrayRow = 0;
		for(int i=0;i<strlen(line);i++){
			if(line[i]!=':')
			{
				temp[tempArrayRow][index++] = line[i];
			}
			else
			{
				temp[tempArrayRow++][index] = 0;
				index = 0;
			}
		}
		temp[tempArrayRow++][index] = 0;
		int node = atoi(temp[0]);
		global_node_info[node].timestamp = 0;
		global_node_info[node].isAlive = TRUE;
		global_node_info[node].heartbeats = 0;
		global_node_info[node].id = node;
		global_node_info[node].port = atoi(temp[2]);
		strcpy(global_node_info[node].IP,temp[1]);
		global_node_info[node].timestamp = 0;
	}

	fclose(file);
}


//write the node address details. lst node signals the rest of the nodes OK.
void writeFile(char* IP, int PORT){

	char endPointInfo[100];
	char line[100];
	char temp[10];
	char *fileName = END_POINTS_FILE;
	FILE *file;

	if(access(fileName, F_OK)!=-1){
		//File Exists
		file = fopen(fileName, "a+");
	}
	else
	{
		//File Does not Exists

		isFileOwner = TRUE;
		file = fopen(fileName, "w");
	}


	int count = 0;
	while(fgets(line, sizeof(line), file) != NULL)
		count=count+1;

	I = count;

	sprintf(endPointInfo, "%d:%s:%d\n", count,IP,PORT);
	fputs(endPointInfo, file);
	fclose(file);

	if(count==N-1){
		int i=0;
		loadNodeInfo();
		for(i=0; i<count; i++){
			sendMsgToNode("OK", global_node_info[i]);
		}
	}
}

