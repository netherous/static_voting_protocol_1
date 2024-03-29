#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <strings.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <map>
#include <chrono>

using namespace std;
using namespace std::chrono;

vector<pair<string,int>> server_ip;
vector<int>server_sockfd;
map<int,int>fd_node;
stringstream sstream;
void info();

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
	fd_node[sockfd] = i;
	server_sockfd.push_back(sockfd);
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


void update_handler(){
	int x;
	cin >> x;
	x--;
	cout << "update server " << char('A'+x) << endl;
	string r="update";
	if(write(server_sockfd[x], r.c_str(), strlen(r.c_str())) < 0 ) perror("ERROR Writing");
	char buff[10];
	bzero(buff,sizeof(buff));
	if(read(server_sockfd[x], buff, sizeof(buff)) < 0 ) perror("ERROR Reading");
	cout << buff << endl;
	// info();
}
void p1t_handler(){
	cout << "partitioning phase 1" << endl;
	string r="p1t";
	for(int fd: server_sockfd){
		if(write(fd, r.c_str(), r.size()) < 0 ) perror("ERROR Writing");
	}
	// info();
}
void p2t_handler(){
	cout << "partitioning phase 2" << endl;
	string r="p2t";
	for(int fd: server_sockfd){
		if(write(fd, r.c_str(), strlen(r.c_str())) < 0 ) perror("ERROR Writing");
	}
	// info();
}
void m1g_handler(){
	cout << "merging" << endl;
	string r="m1g";
	for(int fd: server_sockfd){
		if(write(fd, r.c_str(), strlen(r.c_str())) < 0 ) perror("ERROR Writing");
	}
	// info();
}
void info(){
	// cout << "requiring info" <<endl;	
	string r="info";
	for(int fd: server_sockfd){
		if(write(fd, r.c_str(), strlen(r.c_str())) < 0 ) perror("ERROR Writing");
		char buff[1000];
		bzero(buff,sizeof(buff));
		if(read(fd, buff, sizeof(buff)) < 0 ) perror("ERROR Reading");
		cout << string(buff) << endl;
	}
	cout << "info end---------" << endl;
}
void launch_process(){
	cout<<"launching client " <<endl;
	read_file("server_ip.txt");
	for(int i = 0 ; i < server_ip.size(); i++){
		create_connection(i);
	}
	for(int fd: server_sockfd){
		string r="client";
		if(write(fd, r.c_str(), strlen(r.c_str())) < 0 ) perror("ERROR Writing");
	}
	while(1){
		string input;
		cin >> input;
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
	
}


int main (int argc, char* argv []){
	// check_program_arguments(argc, argv);
	// int pid = fork();
	launch_process();
	return 0;
}


