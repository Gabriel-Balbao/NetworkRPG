#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>
#include <fcntl.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>

#include "controller.h"
#include "characters/character.h"
#include "characters/mage.h"
#include "characters/halfling.h"
#include "characters/orc.h"
#include "constants.h"

// Globals
std::vector<Character *> players;
std::vector<int> clientSockets; // entries set to -1 when socket closed

std::mutex playersMutex;
std::atomic<int> readyPlayers{0};
std::atomic<bool> serverRunning{true};

// Broadcast to all clients. Skip invalid sockets (marked as -1).
void broadcastMessage(const std::string& msg) {
    std::lock_guard<std::mutex> lock(playersMutex);
    for (size_t i = 0; i < clientSockets.size(); ++i) {
        int sock = clientSockets[i];
        if (sock < 0) continue; // skip closed entries
        int res = send(sock, msg.c_str(), msg.size(), 0);
        if (res < 0) {
            // If send fails, close and mark entry -1 to avoid reuse and future errors.
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("send failed in broadcastMessage; closing socket");
                close(sock);
                clientSockets[i] = -1;
            }
        }
    }
}

// Graceful shutdown helper. Sends a shutdown message to all clients, closes their sockets, and stops the server
void shutdownServer(const std::string& message = "Server is shutting down.\n") {
    std::lock_guard<std::mutex> lock(playersMutex);

    for(size_t i = 0; i < clientSockets.size(); ++i){
        int sock = clientSockets[i];
        if(sock < 0) continue;

        // Sends custom message, closes client socket and marks as closed
        send(sock, message.c_str(), message.size(), 0); 
        close(sock); 
        clientSockets[i] = -1;
    }

    serverRunning.store(false); // Mark server as stopped
    std::cout << "SHUTDOWN: " << message;
}

void handlePlayerSetup(int sock){
    // Send configuration prompt
    std::string askMsg = std::string(INPUT) +
                         " Configure your avatar. Type your name and your class (ex.: Conan Halfling): ";
    send(sock, askMsg.c_str(), askMsg.size(), 0);

    std::string input;
    char ch;
    while(serverRunning.load()){
        int res = recv(sock, &ch, 1, MSG_PEEK | MSG_DONTWAIT);

        // Handles player disconnection during setup, removes them, and shuts down the server if too few players remain
        if(res == 0){
            std::cout << "Player disconnected during avatar setup!\n";
            close(sock);

            // Safely removes the disconnected player's socket from the client list using a mutex lock
            {
                std::lock_guard<std::mutex> lock(playersMutex);
                auto it = std::find(clientSockets.begin(), clientSockets.end(), sock);
                if (it != clientSockets.end()) {
                    clientSockets.erase(it);
                }
            }

            if(clientSockets.size() < MIN_PLAYERS){
                shutdownServer("Insufficient players in lobby!");
            }

            return;
        }   
        // Handles non-blocking read behavior and errors during setup, retrying or closing the socket if needed
        else if(res < 0){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }
            perror("recv error during setup");
            close(sock);

            return;
        }

        // Actual read
        res = recv(sock, &ch, 1, 0);
        if(res <= 0) continue;
        if(ch == '\n') break;

        input.push_back(ch);
    }

    if(!serverRunning.load()) return;

    // Parse input
    std::istringstream iss(input);
    std::string name, classType;
    iss >> name >> classType;

    Character* player = nullptr;
    if(classType == "Halfling") player = new Halfling(name);
    else if(classType == "Mage") player = new Mage(name);
    else if(classType == "Orc") player = new Orc(name);
    else player = new Halfling(name); // fallback

    // Find index of this socket in clientSockets and assign it
    int index = -1;
    {
        std::lock_guard<std::mutex> lock(playersMutex);
        for(int i = 0; i < (int) clientSockets.size(); i++){
            if(clientSockets[i] == sock){
                index = i;
                break;
            }
        }

        player->setSocketIndex(index);
        players.push_back(player);
    }

    // Confirmation message
    std::string confirmMsg = "You selected " + name + ", race of " + player->getClass() + "!\n"
                             "Please wait while others finish.\n";
    send(sock, confirmMsg.c_str(), confirmMsg.size(), 0);

    readyPlayers++;
}

