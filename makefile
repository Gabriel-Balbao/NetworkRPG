# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread

# Executables
SERVER = server
CLIENT = client

# Server source files
SERVER_SRCS = server.cpp controller.cpp \
              characters/character.cpp characters/mage.cpp \
              characters/halfling.cpp characters/orc.cpp \
			  constants.h

# Client source files
CLIENT_SRCS = client.cpp

# Default target: build everything
all: $(SERVER) $(CLIENT)

# Compile the server
$(SERVER): $(SERVER_SRCS)
	$(CXX) $(CXXFLAGS) $(SERVER_SRCS) -o $(SERVER)

# Compile the client
$(CLIENT): $(CLIENT_SRCS)
	$(CXX) $(CXXFLAGS) $(CLIENT_SRCS) -o $(CLIENT)

# Clean executables
clean:
	rm -f $(SERVER) $(CLIENT)
