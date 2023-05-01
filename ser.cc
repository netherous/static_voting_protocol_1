#include <asm-generic/socket.h>
#include <set>
#include <algorithm>
#include <queue>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <cstdio>
#include <string.h>
#include <arpa/inet.h>
#include <vector>
#include <sstream>
#include <map>

using namespace std;

std::vector<int> fd_list;
constexpr int SERVER_AMT = 8;
int SID = 0;
vector<pair<string,int>> server_ip;
vector<int>server_sockfd(10,0);
map<int,int>fd_node;
stringstream sstream;
int client_fd = -1;
int vn =1;
int ru = 8;
vector<int> ds = {1,9,9};
set<int> connection_set;
bool lock = false;

void sendm(string s, int fd){
	if(write(fd, s.c_str(), s.size()) < 0 ) perror("ERROR Writing");
}

void create_connection(int i){
	const char * IP = server_ip[i].first.c_str();
	int portno = server_ip[i].second;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0 ){
        perror("Error creating socket");
        exit(1);
    } 
    struct sockaddr_in servaddr;
    bzero ((char*) & servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portno);
    servaddr.sin_addr.s_addr = inet_addr(IP);

    int ret = connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    if(ret <0){
        perror("Error connection");
        exit(1);
    }
	fd_node[sockfd] = i+1;
	server_sockfd[i+1] = sockfd;
	string r="server " + to_string(SID);
	if(write(sockfd, r.c_str(), strlen(r.c_str())) < 0 ) perror("ERROR Writing");
}

void read_file(string file_name){
	string content;
	ifstream fin(file_name);

	while(getline(fin,content)){
		stringstream ss(content);
		server_ip.emplace_back("",0);
		ss >> server_ip.back().first >> server_ip.back().second;
	}
}

