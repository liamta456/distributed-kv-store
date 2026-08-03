#include "net/socket_utils.h"

#include <climits>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <winsock2.h>

std::vector<unsigned short> nodePorts = {3000, 3001, 3002};
std::map<unsigned int, unsigned short> ring; // Entry: (hash, port)

void populateRing() {
    for (int i = 0; i < nodePorts.size(); i++) {
        ring.insert({UINT_MAX / nodePorts.size() * i, nodePorts[i]});
    }
}

unsigned int hashKey(const std::string &key) {
    return std::hash<std::string>{}(key);
}

std::optional<std::string> extractKey(const std::string &command) {
    std::stringstream commandStream(command);
    std::string op;
    std::string key;

    if (!(commandStream >> op) || !(commandStream >> key)) {
        return std::nullopt;
    }

    return key;
}

void handleClient(SOCKET clientSock) {
    while (clientSock != INVALID_SOCKET) {
        /* --- Receive Client Command --- */

        auto commandOpt = receiveString(clientSock);
        if (!commandOpt.has_value()) {
            std::cout << "\nConnection closed by client." << std::endl;
            closeSocket(clientSock);
            break;
        }
        std::cout << "\nReceived from client: " << commandOpt.value() << std::endl;

        /* --- Choose Proper Storage Node --- */

        auto keyOpt = extractKey(commandOpt.value());
        if (!keyOpt.has_value()) {
            std::cout << "Error: Failed to extract key from client command." << std::endl;
            closeSocket(clientSock);
            break;
        }

        unsigned int keyHash = hashKey(keyOpt.value());

        auto ringIt = ring.lower_bound(keyHash);
        if (ringIt == ring.end()) {
            ringIt = ring.begin();
        }

        unsigned short nodePort = ringIt->second;
        std::cout << "Routing key='" << keyOpt.value() << "' to port " << nodePort << "." << std::endl;

        /* --- Connect to Storage Node --- */

        SOCKET nodeSock = createSocket();
        if (nodeSock == INVALID_SOCKET) {
            std::cerr << "Error: Failed to create storage node socket." << std::endl;
            closeSocket(clientSock);
            break;
        }
        std::cout << "Storage node socket created." << std::endl;

        if (connectSocket(nodeSock, nodePort) == false) {
            std::cerr << "Error: Failed to connect to storage node." << std::endl;
            closeSocket(nodeSock);
            closeSocket(clientSock);
            break;
        }
        std::cout << "Storage node connection established." << std::endl;

        /* --- Send Client Command to Storage Node --- */

        if (sendString(nodeSock, commandOpt.value()) == false) {
            std::cerr << "Error: Failed to send command to storage node." << std::endl;
            closeSocket(nodeSock);
            closeSocket(clientSock);
            break;
        }

        /* --- Receive Response from Storage Node --- */

        auto resOpt = receiveString(nodeSock);
        if (!resOpt.has_value()) {
            std::cout << "Connection closed by storage node." << std::endl;
            closeSocket(nodeSock);
            closeSocket(clientSock);
            break;
        }
        std::cout << "Received from storage node: " << resOpt.value() << std::endl;

        /* --- Send Response to Client --- */

        if (sendString(clientSock, resOpt.value()) == false) {
            std::cerr << "Error: Failed to send response to client." << std::endl;
            closeSocket(nodeSock);
            closeSocket(clientSock);
            break;
        }

        /* --- Close Connection to Node --- */

        closeSocket(nodeSock);
    }
}

int main() {
    populateRing();

    std::cout << "--- KV STORE: PROXY API ---" << std::endl;

    /* --- WSA Setup --- */

    WSADATA wsaData;
    if (initWinsock(wsaData) == false) {
        return 1;
    }
    std::cout << "\nWinsock initialized." << std::endl;

    /* --- Listen for Client Connection --- */

    SOCKET listenSock = createSocket();
    if (listenSock == INVALID_SOCKET) {
        std::cerr << "\nError: Failed to create listening socket." << std::endl;
        cleanupWinsock();
        return 1;
    }
    std::cout << "\nListening socket created." << std::endl;

    unsigned short listenPort = 3030;
    if (bindAndListen(listenSock, listenPort) == false) {
        std::cerr << "Error: Failed to bind and listen from listening socket." << std::endl;
        closeSocket(listenSock);
        cleanupWinsock();
        return 1;
    };
    std::cout << "Listening socket binded and listening on port " << listenPort << "..." << std::endl;

    /* --- Main Loop: Data Transfer Between Client and Server --- */

    while (true) {
        /* --- Accept Client Connection --- */
        
        SOCKET clientSock = INVALID_SOCKET;
        clientSock = acceptClient(listenSock);
        if (clientSock == INVALID_SOCKET) {
            std::cerr << "\nError: Failed to accept client connection." << std::endl;
            continue;
        }
        std::cout << "\nClient connection established." << std::endl;

        std::thread(handleClient, clientSock).detach();
    }

    /* --- Cleanup and Safe Exit --- */

    closeSocket(listenSock);
    cleanupWinsock();
    
    return 0;
}
