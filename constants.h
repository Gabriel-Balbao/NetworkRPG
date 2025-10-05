#ifndef CONFIG_H
#define CONFIG_H

// Network Settings 
#define PORT 5050
#define MAX_PLAYERS 6
#define MIN_PLAYERS 2
#define LOBBY_TIME 5 // seconds

// Protocol Keywords
#define INPUT "INPUT"
#define SYS   "SYS"
#define GAME  "GAME"

// Messages
#define WAITING_MSG "Waiting for connections..."
#define WELCOME_MSG "You're in the lobby!"
#define DISCONNECT_MSG "A player disconnected!"

#endif 