void create_socket_address(struct sockaddr_in &addr, unsigned short int port){
	bzero((char*) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
}

int start_server(int server_id){
	struct sockaddr_in addr;
	//Create socket for listening
	int sockfd = socket(AF_INET, SOCK_STREAM,0);
	if(sockfd <0) perror("ERROR socket: ");	

	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");
	create_socket_address(addr, 19190+server_id);

	if(bind(sockfd,(struct sockaddr*) &addr, sizeof(addr))) perror ("Error bind: ");

	if(listen (sockfd, 5)) perror("Error listening");
	
	return sockfd;

} 

void client_handler();

void input(int fd){
	if(fd == client_fd){
		client_handler();
		return;
	}
	char buff[200];
	bzero(buff,sizeof(buff));
	if(read(fd,buff,sizeof(buff)) < 0) perror("ERROR Reading");
	string str(buff);
	if(str == "client"){
		client_fd = fd;
		cout << "server " << SID << " connected client" << endl;
	}else{
		cout << "server " << SID << " received " << buff << endl;
		stringstream ss(str);
		string first;
		ss >> first;
		// cout << first << endl;
		if(first == "server"){
			string second;
			ss >> second;
			int n = stoi(second);
			// cout <<"server " <<SID<< " accepted server " <<n << ", " << fd << endl;
			server_sockfd[n] = fd;
			fd_node[fd] = n;		
		}else if(first == "vote"){

		}else if(first == "abort"){
			cout << "server " << SID << " aborted" << endl;
		}else if(first == "commit"){

		}
	}
}

void process_connections(int sockfd){
	struct sockaddr_in clientaddr;
	socklen_t clilen = sizeof(clientaddr);	
	bzero((char *) &clientaddr,clilen);
	fd_set fds, fds1;
	FD_ZERO(&fds);
	FD_SET(sockfd,&fds);
	for(int f: server_sockfd){
		if(f != 0)
			FD_SET(f, &fds);
	}
	bool finished = 0;
	while(1 && !finished){
		fds1 = fds;
		if(select(FD_SETSIZE, &fds1, NULL,NULL,NULL) < 0){
			perror("ERROR select");
			exit(EXIT_FAILURE);
		}
		for(int fd = 0; fd < FD_SETSIZE; ++fd){
			if(FD_ISSET(fd, &fds1)){
				if(fd == sockfd){
					int clientfd = accept(sockfd, (struct sockaddr*) &clientaddr, &clilen);
					if(clientfd < 0) perror("ERROR accepting");
					// std::cout << clientfd << std::endl;
					// printf("server %d: got connection from %s port %d\n", SID,
					// 		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
					FD_SET(clientfd,&fds);
					fd_list.push_back(clientfd);
				}else{
					//TODO process the given file descriptor	
					// cout << fd << endl;
					input(fd);
				}
			}
		}
	}
}

void server(int server_id){
	SID = server_id;
	cout<< "starting server" << server_id<< endl;
	int sockfd = start_server(server_id);
	sleep(2);
	read_file("server_ip.txt");
	for(int i = 0 ; i < SID-1; i++){
		create_connection(i);
	}
	for(int i = 1; i <= SERVER_AMT;i++){
		if(i!=SID) connection_set.insert(i);
	}
	sleep(1);
	process_connections(sockfd);
	close(sockfd);
}

void forking(int amt){
	if(amt==0)return;
	if(fork() == 0){
		forking(--amt);
	}else{
		server(amt);
	}
}

int main(){
	forking(SERVER_AMT);
	return 0;
}

void update_handler();
void p1t_handler();	
void m1g_handler();
void p2t_handler();
void info();
void client_handler(){
	char buff[20];
	bzero(buff,sizeof(buff));
	if(read(client_fd, buff, sizeof(buff)) < 0 ) perror("ERROR Reading");
	string input(buff);
	string temp = "Server " + to_string(SID);
	cout << temp << " handling client : "<<input << endl;
	if(input=="update"){
		update_handler();
	}else if(input == "p1t"){
		p1t_handler();	
	}else if(input == "p2t"){
		p2t_handler();
	}else if(input == "m1g"){
		m1g_handler();
	}else{
		info();
	}	
}
bool is_distinguished();
void abourt();
void commit();
void update_handler(){
	if(!is_distinguished()){
		abourt();
		sendm("failed",client_fd);
	}else{
		commit();
		sendm("success",client_fd);
	}
}
void p1t_handler(){
	if(SID <= 4){
		for(int i = 5; i<=8;++i)
			connection_set.erase(i);
	}else{
		for(int i = 1; i<=4;++i)
			connection_set.erase(i);
	}
	cout << "size : " << connection_set.size()<< endl;
}	
void m1g_handler(){
	if(SID > 1 && SID < 8){
		for(int i = 2; i < 8; i++)
			if(SID != i) connection_set.insert(i);
	}
}
void p2t_handler(){
	if(SID==1){
		connection_set.clear();
	}else if(SID == 8){
		connection_set.clear();
	}
	else if(SID <= 4){
		connection_set.erase(1);
	}else{
		connection_set.erase(8);
	}
}
void info(){
	stringstream ss;
	ss << "Server " << SID << " info: ";
	ss <<"vn:"<< vn << " ru:" << ru << " ds:";
	for(int k: ds){
		if(k < 9) ss << k << " ";
	}
	ss <<"conn:";
	for(int k: connection_set)
		ss << k << " ";
	string s = ss.str();
	cout << s << endl;
	if(write(client_fd, s.c_str(), strlen(s.c_str())) < 0 ) perror("ERROR Writing");
}
bool is_distinguished(){
	return false;
}
void abourt(){
	lock = false;
	for(int i : connection_set){
		sendm("abort", server_sockfd[i]);
	}
}

void commit(){
	lock = false;
	stringstream ss;
	ss << "commit "<< vn << " " << ru << " ";
	for(int k:ds){
		ss << k << " ";
	}
	for(int i: connection_set){
		sendm(ss.str(), server_sockfd[i]);
	}
};
