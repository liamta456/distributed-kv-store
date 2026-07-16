#include "net/socket_utils.h"

#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <winsock2.h>

int main() {
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

    SOCKET clientSock = INVALID_SOCKET;

    while (true) {
        /* --- Accept Client Connection --- */

        clientSock = acceptClient(listenSock);
        if (clientSock == INVALID_SOCKET) {
            std::cerr << "\nError: Failed to accept client connection." << std::endl;
            continue;
        }
        std::cout << "\nClient connection established." << std::endl;

        while (clientSock != INVALID_SOCKET) {
            /* --- Receive Client Command --- */

            auto commandOpt = receiveString(clientSock);
            if (!commandOpt.has_value()) {
                std::cout << "\nConnection closed by client." << std::endl;
                closeSocket(clientSock);
                break;
            }
            std::cout << "\nReceived from client: " << commandOpt.value() << std::endl;

            /* --- Connect to Storage Node --- */

            SOCKET nodeSock = createSocket();
            if (nodeSock == INVALID_SOCKET) {
                std::cerr << "Error: Failed to create storage node socket." << std::endl;
                closeSocket(clientSock);
                break;
            }
            std::cout << "Storage node socket created." << std::endl;

            unsigned short nodePort = 3000;
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

    /* --- Cleanup and Safe Exit --- */

    closeSocket(clientSock);
    closeSocket(listenSock);
    cleanupWinsock();
    
    return 0;
}
