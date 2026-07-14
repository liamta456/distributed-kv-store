#include "net/socket_utils.h"

#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

#include <winsock2.h>

// Key-value, in-memory store
std::unordered_map<std::string, std::string> store;

// Function declarations
void put(const std::string &key, const std::string &value);
std::optional<std::string> get(const std::string &key);
bool del(const std::string &key);

int main() {
    WSADATA wsaData;
    if (initWinsock(wsaData) == false) {
        return 1;
    }
    std::cout << "Winsock initialized." << std::endl;

    SOCKET serverSock = createSocket();
    if (serverSock == INVALID_SOCKET) {
        cleanupWinsock();
        return 1;
    }
    std::cout << "Server socket created." << std::endl;

    unsigned short port = 3000;
    if (bindAndListen(serverSock, port) == false) {
        closeSocket(serverSock);
        cleanupWinsock();
        return 1;
    };
    std::cout << "Server socket binded and listening on port " << port << "..." << std::endl;

    SOCKET clientSock = acceptClient(serverSock);
    if (clientSock == INVALID_SOCKET) {
        closeSocket(serverSock);
        cleanupWinsock();
        return 1;
    }
    std::cout << "Client connection established." << std::endl;

    while (true) {
        auto data = receiveString(clientSock);
        if (data.has_value()) {
            std::cout << "Received: " << data.value() << std::endl;
        } else {
            std::cout << "Connection closed by client." << std::endl;
            break;
        }  
    }

    closeSocket(serverSock);
    closeSocket(clientSock);
    cleanupWinsock();

    return 0;
}

// Function definitions

void put(const std::string &key, const std::string &value) {
    store.insert_or_assign(key, value);
}

std::optional<std::string> get(const std::string &key) {
    auto iterator = store.find(key);
    if (iterator == store.end()) {
        return std::nullopt;
    }
    return iterator->second;
}

// Returns true if KV was deleted, false if not found
bool del(const std::string &key) {
    return store.erase(key) == 1;
}