int main(){
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creates a TCP socket for the server
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Configures the socket to allow address and port reuse
    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0){
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Sets the server socket to non-blocking mode
    int flags = fcntl(server_fd, F_GETFL, 0);
    if(fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) < 0){
        perror("fcntl F_SETFL failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Initializes the server address structure with IPv4, any incoming IP, and the specified port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Binds the server socket to the specified IP address and port
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Puts the server socket into listening mode with a backlog of 5, exiting on failure
    if(listen(server_fd, 5) < 0){
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << std::string(WAITING_MSG) << std::endl;

    time_t countdownStart = 0;
    bool countdownRunning = false;
    int lastRemaining = -1;

    // Lobby loop
    while(clientSockets.size() < MAX_PLAYERS && serverRunning.load()){
        int newSock = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen);

        if(newSock >= 0){ 
            // Adds a new client socket, sets it to non-blocking and sends a welcome message
            clientSockets.push_back(newSock);

            int flags = fcntl(newSock, F_GETFL, 0);
            fcntl(newSock, F_SETFL, flags | O_NONBLOCK);

            std::string welcomeMsg = std::string(WELCOME_MSG) + " Currently " + std::to_string(clientSockets.size()) + " player(s) here.\n";
            send(newSock, welcomeMsg.c_str(), welcomeMsg.size(), 0);

            std::cout << "Player connected! (" << clientSockets.size() << "/" << MAX_PLAYERS << ")\n" << std::flush; 

            countdownStart = time(nullptr); // Reset countdown timer
        }

        // Checks for disconnected clients in the lobby, removes them, and notifies others
        for(auto it = clientSockets.begin(); it != clientSockets.end();){
            int sock = *it;
            char buf;
            int res = recv(sock, &buf, 1, MSG_PEEK);

            if(res == 0 || (res < 0 && errno != EAGAIN && errno != EWOULDBLOCK)){
                close(sock);
                it = clientSockets.erase(it);

                std::string disconMsg = std::string(DISCONNECT_MSG) + " Now " + std::to_string(clientSockets.size()) + "/" +
                    std::to_string(MAX_PLAYERS) + " players in lobby.\n";
                broadcastMessage(disconMsg);

                std::cout << disconMsg << std::flush;

                countdownStart = time(nullptr); // Reset countdown timer
            } 
            else ++it;
        }

        countdownRunning = (clientSockets.size() >= MIN_PLAYERS);

        // Handles the lobby countdown, broadcasting remaining time and starting the game when it reaches zero
        if(countdownRunning){
            int elapsed = (int)(time(nullptr) - countdownStart);
            int remaining = LOBBY_TIME - elapsed;

            if(remaining != lastRemaining){
                std::string countMsg = "Game starts in " + std::to_string(remaining) + "s...\r";
                std::cout << countMsg << std::flush;
                broadcastMessage(countMsg);
                lastRemaining = remaining;
            }
            if(remaining <= 0){
                std::cout << "Starting game!\n";
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Shuts down the server socket and exits if the server is no longer running
    if(!serverRunning.load()){
        shutdownServer();
        close(server_fd);
        return 0;
    }

    broadcastMessage("Game starting with " + std::to_string(clientSockets.size()) + " players. Get ready!\n\n");

    // Creates and launches a setup thread for each connected player
    std::vector<std::thread> threads;
    for(int sock : clientSockets){
        threads.emplace_back(handlePlayerSetup, sock);
    }

    // Waits for all player setup threads to finish execution
    for(auto& t : threads){
        if(t.joinable()) t.join();
    }

    // Shuts down the server socket and exits if the server is no longer running
    if(!serverRunning.load()){
        shutdownServer();
        close(server_fd);
        return 0;
    }

    broadcastMessage("All players are ready. Let's start!\n\n");

    // Temporarily set client sockets to blocking for the game loop
    {
        std::lock_guard<std::mutex> lock(playersMutex);
        for (int i = 0; i < (int)clientSockets.size(); ++i) {
            int sock = clientSockets[i];
            if (sock < 0) continue;
            int f = fcntl(sock, F_GETFL, 0);
            fcntl(sock, F_SETFL, f & ~O_NONBLOCK);
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Checks if there are enough players; shuts down the server if below the minimum
    {
        std::lock_guard<std::mutex> lock(playersMutex);
        if(players.size() < MIN_PLAYERS){
            shutdownServer();
            close(server_fd);
            return 0;
        }
    }

    // Creates controller
    Controller controller(players);

    while(!controller.isBattleOver() && serverRunning.load()){
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        Character *current = controller.getCurrentPlayer();
        int index = current->getSocketIndex();
        int sock = clientSockets[index];

        // Skips the turn if the current player is dead or disconnected
        if(!current->isAlive() || sock < 0){
            controller.nextTurn();
            continue;
        }

        // Prompts the player for their action, handles input, validates it, and manages disconnections
        int action = -1;
        while(true){
            std::string actionMsg = std::string(INPUT) +
                                    " Your turn! Choose action (0=ATTACK, 1=CAST_SPELL, 2=SPECIAL_MOVE): ";
            send(sock, actionMsg.c_str(), actionMsg.size(), 0);

            std::string input;
            char ch;
            while(true){
                int n = recv(sock, &ch, 1, 0);
                if(n <= 0){ // Disconnection
                    close(sock);
                    clientSockets[index] = -1;
                    current->setDead();
                    broadcastMessage(current->getName() + " disconnected and is out!\n");

                    break;
                }
                if(ch == '\n') break;

                input.push_back(ch);
            }

            if(clientSockets[index] < 0 || !current->isAlive()) break;

            action = atoi(input.c_str());
            if(action >= 0 && action <= 2) break;

            send(sock, "Invalid action! Try again.\n", 28, 0);
        }

        // Skips the turn if the current player is dead or disconnected
        if(!current->isAlive() || clientSockets[index] < 0){
            controller.nextTurn();
            continue;
        }

        // Prompts the player to choose a valid target, handling input and disconnections
        int targetIndex = -1;
        while(true){
            std::ostringstream targetList;
            targetList << "Choose target:\n";
            for(size_t i = 0; i < players.size(); ++i){
                if(players[i] == current || !players[i]->isAlive()) continue;
                targetList << i << ": " << players[i]->getName()
                        << " (HP: " << players[i]->getHealth() << ", Alive)\n";
            }
            send(sock, targetList.str().c_str(), targetList.str().size(), 0);

            std::string input;
            char ch;
            while(true){
                int n = recv(sock, &ch, 1, 0);
                if(n <= 0){ // Disconnection
                    close(sock);
                    clientSockets[index] = -1;
                    current->setDead();
                    broadcastMessage(current->getName() + " disconnected and is out!\n");
                    break;
                }
                if(ch == '\n') break;
                input.push_back(ch);
            }

            if(clientSockets[index] < 0 || !current->isAlive()) break;

            targetIndex = atoi(input.c_str());
            if(targetIndex >= 0 && targetIndex < (int)players.size() &&
                players[targetIndex] != current &&
                players[targetIndex]->isAlive()) break;

            send(sock, "Invalid target! \n", 15, 0);
        }

        // Skips the turn if the current player is dead or disconnected
        if(!current->isAlive() || clientSockets[index] < 0){
            controller.nextTurn();
            continue;
        }

        // Executes the chosen action on the target and broadcasts the result to all players
        Character *target = players[targetIndex];
        ActionResult result = controller.applyAction(current, action, target);

        std::ostringstream resultMsg;
        if(result.isError)
            resultMsg << "Error: " << result.message << "\n";
        else
            resultMsg << current->getName() << " used action on " << target->getName() << ". " 
                    << result.message;
        broadcastMessage(resultMsg.str());

        // Turn final state message
        std::ostringstream statusMsg;
        statusMsg << "\n==== Status after this turn ====\n";
        for(size_t i = 0; i < players.size(); ++i) {
            statusMsg << i << ": " << players[i]->getName()
                    << " (HP: " << players[i]->getHealth()
                    << ", " << (players[i]->isAlive() ? "Alive" : "Dead") << ")\n";
        }
        statusMsg << "================================\n\n";
        broadcastMessage(statusMsg.str());

        // Next turn
        controller.nextTurn();
    }

    // End of the game
    broadcastMessage("Battle is over!\n");
    for(int sock : clientSockets){
        if(sock >= 0) close(sock);
    }

    close(server_fd);

    return 0;
}
