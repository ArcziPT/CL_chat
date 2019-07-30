#include "Socket.h"
#include <algorithm>
#include <errno.h>
#include "../TextHelper/TextHelper.h"

void Socket::start(int port/*, InterThreadQueue<Text>* msg_queue*/){
    this->port = port; 
    //this->msg_queue = msg_queue;
    
    // socket create and verification 
    server_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if(server_fd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else{
        #ifdef DEBUG
        printf("Socket successfully created..\n"); 
        #endif
    }

    bzero(&servaddr, sizeof(servaddr)); 
    
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(port); 
    
    // Binding newly created socket to given IP and verification 
    if ((bind(server_fd, (sockaddr*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else{
        #ifdef DEBUG
        printf("Socket successfully binded..\n"); 
        #endif
    }

    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    }
    
    int fl = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, fl | O_NONBLOCK);

    //new thread
    std::thread t1(&Socket::server_listen_thread, this);
    lt = std::move(t1);
}

void Socket::_connect(const std::string& ip, int port){ 
    // socket create and varification 
    server_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (server_fd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else{
        #ifdef DEBUG
        printf("Socket successfully created..\n"); 
        #endif
    }

    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str()); 
    servaddr.sin_port = htons(port); 
  
    // connect the client socket to server socket 
    if (connect(server_fd, (sockaddr*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 
    else{
        #ifdef DEBUG
        printf("connected to the server..\n");
        #endif
    }

    int fl = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, fl | O_NONBLOCK);

    tcp_client client;
    client.sock_fd = server_fd;

    clients[0] = client;
}

void Socket::server_listen_thread(){
    while(isRunning){
        int connfd, len; 
        struct sockaddr_in cli;
        
        len = sizeof(cli); 
        // Accept the data packet from client and verification 
        connfd = accept(server_fd, (sockaddr*)&cli, (socklen_t*)&len); 
        if(connfd == -1 && (errno == EWOULDBLOCK or errno == EAGAIN))
            continue;
        
        else if(connfd == -1){
            printf("server acccept failed...\n"); 
            exit(0); 
        }

        int fl = fcntl(connfd, F_GETFL, 0);
        fcntl(connfd, F_SETFL, fl | O_NONBLOCK);

        clients_count++;

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cli.sin_addr), ip, INET_ADDRSTRLEN);
        auto port = ntohs(cli.sin_port);
        //print("Socket", "new client, ip->" + std::string(ip) + ":" + std::to_string(port));

        sleep(1);
        tcp_client client;
        client.sock_fd = connfd;
        client.ip = std::string(ip);
        client.port = port;
        clients[clients_count] = client;
    }
}

void Socket::refresh(){
    char buff[MAX];    
    for(auto& client : clients){ 
        bzero(buff, MAX); 
  
        int sockfd = client.second.sock_fd;

        // read the message from client and copy it in buffer 
        int len = read(sockfd, buff, sizeof(buff));
        if(len == 0){
            close(client.second.sock_fd); //close socket
            disconnected.push_back(client.first);
            //print("Socket", "client disconnected, ip->" + client.second.ip + ":" + std::to_string(client.second.port));
            continue;
        }

        if(len == -1 && (errno == EWOULDBLOCK or errno == EAGAIN))
            continue;
        
        auto str = std::string(buff);         
        auto req_msgs = TextHelper::split(str, (char)0x3); //0x3 is added at the end of packet in case of merging two or more packets

        for(auto req_msg : req_msgs){
            Request* req = new Request();
            req->client_id = client.first;
            req->msg = req_msg;
            requests.push(req);
        }
    }
    for(int i : disconnected){
        clients.erase(i);
    }
}

std::vector<int> Socket::get_disconnected(){
    auto temp = disconnected;
    disconnected.clear();
    return temp;
}

void Socket::stop(){
    while(!requests.empty()){
        auto it = requests.front();
        delete it;
        requests.pop();
    }

    for(auto c : clients){
        close(c.second.sock_fd);
    }

    close(server_fd);

    isRunning = false;

    lt.join();

};

Request* Socket::get_request(){
    if(!requests.empty()){
        auto req = requests.front();
        requests.pop();
        return req;
    }

    return nullptr;
}

void Socket::send_response(const Request& res){
    if(clients.find(res.client_id) == clients.end())
        return;

    char* buf = new char[res.msg.length()+1];
    strcpy(buf, res.msg.c_str());
    buf[res.msg.length()] = 0x3;
    int sockfd = clients[res.client_id].sock_fd;
    
    write(sockfd, buf, res.msg.length()+1);
    delete []buf;
}

/*void Socket::print(const std::string& tag, const std::string& msg){
    //TODO: color
    Text text(tag, msg, 1);
    msg_queue->push(text);
}*/