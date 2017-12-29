#include <iostream>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <chrono>
#include <sstream>
#include <unistd.h>
#include <fstream>
#include <sys/time.h>

using namespace std;

#define MAXLINE 1024 /*max text line length*/
#define BACKLOG 5 /*maximum number of client connections */
#define MAXPORT 6

struct Agent{
    char ip_addr[INET_ADDRSTRLEN];
    std::chrono::steady_clock::time_point start;
};


int get_index(struct sockaddr *client, vector<Agent> v){
    struct in_addr ip_add = ((struct sockaddr_in*)client)->sin_addr;
    char str[INET_ADDRSTRLEN];
    inet_ntop( AF_INET, &ip_add, str, INET_ADDRSTRLEN);
    for(int i = 0; i < v.size(); i++) {
        if(strcmp(v[i].ip_addr, str) == 0){
            return i;
        }
    }
    return -1;
}

int main (int argc, char* argv[]) {
    int sd, n;
    socklen_t client_len;
    struct sockaddr_in client_addr, serv_address;
    int elem = 0;
    char *OK = "$OK";
    char *AM = "$ALREADY MEMBER";
    char *NM = "$NOT MEMBER";
    char* filename = "Logfile.txt";
    vector<Agent> ag;
    ifstream infile(filename);
    if (infile.good())
        remove(filename);
    ofstream file;
    char cPort[MAXPORT];
    int nPort;

    if (argc < 1)
      {
        printf ("Port number is required \r\n");
        return(0);
      }
      memset(cPort, 0, MAXPORT);
      sprintf(cPort,"%s",argv[1]);
      nPort = atoi(cPort);

    // create a new socket to use for communication
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf (stderr, "ERROR: socket() failed\n");
        exit (-1);
    }

    memset((char *)&serv_address, 0, sizeof(serv_address));
    serv_address.sin_family = AF_INET;
    serv_address.sin_addr.s_addr = INADDR_ANY;
    serv_address.sin_port = htons(nPort);

    if(bind(sd, (struct sockaddr *) &serv_address, sizeof(serv_address)) < 0){
        fprintf (stderr, "ERROR: bind() failed\n");
        exit (-1);
    }
    if(listen(sd, BACKLOG) < 0){
      fprintf (stderr, "ERROR: listen() failed\n");
      exit (-1);
    }

    while(true) {
        int recvMsg = 0;
        char buf[MAXLINE];
        client_len = sizeof(client_addr);
        n = accept (sd, (struct sockaddr *) &client_addr, &client_len);
        if(n <= 0){
            fprintf (stderr, "ERROR: accept() failed\n");
            exit (-1);
        }else {
            memset(buf,0,MAXLINE);
            recvMsg = read(n, buf, MAXLINE);
        }

        Agent a;
        int op;
        time_t timev;
        if(strcmp(buf, "#JOIN") == 0){
            op = 1;
        }
        if(strcmp(buf, "#LEAVE") == 0)
            op = 2;
        if(strcmp(buf, "#LIST") == 0)
            op = 3;
        if(strcmp(buf, "#LOG") == 0)
            op = 4;

        char buffer[30];
        struct timeval tv;
        time_t curtime;

        switch(op) {
            case 1:
                {
                struct in_addr ip = client_addr.sin_addr;
                inet_ntop( AF_INET, &ip, a.ip_addr, INET_ADDRSTRLEN);
                file.open(filename, ios::app);
                gettimeofday(&tv, NULL);
                curtime=tv.tv_sec;
                strftime(buffer,30,"%m-%d-%Y:%T.",localtime(&curtime));
                file << buffer;
                file << tv.tv_usec;
                file << ": Received a \"#JOIN\" action from agent ";
                file << "\"" ;
                file << a.ip_addr;
                file << "\"\n";
                if(ag.empty()){
                    a.start = std::chrono::steady_clock::now();
                    ag.push_back(a);
                    send(n, OK, (int)strlen(OK), 0);
                    gettimeofday(&tv, NULL);
                    curtime=tv.tv_sec;
                    strftime(buffer,30,"%m-%d-%Y:%T.",localtime(&curtime));
                    file << buffer;
                    file << tv.tv_usec;
                    file << ": Responded to agent ";
                    file << "\"" ;
                    file << a.ip_addr;
                    file << "\"" ;
                    file << " with \"";
                    file << OK;
                    file << "\"\n";
                    file.close();
                }else {
                    int i = get_index((struct sockaddr *) &client_addr, ag);
                    if(i == -1) {
                        struct in_addr ip = client_addr.sin_addr;
                        inet_ntop( AF_INET, &ip, a.ip_addr, INET_ADDRSTRLEN);
                        a.start = std::chrono::steady_clock::now();
                        ag.push_back(a);
                        send(n, OK, strlen(OK), 0);
                        gettimeofday(&tv, NULL);
                        curtime=tv.tv_sec;
                        strftime(buffer,30,"%m-%d-%Y:%T.",localtime(&curtime));
                        file << buffer;
                        file << tv.tv_usec;
                        file << ": Responded to agent ";
                        file << "\"" ;
                        file << a.ip_addr;
                        file << "\"" ;
                        file << " with \"";
                        file << OK;
                        file << "\"\n";
                        file.close();
                    }else{
                        send(n, AM, strlen(AM), 0);
                        gettimeofday(&tv, NULL);
                        curtime=tv.tv_sec;
                        strftime(buffer,30,"%m-%d-%Y:%T.",localtime(&curtime));
                        file << buffer;
                        file << tv.tv_usec;
                        file << ": Responded to agent ";
                        file << "\"" ;
                        file << a.ip_addr;
                        file << "\"" ;
                        file << " with \"";
                        file << AM;
                        file << "\"\n";
                        file.close();
                    }
                }
                }
                break;
            case 2:
                {
                    struct in_addr ip = client_addr.sin_addr;
                    inet_ntop( AF_INET, &ip, a.ip_addr, INET_ADDRSTRLEN);
                    file.open(filename, ios::app);
                    gettimeofday(&tv, NULL);
                    curtime=tv.tv_sec;
                    strftime(buffer,30,"%m-%d-%Y:%T.",localtime(&curtime));
                    file << buffer;
                    file << tv.tv_usec;
                    file << ": Received a \"#LEAVE\" action from agent ";
                    file << "\"" ;
                    file << a.ip_addr;
                    file << "\"\n";
                    int i = get_index((struct sockaddr *) &client_addr, ag);
                    if(i == -1){
                        send(n, NM, strlen(NM), 0);
                        gettimeofday(&tv, NULL);
                        curtime=tv.tv_sec;
                        strftime(buffer,30,"%m-%d-%Y:%T.",localtime(&curtime));
                        file << buffer;
                        file << tv.tv_usec;
                        file << ": Responded to agent ";
                        file << "\"" ;
                        file << a.ip_addr;
                        file << "\"" ;
                        file << " with \"";
                        file << NM;
                        file << "\"\n";
                        file.close();
                    }else{
                        ag.erase(ag.begin() + i);
                        send(n, OK, strlen(OK), 0);
                        gettimeofday(&tv, NULL);
                        curtime=tv.tv_sec;
                        strftime(buffer,30,"%m-%d-%Y:%T.",localtime(&curtime));
                        file << buffer;
                        file << tv.tv_usec;
                        file << ": Responded to agent ";
                        file << "\"" ;
                        file << a.ip_addr;
                        file << "\"" ;
                        file << " with \"";
                        file << OK;
                        file << "\"\n";
                        file.close();
                    }
                }
                break;
            case 3:
                {
                    string list_request[ag.size()];
                    struct in_addr ip = client_addr.sin_addr;
                    inet_ntop( AF_INET, &ip, a.ip_addr, INET_ADDRSTRLEN);
                    file.open(filename, ios::app);
                    gettimeofday(&tv, NULL);
                    curtime=tv.tv_sec;
                    strftime(buffer,30,"%m-%d-%Y:%T.",localtime(&curtime));
                    file << buffer;
                    file << tv.tv_usec;
                    file << ": Received a \"#LIST\" action from agent ";
                    file << "\"" ;
                    file << a.ip_addr;
                    file << "\"\n";
                    if(get_index((struct sockaddr *) &client_addr, ag) != -1){
                        auto end = std::chrono::steady_clock::now();
                        for(int j = 0; j < ag.size(); j++) {
                            double elapsed_time = double(std::chrono::duration_cast <std::chrono::nanoseconds> (end - ag[j].start).count());
                            elapsed_time = elapsed_time/1e9;
                            std::stringstream ss;
                            ss << "<" << ag[j].ip_addr << ", " << int(elapsed_time) << "secs>";
                            std::string g = ss.str();
                            list_request[j] = g;
                        }
                        gettimeofday(&tv, NULL);
                        curtime=tv.tv_sec;
                        strftime(buffer,30,"%m-%d-%Y:%T.",localtime(&curtime));
                        file << buffer;
                        file << tv.tv_usec;
                        file << ": Responded to agent ";
                        file << "\"" ;
                        file << a.ip_addr;
                        file << "\"" ;
                        file << " with \"";
                        for(int i = 0; i<ag.size(); i++){
                            file << list_request[i];
                        }
                        file << "\"\n";
                        send(n, list_request, sizeof(list_request), 0);
                        file.close();
                    }else{
                        gettimeofday(&tv, NULL);
                        curtime=tv.tv_sec;
                        strftime(buffer,30,"%m-%d-%Y:%T.",localtime(&curtime));
                        file << buffer;
                        file << tv.tv_usec;
                        file << ": No response is supplied to agent ";
                        file << "\"" ;
                        file << a.ip_addr;
                        file << "\"" ;
                        file << " (agent not active)\n";
                        file.close();
                    }
                }
                break;
            case 4:
                {
                    string list_request[ag.size()];
                    struct in_addr ip = client_addr.sin_addr;
                    inet_ntop( AF_INET, &ip, a.ip_addr, INET_ADDRSTRLEN);
                    file.open(filename, ios::app);
                    gettimeofday(&tv, NULL);
                    curtime=tv.tv_sec;
                    strftime(buffer,30,"%m-%d-%Y:%T.",localtime(&curtime));
                    file << buffer;
                    file << tv.tv_usec;
                    file << ": Received a \"#LOG\" action from agent ";
                    file << "\"" ;
                    file << a.ip_addr;
                    file << "\"\n";
                    if(get_index((struct sockaddr *) &client_addr, ag) != -1){
                        gettimeofday(&tv, NULL);
                        curtime=tv.tv_sec;
                        strftime(buffer,30,"%m-%d-%Y:%T.",localtime(&curtime));
                        file << buffer;
                        file << tv.tv_usec;
                        file << ": Responsed to agent ";
                        file << "\"" ;
                        file << a.ip_addr;
                        file << "\"" ;
                        file << " with log file\n";
                        file.close();
                    }else{
                        gettimeofday(&tv, NULL);
                        curtime=tv.tv_sec;
                        strftime(buffer,30,"%m-%d-%Y:%T.",localtime(&curtime));
                        file << buffer;
                        file << tv.tv_usec;
                        file << ": No response is supplied to agent ";
                        file << "\"" ;
                        file << a.ip_addr;
                        file << "\"" ;
                        file << " (agent not active)\n";
                        file.close();

                    }
                    char sendbuf[MAXLINE]; //send buffer
                    FILE *fp = fopen(filename, "rb");
                    if(fp == NULL){
                        cout << "File Open Error" << endl;
                        exit (-1);
                    }
                    bzero(sendbuf, MAXLINE);
                    int bytes_read;
                    while(bytes_read = fread(sendbuf, sizeof(char), MAXLINE, fp) > 0){
                        if(send(n, sendbuf, bytes_read, 0) < 0){
                            fprintf (stderr, "ERROR: send() failed\n");
                            exit (-1);
                        }
                        bzero(sendbuf, MAXLINE);
                    }
                }
                break;
        }

        if (recvMsg < 0) {
            fprintf (stderr, "ERROR: recv() failed\n");
            exit (-1);
        }

        close(n);
    }
    close(sd);
    //file.close();
}
