# Define the compiler and flags
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Target executable names
SERVER_TARGET = server
CLIENT_TARGET = client

# The 'all' rule builds both targets by default
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Rule to build the server executable
$(SERVER_TARGET): server.cpp
	$(CXX) $(CXXFLAGS) server.cpp -o $(SERVER_TARGET)

# Rule to build the client executable
$(CLIENT_TARGET): client.cpp
	$(CXX) $(CXXFLAGS) client.cpp -o $(CLIENT_TARGET)

# Clean rule to wipe out the binaries and start fresh
clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET)