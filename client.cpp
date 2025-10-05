#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <atomic>
#include <sys/select.h>

#include "constants.h"

std::atomic<bool> running{true};

// Thread to receive messages from the server
void receiveMessages(int sock) {
    char buffer[1024];
    while(running.load()){
        memset(buffer, 0, sizeof(buffer));
        int valread = read(sock, buffer, sizeof(buffer)-1);
        if(valread <= 0){
            running.store(false); // server closed
            break;
        }

        buffer[valread] = '\0';
        std::string message(buffer);

        // Check if it is an input prompt
        if(message.rfind(INPUT, 0) == 0){ // starts with INPUT
            // Remove "INPUT" + space
            std::string prompt = message.substr(strlen(INPUT) + 1);
            std::cout << prompt << std::flush;
            continue; // do not print the rest
        }

        // Process messages containing '\r'
        size_t pos = 0;
        while((pos = message.find('\r')) != std::string::npos){
            std::cout << message.substr(0, pos) << "\r" << std::flush;
            message = message.substr(pos + 1);
        }

        if(!message.empty()){
            std::cout << message << std::flush;
        }

        // Detect server shutdown
        if(message.find("Server shutting down") != std::string::npos){
            running.store(false);
            break;
        }
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        std::cerr << "Error creating socket\n"; 
        return 1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){
        std::cerr << "Invalid address\n"; 
        return 1;
    }

    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        std::cerr << "Connection failed\n"; 
        return 1;
    }

    // Thread to receive messages asynchronously
    std::thread recvThread(receiveMessages, sock);

    // Main loop: use select to avoid blocking on stdin
    while(running.load()){
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 0.1s

        int activity = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);

        if(activity > 0 && FD_ISSET(STDIN_FILENO, &readfds)){
            std::string input;
            if(!std::getline(std::cin, input)){
                running.store(false);
                break;
            }
            input += "\n";
            send(sock, input.c_str(), input.size(), 0);
        }

        if(!running.load()) break;
    }

    // Wait for receiver thread to finish
    if(recvThread.joinable()) recvThread.join();
    close(sock);

    return 0;
}
