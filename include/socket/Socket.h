#ifndef SOCKET_H
#define SOCKET_H

#include <unistd.h>
#include <iostream> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <vector>
#include <memory>
#include <queue>
#include <thread>
#include <algorithm>
#include <fcntl.h>
#include <arpa/inet.h>
#include <map>
#include <mutex>

#include "Request.h"
#include "../InterThreadQueue/InterThreadQueue.h"
#include "../TextHelper/Text.h"


#define CLIENT 0
#define SERVER 1
#define MAX 1024

struct tcp_client{
    int sock_fd;
    std::string ip;
    uint16_t port;
    //TODO
};

class Socket{
public:
    void start(int port/*, InterThreadQueue<Text>* msg_queue*/);
    void _connect(const std::string& ip, int port);

    void refresh();
    void stop();
    std::vector<int> get_disconnected();

    Request* get_request();
    void send_response(const Request& res);

private:
    //print
    //InterThreadQueue<Text>* msg_queue;
    //void print(const std::string& tag, const std::string& msg);

    int port;
    int server_fd;
    struct sockaddr_in servaddr;
    int addrlen = sizeof(servaddr);

    bool isRunning = true;

    std::thread lt;

    int clients_count = 0;
    std::map<int, tcp_client> clients;
    std::vector<int> disconnected;
    std::queue<Request*> requests;

    void server_listen_thread();
};

#endif