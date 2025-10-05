# Multiplayer Combat System – Computer Networks Project

## Team Members

- Gabriel Balbão Bazon – NUSP: 13676408  
- João Marcelo Ferreira Battaglini – NUSP: 13835472  
- Leonardo Hannas de Carvalho Santos – NUSP: 11800480  

---

## Project Description

This project implements a **free multiplayer combat system** between two or more players using **TCP sockets** for network communication.  
The main goal is to explore client-server communication, turn-based synchronization, and real-time message exchange to create an interactive combat experience.

### Known Limitation

If a player disconnects **while another is choosing their action**, the server may broke.  
This limitation is documented and does not affect other aspects of the game.

---

## Project Structure

The project is divided into two main parts:

- **Server (`server.cpp`)**: Manages player connections, processes combat commands, and keeps the game state synchronized.  
- **Client (`client.cpp`)**: Allows players to connect to the server, send combat commands, and receive real-time updates about the game state.

---

## Main Features

- **TCP Socket Communication** for networked multiplayer.  
- **Dynamic Lobby with Countdown**: The server automatically starts the match when the minimum number of players is connected, showing a visible countdown to all participants.  
- **Resilient Setup**: The lobby detects player disconnections before the match begins and automatically adjusts the active participant count.  
- **Turn-Based Combat System**: Players alternate turns performing actions.  
- **State Synchronization**: The server maintains the game state and broadcasts updates after every action.  

---

## Technologies Used

- **Language**: C++  
- **Networking Library**: TCP Sockets  

---

## Prerequisites

- **C++ Compiler** (g++ recommended)  
- **Operating System**: Linux, macOS, or Windows with socket support  

---

## How to Compile and Run

### Compiling

Use the provided Makefile to compile both server and client:

`make`

## Executables

This generates two executables:

- `server` – the game server  
- `client` – the client used by players to connect  

---

## Running the Program

### Start the server:

`./server`

### Start the client:

``./client``

# Usage

## Server

### Start the server
Run the server executable. It opens a TCP socket, waits for clients, and maintains a lobby until the required number of players (defined by `MIN_PLAYERS`) is reached.

### Lobby countdown
When enough players are connected, a countdown starts. The server broadcasts the remaining time until the game begins.

## Clients

### Connect to the server
Each player runs a client instance and connects to the server's IP and port.

### Handle disconnections
If a client disconnects during the lobby, the server removes it and resets the countdown if necessary. If the number of players drops below the minimum, the server performs a graceful shutdown.

## Class Selection

### Configure avatars
Once connected, each player is prompted to input their name and class (`Halfling`, `Mage`, or `Orc`).

### Validation
The server validates input and assigns a default class (`Halfling`) if the input is invalid.

### Ready state
Each player is confirmed and waits until all other players complete setup. Only when all players are ready does the combat start.

## Combat

### Turn-based actions
Players take turns performing actions:

0 = ATTACK
1 = CAST_SPELL
2 = SPECIAL_MOVE


### Target selection
During a turn, players select an opponent target from a list of alive players.

### Server processing
The server executes the action, updates HP and status, and broadcasts the result to all clients.

### Disconnection handling
If a player disconnects during combat, the server marks them as dead and notifies all remaining players.

## Game End

### Determine outcome
When a player’s HP reaches zero, the server continues the game until only one or more players remain alive.

### Broadcast results
The server sends the final battle results to all clients.

### Cleanup
All sockets are closed and the server either returns to the lobby state or shuts down if conditions are not met.

## Notes

- **Non-blocking sockets** are used during the lobby to handle multiple connections simultaneously.  
- **Threaded setup:** Each player’s setup is handled in a separate thread for simultaneous configuration.  
- **Synchronized access:** Shared data (player list, sockets) is protected by mutexes.  
- **Graceful shutdown:** The server can send a custom shutdown message to all clients when terminating.




